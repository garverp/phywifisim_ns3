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
 * Based on propagation-loss-model.h by
 *      Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Major Modifications:
 *      Jens Mittag <jens.mittag@kit.edu>
 *      Stylianos Papanastasiou <stylianos@gmail.com>
 */

#ifndef PHYSIM_PROPAGATION_LOSS_MODEL_H
#define PHYSIM_PROPAGATION_LOSS_MODEL_H

#include "ns3/object.h"
#include "ns3/random-variable.h"
#include <itpp/itcomm.h>
#include "physim-wifi-phy.h"
#include "physim-vehicular-channel-spec.h"
#include "physim-vehicular-TDL-channel.h"

namespace ns3 {

class MobilityModel;

/**
 * \brief A model for the propagation loss through a transmission medium for the detailed physical
 * layer implementation, in which each frame is represented by a complex vector of samples.
 *
 * Applies propagation loss models on top of a complex vector representing the sampled waveform of
 * the OFDM frame.
 *
 * Note: this is a dummy propagation loss model which is used as an entry point for the chaining of several real propagation
 * loss models.
 */
class PhySimPropagationLossModel : public Object
{
public:
  static TypeId GetTypeId (void);

  PhySimPropagationLossModel ();
  virtual ~PhySimPropagationLossModel ();

  void SetNext (Ptr<PhySimPropagationLossModel> next);

  /**
   * \param tag of the packet which contains the complex representation of the
   *        transmitted waveform samples, which the propagation loss model is going to
   *        modify
   * \param a the mobility model of the source
   * \param b the mobility model of the destination
   */
  void CalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

private:
  PhySimPropagationLossModel (const PhySimPropagationLossModel &o);
  PhySimPropagationLossModel &operator = (const PhySimPropagationLossModel &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  Ptr<PhySimPropagationLossModel> m_next;
};

/**
 * \brief A model for the propagation loss through a transmission medium for the detailed physical
 * layer implementation, in which each frame is represented by a complex vector of samples.
 *
 * Implements the FriisSpace Path Loss Model.
 *
 * Friis free space equation
 * where \f$ P_{t} , G_{t}, G_{r} \f$ and \f$ P \f$ are in mWatt units and
 * \f$ L \f$ is in meter units.
 *
 * \f[
 *  \frac{P}{P_{t}}=\frac{G_{t} * G_{r} * (\lambda^{2})}{(4*\pi*d)^{2}*L}
 * \f]
 *
 * \f$G_{t}\f$: tx gain (unit-less)
 * \f$G_{r}\f$: rx gain (unit-less)
 * \f$P_{t}\f$: tx power (mW)
 * \f$d\f$: distance (m)
 * \f$L\f$: system loss
 * \f$\lambda\f$: wavelength (m)
 *
 * Here, we ignore tx and rx gain and the input and output values
 * are in dB or dBm:
 *
 * \f[
 *
 * r_{x} = t_{x} +  10\log_{10}\frac{\lambda^{2}}{(4*\pi*d)^{2}*L}
 *
 * \f]
 * \f$ rx \f$: rx power (dB)
 * \f$ tx \f$: tx power (dB)
 * \f$ d \f$: distance (m)
 * \f$ L \f$: system loss (unit-less)
 * \f$\lambda\f$: wavelength (m)
 *
 */
class PhySimFriisSpacePropagationLoss : public PhySimPropagationLossModel
{
public:
  static TypeId GetTypeId (void);

  PhySimFriisSpacePropagationLoss ();
  virtual ~PhySimFriisSpacePropagationLoss ();

private:
  PhySimFriisSpacePropagationLoss (const PhySimFriisSpacePropagationLoss &o);
  PhySimFriisSpacePropagationLoss &operator = (const PhySimFriisSpacePropagationLoss &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  double m_speedOfLight;
  double m_systemLoss;

};

/**
 * \brief A model for the propagation loss through a transmission medium for the detailed physical
 * layer implementation, in which each frame is represented by a complex vector of samples.
 *
 * Implements the Two-Ray Ground propagation loss model.
 *
 */
class PhySimTwoRayGroundPropagationLossModel : public PhySimPropagationLossModel
{
public:
  static TypeId GetTypeId (void);

