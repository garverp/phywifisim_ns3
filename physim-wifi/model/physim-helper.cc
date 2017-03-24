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


#include "physim-helper.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-mode.h"
#include "ns3/log.h"
#include "physim-ofdm-symbolcreator.h"

NS_LOG_COMPONENT_DEFINE ("PhySimHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PhySimHelper);

// Short and long training symbols reference in IEEE notation
const itpp::cvec PhySimHelper::m_refShortSymbolCompleteIEEE =
  "0.0459988+0.0459988i -0.132444+0.00233959i -0.0134727-0.0785248i 0.142755-0.0126512i 0.0919975+0i 0.142755-0.0126512i -0.0134727-0.0785248i -0.132444+0.00233959i 0.0459988+0.0459988i 0.00233959-0.132444i -0.0785248-0.0134727i -0.0126512+0.142755i 0+0.0919975i -0.0126512+0.142755i -0.0785248-0.0134727i 0.00233959-0.132444i";
const itpp::cvec PhySimHelper::m_refLongSymbolCompleteIEEE =
  "-0.00512125+0.120325i 0.15625+0i -0.00512125-0.120325i 0.0397497-0.111158i 0.0968319+0.0827979i 0.0211118+0.0278859i 0.0598238-0.0877068i -0.115131-0.0551805i -0.038316-0.106171i 0.0975413-0.0258883i 0.0533377+0.00407633i 0.00098898-0.115005i -0.136805-0.0473798i 0.0244759-0.0585318i 0.0586688-0.014939i -0.0224832+0.160657i 0.119239-0.00409559i 0.0625-0.0625i 0.0369179+0.0983442i -0.0572063+0.0392986i -0.131263+0.0652272i 0.0822183+0.0923566i 0.0695568+0.014122i -0.0603101+0.0812861i -0.0564551-0.0218039i -0.0350413-0.150888i -0.121887-0.0165662i -0.127324-0.0205014i 0.0750737-0.0740404i -0.00280594+0.0537743i -0.0918876+0.115129i 0.0917165+0.105872i 0.0122846+0.0975996i -0.15625+0i 0.0122846-0.0975996i 0.0917165-0.105872i -0.0918876-0.115129i -0.00280594-0.0537743i 0.0750737+0.0740404i -0.127324+0.0205014i -0.121887+0.0165662i -0.0350413+0.150888i -0.0564551+0.0218039i -0.0603101-0.0812861i 0.0695568-0.014122i 0.0822183-0.0923566i -0.131263-0.0652272i -0.0572063-0.0392986i 0.0369179-0.0983442i 0.0625+0.0625i 0.119239+0.00409559i -0.0224832-0.160657i 0.0586688+0.014939i 0.0244759+0.0585318i -0.136805+0.0473798i 0.00098898+0.115005i 0.0533377-0.00407633i 0.0975413+0.0258883i -0.038316+0.106171i -0.115131+0.0551805i 0.0598238+0.0877068i 0.0211118-0.0278859i 0.0968319-0.0827979i 0.0397497+0.111158i -0.00512125+0.120325i";
const itpp::cvec PhySimHelper::m_refConjShortSymbolCompleteIEEE = conj (m_refShortSymbolCompleteIEEE);
const itpp::cvec PhySimHelper::m_refConjLongSymbolCompleteIEEE = conj (m_refLongSymbolCompleteIEEE);

// Short and long training symbols reference in IT++ notation
const itpp::cvec PhySimHelper::m_refShortSymbolComplete =
  "0.183995+0.183995i -1.05955+0.0187167i -0.107782-0.628198i 1.14204-0.101209i 0.73598+0i 1.14204-0.101209i -0.107782-0.628198i -1.05955+0.0187167i 0.36799+0.36799i 0.0187167-1.05955i -0.628198-0.107782i -0.101209+1.14204i 0+0.73598i -0.101209+1.14204i -0.628198-0.107782i 0.0187167-1.05955i";
const itpp::cvec PhySimHelper::m_refLongSymbolComplete =
  "-0.0334519+0.78596i 1.02062+0i -0.0334519-0.78596i 0.259644-0.726081i 0.632503+0.540834i 0.137902+0.18215i 0.390768-0.572898i -0.752034-0.360437i -0.250279-0.693505i 0.637137-0.169102i 0.348401+0.0266265i 0.00645999-0.751207i -0.893606-0.309484i 0.159876-0.382328i 0.383223-0.0975811i -0.14686+1.04941i 0.778866-0.0267523i 0.408248-0.408248i 0.241147+0.642381i -0.37367+0.256697i -0.857404+0.426062i 0.537048+0.60327i 0.454343+0.0922442i -0.393944+0.530959i -0.368763-0.142423i -0.228889-0.985599i -0.796163-0.10821i -0.831679-0.133914i 0.490379-0.48363i -0.0183284+0.351252i -0.600207+0.752018i 0.59909+0.691551i 0.0802426+0.637518i -1.02062+0i 0.0802426-0.637518i 0.59909-0.691551i -0.600207-0.752018i -0.0183284-0.351252i 0.490379+0.48363i -0.831679+0.133914i -0.796163+0.10821i -0.228889+0.985599i -0.368763+0.142423i -0.393944-0.530959i 0.454343-0.0922442i 0.537048-0.60327i -0.857404-0.426062i -0.37367-0.256697i 0.241147-0.642381i 0.408248+0.408248i 0.778866+0.0267523i -0.14686-1.04941i 0.383223+0.0975811i 0.159876+0.382328i -0.893606+0.309484i 0.00645999+0.751207i 0.348401-0.0266265i 0.637137+0.169102i -0.250279+0.693505i -0.752034+0.360437i 0.390768+0.572898i 0.137902-0.18215i 0.632503-0.540834i 0.259644+0.726081i -0.0334519+0.78596i";
const itpp::cvec PhySimHelper::m_refConjShortSymbolComplete = conj (m_refShortSymbolComplete);
const itpp::cvec PhySimHelper::m_refConjLongSymbolComplete = conj (m_refLongSymbolComplete);

// Define rate bits for signal header
const itpp::Array<itpp::bvec> PhySimHelper::m_rateBits = "{[1 1 0 1] [1 1 1 1] [0 1 0 1] [0 1 1 1] [1 0 0 1] [1 0 1 1] [0 0 0 1] [0 0 1 1]}";

const Ptr<PhySimBlockInterleaver> PhySimHelper::m_interleaver = CreateObject<PhySimBlockInterleaver>();
const Ptr<PhySimConvolutionalEncoder> PhySimHelper::m_convEncoder = CreateObject<PhySimConvolutionalEncoder>();
const Ptr<PhySimOFDMSymbolCreator> PhySimHelper::m_ofdmSymbolCreator = CreateObject<PhySimOFDMSymbolCreator>();
Ptr<PhySimChannelEstimator> PhySimHelper::m_estimator;
const Ptr<PhySimScrambler> PhySimHelper::m_scrambler = CreateObject<PhySimScrambler> ();

TypeId
PhySimHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimHelper")
    .SetParent<Object> ()
    .AddConstructor<PhySimHelper> ();
  return tid;
}

