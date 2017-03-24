/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA, 2009-2010 Stylianos Papanastasiou, Jens Mittag
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
 * Based on propagation-loss-model.cc by
 *      Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Major Modifications:
 *      Jens Mittag <jens.mittag@kit.edu>
 *      Stylianos Papanastasiou <stylianos@gmail.com>
 */

#include "physim-propagation-loss-model.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"

NS_LOG_COMPONENT_DEFINE ("PhySimPropagationLossModel");

namespace ns3 {

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimPropagationLossModel);

TypeId
PhySimPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimPropagationLossModel")
    .SetParent<Object> ()
    .AddConstructor<PhySimPropagationLossModel> ();
  return tid;
}

PhySimPropagationLossModel::PhySimPropagationLossModel ()
  : m_next (0)
{
}

PhySimPropagationLossModel::~PhySimPropagationLossModel ()
{
}

void
PhySimPropagationLossModel::SetNext (Ptr<PhySimPropagationLossModel> next)
{
  m_next = next;
}

void
PhySimPropagationLossModel::CalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<
                                           MobilityModel> a, Ptr<MobilityModel> b) const
{
  DoCalcRxPower (tag, a, b);
  if (m_next != 0)
    {
      m_next->CalcRxPower (tag, a, b);
    }
}

void
PhySimPropagationLossModel::DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<
                                             MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_DEBUG ("PhySimPropagationLossModel::DoCalcRxPower() will do nothing, I'm dummy only.");
  return;
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimFriisSpacePropagationLoss);

TypeId
PhySimFriisSpacePropagationLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimFriisSpacePropagationLoss")
    .SetParent<PhySimPropagationLossModel> ()
    .AddConstructor<PhySimFriisSpacePropagationLoss> ()
    .AddAttribute ("SystemLoss",
                   "The system loss factor factor",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&PhySimFriisSpacePropagationLoss::m_systemLoss),
                   MakeDoubleChecker<double> ());
  return tid;
}

PhySimFriisSpacePropagationLoss::PhySimFriisSpacePropagationLoss ()
  : PhySimPropagationLossModel (),
    m_speedOfLight (299792458.0)
{
}

PhySimFriisSpacePropagationLoss::~PhySimFriisSpacePropagationLoss ()
{
}

void
PhySimFriisSpacePropagationLoss::DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag,
                                                Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double m_lambda = m_speedOfLight / tag->GetFrequency ();
  double m_distance = a->GetDistanceFrom (b);
  NS_LOG_DEBUG ("PhySimFriisSpacePropagationLoss::DoCalcRxPower() m_lambda = " << m_lambda << " m_distance = " << m_distance);
  if (m_distance <= 0)
    {
      return;
    }

  double numerator = m_lambda * m_lambda;
  double denominator = 16 * M_PI * M_PI * m_distance * m_distance
    * m_systemLoss;
  double m_pathLoss = 10 * log10 (numerator / denominator);
  NS_LOG_DEBUG ("PhySimFriisSpacePropagationLoss::DoCalcRxPower() m_pathLoss = " << m_pathLoss);

  itpp::cvec input = tag->GetRxedSamples ();
  itpp::cvec output = input * sqrt (pow (10, m_pathLoss / 10.0));
  tag->SetPathLoss (m_pathLoss);
  tag->SetRxSamples (output);
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimTwoRayGroundPropagationLossModel);

TypeId
PhySimTwoRayGroundPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimTwoRayGroundPropagationLossModel")
    .SetParent<PhySimPropagationLossModel> ()
    .AddConstructor<PhySimTwoRayGroundPropagationLossModel> ()
    .AddAttribute ("SystemLoss",
                   "The system loss factor factor",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&PhySimTwoRayGroundPropagationLossModel::m_systemLoss),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("HeightAboveZ",
	           "The height of the antenna (m) above the node's Z coordinate",
	           DoubleValue (1.5),
		   MakeDoubleAccessor (&PhySimTwoRayGroundPropagationLossModel::m_heightAboveZ),
		   MakeDoubleChecker<double> ())
    .AddAttribute ("SpeedLight",
		   "The speed of light (in m/s)",
		   DoubleValue (299792458.0),
		   MakeDoubleAccessor (&PhySimTwoRayGroundPropagationLossModel::m_speedOfLight),
		   MakeDoubleChecker<double> ());
  return tid;
}

