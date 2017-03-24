/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA, 2009 Stylianos Papanastasiou, Jens Mittag
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
 * Based on interference-helper.h by:
 *      Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Major modifications by:
 *      Jens Mittag <jens.mittag@kit.edu>
 *      Stylianos Papanastasiou <stylianos@gmail.com>
 */

#ifndef PHYSIM_INTERFERENCE_HELPER_H
#define PHYSIM_INTERFERENCE_HELPER_H

#include <stdint.h>
#include <vector>
#include <list>
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"
#include "ns3/nstime.h"
#include "ns3/ref-count-base.h"
#include "ns3/packet.h"
#include "ns3/event-id.h"
#include "ns3/object.h"

#include <itpp/itcomm.h>

namespace ns3 {

class PhySimWifiPhyTag;

/**
 * \class PhySimInterferenceHelper
 * \brief A module that keeps track of incoming frames at PhySimWifiPhy devices
 *        and that provides the functionality to compute the cumulative signal of
 *        overlapping frames, to compute the remaining time until the signal strength
 *        of all overlapping signals drops below a certain threshold, and the
 *        functionality to compute the remaining time until the signal strength
 *        exceeds a certain threshold.
 *
 * The module includes the white Gaussian noise that is present at any WiFi receiver
 * and manages the list of all incoming signals/frames. Therefore, it employs two
 * lists, PhySimInterferenceHelper::Events and PhySimInterferenceHelper::Noises, which
 * are basically std::list containers of the private classes PhysimInterferenceHelper::Event
 * and PhySimInterferenceHelper::NoiseChunk
 *
 * In the current implementation, the functionality of this module is used by PhySimWifiPhy
 * to request the cumulative complex time signals within the PhySimWifiPhy::EndPreamble(),
 * PhySimWifiPhy::EndHeader() and PhySimWifiPhy::EndRx() methods before trying to detect a signal,
 * to estimate the channel and to decode the signal header or the data symbols.
 *
 * In addition, this module provides the ability to compute the signal-to-interference-noise
 * ratio (SINR) for the Preamble, the Signal Header and the Payload of a frame. The methods
 * offered for this are
 *
 *  - PhySimInterferenceHelper::CalculatePreambleSinr(Ptr<PhySimInterferenceHelper::Event> event)
 *  - PhySimInterferenceHelper::CalculateHeaderSinr(Ptr<PhySimInterferenceHelper::Event> event)
 *  - PhySimInterferenceHelper::CalculatePayloadSinr(Ptr<PhySimInterferenceHelper::Event> event)
 *  - PhySimInterferenceHelper::CalculateOverallSinr(Ptr<PhySimInterferenceHelper::Event> event)
 *
 */
class PhySimInterferenceHelper : public Object
{
public:
  /**
   * \class Event
   * A class that represents the event that a single frame arrived at the physical
   * layer. It keeps track of information such as packet duration, starting and end
   * time of the packet transmission, and several other aspects.
   *
   * The underlying data structure that is used is the PhySimWifiPhyTag, that
   * encapsulates all the information associated to a packet in PhySimWifiPhy.
   *
   * In addition, the class enables an easy check of whether one event
   * overlaps with another event.
   */
  class Event : public RefCountBase
  {
public:
    Event (Ptr<PhySimWifiPhyTag> tag);
    virtual ~Event ();

