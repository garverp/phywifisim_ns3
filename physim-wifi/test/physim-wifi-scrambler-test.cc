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
#include "physim-wifi-scrambler-test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhySimWifiScramblerTest");

PhySimWifiScramblerTest::PhySimWifiScramblerTest ()
  : TestCase ("PhySim WiFi scrambler test case")
{
}

PhySimWifiScramblerTest::~PhySimWifiScramblerTest ()
{
}

void
PhySimWifiScramblerTest::DoRun (void)
{

  bool success;

  // Provide known seed for predictable results
  SeedManager::SetSeed (1);

  for (int i = 0; i < 3; ++i)
    {
      // create random 7-bit seed
      itpp::bvec seed = itpp::randb (7);
      success = RunSingle (seed);
      if (!success)
        {
          NS_LOG_DEBUG ("FAIL: Scrambler test for seed " << seed);
        }
      else
        {
          NS_LOG_DEBUG ("PASS: Scrambler test for seed " << seed);
        }
      NS_TEST_EXPECT_MSG_EQ ( success, true, "Convolutional test failed for seed" << seed << " : Input bits before encoding and output bits after de-coding do not match");
    }

  success = RunAnnexG ();
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: Scrambler test for input as in standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Scrambler test for input as in standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "Scrambler test failed as scrambled bits do not match standard annex G");
}

bool
PhySimWifiScramblerTest::RunSingle (itpp::bvec seed)
{
  // create some random bits
  itpp::bvec bits = itpp::randb (144);
  itpp::bvec scrambledBits = m_scrambler.Scramble (bits,seed);
  itpp::bvec reverseSeed = itpp::reverse (seed);
  itpp::bvec deScrambledBitsAndPadding = m_scrambler.DeScramble (scrambledBits,reverseSeed);
  // remove the 7 pad bits at the beginning
  itpp::bvec deScrambledBits = deScrambledBitsAndPadding.get (7,deScrambledBitsAndPadding.size () - 1);
  return (deScrambledBits == bits);
}

// Verify encoder against the example in the standard annex G
bool
PhySimWifiScramblerTest::RunAnnexG (void)
{
  // Table G.13 -- First 144 DATA bits
  const itpp::bvec
  first144Bits =
    "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 1 0 0 0 0 1 0 1 1 0 0 1 1 1 1 1 0 1 1 0 0 0 1 1 0 0 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1 1 0 1 0 1 1 1 0 0 0 0 0 0 0 0 0 1 1 1 1 0 0 1 0 0 0 1 1 1 1";
  // Table G.16 -- First 144 Bits After scrambling
  const itpp::bvec
  first144BitsAfterScrambling =
    "0 1 1 0 1 1 0 0 0 0 0 1 1 0 0 1 1 0 0 0 1 0 0 1 1 0 0 0 1 1 1 1 0 1 1 0 1 0 0 0 0 0 1 0 0 0 0 1 1 1 1 1 0 1 0 0 1 0 1 0 0 1 0 1 0 1 1 0 0 0 0 1 0 1 0 0 1 1 1 1 1 1 0 1 0 1 1 1 1 0 1 0 1 1 1 0 0 0 1 0 0 1 0 0 0 0 0 0 1 1 0 0 1 1 1 1 0 0 1 1 0 0 1 1 1 0 1 0 1 1 1 0 0 1 0 0 1 0 1 1 1 1 0 0";
  const itpp::bvec
  annexSeed = "1 0 1 1 1 0 1";
  itpp::bvec scrambledBits = m_scrambler.Scramble (first144Bits, annexSeed);
  return (scrambledBits == first144BitsAfterScrambling);
}
