/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Jens Mittag
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jens Mittag <jens.mittag@kit.edu>
 */

#include "ns3/core-module.h"
#include "ns3/common-module.h"
#include "ns3/node-module.h"
#include "ns3/helper-module.h"
#include "ns3/wifi-module.h"
#include "ns3/physim-wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/contrib-module.h"

#include <itpp/itcomm.h>

#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <numeric>

NS_LOG_COMPONENT_DEFINE ("Main");

using namespace ns3;

class Experiment
{

public:
  void
  Run (std::string wifiMode, uint32_t seed, double txpower, uint32_t packetSize, double simulationTime, double pathLoss, double bgNoise)
  {

    // Save experiment information
    m_wifiMode = wifiMode;
    m_packetSize = packetSize;
    m_pathLoss = pathLoss;
    m_sinr = (txpower - pathLoss - bgNoise);

    // Set the counter for all statistics to zero
    m_txCount[m_sinr] = 0;
    m_rxOkCount[m_sinr] = 0;
    m_rxErrorCount[m_sinr] = 0;
    m_hdrOkCount[m_sinr] = 0;
    m_hdrErrorCount[m_sinr] = 0;
    m_syncOkCount[m_sinr] = 0;
    m_syncErrorCount[m_sinr] = 0;

    // Disable fragmentation
    Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));

    // Do not use a fixed scrambler
    Config::SetDefault ("ns3::PhySimScrambler::UseFixedScrambler", BooleanValue (false) );

    // Configure PhySimWifiPhy
    Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerEnd", DoubleValue (txpower) );
    Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerStart", DoubleValue (txpower) );
    Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerLevels", UintegerValue (1) );
    Config::SetDefault ("ns3::PhySimWifiPhy::TxGain", DoubleValue (0.0) );
    Config::SetDefault ("ns3::PhySimWifiPhy::RxGain", DoubleValue (0.0) );

    // Set the correct Channel Estimator implementation
    Config::SetDefault ("ns3::PhySimWifiPhy::ChannelEstimator", StringValue ("ns3::PhySimChannelFrequencyOffsetEstimator") );

    // Set noise floor of -99 dBm
    Config::SetDefault ("ns3::PhySimInterferenceHelper::NoiseFloor", DoubleValue (bgNoise) );

    // Enable/Disable Soft-Decision Viterbi
    // Method 1: Soft Decision
    Config::SetDefault ("ns3::PhySimConvolutionalEncoder::SoftViterbiDecision", BooleanValue (false));
    Config::SetDefault ("ns3::PhySimOFDMSymbolCreator::SoftViterbiDecision", BooleanValue (false));

    PhySimWifiChannelHelper wifiChannel;
    wifiChannel.AddPropagationLoss ("ns3::PhySimPropagationLossModel");

    // In the future, we can also add more
    wifiChannel.AddPropagationLoss ("ns3::PhySimConstantPropagationLoss");
    Config::SetDefault ("ns3::PhySimConstantPropagationLoss::PathLoss", DoubleValue (m_pathLoss) );
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");

    SeedManager::SetSeed (seed);

    PhySimWifiPhyHelper wifiPhy = PhySimWifiPhyHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());

    WifiHelper wifi = WifiHelper::Default ();
    wifi.SetStandard(WIFI_PHY_STANDARD_80211a);
    NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
    wifiMac.SetType ("ns3::AdhocWifiMac");
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue (wifiMode),
                                  "NonUnicastMode", StringValue (wifiMode));

    NodeContainer nodes;
    nodes.Create (2);

    // Install WifiPhy and set up mobility
    wifi.Install (wifiPhy, wifiMac, nodes);

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    positionAlloc->Add (Vector (1.0, 0.0, 0.0));
    mobility.SetPositionAllocator (positionAlloc);
    mobility.Install (nodes);

    // Add packet socket handlers
    PacketSocketHelper packetSocket;
    packetSocket.Install (nodes);

    // Configure packet socket for receiving node 0
    PacketSocketAddress socketOn0;
    socketOn0.SetAllDevices ();
    socketOn0.SetPhysicalAddress (Mac48Address::GetBroadcast ());
    socketOn0.SetProtocol (1);
    PacketSinkHelper sink ("ns3::PacketSocketFactory", Address (socketOn0));

    // Install OnOffApplication on transmitting node 1
    PacketSocketAddress socketTo0;
    socketTo0.SetAllDevices ();
    socketTo0.SetPhysicalAddress (Mac48Address::GetBroadcast ());
    socketTo0.SetProtocol (1);
    OnOffHelper onOff ("ns3::PacketSocketFactory", Address (socketTo0));
    onOff.SetAttribute ("PacketSize", UintegerValue (m_packetSize));
    onOff.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (1.0)));
    onOff.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0.0)));
    uint32_t datarate = 10 * m_packetSize;
    std::ostringstream oss;
    oss.str ("");
    oss << datarate << "B/s";
    // Set the data rate
    onOff.SetAttribute ("DataRate", DataRateValue (DataRate (oss.str ()) ));

    // Configure start/stop times of the application/sink
    ApplicationContainer app;
    app = sink.Install (nodes.Get (1));
    app = onOff.Install (nodes.Get (0));
    app.Start (Seconds (1.0));
    app.Stop (Seconds (simulationTime - 1.0));

    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/Tx", MakeCallback (&Experiment::PhyTxTrace, this) );
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/RxOk", MakeCallback (&Experiment::PhyRxOkTrace, this) );
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/HeaderOk", MakeCallback (&Experiment::PhyHeaderOkTrace, this) );
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PreambleOk", MakeCallback (&Experiment::PhyPreambleOkTrace, this) );
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/RxError", MakeCallback (&Experiment::PhyRxErrorTrace, this) );
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/HeaderError", MakeCallback (&Experiment::PhyHeaderErrorTrace, this) );
    Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PreambleError", MakeCallback (&Experiment::PhyPreambleErrorTrace, this) );

    Simulator::Run ();
    Simulator::Destroy ();
  }

  void Save (std::string filename, uint32_t seed, uint32_t size)
  {

    std::stringstream sOut, pOut;
    sOut << seed;
    pOut << size;
    filename.append ("-PayloadSize-");
    filename.append (pOut.str ());
    filename.append ("-Seed-");
    filename.append (sOut.str ());
    std::string target;
    std::ofstream out;
    std::map<double, uint32_t>::iterator it;

    // (1) Save probability of frame reception w.r.t pathLoss-based SINR calculation
    target = filename;
    target.append ("-PR-RxOk-wrt-PathLoss-SINR.data");
    out.open ( target.c_str () );
    out << std::endl << "# Probability of Frame reception w.r.t. SINR (calculated after PathLoss):" << std::endl;

    for ( it = m_txCount.begin () ; it != m_txCount.end (); it++ )
      {
        double sinr = it->first;
        uint32_t txCount = it->second;
        uint32_t rxOkCount = m_rxOkCount[sinr];
        uint32_t rxErrorCount = m_rxErrorCount[sinr];
        double rxRatio = ((double) rxOkCount / (double) txCount);

        out << "SINR: " << sinr << " Ratio: " << rxRatio
            << " ( rxOkCount = " << rxOkCount << ", rxErrorCount = " << rxErrorCount << ", txCount = " << txCount << " )" << std::endl;
      }
    out.close ();

    // (2) Save probability of successful header decode w.r.t pathLoss-based SINR calculation
    target = filename;
    target.append ("-PR-HeaderOk-wrt-PathLoss-SINR.data");
    out.open ( target.c_str () );
    out << std::endl << "# Probability of successful header decode w.r.t. SINR (calculated after PathLoss):" << std::endl;
    for ( it = m_txCount.begin () ; it != m_txCount.end (); it++ )
      {
        double sinr = it->first;
        uint32_t txCount = it->second;
        uint32_t hdrOkCount = m_hdrOkCount[sinr];
        uint32_t hdrErrorCount = m_hdrErrorCount[sinr];
        double hdrRatio = ((double) hdrOkCount / (double) txCount);

        out << "SINR: " << sinr << " Ratio: " << hdrRatio
            << " ( hdrOkCount = " << hdrOkCount << ", hdrErrorCount = " << hdrErrorCount << ", txCount = " << txCount << " )" << std::endl;
      }
    out.close ();

    // (3) Save probability of successful preamble detection w.r.t pathLoss-based SINR calculation
    target = filename;
    target.append ("-PR-PreambleOk-wrt-PathLoss-SINR.data");
    out.open ( target.c_str () );
    out << std::endl << "# Probability of successful preamble detection w.r.t. SINR (calculated after PathLoss):" << std::endl;
    for ( it = m_txCount.begin () ; it != m_txCount.end (); it++ )
      {
        double sinr = it->first;
        uint32_t txCount = it->second;
        uint32_t syncOkCount = m_syncOkCount[sinr];
        uint32_t syncErrorCount = m_syncErrorCount[sinr];
        double syncRatio = ((double) syncOkCount / (double) txCount);

        out << "SINR: " << sinr << " Ratio: " << syncRatio
            << " ( syncOkCount = " << syncOkCount << ", syncErrorCount = " << syncErrorCount << ", txCount = " << txCount << " )" << std::endl;
      }
    out.close ();
  }

