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
// Copyright (c) 1995-2020 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#include <cmath>

#include "Console.hxx"
#include "FrameBuffer.hxx"

#include "PaletteHandler.hxx"

PaletteHandler::PaletteHandler(OSystem& system)
  : myOSystem(system)
{
  // Load user-defined palette for this ROM
  loadUserPalette();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PaletteHandler::PaletteType PaletteHandler::toPaletteType(const string& name) const
{
  if(name == SETTING_Z26)
    return PaletteType::Z26;

  if(name == SETTING_USER && myUserPaletteDefined)
    return PaletteType::User;

  if(name == SETTING_CUSTOM)
    return PaletteType::Custom;

  return PaletteType::Standard;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string PaletteHandler::toPaletteName(PaletteType type) const
{
  const string SETTING_NAMES[int(PaletteType::NumTypes)] = {
    SETTING_STANDARD, SETTING_Z26, SETTING_USER, SETTING_CUSTOM
  };

  return SETTING_NAMES[type];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::cyclePalette(bool next)
{
  const string MESSAGES[PaletteType::NumTypes] = {
    "Standard Stella", "Z26", "User-defined", "Custom"
  };
  int type = toPaletteType(myOSystem.settings().getString("palette"));

  if(next)
  {
    if(type == PaletteType::MaxType)
      type = PaletteType::Standard;
    else
      type++;
    // If we have no user-defined palette, we will skip it
    if(type == PaletteType::User && !myUserPaletteDefined)
      type++;
  }
  else
  {
    if(type == PaletteType::MinType)
      type = PaletteType::MaxType;
    else
      type--;
    // If we have no user-defined palette, we will skip it
    if(type == PaletteType::User && !myUserPaletteDefined)
      type--;
  }

  const string palette = toPaletteName(PaletteType(type));
  const string message = MESSAGES[type] + " palette";

  myOSystem.frameBuffer().showMessage(message);

  setPalette(palette);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::cycleAdjustable(bool next)
{
  const bool isCustomPalette = SETTING_CUSTOM == myOSystem.settings().getString("palette");
  bool isPhaseShift;

  do {
    if(next)
    {
      myCurrentAdjustable++;
      myCurrentAdjustable %= NUM_ADJUSTABLES;
    }
    else
    {
      if(myCurrentAdjustable == 0)
        myCurrentAdjustable = NUM_ADJUSTABLES - 1;
      else
        myCurrentAdjustable--;
    }
    isPhaseShift = myAdjustables[myCurrentAdjustable].value == nullptr;

    // skip phase shift when 'Custom' palette is not selected
  } while(isPhaseShift && !isCustomPalette);

  ostringstream buf;
  buf << "Palette adjustable '" << myAdjustables[myCurrentAdjustable].name
    << "' selected (";
  if(isPhaseShift)
    buf << (myOSystem.console().timing() == ConsoleTiming::pal ? myPhasePAL : myPhaseNTSC)
      << DEGREE << ")";
  else
    buf << scaleTo100(*myAdjustables[myCurrentAdjustable].value) << "%)";

  myOSystem.frameBuffer().showMessage(buf.str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::changeAdjustable(bool increase)
{
  if(myAdjustables[myCurrentAdjustable].value == nullptr)
    changeColorPhaseShift(increase);
  else
  {
    int newVal = scaleTo100(*myAdjustables[myCurrentAdjustable].value);

    if(increase)
      newVal += 2;    // += 2%
    else
      newVal -= 2;    // -= 2%
    newVal = BSPF::clamp(newVal, 0, 100);

    *myAdjustables[myCurrentAdjustable].value = scaleFrom100(newVal);

    ostringstream buf;
    buf << "Custom '" << myAdjustables[myCurrentAdjustable].name
        << "' set to " << newVal << "%";

    myOSystem.frameBuffer().showMessage(buf.str());
    setPalette();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::changeColorPhaseShift(bool increase)
{
  const ConsoleTiming timing = myOSystem.console().timing();

  // SECAM is not supported
  if(timing != ConsoleTiming::secam)
  {
    const bool isNTSC = timing == ConsoleTiming::ntsc;
    const float shift = isNTSC ? DEF_NTSC_SHIFT : DEF_PAL_SHIFT;
    float newPhase = isNTSC ? myPhaseNTSC : myPhasePAL;

    if(increase)        // increase color phase shift
      newPhase += 0.3F;
    else                // decrease color phase shift
      newPhase -= 0.3F;
    newPhase = BSPF::clamp(newPhase, shift - MAX_SHIFT, shift + MAX_SHIFT);

    if(isNTSC)
      myPhaseNTSC = newPhase;
    else
      myPhasePAL = newPhase;

    generateCustomPalette(timing);
    setPalette(SETTING_CUSTOM);

    ostringstream ss;
    ss << "Color phase shift at "
      << std::fixed << std::setprecision(1) << newPhase << DEGREE;

    myOSystem.frameBuffer().showMessage(ss.str());
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::loadConfig(const Settings& settings)
{
  // Load adjustables
  myPhaseNTSC   = BSPF::clamp(settings.getFloat("tv.phase_ntsc"),
                              DEF_NTSC_SHIFT - MAX_SHIFT, DEF_NTSC_SHIFT + MAX_SHIFT);
  myPhasePAL    = BSPF::clamp(settings.getFloat("tv.phase_pal"),
                              DEF_PAL_SHIFT - MAX_SHIFT, DEF_PAL_SHIFT + MAX_SHIFT);

  myHue         = BSPF::clamp(settings.getFloat("tv.hue"), -1.0F, 1.0F);
  mySaturation  = BSPF::clamp(settings.getFloat("tv.saturation"), -1.0F, 1.0F);
  myContrast    = BSPF::clamp(settings.getFloat("tv.contrast"), -1.0F, 1.0F);
  myBrightness  = BSPF::clamp(settings.getFloat("tv.brightness"), -1.0F, 1.0F);
  myGamma       = BSPF::clamp(settings.getFloat("tv.gamma"), -1.0F, 1.0F);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::saveConfig(Settings& settings) const
{
  // Save adjustables
  settings.setValue("tv.phase_ntsc", myPhaseNTSC);
  settings.setValue("tv.phase_pal", myPhasePAL);

  settings.setValue("tv.hue", myHue);
  settings.setValue("tv.saturation", mySaturation);
  settings.setValue("tv.contrast", myContrast);
  settings.setValue("tv.brightness", myBrightness);
  settings.setValue("tv.gamma", myGamma);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::setAdjustables(const Adjustable& adjustable)
{
  myPhaseNTSC   = adjustable.phaseNtsc / 10.F;
  myPhasePAL    = adjustable.phasePal / 10.F;

  myHue         = scaleFrom100(adjustable.hue);
  mySaturation  = scaleFrom100(adjustable.saturation);
  myContrast    = scaleFrom100(adjustable.contrast);
  myBrightness  = scaleFrom100(adjustable.brightness);
  myGamma       = scaleFrom100(adjustable.gamma);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::getAdjustables(Adjustable& adjustable) const
{
  adjustable.phaseNtsc   = myPhaseNTSC * 10.F;
  adjustable.phasePal    = myPhasePAL * 10.F;

  adjustable.hue         = scaleTo100(myHue);
  adjustable.saturation  = scaleTo100(mySaturation);
  adjustable.contrast    = scaleTo100(myContrast);
  adjustable.brightness  = scaleTo100(myBrightness);
  adjustable.gamma       = scaleTo100(myGamma);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::setPalette(const string& name)
{
  myOSystem.settings().setValue("palette", name);

  setPalette();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::setPalette()
{
  if(myOSystem.hasConsole())
  {
    const string& name = myOSystem.settings().getString("palette");

    // Look at all the palettes, since we don't know which one is
    // currently active
    static constexpr BSPF::array2D<const PaletteArray*, PaletteType::NumTypes, int(ConsoleTiming::numTimings)> palettes = {{
      { &ourNTSCPalette,       &ourPALPalette,       &ourSECAMPalette     },
      { &ourNTSCPaletteZ26,    &ourPALPaletteZ26,    &ourSECAMPaletteZ26  },
      { &ourUserNTSCPalette,   &ourUserPALPalette,   &ourUserSECAMPalette },
      { &ourCustomNTSCPalette, &ourCustomPALPalette, &ourSECAMPalette     }
      }};
    // See which format we should be using
    const ConsoleTiming timing = myOSystem.console().timing();
    const PaletteType paletteType = toPaletteType(name);
    // Now consider the current display format
    const PaletteArray* palette = palettes[paletteType][int(timing)];

    if(paletteType == PaletteType::Custom)
      generateCustomPalette(timing);

    myOSystem.frameBuffer().setTIAPalette(adjustedPalette(*palette));
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PaletteArray PaletteHandler::adjustedPalette(const PaletteArray& palette)
{
  PaletteArray destPalette;
  // Constants for saturation and gray scale calculation
  constexpr float PR = .2989F;
  constexpr float PG = .5870F;
  constexpr float PB = .1140F;
  // Generate adjust table
  constexpr int ADJUST_SIZE = 256;
  constexpr int RGB_UNIT = 1 << 8;
  constexpr float RGB_OFFSET = 0.5F;
  const float hue = myHue;
  const float brightness = myBrightness * (0.5F * RGB_UNIT) + RGB_OFFSET;
  const float contrast = myContrast * (0.5F * RGB_UNIT) + RGB_UNIT;
  const float saturation = mySaturation + 1;
  const float gamma = 1.1333F - myGamma * 0.5F;
  /* match common PC's 2.2 gamma to TV's 2.65 gamma */
  constexpr float toFloat = 1.F / (ADJUST_SIZE - 1);
  std::array<float, ADJUST_SIZE> adjust;

  for(int i = 0; i < ADJUST_SIZE; i++)
    adjust[i] = powf(i * toFloat, gamma) * contrast + brightness;

  // Transform original palette into destination palette
  for(size_t i = 0; i < destPalette.size(); i += 2)
  {
    const uInt32 pixel = palette[i];
    int r = (pixel >> 16) & 0xff;
    int g = (pixel >> 8)  & 0xff;
    int b = (pixel >> 0)  & 0xff;

    // adjust hue (different for NTSC and PAL?) and saturation
    adjustHueSaturation(r, g, b, hue, saturation);

    // adjust contrast, brightness, gamma
    r = adjust[r];
    g = adjust[g];
    b = adjust[b];

    r = BSPF::clamp(r, 0, 255);
    g = BSPF::clamp(g, 0, 255);
    b = BSPF::clamp(b, 0, 255);

    destPalette[i] = (r << 16) + (g << 8) + b;

    // Fill the odd numbered palette entries with gray values (calculated
    // using the standard RGB -> grayscale conversion formula)
    // Used for PAL color-loss data and 'greying out' the frame in the debugger.
    const uInt8 lum = static_cast<uInt8>((r * PR) + (g * PG) + (b * PB));

    destPalette[i + 1] = (lum << 16) + (lum << 8) + lum;
  }
  return destPalette;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::loadUserPalette()
{
  if (!myOSystem.checkUserPalette(true))
    return;

  const string& palette = myOSystem.paletteFile();
  ifstream in(palette, std::ios::binary);

  // Now that we have valid data, create the user-defined palettes
  std::array<uInt8, 3> pixbuf;  // Temporary buffer for one 24-bit pixel

  for(int i = 0; i < 128; i++)  // NTSC palette
  {
    in.read(reinterpret_cast<char*>(pixbuf.data()), 3);
    const uInt32 pixel = (int(pixbuf[0]) << 16) + (int(pixbuf[1]) << 8) + int(pixbuf[2]);
    ourUserNTSCPalette[(i<<1)] = pixel;
  }
  for(int i = 0; i < 128; i++)  // PAL palette
  {
    in.read(reinterpret_cast<char*>(pixbuf.data()), 3);
    const uInt32 pixel = (int(pixbuf[0]) << 16) + (int(pixbuf[1]) << 8) + int(pixbuf[2]);
    ourUserPALPalette[(i<<1)] = pixel;
  }

  std::array<uInt32, 16> secam;  // All 8 24-bit pixels, plus 8 colorloss pixels
  for(int i = 0; i < 8; i++)     // SECAM palette
  {
    in.read(reinterpret_cast<char*>(pixbuf.data()), 3);
    const uInt32 pixel = (int(pixbuf[0]) << 16) + (int(pixbuf[1]) << 8) + int(pixbuf[2]);
    secam[(i<<1)]   = pixel;
    secam[(i<<1)+1] = 0;
  }
  uInt32* ptr = ourUserSECAMPalette.data();
  for(int i = 0; i < 16; ++i)
  {
    const uInt32* s = secam.data();
    for(int j = 0; j < 16; ++j)
      *ptr++ = *s++;
  }

  myUserPaletteDefined = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::generateCustomPalette(ConsoleTiming timing)
{
  constexpr int NUM_CHROMA = 16;
  constexpr int NUM_LUMA = 8;
  constexpr float SATURATION = 0.25F; // default saturation

  float color[NUM_CHROMA][2] = {{0.0F}};

  if(timing == ConsoleTiming::ntsc)
  {
    // YIQ is YUV shifted by 33�
    constexpr float offset = 33 * BSPF::PI_f / 180;
    const float shift = myPhaseNTSC * BSPF::PI_f / 180;

    // color 0 is grayscale
    for(int chroma = 1; chroma < NUM_CHROMA; chroma++)
    {
      color[chroma][0] = SATURATION * sinf(offset + shift * (chroma - 1));
      color[chroma][1] = SATURATION * sinf(offset + shift * (chroma - 1 - BSPF::PI_f));
    }

    for(int chroma = 0; chroma < NUM_CHROMA; chroma++)
    {
      const float I = color[chroma][0];
      const float Q = color[chroma][1];

      for(int luma = 0; luma < NUM_LUMA; luma++)
      {
        const float Y = 0.05F + luma / 8.24F; // 0.05..~0.90

        float R = Y + 0.956F * I + 0.621F * Q;
        float G = Y - 0.272F * I - 0.647F * Q;
        float B = Y - 1.106F * I + 1.703F * Q;

        if(R < 0) R = 0;
        if(G < 0) G = 0;
        if(B < 0) B = 0;

        R = powf(R, 0.9F);
        G = powf(G, 0.9F);
        B = powf(B, 0.9F);

        int r = BSPF::clamp(R * 255.F, 0.F, 255.F);
        int g = BSPF::clamp(G * 255.F, 0.F, 255.F);
        int b = BSPF::clamp(B * 255.F, 0.F, 255.F);

        ourCustomNTSCPalette[(chroma * NUM_LUMA + luma) << 1] = (r << 16) + (g << 8) + b;
      }
    }
  }
  else if(timing == ConsoleTiming::pal)
  {
    constexpr float offset = BSPF::PI_f;
    const float shift = myPhasePAL * BSPF::PI_f / 180;
    constexpr float fixedShift = 22.5F * BSPF::PI_f / 180;

    // colors 0, 1, 14 and 15 are grayscale
    for(int chroma = 2; chroma < NUM_CHROMA - 2; chroma++)
    {
      int idx = NUM_CHROMA - 1 - chroma;
      color[idx][0] = SATURATION * sinf(offset - fixedShift * chroma);
      if ((idx & 1) == 0)
        color[idx][1] = SATURATION * sinf(offset - shift * (chroma - 3.5F) / 2.F);
      else
        color[idx][1] = SATURATION * -sinf(offset - shift * chroma / 2.F);
    }

    for(int chroma = 0; chroma < NUM_CHROMA; chroma++)
    {
      const float U = color[chroma][0];
      const float V = color[chroma][1];

      for(int luma = 0; luma < NUM_LUMA; luma++)
      {
        const float Y = 0.05F + luma / 8.24F; // 0.05..~0.90

        // Most sources
        float R = Y + 1.403F * V;
        float G = Y - 0.344F * U - 0.714F * V;
        float B = Y + 1.770F * U;
        // German Wikipedia, huh???
        //float B = Y + 1 / 0.493 * U;
        //float R = Y + 1 / 0.877 * V;
        //float G = 1.704 * Y - 0.590 * R - 0.194 * B;

        if(R < 0) R = 0.0;
        if(G < 0) G = 0.0;
        if(B < 0) B = 0.0;

        R = powf(R, 1.2F);
        G = powf(G, 1.2F);
        B = powf(B, 1.2F);

        int r = BSPF::clamp(R * 255.F, 0.F, 255.F);
        int g = BSPF::clamp(G * 255.F, 0.F, 255.F);
        int b = BSPF::clamp(B * 255.F, 0.F, 255.F);

        ourCustomPALPalette[(chroma * NUM_LUMA + luma) << 1] = (r << 16) + (g << 8) + b;
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PaletteHandler::adjustHueSaturation(int& R, int& G, int& B, float H, float S)
{
  // Adapted from http://beesbuzz.biz/code/16-hsv-color-transforms
  // (C) J. �Fluffy� Shagam
  // License: CC BY-SA 4.0
  const float su = S * cosf(-H * BSPF::PI_f);
  const float sw = S * sinf(-H * BSPF::PI_f);
  const float r = (.299F + .701F * su + .168F * sw) * R
                + (.587F - .587F * su + .330F * sw) * G
                + (.114F - .114F * su - .497F * sw) * B;
  const float g = (.299F - .299F * su - .328F * sw) * R
                + (.587F + .413F * su + .035F * sw) * G
                + (.114F - .114F * su + .292F * sw) * B;
  const float b = (.299F - .300F * su + 1.25F * sw) * R
                + (.587F - .588F * su - 1.05F * sw) * G
                + (.114F + .886F * su - .203F * sw) * B;

  R = BSPF::clamp(r, 0.F, 255.F);
  G = BSPF::clamp(g, 0.F, 255.F);
  B = BSPF::clamp(b, 0.F, 255.F);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const PaletteArray PaletteHandler::ourNTSCPalette = {
  0x000000, 0, 0x4a4a4a, 0, 0x6f6f6f, 0, 0x8e8e8e, 0,
  0xaaaaaa, 0, 0xc0c0c0, 0, 0xd6d6d6, 0, 0xececec, 0,
  0x484800, 0, 0x69690f, 0, 0x86861d, 0, 0xa2a22a, 0,
  0xbbbb35, 0, 0xd2d240, 0, 0xe8e84a, 0, 0xfcfc54, 0,
  0x7c2c00, 0, 0x904811, 0, 0xa26221, 0, 0xb47a30, 0,
  0xc3903d, 0, 0xd2a44a, 0, 0xdfb755, 0, 0xecc860, 0,
  0x901c00, 0, 0xa33915, 0, 0xb55328, 0, 0xc66c3a, 0,
  0xd5824a, 0, 0xe39759, 0, 0xf0aa67, 0, 0xfcbc74, 0,
  0x940000, 0, 0xa71a1a, 0, 0xb83232, 0, 0xc84848, 0,
  0xd65c5c, 0, 0xe46f6f, 0, 0xf08080, 0, 0xfc9090, 0,
  0x840064, 0, 0x97197a, 0, 0xa8308f, 0, 0xb846a2, 0,
  0xc659b3, 0, 0xd46cc3, 0, 0xe07cd2, 0, 0xec8ce0, 0,
  0x500084, 0, 0x68199a, 0, 0x7d30ad, 0, 0x9246c0, 0,
  0xa459d0, 0, 0xb56ce0, 0, 0xc57cee, 0, 0xd48cfc, 0,
  0x140090, 0, 0x331aa3, 0, 0x4e32b5, 0, 0x6848c6, 0,
  0x7f5cd5, 0, 0x956fe3, 0, 0xa980f0, 0, 0xbc90fc, 0,
  0x000094, 0, 0x181aa7, 0, 0x2d32b8, 0, 0x4248c8, 0,
  0x545cd6, 0, 0x656fe4, 0, 0x7580f0, 0, 0x8490fc, 0,
  0x001c88, 0, 0x183b9d, 0, 0x2d57b0, 0, 0x4272c2, 0,
  0x548ad2, 0, 0x65a0e1, 0, 0x75b5ef, 0, 0x84c8fc, 0,
  0x003064, 0, 0x185080, 0, 0x2d6d98, 0, 0x4288b0, 0,
  0x54a0c5, 0, 0x65b7d9, 0, 0x75cceb, 0, 0x84e0fc, 0,
  0x004030, 0, 0x18624e, 0, 0x2d8169, 0, 0x429e82, 0,
  0x54b899, 0, 0x65d1ae, 0, 0x75e7c2, 0, 0x84fcd4, 0,
  0x004400, 0, 0x1a661a, 0, 0x328432, 0, 0x48a048, 0,
  0x5cba5c, 0, 0x6fd26f, 0, 0x80e880, 0, 0x90fc90, 0,
  0x143c00, 0, 0x355f18, 0, 0x527e2d, 0, 0x6e9c42, 0,
  0x87b754, 0, 0x9ed065, 0, 0xb4e775, 0, 0xc8fc84, 0,
  0x303800, 0, 0x505916, 0, 0x6d762b, 0, 0x88923e, 0,
  0xa0ab4f, 0, 0xb7c25f, 0, 0xccd86e, 0, 0xe0ec7c, 0,
  0x482c00, 0, 0x694d14, 0, 0x866a26, 0, 0xa28638, 0,
  0xbb9f47, 0, 0xd2b656, 0, 0xe8cc63, 0, 0xfce070, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const PaletteArray PaletteHandler::ourPALPalette = {
  0x000000, 0, 0x121212, 0, 0x242424, 0, 0x484848, 0, // 180 0
  0x6c6c6c, 0, 0x909090, 0, 0xb4b4b4, 0, 0xd8d8d8, 0, // was 0x111111..0xcccccc
  0x000000, 0, 0x121212, 0, 0x242424, 0, 0x484848, 0, // 198 1
  0x6c6c6c, 0, 0x909090, 0, 0xb4b4b4, 0, 0xd8d8d8, 0,
  0x1d0f00, 0, 0x3f2700, 0, 0x614900, 0, 0x836b01, 0, // 1b0 2
  0xa58d23, 0, 0xc7af45, 0, 0xe9d167, 0, 0xffe789, 0, // was ..0xfff389
  0x002400, 0, 0x004600, 0, 0x216800, 0, 0x438a07, 0, // 1c8 3
  0x65ac29, 0, 0x87ce4b, 0, 0xa9f06d, 0, 0xcbff8f, 0,
  0x340000, 0, 0x561400, 0, 0x783602, 0, 0x9a5824, 0, // 1e0 4
  0xbc7a46, 0, 0xde9c68, 0, 0xffbe8a, 0, 0xffd0ad, 0, // was ..0xffe0ac
  0x002700, 0, 0x004900, 0, 0x0c6b0c, 0, 0x2e8d2e, 0, // 1f8 5
  0x50af50, 0, 0x72d172, 0, 0x94f394, 0, 0xb6ffb6, 0,
  0x3d0008, 0, 0x610511, 0, 0x832733, 0, 0xa54955, 0, // 210 6
  0xc76b77, 0, 0xe98d99, 0, 0xffafbb, 0, 0xffd1d7, 0, // was 0x3f0000..0xffd1dd
  0x001e12, 0, 0x004228, 0, 0x046540, 0, 0x268762, 0, // 228 7
  0x48a984, 0, 0x6acba6, 0, 0x8cedc8, 0, 0xafffe0, 0, // was 0x002100, 0x00431e..0xaeffff
  0x300025, 0, 0x5f0047, 0, 0x811e69, 0, 0xa3408b, 0, // 240 8
  0xc562ad, 0, 0xe784cf, 0, 0xffa8ea, 0, 0xffc9f2, 0, // was ..0xffa6f1, 0xffc8ff
  0x001431, 0, 0x003653, 0, 0x0a5875, 0, 0x2c7a97, 0, // 258 9
  0x4e9cb9, 0, 0x70bedb, 0, 0x92e0fd, 0, 0xb4ffff, 0,
  0x2c0052, 0, 0x4e0074, 0, 0x701d96, 0, 0x923fb8, 0, // 270 a
  0xb461da, 0, 0xd683fc, 0, 0xe2a5ff, 0, 0xeec9ff, 0, // was ..0xf8a5ff, 0xffc7ff
  0x001759, 0, 0x00247c, 0, 0x1d469e, 0, 0x3f68c0, 0, // 288 b
  0x618ae2, 0, 0x83acff, 0, 0xa5ceff, 0, 0xc7f0ff, 0,
  0x12006d, 0, 0x34038f, 0, 0x5625b1, 0, 0x7847d3, 0, // 2a0 c
  0x9a69f5, 0, 0xb48cff, 0, 0xc9adff, 0, 0xe1d1ff, 0, // was ..0xbc8bff, 0xdeadff, 0xffcfff,
  0x000070, 0, 0x161292, 0, 0x3834b4, 0, 0x5a56d6, 0, // 2b8 d
  0x7c78f8, 0, 0x9e9aff, 0, 0xc0bcff, 0, 0xe2deff, 0,
  0x000000, 0, 0x121212, 0, 0x242424, 0, 0x484848, 0, // 2d0 e
  0x6c6c6c, 0, 0x909090, 0, 0xb4b4b4, 0, 0xd8d8d8, 0,
  0x000000, 0, 0x121212, 0, 0x242424, 0, 0x484848, 0, // 2e8 f
  0x6c6c6c, 0, 0x909090, 0, 0xb4b4b4, 0, 0xd8d8d8, 0,
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const PaletteArray PaletteHandler::ourSECAMPalette = {
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff50ff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const PaletteArray PaletteHandler::ourNTSCPaletteZ26 = {
  0x000000, 0, 0x505050, 0, 0x646464, 0, 0x787878, 0,
  0x8c8c8c, 0, 0xa0a0a0, 0, 0xb4b4b4, 0, 0xc8c8c8, 0,
  0x445400, 0, 0x586800, 0, 0x6c7c00, 0, 0x809000, 0,
  0x94a414, 0, 0xa8b828, 0, 0xbccc3c, 0, 0xd0e050, 0,
  0x673900, 0, 0x7b4d00, 0, 0x8f6100, 0, 0xa37513, 0,
  0xb78927, 0, 0xcb9d3b, 0, 0xdfb14f, 0, 0xf3c563, 0,
  0x7b2504, 0, 0x8f3918, 0, 0xa34d2c, 0, 0xb76140, 0,
  0xcb7554, 0, 0xdf8968, 0, 0xf39d7c, 0, 0xffb190, 0,
  0x7d122c, 0, 0x912640, 0, 0xa53a54, 0, 0xb94e68, 0,
  0xcd627c, 0, 0xe17690, 0, 0xf58aa4, 0, 0xff9eb8, 0,
  0x730871, 0, 0x871c85, 0, 0x9b3099, 0, 0xaf44ad, 0,
  0xc358c1, 0, 0xd76cd5, 0, 0xeb80e9, 0, 0xff94fd, 0,
  0x5d0b92, 0, 0x711fa6, 0, 0x8533ba, 0, 0x9947ce, 0,
  0xad5be2, 0, 0xc16ff6, 0, 0xd583ff, 0, 0xe997ff, 0,
  0x401599, 0, 0x5429ad, 0, 0x683dc1, 0, 0x7c51d5, 0,
  0x9065e9, 0, 0xa479fd, 0, 0xb88dff, 0, 0xcca1ff, 0,
  0x252593, 0, 0x3939a7, 0, 0x4d4dbb, 0, 0x6161cf, 0,
  0x7575e3, 0, 0x8989f7, 0, 0x9d9dff, 0, 0xb1b1ff, 0,
  0x0f3480, 0, 0x234894, 0, 0x375ca8, 0, 0x4b70bc, 0,
  0x5f84d0, 0, 0x7398e4, 0, 0x87acf8, 0, 0x9bc0ff, 0,
  0x04425a, 0, 0x18566e, 0, 0x2c6a82, 0, 0x407e96, 0,
  0x5492aa, 0, 0x68a6be, 0, 0x7cbad2, 0, 0x90cee6, 0,
  0x044f30, 0, 0x186344, 0, 0x2c7758, 0, 0x408b6c, 0,
  0x549f80, 0, 0x68b394, 0, 0x7cc7a8, 0, 0x90dbbc, 0,
  0x0f550a, 0, 0x23691e, 0, 0x377d32, 0, 0x4b9146, 0,
  0x5fa55a, 0, 0x73b96e, 0, 0x87cd82, 0, 0x9be196, 0,
  0x1f5100, 0, 0x336505, 0, 0x477919, 0, 0x5b8d2d, 0,
  0x6fa141, 0, 0x83b555, 0, 0x97c969, 0, 0xabdd7d, 0,
  0x344600, 0, 0x485a00, 0, 0x5c6e14, 0, 0x708228, 0,
  0x84963c, 0, 0x98aa50, 0, 0xacbe64, 0, 0xc0d278, 0,
  0x463e00, 0, 0x5a5205, 0, 0x6e6619, 0, 0x827a2d, 0,
  0x968e41, 0, 0xaaa255, 0, 0xbeb669, 0, 0xd2ca7d, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const PaletteArray PaletteHandler::ourPALPaletteZ26 = {
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0,
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0,
  0x533a00, 0, 0x674e00, 0, 0x7b6203, 0, 0x8f7617, 0,
  0xa38a2b, 0, 0xb79e3f, 0, 0xcbb253, 0, 0xdfc667, 0,
  0x1b5800, 0, 0x2f6c00, 0, 0x438001, 0, 0x579415, 0,
  0x6ba829, 0, 0x7fbc3d, 0, 0x93d051, 0, 0xa7e465, 0,
  0x6a2900, 0, 0x7e3d12, 0, 0x925126, 0, 0xa6653a, 0,
  0xba794e, 0, 0xce8d62, 0, 0xe2a176, 0, 0xf6b58a, 0,
  0x075b00, 0, 0x1b6f11, 0, 0x2f8325, 0, 0x439739, 0,
  0x57ab4d, 0, 0x6bbf61, 0, 0x7fd375, 0, 0x93e789, 0,
  0x741b2f, 0, 0x882f43, 0, 0x9c4357, 0, 0xb0576b, 0,
  0xc46b7f, 0, 0xd87f93, 0, 0xec93a7, 0, 0xffa7bb, 0,
  0x00572e, 0, 0x106b42, 0, 0x247f56, 0, 0x38936a, 0,
  0x4ca77e, 0, 0x60bb92, 0, 0x74cfa6, 0, 0x88e3ba, 0,
  0x6d165f, 0, 0x812a73, 0, 0x953e87, 0, 0xa9529b, 0,
  0xbd66af, 0, 0xd17ac3, 0, 0xe58ed7, 0, 0xf9a2eb, 0,
  0x014c5e, 0, 0x156072, 0, 0x297486, 0, 0x3d889a, 0,
  0x519cae, 0, 0x65b0c2, 0, 0x79c4d6, 0, 0x8dd8ea, 0,
  0x5f1588, 0, 0x73299c, 0, 0x873db0, 0, 0x9b51c4, 0,
  0xaf65d8, 0, 0xc379ec, 0, 0xd78dff, 0, 0xeba1ff, 0,
  0x123b87, 0, 0x264f9b, 0, 0x3a63af, 0, 0x4e77c3, 0,
  0x628bd7, 0, 0x769feb, 0, 0x8ab3ff, 0, 0x9ec7ff, 0,
  0x451e9d, 0, 0x5932b1, 0, 0x6d46c5, 0, 0x815ad9, 0,
  0x956eed, 0, 0xa982ff, 0, 0xbd96ff, 0, 0xd1aaff, 0,
  0x2a2b9e, 0, 0x3e3fb2, 0, 0x5253c6, 0, 0x6667da, 0,
  0x7a7bee, 0, 0x8e8fff, 0, 0xa2a3ff, 0, 0xb6b7ff, 0,
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0,
  0x000000, 0, 0x4c4c4c, 0, 0x606060, 0, 0x747474, 0,
  0x888888, 0, 0x9c9c9c, 0, 0xb0b0b0, 0, 0xc4c4c4, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const PaletteArray PaletteHandler::ourSECAMPaletteZ26 = {
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0,
  0x000000, 0, 0x2121ff, 0, 0xf03c79, 0, 0xff3cff, 0,
  0x7fff00, 0, 0x7fffff, 0, 0xffff3f, 0, 0xffffff, 0
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PaletteArray PaletteHandler::ourUserNTSCPalette  = { 0 }; // filled from external file

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PaletteArray PaletteHandler::ourUserPALPalette   = { 0 }; // filled from external file

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PaletteArray PaletteHandler::ourUserSECAMPalette = { 0 }; // filled from external file

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PaletteArray PaletteHandler::ourCustomNTSCPalette = { 0 }; // filled by function

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PaletteArray PaletteHandler::ourCustomPALPalette  = { 0 }; // filled by function
