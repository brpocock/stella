//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2022 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "Joy2BPlus.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Joy2BPlus::Joy2BPlus(Jack jack, const Event& event, const System& system)
  : Joystick(jack, event, system, Controller::Type::Joy2BPlus)
{
  if(myJack == Jack::Left) {
    myButtonIIEvent = Event::LeftJoystickFire5;
    myButtonIIIEvent = Event::LeftJoystickFire9;
  } else {
    myButtonIIEvent = Event::RightJoystickFire5;
    myButtonIIIEvent = Event::RightJoystickFire9;
  }

  setPin(AnalogPin::Five, AnalogReadout::connectToVcc());
  setPin(AnalogPin::Nine, AnalogReadout::connectToVcc());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Joy2BPlus::updateButtons()
{
  bool firePressed = myEvent.get(myFireEvent) != 0;
  // The Joy2b+ has three “fire” buttons, I (standard Fire),
  // II (same as Genesis (C) button), and III (unique).
  bool buttonIIPressed = myEvent.get(myButtonIIEvent) != 0;
  bool buttonIIIPressed = myEvent.get(myButtonIIIEvent) != 0;

  updateMouseButtons(firePressed, buttonIIPressed);

  setPin(DigitalPin::Six, !getAutoFireState(firePressed));
  setPin(AnalogPin::Five, buttonIIPressed ? AnalogReadout::connectToGround() : AnalogReadout::connectToVcc());
  setPin(AnalogPin::Nine, buttonIIIPressed ? AnalogReadout::connectToGround() : AnalogReadout::connectToVcc());
}
