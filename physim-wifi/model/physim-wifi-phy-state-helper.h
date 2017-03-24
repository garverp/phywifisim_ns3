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
#ifndef PHYSIM_WIFI_PHY_STATE_HELPER_H
#define PHYSIM_WIFI_PHY_STATE_HELPER_H

#include "physim-wifi-phy.h"
#include "ns3/wifi-phy.h"
#include <vector>

namespace ns3 {

/**
 * \brief The state helper for the PhySimWifiPhy implementation.
 *
 * This class does not implemented the state machine behind the PhySimWifiPhy module itself, but wraps all
 * logging, tracing and signaling functionality which is related to the state machine. Moreover, it is simply
 * used to keep track of the physical layer states. The possible and allowed state transitions are implemented
 * in this class here and initiated from within PhySimWifiPhy.
 *
 * In general, the physical layer of the PhySimWifiPhy implementation can be in one of the 5 different states that
 * are defined in PhySimWifiPhy::State
 *
 * - IDLE: the energy at the receiver is below the CcaModelThreshold and no frame is detected or being decoded
 * - TX: transceiver is currently transmitting a frame
 * - SYNCING: the preamble of a frame has been detected and the transceiver is in signal header decoding stage
 * - RX: the signal header has been decoded successfully and the transceiver is in payload decoding stage
 * - CCA_BUSY: the energy at the receiver is above the CcaModelThreshold an no frame is detected or being decoded
 *
 * By default, the physical layer is in IDLE state. Whenever the physical layer starts to transmit a packet, it changes
 * its state to TX through the method
 *
 *  - PhySimWifiPhyStateHelper::SwitchToTx
 *
 * The change back to IDLE is not modeled explicitly and instead recovered implicitly (Note: there is no single variable
 * that tracks the state, Instead, several variables are used to track the starting/ending times of the last TX, SYNCING,
 * RX and CCA_BUSY periods. By comparing these times with the current time, the current state of the Phy is derived).
 *
 * During the reception of a packet, the state will typically change from IDLE to SYNCING (that is after the preamble
 * has been detected successfully), from SYNCING to RX (that is after the signal header has been decoded successfully)
 * and finally back to IDLE (after the payload has been decoded). These transitions are represented by the following
 * class methods
 *
 *  - PhySimWifiPhyStateHelper::SwitchToSync
 *  - PhySimWifiPhyStateHelper::SwitchFromSyncEndOk
 *  - PhySimWifiPhyStateHelper::SwitchToRx
 *  - PhySimWifiPhyStateHelper::SwitchFromRxEndOk
 *  - PhySimWifiPhyStateHelper::SwitchFromRxEndError
 *
 * The final state change (from RX back to IDLE) is performed either in SwitchFromRxEndOk or SwitchFromRxEndError,
 * depending on the success of the payload decoding stage. In case the signal header decoding fails (e.g. due to
 * a wrong parity bit or a failure in the plausibility check), the function
 *
 *  - PhySimWifiPhyStateHelper::SwitchFromSyncEndError
 *
 * will implicitly perform the transition from SYNCING to IDLE, instead of going further to RX state.
 *
 * Since v1.1 of PhySim, the packet capture capabilities of recent chipsets is supported. To reflect the additional
 * state transitions, two methods have been added since:
 *
 *  - PhySimWifiPhyStateHelper::SwitchFromSyncToSync
 *  - PhySimWifiPhyStateHelper::SwitchFromRxToSync
 *
 * that handle the proper state change updates in case the PhySimWifiPhy::EndPreamble event of the captured packet
 * is executed during SYNC or RX state.
 *
 * In order to correctly implement the CCA.indicate (carrier sense) mechanism of IEEE 802.11, an additional method
 * called
 *
 *  - PhySimWifiPhyStateHelper::SwitchMaybeToCcaBusy
 *
 * is provided, that sets the current state to CCA_BUSY. This method is called from PhySimWifiPhy::StartCcaBusy
 * whenever the cumulative signals that are present at the receiver exceed the CCA busy threshold (that is defined by
 * the attribute 'CcaModelThreshold' in PhySimWifiPhy) and the current physical layer state would be IDLE.
 *
 *
 */
class PhySimWifiPhyStateHelper : public Object
{
public:
  static TypeId GetTypeId (void);

