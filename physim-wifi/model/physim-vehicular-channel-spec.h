/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Stylianos Papanastasiou, Jens Mittag
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

#ifndef PHYSIM_VEHICULAR_CHANNEL_SPEC_H
#define PHYSIM_VEHICULAR_CHANNEL_SPEC_H

#include "ns3/object.h"
#include "ns3/random-variable.h"
#include "ns3/enum.h"
#include <itpp/itcomm.h>

namespace ns3 {

/*!
 * Vehicular channel names for use with the model.
 *
 * Vehicular channels with the OR suffix are the original ones as described in the paper
 * , the others are recomputed so that the sum of the power of the paths = power of the
 * corresponding tap.
 */
enum VEHICULAR_CHANNEL_PROFILE
{
  V2V_EXPRESSWAY_ONCOMING,
  V2V_EXPRESSWAY_ONCOMING_OR,
  RTV_URBAN_CANYON,
  RTV_URBAN_CANYON_OR,
  RTV_EXPRESSWAY,
  RTV_EXPRESSWAY_OR,
  V2V_URBAN_CANYON_ONCOMING,
  V2V_URBAN_CANYON_ONCOMING_OR,
  RTV_SUBURBAN_STREET,
  RTV_SUBURBAN_STREET_OR,
  V2V_EXPRESS_SAME_DIREC_WITH_WALL,
  V2V_EXPRESS_SAME_DIREC_WITH_WALL_OR
};

/*!
 * \brief Description of the shape of the Doppler spectrum.
 */
enum VEHICULAR_DOPPLER_SPECTRUM
{
  Classic3dB, Classic6dB, ROUND, FLAT
};

/*
 *\brief Tap for a vehicular channel. Contains path information (RiceanK, freq. Shift, etc...).
 */
class Vehicular_Channel_Tap
{
public:
  Vehicular_Channel_Tap ();
  Vehicular_Channel_Tap (itpp::vec rpathloss, itpp::vec Rk, itpp::ivec fShift,
                         itpp::ivec fDoppler, itpp::ivec lDoppler,
                         itpp::Array<enum VEHICULAR_DOPPLER_SPECTRUM> vshapes);
  // ! Destructor
  virtual ~Vehicular_Channel_Tap () {}

  int get_no_paths () const;
  itpp::vec get_relative_path_loss () const;
  itpp::vec get_rician_K () const;
  itpp::ivec get_freq_Shift () const;
  itpp::ivec get_fading_doppler () const;
  itpp::ivec get_LOS_Doppler () const;
  itpp::Array<enum VEHICULAR_DOPPLER_SPECTRUM> get_Doppler_Shapes () const;

private:
  itpp::vec relative_path_loss;   	// relative path loss in dB per path
  itpp::vec ricianK;   				// ricean K (in dB) per tap, 0 means no ricean K
  itpp::ivec freqShift;   			// freq. Shift per path
  itpp::ivec fadingDoppler;   		// fadingDoppler per path
  itpp::ivec LOSDoppler;   			// LOS Doppler per path
  itpp::Array<enum VEHICULAR_DOPPLER_SPECTRUM> shapes;
};

/*!
 * \brief Contains specifications of vehicular channels.
 *
 * Contains the details on tap and path characteristics for each channel described in
 * [Marum09]. The original values make it so that the relative path powers do not sum up
 * to equal the relative tap powers. To correct these (mostly small) discrepancies we
 * include new "balanced" values alongside the original ones. The correction for the data
 * mentioned in [Ivan10] is also included (in RTV-E the K factor of the first path is
 *  5.3 rather than -5.3).
 *
 * References:
 * - [Marum09] Guillermo Acosta-Marum, Marry Ann Ingram, Six Time- and Frequency-
 *  Selective Empirical Channel Models for Vehicular Wireless LANsm, 2009.
 * - [Ivan10] Iulia Ivan, Philippe Besnie, Xavier Bunlon, Loïs Le Danvic, Matthieu Crussière,
 *  M’hamed Drissi, Influence of propagation channel modeling on V2X physical layer
 *  performance, EuCAP 2010.
 *
 */
class Vehicular_Channel_Specification : public Object
{
  friend class Vehicular_TDL_Channel;
public:
  static TypeId
  GetTypeId (void);
  Vehicular_Channel_Specification ();
  // ! Initialize with predetermined channel profile
  Vehicular_Channel_Specification (const enum VEHICULAR_CHANNEL_PROFILE prof);
  // ! Destructor
  virtual ~Vehicular_Channel_Specification () {}

  // ! Set channel profile to a predetermined profile
  void set_channel_profile (const enum VEHICULAR_CHANNEL_PROFILE prof);

  // ! Return power profile in dB
  itpp::vec get_avg_power_dB () const { return tap_power; }

  // ! Return delay profile in seconds
  itpp::vec get_delay_prof () const { return tap_delay; }

  // ! Get doppler spectrum for tap \a index
  itpp::Array<enum VEHICULAR_DOPPLER_SPECTRUM>
  get_doppler_spectrum (int tap_number) const
  {
    return taps (tap_number).get_Doppler_Shapes ();
  }

  // ! Get relative power (Rice factor) for tap \a tap_number
  itpp::vec
  get_LOS_power (int tap_number) const
  {
    return taps (tap_number).get_rician_K ();
  }
  // ! Get relative Doppler for tap \a tap_number
  itpp::ivec
  get_LOS_doppler (int tap_number) const
  {
    return taps (tap_number).get_LOS_Doppler ();
  }

  // ! Return the number of channel taps
  int
  get_no_taps () const
  {
    return taps.size();
  }

  const itpp::Array<Vehicular_Channel_Tap>&
  get_taps ()
  {
    return taps;
  }

  enum VEHICULAR_CHANNEL_PROFILE get_channel_profile () { return profile; }

  static std::string
  get_channel_profile_name (enum VEHICULAR_CHANNEL_PROFILE profile);

  double
  get_relative_speed () const
  {
    return relSpeed;
  }

  friend std::ostream&
  operator<< (std::ostream &out, Ptr<Vehicular_Channel_Specification> model)
  {
    return out << Vehicular_Channel_Specification::get_channel_profile_name (
             model->get_channel_profile ());
  }
private:
  void
  set_tap_power_delay (const itpp::vec &avg_power_dB,
                       const itpp::vec &delay_prof);

  enum VEHICULAR_CHANNEL_PROFILE profile;
  // !Each channel has taps
  itpp::Array<Vehicular_Channel_Tap> taps;
  itpp::vec tap_delay;   // !< tap delay in secs
  itpp::vec tap_power;   // !< tap power in dB
  double relSpeed;   // !< relative vehicle speed for the measurements within

};

} // namespace ns3

#endif /* PHYSIM_VEHICULAR_CHANNEL_SPEC_H */
