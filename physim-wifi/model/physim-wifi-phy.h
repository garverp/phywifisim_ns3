/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA, 2009-2010 Stylianos Papanastasiou, Jens Mittag, 2011 Michele Segata
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
 * Based on yans-wifi-phy.h by:
 *      Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Major modifications by:
 *      Jens Mittag <jens.mittag@kit.edu>
 *      Stylianos Papanastasiou <stylianos@gmail.com>
 * Introduction of triangular random variable for frequency-offset generation by:
 *      Michele Segata <msegata@disi.unitn.it>
 */

#ifndef PHYSIM_WIFI_PHY_H
#define PHYSIM_WIFI_PHY_H

#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/random-variable.h"
#include "ns3/net-device.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"
#include "ns3/wifi-phy-standard.h"
#include "physim-ofdm-symbolcreator.h"
#include "physim-convolutional-encoder.h"
#include "physim-blockinterleaver.h"
#include "physim-scrambler.h"
#include "physim-interference-helper.h"
#include "physim-signal-detector.h"
#include "physim-channel-estimator.h"
#include "physim-wifi-phy-tag.h"

#include <itpp/itcomm.h>

namespace ns3 {

class PhySimWifiChannel;
class NetDevice;
class PhySimWifiPhyStateHelper;

/**
 * \brief An accurate IEEE 802.11 physical layer implementation for purely OFDM-based communication (802.11a and p, partially g mode).
 *        The model simulates the complete set of signal processing that is necessary to transform the bits constituting a packet into
 *        a sequence of complex time samples and vice versa (including signal detection and channel estimation). By doing so, the abstraction
 *        of a packet (having only a length/duration and a fixed average received power associated to it) is broken up, and thus channel
 *        and/or interference effects that have an impact on the reception performance are modeled appropriately.
 *
 * The class implements the ns3::WifiPhy interface and can be used as a drop-in replacement for the default/standard YansWifiPhy
 * implementation of the official ns-3 WiFi module. However, 1) instead of using the classes WifiChannel, InterferenceHelper and derivatives of
 * PropagationLossModel, one has to use
 *
 * - PhySimWifiChannel
 * - PhySimInterferenceHelper
 * - PhySimPropagationLossModel
 *
 * and 2) compared to the YansWifiPhy implementation, some features are not supported yet, e.g. multiple channel operations and channel
 * switching. Similar to the YansWifiHelper that connects everything together, there is also an equivalent PhySimWifiHelper that does the same
 * job.
 *
 * This classed is based on the functionality provided by different helper classes and interfaces, which implement the logic of the major signal
 * processing steps for frame transmission and frame reception, namely
 *
 * - PhySimBlockInterleaver
 * - PhySimScrambler
 * - PhySimConvolutionalEncoder
 * - PhySimOFDMSymbolCreator
 * - PhySimChannelEstimator as an interface for
 *   - PhySimSimpleChannelEstimator
 *   - PhySimChannelFrequencyOffsetEstimator
 * - PhySimSignalDetector
 *
 * In addition, there are helper modules which support the implementation of the physical layer state machine, the interference management and
 * the cross-layer information handling, in particular
 *
 * - PhySimHelper for static methods such as the calculation of the OFDM symbol signal strength or similar
 * - PhySimInterferenceHelper
 * - PhySimWifiPhyStateHelper
 * - PhySimWifiPhyTag
 *
 * The entry points for interaction with the MAC layer and the wireless channel are PhySimWifiPhy::SendPacket and PhySimWifiPhy::StartReceivePacket
 *
 */
class PhySimWifiPhy : public WifiPhy
{
public:
  enum State
  {
    /**
     * The PHY layer is synchronized upon a packet (only preamble was detected and coarse offset synchronization was successful).
     **/
    SYNCING,
    /**
     * The PHY layer is synchronized upon a packet and has decoded the signal header of the frame successfully
     **/
    RX,
    /**
     *  The PHY layer is transmitting a packet.
     */
    TX,
    /**
     * The PHY layer is in idle mode.
     **/
    IDLE,
    /**
     * The PHY layer is not synchronized, but the detected energy is above CCA threshold
     **/
    CCA_BUSY
  };