    /**
     * Get the duration of the frame transmission. Actually, the term 'transmission' is not semantically correct, because it
     * is from the perspective of the sending node, but this is the perspective of the receiving node. However, the duration is
     * the same for both perspectives.
     */
    Time GetDuration (void) const;
    /**
     * Returns the point in time at which this packet (or event) starts at the receiving node. More precisley, this is the
     * time at which the first signal of the frame arrives at the receiving node.
     */
    Time GetStartTime (void) const;
    /**
     * Returns the point in time at which this packet (or event) ends at the receiving node. More precisley, this is the
     * time at which the last signal of the frame arrives at the receiving node.
     */
    Time GetEndTime (void) const;
    /**
     * Returns whether the event overlaps with the given timestamp or not.
     * \param time The timestamp against which to check the overlap
     */
    bool Overlaps (Time time) const;
    /**
     * Returns the number of bits, which are encoded in the transmitted packet that corresponds to this event
     */
    uint32_t GetSize (void) const;
    /**
     * Returns the WiFi mode that was used for the transmission of the packet that corresponds to this event
     */
    WifiMode GetWifiMode (void) const;
    /**
     * Returns the WiFi preamble type that was used in the transmission of the packet, that corresponds to this event
     */
    enum WifiPreamble GetPreambleType (void) const;
    /**
     * Returns the underlying WifiPhyTag if further information is required. Normally this is not needed
     */
    Ptr<PhySimWifiPhyTag> GetWifiPhyTag (void) const;

private:
    Time m_startTime;
    Ptr<PhySimWifiPhyTag> m_tag;
  };

  /**
   * \class NoiseChunk
   * A class that models a chunk of background noise, or more precisely white Gaussian noise,
   * that is present at each receiver. In order to not compute random Gaussian distributed
   * complex time samples twice and to be able to return identical samples for a given time window
   * in successive requests, the generated samples are saved in this data structure.
   *
   * Each instance of this class then represents a specific time window, with a fixed starting time
   * and ending time. This class is used by PhySimInterferenceHelper to create chunks of background noise
   * whenever necessary and to reuse previously generated samples in case the background noise for an
   * already generated time window is requested again at a later stage.
   *
   * By 'merging' or 'concatenating' individual noise chunks, the class PhySimInterferenceHelper is
   * also able to serve requests for time windows that span over multiple already existing noise chunks.
   */
  class NoiseChunk : public RefCountBase
  {
  public:
    NoiseChunk (itpp::cvec noise, Time start, Time end);
    ~NoiseChunk ();
    Time GetStart () const;
    Time GetEnd () const;
    itpp::cvec GetNoise () const;

  private:
    itpp::cvec m_noise;
    Time m_start;
    Time m_end;
  };

  static TypeId GetTypeId (void);

  PhySimInterferenceHelper ();
  ~PhySimInterferenceHelper ();

  /**
   * Sets the signal strength of the white Gaussian background noise (in dBm), that shall be used when generating random
   * complex time samples.
   */
  void SetNoiseFloorDbm (double noiseFloor);
  /**
   * Returns the current signal strength setting that is used to create the white Gaussian background noise
   */
  double GetNoiseFloorDbm (void) const;

  /**
   * Computes the time duration for which the cumulative signal strength of all overlapping incoming
   * frames stays above a given threshold.
   * \param energydBm The signal strength threshold for which to compute the duration
   * \returns         The expected amount of time the observed energy on the medium will be higher than
   *                  the given threshold.
   */
  Time GetEnergyDuration (double energydBm);
  /**
   * Checks whether the energy that will be present in the future (which we know since this is a simulator)
   * raises above a given threshold at some point in time.
   *
   * \param energydBm The signal strength threshold for which the check is performed.
   * \param duration  The Time object in which the remaining time until the threshold is exceeded will be stored
   * \return          The result of check. True in case the energy raises above the threshold, false otherwise
   */
  bool IsEnergyReached (double energydBm, Time &duration);
  /**
   * Calculates the transmit duration for a given packet size, data rate and preamble type.
   * \param size        The packet size in bytes
   * \param payloadMode The WiFi mode that is used to encode the payload of the packet
   * \param preamble    The preamble type that is used for this transmission
   * \return            The time duration that is necessary to transmit the packet
   */
  Time CalculateTxDuration (uint32_t size, WifiMode payloadMode, WifiPreamble preamble);
  /**
   * Adds a packet and its corresponding PhySimWifiPhyTag to the list of interference
   * \param packet The packet that is added to the list
   * \param tag    The corresponding tag, in which the complex time samples are saved
   * \return       The pointer that represents this packet and that is used as an identifier within
   *               the interference manager
   */
  Ptr<PhySimInterferenceHelper::Event> Add (Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag);

