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

#include "physim-vehicular-TDL-channel.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include <itpp/base/math/misc.h>
#include <itpp/stat/misc_stat.h>

NS_LOG_COMPONENT_DEFINE ("Vehicular_TDL_Channel");

namespace ns3 {
IFFTFadingGenerator::IFFTFadingGenerator (double norm_doppler,
                                          VEHICULAR_DOPPLER_SPECTRUM spectrum, double fShift)
  : Correlated_Fading_Generator (norm_doppler),
    shape (spectrum),
    freqShift (fShift)
{
  time_offset = 0;
}

IFFTFadingGenerator::~IFFTFadingGenerator ()
{
}

void
IFFTFadingGenerator::generate (int no_samples, itpp::cvec &output)
{
  if (init_flag == false)
    {
      init ();
    }

  generate_Spectrum (no_samples, output, shape, freqShift);

  if (los_power > 0.0)
    { // LOS component exist
      for (int i = 0; i < no_samples; i++)
        {
          add_LOS (i, output (i));
        }
    }

  time_offset += no_samples;
}
itpp::cvec
IFFTFadingGenerator::generate (int no_samples)
{
  itpp::cvec output;
  generate (no_samples, output);
  return output;
}

void
IFFTFadingGenerator::reset_time_offset ()
{
  time_offset = 0;
}

void
IFFTFadingGenerator::add_LOS (int idx, std::complex<double>& sample)
{
  double tmp_arg = itpp::m_2pi * los_dopp * (idx + time_offset);
  sample *= los_diffuse;
  sample += los_direct * std::complex<double> (std::cos (tmp_arg), std::sin (
                                                 tmp_arg));
}

void
IFFTFadingGenerator::generate_Spectrum (int no_samples, itpp::cvec &output,
                                        VEHICULAR_DOPPLER_SPECTRUM shape, double freqShift)
{
  int Nfft = itpp::pow2i (itpp::levels2bits (no_samples));
  double df = 1.0 / Nfft;
  int noisesamp = itpp::ceil_i (n_dopp / df);
  int no_upsample = 1;


  while (noisesamp <= 10)
    { // if too few samples, increase the FFT size
      Nfft *= 2;
      no_upsample *= 2;
      df = 1.0 / Nfft;
      noisesamp = itpp::ceil_i (n_dopp / df);
    }

  itpp::vec Fpos = itpp::linspace (0, 0.5, Nfft / 2 + 1);
  itpp::vec F = itpp::concat (Fpos, reverse (-Fpos (1, Nfft / 2 - 1)));
  itpp::vec S = itpp::zeros (Nfft);

  switch (shape)
    {
    case Classic6dB:
      for (int i = 0; i < F.size(); i++)
        {
          if (std::fabs(F(i) - freqShift) < n_dopp)
            {
              double calc = 1 - std::pow((F(i) - freqShift) / n_dopp, 2);
              S(i) = std::sqrt(1.5 / (itpp::pi * n_dopp * std::sqrt(calc)));
            }
          else if (std::fabs(F(i) - freqShift) == n_dopp)
            {
              S(i) = 1000000;
            }
          else
            {
              S(i) = 0;
            }
          NS_ASSERT (!isnan(S(i)));
        }
      break;
    case Classic3dB:
      for (int i = 0; i < F.size(); i++)
        {
          if (std::fabs(F(i) - freqShift) < n_dopp)
            {
              double calc = 1 - std::pow((F(i) - freqShift) / n_dopp, 2);
              S(i) = std::sqrt(std::sqrt(1.5 / (itpp::pi * n_dopp * std::sqrt(calc))));
            }
          else if (std::fabs(F(i) - freqShift) == n_dopp)
            {
              S(i) = 1000000;
            }
          else
            {
              S(i) = 0;
            }
          NS_ASSERT(!isnan(S(i)));
        }
      break;
    case ROUND:
      for (int i = 0; i < F.size(); i++)
        {
          if (std::fabs(F(i) - freqShift) < n_dopp) {
              double calc = 1 - std::pow((F(i) - freqShift) / n_dopp, 2);
              S(i) = std::sqrt(itpp::pi * n_dopp * std::sqrt(calc));
          } else {
              S(i) = 0;
          }
          NS_ASSERT (!isnan(S(i)));
        }
      break;
    case FLAT:
      for (int i = 0; i < F.size (); i++)
        {
          S (i) = 1;
        }
      break;
    }
  S /= itpp::norm (S, 2);
  S *= Nfft;

  itpp::cvec x = itpp::zeros_c (Nfft);

  for (int i = 0; i < noisesamp; ++i)
    {
      std::complex< double > a = itpp::randn_c();
      std::complex< double > b = itpp::randn_c();
      x(i) = S(i) * a;
      x(Nfft - 1 - i) = S(Nfft - 1 - i) * b;
    }

  x = ifft (x);
  output = x.mid (0, no_samples);
}

NS_OBJECT_ENSURE_REGISTERED (Vehicular_TDL_Channel);

TypeId
Vehicular_TDL_Channel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Vehicular_TDL_Channel")
      .SetParent<Object> ()
      .AddConstructor<Vehicular_TDL_Channel> ();
  return tid;
}

Vehicular_TDL_Channel::Vehicular_TDL_Channel ()
{
  channel = CreateObject<Vehicular_Channel_Specification>();
  init_flag = false;
}
Vehicular_TDL_Channel::~Vehicular_TDL_Channel ()
{

}

