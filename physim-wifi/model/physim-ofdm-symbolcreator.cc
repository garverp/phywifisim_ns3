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
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/object-factory.h"
#include "ns3/enum.h"
#include "physim-ofdm-symbolcreator.h"
#include "physim-helper.h"
#include <itpp/itcomm.h>

NS_LOG_COMPONENT_DEFINE ("PhySimOFDMSymbolCreator");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PhySimOFDMSymbolCreator);

const itpp::cvec PhySimOFDMSymbolCreator::m_subcarrierValues = "1 1 1 -1";
const uint32_t PhySimOFDMSymbolCreator::m_subcarrierPolarity[]
  = {
  1, 1, 1, 1, -1, -1, -1, 1, -1, -1, -1, -1, 1, 1, -1, 1, -1, -1, 1, 1, -1, 1, 1, -1,
  1, 1, 1, 1, 1, 1, -1, 1, 1, 1, -1, 1, 1, -1, -1, 1, 1, 1, -1, 1, -1, -1, -1, 1, -1,
  1, -1, -1, 1, -1, -1, 1, 1, 1, 1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, -1, 1, 1, -1,
  -1, -1, 1, 1, -1, -1, -1, -1, 1, -1, -1, 1, -1, 1, 1, 1, 1, -1, 1, -1, 1, -1, 1, -1,
  -1, -1, -1, -1, 1, -1, 1, 1, -1, 1, -1, 1, 1, 1, -1, -1, 1, -1, -1, -1, 1, 1, 1, -1,
  -1, -1, -1, -1, -1, -1
  };

TypeId
PhySimOFDMSymbolCreator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimOFDMSymbolCreator")
    .SetParent<Object> ()
    .AddConstructor<PhySimOFDMSymbolCreator> ()
    .AddAttribute ("IEEECompliantMode",
                   "Flag indicating whether we will operate using the IEEE compliant mode, which means we will (de-)normalise the in-/outputs",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PhySimOFDMSymbolCreator::m_ieeeCompliantMode),
                   MakeBooleanChecker ())
    .AddAttribute ("SoftViterbiDecision",
                   "Flag indicating whether soft or hard decision Viterbi decoding should be performed",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PhySimOFDMSymbolCreator::m_softViterbiDecision),
                   MakeBooleanChecker ())
    .AddAttribute ("SoftDecisionMethod",
                   "The soft Viterbi decision method that shall be used.",
                   EnumValue (itpp::LOGMAP),
                   MakeEnumAccessor (&PhySimOFDMSymbolCreator::m_softMethod),
                   MakeEnumChecker (itpp::LOGMAP, "Log-MAP full calculation method",
                                    itpp::APPROX, "Approximate faster method"));
  return tid;
}

PhySimOFDMSymbolCreator::PhySimOFDMSymbolCreator ()
  : m_bpsk (0),
    m_qpsk (0),
    m_qam16 (0),
    m_qam64 (0)
{
  m_noCarriers = m_DefaultNoCarriers;
  m_norm = m_DefaultNorm;
  m_NCP = m_DefaultNCP;
  m_modulationType = PhySimHelper::BPSK;
  m_constellation = 2;
}

PhySimOFDMSymbolCreator::~PhySimOFDMSymbolCreator ()
{
  delete m_bpsk;
  delete m_qpsk;
  delete m_qam16;
  delete m_qam64;
}

void
PhySimOFDMSymbolCreator::SetChannelEstimator (std::string type)
{
  ObjectFactory factory;
  factory.SetTypeId (type);
  m_estimator = factory.Create<PhySimChannelEstimator> ();
  NS_LOG_DEBUG ("PhySimOFDMSymbolCreator::SetChannelEstimator() using type '" << type << "'");
}