  /**
   * Computes the cumulative signal (as a sequence of complex time samples) for a given time window.
   * The starting and ending time of the window are both considered as part of the window.
   * \param start The starting time of the time window
   * \param end   The ending time of the time window
   * \return      The sequence/vector of complex time samples that represent the cumulative signal during
   *              the requested time window
   */
  itpp::cvec GetCumulativeSamples (Time start, Time end);
  /**
   * Generates the background/thermal noise (as a sequence of complex time samples) for a given time window .
   * \param start The starting time of the time window
   * \param end   The ending time of the time window
   * \return      The sequence/vector of complex time samples that represent the background noise that
   *              is present during the requested time window
   */
  itpp::cvec GetBackgroundNoise (Time start, Time end);
  /**
   * Calculates the signal-to-interference-noise ratio (SINR) of the preamble of a specific packet/frame.
   * Currently, the SINR is calculated by taking the ration between 1) the energy of the cumulative signals
   * minus the signal of the frame for which the SINR is requested and 2) the energy of signal of the frame.
   * \param event The event pointer, which identifies the packet for which the SINR shall be computed.
   * \return      The computed SINR value
   */
  double CalculatePreambleSinr (Ptr<PhySimInterferenceHelper::Event> event);
  /**
   * Calculates the signal-to-interference-noise ratio (SINR) of the signal header of a specific packet/frame.
   * Currently, the SINR is calculated by taking the ration between 1) the energy of the cumulative signals
   * minus the signal of the frame for which the SINR is requested and 2) the energy of signal of the frame.
   * \param event The event pointer, which identifies the packet for which the SINR shall be computed.
   * \return      The computed SINR value
   */
  double CalculateHeaderSinr (Ptr<PhySimInterferenceHelper::Event> event);
  /**
   * Calculates the signal-to-interference-noise ratio (SINR) of the payload of a specific packet/frame.
   * Currently, the SINR is calculated by taking the ration between 1) the energy of the cumulative signals
   * minus the signal of the frame for which the SINR is requested and 2) the energy of signal of the frame.
   * \param event The event pointer, which identifies the packet for which the SINR shall be computed.
   * \return      The computed SINR value
   */
  double CalculatePayloadSinr (Ptr<PhySimInterferenceHelper::Event> event);
  /**
   * Calculates the overall/total signal-to-interference-noise ratio (SINR) of a specific packet/frame.
   * Currently, the SINR is calculated by taking the ration between 1) the energy of the cumulative signals
   * minus the signal of the frame for which the SINR is requested and 2) the energy of signal of the frame.
   * \param event The event pointer, which identifies the packet for which the SINR shall be computed.
   * \return      The computed SINR value
   */
  double CalculateOverallSinr (Ptr<PhySimInterferenceHelper::Event> event);

  /**
   * Sets the symbol time that shall be assumed when generating random background noise
   * \param duration The OFDM symbol time duration
   */
  void SetSymbolTime (Time duration);

private:
  typedef std::list< Ptr<PhySimInterferenceHelper::Event> > Events;

  PhySimInterferenceHelper (const PhySimInterferenceHelper &o);
  PhySimInterferenceHelper &operator = (const PhySimInterferenceHelper &o);

  void AppendEvent (Ptr<PhySimInterferenceHelper::Event> event);
  Time GetMaxPacketDuration (void);
  double CalculateSinr (itpp::cvec reference, itpp::cvec cumNoise);
  uint64_t RoundTimeToIndex (Time t) const;

  double m_noiseFloorDbm;
  bool m_noNoise;
  Time m_symbolDuration;
  Events m_events;
  Time m_maxPacketDuration;

  // For background noise management
  typedef std::list< Ptr<PhySimInterferenceHelper::NoiseChunk> > Noises;
  Noises m_noiseChunks;

};

} // namespace ns3

#endif /* PHYSIM_INTERFERENCE_HELPER_H */
