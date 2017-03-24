/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009,2010 Stylianos Papanastasiou, Jens Mittag
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
 */

#include "physim-wifi-phy-tag.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PhySimWifiPhyTag);

TypeId
PhySimWifiPhyTag::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::PhySimWifiPhyTag")
    .SetParent<Tag> ()
    .AddConstructor<PhySimWifiPhyTag> ()
  ;
  return tid;
}

PhySimWifiPhyTag::PhySimWifiPhyTag ()
  : m_txed (false),
    m_rxSamples (0),
    m_beginLongSymbols (-1),
    m_captured (false),
    m_length (0)
{
}

void
PhySimWifiPhyTag::SetTxParameters (WifiPreamble wifiPreamble,
                                   WifiMode wifiMode,
                                   Time duration,
                                   itpp::bvec databits,
                                   itpp::cvec samples,
                                   double frequency,
                                   double sampleDuration,
                                   Ptr<NetDevice> txDevice)
{
  m_txed = true;
  m_wifiPreamble = wifiPreamble;
  m_txWifiMode = wifiMode;
  m_duration = duration;
  m_txDataBits = databits;
  m_txSamples = samples;
  m_frequency = frequency;
  m_sampleDuration = sampleDuration;
  m_txNetDevice = txDevice;
}

bool
PhySimWifiPhyTag::IsTxed () const
{
  return m_txed;
}

WifiPreamble
PhySimWifiPhyTag::GetWifiPreamble () const
{
  NS_ASSERT (m_txed);
  return m_wifiPreamble;
}

WifiMode
PhySimWifiPhyTag::GetTxWifiMode () const
{
  NS_ASSERT (m_txed);
  return m_txWifiMode;
}

WifiMode
PhySimWifiPhyTag::GetRxWifiMode () const
{
  return m_rxWifiMode;
}

Time
PhySimWifiPhyTag::GetDuration () const
{
  NS_ASSERT (m_txed);
  return m_duration;
}

itpp::bvec
PhySimWifiPhyTag::GetTxedDataBits () const
{
  NS_ASSERT (m_txed);
  return m_txDataBits;
}

itpp::cvec
PhySimWifiPhyTag::GetTxedSamples () const
{
  NS_ASSERT (m_txed);
  return m_txSamples;
}

itpp::cvec
PhySimWifiPhyTag::GetRxedSamples () const
{
  return m_rxSamples;
}

double
PhySimWifiPhyTag::GetFrequency () const
{
  return m_frequency;
}

double
PhySimWifiPhyTag::GetSampleDuration () const
{
  return m_sampleDuration;
}

void
PhySimWifiPhyTag::SetRxSamples (itpp::cvec samples)
{
  m_rxSamples = samples;
}

void
PhySimWifiPhyTag::SetShortSymbolStart (uint32_t sample)
{
  m_beginShortSymbols = sample;
}

uint32_t
PhySimWifiPhyTag::GetShortSymbolStart () const
{
  return m_beginShortSymbols;
}

void
PhySimWifiPhyTag::SetLongSymbolStart (uint32_t sample)
{
  m_beginLongSymbols = sample;
}

uint32_t
PhySimWifiPhyTag::GetLongSymbolStart () const
{
  return m_beginLongSymbols;
}

void
PhySimWifiPhyTag::SetRxWifiMode (WifiMode mode)
{
  m_rxWifiMode = mode;
}

void
PhySimWifiPhyTag::SetDetectedLength (uint32_t length)
{
  m_length = length;
}

void
PhySimWifiPhyTag::SetRxDataBits (itpp::bvec payload)
{
  m_rxDatabits = payload;
}

itpp::bvec
PhySimWifiPhyTag::GetRxDataBits () const
{
  return m_rxDatabits;
}

uint32_t
PhySimWifiPhyTag::GetDetectedLength () const
{
  return m_length;
}

void
PhySimWifiPhyTag::SetInitialEstimate (double estimate)
{
  m_initialEstimate = estimate;
}

double
PhySimWifiPhyTag::GetInitialEstimate () const
{
  return m_initialEstimate;
}