  enum ErrorReason
  {
    /*
     * Packet has been dropped because receiver was already synchronized to a different packet
     */
    STATE_SYNC,
    /*
     * Packet has been dropped because receiver was already in (or switched to) reception state
     */
    STATE_RX,
    /*
     * Packet has been dropped because the receiver was in (or switched to) transmission state
     */
    STATE_TX,
    /*
     * Packet has been dropped because a different packet has been captured in favor of this one
     */
    CAPTURE,
    /*
     * Packet has been dropped because the signal processing did not succeed
     */
    PROCESSING,
    /*
     * Packet has been dropped because of insufficient energy
     */
    INSUFFICIENT_ENERGY
  };

  static TypeId GetTypeId (void);

  PhySimWifiPhy ();
  virtual ~PhySimWifiPhy ();

  /**
   * Sets the wireless channel to which this PHY is connected to
   * \param channel The wireless channel object
   */
  void SetChannel (Ptr<PhySimWifiChannel> channel);
  /**
   * Informs the physical layer that a new frame is 'arriving', in the sense that the first signal has propagated through the ether.
   * That does not imply that a reception process is started, instead, an EndPreamble event is scheduled, at which it will be checked
   * whether the preamble could be detected successfully and whether to start a reception process.
   * \param packet The pointer to the packet, which arrives at this PHY
   * \param tag    The pointer to the associated tag with all the meta-information
   */
  void StartReceivePacket (Ptr<Packet> packet,
                           Ptr<PhySimWifiPhyTag> tag);
  void SetDevice (Ptr<Object> device);
  void SetMobility (Ptr<Object> mobility);
  Ptr<Object> GetDevice (void) const;
  Ptr<Object> GetMobility (void);

  virtual double GetTxPowerStart (void) const;
  virtual double GetTxPowerEnd (void) const;
  virtual uint32_t GetNTxPower (void) const;
  virtual void SetReceiveOkCallback (WifiPhy::RxOkCallback callback);
  virtual void SetReceiveErrorCallback (WifiPhy::RxErrorCallback callback);
  /**
   * Serves the transmission requests coming from upper layers, in the sense that the packet is transformed from packet to bit, from bit to
   * signal level and then put on the wireless channel.
   * \param packet       The packet that shall be sent
   * \param mode         The WiFi mode that indicates at wich data rate the data bits shall be encoded
   * \param preamble     The preamble type that shall be used
   * \param txPowerLevel The transmission power level to be used for this transmission
   */
  virtual void SendPacket (Ptr<const Packet> packet, WifiMode mode, enum WifiPreamble preamble, uint8_t txPowerLevel);
  virtual void RegisterListener (WifiPhyListener *listener);
  /**
   * Returns whether the state of the physical layer is currently CCA_BUSY
   */
  virtual bool IsStateCcaBusy (void);
  /**
   * Returns whether the state of the physical layer is currently IDLE
   */
  virtual bool IsStateIdle (void);
  /**
   * Returns whether the state of the physical layer is currently BUSY, i.e. not IDLE
   */
  virtual bool IsStateBusy (void);
  /**
   * Returns whether the state of the physical layer is currently RX
   */
  virtual bool IsStateRx (void);
  /**
   * Returns whether the state of the physical layer is currently TX
   */
  virtual bool IsStateTx (void);
  /**
   * Returns whether the state of the physical layer is currently SWITCHING. Please not that this state is not yet supported,
   * and has to be added in the future once multiple channels and channel switching are available.
   */
  virtual bool IsStateSwitching (void);
  /**
   * Returns the remaining time until the next state change will happen
   */
  virtual Time GetStateDuration (void);
  /**
   * Returns the remaining time until the state will switch back to IDLE again
   */
  virtual Time GetDelayUntilIdle (void);
  /**
   * Returns the timestamp of the last RX period
   */
  virtual Time GetLastRxStartTime (void) const;
  virtual Time CalculateTxDuration (uint32_t size, WifiMode payloadMode, enum WifiPreamble preamble) const;
  virtual uint32_t GetNModes (void) const;
  virtual WifiMode GetMode (uint32_t mode) const;
  /**
   * Calculates the necessary SNR, under which for a given WiFi mode a desired BER can be achieved.
   * Note: This is not possible to compute anymore, since this physical layer implementation is not based on
   * statistics, but checks explicitly whether all bits are decoded correctly or not. Consequently, this method
   * returns always zero, no matter which parameters are given.
   * \param txMode The WiFi mode for which the minimum SINR is requested
   * \param ber    The maximum BER that is accepted
   * \return       The calculated SINR. So far, we always return 0.0
   */
  virtual double CalculateSnr (WifiMode txMode, double ber) const;
  /**
   * Set the channel number to use for transmissions. This sets the center frequency at which is operated and
   * the physical channel over which packets are sent and received. The numbering scheme that is used is the same
   * as in the IEEE standard.
   */
  virtual void SetChannelNumber (uint16_t id);
  /**
   * Returns the currently used channel number.
   */
  virtual uint16_t GetChannelNumber () const;
  /**
   * Configure the physical layer according to a specific WiFi standard, e.g. according to IEEE 802.11a with 20 MHz channel spacing
   * or IEEE 802.11p CCH
   */
  virtual void ConfigureStandard (enum WifiPhyStandard standard);
  /**
   * Returns the logical wireless channel object that multiplexes multiple PhySimWifiPhy objects
   */
  virtual Ptr<WifiChannel> GetChannel (void) const;

