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

#ifndef PHYSIM_WIFI_TRANSMITTER_RECEIVER_TEST_H
#define PHYSIM_WIFI_TRANSMITTER_RECEIVER_TEST_H

#include "ns3/log.h"
#include "ns3/test.h"
#include <itpp/itcomm.h>

namespace ns3 {

/**
 * Test case to check whether sending a frame from one node to another
 * over perfect channel conditions is possible for all modes of transmission.
 */
class PhySimWifiTransmitterReceiverTest : public TestCase
{
public:
  PhySimWifiTransmitterReceiverTest ();
  virtual ~PhySimWifiTransmitterReceiverTest ();

private:
  void DoRun (void);
  bool RunSingle (std::string wifiMode);
  void PhyRxOkTrace (std::string context, Ptr<const Packet> p, Ptr<const PhySimWifiPhyTag> tag);

  bool m_success;
};

} // namespace ns3

#endif /* PHYSIM_TRANSMITTER_RECEIVER_TEST_H */
