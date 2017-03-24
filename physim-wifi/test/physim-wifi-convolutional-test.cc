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
#include "physim-wifi-convolutional-test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhySimWifiConvolutionalTest");

PhySimWifiConvolutionalTest::PhySimWifiConvolutionalTest ()
  : TestCase ("PhySim WiFi convolutional test case")
{
  m_convolutional = CreateObject<PhySimConvolutionalEncoder> ();
}

PhySimWifiConvolutionalTest::~PhySimWifiConvolutionalTest ()
{
}

void
PhySimWifiConvolutionalTest::DoRun (void)
{
  // Provide known seed for predictable results
  SeedManager::SetSeed (1);

  WifiCodeRate rate[] = { WIFI_CODE_RATE_1_2, WIFI_CODE_RATE_2_3, WIFI_CODE_RATE_3_4};
  std::string description[] = { "1/2 Coding Rate","2/3 Coding Rate", "3/4 Coding Rate"};
  bool success;

  for (int i = 0; i < 3; ++i)
    {
      success = RunSingle (rate[i]);
      if (!success)
        {
          NS_LOG_DEBUG ("FAIL: Convolutional test for " << description[i]);
        }
      else
        {
          NS_LOG_DEBUG ("PASS: Convolutional test for " << description[i]);
        }
      NS_TEST_EXPECT_MSG_EQ ( success, true, "Convolutional test failed for " << description[i] << " : Input bits before encoding and output bits after de-coding do not match");
    }

  success = RunAnnexG ();
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: Convolutional test for input as in standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Convolutional test for input as in standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "Convolutional test failed as encoded bits do not match standard annex G");
}

bool
PhySimWifiConvolutionalTest::RunSingle (WifiCodeRate rate)
{
  // create some random bits
  itpp::bvec bits = itpp::randb (144);
  m_convolutional->SetCodingRate (rate);
  itpp::bvec encodedBits = m_convolutional->Encode (bits);
  itpp::bvec decodedBits = m_convolutional->Decode (to_vec (encodedBits));
  return (decodedBits == bits);
}

// Verify encoder against the example in the standard annex G
bool
PhySimWifiConvolutionalTest::RunAnnexG (void)
{
  // Table G.16 -- First 144 Bits After scrambling
  const itpp::bvec
  first144BitsAfterScrambling =
    "0 1 1 0 1 1 0 0 0 0 0 1 1 0 0 1 1 0 0 0 1 0 0 1 1 0 0 0 1 1 1 1 0 1 1 0 1 0 0 0 0 0 1 0 0 0 0 1 1 1 1 1 0 1 0 0 1 0 1 0 0 1 0 1 0 1 1 0 0 0 0 1 0 1 0 0 1 1 1 1 1 1 0 1 0 1 1 1 1 0 1 0 1 1 1 0 0 0 1 0 0 1 0 0 0 0 0 0 1 1 0 0 1 1 1 1 0 0 1 1 0 0 1 1 1 0 1 0 1 1 1 0 0 1 0 0 1 0 1 1 1 1 0 0";
  // Table G.18 -- Coded bits of first DATA symbol
  const itpp::bvec
  encoded192Bits =
    "0 0 1 0 1 0 1 1 0 0 0 0 1 0 0 0 1 0 1 0 0 0 0 1 1 1 1 1 0 0 0 0 1 0 0 1 1 1 0 1 1 0 1 1 0 1 0 1 1 0 0 1 1 0 1 0 0 0 0 1 1 1 0 1 0 1 0 0 1 0 1 0 1 1 1 1 1 0 1 1 1 1 1 0 1 0 0 0 1 1 0 0 0 0 1 0 1 0 0 0 1 1 1 1 1 1 0 0 0 0 0 0 1 1 0 0 1 0 0 0 0 1 1 1 0 0 1 1 1 1 0 0 0 0 0 0 0 1 0 0 0 0 1 1 1 1 1 0 0 0 0 0 0 0 0 1 1 0 0 1 1 1 1 0 0 0 0 0 1 1 0 1 0 0 1 1 1 1 1 0 1 0 1 1 1 0 1 1 0 0 1 0";
  m_convolutional->SetCodingRate (WIFI_CODE_RATE_3_4);
  itpp::bvec encodedBits = m_convolutional->Encode (first144BitsAfterScrambling);
  return (encodedBits == encoded192Bits);
}
