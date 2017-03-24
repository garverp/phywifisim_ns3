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

#ifndef PHYSIM_HELPER_H
#define PHYSIM_HELPER_H

#include "ns3/object.h"
#include "ns3/wifi-mode.h"
#include <itpp/itcomm.h>
#include "physim-wifi-phy-tag.h"
#include "physim-blockinterleaver.h"
#include "physim-convolutional-encoder.h"
#include "physim-scrambler.h"

namespace ns3 {

class PhySimOFDMSymbolCreator;
class PhySimChannelEstimator;

/**
 * \brief
 */
class PhySimHelper : public Object
{
public:
  /**
   * The different modulation types that can be used in OFDM-based IEEE 802.11 communication.
   */
  enum ModulationType
  {
    BPSK,
    QPSK,
    QAM16,
    QAM64
  };

  static TypeId GetTypeId (void);

  PhySimHelper ();
  virtual ~PhySimHelper ();

  /**
   * Calculates the energy of the sequence of complex time samples stored within the given block.
   * This function is typically used to compute the OFDM symbol signal strength, e.g. for SINR
   * calculations. More precisely, it will return normalized sum S:= |x_{i}|
   * \param block The vector of complex time samples over which to calculate the average signal strength
   * \return      The normalized/average energy over the whole block of complex time samples
   */
  static double GetOFDMSymbolSignalStrength (const itpp::cvec block);
  /**
   * Calculates the power of complex time sample, which is actually the standard norm on complex values.
   */
  static double CalcPower (std::complex<double> num);
  static double DbmToW (double dbm);
  static double DbToRatio (double db);
  static double WToDbm (double w);
  static double RatioToDb (double ratio);
  static double GetPowerDbm (uint8_t power, uint8_t nTxPower, double txPowerBaseDbm, double txPowerEndDbm);

  /**
   * The window function that is specified in the IEEE 802.11 standard in Section 17
   */
  static void WindowFunction (itpp::cvec &input);
  /**
   * Converts from an integer representation into a bit vector representation, e.g. when converting a sequence
   * of uint8_t values into a sequence of bits (which is necessary when converting the payload of a packet to
   * the IT++ bit vector format for further processing)
   * \param input  The integer that shall be converted into binary format
   * \param noBits The number of bits (or the length of the resulting bit vector) that shall be used
   * \return       The bit vector that represents the integer, which the least significant bit on the left side
   */
  static itpp::bvec DecToBin (const uint32_t input, const uint32_t noBits);
  /**
   * Calculates the parity bit for the signal header
   * \param headerPart The bit vector that represents the signal header of the packet
   * \return           The parity bit
   */
  static itpp::bin CalcEvenParity (const itpp::bvec& headerPart);
  /**
   * Performs a plausibility check on the decoded signal header fields, i.e. checks whether the parity bit
   * is correct or not.
   * \param input The decoded signal header in bit vector presentation
   * \return      Result of the parity check
   */
  static bool CheckSignalField (const itpp::bvec &input);
  /**
   * Creates the rate field that is inserted within the signal header, depending on the to be used WiFi mode
   * \param mode The to be used WiFi mode
   * \return     The rate field as a bit vector
   */
  static itpp::bvec CreateRateField (const WifiMode mode);
  /**
   * Returns whether the given rate field corresponds to a known WiFi mode and if yes, stores the instance of this WiFi mode
   * object in the second parameter.
   * \param bits The bit vector that represents the rate field
   * \param mode The WifiMode object in which to save the detected WiFi mode
   * \return     Result of the mode detection. True if the bits corresponded to a known WiFi mode, false otherwise.
   */
  static bool GetWifiMode (const itpp::bvec &bits, WifiMode &mode);

  /**
   * Returns the number of coded bits per OFDM symbol for the given WiFi mode
   * \param mode The WiFi mode for which the number of bits is requested
   * \return     The number of coded bits per OFDM symbol
   */
  static uint32_t GetNCBPS (const WifiMode mode);
  /**
   * Returns the number of data bits per OFDM symbol for the given WiFi mode
   * \param mode The WiFi mode for which the number of bits is requested
   * \return     The number of data bits per OFDM symbol
   */
  static uint32_t GetNDBPS (const WifiMode mode);
  /**
   * Returns the underlying ModulationType that is used by the given WiFi mode. I.e., it should return
   * BPSK for the lowest two data rates, QPSK, for the next two higher rates, QAM16 for the subsequent
   * two higher rates and QAM64 for the two fastest data rates.
   * Note: currently it supports only 10 and 20 MHz channels
   * \param mode The WiFi mode for which to determine the ModulationType
   * \return     The detected modulation type
   */
  static enum ModulationType GetModulationType (const WifiMode mode);
  /**
   * Calculates the duration of a packet transmission for a given configuration.
   * \param     size            The packet size in bytes
   * \param     payloadMode     The to be used modulation scheme (or Wifi mode)
   * \param     preamble        The to be used preamble format
   * \param     symbolTime      The IEEE 802.11 symbol time (4 microseconds for 20 MHz channels,
   *                            8 microseconds for 10 MHz channels
   * \return                    The duration of the packet transmission
   */
  static Time CalculateTxDuration (uint32_t size, WifiMode payloadMode, WifiPreamble preamble, Time symbolTime);
  /**
   * Checks whether the given time samples of a packet contain a successfully decodeable signal header.
   * \param     samples         A sequence of complex time samples with a length of at least 400 samples.
   * \param     tag             A valid PhySimWifiTag as a reference in order to check whether the decoded
   *                            signal header is correct or not.
   * \return                    Whether or not the signal header could be decoded successfully or not
   */
  static bool isHeaderDecodable(itpp::cvec samples, Ptr<const PhySimWifiPhyTag> tag);
  /**
   * Checks whether the given time samples of a packet contain a successfully decodeable payload.
   * \param     samples         A sequence of complex time samples with a length of at least 480 samples.
   * \param     tag             A valid PhySimWifiTag as a reference in order to check whether the decoded
   *                            payload is correct or not.
   * \return                    Whether or not the payload could be decoded successfully or not
   */
  static bool isPayloadDecodable(itpp::cvec samples, Ptr<const PhySimWifiPhyTag> tag);

  static const Ptr<PhySimBlockInterleaver> m_interleaver;
  static const Ptr<PhySimConvolutionalEncoder> m_convEncoder;
  static const Ptr<PhySimOFDMSymbolCreator> m_ofdmSymbolCreator;
  static Ptr<PhySimChannelEstimator> m_estimator;
  static const Ptr<PhySimScrambler> m_scrambler;

  // Short and long training symbols reference in IEEE notation (and their conjugates)
  static const itpp::cvec m_refShortSymbolCompleteIEEE;
  static const itpp::cvec m_refLongSymbolCompleteIEEE;
  static const itpp::cvec m_refConjShortSymbolCompleteIEEE;
  static const itpp::cvec m_refConjLongSymbolCompleteIEEE;

  // Short and long training symbols reference in IT++ notation (and their conjugates)
  static const itpp::cvec m_refShortSymbolComplete;
  static const itpp::cvec m_refLongSymbolComplete;
  static const itpp::cvec m_refConjShortSymbolComplete;
  static const itpp::cvec m_refConjLongSymbolComplete;

  static const itpp::Array<itpp::bvec> m_rateBits;

};

} // namespace ns3

#endif /* PHYSIM_HELPER_H */
