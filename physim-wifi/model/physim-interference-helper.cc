/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA, 2009 Jens Mittag, Stylianos Papanastasiou
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
 * Based on interference-helper.cc by:
 *      Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Major modifications by:
 *      Jens Mittag <jens.mittag@kit.edu>
 *      Stylianos Papanastasiou <stylianos@gmail.com>
 */

#include "physim-interference-helper.h"
#include "physim-wifi-phy.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/double.h"

NS_LOG_COMPONENT_DEFINE ("PhySimInterferenceHelper");

namespace ns3 {

PhySimInterferenceHelper::Event::Event (Ptr<PhySimWifiPhyTag> tag)
  : m_startTime (Simulator::Now ()),
    m_tag (tag)
{
}
PhySimInterferenceHelper::Event::~Event ()
{
}

Time
PhySimInterferenceHelper::Event::GetDuration (void) const
{
  return m_tag->GetDuration ();
}

Time
PhySimInterferenceHelper::Event::GetStartTime (void) const
{
  return m_startTime;
}

Time PhySimInterferenceHelper::Event::GetEndTime (void) const
{
  return m_startTime + m_tag->GetDuration ();
}

bool
PhySimInterferenceHelper::Event::Overlaps (Time time) const
{
  if (m_startTime <= time
      && (m_startTime + m_tag->GetDuration ()) >= time)
    {
      return true;
    }
  else
    {
      return false;
    }
}

uint32_t
PhySimInterferenceHelper::Event::GetSize (void) const
{
  return (m_tag->GetTxedDataBits ().size ());
}

WifiMode
PhySimInterferenceHelper::Event::GetWifiMode (void) const
{
  return m_tag->GetTxWifiMode ();
}

enum WifiPreamble
PhySimInterferenceHelper::Event::GetPreambleType (void) const
{
  return m_tag->GetWifiPreamble ();
}
Ptr<PhySimWifiPhyTag>
PhySimInterferenceHelper::Event::GetWifiPhyTag (void) const
{
  return m_tag;
}


PhySimInterferenceHelper::NoiseChunk::NoiseChunk (itpp::cvec noise, Time start, Time end)
{
  m_noise = noise;
  m_start = start;
  m_end = end;
}

PhySimInterferenceHelper::NoiseChunk::~NoiseChunk ()
{
  Unref ();
}

itpp::cvec
PhySimInterferenceHelper::NoiseChunk::GetNoise () const
{
  return m_noise;
}

Time
PhySimInterferenceHelper::NoiseChunk::GetStart () const
{
  return m_start;
}

Time
PhySimInterferenceHelper::NoiseChunk::GetEnd () const
{
  return m_end;
}

/* -------------------- PhySimInterferenceHelper ---------------------------------------*/

NS_OBJECT_ENSURE_REGISTERED (PhySimInterferenceHelper);

TypeId
PhySimInterferenceHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimInterferenceHelper")
    .SetParent<Object> ()
    .AddConstructor<PhySimInterferenceHelper> ()
    .AddAttribute ("NoiseFloor",
                   "Background noise level in dBm",
                   DoubleValue (-99.0),
                   MakeDoubleAccessor (&PhySimInterferenceHelper::m_noiseFloorDbm),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DisableNoise",
                   "Disable the addition of any background noise, e.g. for debugging purposes.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&PhySimInterferenceHelper::m_noNoise),
                   MakeBooleanChecker ())
    .AddAttribute ("MaximumPacketDuration",
                   "Maximum time duration that a packet transmission/reception can last. This parameter is used to control the "
                   "time back to which packets (or their signals) are stored in order to correctly model cumulative noise.",
                   TimeValue (NanoSeconds(10968000)),
                   MakeTimeAccessor (&PhySimInterferenceHelper::m_maxPacketDuration),
                   MakeTimeChecker());
  return tid;
}

PhySimInterferenceHelper::PhySimInterferenceHelper ()
  : m_noNoise (false),
    m_symbolDuration (MicroSeconds (4)),
    m_maxPacketDuration (MicroSeconds (0))
{
}

PhySimInterferenceHelper::~PhySimInterferenceHelper ()
{
}

