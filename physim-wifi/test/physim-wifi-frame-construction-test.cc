/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Jens Mittag, Stylianos Papanastasiou
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

#include "ns3/node.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-preamble.h"
#include "ns3/wifi-phy.h"
#include "ns3/packet.h"
#include "ns3/config.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/object-factory.h"
#include "ns3/physim-wifi-phy.h"
#include "ns3/physim-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "physim-wifi-frame-construction-test.h"
#include <itpp/itcomm.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhySimWifiFrameConstructionTest");

PhySimWifiFrameConstructionTest::PhySimWifiFrameConstructionTest ()
  : TestCase ("PhySim WiFi frame construction test case")
{
  m_refShortSymbol = "0.023+0.023i -0.132+0.002i -0.013-0.079i 0.143-0.013i 0.092+0i 0.143-0.013i -0.013-0.079i -0.132+0.002i 0.046+0.046i 0.002-0.132i -0.079-0.013i -0.013+0.143i 0+0.092i -0.013+0.143i -0.079-0.013i 0.002-0.132i 0.046+0.046i -0.132+0.002i -0.013-0.079i 0.143-0.013i 0.092+0i 0.143-0.013i -0.013-0.079i -0.132+0.002i 0.046+0.046i 0.002-0.132i -0.079-0.013i -0.013+0.143i 0+0.092i -0.013+0.143i -0.079-0.013i 0.002-0.132i 0.046+0.046i -0.132+0.002i -0.013-0.079i 0.143-0.013i 0.092+0i 0.143-0.013i -0.013-0.079i -0.132+0.002i 0.046+0.046i 0.002-0.132i -0.079-0.013i -0.013+0.143i 0+0.092i -0.013+0.143i -0.079-0.013i 0.002-0.132i 0.046+0.046i -0.132+0.002i -0.013-0.079i 0.143-0.013i 0.092+0i 0.143-0.013i -0.013-0.079i -0.132+0.002i 0.046+0.046i 0.002-0.132i -0.079-0.013i -0.013+0.143i 0+0.092i -0.013+0.143i -0.079-0.013i 0.002-0.132i 0.046+0.046i -0.132+0.002i -0.013-0.079i 0.143-0.013i 0.092+0i 0.143-0.013i -0.013-0.079i -0.132+0.002i 0.046+0.046i 0.002-0.132i -0.079-0.013i -0.013+0.143i 0+0.092i -0.013+0.143i -0.079-0.013i 0.002-0.132i 0.046+0.046i -0.132+0.002i -0.013-0.079i 0.143-0.013i 0.092+0i 0.143-0.013i -0.013-0.079i -0.132+0.002i 0.046+0.046i 0.002-0.132i -0.079-0.013i -0.013+0.143i 0+0.092i -0.013+0.143i -0.079-0.013i 0.002-0.132i 0.046+0.046i -0.132+0.002i -0.013-0.079i 0.143-0.013i 0.092+0i 0.143-0.013i -0.013-0.079i -0.132+0.002i 0.046+0.046i 0.002-0.132i -0.079-0.013i -0.013+0.143i 0+0.092i -0.013+0.143i -0.079-0.013i 0.002-0.132i 0.046+0.046i -0.132+0.002i -0.013-0.079i 0.143-0.013i 0.092+0i 0.143-0.013i -0.013-0.079i -0.132+0.002i 0.046+0.046i 0.002-0.132i -0.079-0.013i -0.013+0.143i 0+0.092i -0.013+0.143i -0.079-0.013i 0.002-0.132i 0.046+0.046i -0.132+0.002i -0.013-0.079i 0.143-0.013i 0.092+0i 0.143-0.013i -0.013-0.079i -0.132+0.002i 0.046+0.046i 0.002-0.132i -0.079-0.013i -0.013+0.143i 0+0.092i -0.013+0.143i -0.079-0.013i 0.002-0.132i 0.046+0.046i -0.132+0.002i -0.013-0.079i 0.143-0.013i 0.092+0i 0.143-0.013i -0.013-0.079i -0.132+0.002i 0.046+0.046i 0.002-0.132i -0.079-0.013i -0.013+0.143i 0+0.092i -0.013+0.143i -0.079-0.013i 0.002-0.132i -0.055+0.023i";
  m_refLongSymbol = "-0.055+0.023i 0.012-0.098i 0.092-0.106i -0.092-0.115i -0.003-0.054i 0.075+0.074i -0.127+0.021i -0.122+0.017i -0.035+0.151i -0.056+0.022i -0.06-0.081i 0.07-0.014i 0.082-0.092i -0.131-0.065i -0.057-0.039i 0.037-0.098i 0.062+0.062i 0.119+0.004i -0.022-0.161i 0.059+0.015i 0.024+0.059i -0.137+0.047i 0.001+0.115i 0.053-0.004i 0.098+0.026i -0.038+0.106i -0.115+0.055i 0.06+0.088i 0.021-0.028i 0.097-0.083i 0.04+0.111i -0.005+0.12i 0.156+0i -0.005-0.12i 0.04-0.111i 0.097+0.083i 0.021+0.028i 0.06-0.088i -0.115-0.055i -0.038-0.106i 0.098-0.026i 0.053+0.004i 0.001-0.115i -0.137-0.047i 0.024-0.059i 0.059-0.015i -0.022+0.161i 0.119-0.004i 0.062-0.062i 0.037+0.098i -0.057+0.039i -0.131+0.065i 0.082+0.092i 0.07+0.014i -0.06+0.081i -0.056-0.022i -0.035-0.151i -0.122-0.017i -0.127-0.021i 0.075-0.074i -0.003+0.054i -0.092+0.115i 0.092+0.106i 0.012+0.098i -0.156+0i 0.012-0.098i 0.092-0.106i -0.092-0.115i -0.003-0.054i 0.075+0.074i -0.127+0.021i -0.122+0.017i -0.035+0.151i -0.056+0.022i -0.06-0.081i 0.07-0.014i 0.082-0.092i -0.131-0.065i -0.057-0.039i 0.037-0.098i 0.062+0.062i 0.119+0.004i -0.022-0.161i 0.059+0.015i 0.024+0.059i -0.137+0.047i 0.001+0.115i 0.053-0.004i 0.098+0.026i -0.038+0.106i -0.115+0.055i 0.06+0.088i 0.021-0.028i 0.097-0.083i 0.04+0.111i -0.005+0.12i 0.156+0i -0.005-0.12i 0.04-0.111i 0.097+0.083i 0.021+0.028i 0.06-0.088i -0.115-0.055i -0.038-0.106i 0.098-0.026i 0.053+0.004i 0.001-0.115i -0.137-0.047i 0.024-0.059i 0.059-0.015i -0.022+0.161i 0.119-0.004i 0.062-0.062i 0.037+0.098i -0.057+0.039i -0.131+0.065i 0.082+0.092i 0.07+0.014i -0.06+0.081i -0.056-0.022i -0.035-0.151i -0.122-0.017i -0.127-0.021i 0.075-0.074i -0.003+0.054i -0.092+0.115i 0.092+0.106i 0.012+0.098i -0.156+0i 0.012-0.098i 0.092-0.106i -0.092-0.115i -0.003-0.054i 0.075+0.074i -0.127+0.021i -0.122+0.017i -0.035+0.151i -0.056+0.022i -0.06-0.081i 0.07-0.014i 0.082-0.092i -0.131-0.065i -0.057-0.039i 0.037-0.098i 0.062+0.062i 0.119+0.004i -0.022-0.161i 0.059+0.015i 0.024+0.059i -0.137+0.047i 0.001+0.115i 0.053-0.004i 0.098+0.026i -0.038+0.106i -0.115+0.055i 0.06+0.088i 0.021-0.028i 0.097-0.083i 0.04+0.111i -0.005+0.12i 0.109+0i";
  m_refSIGNAL = "0.109+0i 0.033-0.044i -0.002-0.038i -0.081+0.084i 0.007-0.1i -0.001-0.113i -0.021-0.005i 0.136-0.105i 0.098-0.044i 0.011-0.002i -0.033+0.044i -0.06+0.124i 0.01+0.097i 0-0.008i 0.018-0.083i -0.069+0.027i -0.219+0i -0.069-0.027i 0.018+0.083i 0+0.008i 0.01-0.097i -0.06-0.124i -0.033-0.044i 0.011+0.002i 0.098+0.044i 0.136+0.105i -0.021+0.005i -0.001+0.113i 0.007+0.1i -0.081-0.084i -0.002+0.038i 0.033+0.044i 0.062+0i 0.057+0.052i 0.016+0.174i 0.035+0.116i -0.051-0.202i 0.011+0.036i 0.089+0.209i -0.049-0.008i -0.035+0.044i 0.017-0.059i 0.053-0.017i 0.099+0.1i 0.034-0.148i -0.003-0.094i -0.12+0.042i -0.136-0.07i -0.031+0i -0.136+0.07i -0.12-0.042i -0.003+0.094i 0.034+0.148i 0.099-0.1i 0.053+0.017i 0.017+0.059i -0.035-0.044i -0.049+0.008i 0.089-0.209i 0.011-0.036i -0.051+0.202i 0.035-0.116i 0.016-0.174i 0.057-0.052i 0.062+0i 0.033-0.044i -0.002-0.038i -0.081+0.084i 0.007-0.1i -0.001-0.113i -0.021-0.005i 0.136-0.105i 0.098-0.044i 0.011-0.002i -0.033+0.044i -0.06+0.124i 0.01+0.097i 0-0.008i 0.018-0.083i -0.069+0.027i -0.139+0.05i";
  m_refFirstDATAblock = "-0.139+0.05i 0.004+0.014i 0.011-0.1i -0.097-0.02i 0.062+0.081i 0.124+0.139i 0.104-0.015i 0.173-0.14i -0.04+0.006i -0.133+0.009i -0.002-0.043i -0.047+0.092i -0.109+0.082i -0.024+0.01i 0.096+0.019i 0.019-0.023i -0.087-0.049i 0.002+0.058i -0.021+0.228i -0.103+0.023i -0.019-0.175i 0.018+0.132i -0.071+0.16i -0.153-0.062i -0.107+0.028i 0.055+0.14i 0.07+0.103i -0.056+0.025i -0.043+0.002i 0.016-0.118i 0.026-0.071i 0.033+0.177i 0.02-0.021i 0.035-0.088i -0.008+0.101i -0.035-0.01i 0.065+0.03i 0.092-0.034i 0.032-0.123i -0.018+0.092i 0-0.006i -0.006-0.056i -0.019+0.04i 0.053-0.131i 0.022-0.133i 0.104-0.032i 0.163-0.045i -0.105-0.03i -0.11-0.069i -0.008-0.092i -0.049-0.043i 0.085-0.017i 0.09+0.063i 0.015+0.153i 0.049+0.094i 0.011+0.034i -0.012+0.012i -0.015-0.017i -0.061+0.031i -0.07-0.04i 0.011-0.109i 0.037-0.06i -0.003-0.178i -0.007-0.128i -0.059+0.1i 0.004+0.014i 0.011-0.1i -0.097-0.02i 0.062+0.081i 0.124+0.139i 0.104-0.015i 0.173-0.14i -0.04+0.006i -0.133+0.009i -0.002-0.043i -0.047+0.092i -0.109+0.082i -0.024+0.01i 0.096+0.019i 0.019-0.023i";
  m_refSecondDATAblock = "-0.058+0.016i -0.096-0.045i -0.11+0.003i -0.07+0.216i -0.04+0.059i 0.01-0.056i 0.034+0.065i 0.117+0.033i 0.078-0.133i -0.043-0.146i 0.158-0.071i 0.254-0.021i 0.068+0.117i -0.044+0.114i -0.035+0.041i 0.085+0.07i 0.12+0.01i 0.057+0.055i 0.063+0.188i 0.091+0.149i -0.017-0.039i -0.078-0.075i 0.049+0.079i -0.014-0.007i 0.03-0.027i 0.08+0.054i -0.186-0.067i -0.039-0.027i 0.043-0.072i -0.092-0.089i 0.029+0.105i -0.144+0.003i -0.069-0.041i 0.132+0.057i -0.126+0.07i -0.031+0.109i 0.161-0.009i 0.056-0.046i -0.004+0.028i -0.049+0i -0.078-0.005i 0.015-0.087i 0.149-0.104i -0.021-0.051i -0.154-0.106i 0.024+0.03i 0.046+0.123i -0.004-0.098i -0.061-0.128i -0.024-0.038i 0.066-0.048i -0.067+0.027i 0.054-0.05i 0.171-0.049i -0.108+0.132i -0.161-0.019i -0.07-0.072i -0.177+0.049i -0.172-0.05i 0.051-0.075i 0.122-0.057i 0.009-0.044i -0.012-0.021i 0.004+0.009i -0.03+0.081i -0.096-0.045i -0.11+0.003i -0.07+0.216i -0.04+0.059i 0.01-0.056i 0.034+0.065i 0.117+0.033i 0.078-0.133i -0.043-0.146i 0.158-0.071i 0.254-0.021i 0.068+0.117i -0.044+0.114i -0.035+0.041i 0.085+0.07i";
  m_refThirdDATAblock = "0.001+0.011i -0.099-0.048i 0.054-0.196i 0.124+0.035i 0.092+0.045i -0.037-0.066i -0.021-0.004i 0.042-0.065i 0.061+0.048i 0.046+0.004i -0.063-0.045i -0.102+0.152i -0.039-0.019i -0.005-0.106i 0.083+0.031i 0.226+0.028i 0.14-0.01i -0.132-0.033i -0.116+0.088i 0.023+0.052i -0.171-0.08i -0.246-0.025i -0.062-0.038i -0.055-0.062i -0.004-0.06i 0.034-0i -0.03+0.021i 0.075-0.122i 0.043-0.08i -0.022+0.041i 0.026+0.013i -0.031-0.018i 0.059+0.008i 0.109+0.078i 0.002+0.101i -0.016+0.054i -0.059+0.07i 0.017+0.114i 0.104-0.034i -0.024-0.059i -0.081+0.051i -0.04-0.069i -0.069+0.058i -0.067+0.117i 0.007-0.131i 0.009+0.028i 0.075+0.117i 0.118+0.03i -0.041+0.148i 0.005+0.098i 0.026+0.002i -0.116+0.045i -0.02+0.084i 0.101+0.006i 0.205-0.064i 0.073-0.063i -0.174-0.118i -0.024+0.026i -0.041+0.129i -0.042-0.053i 0.148-0.126i -0.03-0.049i -0.015-0.021i 0.089-0.069i -0.119+0.011i -0.099-0.048i 0.054-0.196i 0.124+0.035i 0.092+0.045i -0.037-0.066i -0.021-0.004i 0.042-0.065i 0.061+0.048i 0.046+0.004i -0.063-0.045i -0.102+0.152i -0.039-0.019i -0.005-0.106i 0.083+0.031i 0.226+0.028i";
  m_refFourthDATAblock = "0.085-0.065i 0.034-0.142i 0.004-0.012i 0.126-0.043i 0.055+0.068i -0.02+0.077i 0.008-0.056i -0.034+0.046i -0.04-0.134i -0.056-0.131i 0.014+0.097i 0.045-0.009i -0.113-0.17i -0.065-0.23i 0.065-0.011i 0.011+0.048i -0.091-0.059i -0.11+0.024i 0.074-0.034i 0.124+0.022i -0.037+0.071i 0.015+0.002i 0.028+0.099i -0.062+0.068i 0.064+0.016i 0.078+0.156i 0.009+0.219i 0.147+0.024i 0.106+0.03i -0.08+0.143i -0.049-0.1i -0.036-0.082i -0.089+0.021i -0.07-0.029i -0.086+0.048i -0.066-0.015i -0.024+0.002i -0.03-0.023i -0.032+0.02i -0.002+0.212i 0.158-0.024i 0.141-0.119i -0.146+0.058i -0.155+0.083i -0.002-0.03i 0.018-0.129i 0.012-0.018i -0.008-0.037i 0.031+0.04i 0.023+0.097i 0.014-0.039i 0.05+0.019i -0.072-0.141i -0.023-0.051i 0.024+0.099i -0.127-0.116i 0.094+0.102i 0.183+0.098i -0.04-0.02i 0.065+0.077i 0.088-0.147i -0.039-0.059i -0.057+0.124i -0.077+0.02i 0.03-0.12i 0.034-0.142i 0.004-0.012i 0.126-0.043i 0.055+0.068i -0.02+0.077i 0.008-0.056i -0.034+0.046i -0.04-0.134i -0.056-0.131i 0.014+0.097i 0.045-0.009i -0.113-0.17i -0.065-0.23i 0.065-0.011i 0.011+0.048i";
  m_refFifthDATAblock = "-0.026-0.021i -0.002+0.041i 0.001+0.071i -0.037-0.117i -0.106-0.062i 0.002+0.057i -0.008-0.011i 0.019+0.072i 0.016+0.059i -0.065-0.077i 0.142-0.062i 0.087+0.025i -0.003-0.103i 0.107-0.152i -0.054+0.036i -0.03-0.003i 0.058-0.02i -0.028+0.007i -0.027-0.099i 0.049-0.075i 0.174+0.031i 0.134+0.156i 0.06+0.077i -0.01-0.022i -0.084+0.04i -0.074+0.011i -0.163+0.054i -0.052-0.008i 0.076-0.042i 0.043+0.101i 0.058-0.018i 0.003-0.09i 0.059-0.018i 0.023-0.031i 0.007-0.017i 0.066-0.017i -0.135-0.098i -0.056-0.081i 0.089+0.154i 0.12+0.122i 0.102+0.001i -0.141+0.102i 0.006-0.011i 0.057-0.039i -0.059+0.066i 0.132+0.111i 0.012+0.114i 0.047-0.106i 0.16-0.099i -0.076+0.084i -0.049+0.073i 0.005-0.086i -0.052-0.108i -0.073+0.129i -0.129-0.034i -0.153-0.111i -0.193+0.098i -0.107-0.068i 0.004-0.009i -0.039+0.024i -0.054-0.079i 0.024+0.084i 0.052-0.002i 0.028-0.044i 0.04+0.018i -0.002+0.041i 0.001+0.071i -0.037-0.117i -0.106-0.062i 0.002+0.057i -0.008-0.011i 0.019+0.072i 0.016+0.059i -0.065-0.077i 0.142-0.062i 0.087+0.025i -0.003-0.103i 0.107-0.152i -0.054+0.036i -0.03-0.003i";
  m_refSixthDATAblock = "0.039-0.09i 0.029+0.025i 0.086-0.029i 0.087-0.082i 0.003-0.036i -0.096-0.089i -0.073-0.046i 0.105-0.02i 0.193+0.018i -0.053-0.073i -0.118-0.149i 0.019-0.019i -0.042+0.026i 0.041+0.009i 0.028-0.076i -0.038-0.068i -0.011+0.01i -0.134-0.064i 0.069-0.067i 0.057+0.006i -0.134+0.098i 0.152+0.036i 0.041-0.085i -0.099-0.049i 0.089-0.099i -0.046+0.018i -0.112+0.135i -0.064+0.018i -0.022+0.053i 0.041+0.077i -0.021+0.145i 0.007+0.179i 0.059+0.041i 0.023+0.064i 0.062+0.022i 0.11-0.081i -0.016-0.054i -0.014-0.017i 0.171+0.008i 0.07-0.027i -0.015+0.002i -0.012+0.053i -0.125+0.009i -0.04+0.012i 0.036+0.114i 0.007+0.09i -0.016-0.082i -0.008-0.013i 0.091+0.03i 0.072-0.068i 0.051+0.063i -0.004+0.049i -0.13-0.048i -0.121+0.061i -0.095+0.078i 0.011+0.005i 0.049+0.001i -0.014-0.011i 0.009-0.063i -0.031+0.04i -0.011+0.004i -0.033-0.111i -0.115+0.137i -0.025+0.049i 0.02-0.16i 0.029+0.025i 0.086-0.029i 0.087-0.082i 0.003-0.036i -0.096-0.089i -0.073-0.046i 0.105-0.02i 0.193+0.018i -0.053-0.073i -0.118-0.149i 0.019-0.019i -0.042+0.026i 0.041+0.009i 0.028-0.076i -0.038-0.068i -0.006+0.005i";
}

