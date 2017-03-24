/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA, 2009,2010 Jens Mittag, Stylianos Papanastasiou
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
 * Based on wifi-phy-state-helper.h by:
 *      Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Major modifications by:
 *      Jens Mittag <jens.mittag@kit.edu>
 *      Stylianos Papanastasiou <stylianos@gmail.com>
 */
#include "physim-wifi-phy-state-helper.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/node.h"

NS_LOG_COMPONENT_DEFINE ("PhySimWifiPhyStateHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PhySimWifiPhyStateHelper);

TypeId
PhySimWifiPhyStateHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PhySimWifiPhyStateHelper")
    .SetParent<Object> ()
    .AddConstructor<PhySimWifiPhyStateHelper> ()
    .AddTraceSource ("State",
                     "The state of the PHY layer",
                     MakeTraceSourceAccessor (&PhySimWifiPhyStateHelper::m_stateLogger));
  return tid;
}

PhySimWifiPhyStateHelper::PhySimWifiPhyStateHelper ()
  : m_syncing (false),
    m_rxing (false),
    m_endTx (Seconds (0)),
    m_endSync (Seconds (0)),
    m_endRx (Seconds (0)),
    m_endCcaBusy (Seconds (0)),
    m_startTx (Seconds (0)),
    m_startSync (Seconds (0)),
    m_startRx (Seconds (0)),
    m_startCcaBusy (Seconds (0)),
    m_previousStateChangeTime (Seconds (0)),
    m_previousStateLogTime (Seconds (0))
{
  NS_LOG_FUNCTION (this);
}

void
PhySimWifiPhyStateHelper::SetReceiveOkCallback (WifiPhy::RxOkCallback callback)
{
  m_rxOkCallback = callback;
}
void
PhySimWifiPhyStateHelper::SetReceiveErrorCallback (WifiPhy::RxErrorCallback callback)
{
  m_rxErrorCallback = callback;
}
void
PhySimWifiPhyStateHelper::RegisterListener (WifiPhyListener *listener)
{
  m_listeners.push_back (listener);
}

bool
PhySimWifiPhyStateHelper::IsStateCcaBusy (void)
{
  return GetState () == PhySimWifiPhy::CCA_BUSY;
}

bool
PhySimWifiPhyStateHelper::IsStateIdle (void)
{
  return (GetState () == PhySimWifiPhy::IDLE);
}
bool
PhySimWifiPhyStateHelper::IsStateBusy (void)
{
  return (GetState () != PhySimWifiPhy::IDLE);
}
bool
PhySimWifiPhyStateHelper::IsStateSync (void)
{
  return (GetState () == PhySimWifiPhy::SYNCING);
}
bool
PhySimWifiPhyStateHelper::IsStateRx (void)
{
  return (GetState () == PhySimWifiPhy::RX);
}
bool
PhySimWifiPhyStateHelper::IsStateTx (void)
{
  return (GetState () == PhySimWifiPhy::TX);
}



Time
PhySimWifiPhyStateHelper::GetStateDuration (void)
{
  return Simulator::Now () - m_previousStateChangeTime;
}

Time
PhySimWifiPhyStateHelper::GetDelayUntilIdle (void)
{
  Time retval = Seconds (0);

  switch (GetState ())
    {
    case PhySimWifiPhy::SYNCING:
      retval = m_endSync - Simulator::Now ();
      break;
    case PhySimWifiPhy::RX:
      retval = m_endRx - Simulator::Now ();
      break;
    case PhySimWifiPhy::TX:
      retval = m_endTx - Simulator::Now ();
      break;
    case PhySimWifiPhy::CCA_BUSY:
      retval = m_endCcaBusy - Simulator::Now ();
      break;
    case PhySimWifiPhy::IDLE:
      retval = Seconds (0);
      break;
    default:
      NS_ASSERT (false);
      // NOTREACHED
      retval = Seconds (0);
      break;
    }
  return retval;
}

Time
PhySimWifiPhyStateHelper::GetLastRxStartTime (void) const
{
  return m_startSync;
}

enum PhySimWifiPhy::State
PhySimWifiPhyStateHelper::GetState (void)
{
  if (m_endTx > Simulator::Now ())
    {
      return PhySimWifiPhy::TX;
    }
  else if (m_syncing)
    {
      return PhySimWifiPhy::SYNCING;
    }
  else if (m_rxing)
    {
      return PhySimWifiPhy::RX;
    }
  else if (m_endCcaBusy > Simulator::Now ())
    {
      return PhySimWifiPhy::CCA_BUSY;
    }
  else
    {
      return PhySimWifiPhy::IDLE;
    }
}