double
PhySimInterferenceHelper::GetNoiseFloorDbm (void) const
{
  return m_noiseFloorDbm;
}

Time
PhySimInterferenceHelper::GetEnergyDuration (double energydBm)
{
  NS_LOG_FUNCTION (energydBm);
  Time now = Simulator::Now ();
  Time duration = MicroSeconds (0);

  // Here we will save the maximum time of the last known
  // transmission
  Time max = now;

  Events::iterator i = m_events.begin ();
  while (i != m_events.end ())
    {
      Time start = (*i)->GetStartTime ();
      Time end = (*i)->GetEndTime ();
      if (start <= now && end >= now)
        {
          if (end > max)
            {
              max = end;
            }
        }
      i++;
    }

  // If there is no interfering packet, simply return 0
  if (max == now)
    {
      return MicroSeconds (0);
    }

  NS_LOG_DEBUG ("PhySimInterferenceHelper:GetEnergyDuration() will request cumulative noise for time frame " << now << " to " << (max + m_symbolDuration));

  // We add one symbol duration to make sure we do not skip the last frame
  // due to partial overlap or rather not having enough samples
  itpp::cvec cumNoise = GetCumulativeSamples (now, max + m_symbolDuration);

  // Now iterate over chunks of 80 samples and calculate their signal strength
  for (int32_t i = 0; i < cumNoise.size () - 80; i += 80)
    {
      double strength = PhySimHelper::GetOFDMSymbolSignalStrength ( cumNoise.get (i, i + 80 - 1) );
      NS_LOG_DEBUG ("PhySimInterferenceHelper:GetEnergyDuration() --> symbol " << ((uint32_t)(i / 80)) << " has signal strength of " << PhySimHelper::RatioToDb (strength) << " dBm");
      NS_LOG_DEBUG ("PhySimInterferenceHelper:IsEnergyReached() --> threshold: " << energydBm);

      // If the energy of the last chunk dropped below 'energydBm'
      if (PhySimHelper::RatioToDb (strength) < energydBm)
        {
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetEnergyDuration() duration = " << (duration + m_symbolDuration));
          return (duration + m_symbolDuration);
        }
      duration += m_symbolDuration;
    }

  NS_LOG_DEBUG ("PhySimInterferenceHelper:GetEnergyDuration() duration = " << duration);
  return duration;
}

bool
PhySimInterferenceHelper::IsEnergyReached (double energydBm, Time &duration)
{
  NS_LOG_FUNCTION (energydBm);
  Time now = Simulator::Now ();

  // So far, set duration to 0 seconds
  duration = Seconds (0);

  // Here we will save the time of the last known transmission end time
  Time max = now;

  NS_LOG_INFO ("PhySimInterferenceHelper:IsEnergyReached() - starting to iterate over all interference events");
  Events::iterator i = m_events.begin ();
  while (i != m_events.end ())
    {
      Time start = (*i)->GetStartTime ();
      Time end = (*i)->GetEndTime ();
      if (start <= now && end >= now)
        {
          if (end > max)
            {
              max = end;
            }
        }
      i++;
    }

  NS_LOG_INFO ("PhySimInterferenceHelper:IsEnergyReached() - all events processed.");
  NS_LOG_INFO ("PhySimInterferenceHelper:IsEnergyReached() - now = " << now);
  NS_LOG_INFO ("PhySimInterferenceHelper:IsEnergyReached() - max = " << max);

  // If there is no interfering packet, simply return 0
  if (max == now)
    {
      NS_LOG_INFO ("PhySimInterferenceHelper:IsEnergyReached() - returning already now because there is no interference.");
      return false;
    }

  NS_LOG_INFO ("PhySimInterferenceHelper:IsEnergyReached() will request cumulative noise for time frame " << now << " to " << (max + m_symbolDuration));

  // We add one symbol duration to make sure we do not skip the last frame
  // due to partial overlap or rather not having enough samples
  max += m_symbolDuration;
  itpp::cvec cumNoise = GetCumulativeSamples (now, max);

  // Now iterate over chunks of 80 samples and calculate their signal strength
  for (int32_t i = 0; i < cumNoise.size () - 80; i += 80)
    {
      double strength = PhySimHelper::GetOFDMSymbolSignalStrength ( cumNoise.get (i, i + 80 - 1) );
      NS_LOG_INFO ("PhySimInterferenceHelper:IsEnergyReached() --> symbol " << ((uint32_t)(i / 80)) << " has signal PhySimHelper::RatioToDb(strength) of " << PhySimHelper::RatioToDb (strength) << " dBm");
      NS_LOG_INFO ("PhySimInterferenceHelper:IsEnergyReached() --> energydBm: " << energydBm);

      // If the energy of the current chunk raises above 'energydBm'
      if (PhySimHelper::RatioToDb (strength) >= energydBm)
        {
          NS_LOG_INFO ("PhySimInterferenceHelper:IsEnergyReached() duration = " << duration);
          return true;
        }

      // We will always add the symbol duration after the check in order to get the exact time after which
      // the energy will be higher. CCA mechanisms will have to adjust this time (i.e. delay by one symbol time)
      // in order to reflect the fact that real chips can not detect the rise a priori.
      duration += m_symbolDuration;
    }
  return false;
}

