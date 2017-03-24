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
 * Authors:
 *      Stylianos Papanastasiou <stylianos@gmail.com>
 *      Jens Mittag <jens.mittag@kit.edu>
 */

#include "physim-channel-estimator.h"
#include "physim-ofdm-symbolcreator.h"
#include "ns3/log.h"
#include "ns3/double.h"

NS_LOG_COMPONENT_DEFINE ("PhySimChannelEstimator");

namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (PhySimChannelEstimator);

TypeId
PhySimChannelEstimator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimChannelEstimator")
    .SetParent<Object> ()
    .AddConstructor<PhySimChannelEstimator> ()
    .AddAttribute ("Noise Estimate N0",
                   "An estimate of spectral density noise of the AWGN noise at the receiver",
                   DoubleValue (0.0001),
                   MakeDoubleAccessor (&PhySimChannelEstimator::m_N0),
                   MakeDoubleChecker<double> ());
  return tid;
}

PhySimChannelEstimator::PhySimChannelEstimator ()
{
}

PhySimChannelEstimator::~PhySimChannelEstimator ()
{
}

/*
 * \brief Given samples up to the SIGNAL (i.e. the short and long training sequences)
 *  calculate an initial channel estimate.
 */
double
PhySimChannelEstimator::GetInitialChannelEstimation (const itpp::cvec &input)
{
  return 0.0;
}

/*
 * \brief Apply the estimate calculated from the training sequences. This function should only be called
 * twice: Once for the training sequence and once for the actual OFDM blocks.
 *
 * @param input time samples
 * @param estimate the initial channel estimation
 * @param phaseOffset acts as a flag for the TS, so we define 0 for training sequence, 1 for frame OFDM symbols
 * @return corrected TS or OFDM symbol
 */
itpp::cvec
PhySimChannelEstimator::ApplyEstimateFromTrainingSequence (const itpp::cvec &input, double estimate, int phaseOffset)
{
  // Dummy behavior: simply return input
  return input;
}

/*
 * \brief Called for every OFDM symbol to apply the channel estimation and correction
 *
 * @param input frequency domain representation (53 values) of the OFDM symbol
 * @param symbolNo 0 for SIGNAL, 1..n for the rest of the OfDM symbols
 * @return corrected samples
 */
itpp::cvec
PhySimChannelEstimator::ApplyOFDMSymbolCorrection (const itpp::cvec &input, uint32_t symbolNo)
{
  // Dummy behavior: simply return input
  return input;
}

void
PhySimChannelEstimator::SetDetectedNoise (const double estN0)
{
  m_N0 = estN0;
}

/*
 * \brief Returns an estimate of the spectral density of the AWGN noise.
 */
double
PhySimChannelEstimator::GetDetectedNoise ()
{
  return m_N0;
}

/*
 * \brief CReset the estimator.
 */
void
PhySimChannelEstimator::Reset ()
{
}

/*
 * \brief Returns the pilots contained in the freq. domain representation.
 */
itpp::cvec
PhySimChannelEstimator::GetPilots (const itpp::cvec& input)
{
  NS_ASSERT ( input.size () == 53 );
  itpp::cvec pilotsToReturn (4);
  pilotsToReturn (0) = input (5);         // subcarrier index -21
  pilotsToReturn (1) = input (19);        // subcarrier index -7
  pilotsToReturn (2) = input (33);        // subcarrier index 7
  pilotsToReturn (3) = input (47);        // subcarrier index 21
  return pilotsToReturn;
}

NS_OBJECT_ENSURE_REGISTERED (PhySimSimpleChannelEstimator);

TypeId
PhySimSimpleChannelEstimator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimSimpleChannelEstimator")
    .SetParent<PhySimChannelEstimator> ()
    .AddConstructor<PhySimSimpleChannelEstimator> ();
  return tid;
}

PhySimSimpleChannelEstimator::PhySimSimpleChannelEstimator ()
{
}

PhySimSimpleChannelEstimator::~PhySimSimpleChannelEstimator ()
{
}

double
PhySimSimpleChannelEstimator::GetInitialChannelEstimation (const itpp::cvec &input)
{
  return 0.0;
}

itpp::cvec
PhySimSimpleChannelEstimator::ApplyEstimateFromTrainingSequence (const itpp::cvec &input,
                                                                 double estimate,
                                                                 int phaseOffset)
{
  return input;
}

