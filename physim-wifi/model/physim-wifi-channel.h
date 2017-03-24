/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA, 2009-2010 Jens Mittag, Stylianos Papanastasiou
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
 * Based on yans-wifi-channel.h by:
 *      Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Major modifications by:
 *      Jens Mittag <jens.mittag@kit.edu>
 *      Stylianos Papanastasiou <stylianos@gmail.com>
 */
#ifndef PHYSIM_WIFI_CHANNEL_H
#define PHYSIM_WIFI_CHANNEL_H

#include <vector>
#include <map>
#include <stdint.h>
#include "ns3/packet.h"
#include "ns3/wifi-channel.h"
#include "ns3/wifi-mode.h"
#include "ns3/wifi-preamble.h"

namespace ns3 {

class NetDevice;
class PhySimPropagationLossModel;
class PropagationDelayModel;
class PhySimWifiPhy;
class PhySimWifiPhyTag;


/**
 * \brief The interface of a wireless channel implementation over which multiple PhySimWifiPhy instances are connected
 *        to each other.
 *
 * This interface is currently implemented by the following two classes
 *
 * - PhySimWifiUniformChannel
 * - PhySimWifiManualChannel
 *
 * whereas the first one implements a channel which applies the same propagation loss and delay models to all communication
 * links (e.g. Friis or ThreeLogDistance and Rician), and the second one allows to specify different models for individual
 * communication links.
 *
 * Propagation loss models can be configured through the method PhySimWifiChannel::SetPropagationLossModel. If multiple
 * propagation loss models are "chained", only the first model has to be added through this method.
 */
class PhySimWifiChannel : public WifiChannel
{
public:
  static TypeId GetTypeId (void);
  /**
   * Return the number of devices that are connected to that channel
   * \return The number of NetDevice objects connected
   */
  virtual uint32_t GetNDevices (void) const = 0;
  /**
   * Return the NetDevice object for a specific index
   * \param i The index for which to return the NetDevice object
   * \return  A pointer to the desired NetDevice object
   */
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const = 0;
  /**
   * Attach a PhySimwifiPhy instance to the channel
   * \param phy The pointer to the PhySimWifiPhy object
   */
  virtual void Add (Ptr<PhySimWifiPhy> phy) = 0;
  /**
   * Set the root propagation loss model which shall be used to compute the propagation loss effect
   * \param loss A pointer to the root propagation loss model object
   */
  virtual void SetPropagationLossModel (Ptr<PhySimPropagationLossModel> loss) = 0;
  /**
   * Set the propagation delay model that shall be used to compute the propagation delay through space
   * \param delay A pointer to the propagation delay model object
   */
  virtual void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay) = 0;
  /**
   * Transmits a packet from a sending PhySimWifiPhy object to all other PhySimWifiPhy objects that are
   * connected to this channel.
   * \param sender The pointer of the sending node's PhySimWifiPhy instance
   * \param packet The pointer of the packet that is sent
   * \param tag    The pointer of the associated PhySimWifiPhy packet tag that contains all the metainformation, e.g.
   *               the vector with all the complex time samples
   */
  virtual void Send (Ptr<PhySimWifiPhy> sender, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag) = 0;

};

/**
 * \brief A uniform wireless channel for the PhySimWifi physical layer model
 *
 * This class is expected to be used in tandem with the ns3::PhySimWifiPhy class and applies a PhySimPropagationLossModel and a
 * PropagationDelayModel to all transmitted packets. The channel is uniform, in the sense that the same set of delay and loss
 * models is applied to all communication links.
 */
class PhySimWifiUniformChannel : public PhySimWifiChannel
{
public:
  static TypeId GetTypeId (void);
  PhySimWifiUniformChannel ();
  virtual ~PhySimWifiUniformChannel ();
  /**
   * Return the number of devices that are connected to that channel
   * \return The number of NetDevice objects connected
   */
  uint32_t GetNDevices (void) const;
  /**
   * Return the NetDevice object for a specific index
   * \param i The index for which to return the NetDevice object
   * \return  A pointer to the desired NetDevice object
   */
  Ptr<NetDevice> GetDevice (uint32_t i) const;
  /**
   * Attach a PhySimwifiPhy instance to the channel
   * \param phy The pointer to the PhySimWifiPhy object
   */
  void Add (Ptr<PhySimWifiPhy> phy);
  /**
   * Set the root propagation loss model which shall be used to compute the propagation loss effect
   * \param loss A pointer to the root propagation loss model object
   */
  void SetPropagationLossModel (Ptr<PhySimPropagationLossModel> loss);
  /**
   * Set the propagation delay model that shall be used to compute the propagation delay through space
   * \param delay A pointer to the propagation delay model object
   */
  void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay);
  /**
   * Transmits a packet from a sending PhySimWifiPhy object to all other PhySimWifiPhy objects that are
   * connected to this channel.
   * \param sender The pointer of the sending node's PhySimWifiPhy instance
   * \param packet The pointer of the packet that is sent
   * \param tag    The pointer of the associated PhySimWifiPhy packet tag that contains all the metainformation, e.g.
   *               the vector with all the complex time samples
   */
  void Send (Ptr<PhySimWifiPhy> sender, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);