PhySimTwoRayGroundPropagationLossModel::PhySimTwoRayGroundPropagationLossModel ()
  : PhySimPropagationLossModel ()
{
}

PhySimTwoRayGroundPropagationLossModel::~PhySimTwoRayGroundPropagationLossModel ()
{
}

void
PhySimTwoRayGroundPropagationLossModel::DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag,
                                                Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double m_lambda = m_speedOfLight / tag->GetFrequency();
  double m_distance = a->GetDistanceFrom (b);

  NS_LOG_DEBUG ("PhySimTwoRayGroundPropagationLossModel::DoCalcRxPower() m_lambda = " << m_lambda << " m_distance = " << m_distance);
  if (m_distance <= 0)
    {
      return;
    }

  double m_pathLoss, numerator, denominator;

  // Set the height of the Tx and Rx antennas
  double txAntHeight = a->GetPosition ().z + m_heightAboveZ;
  double rxAntHeight = b->GetPosition ().z + m_heightAboveZ;


  double dCross = (4 * M_PI * txAntHeight * rxAntHeight) / m_lambda;
  if (m_distance <= dCross)
    {		//Use Friis
	  numerator = m_lambda * m_lambda;
	  denominator = 16 * M_PI * M_PI * m_distance * m_distance * m_systemLoss;
	  m_pathLoss = 10 * log10 (numerator / denominator);
    }
  else
    {	// Use Two-Ray
      numerator = txAntHeight * txAntHeight * rxAntHeight * rxAntHeight;
      denominator = m_distance * m_distance * m_distance * m_distance * m_systemLoss;
      m_pathLoss = 10 * log10 (numerator / denominator);
    }
  NS_LOG_DEBUG ("PhySimTwoRayGroundPropagationLossModel::DoCalcRxPower() m_pathLoss = " << m_pathLoss);

  itpp::cvec input = tag->GetRxedSamples ();
  itpp::cvec output = input * sqrt (pow (10, m_pathLoss / 10.0));
  tag->SetPathLoss (m_pathLoss);
  tag->SetRxSamples (output);

}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimConstantPropagationLoss);

TypeId
PhySimConstantPropagationLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimConstantPropagationLoss")
    .SetParent<PhySimPropagationLossModel> ()
    .AddConstructor<PhySimConstantPropagationLoss> ()
    .AddAttribute ("PathLoss",
                   "The constant path loss which is applied to the link in dB",
                   DoubleValue (20.0),
                   MakeDoubleAccessor (&PhySimConstantPropagationLoss::m_pathLoss),
                   MakeDoubleChecker<double> ());
  return tid;
}

PhySimConstantPropagationLoss::PhySimConstantPropagationLoss ()
  : PhySimPropagationLossModel ()
{
}

PhySimConstantPropagationLoss::~PhySimConstantPropagationLoss ()
{
}

void
PhySimConstantPropagationLoss::SetPathLoss (double loss)
{
  m_pathLoss = loss;
}

void
PhySimConstantPropagationLoss::DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<
                                                MobilityModel> a, Ptr<MobilityModel> b) const
{
  NS_LOG_DEBUG ("PhySimConstantPropagationLoss::DoCalcRxPower() m_pathLoss = " << m_pathLoss);
  itpp::cvec input = tag->GetRxedSamples ();
  itpp::cvec output = input * sqrt (pow (10, (-1.0) * m_pathLoss / 10.0));
  tag->SetPathLoss (m_pathLoss);
  tag->SetRxSamples (output);
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimLogDistancePropagationLoss);

