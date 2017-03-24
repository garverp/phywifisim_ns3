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

#include "ns3/log.h"
#include "ns3/boolean.h"
#include "physim-scrambler.h"

NS_LOG_COMPONENT_DEFINE ("PhySimScrambler");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PhySimScrambler);

TypeId
PhySimScrambler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimScrambler")
    .SetParent<Object> ()
    .AddConstructor<PhySimScrambler> ()
    .AddAttribute ("UseFixedScrambler",
                   "Flag indicating whether we will operate using the IEEE testing mode for the initial state of the scrambler",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PhySimScrambler::m_fixedScrambler),
                   MakeBooleanChecker ())
  ;
  return tid;
}

PhySimScrambler::PhySimScrambler ()
{
}

PhySimScrambler::~PhySimScrambler ()
{

}

/*!
 * \brief Scramble input bits using a random or fixed initial state.
 */
itpp::bvec
PhySimScrambler::Scramble (const itpp::bvec& input)
{

  itpp::bvec initialState (7);
  if (m_fixedScrambler)
    {
      // Just for testing against IEEE standard example
      NS_LOG_DEBUG ("PhySimScrambler:Scramble() using fixed initialState for scrambler.");
      initialState = "1 0 1 1 1 0 1";
    }
  else
    {
      NS_LOG_DEBUG ("PhySimScrambler:Scramble() using random initialState for scrambler");
      initialState = itpp::randb (7);
    }

  return Scramble (input, initialState);
}

/*!
 * \brief Scramble input bits using a given initial state.
 */
itpp::bvec
PhySimScrambler::Scramble (const itpp::bvec& input, itpp::bvec initialState)
{

  itpp::bvec shiftRegister (initialState);
  itpp::bvec output (size (input));
  itpp::bin shifterBit;

  for (int i = 0; i < size (input); i++)
    {
      shifterBit = shiftRegister (3) + shiftRegister (6); // x^4 + x^7
      output (i) = shifterBit + input (i);
      shiftRegister.shift_right (shifterBit);
    }

  return output;
}

/*!
 * \brief Descramble input bits given an (inversed) initial state.
 *
 * Note: the state is given in reverse as opposed to the scramble case. There is also
 * an addition of seven 0-bits at the beginning of returned bit-vector.
 */
itpp::bvec
PhySimScrambler::DeScramble (const itpp::bvec& input, itpp::bvec initialState)
{
  // The value has to be read from left to right... do that now
  itpp::bvec m_initialState (initialState.size ());

  for (uint32_t i = 0, k = 6; i < 7; ++i, --k)
    {
      m_initialState (i) = initialState (k);
    }

  itpp::bvec data = Scramble (input, m_initialState);
  itpp::bvec added = "0 0 0 0 0 0 0";
  data.ins (0, added);
  return data;
}

} // namespace ns3
