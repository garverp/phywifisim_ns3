/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Jens Mittag, Stylianos Papanastasiou
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
 * Authors:
 *      Jens Mittag <jens.mittag@kit.edu>
 *      Stylianos Papanastasiou <stylianos@gmail.com>
 */

#include "ns3/config.h"
#include "ns3/test.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/packet.h"
#include "ns3/enum.h"
#include "ns3/physim-wifi-helper.h"
#include "ns3/random-variable.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/data-rate.h"
#include "ns3/physim-wifi-phy-tag.h"
#include "ns3/physim-wifi-phy.h"
#include "ns3/pointer.h"
#include "ns3/propagation-delay-model.h"
#include "physim-wifi-simple-estimator-test.h"

NS_LOG_COMPONENT_DEFINE ("PhySimWifiSimpleEstimatorTest");

namespace ns3 {

PhySimWifiSimpleEstimatorTest::PhySimWifiSimpleEstimatorTest ()
  : TestCase ("PhySim simple channel estimator test case")
{
}

PhySimWifiSimpleEstimatorTest::~PhySimWifiSimpleEstimatorTest ()
{
}

void
PhySimWifiSimpleEstimatorTest::DoRun (void)
{
  PhySimWifiPhy::ClearCache ();
  PhySimWifiPhy::ResetRNG ();

  // Provide known seed for predictable results
  SeedManager::SetSeed (1);

  std::string modes[] = { "OfdmRate3MbpsBW10MHz","OfdmRate4_5MbpsBW10MHz","OfdmRate6MbpsBW10MHz","OfdmRate9MbpsBW10MHz","OfdmRate12MbpsBW10MHz","OfdmRate18MbpsBW10MHz","OfdmRate24MbpsBW10MHz","OfdmRate27MbpsBW10MHz"};
  for (int i = 0; i < 8; ++i)
    {
      RunSingle (modes[i]);
      if (!m_success)
        {
          NS_LOG_DEBUG ("FAIL: Simple channel estimator for " << modes[i]);
        }
      else
        {
          NS_LOG_DEBUG ("PASS: Simple channel estimator for " << modes[i]);
        }
      NS_TEST_EXPECT_MSG_EQ ( m_success, true, "Simple channel estimator test failed for " << modes[i] << " : frame was not successfully decoded");
    }
}


bool
PhySimWifiSimpleEstimatorTest::RunSingle (std::string wifiMode)
{
  m_success = false;

  // Disable fragmentation
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));

  // Do not use a fixed scrambler
  Config::SetDefault ("ns3::PhySimScrambler::UseFixedScrambler", BooleanValue (false) );

  // Configure PhySimWifiPhy
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerEnd", DoubleValue (1.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerStart", DoubleValue (1.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerLevels", UintegerValue (1) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxGain", DoubleValue (0.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::RxGain", DoubleValue (0.0) );

  // Set the correct Channel Estimator implementation
  Config::SetDefault ("ns3::PhySimWifiPhy::ChannelEstimator", StringValue ("ns3::PhySimSimpleChannelEstimator") );

  // Set noise floor of -99 dBm
  Config::SetDefault ("ns3::PhySimInterferenceHelper::NoiseFloor", DoubleValue (-99.0) );

  // Only allow hard decisions
  Config::SetDefault ("ns3::PhySimConvolutionalEncoder::SoftViterbiDecision", BooleanValue (false));
  Config::SetDefault ("ns3::PhySimOFDMSymbolCreator::SoftViterbiDecision", BooleanValue (false));

  // Add a channel that reverses polarity of symbols - the simple estimator should be able to correct this
  Ptr<PhySimWifiManualChannel> channel = CreateObject<PhySimWifiManualChannel> ();

  Ptr<PropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (delayModel);
  Ptr<PhySimReversePolarityPropagationLoss> lossModel = CreateObject<PhySimReversePolarityPropagationLoss> ();
  channel->SetPropagationLossModel (lossModel);

  PhySimWifiPhyHelper wifiPhy = PhySimWifiPhyHelper::Default ();
  wifiPhy.SetChannel (channel);

  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetStandard(WIFI_PHY_STANDARD_80211_10MHZ);
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue (wifiMode),
                                "NonUnicastMode", StringValue (wifiMode));

  NodeContainer nodes;
  nodes.Create (2);

  // Install WifiPhy and set up mobility
  wifi.Install (wifiPhy, wifiMac, nodes);

  // Since the simple estimator does not correct frequency offset set it to 0
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/TxCenterFrequencyTolerance", UintegerValue (0) );

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
  // Configure to only send 1 packet
  PacketSocketAddress socketTo0;
  socketTo0.SetAllDevices ();
  socketTo0.SetPhysicalAddress (Mac48Address::GetBroadcast ());
  socketTo0.SetProtocol (1);
  OnOffHelper onOff ("ns3::PacketSocketFactory", Address (socketTo0));
  onOff.SetAttribute ("PacketSize", UintegerValue (512));
  onOff.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (1.0)));
  onOff.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0.0)));
  // Set the data rate
  onOff.SetAttribute ("DataRate", DataRateValue (DataRate ("4kb/s")));

  // Configure start/stop times of the application/sink
  ApplicationContainer app;
  app = sink.Install (nodes.Get (1));
  app = onOff.Install (nodes.Get (0));
  app.Start (Seconds (0.0));
  app.Stop (Seconds (2.0));

  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/RxOk", MakeCallback (&PhySimWifiSimpleEstimatorTest::PhyRxOkTrace, this) );

  Simulator::Run ();
  Simulator::Destroy ();

  return true;
}

void
PhySimWifiSimpleEstimatorTest::PhyRxOkTrace (std::string context, Ptr<const Packet> p, Ptr<const PhySimWifiPhyTag> tag)
{
  m_success = true;
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimReversePolarityPropagationLoss);

TypeId
PhySimReversePolarityPropagationLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimReversePolarityPropagationLoss")
    .SetParent<PhySimPropagationLossModel> ()
    .AddConstructor<PhySimReversePolarityPropagationLoss> ();
  return tid;
}

PhySimReversePolarityPropagationLoss::PhySimReversePolarityPropagationLoss ()
  : PhySimPropagationLossModel ()
{
}

PhySimReversePolarityPropagationLoss::~PhySimReversePolarityPropagationLoss ()
{
}

void
PhySimReversePolarityPropagationLoss::DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag,
                                                     Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  itpp::cvec input = tag->GetRxedSamples ();
  itpp::cvec output = (-1) * input;
  tag->SetPathLoss (0);
  tag->SetRxSamples (output);
}

// ------------------------------------------------------------------------- //
} // namespace ns3