TypeId
PhySimLogDistancePropagationLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimLogDistancePropagationLoss")
    .SetParent<PhySimPropagationLossModel> ()
    .AddConstructor<PhySimLogDistancePropagationLoss> ()
    .AddAttribute ("Exponent",
                   "The exponent of the Path Loss propagation model",
                   DoubleValue (3.0),
                   MakeDoubleAccessor (&PhySimLogDistancePropagationLoss::m_exponent),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ReferenceDistance",
                   "The distance at which the reference loss is calculated (m)",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&PhySimLogDistancePropagationLoss::m_referenceDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ReferenceLoss",
                   "The reference loss at reference distance (dB). (Default is Friis at 1m with 5.15 GHz)",
                   DoubleValue (46.6777),
                   MakeDoubleAccessor (&PhySimLogDistancePropagationLoss::m_referenceLoss),
                   MakeDoubleChecker<double> ());
  return tid;
}

PhySimLogDistancePropagationLoss::PhySimLogDistancePropagationLoss ()
  : PhySimPropagationLossModel ()
{
}

PhySimLogDistancePropagationLoss::~PhySimLogDistancePropagationLoss ()
{
}

void
PhySimLogDistancePropagationLoss::DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag,
                                                 Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double m_distance = a->GetDistanceFrom (b);
  double m_pathLoss = m_referenceLoss + 10 * m_exponent * log10 (m_distance / m_referenceDistance);
  m_pathLoss = (-1.0) * m_pathLoss;
  NS_LOG_DEBUG ("PhySimLogDistancePropagationLoss::DoCalcRxPower() m_pathLoss = " << m_pathLoss);

  itpp::cvec input = tag->GetRxedSamples ();
  itpp::cvec output = input * sqrt (pow (10, m_pathLoss / 10.0));
  tag->SetPathLoss (m_pathLoss);
  tag->SetRxSamples (output);
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimThreeLogDistancePropagationLoss);

TypeId
PhySimThreeLogDistancePropagationLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimThreeLogDistancePropagationLoss")
    .SetParent<PhySimPropagationLossModel> ()
    .AddConstructor<PhySimThreeLogDistancePropagationLoss> ()
    .AddAttribute ("Exponent0",
                   "The exponent for the first field.",
                   DoubleValue (1.9),
                   MakeDoubleAccessor (&PhySimThreeLogDistancePropagationLoss::m_exponent0),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Exponent1",
                   "The exponent for the second field.",
                   DoubleValue (3.8),
                   MakeDoubleAccessor (&PhySimThreeLogDistancePropagationLoss::m_exponent1),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Exponent2",
                   "The exponent for the third field.",
                   DoubleValue (3.8),
                   MakeDoubleAccessor (&PhySimThreeLogDistancePropagationLoss::m_exponent2),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Distance0",
                   "Beginning of the first (near) distance field",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&PhySimThreeLogDistancePropagationLoss::m_distance0),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Distance1",
                   "Beginning of the second (middle) distance field",
                   DoubleValue (200.0),
                   MakeDoubleAccessor (&PhySimThreeLogDistancePropagationLoss::m_distance1),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Distance2",
                   "Beginning of the third (far) distance field",
                   DoubleValue (500.0),
                   MakeDoubleAccessor (&PhySimThreeLogDistancePropagationLoss::m_distance2),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ReferenceLoss",
                   "The reference loss at distance d0 (dB). (Default is Friis at 1m with 5.15 GHz)",
                   DoubleValue (46.6777),
                   MakeDoubleAccessor (&PhySimThreeLogDistancePropagationLoss::m_referenceLoss),
                   MakeDoubleChecker<double> ());
  return tid;
}

PhySimThreeLogDistancePropagationLoss::PhySimThreeLogDistancePropagationLoss ()
  : PhySimPropagationLossModel ()
{
}

PhySimThreeLogDistancePropagationLoss::~PhySimThreeLogDistancePropagationLoss ()
{
}