  /**
   * Sets the to be used channel estimator implementation, by specifying the TypeId name as string
   * \param type The TypeId name of the channel estimator implementation to be used
   */
  void SetChannelEstimator (std::string type);
  /**
   * Sets the to be used signal detector implementation, by specifying the TypeId name as string
   * \param type The TypeId name of the signal detector implementation to be used
   */
  void SetSignalDetector (std::string type);
  /**
   * Set the OFDM symbol time. This is necessary to indidcate whether we are operating in a 20 MHz, 10 MHz or 5 MHz channel.
   * \param duration The OFDM symbol time duration
   */
  void SetSymbolTime (Time duration);
  /**
   * Reset the RNG flag, which triggers an itpp::RNG_reset during the next frame construction process
   */
  static void ResetRNG ();
  /**
   * Resets all cached data structures, e.g. the once created time samples of the OFDM preamble
   */
  static void ClearCache ();

private:
  PhySimWifiPhy (const PhySimWifiPhy &o);
  virtual void DoDispose (void);
  void Configure80211a (void);
  void Configure80211g (void);
  void Configure80211a_10Mhz (void);
  void Configure80211a_5Mhz (void);
  void Configure80211p_CCH (void);
  void Configure80211p_SCH (void);

  /**
   * The callback that is called whenever the end of a preamble is "reached". At such events, the physical layer
   * implementation will perform signal detection, i.e. it will try to lock on to the signal, time synchronization
   * and a first coarse (frequency offset) channel estimation. If detection was successful, an EndHeader event in order
   * to start the next reception stage.
   * \param packet The packet for which this event was scheduled
   * \param tag    The corresponding tag with all the meta-information, most importantly with the vector of the
   *               complex time samples
   * \param event  The NS-3 event identifier of this event
   */
  void EndPreamble (Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag, Ptr<PhySimInterferenceHelper::Event> event);
  /**
   * The callback that is called whenever the end of a signal header is "reached". At such events, the physical layer
   * implementation will decode the OFDM symbol that represents the signal header with the lowest data rate in order
   * to determine the decoding settings for the payload and whether the data passes all integrity checks (e.g. parity)
   * \param packet The packet for which this event was scheduled
   * \param tag    The corresponding tag with all the meta-information, most importantly with the vector of the
   *               complex time samples
   * \param event  The NS-3 event identifier of this event
   */
  void EndHeader (Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag, Ptr<PhySimInterferenceHelper::Event> event);
  /**
   * The callback that is called whenever the end of a packet is "reached". At such events, the physical layer
   * implementation will use the detected reception settings from the former EndHeader event to decode the data
   * bits and check whether those received data bits are equal to the transmitted data bits. If yes, the reception
   * is successful, if not, an RxError is indicated.
   */
  void EndRx (Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag, Ptr<PhySimInterferenceHelper::Event> event);