void
PhySimWifiPhyStateHelper::NotifyTxStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyTxStart (duration);
    }
}
void
PhySimWifiPhyStateHelper::NotifyRxStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxStart (duration);
    }
}
void
PhySimWifiPhyStateHelper::NotifyRxEndOk (void)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndOk ();
    }
}
void
PhySimWifiPhyStateHelper::NotifyRxEndError (void)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndError ();
    }
}
void
PhySimWifiPhyStateHelper::NotifyMaybeCcaBusyStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyMaybeCcaBusyStart (duration);
    }
}

void
PhySimWifiPhyStateHelper::LogPreviousIdleAndCcaBusyStates (void)
{
  Time now = Simulator::Now ();
  Time idleStart = Max (m_endCcaBusy, m_endSync);
  idleStart = Max (idleStart, m_endTx);
  NS_ASSERT (idleStart <= now);
  if (m_endCcaBusy > m_endSync
      && m_endCcaBusy > m_endTx)
    {
      Time ccaBusyStart = Max (m_endTx, m_endSync);
      ccaBusyStart = Max (ccaBusyStart, m_startCcaBusy);
      PerformStateLogging (ccaBusyStart, idleStart, PhySimWifiPhy::CCA_BUSY);
    }
  PerformStateLogging (idleStart, now, PhySimWifiPhy::IDLE);
}