void
PhySimWifiPhyTag::SetPreambleSinr (double value)
{
  m_preambleSinr = value;
}

void
PhySimWifiPhyTag::SetHeaderSinr (double value)
{
  m_headerSinr = value;
}

void
PhySimWifiPhyTag::SetPayloadSinr (double value)
{
  m_payloadSinr = value;
}

void
PhySimWifiPhyTag::SetOverallSinr (double value)
{
  m_overallSinr = value;
}

double
PhySimWifiPhyTag::GetPreambleSinr () const
{
  return m_preambleSinr;
}

double
PhySimWifiPhyTag::GetHeaderSinr () const
{
  return m_headerSinr;
}

double
PhySimWifiPhyTag::GetPayloadSinr () const
{
  return m_payloadSinr;
}

double
PhySimWifiPhyTag::GetOverallSinr () const
{
  return m_overallSinr;
}

void
PhySimWifiPhyTag::SetPathLoss (double value)
{
  m_pathLoss = value;
}

double
PhySimWifiPhyTag::GetPathLoss () const
{
  return m_pathLoss;
}

void
PhySimWifiPhyTag::SetTxPower (double value)
{
  m_txPower = value;
}

double
PhySimWifiPhyTag::GetTxPower () const
{
  return m_txPower;
}

void
PhySimWifiPhyTag::SetRxNetDevice (Ptr<NetDevice> device)
{
  m_rxNetDevice = device;
}

Ptr<NetDevice>
PhySimWifiPhyTag::GetRxNetDevice () const
{
  return m_rxNetDevice;
}

Ptr<NetDevice>
PhySimWifiPhyTag::GetTxNetDevice () const
{
  return m_txNetDevice;
}

void
PhySimWifiPhyTag::SetBackgroundNoise (itpp::cvec samples)
{
  m_bgNoise = samples;
}

itpp::cvec
PhySimWifiPhyTag::GetBackgroundNoise () const
{
  return m_bgNoise;
}

void
PhySimWifiPhyTag::SetShortTrainingSymbolCorrelations (std::vector<double> correlations)
{
  m_shortTrainingSymbolCorrelations = correlations;
}

std::vector<double>
PhySimWifiPhyTag::GetShortTrainingSymbolCorrelations () const
{
  return m_shortTrainingSymbolCorrelations;
}

void
PhySimWifiPhyTag::SetLongTrainingSymbolCorrelations (std::vector<double> correlations)
{
  m_longTrainingSymbolCorrelations = correlations;
}

std::vector<double>
PhySimWifiPhyTag::GetLongTrainingSymbolCorrelations () const
{
  return m_longTrainingSymbolCorrelations;
}

void
PhySimWifiPhyTag::SetCaptured ()
{
  m_captured = true;
}

bool
PhySimWifiPhyTag::IsCaptured () const
{
  return m_captured;
}

TypeId
PhySimWifiPhyTag::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
PhySimWifiPhyTag::GetSerializedSize () const
{
  return sizeof(m_txed)
         + sizeof(m_wifiPreamble)
         + sizeof(m_rxWifiMode)
         + sizeof(m_txWifiMode)
         + sizeof(m_duration)
         + sizeof(m_txDataBits)
         + sizeof(m_txSamples)
         + sizeof(m_rxSamples);
}

void
PhySimWifiPhyTag::Serialize (TagBuffer i) const
{
}

void
PhySimWifiPhyTag::Deserialize (TagBuffer i)
{
}

void
PhySimWifiPhyTag::Print (std::ostream &os) const
{
  os << "txed=" << m_txed;
  if (m_txed)
    {
      os << " wifiPreamble=" << m_wifiPreamble
         << " txWifiMode=" << m_txWifiMode
         << " rxWifiMode=" << m_rxWifiMode
         << " duration=" << m_duration
         << " txDataBits=" << m_txDataBits
         << " txSamples=" << m_txSamples;
    }
  os << " rxSamples=" << m_rxSamples;
}

std::ostream& operator<< (std::ostream& os, const PhySimWifiPhyTag& tag)
{
  tag.Print (os);
  return os;
}

} // namespace ns3