Time
PhySimInterferenceHelper::CalculateTxDuration (uint32_t size, WifiMode payloadMode, WifiPreamble preamble)
{
  return PhySimHelper::CalculateTxDuration(size, payloadMode, preamble, m_symbolDuration);
}

Ptr<PhySimInterferenceHelper::Event>
PhySimInterferenceHelper::Add (Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<PhySimInterferenceHelper::Event> event = Create<PhySimInterferenceHelper::Event> (tag);
  AppendEvent (event);
  Time start = event->GetStartTime ();
  Time end = event->GetEndTime ();
  if (!m_noNoise)
    {
      itpp::cvec noise = GetBackgroundNoise (start, end);
      tag->SetBackgroundNoise (noise);
    }
  return event;
}

itpp::cvec
PhySimInterferenceHelper::GetCumulativeSamples (Time start, Time end)
{
  NS_LOG_FUNCTION (this << start << end);

  // Convert start and end times into sample numbers
  uint64_t indexStart = RoundTimeToIndex (start);
  uint64_t indexEnd = RoundTimeToIndex (end);
  uint64_t numSamples = indexEnd - indexStart;

  NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() - requested cumulative samples from sample" << indexStart << " to " << indexEnd);
  NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() - requested duration equals " << numSamples << " samples");

  // In the beginning we only have random background noise (or nothing up to now)
  itpp::cvec cumulativeSamples (numSamples);
  cumulativeSamples.zeros ();
  itpp::cvec oSamples (numSamples);
  itpp::cvec extractedSamples;

  // Add background noise only if not disabled
  if (!m_noNoise)
    {
      itpp::cvec backgroundNoise = GetBackgroundNoise (start, end);
      NS_ASSERT (backgroundNoise.size() == cumulativeSamples.size());
      cumulativeSamples += backgroundNoise;
    }

  uint32_t count = 1;

  // Iterate over all events and check whether they are inside of this time window
  Events::iterator i = m_events.begin ();
  while (i != m_events.end ())
    {
      uint64_t indexChunkStart = RoundTimeToIndex ((*i)->GetStartTime ());
      uint64_t indexChunkEnd =   RoundTimeToIndex ((*i)->GetEndTime ());

      NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() - processing event " << count << ":");
      NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> indexChunkStart = " << indexChunkStart);
      NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> indexChunkEnd = " << indexChunkEnd);
      NS_ASSERT (indexChunkEnd > indexChunkStart);

      // Case 1: the beginning of this event/frame is between 'indexStart' and 'indexEnd'
      if ( indexChunkStart >= indexStart && indexChunkStart <= indexEnd )
        {
          // determine start and end times within of the overlapping frame part
          uint64_t overlapStart = indexChunkStart;
          uint64_t overlapEnd = (indexEnd < indexChunkEnd) ? (indexEnd) : (indexChunkEnd);

          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (1) overlapStart = " << overlapStart);
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (1) overlapEnd = " << overlapEnd);

          // calculate the number of samples we are going to copy
          uint64_t oNumSamples = overlapEnd - overlapStart;
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (1) oNumSamples = " << oNumSamples);
          NS_ASSERT (oNumSamples >= 0);

          if (oNumSamples > 0)
            {
              // what is the offset w.r.t the cumulative samples?
              uint64_t destinationOffset = indexChunkStart - indexStart;
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (1) destinationOffset = " << destinationOffset);

              // extract frame part we are interested in...
              itpp::cvec interference = (*i)->GetWifiPhyTag ()->GetRxedSamples ();
              extractedSamples = interference.get ( 0, oNumSamples - 1);
              NS_ASSERT (oNumSamples <= static_cast<uint64_t> ((*i)->GetWifiPhyTag ()->GetRxedSamples ().size ()));

              oSamples.zeros ();
              oSamples.replace_mid (destinationOffset, extractedSamples);
              NS_ASSERT (destinationOffset + extractedSamples.size () <= static_cast<uint64_t> (oSamples.size ()));

              // .. and add extracted copy to final Samples
              cumulativeSamples += oSamples;

              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (1) finish!");
            }
          else
            {
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetcumulativeSamples() -     --> (1) finish! (oNumSamples==0)");
            }
        }
      // Case 2: if the ending of this event is between 'start' and 'end'
      else if ( indexChunkEnd > indexStart && indexChunkEnd <= indexEnd )
        {

          // determine start and end times of the overlapping frame part...
          uint64_t overlapEnd = indexChunkEnd;
          uint64_t overlapStart = (indexStart > indexChunkStart) ? (indexStart) : (indexChunkStart);

          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (2) overlapStart = " << overlapStart);
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (2) overlapEnd = " << overlapEnd);

          // calculate the number of samples we are going to copy
          uint64_t oNumSamples = overlapEnd - overlapStart;
          NS_ASSERT (oNumSamples >= 0);
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (2) oNumSamples = " << oNumSamples);

          if (oNumSamples > 0)
            {
              // what is the offset w.r.t the cumulative samples?
              uint64_t destinationOffset = overlapStart - indexStart;
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (2) destinationOffset = " << destinationOffset);

              // what is the offset inside of the source samples?
              uint64_t sourceOffset = (indexChunkStart < indexStart) ? (indexStart - indexChunkStart) : (indexChunkStart - indexStart);
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (2) sourceOffset = " << sourceOffset);

              // extract frame part we are interested in...
              itpp::cvec interference = (*i)->GetWifiPhyTag ()->GetRxedSamples ();
              extractedSamples = interference.get ( sourceOffset, sourceOffset + oNumSamples - 1);
              NS_ASSERT (oNumSamples <= static_cast<uint64_t> ((*i)->GetWifiPhyTag ()->GetRxedSamples ().size ())&&oNumSamples >= 0);

              oSamples.zeros ();
              oSamples.replace_mid ( destinationOffset, extractedSamples);
              NS_ASSERT (destinationOffset + extractedSamples.size () <= static_cast<uint64_t> (oSamples.size ()));

              // ... and add extracted copy to final Samples
              cumulativeSamples += oSamples;

              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (2) finish!");
            }
          else
            {
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (2) finish! (oNumSamples==0)");

            }
        }
      // Case 3: if the frame is overlapping completely, i.e. indexChunkStart < indexStart
      //         and indexChunkEnd > indexEnd
      else if (indexChunkStart < indexStart && indexChunkEnd > indexEnd)
        {

          // determine start and end times of the overlapping frame part
          uint64_t overlapStart = indexStart - indexChunkStart;
          uint64_t overlapEnd = overlapStart + (indexEnd - indexStart);
          NS_ASSERT (overlapEnd > overlapStart);
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (3) overlapStart = " << overlapStart);
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (3) overlapEnd = " << overlapEnd);

          // calculate the number of samples we are going to copy
          uint64_t oNumSamples = overlapEnd - overlapStart;
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (3) oNumSamples = " << oNumSamples);
          NS_ASSERT (oNumSamples >= 0);

          // what is the offset inside of the source Samples?
          uint64_t sourceOffset = overlapStart;
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (3) sourceOffset = " << sourceOffset);

          if (oNumSamples > 0)
            {
              // extract frame part we are interested in...
              itpp::cvec interference = (*i)->GetWifiPhyTag ()->GetRxedSamples ();
              extractedSamples = interference.get ( sourceOffset, sourceOffset + oNumSamples - 1);
              NS_ASSERT (sourceOffset < static_cast<uint64_t> ((*i)->GetWifiPhyTag ()->GetRxedSamples ().size ()));
              NS_ASSERT (sourceOffset + oNumSamples <= static_cast<uint64_t> ((*i)->GetWifiPhyTag ()->GetRxedSamples ().size ()));
              oSamples.zeros ();
              oSamples.replace_mid (0, extractedSamples);
              NS_ASSERT (extractedSamples.size () <= oSamples.size ());
              // ... and add extracted copy to final Samples
              cumulativeSamples += oSamples;

              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (3) finish!");
            }
          else
            {
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() -     --> (3) finish! (oNumSamples==0)");
            }
        }
      NS_LOG_DEBUG ("PhySimInterferenceHelper:GetCumulativeSamples() - finished processing event " << count);
      i++;
      count++;
    }

  return cumulativeSamples;
}