private:
  void
  PhyTxTrace (std::string context, Ptr<const Packet> p, Ptr<const PhySimWifiPhyTag> tag)
  {
    m_txCount[m_sinr]++;
  }

  void
  PhyRxOkTrace (std::string context, Ptr<const Packet> p, Ptr<const PhySimWifiPhyTag> tag)
  {
    m_rxOkCount[m_sinr]++;
  }

  void
  PhyHeaderOkTrace (std::string context, Ptr<const Packet> p, Ptr<const PhySimWifiPhyTag> tag)
  {
    m_hdrOkCount[m_sinr]++;
  }

  void
  PhyPreambleOkTrace (std::string context, Ptr<const Packet> p, Ptr<const PhySimWifiPhyTag> tag)
  {
    m_syncOkCount[m_sinr]++;
  }

  void
  PhyRxErrorTrace (std::string context, Ptr<const Packet> p, Ptr<const PhySimWifiPhyTag> tag, enum PhySimWifiPhy::ErrorReason reason)
  {
    m_rxErrorCount[m_sinr]++;
  }

  void
  PhyHeaderErrorTrace (std::string context, Ptr<const Packet> p, Ptr<const PhySimWifiPhyTag> tag, enum PhySimWifiPhy::ErrorReason reason)
  {
    m_hdrErrorCount[m_sinr]++;
    m_rxErrorCount[m_sinr]++;
  }

  void
  PhyPreambleErrorTrace (std::string context, Ptr<const Packet> p, Ptr<const PhySimWifiPhyTag> tag, enum PhySimWifiPhy::ErrorReason reason)
  {
    m_syncErrorCount[m_sinr]++;
    m_hdrErrorCount[m_sinr]++;
    m_rxErrorCount[m_sinr]++;
  }

  std::string m_wifiMode;
  double m_pathLoss;
  double m_sinr;
  uint32_t m_packetSize;

  std::map<double, uint32_t> m_txCount;
  std::map<double, uint32_t> m_rxOkCount;
  std::map<double, uint32_t> m_rxErrorCount;
  std::map<double, uint32_t> m_hdrOkCount;
  std::map<double, uint32_t> m_hdrErrorCount;
  std::map<double, uint32_t> m_syncOkCount;
  std::map<double, uint32_t> m_syncErrorCount;

};


