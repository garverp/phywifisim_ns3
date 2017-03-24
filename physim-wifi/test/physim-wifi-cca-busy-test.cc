/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Jens Mittag, Karlsruhe Institute of Technology
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
 * Authors: Jens Mittag <jens.mittag@kit.edu>
 */

#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/physim-wifi-phy.h"
#include "ns3/physim-wifi-channel.h"
#include "ns3/physim-propagation-loss-model.h"
#include "ns3/physim-wifi-helper.h"
#include "ns3/wifi-helper.h"
#include "ns3/nqos-wifi-mac-helper.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/mobility-helper.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/on-off-helper.h"
#include "physim-wifi-cca-busy-test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhySimWifiCcaBusyTest");

PhySimWifiCcaSimulationExperiment::PhySimWifiCcaSimulationExperiment ()
  : m_failed (false),
    m_difs (NanoSeconds (34000))
{
  // register all possible physical layer states
  m_checker.RegisterState (PhySimWifiPhy::SYNCING, "SYNC");
  m_checker.RegisterState (PhySimWifiPhy::RX, "RX");
  m_checker.RegisterState (PhySimWifiPhy::TX, "TX");
  m_checker.RegisterState (PhySimWifiPhy::IDLE, "IDLE", true);
  m_checker.RegisterState (PhySimWifiPhy::CCA_BUSY, "CCA_BUSY");

  // register all allowed state transitions
  m_checker.RegisterStateTransition (PhySimWifiPhy::IDLE, PhySimWifiPhy::TX);
  m_checker.RegisterStateTransition (PhySimWifiPhy::TX, PhySimWifiPhy::IDLE);
  m_checker.RegisterStateTransition (PhySimWifiPhy::TX, PhySimWifiPhy::CCA_BUSY);
  m_checker.RegisterStateTransition (PhySimWifiPhy::CCA_BUSY, PhySimWifiPhy::IDLE);
  m_checker.RegisterStateTransition (PhySimWifiPhy::IDLE, PhySimWifiPhy::CCA_BUSY);
  m_checker.RegisterStateTransition (PhySimWifiPhy::IDLE, PhySimWifiPhy::SYNCING);
  m_checker.RegisterStateTransition (PhySimWifiPhy::SYNCING, PhySimWifiPhy::IDLE);
  m_checker.RegisterStateTransition (PhySimWifiPhy::SYNCING, PhySimWifiPhy::RX);
  m_checker.RegisterStateTransition (PhySimWifiPhy::SYNCING, PhySimWifiPhy::TX);
  m_checker.RegisterStateTransition (PhySimWifiPhy::RX, PhySimWifiPhy::CCA_BUSY);
  m_checker.RegisterStateTransition (PhySimWifiPhy::RX, PhySimWifiPhy::IDLE);
  m_checker.RegisterStateTransition (PhySimWifiPhy::SYNCING, PhySimWifiPhy::CCA_BUSY);
  m_checker.RegisterStateTransition (PhySimWifiPhy::CCA_BUSY, PhySimWifiPhy::SYNCING);

}

PhySimWifiCcaSimulationExperiment::~PhySimWifiCcaSimulationExperiment ()
{
}