void
PhySimInterferenceHelper::AppendEvent (Ptr<PhySimInterferenceHelper::Event> event)
{
  /* attempt to remove the events which are
   * not useful anymore.
   * i.e.: all events which end _before_
   *       now - m_maxPacketDuration
   */
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_DEBUG ("PhySimInterferenceHelper:AppendEvent() - m_maxPacketDuration = "  << GetMaxPacketDuration ());
  if (Simulator::Now () > GetMaxPacketDuration ())
    {
      Time end = Simulator::Now () - GetMaxPacketDuration ();
      Events::iterator it = m_events.begin ();
      while (it != m_events.end ())
        {
          if ((*it)->GetEndTime () < end)
            {
              NS_LOG_DEBUG ("PhySimInterferenceHelper:AppendEvent() - deleting event with startTime = "  << (*it)->GetStartTime () << " and endTime = " << (*it)->GetEndTime ());
              it = m_events.erase (it);
            }
          else
            {
              it++;
            }
        }
    }
  m_events.push_back (event);
}

Time
PhySimInterferenceHelper::GetMaxPacketDuration (void)
{
  return m_maxPacketDuration;
}

uint64_t
PhySimInterferenceHelper::RoundTimeToIndex (Time t) const
{
  uint64_t sampleDuration = m_symbolDuration.GetNanoSeconds () / 80.0;
  double d = (double) t.GetNanoSeconds () / (double) sampleDuration;
  return round (d);
}

