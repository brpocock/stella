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

#include "FBSurfaceSDL2.hxx"

#include "ThreadDebugging.hxx"
#include "sdl_blitter/BilinearBlitter.hxx"
#include "sdl_blitter/HqBlitter.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FBSurfaceSDL2::FBSurfaceSDL2(FrameBufferSDL2& buffer,
                             uInt32 width, uInt32 height, const uInt32* data)
  : myFB(buffer),
    mySurface(nullptr),
    myIsVisible(true),
    myIsStatic(false)
{
  myBlitter = make_unique<HqBlitter>(buffer);
  createSurface(width, height, data);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FBSurfaceSDL2::~FBSurfaceSDL2()
{
  ASSERT_MAIN_THREAD;

  if(mySurface)
  {
    SDL_FreeSurface(mySurface);
    mySurface = nullptr;
  }

  free();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::fillRect(uInt32 x, uInt32 y, uInt32 w, uInt32 h, ColorId color)
{
  ASSERT_MAIN_THREAD;

  // Fill the rectangle
  SDL_Rect tmp;
  tmp.x = x;
  tmp.y = y;
  tmp.w = w;
  tmp.h = h;
  SDL_FillRect(mySurface, &tmp, myPalette[color]);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 FBSurfaceSDL2::width() const
{
  return mySurface->w;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 FBSurfaceSDL2::height() const
{
  return mySurface->h;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const Common::Rect& FBSurfaceSDL2::srcRect() const
{
  return mySrcGUIR;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const Common::Rect& FBSurfaceSDL2::dstRect() const
{
  return myDstGUIR;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::setSrcPos(uInt32 x, uInt32 y)
{
  mySrcR.x = x;  mySrcR.y = y;
  mySrcGUIR.moveTo(x, y);

  reinitializeBlitter();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::setSrcSize(uInt32 w, uInt32 h)
{
  mySrcR.w = w;  mySrcR.h = h;
  mySrcGUIR.setWidth(w);  mySrcGUIR.setHeight(h);

  reinitializeBlitter();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::setDstPos(uInt32 x, uInt32 y)
{
  myDstR.x = x;  myDstR.y = y;
  myDstGUIR.moveTo(x, y);

  reinitializeBlitter();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::setDstSize(uInt32 w, uInt32 h)
{
  myDstR.w = w;  myDstR.h = h;
  myDstGUIR.setWidth(w);  myDstGUIR.setHeight(h);

  reinitializeBlitter();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::setVisible(bool visible)
{
  myIsVisible = visible;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::translateCoords(Int32& x, Int32& y) const
{
  x -= myDstR.x;  x /= myDstR.w / mySrcR.w;
  y -= myDstR.y;  y /= myDstR.h / mySrcR.h;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool FBSurfaceSDL2::render()
{
  if(myIsVisible)
  {
    myBlitter->blit(*mySurface);

    return true;
  }
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::invalidate()
{
  ASSERT_MAIN_THREAD;

  SDL_FillRect(mySurface, nullptr, 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::free()
{
  myBlitter->free();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::reload()
{
  reinitializeBlitter();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::resize(uInt32 width, uInt32 height)
{
  ASSERT_MAIN_THREAD;

  if(mySurface)
    SDL_FreeSurface(mySurface);
  free();

  createSurface(width, height, nullptr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::createSurface(uInt32 width, uInt32 height,
                                  const uInt32* data)
{
  ASSERT_MAIN_THREAD;

  // Create a surface in the same format as the parent GL class
  const SDL_PixelFormat* pf = myFB.myPixelFormat;

  mySurface = SDL_CreateRGBSurface(0, width, height,
      pf->BitsPerPixel, pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);

  // We start out with the src and dst rectangles containing the same
  // dimensions, indicating no scaling or re-positioning
  setSrcPos(0, 0);
  setDstPos(0, 0);
  setSrcSize(width, height);
  setDstSize(width, height);

  ////////////////////////////////////////////////////
  // These *must* be set for the parent class
  myPixels = reinterpret_cast<uInt32*>(mySurface->pixels);
  myPitch = mySurface->pitch / pf->BytesPerPixel;
  ////////////////////////////////////////////////////

  if(data)
  {
    myIsStatic = true;
    SDL_memcpy(mySurface->pixels, data, mySurface->w * mySurface->h * 4);
  }

  reinitializeBlitter();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::reinitializeBlitter()
{
  myBlitter->reinitialize(mySrcR, myDstR, myAttributes, myIsStatic ? mySurface : nullptr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void FBSurfaceSDL2::applyAttributes(bool immediate)
{
  reinitializeBlitter();
}
