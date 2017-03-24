/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) Stylianos Papanastasiou, Jens Mittag
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

#ifndef PHYSIM_BLOCKINTERLEAVER_ENCODER_H
#define PHYSIM_BLOCKINTERLEAVER_ENCODER_H

#include "ns3/object.h"
#include "ns3/wifi-mode.h"
#include <map>
#include <itpp/itcomm.h>

namespace ns3 {


/**
 * \brief A block (de)interleaver used by the PhySimWifiPhy implementation.
 */
class PhySimBlockInterleaver : public Object
{
public:
  static TypeId GetTypeId (void);

  PhySimBlockInterleaver ();
  virtual ~PhySimBlockInterleaver ();

  itpp::bvec InterleaveBlock (const itpp::bvec& bits);
  itpp::vec DeinterleaveBlock (const itpp::vec& bits);
  void SetWifiMode (const WifiMode mode);

private:
  itpp::ivec InterleavingIndex (uint32_t sizeWithPadding, uint32_t m_s);
  itpp::ivec DeinterleavingIndex (uint32_t sizeWithPadding, uint32_t m_s);
  uint32_t GetPadding (uint32_t size);

  uint32_t m_NCBPS;             // Number of coded bits per OFDM symbol
  uint32_t m_zeropadding;       // number of zeros added to the last interleaved packet
  itpp::ivec iIndex,diIndex;

  // for performance optimization
  static std::map<uint32_t, itpp::ivec> m_iCache;
  static std::map<uint32_t, itpp::ivec> m_diCache;

};

} // namespace ns3

#endif /* PHYSIM_BLOCKINTERLEAVER_ENCODER_H */