void
PhySimThreeLogDistancePropagationLoss::DoCalcRxPower (
  Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double m_distance = a->GetDistanceFrom (b);
  double m_pathLoss;

  if (m_distance < m_distance0)
    {
      m_pathLoss = 0;
    }
  else if (m_distance < m_distance1)
    {
      m_pathLoss = m_referenceLoss + 10 * m_exponent0 * log10 (m_distance
                                                               / m_distance0);
    }
  else if (m_distance < m_distance2)
    {
      m_pathLoss = m_referenceLoss + 10 * m_exponent0 * log10 (m_distance1
                                                               / m_distance0) + 10 * m_exponent1 * log10 (m_distance / m_distance1);
    }
  else
    {
      m_pathLoss = m_referenceLoss + 10 * m_exponent0 * log10 (m_distance1
                                                               / m_distance0) + 10 * m_exponent1
        * log10 (m_distance2 / m_distance1) + 10 * m_exponent2 * log10 (
          m_distance / m_distance2);
    }

  m_pathLoss = (-1.0) * m_pathLoss;
  NS_LOG_DEBUG ("PhySimThreeLogDistancePropagationLoss::DoCalcRxPower() m_pathLoss = " << m_pathLoss);

  itpp::cvec input = tag->GetRxedSamples ();
  itpp::cvec output = input * sqrt (pow (10, m_pathLoss / 10.0));
  tag->SetPathLoss (m_pathLoss);
  tag->SetRxSamples (output);
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimShadowingPropagationLoss);

TypeId
PhySimShadowingPropagationLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimShadowingPropagationLoss")
    .SetParent<PhySimPropagationLossModel> ()
    .AddConstructor<PhySimShadowingPropagationLoss> ()
    .AddAttribute ("StandardDeviation",
                   "The standard deviation of the shadow fading (dB)",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&PhySimShadowingPropagationLoss::SetStandardDeviation,
                                       &PhySimShadowingPropagationLoss::GetStandardDeviation),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinimumDistance",
                   "The minimum distance to apply this to. With this it is possible to avoid shadowing at small distances.",
                   DoubleValue (50.0),
                   MakeDoubleAccessor (&PhySimShadowingPropagationLoss::m_minDistance),
                   MakeDoubleChecker<double> ());
  return tid;
}

PhySimShadowingPropagationLoss::PhySimShadowingPropagationLoss ()
  : PhySimPropagationLossModel (),
    m_normalVariable (0)
{
}

PhySimShadowingPropagationLoss::~PhySimShadowingPropagationLoss ()
{
}

void
PhySimShadowingPropagationLoss::SetStandardDeviation (double std)
{
  NS_LOG_FUNCTION ( this << std);
  m_stdDeviationDb = std;
  if (m_normalVariable)
    {
      delete m_normalVariable;
    }
  // create new normal variable with zero mean and variance of (standard deviation)^2
  m_normalVariable = new NormalVariable (0, (m_stdDeviationDb
                                             * m_stdDeviationDb));
}

double
PhySimShadowingPropagationLoss::GetStandardDeviation (void) const
{
  return m_stdDeviationDb;
}

void
PhySimShadowingPropagationLoss::DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<
                                                 MobilityModel> a, Ptr<MobilityModel> b) const
{
  double m_distance = a->GetDistanceFrom (b);
  if (m_distance < m_minDistance)
    {
      return;
    }
  double m_pathLoss = m_normalVariable->GetValue ();

  NS_LOG_DEBUG ("PhySimShadowingPropagationLoss::DoCalcRxPower() m_pathLoss = " << m_pathLoss);

  itpp::cvec input = tag->GetRxedSamples ();
  itpp::cvec output = input * sqrt (pow (10, m_pathLoss / 10.0));
  tag->SetRxSamples (output);
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimRicianPropagationLoss);

