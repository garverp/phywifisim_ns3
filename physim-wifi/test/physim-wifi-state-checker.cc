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

#include "physim-wifi-state-checker.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhySimWifiStateChecker");

PhySimWifiStateChecker::Transition::Transition (uint32_t from, uint32_t to)
{
  m_from = from;
  m_to = to;
}

PhySimWifiStateChecker::Transition::~Transition ()
{
}

uint32_t
PhySimWifiStateChecker::Transition::GetFrom ()
{
  return m_from;
}

uint32_t
PhySimWifiStateChecker::Transition::GetTo ()
{
  return m_to;
}

PhySimWifiStateChecker::ProhibitedPeriod::ProhibitedPeriod (Time start, Time end, uint32_t state)
{
  m_start = start;
  m_end = end;
  m_state = state;
}

PhySimWifiStateChecker::ProhibitedPeriod::~ProhibitedPeriod ()
{
}

Time
PhySimWifiStateChecker::ProhibitedPeriod::GetStart ()
{
  return m_start;
}

Time
PhySimWifiStateChecker::ProhibitedPeriod::GetEnd ()
{
  return m_end;
}

uint32_t
PhySimWifiStateChecker::ProhibitedPeriod::GetState ()
{
  return m_state;
}

void
PhySimWifiStateChecker::PeriodSet::Add (Ptr<ProhibitedPeriod> p)
{
  m_periods.insert (p);
}

uint32_t
PhySimWifiStateChecker::PeriodSet::Size () const
{
  return m_periods.size ();
}

std::set<Ptr<PhySimWifiStateChecker::ProhibitedPeriod> >::iterator
PhySimWifiStateChecker::PeriodSet::Begin ()
{
  return m_periods.begin ();
}

std::set<Ptr<PhySimWifiStateChecker::ProhibitedPeriod> >::iterator
PhySimWifiStateChecker::PeriodSet::End ()
{
  return m_periods.end ();
}

PhySimWifiStateChecker::PhySimWifiStateChecker ()
  : m_defaultState (0)
{
}

PhySimWifiStateChecker::~PhySimWifiStateChecker ()
{
}

void
PhySimWifiStateChecker::RegisterNodeId (uint32_t node)
{
  if (m_nodes.find (node) == m_nodes.end ())
    {
      m_nodes.insert (node);
      m_nodeStates[node] = m_defaultState;
    }
}

void
PhySimWifiStateChecker::RegisterState (uint32_t state, std::string label, bool isDefault)
{
  if (m_states.find (state) == m_states.end ())
    {
      m_states.insert (state);
      m_labels[state] = label;
      if (isDefault)
        {
          m_defaultState = state;
          std::set<uint32_t>::iterator it;
          for (it = m_nodes.begin (); it != m_nodes.end (); it++)
            {
              m_nodeStates[*it] = m_defaultState;
            }
        }
    }
}

void
PhySimWifiStateChecker::RegisterStateTransition (uint32_t from, uint32_t to)
{
  Ptr<Transition> t = CreateObject<Transition> (from, to);
  m_transitions.insert (t);
}

void
PhySimWifiStateChecker::RegisterProhibitedStatePeriod (uint32_t node, uint32_t state, Time start, Time end)
{
  NS_LOG_FUNCTION (node << state << start << end);
  Ptr<ProhibitedPeriod> p = CreateObject<ProhibitedPeriod> (start, end, state);

  // Create a new set of prohibited states, if none is yet existing
  Ptr<PeriodSet> set;
  if (m_prohibitions.find (node) == m_prohibitions.end ())
    {
      set = CreateObject<PeriodSet> ();
      m_prohibitions[node] = set;
    }
  else
    {
      set = m_prohibitions[node];
    }

  // Add new prohibition to set
  set->Add (p);
}

void
PhySimWifiStateChecker::Reset ()
{
  // delete all prohibitions
  m_prohibitions.clear ();

  // put everbody back in default state
  std::set<uint32_t>::iterator itx;
  for (itx = m_nodes.begin (); itx != m_nodes.end (); itx++)
    {
      m_nodeStates[*itx] = m_defaultState;
    }
}

bool
PhySimWifiStateChecker::PerformStateTransition (uint32_t node, uint32_t newState)
{
  std::map<uint32_t, uint32_t>::iterator it = m_nodeStates.find (node);
  if (it != m_nodeStates.end ())
    {
      uint32_t from = it->second;

      // Iterate over all registered and allowed transitions to check whether this
      // transition is allowed or not.
      std::set<Ptr<Transition> >::iterator itx;
      for (itx = m_transitions.begin (); itx != m_transitions.end (); itx++)
        {
          Ptr<Transition> t = *itx;
          if ( (t->GetFrom () == from) && (t->GetTo () == newState) )
            {
              m_nodeStates[node] = newState;
              NS_LOG_INFO ("Transition from '" << GetLabel (from) << "' to '" << GetLabel (newState) << "' for Node '" << node << "' allowed in general!");

              // finally check if the new state we are going into is prohibited or not
              return CheckForProhibitions (node, from, newState);
            }

        }

      // If we get here, the performed transition is not valid
      NS_LOG_INFO ("Transition from '" << GetLabel (from) << "' to '" << GetLabel (newState) << "' for Node '" << node << "' rejected (reason: transition not allowed)");
      return false;
    }

  // If we are here, we do not know a node with this identifier, which means
  // the user created a problem and should be notified
  NS_LOG_INFO ("Transition from '?' to '" << GetLabel (newState) << "' for Node '" << node << "' rejected (reason: node unknown)");
  return false;
}

bool
PhySimWifiStateChecker::CheckForProhibitions (uint32_t node, uint32_t oldState, uint32_t newState)
{
  if (m_prohibitions.find (node) != m_prohibitions.end ())
    {
      Ptr<PeriodSet> set = m_prohibitions[node];
      NS_LOG_DEBUG (" --> found " << set->Size () << " prohibitions");
      std::set<Ptr<ProhibitedPeriod> >::iterator it;
      for (it = set->Begin (); it != set->End (); it++)
        {
          Ptr<ProhibitedPeriod> period = *it;
          Time now = Simulator::Now ();
          NS_LOG_DEBUG (" --> now = " << now);
          NS_LOG_DEBUG (" --> period->GetState() = " << period->GetState ());
          NS_LOG_DEBUG (" --> period->GetStart() = " << period->GetStart ());
          NS_LOG_DEBUG (" --> period->GetEnd() = " << period->GetEnd ());
          NS_LOG_DEBUG (" --> oldState = " << oldState);
          NS_LOG_DEBUG (" --> newState = " << newState);
          if ( (oldState == period->GetState () && now >= period->GetStart () && now <= period->GetEnd ())
               || (newState == period->GetState () && now >= period->GetStart () && now < period->GetEnd ()) )
            {
              NS_LOG_INFO ("VETO: Transition from '" << GetLabel (oldState) << "' to '" << GetLabel (newState) << "' for Node '" << node << "' EXPLICITLY prohibited!");
              return false;
            }
        }
    }
  return true;
}

std::string
PhySimWifiStateChecker::GetLabel (uint32_t state)
{
  return m_labels[state];
}
