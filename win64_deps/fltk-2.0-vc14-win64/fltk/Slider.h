//
// "$Id: Slider.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Slider value control. By default it moves vertically with the
// minimum number at the bottom. See HorizontalSlider for one that
// moves across (which is usually the default in other toolkits).
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php
//

#ifndef fltk_Slider_h
#define fltk_Slider_h

#ifndef fltk_Valuator_h
#include "Valuator.h"
#endif

namespace fltk {

class FL_API Slider : public Valuator {

  unsigned short slider_size_;
  unsigned short tick_size_;

public:

  enum { // bit flags for type():
    LINEAR		= 0,
    TICK_ABOVE		= 2,
    TICK_LEFT		= TICK_ABOVE,
    TICK_BELOW		= 4,
    TICK_RIGHT		= TICK_BELOW,
    TICK_BOTH		= TICK_ABOVE|TICK_BELOW,
    LOG			= 8
#ifdef FLTK_1_SLIDER
    // for back-compatability only
    ,VERTICAL		= 0,
    HORIZONTAL		= 1,
    FILL		= 16
#endif
  };
  bool horizontal() const {return !flag(LAYOUT_VERTICAL) || (type()&1);}
  bool log() const {return (type()&LOG)!=0;}

  void draw();
  int handle(int);

  Slider(int x,int y,int w,int h, const char *l = 0);
  static NamedStyle* default_style;

  unsigned short slider_size() const {return slider_size_;}
  void slider_size(int n) {slider_size_ = (unsigned short)n;}
  unsigned short tick_size() const {return tick_size_;}
  void tick_size(int n) {tick_size_ = (unsigned short)n;}
  
#ifdef FLTK_1_SLIDER
  // back comptability:
  Box* slider() const {return buttonbox();}
  void slider(Box* b) {buttonbox(b);}
  void slider_size(double v) {slider_size(int(v*w()));}
#endif

  //protected:

  int slider_position(double value, int w);
  double position_value(int x, int w);
  int handle(int event, const Rectangle&);
  void draw_ticks(const Rectangle&, int min_spacing);
  bool draw(const Rectangle&, Flags flags, bool slot);
};

}

#endif

//
// End of "$Id: Slider.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