int
main (int argc, const char *argv[])
{
  Experiment e;

  if (argc < 7)
    {
      std::cout << "Usage: " << argv[0] << " --mode <wifiMode> --payload <frameSize> --seed <no>" << std::endl;
      std::cout << "Will use default parameters now." << std::endl;
    }

  // Define Wifi modes to test
  std::string wifiMode = "OfdmRate6Mbps";
  uint32_t payloadSize = 500;
  uint32_t seed = 1;

  for (int i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "--mode") == 0)
        {
          wifiMode = argv[i + 1];
          i++;
        }
      else if (strcmp (argv[i], "--payload") == 0)
        {
          payloadSize = atoi (argv[i + 1]);
          i++;
        }
      else if (strcmp (argv[i], "--seed") == 0)
        {
          seed = atoi (argv[i + 1]);
          i++;
        }
    }

  // Define TxPower
  double txPower = 20.0;                 // 20.0 dBm
  // Other values
  double stepSize = 1.0;
  double simulationTime = 102.0;        // = 1000 transmissions (10 frames/second)
  double minPathLoss = 90.0;            // =  30 dB SINR
  double maxPathLoss = 130.0;           // = -10 dB SINR
  double backgroundNoise = -99.0;       // Background noise is set to -99 dBm

  std::cout << "Running experiment with '" << wifiMode << "' and a payload size of '" << payloadSize << "' bytes:" << std::endl;
  std::cout << " - Transmission Power = " << txPower << " dBm" << std::endl;
  std::cout << " - Background Noise   = " << backgroundNoise << " dBm" << std::endl;
  std::cout << " - Minimum PathLoss   = " << minPathLoss << " dB" << std::endl;
  std::cout << " - Minimum PathLoss   = " << maxPathLoss << " dB" << std::endl;

  // Repeat experiment with different pathloss settings in order to
  // obtain a reception curve w.r.t. the SNR of a packet
  for (double d = minPathLoss; d <= maxPathLoss; d += stepSize)
    {
      std::cout << "--> simulating pathloss of " << d << "dB" << std::endl;
      e.Run (wifiMode, seed, txPower, payloadSize, simulationTime, d, backgroundNoise);
    }

  e.Save (wifiMode, seed, payloadSize);

  return 0;
}