  itpp::cvec ConstructPreamble ();
  itpp::cvec ConstructSignalHeader (uint32_t length, const WifiMode mode);
  itpp::cvec ConstructData (const itpp::bvec& bits, const WifiMode mode);
  itpp::bvec DeconstructData (Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag);
  bool ScanSignalField (Ptr<PhySimWifiPhyTag> tag, const itpp::cvec &input);

  itpp::cvec InterleaveAndModulate (const itpp::bvec &encodedScrambledData, const WifiMode mode);
  itpp::cvec InterleaveAndModulateBlock (const itpp::bvec& input, const uint32_t index);
  itpp::vec DeinterleaveAndModulate (const itpp::cvec &input, const WifiMode mode);
  itpp::vec DeinterleaveAndModulateBlock (const itpp::cvec& input, uint32_t symbolNo);

  void CancelAllRunningEndPreambleEvents (enum PhySimWifiPhy::ErrorReason reason);
  void CancelRunningEndHeaderEvent (enum PhySimWifiPhy::ErrorReason reason);
  void CancelRunningEndRxEvent (enum PhySimWifiPhy::ErrorReason reason);

  /**
   * This method looks into the future and checks whether there will be a CCA_BUSY at some point in the future (maybe
   * because the signal strength will exceed the CcaModelThreshold due to fading). If yes, it will schedule a StartCcaBusy event
   * for that timestamp. If no, nothing is happening here.
   */
  void CheckForNextCcaBusyStart ();
  /**
   * The callback that is called whenever a new CCA_BUSY phase shall start. If a CCA_BUSY period is already active, i.e. there
   * is already and EndCcaBusy event scheduled, nothing is happening. If no CCA_BUSY period is active, the physical layer is switched
   * to CCA_BUSY and an EndCcaBusy event is scheduled.
   */
  void StartCcaBusy ();
  /**
   * The callback that is called whenever a running CCA_BUSY phase shall terminate. If we are not already in RX mode, a check for a possible
   * next CCA_BUSY phase is triggered in order to not miss any CCA_BUSY periods.
   */
  void EndCcaBusy ();
  /**
   * This method checks whether the physical layer should consider the given packet for capture or not. In case packet capture is enabled,
   * this method is called during PhySimWifiPhy::StartReceivePacket whenever the physical layer is already in SYNC or RX state. It will check
   * whether the SINR of this packet is sufficiently large (threshold is defined through the CaptureThreshold attribute of the PhySimWifiPhy
   * class). If this is the case, the already running EndHeader or EndRx event will be canceled and a new EndPreamble event will scheduled for
   * the captured packet.
   */
  bool CapturePacket (Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag, Ptr<PhySimInterferenceHelper::Event> event);

private:
  typedef std::vector<WifiMode> Modes;

  double   m_ccaMode1ThresholdDbm;
  double   m_edThresholdDbm;
  double   m_txGainDb;
  double   m_rxGainDb;
  double   m_txPowerBaseDbm;
  double   m_txPowerEndDbm;
  uint32_t m_nTxPower;

  /**
   * The energy per subcarrier is normalised to 1 as per the 802.11 standard and IT++
   * conventions. The energy per OFDM symbols should then be 52 as 52 subcarriers are
   * utilised. Hence the power per sample is 52/64. When converting to time
   * samples (with the IFFT) the power gets scaled by a factor of 1/64. So, in order to
   * get unit power per sample we should then multiply the resulting time samples
   * (after the IFFT) by sqrt(64*64/52). This is what the m_normFactor does.
   */
  static const double m_normFactor;
  static bool m_rngReset;

  Ptr<PhySimWifiChannel> m_channel;
  Ptr<Object> m_device;
  Ptr<Object> m_mobility;

  /**
   * This vector holds the set of transmission modes that this
   * WifiPhy(-derived class) can support. In conversation we call this
   * the DeviceRateSet (not a term you'll find in the standard), and
   * it is a superset of standard-defined parameters such as the
   * OperationalRateSet, and the BSSBasicRateSet (which, themselves,
   * have a superset/subset relationship).
   *
   * Mandatory rates relevant to this WifiPhy can be found by
   * iterating over this vector looking for WifiMode objects for which
   * WifiMode::IsMandatory() is true.
   */
  WifiModeList m_deviceRateSet;