PhySimHelper::PhySimHelper ()
{
}

PhySimHelper::~PhySimHelper ()
{
}

double
PhySimHelper::GetOFDMSymbolSignalStrength (const itpp::cvec block)
{
  double energy = 0.0;
  for ( int32_t i = 0; i < block.size (); i++ )
    {
      energy += norm (block.get (i));
    }
  return ( energy / block.size () );
}

double
PhySimHelper::DbmToW (double dbm)
{
  double mW = pow (10.0,dbm / 10.0);
  return mW / 1000.0;
}

double
PhySimHelper::DbToRatio (double db)
{
  double ratio = pow (10.0,db / 10.0);
  return ratio;
}

double
PhySimHelper::WToDbm (double w)
{
  return 10.0 * log10 (w * 1000.0);
}

double
PhySimHelper::RatioToDb (double ratio)
{
  return 10.0 * log10 (ratio);
}

double
PhySimHelper::GetPowerDbm (uint8_t power, uint8_t nTxPower, double txPowerBaseDbm, double txPowerEndDbm)
{
  NS_ASSERT (txPowerBaseDbm <= txPowerEndDbm);
  NS_ASSERT (nTxPower > 0);
  double dbm = txPowerBaseDbm + power * (txPowerEndDbm - txPowerBaseDbm) / nTxPower;
  return dbm;
}

