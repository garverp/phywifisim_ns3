/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009-2010 Stylianos Papanastasiou, Jens Mittag
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
 * Stylianos Papanastasiou <stylianos@gmail.com>
 * Jens Mittag <jens.mittag@kit.edu>
 *
 */


#include "physim-signal-detector.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/boolean.h"
#include "ns3/double.h"

NS_LOG_COMPONENT_DEFINE ("PhySimSignalDetector");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PhySimSignalDetector);

TypeId
PhySimSignalDetector::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimSignalDetector")
    .SetParent<Object> ()
    .AddConstructor<PhySimSignalDetector> ()
    .AddAttribute ("IEEECompliantMode",
                   "Flag indicating whether we are operating in IEEE compliant mode, which means we have (de-)normalized the in-/outputs",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PhySimSignalDetector::m_ieeeCompliantMode),
                   MakeBooleanChecker ())
    .AddAttribute ("UseAutoCorrelationMethod",
                   "Flag indicating whether to use auto-correlation method for short preamble detection or rather correlation against the known sequence of time samples",
                   BooleanValue (true),
                   MakeBooleanAccessor (&PhySimSignalDetector::m_autoCorrelation),
                   MakeBooleanChecker ())
    .AddAttribute ("CorrelationThreshold",
                   "Minimum correlation between signals to assume that there is a frame",
                   DoubleValue (0.85),
                   MakeDoubleAccessor (&PhySimSignalDetector::m_corrThresh),
                   MakeDoubleChecker<double> ());
  return tid;
}

PhySimSignalDetector::PhySimSignalDetector ()
{
}

PhySimSignalDetector::~PhySimSignalDetector ()
{

}

bool
PhySimSignalDetector::DetectPreamble (Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag, const itpp::cvec &input)
{
  NS_LOG_FUNCTION (packet << tag);
  NS_LOG_DEBUG ("PhySimSignalDetector:DetectPreamble() input.size() = " << input.size());

  // First check whether we detect the short symbols at all
  int32_t beginShortSymbols = ScanForShortTrainingSymbols (tag, input);
  NS_LOG_DEBUG ("PhySimSignalDetector:DetectPreamble() beginShortSymbols = " << beginShortSymbols);
  tag->SetShortSymbolStart(beginShortSymbols);
  int32_t beginLongSymbols;
  if ( beginShortSymbols >= 0 &&
       (input.size() - 1 - beginShortSymbols - 80) >= 64 )  // The second condition is necessary to avoid
                                                            // assertion errors in ScanForLongTrainingSymbols() later
    {
      // If we were successful, we try to perform finer time synchronization using the long training symbols
      // Note: due to the guard interval, the time sample number has to be reduced by 32.
      beginLongSymbols = ScanForLongTrainingSymbols (tag, input(beginShortSymbols + 80, input.size() - 1));
      beginLongSymbols = beginLongSymbols - 32 + 80;

      NS_LOG_DEBUG ("PhySimSignalDetector:DetectPreamble() beginLongSymbols = " << beginLongSymbols);

      // Save the information inside of the tag
      tag->SetLongSymbolStart (beginLongSymbols);

      // If the detected times make sense (and won't lead to out of bound vector accesses)
      if (beginLongSymbols + 160 <= input.size ())
        {
          return true;
        }
    }

  // If we get here, signal detection failed
  return false;
}

int32_t
PhySimSignalDetector::ScanForShortTrainingSymbols (Ptr<PhySimWifiPhyTag> tag, const itpp::cvec &input)
{
  NS_LOG_FUNCTION (tag);
  NS_ASSERT (input.size () >= 48);
  NS_LOG_DEBUG ("PhySimSignalDetector:ScanForShortTrainingSymbols() correlation threshold = " << m_corrThresh);
  int32_t begin = -1;

  // Define some helper variables
  uint32_t scanWindowSize;

  // Configure ourselves, depending on the selected correlation method
  if (m_autoCorrelation)
    {
      NS_LOG_DEBUG ("PhySimSignalDetector:ScanForShortTrainingSymbols() using auto-correlation method");
      scanWindowSize = 48;
    }
  else
    {
      NS_LOG_DEBUG ("PhySimSignalDetector:ScanForShortTrainingSymbols() using correlation against known values");
      scanWindowSize = 16;
    }

  // Define some further helper variables
  itpp::vec norminput (input.size());
  for (int i = 0; i < input.size(); ++i)
    {
      norminput(i) = std::norm (input(i));
    }


  // Collect correlation values to store them in the tag later
  std::vector<double> correlations;

  // Perform the actual correlation scan
  double value;
  int32_t start = 0;
  int32_t end = start + (scanWindowSize - 1);
  int32_t max = input.size () - scanWindowSize;

  while (start <= max && end < input.size ())
    {
      //NS_LOG_DEBUG ("PhySimSignalDetector:ScanForShortTrainingSymbols() start = " << start);
      //NS_LOG_DEBUG ("PhySimSignalDetector:ScanForShortTrainingSymbols() end = " << end);
      //NS_LOG_DEBUG ("PhySimSignalDetector:ScanForShortTrainingSymbols() max = " << max);

      // Calculate the correlation for this sample window
      if (m_autoCorrelation)
        {
          itpp::cvec window = input(start, end);
          itpp::vec normalized = norminput(start+16, end);
          value = ComputeAutoCorrelation (window, normalized, scanWindowSize);
        }
      else
        {
          itpp::cvec window = input(start, end);
          itpp::vec normalized = norminput(start, end);
          value = ComputeCorrelation (window, normalized, scanWindowSize);
        }

      NS_LOG_DEBUG ("PhySimSignalDetector:ScanForShortTrainingSymbols() --> correlation(" << start << ") = " << value);

      // Save correlation value for possible later usage
      correlations.push_back (value);

      // Update begin variable in case the threshold is exceeded for the first time
      if (value >= m_corrThresh)
        {
          if (begin < 0)
            {
              NS_LOG_DEBUG ("PhySimSignalDetector:ScanForShortTrainingSymbols() Setting new begin value.");
              begin = start;
            }
        }

      start++;
      end++;
    }
  tag->SetShortTrainingSymbolCorrelations (correlations);

  return begin;
}

