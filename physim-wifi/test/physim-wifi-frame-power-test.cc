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

#include "ns3/node.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-preamble.h"
#include "ns3/wifi-phy.h"
#include "ns3/packet.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/object-factory.h"
#include "ns3/physim-wifi-phy.h"
#include "ns3/physim-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "physim-wifi-frame-power-test.h"
#include <itpp/itcomm.h>
#include <itpp/stat/misc_stat.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhySimWifiFramePowerTest");

PhySimWifiFramePowerTest::PhySimWifiFramePowerTest ()
  : TestCase ("PhySim WiFi frame power test case")
{
}

PhySimWifiFramePowerTest::~PhySimWifiFramePowerTest ()
{
}

void
PhySimWifiFramePowerTest::DoRun (void)
{
  // Provide known seed for predictable results
  SeedManager::SetSeed (1);

  // Also take care of the transformation from IT++ to IEEE notation
  Config::SetDefault ("ns3::PhySimWifiPhy::NormalizeOFDMSymbols", BooleanValue (false) );
  Config::SetDefault ("ns3::PhySimOFDMSymbolCreator::IEEECompliantMode", BooleanValue (false) );
  Config::SetDefault ("ns3::PhySimSignalDetector::IEEECompliantMode", BooleanValue (false) );

  // Use a random scrambler (the default)
  Config::SetDefault ("ns3::PhySimScrambler::UseFixedScrambler", BooleanValue (false) );

  std::string modes[] = { "OfdmRate3MbpsBW10MHz","OfdmRate4_5MbpsBW10MHz","OfdmRate6MbpsBW10MHz","OfdmRate9MbpsBW10MHz","OfdmRate12MbpsBW10MHz","OfdmRate18MbpsBW10MHz","OfdmRate24MbpsBW10MHz","OfdmRate27MbpsBW10MHz"};
  for (int i = 0; i < 8; ++i)
    {
      RunSingle (modes[i]);
      if (!m_success)
        {
          NS_LOG_DEBUG ("FAIL: Frame power test failed for " << modes[i]);
        }
      else
        {
          NS_LOG_DEBUG ("PASS: Frame power test for " << modes[i]);
        }
      NS_TEST_EXPECT_MSG_EQ ( m_success, true, "Frame power test failed for " << modes[i] << " :  : frame was not successfully decoded");
    }
}

bool
PhySimWifiFramePowerTest::RunSingle (std::string wifiMode)
{
  m_success = true;
  PhySimWifiPhy::ClearCache ();
  PhySimWifiPhy::ResetRNG ();

  // Create a channel object
  Ptr<PhySimWifiChannel> channel = CreateObject<PhySimWifiUniformChannel> ();
  // Create a PHY object
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerEnd", DoubleValue (1.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerStart", DoubleValue (1.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerLevels", UintegerValue (1) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxGain", DoubleValue (0.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::RxGain", DoubleValue (0.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxCenterFrequencyTolerance", UintegerValue (0) );
  Ptr<PhySimWifiPhy> phy = CreateObject<PhySimWifiPhy> ();

  // Attach PHY object to channel
  phy->SetChannel (channel);

  // Connect trace source for Tx events
  phy->TraceConnectWithoutContext ("Tx", MakeCallback (&PhySimWifiFramePowerTest::PhyTxCallback, this));

  // Also create a net device object and a mobility object
  Ptr<Node> node = CreateObject<Node> ();
  Ptr<WifiNetDevice> device = CreateObject<WifiNetDevice> ();
  Ptr<MobilityModel> mobility = CreateObject<ConstantPositionMobilityModel> ();
  mobility->SetPosition (Vector (1.0, 0.0, 0.0));
  node->AggregateObject (mobility);
  phy->SetMobility (node);
  phy->SetDevice (device);

  // Create a packet object of 800 bits
  itpp::bvec bitSequence = itpp::randb (800);
  uint32_t bytes = bitSequence.size () / 8;
  uint8_t *payload = new uint8_t[100];
  for (uint32_t i = 0; i < bytes; i++)
    {
      itpp::bvec extract = bitSequence ((i * 8), (i * 8) + 8 - 1);
      payload[i] = itpp::bin2dec ( extract, false );
    }
  Ptr<const Packet> packet = Create<Packet> ( (const uint8_t*) payload, 100);
  WifiMode mode (wifiMode);

  // Send a packet over the PHY
  phy->SendPacket (packet, mode, WIFI_PREAMBLE_LONG, 1);

  bool success;
  const double tol = 0.005;
  double powerST = CalcPower (m_txSamples (0,159)); // power of short training sequences
  double powerLT = CalcPower (m_txSamples (160,319)); // power of long training sequences

  NS_TEST_EXPECT_MSG_EQ_TOL (powerLT, powerST, tol, "Power of ST != LT (" << powerST << " != " << powerLT << ") in test");
  success = !((powerLT > (powerST + tol)) || (powerLT < (powerST - tol)));                                                     \
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: The power of the short and long training symbols differ significantly");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: The power of the short and long training symbols are approximately the same");
    }

  // POWER of SIGNAL
  double powerSIGNAL = CalcPower (m_txSamples (320,399)); // power of SIGNAL
  NS_TEST_EXPECT_MSG_EQ_TOL (powerSIGNAL, powerST, tol, "Power of SIGNAL != ST (" << powerSIGNAL << " != " << powerLT << ") in test");
  success = !((powerSIGNAL) > (powerST) + (tol) || (powerSIGNAL) < (powerST) - (tol));                                                     \
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: The power of the SIGNAL and the short training symbols differ significantly");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: The power of the SIGNAL and short training symbols are approximately the same");
    }

  // The rest of the OFDM symbols
  for (int k = 0; k + 400 + 79 <= m_txSamples.size () - 1; k += 80)
    {
      double powerOFDM = CalcPower (m_txSamples (400 + k,400 + k + 79)); // power of OFDM symbol
      NS_TEST_EXPECT_MSG_EQ_TOL (powerOFDM, powerST, tol, "Power of  OFDM Symbol != ST (" << powerOFDM << " != " << powerST << ") in test");
      success = !((powerOFDM) > (powerST) + (tol) || (powerOFDM) < (powerST) - (tol));                                                     \
      if (!success)
        {
          NS_LOG_DEBUG ("FAIL: The power of the OFDM symbol " << (k / 80) + 1 << " and the short training symbols differ significantly");
        }
      else
        {
          NS_LOG_DEBUG ("PASS: The power of OFDM symbol " << (k / 80) + 1 << " and short training symbols are approximately the same");
        }
    }

  m_success = success;
  return true;
}

double PhySimWifiFramePowerTest::CalcPower (const itpp::cvec& input)
{
  double sum = 0;
  for (int i = 0; i < input.size (); ++i)
    {
      sum += std::norm (input (i));
    }
  return sum / input.size ();
}

void
PhySimWifiFramePowerTest::PhyTxCallback (Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag)
{
  m_txSamples = tag->GetTxedSamples ();
}
