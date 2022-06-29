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

#ifndef JOY2BPLUS_HXX
#define JOY2BPLUS_HXX

#include "Joystick.hxx"

/**
  The standard Joy2b+ controller works with the 2600 console for
  joystick directions and three buttons.  Button 'I' corresponds to
  the normal fire button (joy0fire), while button 'II' is read through
  INPT1 (analog pin 5) and button 'III' is read through INPT0 (analog pin
  9). This class is based upon the Genesis controller code.

  @author  Bruce-Robert Pocock
  @author  Stephen Anthony
*/
class Joy2BPlus : public Joystick
{
  public:
    /**
      Create a new Joy2BPlus gamepad plugged into the specified jack

      @param jack   The jack the controller is plugged into
      @param event  The event object to use for events
      @param system The system using this controller
    */
    Joy2BPlus(Jack jack, const Event& event, const System& system);
    ~Joy2BPlus() override = default;

  public:
    /**
      Returns the name of this controller.
    */
    string name() const override { return "Joy2b+"; }

  private:
    /**
      Update the button pin states.
    */
    void updateButtons() override;

  private:
    // Pre-compute the events we care about based on given port
    // This will eliminate test for left or right port in update()
    Event::Type myButtonIIEvent;
    Event::Type myButtonIIIEvent;

  private:
    // Following constructors and assignment operators not supported
    Joy2BPlus() = delete;
    Joy2BPlus(const Joy2BPlus&) = delete;
    Joy2BPlus(Joy2BPlus&&) = delete;
    Joy2BPlus& operator=(const Joy2BPlus&) = delete;
    Joy2BPlus& operator=(Joy2BPlus&&) = delete;
};

#endif