itpp::cvec
PhySimSimpleChannelEstimator::ApplyOFDMSymbolCorrection (const itpp::cvec &input, uint32_t symbolNo)
{
  // Get reference Pilot Subcarrier
  itpp::cvec pilots = PhySimOFDMSymbolCreator::GetPilotSubcarrier (symbolNo);
  itpp::cvec actualPilots = GetPilots (input);

  // Calculate ratio between pilots and reference
  itpp::cvec ratio = itpp::elem_div (pilots, actualPilots);

  // Interpolate between them
  itpp::cvec linInterp = itpp::lininterp (ratio, 17); // get 52 elements.
  NS_ASSERT ( linInterp.size () == 52 );
  linInterp.ins (26, 1); // The DC is at position 26

  itpp::cvec result = itpp::elem_mult (input, linInterp);

  return result;
}

void
PhySimSimpleChannelEstimator::Reset ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

NS_OBJECT_ENSURE_REGISTERED (PhySimChannelFrequencyOffsetEstimator);

TypeId
PhySimChannelFrequencyOffsetEstimator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimChannelFrequencyOffsetEstimator")
    .SetParent<PhySimChannelEstimator> ()
    .AddConstructor<PhySimChannelFrequencyOffsetEstimator> ()
  ;
  return tid;
}

PhySimChannelFrequencyOffsetEstimator::PhySimChannelFrequencyOffsetEstimator () : m_channelGains (4)
{
  m_channelGains.zeros ();
  ofdm.set_parameters (64,0,1);
}

PhySimChannelFrequencyOffsetEstimator::~PhySimChannelFrequencyOffsetEstimator ()
{
}

/*!
 * Takes in an time samples vector including short (ST) and long (LT) training sequence and returns
 * an offset correction factor to be applied to the reference vector by multiplying by exp(-jna),
 * where n=128,... and a is the correction factor.
 * Input vector must be more than or equal to 192 samples long (two STs + 160 for the LT).
 *
 * Care must be taken to ensure that the input makes sense, so the following must be true
 * - endOfST must be >=32 and should divisable by 16 (otherwise we ignore first few values until it is)
 * - the beginOfLT + 160 +1 marks the beginning of the SIGNAL header. From then on there should be an extra 80 samples
 * - the rest should be divisable by 80. Otherwise there are too many samples and we ignore the last ones
 *
 * @param input time samples
 * @return a correction factor a, which can be applied to the original samples
 */
double
PhySimChannelFrequencyOffsetEstimator::GetInitialChannelEstimation (const itpp::cvec &input)
{
  int32_t beginOfLT = input.size () - 160;

  // Note: normally, we would do a NS_ASSERT here, but since we do not want
  // to stop our simulation run in such a case, we simply return a value that
  // won't correct anything here...
  if ( beginOfLT <= 31 || input.size () < 192 )
    {
      return 0.0; // results in a factor of 1.0 in exp(-jna), where a = 0.0
    }

  uint32_t endOfShortSymbols = beginOfLT - 1;
  uint32_t numSequences = (endOfShortSymbols + 1) / 16; // estimate how many STs there are

  NS_LOG_DEBUG ("PhySimChannelFrequencyOffsetEstimator:GetInitialChannelEstimation() input.size = " << input.size () << " beginOfLT = " << beginOfLT << " numSequences = " << numSequences);

  // Calculate offset to start from. this is chosen so that the last short training sequence ends
  // exactly before the beginOfLT
  uint32_t startOfShortSymbols = (endOfShortSymbols + 1) - (numSequences * 16);

  NS_LOG_DEBUG ("PhySimChannelFrequencyOffsetEstimator:GetInitialChannelEstimation()  --> startOfShortSymbols = " << startOfShortSymbols);
  NS_LOG_DEBUG ("PhySimChannelFrequencyOffsetEstimator:GetInitialChannelEstimation()  --> endOfShortSymbols = " << endOfShortSymbols);

  double coarseEstimation = InitialCoarseOffsetEstimation ( input (startOfShortSymbols, endOfShortSymbols) );
  NS_LOG_DEBUG ("PhySimChannelFrequencyOffsetEstimator:GetInitialChannelEstimation()  --> coarseEstimation = " << coarseEstimation);
  itpp::cvec correctedLT = correctLT (input (beginOfLT + 32, beginOfLT + 160 - 1), coarseEstimation);
  double fineEstimation = InitialFineOffsetEstimation (correctedLT);
  NS_LOG_DEBUG ("PhySimChannelFrequencyOffsetEstimator:GetInitialChannelEstimation()  --> fineEstimation = " << fineEstimation);
  // find out what the channel gains are
  double TSoffsetEstimator = coarseEstimation + fineEstimation;
  correctedLT = correctLT (correctedLT, TSoffsetEstimator); // second correction

  // demodulate the ofdm
  itpp::cvec firstTrSeq = PhySimOFDMSymbolCreator::TransformToInput (ofdm.demodulate (correctedLT.mid (0, 64)));
  itpp::cvec secondTrSeq = PhySimOFDMSymbolCreator::TransformToInput (ofdm.demodulate (correctedLT.mid (64, 64)));
  itpp::cvec pilotsFirst = GetPilots (firstTrSeq);
  itpp::cvec pilotsSecond = GetPilots (secondTrSeq);
  m_channelGains = (pilotsFirst + pilotsSecond) / 2;
  return TSoffsetEstimator;
}

