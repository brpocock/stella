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

#ifndef CARTRIDGEEFF_HXX
#define CARTRIDGEEFF_HXX

class System;

#include "MT24LC256.hxx"
#include "bspf.hxx"
#include "CartEnhanced.hxx"
#ifdef DEBUGGER_SUPPORT
#include "CartEFFWidget.hxx"
#endif

/**
   Based on EF, the EFF cartridge  adds strobe addresses for i2c access to
   an EEPROM, like an on_cartridge SaveKey interface.

   This class is derived from but does not inherit from CartridgeEF.

   @author Stephen Anthony, Thomas Jentzsch, Bruce_Robert Pocock
*/
class CartridgeEFF : public CartridgeEnhanced
{
  friend class CartridgeEFFWidget;

public:
  /**
     Create a new cartridge using the specified image

     @param image     Pointer to the ROM image
     @param size      The size of the ROM image
     @param md5       The md5sum of the ROM image
     @param settings  A reference to the various settings (read-only)
     @param bsSize    The size specified by the bankswitching scheme
  */
  CartridgeEFF(const ByteBuffer& image, size_t size, const string& md5,
               const Settings& settings, size_t bsSize = 64_KB);
  ~CartridgeEFF() override = default;

public:
  /**
     Get a descriptor for the device name (used in error checking).

     @return The name of the object
  */
  string name() const override { return "CartridgeEFF"; }

#ifdef DEBUGGER_SUPPORT
  /**
     Get debugger widget responsible for accessing the inner workings
     of the cart.
  */
  CartDebugWidget* debugWidget(GuiObject* boss, const GUI::Font& lfont,
                               const GUI::Font& nfont, int x, int y, int w, int h) override
  {
    return new CartridgeEFFWidget(boss, lfont, nfont, x, y, w, h, *this);
  }
#endif

  int descriptionLines();

  string ramDescription();

  uInt8 peek(uInt16 address) override;
  bool poke(uInt16 address, uInt8 value) override;

  void  setNVRamFile(const  string&  nvramdir,  const  string&  romfile)
  override;

  void install(System& system);

private:
  bool checkSwitchBank(uInt16 address, uInt8) override;

  uInt16 hotspot() const override { return 0x1FE0; }

  uInt16 getStartBank() const override { return 1; }

private:
  // RAM size
  static constexpr uInt32 RAM_SIZE = 2048;

  void setI2CClock(bool value);
  void setI2CData(bool value);
   
  bool myI2CData = false;
  bool myI2CClock = false;

  string myEEPROMFile;
  unique_ptr<MT24LC256> myEEPROM;

  void doI2C();

  uInt8 readI2C();

  // Following constructors and assignment operators not supported
  CartridgeEFF() = delete;
  CartridgeEFF(const CartridgeEFF&) = delete;
  CartridgeEFF(CartridgeEFF&&) = delete;
  CartridgeEFF& operator=(const CartridgeEFF&) = delete;
  CartridgeEFF& operator=(CartridgeEFF&&) = delete;
};

#endif
