/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010, Jens Mittag, Karlsruhe Institute of Technology
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
 * Authors: Jens Mittag <jens.mittag@kit.edu>
 */

#ifndef PHYSIM_WIFI_STATE_CHECKER_H_
#define PHYSIM_WIFI_STATE_CHECKER_H_

#include <set>
#include <map>
#include "ns3/nstime.h"
#include "ns3/object.h"

using namespace ns3;

class PhySimWifiStateChecker : public Object
{

public:
  class Transition : public Object
  {
public:
    Transition (uint32_t from, uint32_t to);
    virtual ~Transition ();
    uint32_t GetFrom ();
    uint32_t GetTo ();
private:
    uint32_t m_from;
    uint32_t m_to;
  };

  class ProhibitedPeriod : public Object
  {
public:
    ProhibitedPeriod (Time start, Time end, uint32_t state);
    virtual ~ProhibitedPeriod ();
    Time GetStart ();
    Time GetEnd ();
    uint32_t GetState ();
private:
    Time m_start;
    Time m_end;
    uint32_t m_state;
  };

  class PeriodSet : public Object
  {
public:
    void Add (Ptr<PhySimWifiStateChecker::ProhibitedPeriod> p);
    uint32_t Size () const;
    std::set<Ptr<PhySimWifiStateChecker::ProhibitedPeriod> >::iterator Begin ();
    std::set<Ptr<PhySimWifiStateChecker::ProhibitedPeriod> >::iterator End ();

private:
    std::set<Ptr<PhySimWifiStateChecker::ProhibitedPeriod> > m_periods;
  };

  PhySimWifiStateChecker ();
  virtual ~PhySimWifiStateChecker ();

  /**
   * Register a node for which the physical layer states and transitions will be checked
   * \param node The ID of the node to be included in the checks
   */
  void RegisterNodeId (uint32_t node);

  /**
   * Register a state that is allowed to occur / take place in the physical layer
   * \param state   The integer value of the physical layer state
   * \param label   The human-readable label for this state
   * \param default A flag that indicates whether this is the default state for all nodes in the beginning
   */
  void RegisterState (uint32_t state, std::string label = "", bool isDefault = false);
  /**
   * Register a possible and valid state transition
   * \param from The source physical layer state of the transition
   * \param to   The target physical layer state of the transition
   */
  void RegisterStateTransition (uint32_t from, uint32_t to);
  /**
   * Registers the prohibition of specific states during a given time window
   * \param node  The identifier of the node at which this event shall happen
   * \param state The state that shall start at this node and the given time stamp
   * \param start The starting time of the time window during which the above state is prohibited
   * \param stop  The end time of the time window during which the above state is prohibited
   */
  void RegisterProhibitedStatePeriod (uint32_t node, uint32_t state, Time start, Time end);

  void Reset ();

  /**
   * Notification about a specific state transition that happened during the simulation. This function then checks whether
   * the new state is valid, by checking whether a transition from the current state to the new state exists. Further, it will
   * record the expected duration of the new state, in order to make sure that the new state lasts at least as long as expected.
   * \param node             The identifier of the node at which a state transition was monitored
   * \param newState         The identifier of the new physical layer state
   * \param expectedDuration The expected duration of the new state
   * \return                 Feedback whether the transition was allowed (true) or not (false).
   */
  bool PerformStateTransition (uint32_t node, uint32_t newState);

  bool CheckForProhibitions (uint32_t node, uint32_t oldState, uint32_t newState);

private:
  std::string GetLabel (uint32_t state);

  std::set<uint32_t> m_nodes;                           // The node IDs for which to check
  std::map<uint32_t, uint32_t> m_nodeStates;            // The current PhySimWifi PHY state of all nodes
  std::set<Ptr<Transition> > m_transitions;             // The registered transitions
  std::map<uint32_t, Ptr<PeriodSet> > m_prohibitions;   // The per node list of prohibited state periods

  std::set<uint32_t> m_states;                  // The list of allowed PHY states
  std::map<uint32_t, std::string> m_labels;     // The human-readable labels for each state

  uint32_t m_defaultState;

};

#endif /* PHYSIM_WIFI_STATE_CHECKER_H_ */