/*!
 * \brief Applies the Training Sequence Estimate (if not calculated returns itpp::cvec unchanged)
 * Returns a itpp::cvec with the estimate from the training sequence applied. This function should only be called
 * twice: Once for the training sequence and once for the actual OFDM blocks.
 *
 * @param input         the complex samples to work on
 * @param estimate      the frequency offset estimation from the training sequence
 * @param phaseOffset   indicates whether we are already in the data symbols or still in the training sequence
 *                      0 = still in training sequence, 1 = input contains data symbols
 * @return corrected time samples
 */
itpp::cvec
PhySimChannelFrequencyOffsetEstimator::ApplyEstimateFromTrainingSequence (const itpp::cvec &input,
                                                                          double estimate,
                                                                          int phaseOffset)
{
  itpp::cvec correctedSamples (input);
  uint32_t pOffset = 128 + phaseOffset * 80;
  double offSetWithPhase;
  for (int32_t i = 0; i < input.size (); ++i)
    {
      offSetWithPhase = estimate * (i + pOffset);
      correctedSamples (i) *= std::complex<double> (cos (offSetWithPhase), -1 * sin (offSetWithPhase));
    }
  return correctedSamples;
}

itpp::cvec
PhySimChannelFrequencyOffsetEstimator::ApplyOFDMSymbolCorrection (const itpp::cvec &input, uint32_t symbolNo)
{
  // Correct Residual Carrier Frequency Offset
  itpp::cvec residualCarrierFreqOffsetCorrected = CorrectResidualCarrierFreqOffset (input, symbolNo);

  itpp::cvec pilots = PhySimChannelEstimator::GetPilots (residualCarrierFreqOffsetCorrected);

  // Get reference Pilot Subcarrier
  itpp::cvec refpilots = PhySimOFDMSymbolCreator::GetPilotSubcarrier (symbolNo);

  // Calculate ratio between pilots and reference
  itpp::cvec ratio = itpp::elem_div (refpilots, pilots);

  // Interpolate between them
  itpp::cvec linInterp = itpp::lininterp (ratio, 17); // get 52 elements.
  NS_ASSERT ( linInterp.size () == 52 );
  linInterp.ins (26, 1); // The DC is at position 26

  itpp::cvec result = itpp::elem_mult (residualCarrierFreqOffsetCorrected, linInterp);

  return result;
}

void
PhySimChannelFrequencyOffsetEstimator::Reset ()
{
}

double
PhySimChannelFrequencyOffsetEstimator::InitialFineOffsetEstimation (const itpp::cvec &input)
{
  NS_ASSERT ( input.size () == 128 );

  std::complex<double> sum = 0;

  for (uint32_t i = 0; i < 64; ++i)
    {
      sum += std::conj (input (i)) * input (i + 64);
    }

  return (std::arg (sum) / 64);
}

/*!
 * Takes in a short training sequence (of length at least 32 samples) and returns a
 * coarse frequency offset estimation. This is equation (4) in [Sourour04].
 *
 * @param input the short training sequence
 * @return frequency offset estimation
 */