itpp::cvec
PhySimInterferenceHelper::GetBackgroundNoise (Time start, Time end)
{
  NS_LOG_FUNCTION (start << end);

  uint64_t sampleDuration = m_symbolDuration.GetNanoSeconds () / 80.0;
  NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() sampleDuration = " << sampleDuration);

  itpp::cvec backgroundNoise;
  Noises::iterator it = m_noiseChunks.begin ();

  // First delete all entries in the front which have an
  // endTime older than the longest packet duration
  Time limit = (Simulator::Now () > GetMaxPacketDuration ()) ? Simulator::Now () - GetMaxPacketDuration () : MicroSeconds (0);
  while ( (it != m_noiseChunks.end ()) && (*it)->GetEnd ().GetNanoSeconds () < limit.GetNanoSeconds () )
    {
      NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() erasing chunk with startTime = " << (*it)->GetStart () << " endTime = " << (*it)->GetEnd ());
      it = m_noiseChunks.erase (it);
    }

  // Set the next (or initial) required startTime
  uint64_t nextIndex = RoundTimeToIndex (start);
  uint64_t lastIndex = RoundTimeToIndex (end);
  NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() Init: nextIndex = " << nextIndex << " lastIndex = " << lastIndex);

  // Now go to the chunks which could still be valid
  while (it != m_noiseChunks.end ())
    {

      Ptr<PhySimInterferenceHelper::NoiseChunk> chunk = *it;
      uint64_t indexStart = RoundTimeToIndex ( chunk->GetStart () );
      uint64_t indexEnd = RoundTimeToIndex ( chunk->GetEnd () );
      NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - processing chunk with startTime = " << chunk->GetStart () << " endTime = " << chunk->GetEnd ());
      NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - processing chunk with indexStart = " << indexStart << " indexEnd = " << indexEnd);

      /*
       * In general we can have several cases here when putting the background noise together
       *
       * Case 1: The chunk starts earlier then our expected next start and ends earlier than the desired end
       *
       *          Requested time:           |-------------------------------|
       *          Chunk:              |-------------|
       *
       *          Note: The starting time index of the chunk has to be _smaller_ than nextIndex, but the end time index
       *                is allowed to be as big as the lastIndex
       *
       * Case 2: The chunk is contained completely within the requested time duration
       *
       *          Requested time:           |-------------------------------|
       *          Chunk:                     |-----------------------------|
       *
       *          Note: The chunk is allowed to be congruent with with the requested time,
       *                i.e. chunkIndexStart == nextIndex AND chunkIndexEnd == lastIndex
       *
       * Case 3: The chunk overlaps partially with the end.
       *
       *          Requested time:           |-------------------------------|
       *          Chunk:                                           |--------------|
       *
       *          Note: chunkIndexStart has be within the requested time frame and chunkIndexEnd has to be
       *                greater than lastIndex
       *
       * Case 4: The chunk is greater than the requested time frame
       *
       *          Requested time:           |-------------------------------|
       *          Chunk:               |--------------------------------------------|
       *
       *          Note: chunkIndexStart < nextIndex && chunkIndexEnd > lastIndex
       *
       */

      // Let's check for Case 1
      if (indexStart < nextIndex && indexEnd <= lastIndex && indexEnd >= nextIndex && (indexEnd - nextIndex) > 0)
        {
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (1) starting...");
          uint32_t numSamples = (indexEnd - nextIndex);
          uint32_t offset = nextIndex - indexStart;
          itpp::cvec noise = chunk->GetNoise ().get (offset, offset + numSamples - 1);
          backgroundNoise.ins ( backgroundNoise.size (), noise );
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (1) chunk overlaps from index " << nextIndex << " to " << indexEnd);
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (1) copied " << numSamples << " sample from local index " << offset << " to " << (offset + numSamples - 1));

          // update nextIndex
          nextIndex = indexEnd;
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (1) setting nextIndex to " << nextIndex);

          if (nextIndex >= lastIndex)
            {
              // Ok, we have everything up to lastIndex
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - (1) we have reached the end! return.");
              return backgroundNoise;
            }
        }
      // and then we check for the 2nd case
      else if (indexStart >= nextIndex && indexEnd <= lastIndex)
        {
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (2) starting...");
          uint32_t offset = (indexStart - nextIndex);

          // If there is a gap between indexStart and nextIndex (i.e. offset > 0),
          // we have to fill the gap with a new chunk
          if (offset > 0)
            {
              itpp::cvec noise = sqrt ( pow (10, (m_noiseFloorDbm / 10)) ) * itpp::randn_c (offset);
              backgroundNoise.ins ( backgroundNoise.size (), noise);

              // And create new noise chunk for this part
              Time cStart = NanoSeconds (nextIndex * sampleDuration);
              Time cEnd = NanoSeconds ((nextIndex + offset) * sampleDuration);
              Ptr<PhySimInterferenceHelper::NoiseChunk> nc = Create<PhySimInterferenceHelper::NoiseChunk> (noise, cStart, cEnd);
              m_noiseChunks.insert (it, nc);
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (2) inserted new chunk with startTime = " << cStart << ", endTime = " << cEnd);
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (2) inserted new chunk with startIndex = " << nextIndex << ", endIndex = " << (nextIndex + offset));
            }

          // then insert the already existing chunk
          backgroundNoise.ins ( backgroundNoise.size (), chunk->GetNoise () );
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (2) chunk overlaps from index " << indexStart << " to " << indexEnd);
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (2) copied " << chunk->GetNoise ().size() << " sample from local index 0 to " << (chunk->GetNoise ().size() - 1));

          // update nextIndex
          nextIndex = indexEnd;
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (2) setting nextIndex to " << nextIndex);

          if (nextIndex >= lastIndex)
            {
              // Ok, we have everything up to lastIndex
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - (2) we have reached the end! return.");
              return backgroundNoise;
            }
        }
      // and then for the 3rd case
      else if (indexStart >= nextIndex && indexStart < lastIndex && indexEnd > lastIndex)
        {
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (3) starting...");
          uint32_t numSamples = (lastIndex - indexStart);
          uint32_t offset = (indexStart - nextIndex);

          // If there is a gap between indexStart and nextIndex (i.e. offset > 0),
          // we have to fill the gap with a new chunk
          if (offset > 0)
            {
              itpp::cvec noise = sqrt ( pow (10, (m_noiseFloorDbm / 10)) ) * itpp::randn_c (offset);
              backgroundNoise.ins ( backgroundNoise.size (), noise);

              // And create new noise chunk for this part
              Time cStart = NanoSeconds (nextIndex * sampleDuration);
              Time cEnd = NanoSeconds ((nextIndex + offset) * sampleDuration);
              Ptr<PhySimInterferenceHelper::NoiseChunk> nc = Create<PhySimInterferenceHelper::NoiseChunk> (noise, cStart, cEnd);
              m_noiseChunks.insert (it, nc);
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (3) inserted new chunk with startTime = " << cStart << ", endTime = " << cEnd);
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (3) inserted new chunk with startIndex = " << nextIndex << ", endIndex = " << (nextIndex + offset));
            }

          // then insert the already existing chunk
          backgroundNoise.ins ( backgroundNoise.size (),chunk->GetNoise ().get (0, numSamples - 1));
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (3) chunk overlaps from index " << indexStart << " to " << lastIndex);
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (3) copied " << numSamples << " sample from local index 0 to " << (numSamples - 1));

          // update nextIndex
          nextIndex = lastIndex;
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (3) setting nextIndex to " << nextIndex);

          if (nextIndex >= lastIndex)
            {
              // Ok, we have everything up to lastIndex
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - (3) we have reached the end! return.");
              return backgroundNoise;
            }
        }
      // and finally the 4th case
      else if (indexStart < nextIndex && indexEnd > lastIndex)
        {
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (4) starting...");
          uint32_t numSamples = (lastIndex - nextIndex);
          uint32_t offset = (nextIndex - indexStart);

          // then insert the already existing chunk
          backgroundNoise.ins ( backgroundNoise.size (), chunk->GetNoise ().get (offset, offset + numSamples - 1));
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (4) chunk overlaps completely from index " << nextIndex << " to " << lastIndex);
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (4) copied " << numSamples << " sample from local index " << offset << " to " << ( offset + numSamples - 1));

          // update nextIndex
          nextIndex = lastIndex;
          NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (4) setting nextIndex to " << nextIndex);

          if (nextIndex >= lastIndex)
            {
              // Ok, we have everything up to lastIndex
              NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - (4) we have reached the end! return.");
              return backgroundNoise;
            }
        }

      it++;
    }

  // If no chunk matched...
  if (nextIndex < lastIndex)
    {
      uint32_t numSamples = (lastIndex - nextIndex);
      itpp::cvec noise = sqrt ( pow (10, (m_noiseFloorDbm / 10)) ) * itpp::randn_c (numSamples);
      backgroundNoise.ins ( backgroundNoise.size (), noise);

      // And create new noise chunk for this part
      Time cStart = NanoSeconds (nextIndex * sampleDuration);
      Time cEnd = NanoSeconds (lastIndex * sampleDuration);
      Ptr<PhySimInterferenceHelper::NoiseChunk> nc = Create<PhySimInterferenceHelper::NoiseChunk> (noise, cStart, cEnd);
      m_noiseChunks.insert (it, nc);
      NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (0) inserting additional noise chunk with startTime = " << cStart << ", endTime = " << cEnd);
      NS_LOG_DEBUG ("PhySimInterferenceHelper:GetBackgroundNoise() - --> (0) inserting additional noise chunk with startIndex = " << nextIndex << ", endIndex = " << lastIndex);

    }

  return backgroundNoise;
}