void
PhySimWifiPhyStateHelper::SwitchToTx (Time txDuration, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag)
{
  NotifyTxStart (txDuration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case PhySimWifiPhy::SYNCING:
      m_syncing = false;
      PerformStateLogging (m_startSync, now, PhySimWifiPhy::SYNCING);
      m_endSync = now;
      break;
    case PhySimWifiPhy::RX:
      m_rxing = false;
      PerformStateLogging (m_startRx, now, PhySimWifiPhy::RX);
      break;
    case PhySimWifiPhy::CCA_BUSY:
      {
        Time ccaStart = Max (m_endSync, m_endTx);
        ccaStart = Max (ccaStart, m_startCcaBusy);
        PerformStateLogging (ccaStart, now, PhySimWifiPhy::CCA_BUSY);
      } break;
    case PhySimWifiPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    default:
      NS_ASSERT (false);
      break;
    }
  PerformStateLogging (now, now + txDuration, PhySimWifiPhy::TX);
  m_previousStateChangeTime = now;
  m_endTx = now + txDuration;
  m_startTx = now;
}
void
PhySimWifiPhyStateHelper::SwitchToSync (Time syncDuration, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag)
{
  NS_ASSERT (!IsStateTx () && (IsStateIdle () || IsStateCcaBusy ()));

  // According to the IEEE 802.11 standard Section 17.3.12 we have to perform a PHY-CCA.indicate here
  // for the duration of the signal header. In reality, a receiver would indicate busy already little bit
  // earlier, but since we are discrete event-based here, we wait until the end of the whole preamble
  NotifyMaybeCcaBusyStart (syncDuration);

  Time now = Simulator::Now ();
  Time ccaStart;
  switch (GetState ())
    {
    case PhySimWifiPhy::IDLE:
      m_startSync = now;
      m_previousStateChangeTime = now;
      LogPreviousIdleAndCcaBusyStates ();
      break;

    case PhySimWifiPhy::CCA_BUSY:
      ccaStart = Max (m_endSync, m_endTx);
      ccaStart = Max (ccaStart, m_startCcaBusy);
      PerformStateLogging (ccaStart, now, PhySimWifiPhy::CCA_BUSY);
      m_startSync = now;
      m_previousStateChangeTime = now;
      break;

    case PhySimWifiPhy::SYNCING:
    case PhySimWifiPhy::RX:
    case PhySimWifiPhy::TX:
      NS_FATAL_ERROR ("PhySimWifiPhyStateHelper::SwitchToSync() - Not allowed: we are in SYNC, RX or TX state.");
      break;
    }
  m_syncing = true;
  m_endSync = now + syncDuration;
  NS_ASSERT (IsStateSync ());
}
void
PhySimWifiPhyStateHelper::SwitchFromSyncEndOk (Ptr<Packet> packet, Ptr<const PhySimWifiPhyTag> tag)
{
  DoSwitchFromSync ();
}
void
PhySimWifiPhyStateHelper::SwitchFromSyncEndError (Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag)
{
  DoSwitchFromSync ();
}
void
PhySimWifiPhyStateHelper::SwitchFromSyncToSync (Time syncDuration, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag)
{
  NS_ASSERT (IsStateSync ());

  // According to the IEEE 802.11 standard Section 17.3.12 we have to perform a PHY-CCA.indicate here
  // for the duration of the signal header. In reality, a receiver would indicate busy already little bit
  // earlier, but since we are discrete event-based here, we wait until the end of the whole preamble
  NotifyMaybeCcaBusyStart (syncDuration);

  // Update the ending time of the new sync period
  m_endSync = Simulator::Now () + syncDuration;
}
void
PhySimWifiPhyStateHelper::DoSwitchFromSync (void)
{
  // Ok, first branch is the normal and old way... which means that we have been
  // in SYNC up to now.
  if (IsStateSync ())
    {
      NS_ASSERT (m_syncing);
      PerformStateLogging (m_startSync, Simulator::Now (), PhySimWifiPhy::SYNCING);
      m_previousStateChangeTime = Simulator::Now ();
      m_syncing = false;
      NS_ASSERT (IsStateIdle () || IsStateCcaBusy ());
    }
}
void
PhySimWifiPhyStateHelper::SwitchToRx (Time rxDuration)
{
  NS_ASSERT (IsStateIdle () || IsStateCcaBusy ());
  NS_ASSERT (!m_rxing);

  // Notify upper layers about reception start
  NotifyRxStart (rxDuration);

  Time now = Simulator::Now ();
  Time ccaStart;
  switch (GetState ())
    {
    case PhySimWifiPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    case PhySimWifiPhy::CCA_BUSY:
      ccaStart = Max (m_endSync, m_endTx);
      ccaStart = Max (ccaStart, m_startCcaBusy);
      PerformStateLogging (ccaStart, now, PhySimWifiPhy::CCA_BUSY);
      break;
    case PhySimWifiPhy::SYNCING:
    case PhySimWifiPhy::RX:
    case PhySimWifiPhy::TX:
      NS_ASSERT (false);
      break;
    }
  m_previousStateChangeTime = now;
  m_rxing = true;
  m_startRx = now;
  m_endRx = now + rxDuration;
  NS_ASSERT (IsStateRx ());
}
void
PhySimWifiPhyStateHelper::SwitchFromRxEndOk (Ptr<Packet> packet, Ptr<const PhySimWifiPhyTag> tag)
{
  NotifyRxEndOk ();
  DoSwitchFromRx ();
  if (!m_rxOkCallback.IsNull ())
    {
      m_rxOkCallback (packet, tag->GetPreambleSinr (), tag->GetRxWifiMode (), tag->GetWifiPreamble ());
    }
}
void
PhySimWifiPhyStateHelper::SwitchFromRxEndError (Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag)
{
  NotifyRxEndError ();
  DoSwitchFromRx ();
  if (!m_rxErrorCallback.IsNull ())
    {
      m_rxErrorCallback (packet, tag->GetPreambleSinr ());
    }
}
void
PhySimWifiPhyStateHelper::SwitchFromRxToSync (Time syncDuration, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag)
{
  // First tell the upper layers that we have stopped decoding the current packet
  // Note: we do not use NotifyRxEndError() here since that might imply that there was an error and that
  //       DCF should use special IFS length to deal with that
  NotifyRxEndOk ();
  // Also make sure that we are NOT in RX state anymore
  m_rxing = false;
  // We won't do a DoSwitchFromRx () here, and instead call SwitchToSync ()
  SwitchToSync (syncDuration, packet, tag);

}
void
PhySimWifiPhyStateHelper::DoSwitchFromRx (void)
{
  NS_ASSERT (IsStateRx ());
  NS_ASSERT (m_rxing);

  PerformStateLogging (m_startRx, Simulator::Now (), PhySimWifiPhy::RX);
  m_rxing = false;

  NS_ASSERT (IsStateIdle () || IsStateCcaBusy ());
}
void
PhySimWifiPhyStateHelper::SwitchMaybeToCcaBusy (Time duration)
{
  NotifyMaybeCcaBusyStart (duration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case PhySimWifiPhy::IDLE:
      LogPreviousIdleAndCcaBusyStates ();
      break;
    case PhySimWifiPhy::CCA_BUSY:
    case PhySimWifiPhy::SYNCING:
    case PhySimWifiPhy::TX:
    case PhySimWifiPhy::RX:
      break;
    }
  m_startCcaBusy = now;
  m_endCcaBusy = std::max (m_endCcaBusy, now + duration);
}

void
PhySimWifiPhyStateHelper::SetNetDevice (Ptr<NetDevice> device)
{
  m_device = device;
}

void
PhySimWifiPhyStateHelper::PerformStateLogging (Time start, Time end, enum PhySimWifiPhy::State state)
{
  Time begin = Max (start, m_previousStateLogTime);
  if ((end - begin) > Seconds (0))
    {
      m_stateLogger (m_device, begin, (end - begin), state);
      m_previousStateLogTime = end;
    }
}

} // namespace ns3
