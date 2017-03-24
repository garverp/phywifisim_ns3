/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Stylianos Papanastasiou, Jens Mittag
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

#ifndef PHYSIM_SCRAMBLER_H
#define PHYSIM_SCRAMBLER_H

#include "ns3/object.h"
#include <itpp/itcomm.h>

namespace ns3 {

/**
 * \brief Bit scrambler module (17.3.5.4 in Std).
 */
class PhySimScrambler : public Object
{
public:
  static TypeId GetTypeId (void);

  PhySimScrambler ();
  virtual ~PhySimScrambler ();

  itpp::bvec Scramble (const itpp::bvec& input);
  itpp::bvec Scramble (const itpp::bvec& input, itpp::bvec initialState);
  itpp::bvec DeScramble (const itpp::bvec& input, itpp::bvec initialState);

private:
  bool m_fixedScrambler;

};

} // namespace ns3

#endif /* PHYSIM_SCRAMBLER_H */
