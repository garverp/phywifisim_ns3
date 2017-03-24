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
#include "ns3/boolean.h"
#include "physim-convolutional-encoder.h"

NS_LOG_COMPONENT_DEFINE ("PhySimConvolutionalEncoder");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PhySimConvolutionalEncoder);

TypeId
PhySimConvolutionalEncoder::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimConvolutionalEncoder")
    .SetParent<Object> ()
    .AddConstructor<PhySimConvolutionalEncoder> ()
    .AddAttribute ("SoftViterbiDecision",
                   "Flag indicating whether soft or hard decision Viterbi decoding should be performed",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PhySimConvolutionalEncoder::m_softViterbiDecision),
                   MakeBooleanChecker ())
  ;
  return tid;
}

PhySimConvolutionalEncoder::PhySimConvolutionalEncoder ()
{
  SetupGenPolynomials ();
}

PhySimConvolutionalEncoder::~PhySimConvolutionalEncoder ()
{
}

/*
 * \brief Encode bits according to 17.3.5.5 in Std.
 */
itpp::bvec
PhySimConvolutionalEncoder::Encode (const itpp::bvec& bits)
{
  itpp::bvec encoded = m_code.encode (bits);
  return encoded.mid (0, encoded.size () - GetTailSize (m_codingRate));
}

/*
 * \brief Decodes soft or hard bits using conv. decoding.
 */
itpp::bvec
PhySimConvolutionalEncoder::Decode (const itpp::vec& bits)
{
  itpp::vec tail (GetTailSize (m_codingRate));
  tail.zeros ();
  // Hard decoding is not implemented in IT++ version 4.0.6, so we need to use
  // the soft decoding methods whether we want soft or hard decoding. These require
  // as input a binary vector of values
  // (-1, 1), so we map 1 to -1 and 0 to 1 in order to produce a suitable vector for
  // the soft decoder (if hard decoding is set to true).
  // As stated in 17.3.5.5, uses Viterbi algorithm.
  if (!m_softViterbiDecision)
    { // do hard decoding
      itpp::vec temp (bits.size ());
      for (int i = 0; i < bits.size (); ++i)
        {
          if (bits (i) == 1)
            {
              temp (i) = -1;
            }
          else
            {
              temp (i) = 1;
            }
        }
      // Have to add a tail for the decoder to do its job
      temp.ins (temp.size (), tail);
      return m_code.decode (temp);
    }
  else
    {
      // do soft decoding
      itpp::vec temp = concat (bits, tail);
      return m_code.decode (temp);
    }

}

/*!
 * \brief Sets up punctured convolutional code for a given coding rate.
 *
 * The default setting is to create a punc. matrix for 1/2 coding.
 */
void
PhySimConvolutionalEncoder::SetCodingRate (enum WifiCodeRate rate)
{
  m_codingRate = rate;
  itpp::bmat punc_matrix;
  switch (m_codingRate)
    {
    case WIFI_CODE_RATE_1_2:
      punc_matrix = "1;1";
      break;
    case WIFI_CODE_RATE_2_3:
      punc_matrix = "1 1;1 0"; // used to be "1 0;1 1" for 802.16;
      break;
    case WIFI_CODE_RATE_3_4:
      punc_matrix = "1 1 0;1 0 1";
      break;
    default:
      NS_LOG_WARN ("PhySimConvolutionalEncoder::SetCodingRate- Unknown coding rate given!");
      punc_matrix = "1;1";
    }
  m_code.set_puncture_matrix (punc_matrix);
}

void
PhySimConvolutionalEncoder::SetupGenPolynomials ()
{
  itpp::ivec generator (2); // two generating polynomials 17.3.5.5
  generator (0) = 0133; // g0
  generator (1) = 0171; // g1
  m_code.set_generator_polynomials (generator, 7);

  // Tail adds 8 bits at the end, remove them at the encoding process
  m_code.set_method (itpp::Tail);
}

/*!
 * Returns the size of the Tail (in bits) after conv. encoding
 * according to the coding rate.
 *
 * @return size of bit-tail
 */
int
PhySimConvolutionalEncoder::GetTailSize (enum WifiCodeRate rate)
{
  int tailSize;
  switch (rate)
    {
    case WIFI_CODE_RATE_1_2:
      tailSize = 12;
      break;
    case WIFI_CODE_RATE_2_3:
      tailSize = 9;
      break;
    case WIFI_CODE_RATE_3_4:
      tailSize = 8;
      break;
    default:
      NS_LOG_DEBUG ("PhySimConvolutionalEncoder::GetTailSize(): unknown rate");
      exit (-1);
    }
  return tailSize;
}

} // namespace ns3