void
PhySimHelper::WindowFunction (itpp::cvec &input)
{
  // Sizes for short, long and data samples
  NS_ASSERT ( input.size () == 161 || input.size () == 81 || input.size () == 80 );
  // According to Equation 17-5 of the 802.11 Standard (page 600), the first and the last
  // entry (actually the last minus 1) are multiplied by 0.5, the rest is kept as it is
  input (0) *= 0.5;
  input (input.size () - 1) *= 0.5;
}

itpp::bvec
PhySimHelper::DecToBin (const uint32_t input, const uint32_t noBits)
{
  NS_ASSERT ( input >= 0 && input < pow (2, noBits)); // input must fit in no of bits
  itpp::bvec toReturn (noBits);
  toReturn.zeros ();
  uint32_t decimal = input;
  uint32_t remainder;
  uint32_t index = 0;
  do
    {
      remainder = decimal % 2;
      decimal = decimal / 2;
      toReturn (index++) = remainder;
    }
  while (decimal > 0);

  return toReturn;
}

itpp::bin
PhySimHelper::CalcEvenParity (const itpp::bvec& headerPart)
{
  uint32_t no_of_ones = 0;
  itpp::bin toReturn;
  for (int32_t i = 0; i < headerPart.size (); ++i)
    {
      if (headerPart (i) == 1)
        {
          ++no_of_ones;
        }
    }

  if ((no_of_ones % 2) == 0)
    {
      toReturn = 0;
    }
  else
    {
      toReturn = 1;
    }

  return toReturn;
}

bool
PhySimHelper::CheckSignalField (const itpp::bvec &input)
{
  NS_LOG_FUNCTION (input);
  NS_ASSERT ( input.size () == 24 );
  return (input (17) == CalcEvenParity ( input (0, 16)) );
}

itpp::bvec
PhySimHelper::CreateRateField (const WifiMode mode)
{

  /*
     * Defined elements:
     *    0 --> BPSK_1/2,
     *    1 --> BPSK_2/3,
     *    2 --> QPSK_1/2,
     *    3 --> QPSK_3/4,
     *    4 --> QAM16_1/2
     *    5 --> QAM16_3/4,
     *    6 --> QAM64_2/3,
     *    7 --> QAM64_3/4
     */

  uint8_t index = 0;
  enum WifiCodeRate codingRate = mode.GetCodeRate ();
  enum ModulationType modType = GetModulationType (mode);
  switch ( modType )
    {
    case PhySimHelper::BPSK:
      if (codingRate == WIFI_CODE_RATE_1_2)
        {
          index = 0;
        }
      else if (codingRate == WIFI_CODE_RATE_3_4)
        {
          index = 1;
        }
      else
        {
          NS_LOG_ERROR ("PhySimHelper::CreateRateField(): got an unknown coding rate for PhySimHelper::BPSK");
        }
      break;

    case PhySimHelper::QPSK:
      if (codingRate == WIFI_CODE_RATE_1_2)
        {
          index = 2;
        }
      else if (codingRate == WIFI_CODE_RATE_3_4)
        {
          index = 3;
        }
      else
        {
          NS_LOG_ERROR ("PhySimHelper::CreateRateField(): got an unknown coding rate for PhySimHelper::QPSK");
        }
      break;

    case PhySimHelper::QAM16:
      if (codingRate == WIFI_CODE_RATE_1_2)
        {
          index = 4;
        }
      else if (codingRate == WIFI_CODE_RATE_3_4)
        {
          index = 5;
        }
      else
        {
          NS_LOG_ERROR ("PhySimHelper::CreateRateField(): got an unknown coding rate for PhySimHelper::QAM16");
        }
      break;

    case PhySimHelper::QAM64:
      if (codingRate == WIFI_CODE_RATE_2_3)
        {
          index = 6;
        }
      else if (codingRate == WIFI_CODE_RATE_3_4)
        {
          index = 7;
        }
      else
        {
          NS_LOG_ERROR ("PhySimHelper::CreateRateField(): got an unknown coding rate for PhySimHelper::QAM64");
        }
      break;

    default:
      NS_LOG_ERROR ("PhySimHelper::CreateRateField(): got an unknown WifiMode::ModulationType");
    }

  NS_LOG_DEBUG ("PhySimHelper::CreateRateField() returning the following bits: " << m_rateBits (index));
  return m_rateBits (index);
}

