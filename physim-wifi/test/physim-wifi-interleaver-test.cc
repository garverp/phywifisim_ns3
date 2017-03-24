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

#include <itpp/itcomm.h>
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/object-factory.h"
#include "ns3/physim-helper.h"
#include "ns3/random-variable.h"
#include "physim-wifi-interleaver-test.h"
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhySimWifiInterleaverTest");

PhySimWifiInterleaverTest::PhySimWifiInterleaverTest ()
  : TestCase ("PhySim WiFi interleaver test case")
{
}

PhySimWifiInterleaverTest::~PhySimWifiInterleaverTest ()
{
}

void
PhySimWifiInterleaverTest::DoRun (void)
{

  // Provide seed for predictable results
  SeedManager::SetSeed (1);

  bool success;
  std::string modes[] = { "OfdmRate3MbpsBW10MHz","OfdmRate4_5MbpsBW10MHz","OfdmRate6MbpsBW10MHz","OfdmRate9MbpsBW10MHz","OfdmRate12MbpsBW10MHz","OfdmRate18MbpsBW10MHz","OfdmRate24MbpsBW10MHz","OfdmRate27MbpsBW10MHz"};
  for (int i = 0; i < 8; ++i)
    {
      success = RunSingle (modes[i]);
      if (!success)
        {
          NS_LOG_DEBUG ("FAIL: Interleaver test for " << modes[i]);
        }
      else
        {
          NS_LOG_DEBUG ("PASS: Interleaver test for " << modes[i]);
        }
      NS_TEST_EXPECT_MSG_EQ ( success, true, "Interlever test failed for " << modes[i] << " : Input bits before interleaving and output bits after de-interleavind do not match");
    }
}

bool
PhySimWifiInterleaverTest::RunSingle (std::string wifiMode)
{
  WifiMode mode = wifiMode;
  uint32_t m_NCBPS = PhySimHelper::GetNCBPS (mode);
  // create appropriately sized random bits
  itpp::bvec bits = itpp::randb (m_NCBPS);
  m_interleaver.SetWifiMode (mode);
  itpp::bvec interleavedBits = m_interleaver.InterleaveBlock (bits);
  itpp::bvec deinterleavedBits = itpp::to_bvec (m_interleaver.DeinterleaveBlock (itpp::to_vec (interleavedBits)));
  return (deinterleavedBits == bits);
}