Ptr<PhySimChannelEstimator>
PhySimOFDMSymbolCreator::GetChannelEstimator ()
{
  return m_estimator;
}
/*!
 * \brief Turns a bitstream to an 802.11 OFDM symbol.
 *
 * We assume that the bitstream fits exactly into a single OFDM symbol.
 * Then, 48 subcarriers are created for DATA symbols + 4 pilots
 * We get the DC (pos 26) to 0
 * We get 27-52 to 1-26
 * We insert 0s to to 27-37
 * We get 0-25  to 38-63
 * The result is padded to 64 subcarriers.
 *
 * @param input input bits
 * @param symbolIndex 0 for SIGNAL or 1..n for OFDM data symbols 1..n
 * @return time samples
 */
itpp::cvec
PhySimOFDMSymbolCreator::Modulate (const itpp::bvec &input, const int symbolIndex)
{
  NS_ASSERT ( input.size () == CheckSize () );

  itpp::cvec modulated = ModulateBits (input);

  if (modulated.size () != 48)
    {
      NS_LOG_DEBUG ("PhySimOFDMSymbolCreator::Modulate(): modulated data incorrect");
      exit (-1);
    }

  if (m_ieeeCompliantMode)
    {
      // turns IT++ notation to the one used in IEEE Standard.
      TransformModulation (modulated, true);
    }

  InsertPilots (modulated, symbolIndex);

  // put modulated input in data
  itpp::cvec data (64);
  data = TransformToOutput (modulated);

  // OFDM modulation
  m_ofdm.set_parameters (m_noCarriers, m_NCP, m_norm);
  itpp::cvec output = m_ofdm.modulate (data);

  return Normalise (output, false);
}

/*
 * Takes a cvec representing the 53 OFDM subcarriers (inc. DC) and produces an OFDM symbol. There is no additional modulation performed.
 * The cyclic prefix defines how much (if at all) of a cyclic prefix will be added.
 *
 * @param input an already modulated bit representation
 * @param cyclic_prefix number of samples for cyclic prefix
 * @return an OFDM symbol
 */
itpp::cvec
PhySimOFDMSymbolCreator::Modulate (const itpp::cvec &input, int cyclic_prefix)
{
  NS_ASSERT ( input.size () == 53 );

  itpp::cvec ofdmedOutput;
  itpp::cvec output (64);

  m_NCP = cyclic_prefix;
  m_ofdm.set_parameters (m_noCarriers, m_NCP, m_norm);
  output = TransformToOutput (input);
  ofdmedOutput = m_ofdm.modulate (output);

  NS_LOG_DEBUG ("PhySimOFDMSymbolCreator:Modulate() - will return normalised output");
  output = Normalise (ofdmedOutput, false);

  // Finally set DefaultNCP again...
  m_NCP = m_DefaultNCP;
  return output;
}

/*
 * Given an OFDM symbol return the contained bits (soft or hard).
 *
 * @param input the OFDM symbol
 * @param symbolNo the symbol number which we are currently demodulating
 * @return demodulated bits (soft or hard)
 */
itpp::vec
PhySimOFDMSymbolCreator::DeModulate (const itpp::cvec &input, uint32_t symbolNo)
{

  NS_LOG_FUNCTION ( symbolNo );

  itpp::cvec demodulatedOFDM;
  itpp::cvec fixedInput;

  NS_LOG_DEBUG ("PhySimOFDMSymbolCreator:DeModulate() input.size(): " << input.size () <<
                " m_noCarriers: " << m_noCarriers << " m_NCP: " << m_NCP << " m_norm: " << m_norm);

  m_ofdm.set_parameters (m_noCarriers, m_NCP, m_norm);

  // Normalise the OFDM
  itpp::cvec normalised = Normalise (input, true);
  // Demodulate the OFDM
  demodulatedOFDM = m_ofdm.demodulate (normalised);

  // Perform pilot symbol channel estimation and corrections...
  itpp::cvec transformedInput = TransformToInput (demodulatedOFDM);
  fixedInput = m_estimator->ApplyOFDMSymbolCorrection (transformedInput, symbolNo);

  // remove pilots
  // remember, when you remove stuff it is important to remove them from
  // high to low so that the offsets of the other positions do not change
  fixedInput.del (47);
  fixedInput.del (33);
  fixedInput.del (26);
  fixedInput.del (19);
  fixedInput.del (5);

  if (m_ieeeCompliantMode)
    {
      // IEEE Standard to IT++ notation
      NS_LOG_DEBUG ("PhySimOFDMSymbolCreator:DeModulate() IEEE compliant mode - will apply TransformModulation()");
      TransformModulation (fixedInput, false);
    }

  // Demodulate real data
  return DeModulateBits (fixedInput);
}