int32_t
PhySimSignalDetector::ScanForLongTrainingSymbols (Ptr<PhySimWifiPhyTag> tag, const itpp::cvec &input)
{
  NS_LOG_FUNCTION (tag);
  // The input should be at least 64 samples long for proper detection of long training symbols
  NS_ASSERT (input.size() >= 64);

  // Perform the scan
  uint32_t maxPosFirst = -1;
  uint32_t maxPosSecond = -1;
  double value;
  double maxSeenFirst = 0;
  double maxSeenSecond = 0;

  itpp::vec norminput (input.size ());
  for (int32_t i = 0; i < input.size (); i++)
    {
      norminput(i) = std::norm (input(i));
    }

  // collect correlation values for the TraceSource 'm_longSymbolsTrace'
  std::vector<double> correlations;

  int32_t start = 0;
  int32_t end = start + 63;

  while ( (start <= input.size () - 64) && (end < input.size ()) )
    {

      NS_LOG_DEBUG ("PhySimSignalDetector:ScanForLongTrainingSymbols() start = " << start);
      NS_LOG_DEBUG ("PhySimSignalDetector:ScanForLongTrainingSymbols() end = " << end);

      // Calculate correlation
      itpp::cvec window = input (start, end);
      itpp::vec normalized = norminput (start, end);
      value = ComputeCorrelation (window, normalized, 64);

      NS_LOG_DEBUG ("PhySimSignalDetector:ScanForLongTrainingSymbols() --> correlation(" << start << ") = " << value);

      // Save correlation value for possible later usage
      correlations.push_back (value);

      if (value > maxSeenFirst)
        { // new global maximum
          maxSeenSecond = maxSeenFirst;
          maxPosSecond = maxPosFirst;   // make the old maximum the second highest
          maxSeenFirst = value;
          maxPosFirst = start;          // record position and value of new maximum
        }
      else if (value > maxSeenSecond)
        { // new second maximum
          maxSeenSecond = value;
          maxPosSecond = start;
        }

      // Update counters
      start++;
      end++;
    }

  tag->SetLongTrainingSymbolCorrelations (correlations);

  // TODO: There is probably some checks to be done w.r.t. the distance between the two peaks but for now this will do
  if (maxSeenFirst == -1)
    {
      return 0; // if we have not found anything
    }
  if (maxSeenSecond == -1)
    {
      return maxPosFirst; // if no second best recorded then go with first guess

    }
  return ((maxPosFirst > maxPosSecond) ? maxPosSecond : maxPosFirst);
}

/*!
 * \brief Returns correlation for a given input using method 1 (auto-correlation).
 *
 */
double PhySimSignalDetector::ComputeAutoCorrelation (const itpp::cvec& input, const itpp::vec& norminput, const int32_t window)
{
  double L = 32;
  std::complex<double> sum = 0; // The moving sum

  // Calculate upper branch: C_n
  uint32_t N = input.size() - 1;
  for (int32_t k = 0; k < L; ++k)
    {
      sum += ( input(N-k) * conj( input(N-k-16) ) );
    }

  // Calculate lower branch
  double sumConj = 0;
  for (int32_t i = 0; i < norminput.size(); ++i)
    {
      sumConj += norminput (i);
    }

  // Now divide the two to come up with a correlation
  return std::abs (sum) / sumConj;
}

/*!
 * \brief Returns correlation for a given input using method 2 (expected values).
 * Note that for this method the input must be 16 or 64 samples long
 * which means short and long training sequences respectively.
 */
double PhySimSignalDetector::ComputeCorrelation (const itpp::cvec& input, const itpp::vec& norminput, const int32_t window)
{
  NS_ASSERT ( input.size () == window );

  const itpp::cvec* refSymbols;

  if (window == 16)
    {
      if (m_ieeeCompliantMode)
        {
          refSymbols = &PhySimHelper::m_refConjShortSymbolCompleteIEEE;
        }
      else
        {
          refSymbols = &PhySimHelper::m_refConjShortSymbolComplete;
        }

    }
  else if (window == 64)
    {
      if (m_ieeeCompliantMode)
        {
          refSymbols = &PhySimHelper::m_refConjLongSymbolCompleteIEEE;
        }
      else
        {
          refSymbols = &PhySimHelper::m_refConjLongSymbolComplete;
        }

    }
  else
    {
      NS_LOG_DEBUG ("PhySimSignalDetector:ComputeCorrelation() wrong window size");
      return 0;
    }

  std::complex<double> sum = 0; // the moving sum
  double sumConj = 0;

  for (int32_t i = 0; i < window; i++)
    {
      sumConj += norminput (i);
    }

  double normFactor = sqrt (sumConj);
  for (int32_t k = 0; k < window; k++)
    {
      sum += input (k) * (*refSymbols)(k); // itpp::conj(refSymbols(k));
    }

  NS_LOG_DEBUG ("PhySimSignalDetector:ComputeCorrelation() --> sumConj = " << sumConj);
  NS_LOG_DEBUG ("PhySimSignalDetector:ComputeCorrelation() --> sum = " << sum);

  sum *= normFactor; // apply normalization
  return abs (sum) / sumConj;
}
} // namespace ns3
