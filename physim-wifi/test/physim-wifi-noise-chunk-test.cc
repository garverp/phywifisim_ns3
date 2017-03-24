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
 * Authors: Jens Mittag <jens.mittag@kit.edu>
 */

#include "ns3/physim-interference-helper.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/random-variable.h"
#include "physim-wifi-noise-chunk-test.h"
#include <itpp/itcomm.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhySimWifiNoiseChunkTest");

PhySimWifiNoiseChunkTest::PhySimWifiNoiseChunkTest ()
  : TestCase ("PhySim WiFi interference manager test case")
{
}

PhySimWifiNoiseChunkTest::~PhySimWifiNoiseChunkTest ()
{
}

void
PhySimWifiNoiseChunkTest::DoRun (void)
{
  // Provide known seed for predictable results
  SeedManager::SetSeed (1);

  Ptr<PhySimInterferenceHelper> m_interference = CreateObject<PhySimInterferenceHelper> ();
  itpp::cvec noise1, noise2, chunk1, chunk2, chunk3;

  // Test case 1: chunkStart < start AND chunkEnd < end AND chunkEnd >= start
  // chunk (100ns - 200ns)
  chunk1 = m_interference->GetBackgroundNoise (NanoSeconds (100), NanoSeconds (200));
  // noise (150ns - 550ns)
  noise1 = m_interference->GetBackgroundNoise (NanoSeconds (150), NanoSeconds (550));
  if (noise1.get (0) != chunk1.get (1))
    {
      NS_LOG_DEBUG ("FAIL: Case 1, partially overlapping noise requests are not matching in overlap area");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Case 1, partially overlapping noise requests are not matching in overlap area");
    }
  NS_TEST_EXPECT_MSG_EQ ( (noise1.get (0) == chunk1.get (1)), true, "Case 1: partially overlapping noise requests are not matching in overlap area");

  // Test case 2: chunkStart >= start AND chunkEnd <= end
  // chunk (1300ns - 1400ns)
  chunk1 = m_interference->GetBackgroundNoise (NanoSeconds (1300), NanoSeconds (1400));
  // noise (1150ns - 1550ns)
  noise1 = m_interference->GetBackgroundNoise (NanoSeconds (1150), NanoSeconds (1550));
  if (noise1.get (3,4) != chunk1)
    {
      NS_LOG_DEBUG ("FAIL: Case 2, completely overlapping noise requests are not matching in overlap area");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Case 2, completely overlapping noise requests are not matching in overlap area");
    }
  NS_TEST_EXPECT_MSG_EQ ( (noise1.get (3,4) == chunk1), true, "Case 2: completely overlapping noise requests are not matching in overlap area");

  // Test case 3: chunkStart >= start AND chunkStart <= end AND chunkEnd > end
  // chunk (2500ns - 2600ns)
  chunk1 = m_interference->GetBackgroundNoise (NanoSeconds (2500), NanoSeconds (2600));
  // noise (2150ns - 2550ns)
  noise1 = m_interference->GetBackgroundNoise (NanoSeconds (2150), NanoSeconds (2550));
  if (noise1.get (7) != chunk1.get (0))
    {
      NS_LOG_DEBUG ("FAIL: Case 3, partially overlapping noise requests are not matching in overlap area");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Case 3, partially overlapping noise requests are not matching in overlap area");
    }
  NS_TEST_EXPECT_MSG_EQ ( (noise1.get (7) == chunk1.get (0)), true, "Case 3: partially overlapping noise requests are not matching in overlap area");

  // Test case 4: chunkStart <= start && chunkEnd >= end
  // chunk (3100ns - 3600ns)
  chunk1 = m_interference->GetBackgroundNoise (NanoSeconds (3100), NanoSeconds (3600));
  // noise (3150ns - 3550ns)
  noise1 = m_interference->GetBackgroundNoise (NanoSeconds (3150), NanoSeconds (3550));
  if (noise1 != chunk1.get (1,8))
    {
      NS_LOG_DEBUG ("FAIL: Case 4, completely overlapping noise requests are not matching in overlap area");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Case 4, completely overlapping noise requests are not matching in overlap area");
    }
  NS_TEST_EXPECT_MSG_EQ ( (noise1 == chunk1.get (1,8)), true, "Case 4: completely overlapping noise requests are not matching in overlap area");

  // Test case 5a: multiple chunks together
  // 1st chunk (5100ns - 5350ns)
  chunk1 = m_interference->GetBackgroundNoise (NanoSeconds (5100), NanoSeconds (5350));
  // 2nd chunk (5450ns - 5500ns)
  chunk2 = m_interference->GetBackgroundNoise (NanoSeconds (5450), NanoSeconds (5500));
  // 3rd chunk (5700ns - 5900ns)
  chunk3 = m_interference->GetBackgroundNoise (NanoSeconds (5700), NanoSeconds (5900));
  // noise (5000ns - 6000ns)
  noise1 = m_interference->GetBackgroundNoise (NanoSeconds (5000), NanoSeconds (6000));
  bool first = (chunk1 == noise1.get (2,6));
  bool second = (chunk2 == noise1.get (9,9));
  bool third = (chunk3 == noise1.get (14,17));
  bool final = first && second && third;
  if (!final)
    {
      NS_LOG_DEBUG ("FAIL: Case 5a, multiple distinct noise requests are not concatenated correctly");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Case 5a, multiple distinct noise requests are not concatenated correctly");
    }
  NS_TEST_EXPECT_MSG_EQ ( final, true, "Case 5a: multiple distinct noise requests are not concatenated correctly");

  // Test case 5b: multiple chunks together
  // Running Test Case 5b: requesting a noise from an area which exists already by means of multiple chunks");
  noise2 = m_interference->GetBackgroundNoise (NanoSeconds (5100), NanoSeconds (5550));
  if (noise2 != noise1.get (2,10))
    {
      NS_LOG_DEBUG ("FAIL: Case 5b, multiple overlapping noise requests are not matching in their overlap area");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Case 5b, multiple overlapping noise requests are not matching in their overlap area");
    }
  NS_TEST_EXPECT_MSG_EQ ( (noise2 == noise1.get (2,10)), true, "Case 5b: multiple overlapping noise requests are not matching in their overlap area");
}
