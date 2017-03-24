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

#ifndef PHYSIM_WIFI_SIMPLE_ESTIMATOR_TEST_H
#define PHYSIM_WIFI_SIMPLE_ESTIMATOR_TEST_H

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/physim-propagation-loss-model.h"

#include <itpp/itcomm.h>

namespace ns3 {

/**
 * Test case to check whether the simple estimator can reverse the effect of a
 * channel that inverses the polarity of the OFDM symbols. In the absense of significant
 * noise the effect should be completely reversible.
 */
class PhySimWifiSimpleEstimatorTest : public TestCase
{
public:
  PhySimWifiSimpleEstimatorTest ();
  virtual ~PhySimWifiSimpleEstimatorTest ();

private:
  void DoRun (void);
  bool RunSingle (std::string wifiMode);
  void PhyRxOkTrace (std::string context, Ptr<const Packet> p, Ptr<const PhySimWifiPhyTag> tag);

  bool m_success;
};

/**
 * Channel model that reverses the polarity of the OFDM symbols
 */
class PhySimReversePolarityPropagationLoss : public PhySimPropagationLossModel
{
public:
  static TypeId GetTypeId (void);

  PhySimReversePolarityPropagationLoss ();
  virtual ~PhySimReversePolarityPropagationLoss ();

private:
  PhySimReversePolarityPropagationLoss (const PhySimReversePolarityPropagationLoss &o);
  PhySimReversePolarityPropagationLoss &operator = (const PhySimReversePolarityPropagationLoss &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

};

} // namespace ns3

#endif /* PHYSIM_SIMPLE_ESTIMATOR_TEST_H */