void Vehicular_TDL_Channel::set_channel_profile (enum VEHICULAR_CHANNEL_PROFILE profile)
{
  channel->set_channel_profile(profile);
  N_taps = channel->get_no_taps ();
  a_prof = pow (10.0, channel->get_avg_power_dB () / 20.0); // Convert power profile to amplitude profile
  d_prof = channel->get_delay_prof ();
  relSpeed = channel->get_relative_speed ();
  init_flag = false;
}

void Vehicular_TDL_Channel::get_channel_profile (itpp::vec &avg_power_dB,
                                                 itpp::vec &delay_prof) const
{
  avg_power_dB = channel->get_avg_power_dB ();
  delay_prof = d_prof;
}

itpp::vec Vehicular_TDL_Channel::get_avg_power_dB () const
{
  return channel->get_avg_power_dB ();
}

void Vehicular_TDL_Channel::init ()
{

  it_assert (N_taps > 0,
             "Vehicular_TDL_Channel::init(): Channel profile not defined yet");

  if (fading_gen.size () > 0)
    {   // delete all old generators
      fading_gen.clear ();
    }
  NS_ASSERT (fading_gen.size () == 0); // make sure no generators defined
  for (int i = 0; i < N_taps; ++i)
    {
      itpp::Array<VEHICULAR_DOPPLER_SPECTRUM> spectrum = channel->taps (i).get_Doppler_Shapes ();
      itpp::ivec fShift = channel->taps (i).get_freq_Shift ();
      itpp::vec los_power = channel->taps (i).get_rician_K (); // Relative power for each Rice component
      itpp::ivec los_dopp = channel->taps (i).get_LOS_Doppler (); // Relative LOS Doppler for each Rice component
      for (int j = 0; j < channel->taps (i).get_no_paths (); ++j)
        {
          // need to create fading generators for each path
          double vratio = relSpeed / channel->get_relative_speed ();
          double adjustedFadingDopp = static_cast<double> (channel->taps (i).get_fading_doppler ().get (j)) * 1e-7 * vratio;
          double adjustedFreqShift = static_cast<double> (fShift (j)) * 1e-7 * vratio;
          IFFTFadingGenerator gen = IFFTFadingGenerator (adjustedFadingDopp,spectrum (j), adjustedFreqShift);
          if (los_power (j) != 0)
            {
              double adjustedLosDopp = los_dopp (j) * 1e-7 * vratio;
              gen.set_LOS_power (pow (10,los_power (j) / 10)); // los_power is ricianK so is it 10^(LB/10)
              gen.set_LOS_doppler (adjustedLosDopp);  // los_dopp is as set in the table
            }
          gen.init ();
          fading_gen.push_back (gen);
        }
    }
  init_flag = true;
}
void Vehicular_TDL_Channel::reset_gens ()
{
  if (init_flag == false)
    {
      init ();
    }
  for (uint i = 0; i < fading_gen.size (); ++i)
    {
      fading_gen.at (i).reset_time_offset ();
    }
}

void Vehicular_TDL_Channel::generate (const int no_samples, itpp::Array<itpp::cvec> &channel_coeff)
{
  if (init_flag == false)
    {
      init ();
    }
  itpp::Array<itpp::cvec> tap_coeff;
  tap_coeff.set_size (N_taps,false);
  channel_coeff.set_size (N_taps, false);
  itpp::cvec path_coeff (no_samples);
  double tap_power_sum;   // after the path addition make taps have unit power
  for (int i = 0; i < N_taps; ++i)
    {
      path_coeff.zeros ();
      tap_power_sum = 0;
      itpp::vec rel_path_gain = channel->taps (i).get_relative_path_loss ();
      for (int j = 0; j < channel->taps (i).get_no_paths (); ++j)
        {  // add the paths
          path_coeff += pow (10,rel_path_gain (j) / 20.0) * fading_gen[j].generate (no_samples);
          tap_power_sum += pow (10,rel_path_gain (j) / 10);
        }
      tap_coeff (i) = path_coeff / sqrt (tap_power_sum);
      NS_ASSERT (a_prof (i) == pow (10, channel->tap_power (i) / 20));
      channel_coeff (i) = a_prof (i) * tap_coeff (i);
    }
}

void Vehicular_TDL_Channel::filter (const itpp::cvec &input, itpp::cvec &output, itpp::Array<itpp::cvec> &channel_coeff)
{
  generate (input.size (),channel_coeff);
  filter_known_channel (input,output,channel_coeff);
}

void Vehicular_TDL_Channel::set_relative_speed (double speed)
{
  relSpeed = speed;
  init_flag = false;
}

void Vehicular_TDL_Channel::filter_known_channel (const itpp::cvec &input, itpp::cvec &output, const itpp::Array<itpp::cvec> &channel_coeff)
{
  int maxdelay = static_cast<int> (max (d_prof * 1e7)); // delay value per tap in samples
  output.set_size (input.size () + maxdelay, false);
  output.zeros ();
  int delayinsamples;
  for (int i = 0; i < N_taps; i++)
    {
      delayinsamples = static_cast<int> (d_prof (i) * 1e7);
      output += itpp::concat (itpp::zeros_c (delayinsamples), itpp::elem_mult (input, channel_coeff (i)), itpp::zeros_c (maxdelay - delayinsamples));
    }
}
} // namespace ns3
