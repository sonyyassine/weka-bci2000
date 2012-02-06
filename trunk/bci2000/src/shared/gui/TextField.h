////////////////////////////////////////////////////////////////////////////////
// $Id: TextField.h 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A GraphObject displaying a line of text.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef TEXT_FIELD_H
#define TEXT_FIELD_H

#include "GraphObject.h"

#include "Color.h"
#include <string>

class TextField : public GUI::GraphObject
{
 public:
  TextField( GUI::GraphDisplay& display, int zOrder = MessageZOrder );
  virtual ~TextField();
  // Properties
  TextField& SetText( const std::string& );
  const std::string& Text() const;
  TextField& SetTextHeight( float );
  float TextHeight() const;
  TextField& SetTextColor( RGBColor );
  RGBColor TextColor() const;
  TextField& SetColor( RGBColor );
  RGBColor Color() const;

 protected:
  // GraphObject event handlers
  virtual void OnChange( GUI::DrawContext& );
  virtual void OnPaint( const GUI::DrawContext& );

 protected:
  // We introduce a separate, protected DoPaint() function to simplify
  // implementation of a derived TextStimulus class that renders itself
  // dependent on its highlighted state.
  void DoPaint( const GUI::DrawContext&, RGBColor textColor, RGBColor backgroundColor );

 private:
  std::string mText;
  float       mTextHeight;
  RGBColor    mColor,
              mTextColor;
};

#endif // TEXT_FIELD_H

