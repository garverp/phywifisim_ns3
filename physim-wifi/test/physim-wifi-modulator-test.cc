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
#include "physim-wifi-modulator-test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhySimWifiModulatorTest");

PhySimWifiModulatorTest::PhySimWifiModulatorTest ()
  : TestCase ("PhySim WiFi modulator test case")
{
  CreateObject<PhySimChannelEstimator> ();
  m_symbolcreator = CreateObject<PhySimOFDMSymbolCreator> ();
  m_symbolcreator->SetChannelEstimator ("ns3::PhySimChannelEstimator");
}

PhySimWifiModulatorTest::~PhySimWifiModulatorTest ()
{
}

void
PhySimWifiModulatorTest::DoRun (void)
{
  // Provide known seed for predictable results
  SeedManager::SetSeed (1);

  PhySimHelper::ModulationType modulation = PhySimHelper::BPSK;
  uint8_t  constellation = 2;
  uint16_t  blockSize = 48;
  std::string description = "BPSK";
  RunSingle (modulation, constellation, blockSize, description);

  modulation = PhySimHelper::QPSK;
  constellation = 4;
  blockSize = 96;
  description = "QPSK";
  RunSingle (modulation, constellation, blockSize, description);

  modulation = PhySimHelper::QAM16;
  constellation = 16;
  blockSize = 192;
  description = "QAM-16";
  RunSingle (modulation, constellation, blockSize, description);

  modulation = PhySimHelper::QAM64;
  constellation = 64;
  blockSize = 288;
  description = "QAM-64";
  RunSingle (modulation, constellation, blockSize, description);
}

bool
PhySimWifiModulatorTest::RunSingle (PhySimHelper::ModulationType modulation,uint8_t constellation, uint16_t blockSize, std::string description)
{
  // create some random bits
  itpp::bvec bits = itpp::randb (blockSize);
  m_symbolcreator->SetModulationType (modulation,constellation);
  itpp::cvec modulatedBits = m_symbolcreator->Modulate (bits,0);
  itpp::bvec demodulatedBits = to_bvec (m_symbolcreator->DeModulate (modulatedBits,0));
  bool success = (demodulatedBits == bits);
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: Modulation test for " << description);
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Modulation test for " << description);
    }
  NS_TEST_EXPECT_MSG_EQ (success, true, "Modulation test failed for " << description);
  return success;
}
