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

#include "ns3/object.h"
#include "ns3/log.h"
#include "physim-blockinterleaver.h"
#include "physim-helper.h"

NS_LOG_COMPONENT_DEFINE ("PhySimBlockInterleaver");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PhySimBlockInterleaver);

std::map<uint32_t, itpp::ivec> PhySimBlockInterleaver::m_iCache;
std::map<uint32_t, itpp::ivec> PhySimBlockInterleaver::m_diCache;

TypeId
PhySimBlockInterleaver::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimBlockInterleaver")
    .SetParent<Object> ()
    .AddConstructor<PhySimBlockInterleaver> ()
  ;
  return tid;
}

PhySimBlockInterleaver::PhySimBlockInterleaver ()
{
  m_zeropadding = 0;
}

PhySimBlockInterleaver::~PhySimBlockInterleaver ()
{

}

/*
 * \brief Takes in a bit-block and interleaves it. Block size depends on data rate.
 * @param bits input bits
 * @return interleaved bit-block
 */
itpp::bvec
PhySimBlockInterleaver::InterleaveBlock (const itpp::bvec& bits)
{
  m_zeropadding = GetPadding (bits.size ()); // calculate zero padding size
  int sizeWithPadding = bits.size () + m_zeropadding;

  itpp::bvec input (sizeWithPadding);
  input.zeros ();
  input.replace_mid (0, bits); // insert bits in new vector

  itpp::bvec output (sizeWithPadding);
  // index now has values like [i1, i2, i3, ...] and we want to set
  // output[i1]=input[0], output[i2]=input[2] and so on
  for (int k = 0; k < sizeWithPadding; ++k)
    { // applies interleaving index to the input vector
      output (iIndex (k)) = input (k);
    }

  return output;
}

/*!
 * \brief Takes in a bit-block and deinterleaves it. Block size depends on rate.
 *
 * The parameter and return types are vec rather than bvec because the values are not
 * always necessarily 0 or 1 but may be numerical indications to be used for soft-decoding.
 *
 * @param input input bits
 * @return de-interleaved bit-block
 */
itpp::vec
PhySimBlockInterleaver::DeinterleaveBlock (const itpp::vec& bits)
{
  m_zeropadding = GetPadding (bits.size ()); // calculate zero padding size
  int sizeWithPadding = bits.size () + m_zeropadding;

  itpp::vec deint (sizeWithPadding);

  for (int k = 0; k < sizeWithPadding; k++)
    { // applies deinterleaving index to the input vector
      deint (diIndex (k)) = bits (k);
    }

  itpp::vec output = deint.split (bits.size () - m_zeropadding); // removes zero-padding bits
  return output;
}

void
PhySimBlockInterleaver::SetWifiMode (const WifiMode mode)
{
  m_NCBPS = PhySimHelper::GetNCBPS (mode);

  uint32_t m_NBPSC = (m_NCBPS / 48);
  // According to Equation 17-17
  uint32_t m_s = itpp::ceil_i (static_cast<double> (m_NBPSC) / 2.0);

  // Adjust interleaving index according to selected wifi mode
  std::map<uint32_t, itpp::ivec>::iterator it = m_iCache.find (m_NCBPS);
  if (it != m_iCache.end ())
    {
      iIndex = it->second;
    }
  else
    {
      iIndex = InterleavingIndex (m_NCBPS, m_s);
      m_iCache.insert ( std::pair<uint32_t, itpp::ivec> (m_NCBPS, iIndex) );
    }

  // Adjust interleaving index according to selected wifi mode
  it = m_diCache.find (m_NCBPS);
  if (it != m_diCache.end ())
    {
      diIndex = it->second;
    }
  else
    {
      diIndex = DeinterleavingIndex (m_NCBPS, m_s);
      m_diCache.insert ( std::pair<uint32_t, itpp::ivec> (m_NCBPS, diIndex) );
    }
}

/*
 * Creates an interleaving index, which contains the new positions
 * of the bits. So an interleaving index element
 * i[0] will contain as a value the position that element 0
 * should have in the bit stream.
 *
 * @param sizeWithPadding the block size
 * @return interleaving index
 */
itpp::ivec
PhySimBlockInterleaver::InterleavingIndex (uint32_t sizeWithPadding, uint32_t m_s)
{
  itpp::ivec firstindex (sizeWithPadding);

  // First permutation 17.3.5.6, Equation 17-15
  for (uint32_t k = 0; k < sizeWithPadding; k++)
    {
      firstindex (k) = (sizeWithPadding / 16) * itpp::mod (k, 16) + itpp::floor_i (k / 16);
    }

  itpp::ivec secondindex (sizeWithPadding);

  // Second permutation 17.3.5.6, Equation 17-16
  for (uint32_t k = 0; k < sizeWithPadding; k++)
    {
      secondindex (k) = m_s * itpp::floor_i (firstindex (k) / m_s) +
        itpp::mod (firstindex (k) + sizeWithPadding - itpp::floor_i (16 * firstindex (k) / sizeWithPadding),
                   m_s);
    }

  return secondindex;
}

/*
 * Same as the InterleavingIndex, but used for deinterleaving.
 * @param sizeWithPadding the blocksize
 * @return deinterleaving index
 */
itpp::ivec
PhySimBlockInterleaver::DeinterleavingIndex (uint32_t sizeWithPadding, uint32_t m_s)
{
  itpp::ivec firstindex (sizeWithPadding);
  for (uint32_t j = 0; j < sizeWithPadding; j++)
    {
      firstindex (j) = m_s * itpp::floor_i (j / m_s) + itpp::mod (j + itpp::floor_i (16 * j
                                                                                     / (sizeWithPadding)), m_s); // Equation 17-18
    }

  itpp::ivec secondindex (sizeWithPadding);
  for (uint32_t j = 0; j < sizeWithPadding; j++)
    {
      secondindex (j) = 16 * firstindex (j) - (sizeWithPadding - 1) * itpp::floor_i (
          16 * firstindex (j) / sizeWithPadding);
    }
  return secondindex;
}

/*!
 * Returns the number of additional zero-valued elements to be appended
 * to the frame so that its size is a block multiple.
 *
 * @param size size of the frame
 * @return number of 0-bits to be appended
 */
uint32_t
PhySimBlockInterleaver::GetPadding (uint32_t size)
{
  int remaining_bits = size % m_NCBPS;
  if (remaining_bits == 0)
    {
      return 0;
    }

  NS_LOG_DEBUG ("PhySimBlockInterleaver::GetPadding(): padding was needed (shouldn't be the case in a normal run)");
  return (m_NCBPS - remaining_bits);
}


} // namespace ns3