bool
PhySimHelper::GetWifiMode (const itpp::bvec &bits, WifiMode &mode)
{
  if (bits == m_rateBits (0))
    {
      mode = WifiPhy::GetOfdmRate6Mbps ();
    }
  else if (bits == m_rateBits (1))
    {
      mode = WifiPhy::GetOfdmRate9Mbps ();
    }
  else if (bits == m_rateBits (2))
    {
      mode = WifiPhy::GetOfdmRate12Mbps ();
    }
  else if (bits == m_rateBits (3))
    {
      mode = WifiPhy::GetOfdmRate18Mbps ();
    }
  else if (bits == m_rateBits (4))
    {
      mode = WifiPhy::GetOfdmRate24Mbps ();
    }
  else if (bits == m_rateBits (5))
    {
      mode = WifiPhy::GetOfdmRate36Mbps ();
    }
  else if (bits == m_rateBits (6))
    {
      mode = WifiPhy::GetOfdmRate48Mbps ();
    }
  else if (bits == m_rateBits (7))
    {
      mode = WifiPhy::GetOfdmRate54Mbps ();
    }
  else
    {
      // return false if rate field is invalid
      return false;
    }
  return true;
}

uint32_t
PhySimHelper::GetNCBPS (const WifiMode mode)
{
  NS_ASSERT ( mode.GetModulationClass () == WIFI_MOD_CLASS_OFDM );
  if ( (mode.GetBandwidth () == 10000000 && mode.GetPhyRate () ==  6000000)
       || (mode.GetBandwidth () == 20000000 && mode.GetPhyRate () == 12000000))
    {
      return 48;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetPhyRate () == 12000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetPhyRate () == 24000000))
    {
      return 96;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetPhyRate () == 24000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetPhyRate () == 48000000))
    {
      return 192;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetPhyRate () == 36000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetPhyRate () == 72000000))
    {
      return 288;
    }
  NS_LOG_DEBUG ("PhySimHelper::GetNCBPS() fatal error: no branch caught the case");
  NS_ASSERT ( false );
  return 0;
}

uint32_t
PhySimHelper::GetNDBPS (const WifiMode mode)
{
  NS_ASSERT ( mode.GetModulationClass () == WIFI_MOD_CLASS_OFDM );
  if ( (mode.GetBandwidth () == 10000000 && mode.GetDataRate () == 3000000)
       || (mode.GetBandwidth () == 20000000 && mode.GetDataRate () == 6000000))
    {
      return 24;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetDataRate () == 4500000)
            || (mode.GetBandwidth () == 20000000 && mode.GetDataRate () == 9000000))
    {
      return 36;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetDataRate () ==  6000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetDataRate () == 12000000))
    {
      return 48;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetDataRate () ==  9000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetDataRate () == 18000000))
    {
      return 72;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetDataRate () == 12000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetDataRate () == 24000000))
    {
      return 96;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetDataRate () == 18000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetDataRate () == 36000000))
    {
      return 144;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetDataRate () == 24000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetDataRate () == 48000000))
    {
      return 192;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetDataRate () == 27000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetDataRate () == 54000000))
    {
      return 216;
    }
  NS_LOG_DEBUG ("PhySimHelper::GetNDBPS() fatal error: no branch caught the case");
  NS_ASSERT ( false );
  return 0;
}