TypeId
PhySimRicianPropagationLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimRicianPropagationLoss")
    .SetParent<PhySimPropagationLossModel> ()
    .AddConstructor<PhySimRicianPropagationLoss> ()
    .AddAttribute ("MinimumRelativeSpeed",
                   "The minimum relative speed [m/s] we want to allow. Every calculated value below will be set to this value.",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&PhySimRicianPropagationLoss::m_minRelativeSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("LineOfSightPower",
                   "The relative LOS power to get Ricean Fading (defaults to 0, i.e. Rayleigh if not set)",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&PhySimRicianPropagationLoss::m_lineOfSightPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("UseShortcut",
                   "Use a computational shortcut for the creation of Rayleigh coefficients for each sample, with a slight accuracy loss.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PhySimRicianPropagationLoss::m_shortcut),
                   MakeBooleanChecker ())
    .AddAttribute ("LineOfSightDoppler",
                   "The relative LOS doppler (defaults to 0.7)", DoubleValue (0.7),
                   MakeDoubleAccessor (&PhySimRicianPropagationLoss::m_lineOfSightDoppler),
                   MakeDoubleChecker<double> ());
  return tid;
}

PhySimRicianPropagationLoss::PhySimRicianPropagationLoss ()
  : PhySimPropagationLossModel ()
{
  m_generator = new itpp::Rice_Fading_Generator (0.7, itpp::Jakes, 16);
}

PhySimRicianPropagationLoss::~PhySimRicianPropagationLoss ()
{
  delete (m_generator);
}

void
PhySimRicianPropagationLoss::DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<
                                              MobilityModel> a, Ptr<MobilityModel> b) const
{
  itpp::cvec input (tag->GetRxedSamples ());
  NS_LOG_DEBUG ("PhySimRicianPropagationLoss::DoCalcRxPower() input.size() = " << input.size ());
  itpp::cvec output (input.size ());
  itpp::cvec coeff (output.size ());
  double m_speed = a->GetRelativeSpeed (b); // relative speed in m/s
  if (m_speed < m_minRelativeSpeed)
    {
      m_speed = m_minRelativeSpeed;
    }
  NS_LOG_DEBUG ("PhySimRicianPropagationLoss::DoCalcRxPower() relativeSpeed = " << m_speed);
  if (m_speed == 0.0)
    {
      coeff = 1.0;
      output = coeff (0) * input;
    }
  else
    {
      double m_maxDoppler = CalcMaxDopplerFrequency (tag->GetFrequency (),
                                                     m_speed);
      double m_normDoppler = m_maxDoppler * tag->GetSampleDuration ();
      double product = m_normDoppler * output.size ();
      // IT++ requires at least 7 frequencies and a
      // normalized doppler greater than zero
      NS_ASSERT (m_normDoppler > 0);
      // Ni=max(fm*num.of.samples,7), the min value set as in Matthias Patzold, Mobile fading channels, Wiley, 2002, p. 128, fig. 5.24
      uint32_t m_noFrequencies = std::max (product, 7.0);
      NS_LOG_DEBUG ("PhySimRicianPropagationLoss::DoCalcRxPower() m_maxDoppler = " << m_maxDoppler);
      NS_LOG_DEBUG ("PhySimRicianPropagationLoss::DoCalcRxPower() m_normDoppler = " << m_normDoppler);
      NS_LOG_DEBUG ("PhySimRicianPropagationLoss::DoCalcRxPower() m_noFrequencies = " << m_noFrequencies);
      // The shortcut can be used as follows
      // If product is too small we don't have to generate different coefficients for each sample as the tap values
      // do not change significantly in time... we can get away with only calculating 1 coefficient value and use that
      // for all samples
      if (m_shortcut && (product < m_max_time_static)) // if spacing between frames is s.t. fm >1
        {
          itpp::Complex_Normal_RNG cn_rng;
          std::complex<double> coeff_factor = cn_rng.sample (); // cn_rng();
          coeff = coeff_factor;
          if (m_lineOfSightPower == 0.0) // no LOS
            {
              output = coeff_factor * input;
            }
          else
            {
              // if we have LOS for Ricean
              // add a los component to each coeff factor i
              // unfortunately we cannot reuse the add_LOS method in generator as it is protected
              double los_diffuse = std::sqrt (1 / (1 + m_lineOfSightPower));
              double los_direct = std::sqrt (m_lineOfSightPower / (1
                                                                   + m_lineOfSightPower));
              for (int i = 0; i < output.size (); ++i)
                {
                  add_LOS (m_lineOfSightDoppler, m_normDoppler, los_diffuse,
                           los_direct, i, coeff (i));
                  output (i) = coeff (i) * input (i);
                }
            }
        }
      else
        {
          UpdateGeneratorParameters (m_normDoppler, m_noFrequencies);
          if (m_lineOfSightPower > 0.0)
            {
              m_generator->set_LOS_power (m_lineOfSightPower);
            }
          coeff = m_generator->generate (input.size ());
          // Each channel profile impulse response corresponds to one time sample, so multiply and return result
          for (int i = 0; i < output.size (); ++i)
            {
              output (i) = coeff (i) * input (i);
            }
        }
    }NS_LOG_DEBUG ("PhySimRicianPropagationLoss::DoCalcRxPower() m_coeff = " << coeff);
  tag->SetRxSamples (output);
}

