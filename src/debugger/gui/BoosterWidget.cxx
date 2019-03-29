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
// Copyright (c) 1995-2019 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include "BoosterWidget.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BoosterWidget::BoosterWidget(GuiObject* boss, const GUI::Font& font,
                             int x, int y, Controller& controller)
  : ControllerWidget(boss, font, x, y, controller)
{
  const string& label = isLeftPort() ? "Left (Booster)" : "Right (Booster)";

  const int fontHeight = font.getFontHeight();
  int xpos = x, ypos = y, lwidth = font.getStringWidth("Right (Booster)");
  StaticTextWidget* t;

  t = new StaticTextWidget(boss, font, xpos, ypos+2, lwidth,
                           fontHeight, label, TextAlign::Left);
  xpos += t->getWidth()/2 - 5;  ypos += t->getHeight() + 10;
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
  ypos = 20 + (myPins[kJUp]->getHeight() + 10) * 3;
  myPins[kJFire] = new CheckboxWidget(boss, font, xpos, ypos, "Fire",
                                      CheckboxWidget::kCheckActionCmd);
  myPins[kJFire]->setID(kJFire);
  myPins[kJFire]->setTarget(this);

  ypos += myPins[kJFire]->getHeight() + 5;
  myPins[kJBooster] = new CheckboxWidget(boss, font, xpos, ypos, "Booster",
                                         CheckboxWidget::kCheckActionCmd);
  myPins[kJBooster]->setID(kJBooster);
  myPins[kJBooster]->setTarget(this);

  ypos += myPins[kJBooster]->getHeight() + 5;
  myPins[kJTrigger] = new CheckboxWidget(boss, font, xpos, ypos, "Trigger",
                                         CheckboxWidget::kCheckActionCmd);
  myPins[kJTrigger]->setID(kJTrigger);
  myPins[kJTrigger]->setTarget(this);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BoosterWidget::loadConfig()
{
  myPins[kJUp]->setState(!myController->getPin(ourPinNo[kJUp]));
  myPins[kJDown]->setState(!myController->getPin(ourPinNo[kJDown]));
  myPins[kJLeft]->setState(!myController->getPin(ourPinNo[kJLeft]));
  myPins[kJRight]->setState(!myController->getPin(ourPinNo[kJRight]));
  myPins[kJFire]->setState(!myController->getPin(ourPinNo[kJFire]));

  myPins[kJBooster]->setState(
    myController->getPin(Controller::AnalogPin::Five) == Controller::MIN_RESISTANCE);
  myPins[kJTrigger]->setState(
    myController->getPin(Controller::AnalogPin::Nine) == Controller::MIN_RESISTANCE);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BoosterWidget::handleCommand(
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
      case kJFire:
        myController->setPin(ourPinNo[id], !myPins[id]->getState());
        break;
      case kJBooster:
        myController->setPin(Controller::AnalogPin::Five,
          myPins[id]->getState() ? Controller::MIN_RESISTANCE :
                                   Controller::MAX_RESISTANCE);
        break;
      case kJTrigger:
        myController->setPin(Controller::AnalogPin::Nine,
          myPins[id]->getState() ? Controller::MIN_RESISTANCE :
                                   Controller::MAX_RESISTANCE);
        break;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Controller::DigitalPin BoosterWidget::ourPinNo[5] = {
  Controller::DigitalPin::One, Controller::DigitalPin::Two, Controller::DigitalPin::Three, Controller::DigitalPin::Four,
  Controller::DigitalPin::Six
};
