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

#ifndef PHYSIM_OFDM_SYMBOLCREATOR_H
#define PHYSIM_OFDM_SYMBOLCREATOR_H

#include "ns3/object.h"
#include "physim-channel-estimator.h"
#include "ns3/wifi-mode.h"
#include <itpp/itcomm.h>

namespace ns3 {

/**
 * \brief The OFDM symbol creator module which is used by PhySimWifiPhy
 */
class PhySimOFDMSymbolCreator : public Object
{
public:
  static TypeId GetTypeId (void);

  PhySimOFDMSymbolCreator ();
  virtual ~PhySimOFDMSymbolCreator ();

  // this function is only used for the modulation of signal header and data bits
  itpp::cvec Modulate (const itpp::bvec &input, const int symbolIndex);
  // this function is only used for the modulation of the preamble
  itpp::cvec Modulate (const itpp::cvec &input, int cyclic_prefix = 0);
  itpp::vec DeModulate (const itpp::cvec &input, uint32_t symbolNo = 0);

  void SetModulationType (enum PhySimHelper::ModulationType type, uint8_t constellation);
  itpp::cvec Normalise (const itpp::cvec &input, bool multiply);

  static itpp::cvec TransformToOutput (const itpp::cvec &input);
  static itpp::cvec TransformToInput (const itpp::cvec &input);
  static itpp::cvec GetPilotSubcarrier (const int symbolIndex);

  void SetChannelEstimator (std::string type);
  Ptr<PhySimChannelEstimator> GetChannelEstimator ();

private:
  int CheckSize ();
  void SwitchPartsAndSign (itpp::cvec& input);
  void TransformModulation (itpp::cvec &modulated, bool toIEEE);
  void TransformModulationQPSK (itpp::cvec &modulated, bool toIEEE);
  void InsertPilots (itpp::cvec &input, int symbolIndex);

  itpp::cvec ModulateBits (const itpp::bvec& input);
  itpp::vec DeModulateBits (const itpp::cvec& input);

  enum PhySimHelper::ModulationType m_modulationType;
  uint8_t m_constellation;
  uint32_t m_noCarriers;
  uint32_t m_NCP;
  uint32_t m_norm;

  static const uint32_t m_DefaultNoCarriers = 64;
  static const uint32_t m_DefaultNCP = 16;
  static const uint32_t m_DefaultNorm = 1;

  itpp::BPSK_c *m_bpsk;
  itpp::QPSK *m_qpsk;
  itpp::QAM *m_qam16;
  itpp::QAM *m_qam64;
  itpp::OFDM m_ofdm;

  // ! Polarity of symbols (according to 17-25 in Standard)
  static const uint32_t m_subcarrierPolarity[127];

  // ! Subcarrier values for each OFDM symbol (according to 17-24 in Standard)
  static const itpp::cvec m_subcarrierValues;

  bool m_ieeeCompliantMode;
  bool m_softViterbiDecision;
  itpp::Soft_Method m_softMethod;

  // For channel estimation based on pilot sub-carriers
  Ptr<PhySimChannelEstimator> m_estimator;

};


} // namespace ns3

#endif /* PHYSIM_OFDM_SYMBOLCREATOR_H */