  PhySimTwoRayGroundPropagationLossModel ();
  virtual ~PhySimTwoRayGroundPropagationLossModel ();

private:
  PhySimTwoRayGroundPropagationLossModel (const PhySimTwoRayGroundPropagationLossModel &o);
  PhySimTwoRayGroundPropagationLossModel &operator = (const PhySimTwoRayGroundPropagationLossModel &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  double m_speedOfLight;
  double m_systemLoss;
  double m_heightAboveZ;
};

/**
 * \brief A propagation loss model which applies a constant propagation loss to the link, independent of
 * the distance between sender and receiver.
 *
 */
class PhySimConstantPropagationLoss : public PhySimPropagationLossModel
{
public:
  static TypeId GetTypeId (void);

  PhySimConstantPropagationLoss ();
  virtual ~PhySimConstantPropagationLoss ();

  void SetPathLoss (double loss);

private:
  PhySimConstantPropagationLoss (const PhySimConstantPropagationLoss &o);
  PhySimConstantPropagationLoss & operator = (const PhySimConstantPropagationLoss &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  double m_pathLoss;
};

/**
 *  \brief A model for the propagation loss through a transmission medium for the detailed physical
 * layer implementation, in which each frame is represented by a complex vector of samples.
 *
 * The generic formula is:
 * \f[
 *
 * r_{x} = 10\log_{10}(Pr0(t_{x})) - n * 10 * \log(\frac{d}{d_{0}})
 *
 * \f]
 *
 * \f$ Pr0 \f$: rx power at reference distance d0 (mW)
 * \f$d_{0}\f$: reference distance: 1.0 (m)
 * \f$d\f$: distance (m)
 * \f$t_{x}\f$: tx power (dB)
 * \f$r_{x}\f$: rx power (dB)
 *
 * In this case we have:
 *
 * \f[
 *
 * r_{x} = rx0(t_{x}) - 10 * n * \log(\frac{d}{d_{0}})
 *
 * \f]
 *
 * Note: for uniformity (with the traditional NS-3 model) the above has been changed to
 *
 * \f[
 *
 * r_{x} = rx0(t_{x}) + 10 * n * \log(\frac{d_{0}}{d})
 *
 * \f]
 *
 */
class PhySimLogDistancePropagationLoss : public PhySimPropagationLossModel
{
public:
  static TypeId GetTypeId (void);

  PhySimLogDistancePropagationLoss ();
  virtual ~PhySimLogDistancePropagationLoss ();

private:
  PhySimLogDistancePropagationLoss (const PhySimLogDistancePropagationLoss &o);
  PhySimLogDistancePropagationLoss &operator = (const PhySimLogDistancePropagationLoss &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  double m_exponent;
  double m_referenceDistance;
  double m_referenceLoss;

};

/**
 * \brief A model for the propagation loss through a transmission medium for the detailed physical
 * layer implementation, in which each frame is represented by a complex vector of samples.
 *
 * Implements the 3-log-distance propagation model as in the NS-3 simulator.
 *
 *	\sa \c PhySimLogDistancePropagationLoss only this time there are
 *	3 distances defined where the fading rate changes.
 *
 */
class PhySimThreeLogDistancePropagationLoss : public PhySimPropagationLossModel
{
public:
  static TypeId GetTypeId (void);

  PhySimThreeLogDistancePropagationLoss ();
  virtual ~PhySimThreeLogDistancePropagationLoss ();

private:
  PhySimThreeLogDistancePropagationLoss (const PhySimThreeLogDistancePropagationLoss &o);
  PhySimThreeLogDistancePropagationLoss &operator = (const PhySimThreeLogDistancePropagationLoss &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  double m_distance0;
  double m_distance1;
  double m_distance2;

  double m_exponent0;
  double m_exponent1;
  double m_exponent2;

  double m_referenceLoss;

};

/**
 * \brief A model for the propagation loss through a transmission medium for the detailed physical
 * layer implementation, in which each frame is represented by a complex vector of samples.
 *
 * The implementation reflects the behavior of the Shadowing large-scale fading model.
 *
 * Note: since the standard deviation is given in dB (logarithmic scale) already, this implementation
 * uses the gaussian normal random variable class instead of the log normal random variable.
 *
 */
class PhySimShadowingPropagationLoss : public PhySimPropagationLossModel
{
public:
  static TypeId GetTypeId (void);

