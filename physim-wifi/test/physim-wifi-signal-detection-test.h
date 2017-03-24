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

#ifndef PHYSIM_WIFI_SIGNAL_DETECTION_TEST_H
#define PHYSIM_WIFI_SIGNAL_DETECTION_TEST_H

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/physim-wifi-phy-tag.h"
#include <itpp/itcomm.h>

using namespace ns3;

/**
 * Test case to check whether the signal detection mechanism works sufficiently
 * well.
 */
class PhySimWifiSignalDetectionTest : public TestCase
{

public:
  PhySimWifiSignalDetectionTest ();
  virtual ~PhySimWifiSignalDetectionTest ();

private:
  void DoRun (void);
  void PhyTxCallback (Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  bool RunSingle (Ptr<PhySimSignalDetector> detector, Ptr<Packet> packet, itpp::cvec &samples, Ptr<PhySimWifiPhyTag> tag);

  itpp::cvec m_txSamples;

};

#endif /* PHYSIM_WIFI_SIGNAL_DETECTION_TEST_H */