void
PhySimOFDMSymbolCreator::SetModulationType (enum PhySimHelper::ModulationType type, uint8_t constellation)
{

  m_modulationType = type;
  m_constellation = constellation;
}

/*!
 * \brief (De)normalise the OFDM output produced by IT++.
 *
 * IT++ by default normalises the OFDM modulated samples. We reverse the normalisation
 * so as to be consistent with the conventions used in the 802.11 standard.
 * The output is normalised according to m_noCarriers, m_norm and m_NCP so they always need to be
 * up to date.
 * @param input time samples to (de)normalise
 * @param multiply if true normalise by mult. with a factor, otherwise by division
 * @return normalised time samples
 */
itpp::cvec
PhySimOFDMSymbolCreator::Normalise (const itpp::cvec &input, bool multiply)
{
  double normFactor = sqrt (static_cast<double> (m_norm * m_noCarriers * m_noCarriers) / (m_noCarriers + m_NCP));
  if (multiply)
    {
      return input * normFactor;
    }
  else
    {
      return input / normFactor;
    }
}

/*
 * \brief Prepares a 53-bit input vector for the IFFT.
 *
 * Maps elements 0..52 of the array to an array 0..63 as input for the IFFT.
 * Note:If alternate is true then this transformation does not take place, instead
 * 6 zeroes are prepended at the front and 5 at the end.
 * @param input the input should be an array of 53 elements. Element 26 should be 0 (the DC)
 * @return correctly placed inputs ready for the IFFT
 */
itpp::cvec
PhySimOFDMSymbolCreator::TransformToOutput (const itpp::cvec &input)
{
  // The following must be true it input is to be used for IFFT
  NS_ASSERT ( input.size () == 53 );
  NS_ASSERT ( input (26) == 0.0);
  itpp::cvec data (64);
  data.zeros ();
  // map Fig. 17-3
  // data(0) should be input(26) -offset 0- which is 0 so we leave as is
  // data(1..26) should be input elements(27-53) -offset 1..26-
  data.replace_mid (1, input.mid (27, 26));
  // data (27..37) should be zero so we leave them as is
  // data (38..63) should be input 0 to 25
  data.replace_mid (38, input.mid (0, 26));
  return data;
}

/*
 * \brief Read just the mixed input into a properly ordered array
 *
 * Take care to choose the right transformation (according to the flag)
 * @param input - a 64 bit input
 * @return correctly positioned complex representations
 */
itpp::cvec
PhySimOFDMSymbolCreator::TransformToInput (const itpp::cvec &input)
{
  NS_ASSERT ( input.size () == 64 );
  itpp::cvec data (53);
  data (26) = 0; // the DC
  data.replace_mid (0, input.mid (38, 26)); // 38-63 is really 0..25
  data.replace_mid (27, input.mid (1, 26)); // 1-26 is really 27-52
  return data;
}

int
PhySimOFDMSymbolCreator::CheckSize ()
{
  switch (m_modulationType)
    {
    case PhySimHelper::BPSK:
      return 48;
    case PhySimHelper::QPSK:
      return 96;
    case PhySimHelper::QAM16:
      return 192;
    case PhySimHelper::QAM64:
      return 288;
    default:
      NS_LOG_DEBUG ("PhySimOFDMSymbolCreator::CheckSize(): unsupported modulation type");
      exit (-1);
    }
}