bool
PhySimWifiCcaSimulationExperiment::DoRun (enum TestId number)
{
  // reset the state checker
  m_checker.Reset ();
  m_failed = false;

  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::PhySimScrambler::UseFixedScrambler", BooleanValue (false) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerEnd", DoubleValue (1.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerStart", DoubleValue (1.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerLevels", UintegerValue (1) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxGain", DoubleValue (0.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::RxGain", DoubleValue (0.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::ChannelEstimator", StringValue ("ns3::PhySimChannelFrequencyOffsetEstimator") );
  Config::SetDefault ("ns3::PhySimInterferenceHelper::NoiseFloor", DoubleValue (-99.0) );

  SeedManager::SetRun (1);

  // Configure the wireless channel
  Ptr<PhySimWifiManualChannel> channel = CreateObject<PhySimWifiManualChannel> ();
  Ptr<PropagationDelayModel> defaultDelay = CreateObject<ConstantSpeedPropagationDelayModel> ();
  channel->SetPropagationDelayModel (defaultDelay);
  Ptr<PhySimConstantPropagationLoss> defaultLossModel = CreateObject<PhySimConstantPropagationLoss> ();
  channel->SetPropagationLossModel (defaultLossModel);
  // Special link which is used later for specific test runs
  Ptr<PhySimConstantPropagationLoss> nodes2and0LossModel = CreateObject<PhySimConstantPropagationLoss> ();
  Ptr<PhySimConstantPropagationLoss> nodes2and1LossModel = CreateObject<PhySimConstantPropagationLoss> ();

  PhySimWifiPhyHelper wifiPhy = PhySimWifiPhyHelper::Default ();
  wifiPhy.SetChannel (channel);

  WifiHelper wifi = WifiHelper::Default ();
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("OfdmRate6Mbps"),
                                "NonUnicastMode", StringValue ("OfdmRate6Mbps"));

  NodeContainer nodes;
  nodes.Create (3);

  // we will have at most 3 nodes
  std::vector<Ptr<Node> >::const_iterator it;
  uint32_t i = 0;
  for (it = nodes.Begin (); it != nodes.End (); it++)
    {
      Ptr<Node> n = *it;
      m_checker.RegisterNodeId (n->GetId ());
      m_nodes[i] = n->GetId ();
      i++;
    }

  // Install WifiPhy and set up mobility
  wifi.Install (wifiPhy, wifiMac, nodes);


  // We have 3 nodes in a row with 0m spacing in between
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (1.0, 0.0, 0.0));
  positionAlloc->Add (Vector (1.0, 0.0, 0.0));
  positionAlloc->Add (Vector (1.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  // Add packet socket handlers
  PacketSocketHelper packetSocket;
  packetSocket.Install (nodes);

  // Configure packet socket: use broadcasts
  PacketSocketAddress socket;
  socket.SetAllDevices ();
  socket.SetPhysicalAddress (Mac48Address::GetBroadcast ());
  socket.SetProtocol (1);

  // Install PacketSink on Node 0
  PacketSinkHelper sink ("ns3::PacketSocketFactory", Address (socket));

  // Create broadcast packet socket on Node 0
  PacketSocketAddress socket0To1;
  socket0To1.SetAllDevices ();
  socket0To1.SetPhysicalAddress (Mac48Address::GetBroadcast ());
  socket0To1.SetProtocol (1);

  // Create broadcast packet socket on Node 1
  PacketSocketAddress socket1ToAll;
  socket1ToAll.SetAllDevices ();
  socket1ToAll.SetPhysicalAddress (Mac48Address::GetBroadcast ());
  socket1ToAll.SetProtocol (1);

  // Create broadcast packet socket on Node 2
  PacketSocketAddress socket2To1;
  socket2To1.SetAllDevices ();
  socket2To1.SetPhysicalAddress (Mac48Address::GetBroadcast ());
  socket2To1.SetProtocol (1);

  // Create OnOffApplication on Node 0
  OnOffHelper onOff0To1 ("ns3::PacketSocketFactory", Address (socket0To1));
  onOff0To1.SetAttribute ("PacketSize", UintegerValue (20));
  onOff0To1.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (1.0)));
  onOff0To1.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0.0)));
  onOff0To1.SetAttribute ("DataRate", DataRateValue (DataRate ("20B/s")) );

  // Create OnOffApplication on Node 1
  OnOffHelper onOff1ToAll ("ns3::PacketSocketFactory", Address (socket1ToAll));
  onOff1ToAll.SetAttribute ("PacketSize", UintegerValue (20));
  onOff1ToAll.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (1.0)));
  onOff1ToAll.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0.0)));
  onOff1ToAll.SetAttribute ("DataRate", DataRateValue (DataRate ("20B/s")) );

  // Create OnOffApplication on Node 2
  OnOffHelper onOff2To1 ("ns3::PacketSocketFactory", Address (socket2To1));
  onOff2To1.SetAttribute ("PacketSize", UintegerValue (20));
  onOff2To1.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (1.0)));
  onOff2To1.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0.0)));
  onOff2To1.SetAttribute ("DataRate", DataRateValue (DataRate ("20B/s")) );

  // Install OnOffApplication on Node 0
  ApplicationContainer app0To1 = onOff0To1.Install (nodes.Get (0));

  // Install OnOffApplication on Node 2
  ApplicationContainer app2To1 = onOff2To1.Install (nodes.Get (2));

  // For later use (see blow)
  ApplicationContainer app1ToAll;
  std::ostringstream oss;

  switch (number)
    {
    case PhySimWifiCcaSimulationExperiment::TEST_1A:

      // Set CCA busy threshold to -95.0 dBm and
      //     pathloss to 70.0 dB, such that each signal has a higher strength
      Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/CcaModelThreshold", DoubleValue (-95.0));
      defaultLossModel->SetPathLoss (70.0);

      // Tell the checker to prohibit TX during the CCA_BUSY period for node 2
      m_checker.RegisterProhibitedStatePeriod (m_nodes[2], PhySimWifiPhy::TX, NanoSeconds (1000000000), NanoSeconds (1000100000) + m_difs);

      // Start the second transmission during IDLE state (which means we have a short off phase first)
      // which means we start prior to the signal header
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000012)));
      app2To1.Start (Seconds (0));
      app0To1.Start (Seconds (0));

      break;

    case PhySimWifiCcaSimulationExperiment::TEST_1B:

      // Set CCA busy threshold to -50.0 dBm and
      //     pathloss to 70.0 dB, such that each signal has a lower strength
      Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/CcaModelThreshold", DoubleValue (-50.0));
      defaultLossModel->SetPathLoss (70.0);

      // Tell the checker to prohibit CCA_BUSY during the TX period of node 2
      m_checker.RegisterProhibitedStatePeriod (m_nodes[2], PhySimWifiPhy::CCA_BUSY, NanoSeconds (1000000000), NanoSeconds (1000100000));

      // Start the second transmission during IDLE state (which means we have a short off phase first)
      // which means we start prior to the signal header
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000012)));
      app2To1.Start (Seconds (0));
      app0To1.Start (Seconds (0));

      break;

    case PhySimWifiCcaSimulationExperiment::TEST_2A:

      // Set CCA busy threshold to -95.0 dBm and
      //     pathloss to 70.0 dB, such that each signal has a higher strength
      Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/CcaModelThreshold", DoubleValue (-95.0));
      defaultLossModel->SetPathLoss (70.0);

      // Tell the checker to prohibit TX during the CCA_BUSY period for node 2
      m_checker.RegisterProhibitedStatePeriod (m_nodes[2], PhySimWifiPhy::TX, NanoSeconds (1000000000), NanoSeconds (1000100000) + m_difs);

      // Start the second transmission during SYNC state
      // which means we start after EndPreamble() and prior to EndHeader()
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000018)));
      app0To1.Start (Seconds (0));
      app2To1.Start (Seconds (0));

      break;

    case PhySimWifiCcaSimulationExperiment::TEST_2B:

      // Set CCA busy threshold to -95.0 dBm and
      //     pathloss to 70.0 dB, such that each signal has a higher strength
      Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/CcaModelThreshold", DoubleValue (-50.0));
      defaultLossModel->SetPathLoss (70.0);

      // Tell the checker to prohibit CCA_BUSY during the TX period of node 2
      m_checker.RegisterProhibitedStatePeriod (m_nodes[2], PhySimWifiPhy::CCA_BUSY, NanoSeconds (1000000000), NanoSeconds (1000100000));

      // Start the second transmission during SYNC state
      // which means we start after EndPreamble() and prior to EndHeader()
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000018)));
      app0To1.Start (Seconds (0));
      app2To1.Start (Seconds (0));

      break;

    case PhySimWifiCcaSimulationExperiment::TEST_3A:

      // Set CCA busy threshold to -95.0 dBm and
      //     pathloss to 70.0 dB, such that each signal has a higher strength
      Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/CcaModelThreshold", DoubleValue (-95.0));
      defaultLossModel->SetPathLoss (70.0);

      // For this case, we want to have NO possible communication between nodes 0 <--> 2
      // such that we can create a CCA busy situation at node 1 and inject another incoming frame
      nodes2and0LossModel->SetPathLoss (190.0);
      channel->SetExplicitPropagationLossModel (m_nodes[0], m_nodes[2], nodes2and0LossModel);
      channel->SetExplicitPropagationLossModel (m_nodes[2], m_nodes[0], nodes2and0LossModel);
      // Make sure that the interfering transmission of Node 2 is not too strong and destroying
      // the successful reception at Node 1
      nodes2and1LossModel->SetPathLoss (85.0);
      channel->SetExplicitPropagationLossModel (m_nodes[2], m_nodes[1], nodes2and1LossModel);

      // Tell the checker to prohibit TX during the RX and CCA_BUSY period of Node 1
      m_checker.RegisterProhibitedStatePeriod (m_nodes[1], PhySimWifiPhy::TX, NanoSeconds (1000100000), NanoSeconds (1000100000) + m_difs);

      // Start somewhere in the reception process with interfering and
      // stop interfering before the end of the first frame
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000008)));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (4));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("4B/s")));
      app0To1.Start (Seconds (0));
      app2To1.Start (Seconds (0));

      // Also start a transmission process by node 1 itself
      app1ToAll = onOff1ToAll.Install (nodes.Get (1));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000020)));
      app1ToAll.Start (Seconds (0));

      break;

    case PhySimWifiCcaSimulationExperiment::TEST_3B:

      // Set CCA busy threshold to -95.0 dBm and
      //     pathloss to 70.0 dB, such that each signal has a higher strength
      Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/CcaModelThreshold", DoubleValue (-95.0));
      defaultLossModel->SetPathLoss (70.0);

      // For these two cases, we want to have NO possible communication between nodes 0 <--> 2
      // such that we can create a CCA busy situation at node 1 and inject another incoming frame
      nodes2and0LossModel->SetPathLoss (190.0);
      channel->SetExplicitPropagationLossModel (m_nodes[0], m_nodes[2], nodes2and0LossModel);
      channel->SetExplicitPropagationLossModel (m_nodes[2], m_nodes[0], nodes2and0LossModel);
      // Make sure that the interfering transmission of Node 2 is not too strong and destroying
      // the successful reception at Node 1
      nodes2and1LossModel->SetPathLoss (85.0);
      channel->SetExplicitPropagationLossModel (m_nodes[2], m_nodes[1], nodes2and1LossModel);

      // Tell the checker to prohibit TX during the RX and CCA_BUSY period of Node 1
      m_checker.RegisterProhibitedStatePeriod (m_nodes[1], PhySimWifiPhy::TX, NanoSeconds (1000100000), NanoSeconds (1000136000) + m_difs);

      // Start somewhere in the reception process with interfering and
      // stop interfering after the end of the first frame
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000008)));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (40));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("40B/s")));
      app0To1.Start (Seconds (0));
      app2To1.Start (Seconds (0));

      // Also start a transmission process by node 1 itself
      app1ToAll = onOff1ToAll.Install (nodes.Get (1));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000020)));
      app1ToAll.Start (Seconds (0));

      break;

    case PhySimWifiCcaSimulationExperiment::TEST_4A:

      // Set CCA busy threshold to -95.0 dBm and
      //     pathloss to 70.0 dB, such that each signal has a higher strength
      Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/CcaModelThreshold", DoubleValue (-95.0));
      defaultLossModel->SetPathLoss (70.0);

      // For these two cases, we want to have NO possible communication from node 1 --> 0
      nodes2and0LossModel->SetPathLoss (190.0);
      channel->SetExplicitPropagationLossModel (m_nodes[1], m_nodes[0], nodes2and0LossModel);

      // Let node 1 start its transmissions first, using a higher rate than PHY can handle
      // such that we have already a packet in the queue before the previos one is finished
      app1ToAll = onOff1ToAll.Install (nodes.Get (1));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (1000));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.0)));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("24MBps")));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/MaxBytes";
      Config::Set (oss.str (), UintegerValue (2000));
      app1ToAll.Start (Seconds (1.0));

      // Node 2 shall communicate nothing
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (2.0)));

      // Then start a shorter transmission by node 0
      oss.str ("");
      oss << "/NodeList/" << m_nodes[0] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (500));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[0] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("500B/s")));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[0] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000100)));
      app0To1.Start (Seconds (0));

      // Tell the checker to prohibit any CCA_BUSY for Node 1
      m_checker.RegisterProhibitedStatePeriod (m_nodes[1], PhySimWifiPhy::CCA_BUSY, NanoSeconds (0), NanoSeconds (1002963666));

      break;

    case PhySimWifiCcaSimulationExperiment::TEST_4B:

      // Set CCA busy threshold to -95.0 dBm and
      //     pathloss to 70.0 dB, such that each signal has a higher strength
      Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/CcaModelThreshold", DoubleValue (-95.0));
      defaultLossModel->SetPathLoss (70.0);

      // For these two cases, we want to have NO possible communication from node 1 --> 0
      nodes2and0LossModel->SetPathLoss (190.0);
      channel->SetExplicitPropagationLossModel (m_nodes[1], m_nodes[0], nodes2and0LossModel);

      // Let node 1 start its transmissions first, using a higher rate than PHY can handle
      // such that we have already a packet in the queue before the previos one is finished
      app1ToAll = onOff1ToAll.Install (nodes.Get (1));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (1000));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.0)));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("24MBps")));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/MaxBytes";
      Config::Set (oss.str (), UintegerValue (2000));
      app1ToAll.Start (Seconds (1.0));

      // Node 2 shall communicate nothing
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (2.0)));

      // Then start a shorter transmission by node 0
      oss.str ("");
      oss << "/NodeList/" << m_nodes[0] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (1000));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[0] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("1000B/s")));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[0] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000100)));
      app0To1.Start (Seconds (0));

      // Tell the checker to prohibit any TX for Node 1 during the transmission by Node 0
      m_checker.RegisterProhibitedStatePeriod (m_nodes[1], PhySimWifiPhy::TX, NanoSeconds (1001449666), NanoSeconds (1001507999));

      break;

    case PhySimWifiCcaSimulationExperiment::TEST_5A:

      // Set CCA busy threshold to -95.0 dBm and
      //     pathloss to 70.0 dB, such that each signal has a higher strength
      Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/CcaModelThreshold", DoubleValue (-95.0));
      defaultLossModel->SetPathLoss (70.0);

      // For this cases, we want to have NO possible communication between nodes 0 <--> 2
      // such that we can create a CCA busy situation at node 1 and inject another incoming frame
      nodes2and0LossModel->SetPathLoss (190.0);
      channel->SetExplicitPropagationLossModel (m_nodes[0], m_nodes[2], nodes2and0LossModel);
      channel->SetExplicitPropagationLossModel (m_nodes[2], m_nodes[0], nodes2and0LossModel);
      // Make sure that the interfering transmission of Node 2 is not too strong and destroying
      // the successful reception at Node 1
      nodes2and1LossModel->SetPathLoss (85.0);
      channel->SetExplicitPropagationLossModel (m_nodes[2], m_nodes[1], nodes2and1LossModel);

      // Node 0 shall send a short packet of 40 bytes and start sending immediately
      oss.str ("");
      oss << "/NodeList/" << m_nodes[0] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (40));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[0] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("40B/s")));
      app0To1.Start (Seconds (0));

      // Node 2 will start sending during the reception period (RX state) of Node 1 with a smaller packet
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (4));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("4B/s")));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000022)));
      app2To1.Start (Seconds (0));

      // Also start a transmission process by Node 1 itself to check that it does send AFTER the transmission
      // of Node 0 has finished
      app1ToAll = onOff1ToAll.Install (nodes.Get (1));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000024)));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (40));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("40B/s")));
      app1ToAll.Start (Seconds (0));

      // Tell the checker to prohibit any TX attempts during the transmission by Node 0 plus an
      // additional DIFS period
      m_checker.RegisterProhibitedStatePeriod (m_nodes[1], PhySimWifiPhy::TX, NanoSeconds (1000000000), NanoSeconds (1000128000) + m_difs);

      break;

    case PhySimWifiCcaSimulationExperiment::TEST_5B:

      // Set CCA busy threshold to -95.0 dBm and
      //     pathloss to 70.0 dB, such that each signal has a higher strength
      Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/CcaModelThreshold", DoubleValue (-95.0));
      defaultLossModel->SetPathLoss (70.0);

      // For this cases, we want to have NO possible communication between nodes 0 <--> 2
      // such that we can create a CCA busy situation at node 1 and inject another incoming frame
      nodes2and0LossModel->SetPathLoss (190.0);
      channel->SetExplicitPropagationLossModel (m_nodes[0], m_nodes[2], nodes2and0LossModel);
      channel->SetExplicitPropagationLossModel (m_nodes[2], m_nodes[0], nodes2and0LossModel);
      // Make sure that the interfering transmission of Node 2 is not too strong and destroying
      // the successful reception at Node 1
      nodes2and1LossModel->SetPathLoss (85.0);
      channel->SetExplicitPropagationLossModel (m_nodes[2], m_nodes[1], nodes2and1LossModel);

      // Node 0 shall send a short packet of 40 bytes and start sending immediately
      oss.str ("");
      oss << "/NodeList/" << m_nodes[0] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (40));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[0] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("40B/s")));
      app0To1.Start (Seconds (0));

      // Node 2 will start sending during the reception period (RX state) of Node 1 with a greater packet
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (80));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("80B/s")));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[2] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000022)));
      app2To1.Start (Seconds (0));

      // Also start a transmission process by Node 1 itself to check that it does send AFTER the transmission
      // of Node 0 has finished
      app1ToAll = onOff1ToAll.Install (nodes.Get (1));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/OffTime";
      Config::Set (oss.str (), RandomVariableValue (ConstantVariable (0.000024)));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/PacketSize";
      Config::Set (oss.str (), UintegerValue (40));
      oss.str ("");
      oss << "/NodeList/" << m_nodes[1] << "/ApplicationList/*/$ns3::OnOffApplication/DataRate";
      Config::Set (oss.str (), DataRateValue (DataRate ("40B/s")));
      app1ToAll.Start (Seconds (0));

      // Tell the checker to prohibit any TX attempts during the transmission by Node 0 and Node 2 plus an
      // additional DIFS period
      m_checker.RegisterProhibitedStatePeriod (m_nodes[1], PhySimWifiPhy::TX, NanoSeconds (1000000000), NanoSeconds (1000201999) + m_difs);

      break;
    }

  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/State/State", MakeCallback (&PhySimWifiCcaSimulationExperiment::StateLogger, this) );

  Simulator::Stop (Seconds (2.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return m_failed;
}

void
PhySimWifiCcaSimulationExperiment::StateLogger (std::string context, Ptr<NetDevice> device, Time start, Time duration, enum PhySimWifiPhy::State state)
{
  uint32_t node = device->GetNode ()->GetId ();
  if (start > Seconds (0))
    {
      m_failed = (m_failed || !m_checker.PerformStateTransition (node, state));
    }
}


PhySimWifiCcaBusyTest::PhySimWifiCcaBusyTest ()
  : TestCase ("PhySim WiFi CCA busy mechanism test case")
{
}

PhySimWifiCcaBusyTest::~PhySimWifiCcaBusyTest ()
{
}

void
PhySimWifiCcaBusyTest::DoRun (void)
{
  std::vector<PhySimWifiCcaSimulationExperiment::Test> tests;
  PhySimWifiCcaSimulationExperiment::Test test1a = {
    PhySimWifiCcaSimulationExperiment::TEST_1A,
    "CCA_BUSY test number 1a (is CCA busy handled correctly if we are in IDLE state, and new frame is stronger than CcaThreshold?)."
  };
  PhySimWifiCcaSimulationExperiment::Test test1b = {
    PhySimWifiCcaSimulationExperiment::TEST_1B,
    "CCA_BUSY test number 1b (is CCA busy handled correctly if we are in IDLE state, and new frame is weaker than CcaThreshold?)."
  };
  PhySimWifiCcaSimulationExperiment::Test test2a = {
    PhySimWifiCcaSimulationExperiment::TEST_2A,
    "CCA_BUSY test number 2a (is CCA busy handled correctly if we are in SYNC state, and new frame is stronger than CcaThreshold?)."
  };
  PhySimWifiCcaSimulationExperiment::Test test2b = {
    PhySimWifiCcaSimulationExperiment::TEST_2B,
    "CCA_BUSY test number 2b (is CCA busy handled correctly if we are in SYNC state, and new frame is weaker than CcaThreshold?)."
  };
  PhySimWifiCcaSimulationExperiment::Test test3a = {
    PhySimWifiCcaSimulationExperiment::TEST_3A,
    "CCA_BUSY test number 3a (is CCA busy handled correctly if we are in CCA_BUSY state, and a new frame arrives which finishes earlier than the previous one?)."
  };
  PhySimWifiCcaSimulationExperiment::Test test3b = {
    PhySimWifiCcaSimulationExperiment::TEST_3B,
    "CCA_BUSY test number 3b (is CCA busy handled correctly if we are in CCA_BUSY state, and a new frame arrives which finishes later than the previous one?)."
  };
  PhySimWifiCcaSimulationExperiment::Test test4a = {
    PhySimWifiCcaSimulationExperiment::TEST_4A,
    "CCA_BUSY test number 4a (is CCA busy handled correctly if we are in TX state, and a new frame arrives which finishes earlier than the own transmission?)."
  };
  PhySimWifiCcaSimulationExperiment::Test test4b = {
    PhySimWifiCcaSimulationExperiment::TEST_4B,
    "CCA_BUSY test number 4b (is CCA busy handled correctly if we are in TX state, and a new frame arrives which finishes later than the own transmission?)."
  };
  PhySimWifiCcaSimulationExperiment::Test test5a = {
    PhySimWifiCcaSimulationExperiment::TEST_5A,
    "CCA_BUSY test number 5a (is CCA busy is handled correctly if we are already in RX state, and a new frame arrives which finishes earlier than the previous one?)."
  };
  PhySimWifiCcaSimulationExperiment::Test test5b = {
    PhySimWifiCcaSimulationExperiment::TEST_5B,
    "CCA_BUSY test number 5b (is CCA busy is handled correctly if we are already in RX state, and a new frame arrives which finishes later than the previous one?)."
  };
  tests.push_back (test1a);
  tests.push_back (test1b);
  tests.push_back (test2a);
  tests.push_back (test2b);
  tests.push_back (test3a);
  tests.push_back (test3b);
  tests.push_back (test4a);
  tests.push_back (test4b);
  tests.push_back (test5a);
  tests.push_back (test5b);

  std::vector<PhySimWifiCcaSimulationExperiment::Test>::iterator it;
  for (it = tests.begin (); it != tests.end (); it++)
    {
      PhySimWifiCcaSimulationExperiment::Test t = *it;
      PhySimWifiCcaSimulationExperiment e;
      if (e.DoRun (t.id))
        {
          NS_TEST_EXPECT_MSG_EQ ( false, true, t.desc);
          NS_LOG_INFO ("FAIL: " << t.desc);
        }
      else
        {
          NS_LOG_INFO ("PASS: " << t.desc);
        }
    }
}