enum PhySimHelper::ModulationType
PhySimHelper::GetModulationType (const WifiMode mode)
{
  NS_ASSERT ( mode.GetModulationClass () == WIFI_MOD_CLASS_OFDM );
  if ( (mode.GetBandwidth () == 10000000 && mode.GetPhyRate () ==  6000000)
       || (mode.GetBandwidth () == 20000000 && mode.GetPhyRate () == 12000000))
    {
      return PhySimHelper::BPSK;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetPhyRate () == 12000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetPhyRate () == 24000000))
    {
      return PhySimHelper::QPSK;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetPhyRate () == 24000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetPhyRate () == 48000000))
    {
      return PhySimHelper::QAM16;
    }
  else if ( (mode.GetBandwidth () == 10000000 && mode.GetPhyRate () == 36000000)
            || (mode.GetBandwidth () == 20000000 && mode.GetPhyRate () == 72000000))
    {
      return PhySimHelper::QAM64;
    }
  NS_LOG_DEBUG ("PhySimHelper::GetModulationType() fatal error: no branch caught the case");
  NS_ASSERT ( false );
}

Time
PhySimHelper::CalculateTxDuration (uint32_t size, WifiMode payloadMode, WifiPreamble preamble, Time symbolTime)
{
  uint32_t tailNservice = 22;   // service field (16 bits) plus tail bits (6 bits)
  uint32_t ndbps = PhySimHelper::GetNDBPS (payloadMode);
  uint32_t padding = ndbps - (tailNservice + (8 * size)) % (ndbps);

  uint64_t delay = 0;
  // Preamble has 4 symbols
  delay += (4 * symbolTime.GetNanoSeconds ());
  // Signal header is only 1 symbol
  delay += symbolTime.GetNanoSeconds ();
  // number of data symbols times symbol time
  uint32_t numSymbols = ceil ( (tailNservice + (8 * size) + padding) / ndbps); // see Equation 17-11
  delay += symbolTime.GetNanoSeconds () * numSymbols;

  return NanoSeconds (delay);
}

bool
PhySimHelper::isHeaderDecodable(itpp::cvec samples, Ptr<const PhySimWifiPhyTag> tag)
{
  // create channel estimator instance if not yet existing
  if (m_ofdmSymbolCreator->GetChannelEstimator() == 0)
    {
      m_ofdmSymbolCreator->SetChannelEstimator("ns3::PhySimChannelFrequencyOffsetEstimator");
      m_estimator = m_ofdmSymbolCreator->GetChannelEstimator ();
    }

  // first reset the channel estimator
  m_estimator->Reset ();

  // get initial estimate first
  itpp::cvec shortSymbols = samples(0, 319);
  double estimate = m_estimator->GetInitialChannelEstimation ( shortSymbols );
  // use only the header in the following
  itpp::cvec header = samples(320,399);
  header = m_estimator->ApplyEstimateFromTrainingSequence (header, estimate, 0);

  // Demodulate
  m_ofdmSymbolCreator->SetModulationType (PhySimHelper::BPSK, 2);
  itpp::vec demodulated = (m_ofdmSymbolCreator->DeModulate (header, true));

  // De-Interleave
  m_interleaver->SetWifiMode (WifiPhy::GetOfdmRate6Mbps ());
  itpp::vec deInterleavedHeader = m_interleaver->DeinterleaveBlock (demodulated);

  // Viterbi-Decode
  m_convEncoder->SetCodingRate (WIFI_CODE_RATE_1_2);
  itpp::bvec decoded = m_convEncoder->Decode (deInterleavedHeader);

  // if parity bit is o.k.
  if (PhySimHelper::CheckSignalField (decoded))
    {
      // Determine WifiMode and length...
      WifiMode mode;
      PhySimHelper::GetWifiMode (decoded (0, 3), mode);
      uint32_t length = itpp::bin2dec ( decoded (5, 16), false );
      // ... and compare it against what has been used for transmission
      if (length < 1 || length > 4095 || !(mode == tag->GetTxWifiMode()))
        {
          return false;
        }
      else
        {
          return true;
        }
    }
  else
    {
      return false;
    }
}

