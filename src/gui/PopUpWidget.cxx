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

#include "bspf.hxx"
#include "FrameBuffer.hxx"
#include "FBSurface.hxx"
#include "Font.hxx"
#include "ContextMenu.hxx"
#include "DialogContainer.hxx"
#include "PopUpWidget.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PopUpWidget::PopUpWidget(GuiObject* boss, const GUI::Font& font,
                         int x, int y, int w, int h, const VariantList& list,
                         const string& label, int labelWidth, int cmd)
  : Widget(boss, font, x, y - 1, w, h + 2),
    CommandSender(boss),
    _label(label),
    _labelWidth(labelWidth)
{
  _flags = Widget::FLAG_ENABLED | Widget::FLAG_RETAIN_FOCUS;
  _bgcolor = kDlgColor;
  _bgcolorhi = kDlgColor;     // do not highlight the background
  _textcolor = kTextColor;
  _textcolorhi = kTextColor;  // do not highlight the label

  if(!_label.empty() && _labelWidth == 0)
    _labelWidth = _font.getStringWidth(_label);

  setArrow();

  _w = w + _labelWidth + dropDownWidth(font); // 23

  // vertically center the arrows and text
  myTextY   = (_h - _font.getFontHeight()) / 2;
  myArrowsY = (_h - _arrowHeight) / 2;

  myMenu = make_unique<ContextMenu>(this, font, list, cmd, w);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::setID(uInt32 id)
{
  myMenu->setID(id);

  Widget::setID(id);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::addItems(const VariantList& items)
{
  myMenu->addItems(items);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::setSelected(const Variant& tag, const Variant& def)
{
  myMenu->setSelected(tag, def);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::setSelectedIndex(int idx, bool changed)
{
  _changed = changed;
  myMenu->setSelectedIndex(idx);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::setSelectedMax(bool changed)
{
  _changed = changed;
  myMenu->setSelectedMax();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::clearSelection()
{
  myMenu->clearSelection();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int PopUpWidget::getSelected() const
{
  return myMenu->getSelected();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string& PopUpWidget::getSelectedName() const
{
  return myMenu->getSelectedName();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const Variant& PopUpWidget::getSelectedTag() const
{
  return myMenu->getSelectedTag();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::handleMouseDown(int x, int y, MouseButton b, int clickCount)
{
  if(isEnabled() && !myMenu->isVisible())
  {
    // Add menu just underneath parent widget
    myMenu->show(getAbsX() + _labelWidth, getAbsY() + getHeight(),
                 dialog().surface().dstRect(), myMenu->getSelected());
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::handleMouseWheel(int x, int y, int direction)
{
  if(isEnabled() && !myMenu->isVisible())
  {
    if(direction < 0)
      myMenu->sendSelectionUp();
    else
      myMenu->sendSelectionDown();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::handleMouseEntered()
{
  setFlags(Widget::FLAG_HILITED);
  setDirty();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::handleMouseLeft()
{
  clearFlags(Widget::FLAG_HILITED);
  setDirty();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool PopUpWidget::handleEvent(Event::Type e)
{
  if(!isEnabled())
    return false;

  switch(e)
  {
    case Event::UISelect:
      handleMouseDown(0, 0, MouseButton::LEFT, 0);
      return true;
    case Event::UIUp:
    case Event::UILeft:
    case Event::UIPgUp:
      return myMenu->sendSelectionUp();
    case Event::UIDown:
    case Event::UIRight:
    case Event::UIPgDown:
      return myMenu->sendSelectionDown();
    case Event::UIHome:
      return myMenu->sendSelectionFirst();
    case Event::UIEnd:
      return myMenu->sendSelectionLast();
    default:
      return false;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::handleCommand(CommandSender* sender, int cmd, int data, int id)
{
  // Intercept all events sent through the PromptWidget
  // They're likely from our ContextMenu, indicating a redraw is required
  dialog().setDirty();

  // Pass the cmd on to our parent
  sendCommand(cmd, data, id);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::setArrow()
{
  // Small down arrow
  static constexpr std::array<uInt32, 7> down_arrow = {
    0b100000001,
    0b110000011,
    0b111000111,
    0b011101110,
    0b001111100,
    0b000111000,
    0b000010000,
  };
  // Large down arrow
  static constexpr std::array<uInt32, 10> down_arrow_large = {
    0b1000000000001,
    0b1100000000011,
    0b1110000000111,
    0b1111000001111,
    0b0111100011110,
    0b0011110111100,
    0b0001111111000,
    0b0000111110000,
    0b0000011100000,
    0b0000001000000
  };

  if(_font.getFontHeight() < 24)
  {
    _textOfs = 3;
    _arrowWidth = 9;
    _arrowHeight = 7;
    _arrowImg = down_arrow.data();
  }
  else
  {
    _textOfs = 5;
    _arrowWidth = 13;
    _arrowHeight = 10;
    _arrowImg = down_arrow_large.data();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void PopUpWidget::drawWidget(bool hilite)
{
//cerr << "PopUpWidget::drawWidget\n";
  FBSurface& s = dialog().surface();
  bool onTop = _boss->dialog().isOnTop();

  int x = _x + _labelWidth;
  int w = _w - _labelWidth;

  // Draw the label, if any
  if(_labelWidth > 0)
    s.drawString(_font, _label, _x, _y + myTextY, _labelWidth,
                 isEnabled() && onTop ? _textcolor : kColor, TextAlign::Left);

  // Draw a thin frame around us.
  s.frameRect(x, _y, w, _h, isEnabled() && hilite ? kWidColorHi : kColor);
  s.frameRect(x + w - (_arrowWidth * 2 - 2), _y + 1, (_arrowWidth * 2 - 3), _h - 2,
              isEnabled() && hilite ? kWidColorHi : kBGColorLo);

  // Fill the background
  s.fillRect(x + 1, _y + 1, w - (_arrowWidth * 2 - 1), _h - 2,
             onTop ? _changed ? kDbgChangedColor : kWidColor : kDlgColor);
  s.fillRect(x + w - (_arrowWidth * 2 - 3), _y + 2, (_arrowWidth * 2 - 5), _h - 4,
             onTop ? isEnabled() && hilite ? kWidColor : kBGColorHi : kBGColorLo);
  // Draw an arrow pointing down at the right end to signal this is a dropdown/popup
  s.drawBitmap(_arrowImg, x + w - (_arrowWidth * 1.5 - 1), _y + myArrowsY + 1,
               !(isEnabled() && onTop) ? kColor : kTextColor, _arrowWidth, _arrowHeight);

  // Draw the selected entry, if any
  const string& name = myMenu->getSelectedName();
  w -= dropDownWidth(_font);
  TextAlign align = (_font.getStringWidth(name) > w) ?
                     TextAlign::Right : TextAlign::Left;
  s.drawString(_font, name, x + _textOfs, _y + myTextY, w,
               !(isEnabled() && onTop) ? kColor : _changed ? kDbgChangedTextColor : kTextColor, align);
}
