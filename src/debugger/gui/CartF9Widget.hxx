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
// $Id: CartF9Widget.hxx brpocock $
//============================================================================

#ifndef CARTRIDGEF9_WIDGET_HXX
#define CARTRIDGEF9_WIDGET_HXX

class CartridgeF9;
class PopUpWidget;

#include "CartDebugWidget.hxx"

class CartridgeF9Widget : public CartDebugWidget
{
  public:
    CartridgeF9Widget(GuiObject* boss, const GUI::Font& lfont,
                      const GUI::Font& nfont,
                      int x, int y, int w, int h,
                      CartridgeF9& cart);
    virtual ~CartridgeF9Widget() = default;

  private:
    CartridgeF9& myCart;
    PopUpWidget* myBank;

    enum { kBankChanged = 'bkCH' };

  private:
    void loadConfig() override;
    void handleCommand(CommandSender* sender, int cmd, int data, int id) override;

    string bankState() override;

    // Following constructors and assignment operators not supported
    CartridgeF9Widget() = delete;
    CartridgeF9Widget(const CartridgeF9Widget&) = delete;
    CartridgeF9Widget(CartridgeF9Widget&&) = delete;
    CartridgeF9Widget& operator=(const CartridgeF9Widget&) = delete;
    CartridgeF9Widget& operator=(CartridgeF9Widget&&) = delete;
};

#endif
