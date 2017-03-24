/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) Stylianos Papanastasiou, Jens Mittag
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
 * Jens Mittag <jens.mittag@kit.edu>
 * Stylianos Papanastasiou <stylianos@gmail.com>
 *
 */

#ifndef PHYSIM_CONVOLUTIONAL_ENCODER_H
#define PHYSIM_CONVOLUTIONAL_ENCODER_H

#include "ns3/object.h"
#include "ns3/wifi-mode.h"
#include <itpp/itcomm.h>

namespace ns3 {


/**
 * \brief A convolutional encoder used by the PhySimWifiPhy implementation.
 */


class PhySimConvolutionalEncoder : public Object
{
public:
  static TypeId GetTypeId (void);

  PhySimConvolutionalEncoder ();
  virtual ~PhySimConvolutionalEncoder ();
  void SetCodingRate (enum WifiCodeRate rate);

  itpp::bvec Encode (const itpp::bvec& bits);
  itpp::bvec Decode (const itpp::vec& bits);

private:
  void SetupGenPolynomials ();
  int GetTailSize (enum WifiCodeRate rate);

  itpp::Punctured_Convolutional_Code m_code;
  enum WifiCodeRate m_codingRate;
  bool m_softViterbiDecision;

};

} // namespace ns3

#endif /* PHYSIM_CONVOLUTIONAL_ENCODER_H */
