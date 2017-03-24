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

#include "ns3/log.h"
#include "ns3/test.h"
#include "physim-wifi-test-suite.h"
#include "physim-wifi-frame-construction-test.h"
#include "physim-wifi-noise-chunk-test.h"
#include "physim-wifi-interleaver-test.h"
#include "physim-wifi-convolutional-test.h"
#include "physim-wifi-scrambler-test.h"
#include "physim-wifi-modulator-test.h"
#include "physim-wifi-cca-busy-test.h"
#include "physim-wifi-transmitter-receiver-test.h"
#include "physim-wifi-simple-estimator-test.h"
#include "physim-wifi-freqoffset-estimator-test.h"
#include "physim-wifi-frame-power-test.h"
#include "physim-wifi-signal-detection-test.h"
#include "physim-wifi-vehicular-channel-test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhySimWifiTestSuite");

PhySimWifiTestSuite::PhySimWifiTestSuite ()
  : TestSuite ("devices-physim-wifi", UNIT)
{
  AddTestCase (new PhySimWifiNoiseChunkTest);
  AddTestCase (new PhySimWifiCcaBusyTest);
  AddTestCase (new PhySimWifiFrameConstructionTest);
  AddTestCase (new PhySimWifiInterleaverTest);
  AddTestCase (new PhySimWifiConvolutionalTest);
  AddTestCase (new PhySimWifiScramblerTest);
  AddTestCase (new PhySimWifiModulatorTest);
  AddTestCase (new PhySimWifiTransmitterReceiverTest);
  AddTestCase (new PhySimWifiSimpleEstimatorTest);
  AddTestCase (new PhySimWifiFreqOffsetEstimatorTest);
  AddTestCase (new PhySimWifiFramePowerTest);
  AddTestCase (new PhySimWifiSignalDetectionTest);
  AddTestCase (new PhySimWifiVehicularChannelTest);
}

// create an instance of the test suite
PhySimWifiTestSuite g_phySimWifiTestSuite;