double
PhySimInterferenceHelper::CalculatePreambleSinr (Ptr<PhySimInterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (event);
  itpp::cvec preamble = event->GetWifiPhyTag ()->GetRxedSamples ().get (0, 319);
  Time endPreamble = event->GetStartTime () + m_symbolDuration + m_symbolDuration + m_symbolDuration + m_symbolDuration;
  itpp::cvec cumNoise = GetCumulativeSamples (event->GetStartTime (), endPreamble);
  cumNoise -= preamble;
  double sinr = CalculateSinr (preamble, cumNoise);
  return sinr;
}

double
PhySimInterferenceHelper::CalculateHeaderSinr (Ptr<PhySimInterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (event);
  itpp::cvec header = event->GetWifiPhyTag ()->GetRxedSamples ().get (320, 399);
  Time startHeader = event->GetStartTime () + m_symbolDuration + m_symbolDuration + m_symbolDuration + m_symbolDuration;
  Time endHeader = startHeader + m_symbolDuration;
  itpp::cvec cumNoise = GetCumulativeSamples (startHeader, endHeader);
  NS_ASSERT (header.size () == cumNoise.size () && header.size () == 80);
  cumNoise -= header;
  double sinr = (PhySimHelper::GetOFDMSymbolSignalStrength (header) / PhySimHelper::GetOFDMSymbolSignalStrength (cumNoise));
  sinr = 10.0 * log10 (sinr);
  return sinr;
}