/*
 * Inserts pilots in the right places in the input vector (depending on the symbol index).
 *
 * @param input OFDM time samples
 * @param symbolIndex position of OFDM symbol in the PPDU (0 means SIGNAL)
 */
void
PhySimOFDMSymbolCreator::InsertPilots (itpp::cvec &input, int symbolIndex)
{
  itpp::cvec pilotSubCarriers = GetPilotSubcarrier (symbolIndex);
  input.ins (5, pilotSubCarriers (0)); // subcarrier index -21 (index 5 if -27 is origin)
  input.ins (19, pilotSubCarriers (1)); // -7
  input.ins (26, 0 + 0i); // DC -0 subcarrier (has to be 0) -- 17.3.5.9
  input.ins (33, pilotSubCarriers (2)); // 7
  input.ins (47, pilotSubCarriers (3)); // 21
  // size will be 52 total = 48 data+ 4 pilot really, 53 to also account for the DC at offset 0
}

itpp::cvec
PhySimOFDMSymbolCreator::ModulateBits (const itpp::bvec& input)
{
  switch (m_modulationType)
    {
    case PhySimHelper::BPSK:
      if (!m_bpsk)
        {
          m_bpsk = new itpp::BPSK_c;
        }
      return m_bpsk->modulate_bits (input);

    case PhySimHelper::QPSK:
      if (!m_qpsk)
        {
          m_qpsk = new itpp::QPSK;
        }
      return m_qpsk->modulate_bits (input);

    case PhySimHelper::QAM16:
      if (!m_qam16)
        {
          m_qam16 = new itpp::QAM (16);
        }
      return m_qam16->modulate_bits (input);

    case PhySimHelper::QAM64:
      if (!m_qam64)
        {
          m_qam64 = new itpp::QAM (64);
        }
      return m_qam64->modulate_bits (input);

    default: // Default is BPSK
      if (!m_bpsk)
        {
          m_bpsk = new itpp::BPSK_c;
        }
      return m_bpsk->modulate_bits (input);
    }
}

itpp::vec
PhySimOFDMSymbolCreator::DeModulateBits (const itpp::cvec& input)
{
  double spectralNoiseDensity = m_estimator->GetDetectedNoise ();
  switch (m_modulationType)
    {
    case PhySimHelper::BPSK:
      if (!m_bpsk)
        {
          m_bpsk = new itpp::BPSK_c;
        }
      if (!m_softViterbiDecision)
        {
          return itpp::to_vec (m_bpsk->demodulate_bits (input));
        }
      else
        {
          return m_bpsk->demodulate_soft_bits (input, spectralNoiseDensity, m_softMethod);
        }

    case PhySimHelper::QPSK:
      if (!m_qpsk)
        {
          m_qpsk = new itpp::QPSK;
        }
      if (!m_softViterbiDecision)
        {
          return itpp::to_vec (m_qpsk->demodulate_bits (input));
        }
      else
        {
          return m_qpsk->demodulate_soft_bits (input, spectralNoiseDensity, m_softMethod);
        }

    case PhySimHelper::QAM16:
      if (!m_qam16)
        {
          m_qam16 = new itpp::QAM (16);
        }
      if (!m_softViterbiDecision)
        {
          return itpp::to_vec (m_qam16->demodulate_bits (input));
        }
      else
        {
          return m_qam16->demodulate_soft_bits (input, spectralNoiseDensity, m_softMethod);
        }

    case PhySimHelper::QAM64:
      if (!m_qam64)
        {
          m_qam64 = new itpp::QAM (64);
        }
      if (!m_softViterbiDecision)
        {
          return itpp::to_vec (m_qam64->demodulate_bits (input));
        }
      else
        {
          return m_qam64->demodulate_soft_bits (input, spectralNoiseDensity, m_softMethod);
        }

    default:   // Default is BPSK
      if (!m_bpsk)
        {
          m_bpsk = new itpp::BPSK_c;
        }
      if (!m_softViterbiDecision)
        {
          return itpp::to_vec (m_bpsk->demodulate_bits (input));
        }
      else
        {
          return m_bpsk->demodulate_soft_bits (input, spectralNoiseDensity, m_softMethod);
        }
    }
}

