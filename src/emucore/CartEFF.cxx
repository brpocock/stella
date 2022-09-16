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

#include "System.hxx"
#include "CartEFF.hxx"
#include "FSNode.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeEFF::CartridgeEFF(const ByteBuffer& image, size_t size,
                         const string& md5, const Settings& settings,
                         size_t bsSize)
  : CartridgeEnhanced(image, size, md5, settings, bsSize),
    myEEPROM{nullptr}
{
  myRamSize = RAM_SIZE;
  myRamBankCount = 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeEFF::checkSwitchBank(uInt16 address, uInt8)
{
  address &= ROM_MASK;

  // Switch banks if necessary
  if((address >= 0x0FE0) && (address <= 0x0FEF))
  {
    bank(address - 0x0FE0);
    return true;
  }
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int CartridgeEFF::descriptionLines()
{
  return 24; // this can be pretty verbose
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string CartridgeEFF::ramDescription()
{
  ostringstream info;

  info << (myRamSize / 1024) << "kiB EEPROM\n"
       << "i²c serial interface @ $FFF0 - $FFF3\n"
       << "i²c read port @ $FFF4 ($FFF5)\n";

  return info.str();
}

uInt8 CartridgeEFF::peek(uInt16 address)
{
  if ((address >= 0x0FF0) && (address <= 0x0FF4))
    {
      switch (address) {
      case 0x0ff0:
        setI2CClock(false);
        break;
      case 0x0ff1:
        setI2CClock(true);
        break;
      case 0x0ff2:
        setI2CData(false);
        break;
      case 0x0ff3:
        setI2CData(true);
        break;
      case 0x0ff4:
        return readI2C();
      };
    }
  return CartridgeEnhanced::peek(address);
}

bool CartridgeEFF::poke(uInt16 address, uInt8 value)
{
  if ((address >= 0x0FF0) && (address <= 0x0FF3))
    {
      switch (address) {
      case 0x0ff0:
        setI2CClock(false);
        setI2CClock(false);
        return false;
      case 0x0ff1:
        setI2CClock(true);
        setI2CClock(true);
        return false;
      case 0x0ff2:
        setI2CData(false);
        setI2CData(false);
        return false;
      case 0x0ff3:
        setI2CData(true);
        setI2CData(true);
        return false;
      };
    }  
  return CartridgeEnhanced::poke(address, value);
}

void CartridgeEFF::setI2CClock(bool value) {
  myI2CClock = value;
  doI2C();
}

void CartridgeEFF::setI2CData(bool value) {
  myI2CData = value;
  doI2C();
}

void CartridgeEFF::doI2C() {
  // TODO
}

uInt8 CartridgeEFF::readI2C() {
  // TODO
  return myImage.get()[0x0ff4]; // zero value
}

void CartridgeEFF::setNVRamFile(const string& nvramdir,
                                const string& romfile)
{
  myEEPROMFile = nvramdir + romfile + "_eeprom.dat";
}
  
void CartridgeEFF::install(System& system)
{
  myEEPROM = make_unique<MT24LC256>(FSNode(myEEPROMFile), system, nullptr);
}
