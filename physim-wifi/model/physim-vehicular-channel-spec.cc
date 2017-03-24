/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Stylianos Papanastasiou, Jens Mittag
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
 * Stylianos Papanastasiou <stylianos@gmail.com>
 * Jens Mittag <jens.mittag@kit.edu>
 *
 */

#include "physim-vehicular-channel-spec.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"

NS_LOG_COMPONENT_DEFINE ("Vehicular_Channel_Specification");

namespace ns3 {

/* Vehicular Channel Tap */

Vehicular_Channel_Tap::Vehicular_Channel_Tap ()
{
}

Vehicular_Channel_Tap::Vehicular_Channel_Tap (itpp::vec rpathloss, itpp::vec Rk, itpp::ivec fShift,
											  itpp::ivec fDoppler, itpp::ivec lDoppler,
											  itpp::Array<enum VEHICULAR_DOPPLER_SPECTRUM> vshapes)
  : relative_path_loss (rpathloss),
    ricianK (Rk),
    freqShift (fShift),
    fadingDoppler (fDoppler),
    LOSDoppler (lDoppler),
    shapes (vshapes)
{
}

int
Vehicular_Channel_Tap::get_no_paths () const
{
	return relative_path_loss.length ();
}

itpp::vec
Vehicular_Channel_Tap::get_relative_path_loss () const
{
	return relative_path_loss;
}

itpp::vec
Vehicular_Channel_Tap::get_rician_K () const
{
	return ricianK;
}

itpp::ivec
Vehicular_Channel_Tap::get_freq_Shift () const
{
	return freqShift;
}

itpp::ivec
Vehicular_Channel_Tap::get_fading_doppler () const
{
	return fadingDoppler;
}

itpp::ivec
Vehicular_Channel_Tap::get_LOS_Doppler () const
{
	return LOSDoppler;
}

itpp::Array<enum VEHICULAR_DOPPLER_SPECTRUM>
Vehicular_Channel_Tap::get_Doppler_Shapes () const
{
	return shapes;
}
/* Vehicular Channel Specification */

NS_OBJECT_ENSURE_REGISTERED (Vehicular_Channel_Specification);

TypeId
Vehicular_Channel_Specification::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Vehicular_Channel_Specification")
		  .SetParent<Object> ()
		  .AddConstructor<Vehicular_Channel_Specification> ();
  return tid;
}