double
PhySimRicianPropagationLoss::CalcMaxDopplerFrequency (double frequency,
                                                      double speed) const
{
  return speed * frequency / 299792458.0; // (v * freq) / c0
}

void
PhySimRicianPropagationLoss::UpdateGeneratorParameters (double norm_doppler,
                                                        int nofrequencies) const
{
  m_generator->set_norm_doppler (norm_doppler);
  m_generator->set_no_frequencies (nofrequencies);
}

void
PhySimRicianPropagationLoss::add_LOS (double los_dopp, double n_dopp,
                                      double los_diffuse, double los_direct, int idx,
                                      std::complex<double>& sample) const
{
  double tmp_arg = itpp::m_2pi * los_dopp * n_dopp * (idx + 0); // no time offset here
  sample *= los_diffuse;
  sample += los_direct * std::complex<double> (std::cos (tmp_arg), std::sin (
                                                 tmp_arg));
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimTappedDelayLinePropagationLoss);

TypeId
PhySimTappedDelayLinePropagationLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimTappedDelayLinePropagationLoss")
    .SetParent<PhySimPropagationLossModel> ()
    .AddConstructor<PhySimTappedDelayLinePropagationLoss> ()
    .AddAttribute ("ChannelProfile",
                   "The channel profile which used for this TDL model.",
                   EnumValue (itpp::ITU_Vehicular_A),
                   MakeEnumAccessor (&PhySimTappedDelayLinePropagationLoss::SetChannelProfile,
                                     &PhySimTappedDelayLinePropagationLoss::GetChannelProfile),
                   MakeEnumChecker (
                     itpp::ITU_Vehicular_A, "ITU Vehicular A, 6-tap channel",
                     itpp::ITU_Vehicular_B, "ITU Vehicular B, 6-tap channel",
                     itpp::ITU_Pedestrian_A, "ITU Pedestrian A, 4-tap channel",
                     itpp::ITU_Pedestrian_B, "ITU Pedestrian B, 6-tap channel",
                     itpp::COST207_RA, "COST207_RA, Rural area, 4-tap channel",
                     itpp::COST207_RA6, "COST207_RA6, Rural area, 6-tap channel",
                     itpp::COST207_TU, "COST207_TU, Typical urban, 6-tap channel",
                     itpp::COST207_TU6alt, "COST207_TU6alt, Typical urban, alternative 6-tap channel",
                     itpp::COST207_TU12, "COST207_TU12, Typical urban, 12-tap channel",
                     itpp::COST207_TU12alt, "COST207_TU12alt, Typical urban, alternative 12-tap channel",
                     itpp::COST207_BU, "COST207_BU, Bad urban, 6-tap channel",
                     itpp::COST207_BU6alt, "COST207_BU6alt, Bad urban, alternative 6-tap channel",
                     itpp::COST207_BU12, "COST207_BU12, Bad urban, 12-tap channel",
                     itpp::COST207_BU12alt, "COST207_BU12alt, Bad urban, alternative 12-tap channel",
                     itpp::COST207_HT, "COST207_HT, Hilly terrain, 6-tap channel",
                     itpp::COST207_HT6alt, "COST207_HT6alt, Hilly terrain, alternative 6-tap channel",
                     itpp::COST207_HT12, "COST207_HT12, Hilly terrain, 12-tap channel",
                     itpp::COST207_HT12alt, "COST207_HT12alt, Hilly terrain, alternative 12-tap channel",
                     itpp::COST259_TUx, "COST259_TUx, Typical urban, 20-tap channel",
                     itpp::COST259_RAx, "COST259_RAx, Rural ara, 10-tap channel",
                     itpp::COST259_HTx, "COST259_HTx, Hilly terrain, 20-tap channel"));
  return tid;
}