  PhySimWifiPhyStateHelper ();

  void SetReceiveOkCallback (WifiPhy::RxOkCallback callback);
  void SetReceiveErrorCallback (WifiPhy::RxErrorCallback callback);
  void RegisterListener (WifiPhyListener *listener);
  enum PhySimWifiPhy::State GetState (void);

  /**
   * \returns True if the state of the PHY is currently PhySimWifiPhy::CCA_BUSY
   */
  bool IsStateCcaBusy (void);
  /**
   * \returns True if the state of the PHY is currently PhySimWifiPhy::IDLE
   */
  bool IsStateIdle (void);
  /**
   * \returns True if the state of the PHY is currently not PhySimWifiPhy::IDLE
   */
  bool IsStateBusy (void);
  /**
   * \returns True if the state of the PHY is currently PhySimWifiPhy::SYNCING, which means
   *          PHY is synchronized to a preamble and currently decoding the signal header
   */
  bool IsStateSync (void);
  /**
   * \returns True if the state of the PHY is currently PhySimWifiPhy::RX, which means
   *          PHY has decoded a signal header correctly and is processing data symbols
   */
  bool IsStateRx (void);
  /**
   * \returns True if the state of the PHY is currently PhySimWifiPhy::TX
   */
  bool IsStateTx (void);

  /**
   * \returns The remaining time of the current state. After this time, the PHY will switch to
   *          another state or (in case of PhySimWifiPhy::IDLE or PhySimWifiPhy::CCA_BUSY) stay
   *          in this state.
   */
  Time GetStateDuration (void);
  /**
   * \returns The remaining time until the PHY is expected to be in PhySimWifiPhy::IDLE again
   */
  Time GetDelayUntilIdle (void);
  /**
   * \returns The time at which the last frame reception started.
   */
  Time GetLastRxStartTime (void) const;