private:
  typedef std::vector<Ptr<PhySimWifiPhy> > PhyList;
  void Receive (uint32_t i, Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag) const;

  PhyList m_phyList;
  Ptr<PhySimPropagationLossModel> m_loss;
  Ptr<PropagationDelayModel> m_delay;
};

/**
 * \brief An advanced wireless channel implementation for the PhySimWifiPhy based physical layer devices, in which it is
 * possible to define channel conditions on a per link basis.
 *
 * The implementation allows to define the channel conditions between each sender/receiver pair manually and
 * uses only a default/fallback propagation loss and delay model if no specific loss has been defined explicitly. This
 * is very useful to create test scenarios in which you want to fool the MAC layer logic, e.g. if you want to disable
 * CCA mechanisms.
 */
class PhySimWifiManualChannel : public PhySimWifiChannel
{
public:
  static TypeId GetTypeId (void);
  PhySimWifiManualChannel ();
  virtual ~PhySimWifiManualChannel ();
  /**
   * Return the number of devices that are connected to that channel
   * \return The number of NetDevice objects connected
   */
  uint32_t GetNDevices (void) const;
  /**
   * Return the NetDevice object for a specific index
   * \param i The index for which to return the NetDevice object
   * \return  A pointer to the desired NetDevice object
   */
  Ptr<NetDevice> GetDevice (uint32_t i) const;
  /**
   * Attach a PhySimwifiPhy instance to the channel
   * \param phy The pointer to the PhySimWifiPhy object
   */
  void Add (Ptr<PhySimWifiPhy> phy);
  /**
   * Set the root propagation loss model which shall be used to compute the propagation loss effect
   * \param loss A pointer to the root propagation loss model object
   */
  void SetPropagationLossModel (Ptr<PhySimPropagationLossModel> loss);
  /**
   * Set the root propagation loss model for a specific sender/receiver pair.
   * \param sender   The sending node identifier of the communication link
   * \param receiver The receiving node identifier of the communication link
   * \param loss     A pointer to the root propagation loss model object for that link
   */
  void SetExplicitPropagationLossModel (uint32_t sender, uint32_t receiver, Ptr<PhySimPropagationLossModel> loss);
  /**
   * Set the propagation delay model that shall be used to compute the propagation delay through space
   * \param delay A pointer to the propagation delay model object
   */
  void SetPropagationDelayModel (Ptr<PropagationDelayModel> delay);
  /**
   * Set the propagation delay model for a specific sender/receiver pair
   * \param sender   The sending node identifier of the communication link
   * \param receiver The receiving node identifier of the communication link
   * \param delay    A pointer to the propagation delay model object for that link
   */
  void SetExplicitPropagationDelayModel (uint32_t sender, uint32_t receiver, Ptr<PropagationDelayModel> delay);
  /**
   * Transmits a packet from a sending PhySimWifiPhy object to all other PhySimWifiPhy objects that are
   * connected to this channel.
   * \param sender The pointer of the sending node's PhySimWifiPhy instance
   * \param packet The pointer of the packet that is sent
   * \param tag    The pointer of the associated PhySimWifiPhy packet tag that contains all the metainformation, e.g.
   *               the vector with all the complex time samples
   */
  void Send (Ptr<PhySimWifiPhy> sender, Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag);

private:
  typedef std::vector<Ptr<PhySimWifiPhy> > PhyList;
  void Receive (uint32_t i, Ptr<Packet> packet, Ptr<PhySimWifiPhyTag> tag) const;


  PhyList m_phyList;
  Ptr<PhySimPropagationLossModel> m_defaultLoss;
  Ptr<PropagationDelayModel> m_defaultDelay;

  // A two-step map for storing the delay and propagation loss configurations
  // of each sender->receiver constellation
  std::map<uint32_t, std::map<uint32_t, Ptr<PhySimPropagationLossModel> > > m_loss;
  std::map<uint32_t, std::map<uint32_t, Ptr<PropagationDelayModel> > > m_delay;

};

} // namespace ns3


#endif /* PHYSIM_WIFI_CHANNEL_H */