  PhySimShadowingPropagationLoss ();
  virtual ~PhySimShadowingPropagationLoss ();

  void SetStandardDeviation (double std);
  double GetStandardDeviation (void) const;

private:
  PhySimShadowingPropagationLoss (const PhySimShadowingPropagationLoss &o);
  PhySimShadowingPropagationLoss &operator = (const PhySimShadowingPropagationLoss &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  double m_stdDeviationDb; // standard deviation of shadowing fading (in dB)
  double m_minDistance; // minimum distance to apply this from

  NormalVariable *m_normalVariable;

};

/**
 * \brief A model for the propagation loss through a transmission medium for the detailed physical
 * layer implementation, in which each frame is represented by a complex vector of samples.
 *
 * The implementation reflects the behavior of a Rician small-scale fading.
 *
 */
class PhySimRicianPropagationLoss : public PhySimPropagationLossModel
{
public:
  static TypeId GetTypeId (void);

  PhySimRicianPropagationLoss ();
  virtual ~PhySimRicianPropagationLoss ();

private:
  PhySimRicianPropagationLoss (const PhySimRicianPropagationLoss &o);
  PhySimRicianPropagationLoss &operator = (const PhySimRicianPropagationLoss &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  double CalcMaxDopplerFrequency (double frequency, double speed) const;
  void add_LOS (double los_dopp, double n_dopp, double los_diffuse, double los_direct, int idx, std::complex<double>& sample) const;
  void UpdateGeneratorParameters (double norm_doppler, int nofrequencies) const;

  bool m_shortcut;
  double m_minRelativeSpeed;
  double m_lineOfSightPower;
  double m_lineOfSightDoppler;
  static const double m_max_time_static = 1e-2;
  itpp::Rice_Fading_Generator* m_generator;
};

/**
 * \brief A model for the propagation loss through a transmission medium for the detailed physical
 * layer implementation, in which each frame is represented by a complex vector of samples.
 *
 * The implementation follows the Tapped Delay Line (TDL) model and is based on the TDL implementation
 * provided by IT++.
 *
 * @see http://itpp.sourceforge.net/
 *
 */
class PhySimTappedDelayLinePropagationLoss : public PhySimPropagationLossModel
{
public:
  static TypeId GetTypeId (void);

  PhySimTappedDelayLinePropagationLoss ();
  virtual ~PhySimTappedDelayLinePropagationLoss ();

  void SetChannelProfile (enum itpp::CHANNEL_PROFILE profile);
  enum itpp::CHANNEL_PROFILE GetChannelProfile () const;

private:
  PhySimTappedDelayLinePropagationLoss ( const PhySimTappedDelayLinePropagationLoss &o);
  PhySimTappedDelayLinePropagationLoss &operator = (const PhySimTappedDelayLinePropagationLoss &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  double
  CalcMaxDopplerFrequency (double frequency, double speed) const;

  enum itpp::CHANNEL_PROFILE m_channelProfile;

};

/*!
 * \brief Channel models for Vehicular Communications (only 802.11p)
 *
 * Implements the channel models described in [Marum09]
 *
 * References:
 * - [Marum09] Guillermo Acosta-Marum, Marry Ann Ingram, Six Time- and Frequency-
 *  Selective Empirical Channel Models for Vehicular Wireless LANs, 2009.
 */
class PhySimVehicularChannelPropagationLoss : public PhySimPropagationLossModel
{
public:
  static TypeId GetTypeId (void);
  PhySimVehicularChannelPropagationLoss ();
  virtual ~PhySimVehicularChannelPropagationLoss ();

  void SetChannelProfile (VEHICULAR_CHANNEL_PROFILE profile);
  VEHICULAR_CHANNEL_PROFILE GetChannelProfile () const;

private:
  PhySimVehicularChannelPropagationLoss (const PhySimVehicularChannelPropagationLoss &o);
  PhySimVehicularChannelPropagationLoss &operator = (const PhySimVehicularChannelPropagationLoss &o);
  virtual void DoCalcRxPower (Ptr<PhySimWifiPhyTag> tag, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  VEHICULAR_CHANNEL_PROFILE m_profile;
  Ptr<Vehicular_TDL_Channel> m_channel;
  double m_minRelativeSpeed;

};
} // namespace ns3

#endif /* PHYSIM_PROPAGATION_LOSS_MODEL_H */
