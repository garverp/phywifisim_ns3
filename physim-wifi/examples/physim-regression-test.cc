/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Jens Mittag
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
 * Author: Jens Mittag <jens.mittag@kit.edu>
 */

#include "ns3/core-module.h"
#include "ns3/common-module.h"
#include "ns3/node-module.h"
#include "ns3/helper-module.h"
#include "ns3/physim-wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/contrib-module.h"

NS_LOG_COMPONENT_DEFINE ("Main");

using namespace ns3;

int
main (int argc, char *argv[])
{
  LogComponentEnable ("PhySimWifiFrameConstructionTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiNoiseChunkTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiInterleaverTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiConvolutionalTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiScramblerTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiModulatorTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiCcaBusyTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiTransmitterReceiverTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiSimpleEstimatorTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiFreqOffsetEstimatorTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiFramePowerTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiSignalDetectionTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiIncreaseSNRTest", LOG_LEVEL_ALL);
  LogComponentEnable ("PhySimWifiVehicularChannelTest", LOG_LEVEL_ALL);

  PhySimWifiTestSuite m_phySimWifiTestSuite;
  m_phySimWifiTestSuite.Run ();
  return 0;
}