PhySimTappedDelayLinePropagationLoss::PhySimTappedDelayLinePropagationLoss ()
  : PhySimPropagationLossModel ()
{
}

PhySimTappedDelayLinePropagationLoss::~PhySimTappedDelayLinePropagationLoss ()
{
}

void
PhySimTappedDelayLinePropagationLoss::SetChannelProfile (
  enum itpp::CHANNEL_PROFILE profile)
{
  m_channelProfile = profile;
}

enum itpp::CHANNEL_PROFILE
PhySimTappedDelayLinePropagationLoss::GetChannelProfile () const
{
  return m_channelProfile;
}

void
PhySimTappedDelayLinePropagationLoss::DoCalcRxPower (
  Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double m_speed = a->GetRelativeSpeed (b); // relative speed in m/s
  double m_maxDoppler = CalcMaxDopplerFrequency (tag->GetFrequency (), m_speed);
  double m_normDoppler = m_maxDoppler * tag->GetSampleDuration ();

  // IT++ requires a normalized doppler greater than zero
  NS_ASSERT (m_normDoppler > 0);

  itpp::TDL_Channel m_channel;
  m_channel.set_norm_doppler (m_normDoppler);

  NS_LOG_DEBUG ("PhySimTappedDelayLinePropagationLoss::DoCalcRxPower() relativeSpeed = " << m_speed);
  NS_LOG_DEBUG ("PhySimTappedDelayLinePropagationLoss::DoCalcRxPower() m_maxDoppler = " << m_maxDoppler);
  NS_LOG_DEBUG ("PhySimTappedDelayLinePropagationLoss::DoCalcRxPower() m_normDoppler = " << m_normDoppler);

  itpp::cvec input = tag->GetRxedSamples ();
  itpp::Array<itpp::cvec> coeff;
  itpp::cvec output = m_channel.filter (input, coeff);

  NS_LOG_DEBUG ("PhySimTappedDelayLinePropagationLoss::DoCalcRxPower() coeff = " << coeff);

  tag->SetRxSamples (output);
}

double
PhySimTappedDelayLinePropagationLoss::CalcMaxDopplerFrequency (
  double frequency, double speed) const
{
  return speed * frequency / 299792458.0; // (v * freq) / c0
}

// ------------------------------------------------------------------------- //

NS_OBJECT_ENSURE_REGISTERED (PhySimVehicularChannelPropagationLoss);