double
PhySimInterferenceHelper::CalculatePayloadSinr (Ptr<PhySimInterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (event);
  itpp::cvec rxSamples = event->GetWifiPhyTag ()->GetRxedSamples ();
  itpp::cvec payload = rxSamples.get (400, rxSamples.size () - 2);
  Time startPayload = event->GetStartTime () + m_symbolDuration + m_symbolDuration + m_symbolDuration + m_symbolDuration + m_symbolDuration;
  Time endPayload = event->GetEndTime ();
  itpp::cvec cumNoise = GetCumulativeSamples (startPayload, endPayload);
  cumNoise -= payload;
  double sinr = CalculateSinr (payload, cumNoise);
  return sinr;
}

double
PhySimInterferenceHelper::CalculateOverallSinr (Ptr<PhySimInterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (event);
  itpp::cvec rxSamples = event->GetWifiPhyTag ()->GetRxedSamples ();
  itpp::cvec total = rxSamples.get (0, rxSamples.size () - 2);
  itpp::cvec cumNoise = GetCumulativeSamples (event->GetStartTime (), event->GetEndTime ());
  cumNoise -= total;
  double sinr = CalculateSinr (total, cumNoise);
  return sinr;
}

void
PhySimInterferenceHelper::SetSymbolTime (Time duration)
{
  m_symbolDuration = duration;
}

