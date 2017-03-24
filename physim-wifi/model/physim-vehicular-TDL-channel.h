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

#ifndef PHYSIM_VEHICULAR_TDL_CHANNEL_H
#define PHYSIM_VEHICULAR_TDL_CHANNEL_H

#include "physim-vehicular-channel-spec.h"
#include "ns3/object.h"
#include "ns3/random-variable.h"
#include "ns3/enum.h"
#include <itpp/itcomm.h>

namespace ns3 {
/*!
 * \brief IFFT type Fading generator for use in creating Vehicular Channel Models
 *
 * This implementation mirrors the IFFT generator in IT++. The main addition is support
 * for doppler spectra other than Jakes' (Classic 6dB), namely Classic 3dB, Round, and
 * Flat.
 *
 * References:
 * - [Marum09] Guillermo Acosta-Marum, Marry Ann Ingram, Six Time- and Frequency-
 *  Selective Empirical Channel Models for Vehicular Wireless LANsm, 2009.
 */
class IFFTFadingGenerator : public itpp::Correlated_Fading_Generator
{
public:
  IFFTFadingGenerator (double norm_doppler,
                       VEHICULAR_DOPPLER_SPECTRUM spectrum, double fShift);
  virtual ~IFFTFadingGenerator ();

  virtual void init () { init_flag = true; }
  void reset_time_offset ();
  void generate (int no_samples, itpp::cvec &output);
  itpp::cvec generate (int no_samples);

protected:
  // ! Generator for VEHICULAR_DOPPLER_SPECTRUM spectrum
  void generate_Spectrum (int no_samples, itpp::cvec &output,
                     VEHICULAR_DOPPLER_SPECTRUM shape, double freqShift);
  void add_LOS (int idx, std::complex<double>& sample);

private:

  IFFTFadingGenerator ();
  VEHICULAR_DOPPLER_SPECTRUM shape;
  double freqShift;
};

/*!
 * \brief An implementation of vehicular channels modelled after actual measurements.
 * These reflect small-scale fading only.
 *
 * This is an implementation of the vehicular channel models described in [Marum09].
 *
 * References:
 * - [Marum09] Guillermo Acosta-Marum, Marry Ann Ingram, Six Time- and Frequency-
 *  Selective Empirical Channel Models for Vehicular Wireless LANsm, 2009.
 */
class Vehicular_TDL_Channel : public Object
{
public:
  static TypeId GetTypeId (void);

  Vehicular_TDL_Channel ();
  virtual ~Vehicular_TDL_Channel ();

  // ! Set the relative speed between source and destination
  void set_relative_speed (double speed);
  // ! Set channel profile
  void set_channel_profile (enum VEHICULAR_CHANNEL_PROFILE profile);

  Ptr<Vehicular_Channel_Specification> get_channel_spec () { return channel; }
  // ! Get both average power profile in dB and power delay profile in samples
  void get_channel_profile (itpp::vec &avg_power_dB, itpp::vec &delay_prof) const;
  // ! Return power profile in dB
  itpp::vec get_avg_power_dB () const;
  // ! Return delay profile in samples
  itpp::vec get_delay_prof () const { return d_prof; }
  // ! Return the number of channel taps
  int get_no_taps () const { return N_taps; }

  void generate (int no_samples, itpp::Array<itpp::cvec> &channel_coeff);
  void filter_known_channel (const itpp::cvec &input, itpp::cvec &output,
                        const itpp::Array<itpp::cvec> &channel_coeff);
  // ! Generate channel coefficients and filter the \a input. Return output and channel coefficients
  void filter (const itpp::cvec &input, itpp::cvec &output, itpp::Array<itpp::cvec> &channel_coeff);

  // ! \brief Reset the fading generators time offset - should be done if following frames are not directly adjacent in time
  void reset_gens ();

  friend std::ostream&
  operator<< (std::ostream &out, Vehicular_TDL_Channel model)
  {
    out << model.get_channel_spec () << " no_taps:" << model.get_no_taps ()
          << " avg.Power (dB):" << model.get_avg_power_dB ()
          << "Delay profile: " << model.get_channel_spec ()->get_delay_prof ();
    for (int i = 0; i < model.get_channel_spec ()->get_no_taps (); ++i)
      {
        out << ", tap " << i << " pathloss: ";
        out
              << (model.get_channel_spec ()->get_taps ())(i).get_relative_path_loss ();
      }
    return out;
  }

protected:
  Ptr<Vehicular_Channel_Specification> channel;
  bool init_flag;   // !< Channel ready to produce data
  itpp::vec a_prof;   // !< Amplitude of each tap (converted from tap power)
  itpp::vec d_prof;   // !< Delay value for each tap
  int N_taps;   // !< Number of taps
  double relSpeed;   // !< Relative speed in m/sec
  std::vector<IFFTFadingGenerator> fading_gen;   // !< Fading generators for each tap
  // discretization is not necessary because the intertap delay is a multiple of the sampling time
  // for 802.11p with 10 Mhz channel

private:
  void init ();
};
} // namespace ns3

#endif /* PHYSIM_VEHICULAR_TDL_CHANNEL_H */
