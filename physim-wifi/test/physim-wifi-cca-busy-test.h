/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Jens Mittag, Karlsruhe Institute of Technology
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

#ifndef PHYSIM_WIFI_CCA_BUSY_TEST_H
#define PHYSIM_WIFI_CCA_BUSY_TEST_H

#include "ns3/test.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/net-device.h"
#include "ns3/physim-wifi-phy-tag.h"
#include "ns3/physim-wifi-phy.h"
#include "physim-wifi-state-checker.h"

using namespace ns3;

class PhySimWifiCcaSimulationExperiment
{

public:
  enum TestId
  {
    TEST_1A,
    TEST_1B,
    TEST_2A,
    TEST_2B,
    TEST_3A,
    TEST_3B,
    TEST_4A,
    TEST_4B,
    TEST_5A,
    TEST_5B
  };

  struct Test
  {
    enum TestId id;
    std::string desc;
  };

  PhySimWifiCcaSimulationExperiment ();
  virtual ~PhySimWifiCcaSimulationExperiment ();

  bool DoRun (enum TestId number);

private:
  void StateLogger (std::string context, Ptr<NetDevice> device, Time start, Time duration, enum PhySimWifiPhy::State state);

  PhySimWifiStateChecker m_checker;
  bool m_failed;
  Time m_difs;
  std::map<uint32_t, uint32_t> m_nodes;

};

class PhySimWifiCcaBusyTest : public TestCase
{

public:
  PhySimWifiCcaBusyTest ();
  virtual ~PhySimWifiCcaBusyTest ();

private:
  void DoRun (void);

};

#endif /* PHYSIM_WIFI_CCA_BUSY_TEST_H */