  /**
   * Handles all processing which shall be done when a transmission is started.
   *
   * It will notify all WifiPhyListener objects who have registered for
   * WifiPhyListener::NotifyTxStart and will finally perform the state change itself.
   */
  void SwitchToTx (Time txDuration, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  /**
   * Handles all processing which shall be done when a preamble has been detected, i.e. signal detection and
   * initial channel estimation were successful.
   *
   * It will perform the state change itself.
   */
  void SwitchToSync (Time syncDuration, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  /**
   * This function is called if at the end of the PhySimWifiPhy::SYNCING state the signal header could be
   * decoded successfully.
   *
   * In this case, PhySimWifiPhyStateHelper::DoSwitchFromSync() is called.
   */
  void SwitchFromSyncEndOk (Ptr<Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  /**
   * This function is called if at the end of the PhySimWifiPhy::SYNCING state the signal header could not be
   * decoded successfully.
   *
   * In this case, PhySimWifiPhyStateHelper::DoSwitchFromSync is called as well.
   */
  void SwitchFromSyncEndError (Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  /**
   * This function is called in case of a successful packet capture during SYNC state. If this happens, we have to
   * switch from SYNC to SYNC at the EndPreamble event (of the captured packet), which is handled in this method.
   */
  void SwitchFromSyncToSync (Time syncDuration, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  /**
   * Handles all processing which shall be done when a frame header has been decoded successfully, i.e.
   * modulation and coding scheme as well as the frame length have passed the plausibility check and parity
   * bit is correct.
   *
   * It will notify all WifiPhyListener objects who have registered for WifiPhyListener::NotifyRxStart
   * and finally perform the state change itself.
   */
  void SwitchToRx (Time rxDuration);
  /**
   * This function is called if at the end of the PhySimWifiPhy::RX state all data symbols could be
   * decoded successfully (if we would be strict, we couldn't know here already, since the CRC32 check
   * on MAC layer would normally do this. But for now, we will make the decision in PhySimWifiPhy::EndRx).
   *
   * In this case, all WifiPhyListener objects who have registered for
   * WifiPhyListener::NotifyRxEndOk are notified, PhySimWifiPhyStateHelper::DoSwitchFromRx is
   * called and MacLow is informed using WifiPhy::RxOkCallback.
   */
  void SwitchFromRxEndOk (Ptr<Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  /**
   * This function is called if at the end of the PhySimWifiPhy::RX state not all data symbols could be
   * decoded successfully (if we would be strict, we couldn't know here already, since the CRC32 check
   * on MAC layer would normally do this. But for now, we will make the decision in PhySimWifiPhy::EndRx).
   *
   * In this case, all WifiPhyListener objects who have registered for
   * WifiPhyListener::NotifyRxEndError are notified, PhySimWifiPhyStateHelper::DoSwitchFromRx is
   * called and MacLow is informed using WifiPhy::RxErrorCallback.
   */
  void SwitchFromRxEndError (Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  /**
   * This function is called in case of a successful packet capture during RX state. If this happens, we have to
   * switch from RX to SYNC at the EndPreamble event (of the captured packet), which is handled in this method.
   */
  void SwitchFromRxToSync (Time syncDuration, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);
  /**
   * Here a possible switch to PhySimWifiPhy::CCA_BUSY is performed if we are not already in this state. In
   * any case, after this function, the state will be PhySimWifiPhy::CCA_BUSY.
   *
   * If the state was PhySimWifiPhy::IDLE before, PhySimWifiPhyStateHelper::LogPreviousIdleAndCcaBusyStates
   * is called to keep track of idle and busy states in the log.
   */
  void SwitchMaybeToCcaBusy (Time duration);

  void SetNetDevice (Ptr<NetDevice> device);

private:
  /**
   * Basically a wrapper function which takes care of the TraceSource PhySimWifiPhyStateHelper::State.
   * A call of this function will trace the transition from a PhySimWifiPhy::IDLE to a PhySimWifiPhy::CCA_BUSY
   * state or the other way around.
   */
  void LogPreviousIdleAndCcaBusyStates (void);

  /**
   * A wrapper function to call WifiPhyListener::NotifyTxStart on all registered listeners
   * \param duration Duration of the transmission
   */
  void NotifyTxStart (Time duration);
  void NotifyWakeup (void);
  /**
   * A wrapper function to call WifiPhyListener::NotifyRxStart on all registered listeners
   * Note: Currently, the DcfManager uses this callback in order to stop its backoff decrementation
   * \param duration Duration of the reception
   */
  void NotifyRxStart (Time duration);
  /**
   * A wrapper function to call WifiPhyListener::NotifyRxEndOk on all registered listeners.
   * Note: Currently, the DcfManager uses this callback in order to continue its backoff decrementation
   */
  void NotifyRxEndOk (void);
  /**
   * A wrapper function to call WifiPhyListener::NotifyRxEndError on all registered listeners
   * Note: Currently, the DcfManager uses this callback in order to continue its backoff decrementation
   */
  void NotifyRxEndError (void);
  /**
   * A wrapper function to call WifiPhyListener::NotifyMaybeCcaBusyStart on all registered listeners
   * \param duration Expected duration for which we will stay in PhySimWifiPhy::CCA_BUSY
   */
  void NotifyMaybeCcaBusyStart (Time duration);
  /**
   * Updates the internal state and performs the logging of the duration of the PhySimWifiPhy::SYNCING state
   * by activating the trace source PhySimWifiPhyStateHelper::State.
   */
  void DoSwitchFromSync (void);
  /**
   * Updates the internal state and performs the logging of the duration of the PhySimWifiPhy::RX state
   * by activating the trace source PhySimWifiPhyStateHelper::State.
   */
  void DoSwitchFromRx (void);

  void PerformStateLogging (Time start, Time duration, enum PhySimWifiPhy::State state);

  typedef std::vector<WifiPhyListener *> Listeners;
  TracedCallback<Ptr<NetDevice>, Time, Time, enum PhySimWifiPhy::State> m_stateLogger;
  Ptr<NetDevice> m_device;

  bool m_syncing;
  bool m_rxing;
  Time m_endTx;
  Time m_endSync;
  Time m_endRx;
  Time m_endCcaBusy;
  Time m_startTx;
  Time m_startSync;
  Time m_startRx;
  Time m_startCcaBusy;
  Time m_previousStateChangeTime;
  Time m_previousStateLogTime;

  Listeners m_listeners;

  // Callbacks for MacLow
  WifiPhy::RxOkCallback m_rxOkCallback;
  WifiPhy::RxErrorCallback m_rxErrorCallback;
};

} // namespace ns3

#endif /* PHYSIM_WIFI_PHY_STATE_HELPER_H */
