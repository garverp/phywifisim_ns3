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

#ifndef PHYSIM_WIFI_TEST_SUITE_H
#define PHYSIM_WIFI_TEST_SUITE_H

#include "ns3/log.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * Unit test suite for the PhySim WiFi module that tests several aspects of the
 * implementation.
 *      - frame construction process
 *      - frame reception process
 *      - block interleaving mechanism
 *      - convolutional encoder/decoder
 *      - bit scrambling mechanism
 *      - OFDM modulation
 *      - transmission/reception over a perfect channel
 *      - simple channel estimator
 *      - power level equality for relevant parts of the frame
 *      - signal detection mechanism
 *      - validity of a basic SNR vs FER expectations
 *      - underlying state machine, in particular the CCA busy indication to MAC layer
 *      - vehicular channel models
 */
class PhySimWifiTestSuite : public TestSuite
{
public:
  PhySimWifiTestSuite ();
};

#endif /* PHYSIM_WIFI_TEST_SUITE_H */