double
PhySimInterferenceHelper::CalculateSinr (itpp::cvec reference, itpp::cvec cumNoise)
{
  NS_LOG_INFO ("PhySimInterferenceHelper:CalculateSinr() reference.size() = " << reference.size () << " cumNoise.size() = " << cumNoise.size ());
  NS_ASSERT (reference.size () == cumNoise.size ());
  double sinr = 0.0;

  // Now iterate over all 'symbols' and calculate the SINR
  for (int32_t i = 0; i < reference.size (); i += 80)
    {
      NS_LOG_DEBUG ("PhySimInterferenceHelper:CalculateSinr() Processing block " << (i % 80));
      double pwrNoise = PhySimHelper::GetOFDMSymbolSignalStrength ( cumNoise.get (i, i + 80 - 1) );
      double pwrReference = PhySimHelper::GetOFDMSymbolSignalStrength ( reference.get (i, i + 80 - 1) );
      NS_LOG_DEBUG ("PhySimInterferenceHelper:CalculateSinr() --> pwrNoise: " << pwrNoise);
      NS_LOG_DEBUG ("PhySimInterferenceHelper:CalculateSinr() --> pwrReference: " << pwrReference);
      NS_LOG_DEBUG ("PhySimInterferenceHelper:CalculateSinr() --> ratio: " << (pwrReference / pwrNoise));
      sinr += ( pwrReference / pwrNoise );
    }

  uint32_t numSymbols = reference.size () / 80;
  sinr = 10.0 * log10 (sinr / numSymbols);
  NS_LOG_DEBUG ("PhySimInterferenceHelper:CalculateSinr() final SINR = " << sinr << "dB");
  return sinr;
}

} // namespace ns3
