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
// Copyright (c) 1995-2017 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include <cstring>

#include "Random.hxx"
#include "System.hxx"
#include "CartF9.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeF9::CartridgeF9(const BytePtr& image, uInt32 size, const Settings& settings)
  : Cartridge(settings),
    myCurrentBank(0),
    myImageSize(size)
{
  // Copy the ROM image into my buffer
  memcpy(myImage, image.get(), std::min(1024*1024u, size));
  createCodeAccessBase(size);

  // Remember startup bank
  myStartBank = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF9::reset()
{
  // Upon reset we switch to the startup bank
  bank(myStartBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeF9::install(System& system)
{
  mySystem = &system;

  // Install pages for the startup bank
  bank(myStartBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeF9::peek(uInt16 address)
{

  // technically, reading 0xff9 will trip the circuitry and
  // send the cartridge to a deterministic bank, based on
  // the contents of the data bus; possibly the bank represented by
  // the byte value read from 0xff9 in the current bank; this is
  // not encouraged behavior, and I'm not even sure if it would
  // work on the hardware yet.
  if(address == 0x0FF9)
  {
    bank((myCurrentBank |
          myImage[(myCurrentBank << 12) + address & 0xfff]));
  }

  return myImage[(myCurrentBank << 12) + address & 0xfff];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeF9::poke(uInt16 address, uInt8 value)
{
  address &= 0x0FFF;

  // Switch banks if necessary
  if(address == 0x0FF9) 
  {
    bank(value);
  }

  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeF9::bank(uInt16 bank)
{
  if(bankLocked()) return false;

  // Remember what bank we're in
  myCurrentBank = (bank % (myImageSize / 4096));
  uInt32 offset = myCurrentBank << 12;

  System::PageAccess access(this, System::PA_READ);

  // Set the page accessing methods for the hot spots
  for(uInt32 addr = (0x1FF9 & ~System::PAGE_MASK); addr < 0x2000;
      addr += System::PAGE_SIZE)
  {
    access.codeAccessBase = &myCodeAccessBase[offset + (addr & 0x0FFF)];
    mySystem->setPageAccess(addr, access);
  }

  // Setup the page access methods for the current bank
  for(uInt32 addr = 0x1000; addr < (0x1FF9u & ~System::PAGE_MASK);
      addr += System::PAGE_SIZE)
  {
    access.directPeekBase = &myImage[offset + (addr & 0x0FFF)];
    access.codeAccessBase = &myCodeAccessBase[offset + (addr & 0x0FFF)];
    mySystem->setPageAccess(addr >> System::PAGE_SHIFT, access);
  }
  return myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeF9::getBank() const
{
  return myCurrentBank;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeF9::bankCount() const
{
  return (myImageSize / 4096);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeF9::patch(uInt16 address, uInt8 value)
{
  myImage[(myCurrentBank << 12) + (address & 0x0FFF)] = value;
  return myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8* CartridgeF9::getImage(uInt32& size) const
{
  size = myImageSize;
  return myImage;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeF9::save(Serializer& out) const
{
  try
  {
    out.putString(name());
    out.putShort(myCurrentBank);
  }
  catch(...)
  {
    cerr << "ERROR: CartridgeF9::save" << endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeF9::load(Serializer& in)
{
  try
  {
    if(in.getString() != name())
      return false;

    myCurrentBank = in.getShort();
  }
  catch(...)
  {
    cerr << "ERROR: CartridgeF9::load" << endl;
    return false;
  }

  // Remember what bank we were in
  bank(myCurrentBank);

  return true;
}
