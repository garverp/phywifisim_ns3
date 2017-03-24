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

#ifndef PHYSIM_WIFI_NOISE_CHUNK_TEST_H
#define PHYSIM_WIFI_NOISE_CHUNK_TEST_H

#include "ns3/test.h"

using namespace ns3;

/**
 * Test case to check whether the frame construction process of the physical
 * layer logic adheres to the reference given in the IEEE 802.11 standard in
 * Section 17.
 */
class PhySimWifiNoiseChunkTest : public TestCase
{

public:
  PhySimWifiNoiseChunkTest ();
  virtual ~PhySimWifiNoiseChunkTest ();

private:
  void DoRun (void);

};

#endif /* PHYSIM_WIFI_NOISE_CHUNK_TEST_H */