double
PhySimChannelFrequencyOffsetEstimator::InitialCoarseOffsetEstimation (const itpp::cvec &input)
{
  // Note: normally we would do an NS_ASSERT here, but since this will break our
  // algorithm/simulation run, we simply return with 0.0 if the assertion is not
  // fulfilled.
  // NS_ASSERT( input.size() >= 32 && ( input.size() % 16 == 0) );
  if ( input.size () < 32 || ( input.size () % 16 != 0) )
    {
      NS_LOG_WARN ("Initial coarse offset estimation error!");
      return 0.0;
    }

  NS_LOG_DEBUG ("PhySimChannelFrequencyOffsetEstimator::InitialCoarseOffsetEstimation() input.size() = " << input.size ());

  std::complex<double> sum = 0;
  // uint32_t numSequences = input.size() / 16;
  uint32_t endIndex = input.size () - 17;
  NS_LOG_DEBUG ("PhySimChannelFrequencyOffsetEstimator::InitialCoarseOffsetEstimation() endIndex = " << endIndex);

  for (uint32_t i = 0; i <= endIndex; ++i)
    {
      NS_LOG_DEBUG ("PhySimChannelFrequencyOffsetEstimator::InitialCoarseOffsetEstimation() i = " << i);
      sum += std::conj (input (i)) * input (i + 16);
    }
  return arg (sum) / 16.0;
}

itpp::cvec
PhySimChannelFrequencyOffsetEstimator::CorrectResidualCarrierFreqOffset (const itpp::cvec &input, uint32_t symbolNo)
{
  NS_ASSERT ( input.size () == 53 );
  double estimator = CalculateResidualCarrierFreqOffset (input, symbolNo);
  if (isnan (estimator))  // if estimator is not valid then return original input
    {
      NS_LOG_WARN ("PhySimChannelFrequencyOffsetEstimator::CorrectResidualCarrierFreqOffset() - Estimator not valid");
      return input;
    }
  NS_LOG_DEBUG ("PhySimChannelFrequencyOffEstimator::CorrectResidualCarrierFreqOffset() - Applying frequency offsetEst: " << estimator);
  itpp::cvec output (input.size ());

  for (int32_t i = 0; i < input.size (); ++i)
    {
      // The paper says not to touch the pilots but we do not implement steps 1 and 3 in section 4
      // so we need to correct the pilots as well as there is no further processing to be done on them
      output (i) = input (i) * std::complex<double> (cos (estimator), sin (-1 * estimator));
    }

  return output;
}

double
PhySimChannelFrequencyOffsetEstimator::CalculateResidualCarrierFreqOffset (const itpp::cvec &input, uint32_t symbolNo)
{
  NS_ASSERT ( input.size () == 53 );
  itpp::cvec refpilots = PhySimOFDMSymbolCreator::GetPilotSubcarrier (symbolNo);
  itpp::cvec recpilots = PhySimChannelEstimator::GetPilots (input);
  itpp::cvec Pls = itpp::elem_mult (recpilots, refpilots);

  std::complex<double> sum = 0;
  for (uint32_t i = 0; i < 4; ++i)
    {
      sum += Pls (i) * std::conj (m_channelGains (i));
    }
  return std::arg (sum);
}

/*
 * \brief Applies a correction factor to the LT (without the GI) and returns the corrected LT
 */
itpp::cvec
PhySimChannelFrequencyOffsetEstimator::correctLT (const itpp::cvec LT, const double correction)
{
  NS_ASSERT (LT.size () == 128);
  itpp::cvec realInput = LT;
  for (int i = 0; i < realInput.size (); ++i)
    {
      realInput (i) *= std::complex<double> (cos (i * correction), sin (-i
                                                                        * correction));
    }
  return realInput;
}

itpp::cvec
PhySimChannelFrequencyOffsetEstimator::GetPilots (const itpp::cvec &input)
{
  NS_ASSERT ( input.size () == 53 );
  itpp::cvec pilotsToReturn (4);
  pilotsToReturn (0) = input (5); // subcarrier index -21
  pilotsToReturn (1) = input (19); // subcarrier index -7
  pilotsToReturn (2) = input (33); // subcarrier index 7
  pilotsToReturn (3) = input (47); // subcarrier index 21
  return pilotsToReturn;
}
} // namespace ns3