bool
PhySimHelper::isPayloadDecodable(itpp::cvec samples, Ptr<const PhySimWifiPhyTag> tag)
{
  // create channel estimator instance if not yet existing
  if (m_ofdmSymbolCreator->GetChannelEstimator() == 0)
    {
      m_ofdmSymbolCreator->SetChannelEstimator("ns3::PhySimChannelFrequencyOffsetEstimator");
      m_estimator = m_ofdmSymbolCreator->GetChannelEstimator ();
    }

  // first reset the channel estimator
  m_estimator->Reset ();

  // get initial estimate first
  itpp::cvec shortSymbols = samples(0, 319);
  double estimate = m_estimator->GetInitialChannelEstimation ( shortSymbols );

  // use only the payload in the following
  itpp::cvec payload = samples(400, samples.size() - 1);
  payload = m_estimator->ApplyEstimateFromTrainingSequence (payload, estimate, 1);

  // calculate length, get Wifi mode, etc...
  WifiMode mode = tag->GetTxWifiMode ();
  uint32_t length = tag->GetTxedDataBits ().size();
  uint32_t tailNservice = 22;   // service field (16 bits) plus tail bits (6 bits)
  uint32_t ndbps = PhySimHelper::GetNDBPS (mode);
  uint32_t padding = ndbps - (tailNservice + length) % (ndbps);
  uint32_t numSymbols = ceil ( (tailNservice + length + padding) / ndbps); // see Equation 17-11

  // De-Interleave and demodulate (we are NOT in the signal header anymore)
  NS_ASSERT ( payload.size () % 80 == 0 );

  m_interleaver->SetWifiMode (mode);
  m_ofdmSymbolCreator->SetModulationType (PhySimHelper::GetModulationType (mode), mode.GetConstellationSize ());

  // There are 80 time samples per OFDM symbol
  //const uint32_t sampleSize = 80;
  int32_t startIndex = 0;
  int32_t endIndex = 80 - 1;
  int32_t symbolNo = 0;
  itpp::vec encodedScrambledData;
  itpp::vec block;

  while (endIndex < payload.size ())
    {
	  symbolNo++; // next block
	  itpp::vec demodulated = m_ofdmSymbolCreator->DeModulate ( payload(startIndex, endIndex), symbolNo);
	  block = m_interleaver->DeinterleaveBlock (demodulated);
	  encodedScrambledData.ins ( encodedScrambledData.size(), block );
	  startIndex += 80;
	  endIndex += 80;
    }

  // Do Viterbi decoding
  m_convEncoder->SetCodingRate (mode.GetCodeRate ());
  itpp::bvec scrambledData = m_convEncoder->Decode (encodedScrambledData);

  // Descramble
  itpp::bvec initialState = scrambledData (0, 6); // read the scrambling sequence from decoded DATA
  itpp::bvec DATA = m_scrambler->DeScramble ( scrambledData ( 7, scrambledData.size () - 1 ), initialState );

  // Calculate no. of symbols needed
  uint32_t nDATA = numSymbols * ndbps; // Equation 17-12 - length of data field
  itpp::bvec finalData = DATA (16, nDATA - padding - 6 - 1);

  return (finalData == tag->GetTxedDataBits ());
}

} // namespace ns3