PhySimWifiFrameConstructionTest::~PhySimWifiFrameConstructionTest ()
{
}

void
PhySimWifiFrameConstructionTest::DoRun (void)
{

  PhySimWifiPhy::ClearCache ();
  PhySimWifiPhy::ResetRNG ();

  // Do not use a random scrambler, else the output won't match the reference
  Config::SetDefault ("ns3::PhySimScrambler::UseFixedScrambler", BooleanValue (true) );
  // Also enable the transformation from IT++ to IEEE notation
  Config::SetDefault ("ns3::PhySimOFDMSymbolCreator::IEEECompliantMode", BooleanValue (true) );
  Config::SetDefault ("ns3::PhySimSignalDetector::IEEECompliantMode", BooleanValue (true) );

  // Create a channel object
  Ptr<PhySimWifiChannel> channel = CreateObject<PhySimWifiUniformChannel> ();
  // Create a PHY object
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerEnd", DoubleValue (0.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerStart", DoubleValue (0.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxPowerLevels", UintegerValue (1) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxGain", DoubleValue (0.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::RxGain", DoubleValue (0.0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::TxCenterFrequencyTolerance", UintegerValue (0) );
  Config::SetDefault ("ns3::PhySimWifiPhy::NormalizeOFDMSymbols", BooleanValue (false) );
  Ptr<PhySimWifiPhy> phy = CreateObject<PhySimWifiPhy> ();

  // Attach PHY object to channel
  phy->SetChannel (channel);

  // Connect trace source for Tx events
  phy->TraceConnectWithoutContext ("Tx", MakeCallback (&PhySimWifiFrameConstructionTest::PhyTxCallback, this));

  // Also create a net device object and a mobility object
  Ptr<Node> node = CreateObject<Node> ();
  Ptr<WifiNetDevice> device = CreateObject<WifiNetDevice> ();
  Ptr<MobilityModel> mobility = CreateObject<ConstantPositionMobilityModel> ();
  mobility->SetPosition (Vector (1.0, 0.0, 0.0));
  node->AggregateObject (mobility);
  phy->SetMobility (node);
  phy->SetDevice (device);

  // Create a packet object (which means we take the bit sequence that represents the packet payload
  // used in the IEEE 802.11 (2007) standard in Annex G
  itpp::bvec bitSequence = "0 0 1 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 1 0 0 0 0 1 0 1 1 0 0 1 1 1 1 1 0 1 1 0 0 0 1 1 0 0 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1 1 0 1 0 1 1 1 0 0 0 0 0 0 0 0 0 1 1 1 1 0 0 1 0 0 0 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 1 0 0 0 0 1 0 1 1 0 1 0 1 1 1 0 1 1 1 0 0 1 1 1 1 0 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 1 0 0 1 0 1 1 1 1 0 1 1 0 1 0 0 1 1 1 1 0 0 0 1 1 0 1 0 0 0 0 0 0 0 1 0 0 0 1 0 0 0 1 1 0 0 1 0 0 1 1 1 0 1 0 0 1 0 1 1 0 1 1 1 0 0 1 1 0 0 0 0 1 0 1 1 0 0 0 1 0 1 1 1 0 0 0 0 0 0 1 0 0 1 1 0 0 1 1 1 0 0 0 0 0 1 1 1 0 1 0 0 0 0 1 1 0 0 1 0 0 1 1 1 0 1 1 0 1 0 1 1 0 0 0 0 0 0 1 0 0 1 1 1 1 0 1 1 0 0 1 1 0 0 1 1 0 0 0 0 0 0 1 0 0 0 0 1 0 0 1 1 0 1 0 0 1 0 1 1 0 0 1 1 0 1 1 1 0 1 0 0 1 0 1 1 0 0 1 1 1 0 1 1 0 1 0 0 1 0 1 1 0 0 0 1 0 1 1 1 0 1 0 0 1 1 1 1 0 0 0 1 1 0 1 0 0 0 1 0 1 0 0 0 0 0 0 1 0 0 0 1 0 1 0 0 0 0 1 1 0 1 0 1 0 1 1 1 0 1 1 1 0 0 1 1 0 0 0 0 1 0 1 1 0 0 0 1 0 1 1 1 0 1 0 1 0 0 1 1 0 0 1 0 0 1 1 1 0 0 0 0 0 0 1 0 0 1 1 1 1 0 1 1 0 0 1 1 0 0 1 1 0 0 0 0 0 0 1 0 0 1 0 1 0 0 0 1 0 0 0 1 1 0 1 1 0 1 0 0 1 1 1 1 0 1 1 0 0 1 1 1 0 1 0 0 1 0 1 1 0 1 0 1 0 1 1 1 0 1 0 1 1 0 1 1 0 0 0 1 1 0 1 0 0 0 1 0 1 0 0 0 0 0 1 1 0 0 0 1 0 1 0 0 1 0 1 1 0 0 1 0 0 1 1 1 0 1 0 1 0 0 1 1 0 1 0 1 1 0 1 0 0 1 0 0 1 0 1 1 0 0 1 1 1 0 1 1 0 1 1 0 0 1 1 1 0 1 0 0 1 0 1 1 0 0 1 0 0 1 1 1 0 1 0 1 0 0 1 1 0 0 0 1 0 0 1 1 0 0 0 0 0 0 1 0 0 1 1 1 0 1 1 1 0 1 0 1 0 0 1 1 0 0 0 0 0 0 1 0 0 0 0 1 0 1 1 1 0 0 1 0 0 1 1 1 0 1 0 1 0 0 1 1 0 1 0 0 0 0 1 1 0 0 1 0 1 1 0 1 1 1 1 1 0 1 0 1 0 1 0 0 1 1 0 0 1 1 0 1 1 0 1 1 1";
  uint32_t bytes = bitSequence.size () / 8;
  uint8_t *payload = new uint8_t[100];
  for (uint32_t i = 0; i < bytes; i++)
    {
      itpp::bvec extract = bitSequence ((i * 8), (i * 8) + 8 - 1);
      payload[i] = itpp::bin2dec ( extract, false );
    }
  Ptr<const Packet> packet = Create<Packet> ( (const uint8_t*) payload, 100);
  WifiMode mode = WifiPhy::GetOfdmRate36Mbps ();

  // Send a packet over the PHY
  phy->SendPacket (packet, mode, WIFI_PREAMBLE_LONG, 1);

  bool success;
  // Short training symbols
  success = CompareVectors (m_txSamples (0,160), m_refShortSymbol);
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: Short training symbols do not match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Short training symbols do match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "Short training symbols do not match with reference provided in IEEE 802.11 (2007) standard annex G");
  // Long training symbols
  success = CompareVectors (m_txSamples (160,320), m_refLongSymbol);
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: Long training symbols do not match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Long training symbols do match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "Long training symbols do not match with reference provided in IEEE 802.11 (2007) standard annex G");
  // Signal header
  success = CompareVectors (m_txSamples (320, 400), m_refSIGNAL);
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: Signal header does not match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: Signal header does match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "Signal header does not match with reference provided in IEEE 802.11 (2007) standard annex G");
  // 1. data symbol
  success = CompareVectors (m_txSamples (400, 479), m_refFirstDATAblock);
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: 1. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: 1. data symbol does match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "1. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
  // 2. data symbol
  success = CompareVectors (m_txSamples (480, 559), m_refSecondDATAblock);
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: 2. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: 2. data symbol does match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "2. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
  // 3. data symbol
  success = CompareVectors (m_txSamples (560, 639), m_refThirdDATAblock);
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: 3. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: 3. data symbol does match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "3. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
  // 4. data symbol
  success = CompareVectors (m_txSamples (640, 719), m_refFourthDATAblock);
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: 4. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: 4. data symbol does match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "4. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
  // 5. data symbol
  success = CompareVectors (m_txSamples (720, 799), m_refFifthDATAblock);
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: 5. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: 5. data symbol does match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "5. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
  // 6. data symbol
  success = CompareVectors (m_txSamples (800, 879), m_refSixthDATAblock(0, 79));
  if (!success)
    {
      NS_LOG_DEBUG ("FAIL: 6. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  else
    {
      NS_LOG_DEBUG ("PASS: 6. data symbol does match with reference provided in IEEE 802.11 (2007) standard annex G");
    }
  NS_TEST_EXPECT_MSG_EQ ( success, true, "6. data symbol does not match with reference provided in IEEE 802.11 (2007) standard annex G");
}

void
PhySimWifiFrameConstructionTest::PhyTxCallback (Ptr<const Packet> packet, Ptr<const PhySimWifiPhyTag> tag)
{
  m_txSamples = tag->GetTxedSamples ();
}

bool
PhySimWifiFrameConstructionTest::CompareVectors (const itpp::cvec &input, const itpp::cvec &reference)
{
  if (input.size () != reference.size ())
    {
      return false;
    }
  uint32_t refSize = std::min (input.size (), reference.size ());
  for (uint32_t i = 0; i < refSize; ++i)
    {
      if (input(i) != reference (i))
        {
          std::complex<double> diff = RoundComplex (input (i)) - reference (i);
          if ( std::abs (diff.real ()) > 0.001f || std::abs (diff.imag ()) > 0.001f )
            {
              return false;
            }
        }
    }
  return true;
}

std::complex<double>
PhySimWifiFrameConstructionTest::RoundComplex (const std::complex<double> input)
{
  return std::complex<double> (RoundIEEE (input.real ()), RoundIEEE (input.imag ()));
}

double
PhySimWifiFrameConstructionTest::RoundIEEE (const double num)
{
  return itpp::round (num * 1000.0) / 1000.0;
}
