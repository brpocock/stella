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
// Copyright (c) 1995-2016 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: CartF9Widget.cxx brpocock $
//============================================================================

#include "CartF9.hxx"
#include "PopUpWidget.hxx"
#include "CartF9Widget.hxx"

#include <sstream>


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeF9Widget::CartridgeF9Widget(
      GuiObject* boss, const GUI::Font& lfont, const GUI::Font& nfont,
      int x, int y, int w, int h, CartridgeF9& cart)
  : CartDebugWidget(boss, lfont, nfont, x, y, w, h),
    myCart(cart)
{
  uInt32 size = cart.bankCount() * 4096;

  ostringstream info;
  info << "Skyline F9 cartridge, " << cart.bankCount() << " × 4kiB banks\n"
       << "Startup bank = "
       << std::dec << cart.myStartBank
       << " ($" << std::hex << cart.myStartBank << ")\n"
       << "Hotspot latch $ff9\n";

  int xpos = 10,
      ypos = addBaseInformation(size, "Skyline $f9", info.str(), 15) + myLineHeight;

  VariantList items;
  for (int i = 0; i < cart.bankCount(); ++i) {
    VarList::push_back(items, static_cast< std::ostringstream & >
                       ((std::ostringstream() <<
                         std::dec << i << " ($" << std::hex << i << ")") ).str());
  }
  myBank =
    new PopUpWidget(boss, _font, xpos, ypos-2, _font.getStringWidth("1024 ($FF) "),
                    myLineHeight, items, "Set bank: ",
                    _font.getStringWidth("Set bank: "), kBankChanged);
  myBank->setTarget(this);
  addFocusWidget(myBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF9Widget::loadConfig()
{
  myBank->setSelectedIndex(myCart.myCurrentBank);

  CartDebugWidget::loadConfig();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF9Widget::handleCommand(CommandSender* sender,
                                      int cmd, int data, int id)
{
  if(cmd == kBankChanged)
  {
    myCart.unlockBank();
    myCart.bank(myBank->getSelected());
    myCart.lockBank();
    invalidate();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string CartridgeF9Widget::bankState()
{
  ostringstream& buf = buffer();

  buf << "Bank = " << std::dec << myCart.myCurrentBank
      << " ($" 
      << std::hex << myCart.myCurrentBank
      << ")";

  return buf.str();
}
