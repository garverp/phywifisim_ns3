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

#ifndef PHYSIM_SIGNALDETECTOR_H
#define PHYSIM_SIGNALDETECTOR_H

#include "ns3/object.h"
#include "physim-helper.h"
#include "physim-wifi-phy-tag.h"
#include "ns3/traced-callback.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include <itpp/itcomm.h>

namespace ns3 {

/*!
 * \brief 802.11a/p signal detector.
 *
 *  A module implementing two signal detection methods (A&B) as detailed in [Liu03].
 *
 * [Liu03] Chia-Horng Liu, "On the design of OFDM signal detection algorithms for hardware implementation,"
 *              Global Telecommunications Conference, 2003. GLOBECOM '03. IEEE , vol.2, no.,
 *              pp. 596- 599 Vol.2, 1-5 Dec. 2003
 */
class PhySimSignalDetector : public Object
{
public:
  static TypeId GetTypeId (void);

  PhySimSignalDetector ();
  virtual ~PhySimSignalDetector ();

  /**
   * The main method of the signal detector that will call scan for short training symbols and eventually also
   * for long training symbols. Only if the detected starting times of the short and long training symbols are
   * valid (in the sense that they do not lead to vector out of bound access in the time sample vectors), and if
   * the correlation threshold is exceeded, it will return true and indicate the start of a packet reception.
   *
   * \return    Whether or not the preamble was detected succesfully
   */
  virtual bool DetectPreamble (Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag, const itpp::cvec &input);

private:

  /**
   * Scans for the repeating pattern of the short training symbols in the given time sample vector and stores the
   * results into the provided PhySimWifiPhyTag object
   *
   * \return The index where the start of the short training symbols is assumed
   */
  virtual int32_t ScanForShortTrainingSymbols (Ptr<PhySimWifiPhyTag> tag, const itpp::cvec &input);
  /**
   * Scans for the repeating pattern of the long training symbols in the given time sample vector and stores the
   * results into the provided PhySimWifiPhyTag object
   *
   * \return The index where the start of the long training symbols is assumed
   */
  virtual int32_t ScanForLongTrainingSymbols (Ptr<PhySimWifiPhyTag> tag, const itpp::cvec &input);

  virtual double ComputeAutoCorrelation (const itpp::cvec& input, const itpp::vec& norminput, const int32_t window);
  virtual double ComputeCorrelation (const itpp::cvec& input, const itpp::vec& norminput, const int32_t window);

  double m_corrThresh;
  bool m_autoCorrelation;
  bool m_ieeeCompliantMode;
};

} // namespace ns3

#endif /* PHYSIM_SIGNALDETECTOR_H */
