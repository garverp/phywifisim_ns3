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

#ifndef PHYSIM_WIFI_FRAME_POWER_TEST_H
#define PHYSIM_WIFI_FRAME_POWER_TEST_H

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/physim-wifi-phy-tag.h"
#include <itpp/itcomm.h>

using namespace ns3;

/**
 * Test case to check whether after the frame construction process
 * the average powers of the short training sequence, the long training
 * sequence and the produced OFDM DATA symbols roughly match (as the
 * 802.11 standard dictates)
 */
class PhySimWifiFramePowerTest : public TestCase
{

public:
  PhySimWifiFramePowerTest ();
  virtual ~PhySimWifiFramePowerTest ();

private:
  void DoRun (void);
  void PhyTxCallback (Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  bool RunSingle (std::string wifiMode);
  double CalcPower (const itpp::cvec& input);

  bool m_success;
  itpp::cvec m_txSamples;
};

#endif /* PHYSIM_WIFI_FRAME_POWER_TEST_H */
