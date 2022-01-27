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

#ifndef CARTRIDGEF9_HXX
#define CARTRIDGEF9_HXX

class System;

#include "bspf.hxx"
#include "CartEnhanced.hxx"
#ifdef DEBUGGER_SUPPORT
  #include "CartF9Widget.hxx"
#endif

/**
  Cartridge class used for Homestar Runner by Paul Slocum.
  There are 16 4K banks (total of 64K ROM).
  Accessing $1FE0 - $1FF9 switches to each bank.

  @author  Stephen Anthony, Thomas Jentzsch
*/
class CartridgeF9 : public CartridgeEnhanced
{
  friend class CartridgeF9Widget;

  public:
    /**
      Create a new cartridge using the specified image

      @param image     Pointer to the ROM image
      @param size      The size of the ROM image
      @param md5       The md5sum of the ROM image
      @param settings  A reference to the various settings (read-only)
      @param bsSize    The size specified by the bankswitching scheme
    */
    CartridgeF9(const ByteBuffer& image, size_t size, const string& md5,
                const Settings& settings, size_t bsSize = 64_KB);
    ~CartridgeF9() override = default;

  public:
    /**
      Get a descriptor for the device name (used in error checking).

      @return The name of the object
    */
    string name() const override { return "CartridgeF9"; }

  #ifdef DEBUGGER_SUPPORT
    /**
      Get debugger widget responsible for accessing the inner workings
      of the cart.
    */
    CartDebugWidget* debugWidget(GuiObject* boss, const GUI::Font& lfont,
        const GUI::Font& nfont, int x, int y, int w, int h) override
    {
      return new CartridgeF9Widget(boss, lfont, nfont, x, y, w, h, *this);
    }
  #endif

  private:
    bool checkSwitchBank(uInt16 address, uInt8 value = 0) override;

    uInt16 hotspot() const override { return 0x1FF9; }

    uInt16 getStartBank() const override { return 0; }

  private:
    // Following constructors and assignment operators not supported
    CartridgeF9() = delete;
    CartridgeF9(const CartridgeF9&) = delete;
    CartridgeF9(CartridgeF9&&) = delete;
    CartridgeF9& operator=(const CartridgeF9&) = delete;
    CartridgeF9& operator=(CartridgeF9&&) = delete;
};

#endif
