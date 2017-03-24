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

#ifndef PHYSIM_WIFI_FRAME_CONSTRUCTION_TEST_H
#define PHYSIM_WIFI_FRAME_CONSTRUCTION_TEST_H

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/physim-wifi-phy-tag.h"
#include <itpp/itcomm.h>

using namespace ns3;

/**
 * Test case to check whether the frame construction process of the physical
 * layer logic adheres to the reference given in the IEEE 802.11 standard in
 * Section 17.
 */
class PhySimWifiFrameConstructionTest : public TestCase
{

public:
  PhySimWifiFrameConstructionTest ();
  virtual ~PhySimWifiFrameConstructionTest ();

private:
  void DoRun (void);
  void PhyTxCallback (Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  bool CompareVectors (const itpp::cvec &input, const itpp::cvec &reference);
  std::complex<double> RoundComplex (const std::complex<double> input);
  double RoundIEEE (const double num);

  itpp::cvec m_txSamples;

  itpp::cvec m_refShortSymbol;
  itpp::cvec m_refLongSymbol;
  itpp::cvec m_refSIGNAL;
  itpp::cvec m_refFirstDATAblock;
  itpp::cvec m_refSecondDATAblock;
  itpp::cvec m_refThirdDATAblock;
  itpp::cvec m_refFourthDATAblock;
  itpp::cvec m_refFifthDATAblock;
  itpp::cvec m_refSixthDATAblock;

};

#endif /* PHYSIM_WIFI_FRAME_CONSTRUCTION_TEST_H */