Vehicular_Channel_Specification::Vehicular_Channel_Specification ()
{
}
Vehicular_Channel_Specification::Vehicular_Channel_Specification (
  const enum VEHICULAR_CHANNEL_PROFILE prof)
{
  set_channel_profile (prof);
}
void
Vehicular_Channel_Specification::set_channel_profile (const enum VEHICULAR_CHANNEL_PROFILE prof)
{
  profile = prof;
  itpp::Array<enum VEHICULAR_DOPPLER_SPECTRUM> vshapes;
  taps.set_size(0);
  switch (profile)
    {
    case V2V_EXPRESSWAY_ONCOMING:
      set_tap_power_delay (itpp::vec ("0 -6.3 -25.1 -22.7"), itpp::vec (
                             "0 100 200 300") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (4, false);
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      vshapes (2) = ROUND;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("-0.03 -24.93 -25.53"),
                                        itpp::vec ("-1.6 0 0"), itpp::ivec ("1451 884 1005"), itpp::ivec (
                                          "60 858 486"), itpp::ivec ("1452 0 0"), vshapes);
      // Tap two
      vshapes.set_size (2, false);
      vshapes (0) = Classic3dB;
      vshapes (1) = ROUND;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-12.96 -7.36"), itpp::vec (
                                          "0 0"), itpp::ivec ("761 1445"), itpp::ivec ("655 56"), itpp::ivec (
                                          "0 0"), vshapes);
      // Tap three
      vshapes.set_size (3, false);
      vshapes (0) = Classic3dB;
      vshapes (1) = FLAT;
      vshapes (2) = ROUND;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-28.38 -28.78 -35.08"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("819 1466 124"), itpp::ivec (
                                          "823 75 99"), itpp::ivec ("0 0 0"), vshapes);
      // Tap four
      vshapes.set_size (3, false);
      vshapes (0) = FLAT;
      vshapes (1) = Classic3dB;
      vshapes (2) = Classic6dB;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-25.28 -33.98 -26.98"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("1437 552 868"), itpp::ivec (
                                          "110 639 858"), itpp::ivec ("0 0 0"), vshapes);
      relSpeed = 280 / 3.6;

      break;
    case V2V_EXPRESSWAY_ONCOMING_OR:
      set_tap_power_delay (itpp::vec ("0 -6.3 -25.1 -22.7"), itpp::vec (
                             "0 100 200 300") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (4, false);
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      vshapes (2) = ROUND;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("0.0 -24.9 -25.5"), itpp::vec (
                                          "-1.6 0 0"), itpp::ivec ("1451 884 1005"), itpp::ivec ("60 858 486"),
                                        itpp::ivec ("1452 0 0"), vshapes);
      // Tap two
      vshapes.set_size (2, false);
      vshapes (0) = Classic3dB;
      vshapes (1) = ROUND;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-13.1 -7.5"),
                                        itpp::vec ("0 0"), itpp::ivec ("761 1445"), itpp::ivec ("655 56"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap three
      vshapes.set_size (3, false);
      vshapes (0) = Classic3dB;
      vshapes (1) = FLAT;
      vshapes (2) = ROUND;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-28.9 -29.3 -35.6"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("819 1466 124"), itpp::ivec (
                                          "823 75 99"), itpp::ivec ("0 0 0"), vshapes);
      // Tap four
      vshapes.set_size (3, false);
      vshapes (0) = FLAT;
      vshapes (1) = Classic3dB;
      vshapes (2) = Classic6dB;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-25.7 -34.4 -27.4"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("1437 552 868"), itpp::ivec (
                                          "110 639 858"), itpp::ivec ("0 0 0"), vshapes);
      relSpeed = 280 / 3.6;

      break;
    case RTV_URBAN_CANYON:
      set_tap_power_delay (itpp::vec ("0 -11.5 -19.0 -25.6 -28.0"), itpp::vec (
                             "0 100 200 300 500") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (5, false);
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = Classic3dB;
      vshapes (2) = Classic3dB;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("-0.03 -28.73 -23.33"),
                                        itpp::vec ("7.5 0 0"), itpp::ivec ("574 -97 -89"), itpp::ivec (
                                          "165 543 478"), itpp::ivec ("654 0 0"), vshapes);
      // Tap two
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      vshapes (2) = Classic6dB;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-23.39 -13.99 -15.79"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("-549 559 115"), itpp::ivec (
                                          "174 196 757"), itpp::ivec ("0 0 0"), vshapes);
      // Tap three
      vshapes.set_size (2, false);
      vshapes (0) = FLAT;
      vshapes (1) = ROUND;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-21.01 -23.31"), itpp::vec (
                                          "0 0"), itpp::ivec ("610 72"), itpp::ivec ("258 929"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap four
      vshapes.set_size (2, false);
      vshapes (0) = FLAT;
      vshapes (1) = Classic3dB;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-30.17 -27.74"), itpp::vec (
                                          "0 0"), itpp::ivec ("183 103"), itpp::ivec ("653 994"), itpp::ivec (
                                          "0 0"), vshapes);
      // Tap five
      vshapes.set_size (2, false);
      vshapes (0) = FLAT;
      vshapes (1) = FLAT;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-28.1 -30.3"),
                                        itpp::vec ("0 0"), itpp::ivec ("720 -20"), itpp::ivec ("220 871"),
                                        itpp::ivec ("0 0"), vshapes);
      relSpeed = 240 / 3.6;

      break;
    case RTV_URBAN_CANYON_OR:
      set_tap_power_delay (itpp::vec ("0 -11.5 -19.0 -25.6 -28.0"), itpp::vec (
                             "0 100 200 300 500") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (5, false);
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = Classic3dB;
      vshapes (2) = Classic3dB;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("-1.8 -30.5 -25.1"), itpp::vec (
                                          "7.5 0 0"), itpp::ivec ("574 -97 -89"), itpp::ivec ("165 543 478"),
                                        itpp::ivec ("654 0 0"), vshapes);
      // Tap two
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      vshapes (2) = Classic6dB;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-27.1 -17.7 -19.5"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("-549 559 115"), itpp::ivec (
                                          "174 196 757"), itpp::ivec ("0 0 0"), vshapes);
      // Tap three
      vshapes.set_size (2, false);
      vshapes (0) = FLAT;
      vshapes (1) = ROUND;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-17.6 -19.9"),
                                        itpp::vec ("0 0"), itpp::ivec ("610 72"), itpp::ivec ("258 929"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap four
      vshapes.set_size (2, false);
      vshapes (0) = FLAT;
      vshapes (1) = Classic3dB;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-23.3 -20.6"),
                                        itpp::vec ("0 0"), itpp::ivec ("183 103"), itpp::ivec ("653 994"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap five
      vshapes.set_size (2, false);
      vshapes (0) = FLAT;
      vshapes (1) = FLAT;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-29.8 -28.0"),
                                        itpp::vec ("0 0"), itpp::ivec ("720 -20"), itpp::ivec ("220 871"),
                                        itpp::ivec ("0 0"), vshapes);

      relSpeed = 240 / 3.6;
      break;
    case RTV_EXPRESSWAY:
      set_tap_power_delay (itpp::vec ("0 -9.3 -20.3 -21.3 -28.8"), itpp::vec (
                             "0 100 200 300 400") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (5, false);
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      vshapes (2) = ROUND;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("-0.01 -36.41 -30.01"),
                                        itpp::vec ("-5.3 0 0"), itpp::ivec ("769 -22 535"), itpp::ivec (
                                          "70 600 376"), itpp::ivec ("770 0 0"), vshapes);
      // Tap two
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      vshapes (2) = FLAT;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-9.98 -19.38 -22.58"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("754 548 -134"), itpp::ivec (
                                          "117 424 530"), itpp::ivec ("0 0 0"), vshapes);
      // Tap three
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = Classic3dB;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-22.8 -23.9"),
                                        itpp::vec ("0 0"), itpp::ivec ("761 88"), itpp::ivec ("104 813"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap four
      vshapes.set_size (2, false);
      vshapes (0) = Classic6dB;
      vshapes (1) = ROUND;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-21.3 -76.6"),
                                        itpp::vec ("0 0"), itpp::ivec ("37 752"), itpp::ivec ("802 91"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap five
      vshapes.set_size (2, false);
      vshapes (0) = Classic6dB;
      vshapes (1) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-28.8 -39.76"), itpp::vec (
                                          "0 0"), itpp::ivec ("16 -755"), itpp::ivec ("807 329"), itpp::ivec (
                                          "0 0"), vshapes);

      relSpeed = 280 / 3.6;
      break;
    case RTV_EXPRESSWAY_OR:
      set_tap_power_delay (itpp::vec ("0 -9.3 -20.3 -21.3 -28.8"), itpp::vec (
                             "0 100 200 300 400") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (5, false);
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      vshapes (2) = ROUND;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("0 -36.4 -30.0"), itpp::vec (
                                          "-5.3 0 0"), itpp::ivec ("769 -22 535"), itpp::ivec ("70 600 376"),
                                        itpp::ivec ("770 0 0"), vshapes);
      // Tap two
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      vshapes (2) = FLAT;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-12.3 -21.7 -24.9"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("754 548 -134"), itpp::ivec (
                                          "117 424 530 "), itpp::ivec ("0 0 0"), vshapes);
      // Tap three
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = Classic3dB;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-24.3 -25.4"),
                                        itpp::vec ("0 0"), itpp::ivec ("761 88"), itpp::ivec ("104 813"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap four
      vshapes.set_size (2, false);
      vshapes (0) = Classic6dB;
      vshapes (1) = ROUND;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-26.8 -28.5"),
                                        itpp::vec ("0 0"), itpp::ivec ("37 752"), itpp::ivec ("802 91"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap five
      vshapes.set_size (2, false);
      vshapes (0) = Classic6dB;
      vshapes (1) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-31.2 -41.8"),
                                        itpp::vec ("0 0"), itpp::ivec ("16 -755"), itpp::ivec ("807 329"),
                                        itpp::ivec ("0 0"), vshapes);
      relSpeed = 280 / 3.6;

      break;
    case V2V_URBAN_CANYON_ONCOMING:
      set_tap_power_delay (itpp::vec ("0 -10.0 -17.8 -21.1 -26.3"), itpp::vec (
                             "0 100 200 300 400") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (5, false);
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("0.07 -17.67"), itpp::vec (
                                          "4.0 0"), itpp::ivec ("1145 833"), itpp::ivec ("284 824"), itpp::ivec (
                                          "1263 0 0"), vshapes);
      // Tap two
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = Classic6dB;
      vshapes (2) = FLAT;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-10.97 -17.07 -34.47"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("707 918 -250"), itpp::ivec (
                                          "871 286 936"), itpp::ivec ("0 0 0"), vshapes);
      // Tap three
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = FLAT;
      vshapes (2) = ROUND;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-23.98 -19.38 -29.78"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("21 677 -188"), itpp::ivec (
                                          "166 726 538"), itpp::ivec ("0 0 0"), vshapes);
      // Tap four
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-24.06 -24.16"), itpp::vec (
                                          "0 0"), itpp::ivec ("538 41"), itpp::ivec ("908 183"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap five
      vshapes.set_size (2, false);
      vshapes (0) = Classic6dB;
      vshapes (1) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-26.3 -34.09"), itpp::vec (
                                          "0 0"), itpp::ivec ("674 -78"), itpp::ivec ("723 260"), itpp::ivec (
                                          "0 0"), vshapes);
      relSpeed = 240 / 3.6;

      break;
    case V2V_URBAN_CANYON_ONCOMING_OR:
      set_tap_power_delay (itpp::vec ("0 -10.0 -17.8 -21.1 -26.3"), itpp::vec (
                             "0 100 200 300 400") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (5, false);
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("0.0 -17.6"),
                                        itpp::vec ("4.0 0"), itpp::ivec ("1145 833"), itpp::ivec ("284 824"),
                                        itpp::ivec ("1263 0 0"), vshapes);
      // Tap two
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = Classic6dB;
      vshapes (2) = FLAT;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-12.9 -19.0 -36.4"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("707 918 -250"), itpp::ivec (
                                          "871 286 936"), itpp::ivec ("0 0 0"), vshapes);
      // Tap three
      vshapes.set_size (3, false);
      vshapes (0) = ROUND;
      vshapes (1) = FLAT;
      vshapes (2) = ROUND;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-25.8 -21.2 -31.6"),
                                        itpp::vec ("0 0 0"), itpp::ivec ("21 677 -188"), itpp::ivec (
                                          "166 726 538"), itpp::ivec ("0 0 0"), vshapes);
      // Tap four
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-28.2 -28.3"),
                                        itpp::vec ("0 0"), itpp::ivec ("538 41"), itpp::ivec ("908 183"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap five
      vshapes.set_size (2, false);
      vshapes (0) = Classic6dB;
      vshapes (1) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-28.5 -35.5"),
                                        itpp::vec ("0 0"), itpp::ivec ("674 -78"), itpp::ivec ("723 260"),
                                        itpp::ivec ("0 0"), vshapes);
      relSpeed = 240 / 3.6;

      break;
    case RTV_SUBURBAN_STREET:
      set_tap_power_delay (itpp::vec (
                             "0 -9.3 -14.0 -18.0 -19.4 -24.9 -27.5 -29.8"), itpp::vec (
                             "0 100 200 300 400 500 600 700") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (8, false);
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("-0.03 -21.53"), itpp::vec (
                                          "3.3 0"), itpp::ivec ("648 171"), itpp::ivec ("152 823"), itpp::ivec (
                                          "635 0 0"), vshapes);
      // Tap two
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = Classic3dB;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-10.09 -17.09"), itpp::vec (
                                          "0 0"), itpp::ivec ("582 -119"), itpp::ivec ("249 515"), itpp::ivec (
                                          "0 0"), vshapes);
      // Tap three
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = FLAT;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-16.01 -18.31"), itpp::vec (
                                          "0 0"), itpp::ivec ("527 62"), itpp::ivec ("223 802"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap four
      vshapes.set_size (2, false);
      vshapes (0) = Classic6dB;
      vshapes (1) = ROUND;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-22.25 -20.05"), itpp::vec (
                                          "0 0"), itpp::ivec ("497 87"), itpp::ivec ("396 851"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap five
      vshapes.set_size (1, false);
      vshapes (0) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-19.4"), itpp::vec ("0"),
                                        itpp::ivec ("43"), itpp::ivec ("747"), itpp::ivec ("0"), vshapes);
      // Tap six
      vshapes.set_size (1, false);
      vshapes (0) = Classic6dB;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-24.9"), itpp::vec ("0"),
                                        itpp::ivec ("114"), itpp::ivec ("742"), itpp::ivec ("0"), vshapes);
      // Tap seven
      vshapes.set_size (1, false);
      vshapes (0) = Classic3dB;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-27.5"), itpp::vec ("0"),
                                        itpp::ivec ("38"), itpp::ivec ("746"), itpp::ivec ("0"), vshapes);
      // Tap eight
      vshapes.set_size (1, false);
      vshapes (0) = Classic3dB;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-29.8"), itpp::vec ("0"),
                                        itpp::ivec ("8"), itpp::ivec ("743"), itpp::ivec ("0"), vshapes);
      relSpeed = 240 / 3.6;

      break;
    case RTV_SUBURBAN_STREET_OR:
      set_tap_power_delay (itpp::vec (
                             "0 -9.3 -14.0 -18.0 -19.4 -24.9 -27.5 -29.8"), itpp::vec (
                             "0 100 200 300 400 500 600 700") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (8, false);
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("0.0 -21.5"),
                                        itpp::vec ("3.3 0"), itpp::ivec ("648 171"), itpp::ivec ("152 823"),
                                        itpp::ivec ("635 0 0"), vshapes);
      // Tap two
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = Classic3dB;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-11.8 -18.8"),
                                        itpp::vec ("0 0"), itpp::ivec ("582 -119"), itpp::ivec ("249 515"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap three
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = FLAT;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-17.6 -19.9"),
                                        itpp::vec ("0 0"), itpp::ivec ("527 62"), itpp::ivec ("223 802"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap four
      vshapes.set_size (2, false);
      vshapes (0) = Classic6dB;
      vshapes (1) = ROUND;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-23.0 -20.8"),
                                        itpp::vec ("0 0"), itpp::ivec ("497 87"), itpp::ivec ("396 851"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap five
      vshapes.set_size (1, false);
      vshapes (0) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-19.4"), itpp::vec ("0"),
                                        itpp::ivec ("43"), itpp::ivec ("747"), itpp::ivec ("0"), vshapes);
      // Tap six
      vshapes.set_size (1, false);
      vshapes (0) = Classic6dB;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-24.9"), itpp::vec ("0"),
                                        itpp::ivec ("114"), itpp::ivec ("742"), itpp::ivec ("0"), vshapes);
      // Tap seven
      vshapes.set_size (1, false);
      vshapes (0) = Classic3dB;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-27.5"), itpp::vec ("0"),
                                        itpp::ivec ("38"), itpp::ivec ("746"), itpp::ivec ("0"), vshapes);
      // Tap eight
      vshapes.set_size (1, false);
      vshapes (0) = Classic3dB;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-29.8"), itpp::vec ("0"),
                                        itpp::ivec ("8"), itpp::ivec ("743"), itpp::ivec ("0"), vshapes);
      relSpeed = 240 / 3.6;

      break;
    case V2V_EXPRESS_SAME_DIREC_WITH_WALL:
      set_tap_power_delay (itpp::vec (
                             "0 -11.2 -19.0 -21.9 -25.3 -24.4 -28.0 -26.1"), itpp::vec (
                             "0 100 200 300 400 500 600 700") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (8, false);
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("-1.4 -5.6"), itpp::vec (
                                          "23.8 0"), itpp::ivec ("-55 -20"), itpp::ivec ("1407 84"), itpp::ivec (
                                          "-60 0"), vshapes);
      // Tap two
      vshapes.set_size (2, false);
      vshapes (0) = Classic3dB;
      vshapes (1) = ROUND;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-14.21 -14.21"), itpp::vec (
                                          "5.7 0"), itpp::ivec ("-56.0 0"), itpp::ivec ("1345 70"), itpp::ivec (
                                          "40 0"), vshapes);
      // Tap three
      vshapes.set_size (1, false);
      vshapes (0) = Classic6dB;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-19"), itpp::vec ("0"),
                                        itpp::ivec ("-87"), itpp::ivec ("358"), itpp::ivec ("0"), vshapes);
      // Tap four
      vshapes.set_size (1, false);
      vshapes (0) = Classic3dB;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-21.9"), itpp::vec ("0"),
                                        itpp::ivec ("-139"), itpp::ivec ("1397"), itpp::ivec ("0"), vshapes);
      // Tap five
      vshapes.set_size (2, false);
      vshapes (0) = Classic6dB;
      vshapes (0) = Classic3dB;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-27.1 -30.0"),
                                        itpp::vec ("0 0"), itpp::ivec ("60 -561"), itpp::ivec ("522 997"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap six
      vshapes.set_size (1, false);
      vshapes (0) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-24.4"), itpp::vec ("0"),
                                        itpp::ivec ("50"), itpp::ivec ("529"), itpp::ivec ("0"), vshapes);
      // Tap seven
      vshapes.set_size (1, false);
      vshapes (0) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-28"), itpp::vec ("0"),
                                        itpp::ivec ("13"), itpp::ivec ("1572"), itpp::ivec ("0"), vshapes);
      // Tap eight
      vshapes.set_size (2, false);
      vshapes (0) = Classic3dB;
      vshapes (0) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-31.5 -29.73"), itpp::vec (
                                          "0 0"), itpp::ivec ("-6 4"), itpp::ivec ("1562 81"), itpp::ivec ("0 0"),
                                        vshapes);
      relSpeed = 280 / 3.6;

      break;
    case V2V_EXPRESS_SAME_DIREC_WITH_WALL_OR:
      set_tap_power_delay (itpp::vec (
                             "0 -11.2 -19.0 -21.9 -25.3 -24.4 -28.0 -26.1"), itpp::vec (
                             "0 100 200 300 400 500 600 700") * 1e-9);
      // Tap one
      NS_ASSERT (taps.length () == 0);
      taps.set_size (8, false);
      vshapes.set_size (2, false);
      vshapes (0) = ROUND;
      vshapes (1) = ROUND;
      taps (0) = Vehicular_Channel_Tap (itpp::vec ("-1.4 -5.6"), itpp::vec (
                                          "23.8 0"), itpp::ivec ("-55 -20"), itpp::ivec ("1407 84"), itpp::ivec (
                                          "-60 0"), vshapes);
      // Tap two
      vshapes.set_size (2, false);
      vshapes (0) = Classic3dB;
      vshapes (1) = ROUND;
      taps (1) = Vehicular_Channel_Tap (itpp::vec ("-14.2 -14.2"), itpp::vec (
                                          "5.7 0"), itpp::ivec ("-56.0 0"), itpp::ivec ("1345 70"), itpp::ivec (
                                          "40 0"), vshapes);
      // Tap three
      vshapes.set_size (1, false);
      vshapes (0) = Classic6dB;
      taps (2) = Vehicular_Channel_Tap (itpp::vec ("-19"), itpp::vec ("0"),
                                        itpp::ivec ("-87"), itpp::ivec ("358"), itpp::ivec ("0"), vshapes);
      // Tap four
      vshapes.set_size (1, false);
      vshapes (0) = Classic3dB;
      taps (3) = Vehicular_Channel_Tap (itpp::vec ("-21.9"), itpp::vec ("0"),
                                        itpp::ivec ("-139"), itpp::ivec ("1397"), itpp::ivec ("0"), vshapes);
      // Tap five
      vshapes.set_size (2, false);
      vshapes (0) = Classic6dB;
      vshapes (0) = Classic3dB;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-27.9 -30.8"),
                                        itpp::vec ("0 0"), itpp::ivec ("60 -561"), itpp::ivec ("522 997"),
                                        itpp::ivec ("0 0"), vshapes);
      // Tap six
      vshapes.set_size (1, false);
      vshapes (0) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-24.4"), itpp::vec ("0"),
                                        itpp::ivec ("50"), itpp::ivec ("529"), itpp::ivec ("0"), vshapes);
      // Tap seven
      vshapes.set_size (1, false);
      vshapes (0) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-28"), itpp::vec ("0"),
                                        itpp::ivec ("13"), itpp::ivec ("1572"), itpp::ivec ("0"), vshapes);
      // Tap eight
      vshapes.set_size (2, false);
      vshapes (0) = Classic3dB;
      vshapes (0) = ROUND;
      taps (4) = Vehicular_Channel_Tap (itpp::vec ("-31.5 -28.1"),
                                        itpp::vec ("0 0"), itpp::ivec ("-6 4"), itpp::ivec ("1562 81"),
                                        itpp::ivec ("0 0"), vshapes);
      relSpeed = 280 / 3.6;

      break;
    default:
      NS_FATAL_ERROR ("Unknown channel profile!");
    }
}
void
Vehicular_Channel_Specification::set_tap_power_delay (
  const itpp::vec &avg_power_dB, const itpp::vec &delay_prof)
{
  NS_ASSERT (min (delay_prof) == 0); // Minimum relative delay must be 0
  NS_ASSERT (avg_power_dB.size () == delay_prof.size ()); // Power and delay vectors must be of equal length;
  NS_ASSERT (delay_prof (0) == 0); // First tap must be at zero delay");
  tap_power = avg_power_dB;
  tap_delay = delay_prof;
}
std::string
Vehicular_Channel_Specification::get_channel_profile_name ( enum VEHICULAR_CHANNEL_PROFILE profile)
{
  switch (profile)
    {
    case V2V_EXPRESSWAY_ONCOMING:
      return std::string ("V2V-Expressway Oncoming");
      break;
    case RTV_URBAN_CANYON:
      return std::string ("RTV-Urban Canyon");
      break;
    case RTV_EXPRESSWAY:
      return std::string ("RTV-Expressway");
      break;
    case V2V_URBAN_CANYON_ONCOMING:
      return std::string ("V2V-Urban Canyon Oncoming");
      break;
    case RTV_SUBURBAN_STREET:
      return std::string ("RTV-Suburban Street");
      break;
    case V2V_EXPRESS_SAME_DIREC_WITH_WALL:
      return std::string ("V2V-Express Same Direction With Wall");
      break;
    default:
      return std::string ("Unknown Vehicular Channel Model");
    }
}

} // namespace ns3
