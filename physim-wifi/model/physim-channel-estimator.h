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

#ifndef PHYSIM_CHANNELESTIMATOR_H
#define PHYSIM_CHANNELESTIMATOR_H

#include "ns3/object.h"
#include "physim-helper.h"
#include <itpp/itcomm.h>

namespace ns3 {

/**
 * \brief Dummy implementation of a channel estimator.
 *
 * This implementation is the base (and dummy) version of a physical layer channel estimator and equalizer.
 * Logically, it does nothing but provides the interface which is used by the PhySim framework.
 *
 * This class can be sub-classed to implement advanced and more sophisticated wireless channel
 * estimators and equalizers.
 *
 */
class PhySimChannelEstimator : public Object
{

public:
  static TypeId GetTypeId (void);
  PhySimChannelEstimator ();
  virtual ~PhySimChannelEstimator ();

  virtual double GetInitialChannelEstimation (const itpp::cvec &input);
  virtual itpp::cvec ApplyEstimateFromTrainingSequence (const itpp::cvec &input, double estimate, int phaseOffset);
  virtual itpp::cvec ApplyOFDMSymbolCorrection (const itpp::cvec &input, uint32_t symbolNo);
  virtual void SetDetectedNoise (const double estN0);
  virtual double GetDetectedNoise ();
  virtual void Reset ();

  static itpp::cvec GetPilots (const itpp::cvec& input);

private:
  double m_N0;
};

/*!
 * \brief A simple channel estimator.
 *
 * The estimator works on a per OFDM symbol basis and checks the difference
 * between the received pilots and the reference. Then it linearly interpolates across
 * the subcarriers as an estimation of the channel effects. It then applies a
 * correction according to that estimate. This is a memory-less estimator, there is no
 * channel effects tracking across symbols it only works on a per-symbol basis.
 *
 */
class PhySimSimpleChannelEstimator : public PhySimChannelEstimator
{
public:
  static TypeId GetTypeId (void);

  PhySimSimpleChannelEstimator ();
  virtual ~PhySimSimpleChannelEstimator ();

  double GetInitialChannelEstimation (const itpp::cvec &input);

  itpp::cvec ApplyEstimateFromTrainingSequence (const itpp::cvec &input, double estimate, int phaseOffset);
  itpp::cvec ApplyOFDMSymbolCorrection (const itpp::cvec &input, uint32_t symbolNo);
  void Reset ();

};

/**
 * \brief A frequency offset channel estimator.
 *
 * Implements the techniques 2 and 3 described in section IV in [Sourour04]
 *
 * References:
 *
 * [Sourour04] Sourour E., El-Ghoroury H., McNeill D.,
 * "Frequency offset estimation and correction in the IEEE 802.11a WLAN",
 * VTC 2004, vol.7, pp. 4923-4927, Sept. 2004.
 */
class PhySimChannelFrequencyOffsetEstimator : public PhySimChannelEstimator
{
public:
  static TypeId GetTypeId (void);

  PhySimChannelFrequencyOffsetEstimator ();
  virtual ~PhySimChannelFrequencyOffsetEstimator ();

  double GetInitialChannelEstimation (const itpp::cvec &input);

  itpp::cvec ApplyEstimateFromTrainingSequence (const itpp::cvec &input, double estimate, int phaseOffset);
  itpp::cvec ApplyOFDMSymbolCorrection (const itpp::cvec &input, uint32_t symbolNo);
  void Reset ();

private:
  // For initial offset estimations based on training sequence
  double InitialFineOffsetEstimation (const itpp::cvec &input);
  double InitialCoarseOffsetEstimation (const itpp::cvec &input);

  // For pilot based offset estimation
  itpp::cvec CorrectResidualCarrierFreqOffset (const itpp::cvec &input, uint32_t symbolNo);
  double CalculateResidualCarrierFreqOffset (const itpp::cvec &input, uint32_t symbolNo);
  itpp::cvec correctLT (const itpp::cvec LT, const double correction);
  itpp::cvec GetPilots (const itpp::cvec &input);

  itpp::cvec m_channelGains;
  itpp::OFDM ofdm;
};
} // namespace ns3

#endif /* PHYSIM_CHANNELESTIMATOR_H */
