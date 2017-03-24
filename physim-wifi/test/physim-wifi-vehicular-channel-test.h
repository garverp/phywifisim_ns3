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

#ifndef PHYSIM_WIFI_VEHICULAR_CHANNEL_TEST_H_
#define PHYSIM_WIFI_VEHICULAR_CHANNEL_TEST_H_

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/ptr.h"
#include "ns3/physim-wifi-phy-tag.h"
#include "ns3/physim-vehicular-channel-spec.h"
#include "ns3/physim-propagation-loss-model.h"
#include <itpp/itcomm.h>

using namespace ns3;

/**
 * Test case to verify that the vehicular channel models are implemented
 * correctly and generate correct channel coefficients
 */
class PhySimWifiVehicularChannelTest : public TestCase
{
public:
  PhySimWifiVehicularChannelTest ();
  virtual ~PhySimWifiVehicularChannelTest ();

private:
  void DoRun (void);
  bool RunSingle (enum VEHICULAR_CHANNEL_PROFILE profile, itpp::cvec first, itpp::cvec second);
  bool CompareVectors (const itpp::cvec &input, const itpp::cvec &reference);
  std::complex<double> RoundComplex (const std::complex<double> input);
  double RoundIEEE (const double num);

};

#endif /* PHYSIM_WIFI_VEHICULAR_CHANNEL_TEST_H_ */