  std::map<Ptr<PhySimInterferenceHelper::Event>, EventId> m_endPreambleEvents;
  EventId m_endHeaderNs3Event;
  EventId m_endRxNs3Event;
  Ptr<PhySimInterferenceHelper::Event> m_endHeaderPhySimEvent;
  Ptr<PhySimInterferenceHelper::Event> m_endRxPhySimEvent;

  EventId m_startCcaBusyEvent;
  EventId m_endCcaBusyEvent;
  UniformVariable m_random;
  WifiPhyStandard m_standard;
  Ptr<PhySimWifiPhyStateHelper> m_state;
  Ptr<PhySimInterferenceHelper> m_interference;

  itpp::cvec m_shortSymbol;
  itpp::cvec m_longSymbol;
  Ptr<PhySimOFDMSymbolCreator> m_ofdmSymbolCreator;
  Ptr<PhySimConvolutionalEncoder> m_convEncoder;
  Ptr<PhySimBlockInterleaver> m_interleaver;
  Ptr<PhySimScrambler> m_scrambler;
  Ptr<PhySimSignalDetector> m_signalDetector;
  Ptr<PhySimChannelEstimator> m_estimator;

  double m_captureThreshold;

  bool m_capture;
  bool m_normalizeOfdmBlocks;
  bool m_calculateHeaderSinr;
  bool m_calculatePayloadSinr;
  bool m_calculateOverallSinr;
  Time m_symbolTime;
  uint32_t m_txCenterFreqTolerance;
  double m_sampleTime;
  double m_frequency;

    // Traces to signal what is going on at StartReceive
  TracedCallback<Ptr<const Packet>, Ptr<const PhySimWifiPhyTag> > m_startRxTrace;
  TracedCallback<Ptr<const Packet>, Ptr<const PhySimWifiPhyTag>, enum PhySimWifiPhy::ErrorReason > m_startRxErrorTrace;

    // Traces to signal a success at the end of an event/stage
  TracedCallback<Ptr<const Packet>, Ptr<const PhySimWifiPhyTag> > m_preambleOkTrace;
  TracedCallback<Ptr<const Packet>, Ptr<const PhySimWifiPhyTag> > m_headerOkTrace;
  TracedCallback<Ptr<const Packet>, Ptr<const PhySimWifiPhyTag> > m_rxOkTrace;

    // Traces to signal a failure at the end of an event/stage
  TracedCallback<Ptr<const Packet>, Ptr<const PhySimWifiPhyTag>, enum PhySimWifiPhy::ErrorReason > m_preambleErrorTrace;
  TracedCallback<Ptr<const Packet>, Ptr<const PhySimWifiPhyTag>, enum PhySimWifiPhy::ErrorReason > m_headerErrorTrace;
  TracedCallback<Ptr<const Packet>, Ptr<const PhySimWifiPhyTag>, enum PhySimWifiPhy::ErrorReason > m_rxErrorTrace;

    // Traces to signal the transmission of a new packet
  TracedCallback<Ptr<const Packet>, Ptr<const PhySimWifiPhyTag> > m_txTrace;

    // Traces to signal that packet arrived with energy below the energy detection threshold
  TracedCallback<Ptr<const Packet>, Ptr<const PhySimWifiPhyTag> > m_energyDetectionFailed;

    // Trace to signal CCA busy start events
  TracedCallback<Ptr<const NetDevice>, Time> m_ccaBusyStartTrace;

    // The following map is used to access the packet objects that correspond
    // to a single "reception" event
  std::map<Ptr<PhySimInterferenceHelper::Event>, Ptr<const Packet> > m_packets;

  static itpp::cvec m_cachedPreamble;

  // A random variable for frequency offset generation
  RandomVariable m_frequencyOffsetGenerator;
};

/**
 * \param os output stream
 * \param state wifi state to stringify
 */
std::ostream& operator<< (std::ostream& os, enum PhySimWifiPhy::State state);
std::ostream& operator<< (std::ostream& os, enum PhySimWifiPhy::ErrorReason reason);

} // namespace ns3


#endif /* PHYSIM_WIFI_PHY_H */