TypeId
PhySimVehicularChannelPropagationLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimVehicularChannelPropagationLoss")
    .SetParent<PhySimPropagationLossModel> ()
    .AddConstructor<PhySimVehicularChannelPropagationLoss> ()
    .AddAttribute ("MinimumRelativeSpeed",
                   "The minimum relative speed [m/s] we want to allow. Every calculated value below will be set to this value.",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&PhySimVehicularChannelPropagationLoss::m_minRelativeSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ChannelProfile",
                   "The channel profile which used for the Vehicular Channel model.",
                   EnumValue (V2V_EXPRESSWAY_ONCOMING),
                   MakeEnumAccessor (&PhySimVehicularChannelPropagationLoss::SetChannelProfile,
                                     &PhySimVehicularChannelPropagationLoss::GetChannelProfile),
                   MakeEnumChecker (
                     V2V_EXPRESSWAY_ONCOMING, "Vehicle-to-Vehicle Expressway Oncoming (balanced)",
                     V2V_EXPRESSWAY_ONCOMING_OR, "Vehicle-to-Vehicle Expressway Oncoming (as in paper)",
                     RTV_URBAN_CANYON, "Roadside-to-Vehicle Urban Canyon (balanced)",
                     RTV_URBAN_CANYON_OR, "Roadside-to-Vehicle Urban Canyon (as in paper)",
                     RTV_EXPRESSWAY, "Roadside-to-Vehicle Expressway (balanced)",
                     RTV_EXPRESSWAY_OR, "Roadside-to-Vehicle Expressway (as in paper)",
                     V2V_URBAN_CANYON_ONCOMING, "Vehicle-to-Vehicle Urban Canyon Oncoming (balanced)",
                     V2V_URBAN_CANYON_ONCOMING_OR, "Vehicle-to-Vehicle Urban Canyon Oncoming (as in paper)",
                     RTV_SUBURBAN_STREET, "Roadside-to-Vehicle Suburban Street (balanced)",
                     RTV_SUBURBAN_STREET_OR, "Roadside-to-Vehicle Suburban Street (as in paper)",
                     V2V_EXPRESS_SAME_DIREC_WITH_WALL, "Vehicle-to-Vehicle Same Direction with Wall(balanced)",
                     V2V_EXPRESS_SAME_DIREC_WITH_WALL_OR, "Vehicle-to-Vehicle Same Direction with Wall(as in paper)"));
  return tid;
}
PhySimVehicularChannelPropagationLoss::PhySimVehicularChannelPropagationLoss ()
  : PhySimPropagationLossModel ()
{
  m_channel = CreateObject<Vehicular_TDL_Channel> ();
}
PhySimVehicularChannelPropagationLoss::~PhySimVehicularChannelPropagationLoss ()
{
}
void
PhySimVehicularChannelPropagationLoss::SetChannelProfile (enum VEHICULAR_CHANNEL_PROFILE profile)
{
  m_profile = profile;
  m_channel->set_channel_profile (profile);
  m_channel->reset_gens ();
}
VEHICULAR_CHANNEL_PROFILE
PhySimVehicularChannelPropagationLoss::GetChannelProfile () const
{
  return m_profile;
}
void
PhySimVehicularChannelPropagationLoss::DoCalcRxPower (
  Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double relativeSpeed = a->GetRelativeSpeed (b);
  if (relativeSpeed < m_minRelativeSpeed)
    {
      relativeSpeed = m_minRelativeSpeed;
    }

  // Frequency must be 5.9 GHz or assumptions in the model break
  NS_ASSERT (tag->GetFrequency () == 5.9e9);
  // Time samples duration must be 100 ns or assumptions in the model break
  NS_ASSERT (tag->GetSampleDuration () == 1e-7);

  m_channel->set_relative_speed (relativeSpeed);
  NS_LOG_DEBUG ("PhySimVehicularChannelPropagationLoss::DoCalcRxPower() relativeSpeed = " << relativeSpeed);
  itpp::cvec input = tag->GetRxedSamples ();
  itpp::Array<itpp::cvec> coeff;
  itpp::cvec output;
  m_channel->filter (input, output, coeff);
  NS_LOG_DEBUG ("PhySimVehicularPropagationLoss::DoCalcRxPower() coeff = " << coeff);
  tag->SetRxSamples (output);
  m_channel->reset_gens (); // Do this unless the next transmission is immediately after and utilises the same channel
}

} // namespace ns3
