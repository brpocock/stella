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

#include "Joy2BPlusWidget.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Joy2BPlusWidget::Joy2BPlusWidget(GuiObject* boss, const GUI::Font& font,
                             int x, int y, Controller& controller)
  : ControllerWidget(boss, font, x, y, controller)
{
  const string& label = getHeader();

  const int fontHeight = font.getFontHeight();
  int xpos = x, ypos = y, lwidth = font.getStringWidth("Right (Joy2BPlus)");

  const StaticTextWidget* t = new StaticTextWidget(boss, font, xpos, ypos+2, lwidth,
                                      fontHeight, label, TextAlign::Left);
  xpos += t->getWidth()/2 - 5;  ypos += t->getHeight() + 20;
  myPins[kJUp] = new CheckboxWidget(boss, font, xpos, ypos, "",
                                    CheckboxWidget::kCheckActionCmd);
  myPins[kJUp]->setID(kJUp);
  myPins[kJUp]->setTarget(this);

  ypos += myPins[kJUp]->getHeight() * 2 + 10;
  myPins[kJDown] = new CheckboxWidget(boss, font, xpos, ypos, "",
                                      CheckboxWidget::kCheckActionCmd);
  myPins[kJDown]->setID(kJDown);
  myPins[kJDown]->setTarget(this);

  xpos -= myPins[kJUp]->getWidth() + 5;
  ypos -= myPins[kJUp]->getHeight() + 5;
  myPins[kJLeft] = new CheckboxWidget(boss, font, xpos, ypos, "",
                                      CheckboxWidget::kCheckActionCmd);
  myPins[kJLeft]->setID(kJLeft);
  myPins[kJLeft]->setTarget(this);

  xpos += (myPins[kJUp]->getWidth() + 5) * 2;
  myPins[kJRight] = new CheckboxWidget(boss, font, xpos, ypos, "",
                                       CheckboxWidget::kCheckActionCmd);
  myPins[kJRight]->setID(kJRight);
  myPins[kJRight]->setTarget(this);

  xpos -= (myPins[kJUp]->getWidth() + 5) * 2;
  ypos = 30 + (myPins[kJUp]->getHeight() + 10) * 3;
  myPins[kJIbtn] = new CheckboxWidget(boss, font, xpos, ypos, "I",
                                      CheckboxWidget::kCheckActionCmd);
  myPins[kJIbtn]->setID(kJIbtn);
  myPins[kJIbtn]->setTarget(this);

  ypos += myPins[kJIbtn]->getHeight() + 5;
  myPins[kJIIbtn] = new CheckboxWidget(boss, font, xpos, ypos, "II",
                                      CheckboxWidget::kCheckActionCmd);
  myPins[kJIIbtn]->setID(kJIIbtn);
  myPins[kJIIbtn]->setTarget(this);

  ypos += myPins[kJIIbtn]->getHeight() + 5;
  myPins[kJIIIbtn] = new CheckboxWidget(boss, font, xpos, ypos, "III",
                                      CheckboxWidget::kCheckActionCmd);
  myPins[kJIIIbtn]->setID(kJIIIbtn);
  myPins[kJIIIbtn]->setTarget(this);

  addFocusWidget(myPins[kJUp]);
  addFocusWidget(myPins[kJLeft]);
  addFocusWidget(myPins[kJRight]);
  addFocusWidget(myPins[kJDown]);
  addFocusWidget(myPins[kJIbtn]);
  addFocusWidget(myPins[kJIIbtn]);
  addFocusWidget(myPins[kJIIIbtn]);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Joy2BPlusWidget::loadConfig()
{
  myPins[kJUp]->setState(!getPin(ourPinNo[kJUp]));
  myPins[kJDown]->setState(!getPin(ourPinNo[kJDown]));
  myPins[kJLeft]->setState(!getPin(ourPinNo[kJLeft]));
  myPins[kJRight]->setState(!getPin(ourPinNo[kJRight]));
  myPins[kJIbtn]->setState(!getPin(ourPinNo[kJIbtn]));

  myPins[kJIIbtn]->setState(
                            getPin(Controller::AnalogPin::Five) == AnalogReadout::connectToGround());
  myPins[kJIIIbtn]->setState(
                            getPin(Controller::AnalogPin::Nine) == AnalogReadout::connectToGround());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Joy2BPlusWidget::handleCommand(
    CommandSender* sender, int cmd, int data, int id)
{
  if(cmd == CheckboxWidget::kCheckActionCmd)
  {
    switch(id)
    {
      case kJUp:
      case kJDown:
      case kJLeft:
      case kJRight:
      case kJIbtn:
        setPin(ourPinNo[id], !myPins[id]->getState());
        break;
      case kJIIbtn:
        setPin(Controller::AnalogPin::Five,
             myPins[id]->getState() ? AnalogReadout::connectToGround() :
             AnalogReadout::connectToVcc());
        break;
      case kJIIIbtn:
        setPin(Controller::AnalogPin::Nine,
             myPins[id]->getState() ? AnalogReadout::connectToGround() :
             AnalogReadout::connectToVcc());
        break;
      default:
        break;
    }
  }
}