/*!
 * Returns the appropriate SubCarrier for an OFDM symbol at a given position.
 * Symbol positions begin with 0, with position 0 indicating the SIGNAL and 1..n the respective DATA symbols.
 * The four pilot symbols should be in subcarriers -21,-7,7,21
 * (or if we disregard the offset, i.e. starting the position count from 0, positions 5, 19, 33, 47).
 * Subcarrier 0 is empty.
 * The values are derived from the subcarrier values vector, where each element is multiplied by
 * a suitable element from the polarity vector (corresponding to the OFDM symbol position),
 * which then provides the 4 element pilot vector.
 * @param symbolIndex 0 for SIGNAL, 1..n for OFDM symbols
 * @return a vector containing the four pilots
 */
itpp::cvec
PhySimOFDMSymbolCreator::GetPilotSubcarrier (const int symbolIndex)
{
  NS_ASSERT ( symbolIndex >= 0 );
  int pilotsPolarity = PhySimOFDMSymbolCreator::m_subcarrierPolarity[symbolIndex % 126];
  return (PhySimOFDMSymbolCreator::m_subcarrierValues * pilotsPolarity);
}

/*
 * \brief Change the polarity of vector and exchange real and imaginary parts.
 */
void
PhySimOFDMSymbolCreator::SwitchPartsAndSign (itpp::cvec& input)
{
  itpp::cvec copy (input);
  for (int i = 0; i < input.size (); ++i)
    {
      input (i) = (-1.0) * std::complex<double> (copy (i).imag (), copy (i).real ());
    }
}

/*!
 * \brief Convert QPSK constellation from IT++ to IEEE (and vice-versa).
 */
void PhySimOFDMSymbolCreator::TransformModulationQPSK (itpp::cvec &modulated,
                                                       bool toIEEE)
{
  if (toIEEE)         // IT++ -> IEEE
    {
      for (int i = 0; i < modulated.size (); ++i)
        {
          std::complex<double> y = std::complex<double> (-modulated (i).real (),
                                                         modulated (i).imag ());
          modulated (i) = y * std::complex<double> (cos (M_PI_4), sin (M_PI_4));
        }
    }
  else           // IEEE -> IT++
    {
      for (int i = 0; i < modulated.size (); ++i)
        {
          std::complex<double> z = modulated (i) * std::complex<double> (cos (-M_PI_4),
                                                                         sin (-M_PI_4));
          modulated (i) = std::complex<double> (-z.real (), z.imag ());
        }
    }
}

/*
 * Transforms the modulation result to use IEEE standard semantics instead of IT++.
 * This should be called during modulation and demodulation to make sure we are
 * being consistent.
 * @param modulated the representation to be transformed
 * @param toIEEE true if converting from IT++ to IEEE, false otherwise
 */
void
PhySimOFDMSymbolCreator::TransformModulation (itpp::cvec &modulated, bool toIEEE)
{
  switch (m_modulationType)
    {
    case PhySimHelper::BPSK:
      modulated *= -1;
      break;
    case PhySimHelper::QPSK:
      TransformModulationQPSK (modulated, toIEEE);
      break;
    case PhySimHelper::QAM16:
      SwitchPartsAndSign (modulated);
      break;
    case PhySimHelper::QAM64:
      SwitchPartsAndSign (modulated);
      break;
    default:
      NS_LOG_DEBUG ("PhySimOFDMSymbolCreator::TransformModulation(): unsupported modulation type");
      break;
    }
}

} // namespace ns3
