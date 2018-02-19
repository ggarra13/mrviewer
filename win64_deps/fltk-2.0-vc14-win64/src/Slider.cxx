//
// "$Id: Slider.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/Slider.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/draw.h>
#include <fltk/math.h>
#include <config.h>
#include <stdio.h>

using namespace fltk;

/*! \class fltk::Slider

  This widget contains a sliding "knob" that controls a
  single floating-point value. Moving the box all the way to the left
  or bottom sets it to the minimum(), and to the top/right to the
  maximum() value. The minimum() may be greater than the maximum()
  in case you want the larger number at the opposite end.

  See Valuator for how to set or get the value or handle callbacks
  when the value changes.

  \image html slider.gif

  The appearance of the base class may be changed in several ways:

  Setting the box() to any value other than the default of FLAT_BOX,
  as shown in the example on the bottom-left, will remove the "slot"
  and draw a box around the slider and the tick marks. The color()
  (which defaults to GRAY75) is used to fill in the area behind the
  slider and tick marks.

  You can use set_vertical() to make the slider move up/down rather
  than horizontally.

  The following bits may be or'd together and given to type():
  - fltk::Slider::TICK_ABOVE : Put tick marks above the horizontal slider.
  - fltk::Slider::TICK_LEFT : Put tick marks to the left of a vertical slider (same value as TICK_ABOVE)
  - fltk::Slider::TICK_BELOW : Put tick marks below the horizontal slider.
  - fltk::Slider::TICK_RIGHT : Put tick marks to the right of a vertical slider (same value as TICK_BELOW)
  - fltk::Slider::TICK_BOTH : Put tick marks on both sides of the slider.
  - fltk::Slider::LOG : Use a logarithimic or power scale for the slider.

  The tick marks are placed the slider_size() or more apart (they are
  also no closer than the step() value). The color of the tick marks
  is controlled by textcolor(), and the font used to draw the numbers
  is textfont() and textsize() (which defaults to 8).

  You can change the glyph() function to change how the moving part is
  drawn. The drawflags() determines what part of the slider is being
  drawn. The ScrollBar subclass turns on ALIGN_TOP/LEFT/RIGHT/BOTTOM
  to draw the buttons at the end (this is the same as Widget::default_glyph()
  takes and the default slider glyph just calls that). The moving part
  has bit 16 turned on, unless this is a fill slider in which case
  bit 16 is off. You can check the LAYOUT_VERTICAL flag to see which
  direction the slider is going in.

  \image html logslider.gif
  "Log" sliders have a non-uniform scale. This diagram shows some
  examples. The scale is truly logarithmic if both the minimum() and
  the maximum() are non-zero and have the same sign. Otherwise the
  slider position is the square root of the value (or -sqrt(-value)
  for negative values).

*/

/*! \fn void Slider::slider_size(int)

  Set the dimensions of the moving piece of slider. This is measured
  in pixels in a direction parallel to the slider's movement. The
  default value is 12.

  Setting slider_size() to zero will make the slider into a "fill"
  slider that draws a solid bar from the left/bottom to the current
  value. This is useful for indicating progress or as a output
  indicator.
*/

/*! \fn void Slider::tick_size(int)
  The slider is shrunk this many pixels from the widget's width so
  that the tick marks are visible next to it. The default value is 4.
*/

/*! This is used by subclasses to draw the slider correctly.
  Return the location of the left/top edge of a box of slider_size() would
  be if the slider is set to \a value and can move in an area
  of \a w pixels. The range() and log() settings are taken into
  account.
*/
int Slider::slider_position(double value, int w) {
  double A = minimum();
  double B = maximum();
  if (B == A) return 0;
  bool flip = B < A;
  if (flip) {A = B; B = minimum();}
  if (!horizontal()) flip = !flip;
  // if both are negative, make the range positive:
  if (B <= 0) {flip = !flip; double t = A; A = -B; B = -t; value = -value;}
  double fraction;
  if (!log()) {
    // linear slider
    fraction = (value-A)/(B-A);
  } else if (A > 0) {
    // logatithmic slider
    if (value <= A) fraction = 0;
    else fraction = (::log(value)-::log(A))/(::log(B)-::log(A));
  } else if (A == 0) {
    // squared slider
    if (value <= 0) fraction = 0;
    else fraction = sqrt(value/B);
  } else {
    // squared signed slider
    if (value < 0) fraction = (1-sqrt(value/A))*.5;
    else fraction = (1+sqrt(value/B))*.5;
  }
  if (flip) fraction = 1-fraction;
  w -= slider_size_; if (w <= 0) return 0;
  if (fraction >= 1) return w;
  else if (fraction <= 0) return 0;
  else return int(fraction*w+.5);
}

/*! This is used by subclasses to handle events correctly:
  Return the value if the left/top edge of a box of slider_size() is placed
  at \a X pixels from the edge of an area of size \a w pixels. The range()
  and log() settings are taken into account, and it also rounds the value
  to multiples of step(), using powers of 10 larger and multiples of 2 or
  5 to get the steps close to 1 pixel so the user is presented with nice
  numerical values.
*/
double Slider::position_value(int X, int w) {
  w -= slider_size_; if (w <= 0) return minimum();
  double A = minimum();
  double B = maximum();
  bool flip = B < A;
  if (flip) {A = B; B = minimum();}
  if (!horizontal()) flip = !flip;
  if (flip) X = w-X;
  double fraction = double(X)/w;
  if (fraction <= 0) return A;
  if (fraction >= 1) return B;
  // if both are negative, make the range positive:
  flip = (B <= 0);
  if (flip) {double t = A; A = -B; B = -t; fraction = 1-fraction;}
  double value;
  double derivative;
  if (!log()) {
    // linear slider
    value = fraction*(B-A)+A;
    derivative = (B-A)/w;
  } else if (A > 0) {
    // log slider
    double d = (::log(B)-::log(A));
    value = exp(fraction*d+::log(A));
    derivative = value*d/w;
  } else if (A == 0) {
    // squared slider
    value = fraction*fraction*B;
    derivative = 2*fraction*B/w;
  } else {
    // squared signed slider
    fraction = 2*fraction - 1;
    if (fraction < 0) B = A;
    value = fraction*fraction*B;
    derivative = 4*fraction*B/w;
  }
  // find nicest multiple of 10,5, or 2 of step() that is close to 1 pixel:
  if (step() && derivative > step()) {
    double w = log10(derivative);
    double l = ceil(w);
    int num = 1;
    int i; for (i = 0; i < l; i++) num *= 10;
    int denom = 1;
    for (i = -1; i >= l; i--) denom *= 10;
    if (l-w > 0.69897) denom *= 5;
    else if (l-w > 0.30103) denom *= 2;
    value = floor(value*denom/num+.5)*num/denom;
  }
  if (flip) return -value;
  return value;
}

static char* printtick(char* buffer, double v) {
  if (fabs(v)>=1) {
    sprintf(buffer, "%g", v);
    return buffer;
  } else {
    sprintf(buffer, "%.3g", v);
    char* p = buffer;
    if (v < 0) p++;
    while (p[0]=='0' && p[1]) p++;
    if (v < 0) *--p = '-';
    return p;
  }
}

/*! Draw tick marks. These lines cross the passed rectangle perpendicular to
  the slider direction. In the direction parallel to the slider direction
  the box should have the same size as the area the slider moves in. */
void Slider::draw_ticks(const Rectangle& r, int min_spacing)
{
  int x1, sx1, y1, sy1, x2, y2, dx, dy, w;
  if (horizontal()) {
    sx1 = x1 = x2 = r.x()+(slider_size_-1)/2; dx = 1;
    y1 = r.y(); y2 = r.b()-1; dy = 0;
    sy1 = y1+1+r.h()/4;
    w = r.w();
  } else {
    x1 = r.x(); x2 = r.r()-1; dx = 0;
    sx1 = x1+1+r.w()/4;
    sy1 = y1 = y2 = r.y()+(slider_size_-1)/2; dy = 1;
    w = r.h();
  }
  if (w <= 0) return;
  double A = minimum();
  double B = maximum();
  if (A > B) {A = B; B = minimum();}
  //if (!finite(A) || !finite(B)) return;

  if (min_spacing < 1) min_spacing = 10; // fix for fill sliders
  // Figure out approximate size of min_spacing at zero:

  double mul = 1; // how far apart tick marks are
  double div = 1;
  int smallmod = 5; // how many tick marks apart "larger" ones are
  int nummod = 10; // how many tick marks apart numbers are
  int powincr = 10000;

  if (!log()) {
    double derivative = (B-A)*min_spacing/w;
    if (derivative < step()) derivative = step();
    while (mul*5 <= derivative) mul *= 10;
    while (mul > derivative*2*div) div *= 10;
    if (derivative*div > mul*2) {mul *= 5; smallmod = 2;}
    else if (derivative*div > mul) {mul *= 2; nummod = 5;}
  } else if (A > 0) {
    // log slider
    while (mul*5 <= A) mul *= 10;
    while (mul > A*2*div) div *= 10;
    powincr = 10;
    double d = exp(min_spacing*::log(B/A)/w*3);
    if (d >= 5) {mul *= 10; smallmod = nummod = 1; powincr = 1;}
    else if (d >= 2) {mul *= 5; smallmod = powincr = nummod = 2;}
  } else {
    // squared slider, derivative at edge is zero, use value at 1 pixel
    double derivative = B*min_spacing*min_spacing/(w*w);
    if (A < 0) derivative *= 4;
    if (derivative < step()) derivative = step();
    while (mul < derivative) mul *= 10;
    while (mul >= 10*derivative*div) div *= 10;
    powincr = 10;
    //if (derivative > num) {num *= 5; smallmod = powincr = nummod = 2;}
  }

  Color textcolor = this->textcolor();
  Color linecolor = lerp(getbgcolor(), textcolor, .66666f);
  setcolor(linecolor);
  setfont(textfont(), textsize());
  float yt = horizontal() ? y1+getsize()-getdescent() : y1-1;
  double v; char buffer[20]; char* p; int t; float x, y;
  for (int n = 0; ; n++) {
    // every ten they get further apart for log slider:
    if (n > powincr) {mul *= 10; n = (n-1)/10+1;}
    v = mul*n/div;
    if (v >= fabs(A) && v >= fabs(B)) break;
    if (n%smallmod) {
      if (v > A && v < B) {
        t = slider_position(v, w);
        drawline(sx1+dx*t, sy1+dy*t, x2+dx*t, y2+dy*t);
      }
      if (v && -v > A && -v < B) {
        t = slider_position(-v, w);
        drawline(sx1+dx*t, sy1+dy*t, x2+dx*t, y2+dy*t);
      }
    } else {
      if (v > A && v < B) {
        t = slider_position(v, w);
        drawline(x1+dx*t, y1+dy*t, x2+dx*t, y2+dy*t);
        if (n%nummod == 0) {
          p = printtick(buffer, v);
          x = x1+dx*t+1;
          y = yt+dy*t;
          if (dx && (x < r.x()+3*min_spacing || x >= r.r()-5*min_spacing));
          else if (dy && (y < r.y()+5*min_spacing || y >= r.b()-3*min_spacing));
          else {
            setcolor(textcolor);
            drawtext(p, x,y);
            setcolor(linecolor);
          }
        }
      }
      if (v && -v > A && -v < B) {
        t = slider_position(-v, w);
        drawline(x1+dx*t, y1+dy*t, x2+dx*t, y2+dy*t);
        if (n%nummod == 0) {
          p = printtick(buffer, v);
          x = x1+dx*t+1;
          y = yt+dy*t;
          if (dx && (x < r.x()+3*min_spacing || x >= r.r()-5*min_spacing));
          else if (dy && (y < r.y()+5*min_spacing || y >= r.b()-3*min_spacing));
          else {
            setcolor(textcolor);
            drawtext(p, x,y);
            setcolor(linecolor);
          }
        }
      }
    }
  }

  // draw the end ticks with numbers:

  v = minimum();
  t = slider_position(v, w);
  drawline(x1+dx*t, y1+dy*t, x2+dx*t, y2+dy*t);
  p = printtick(buffer, v);
  x = x1+dx*t+1;
  y = yt+dy*t;
  setcolor(textcolor);
  drawtext(p, x,y);
  setcolor(linecolor);

  v = maximum();
  t = slider_position(v, w);
  drawline(x1+dx*t, y1+dy*t, x2+dx*t, y2+dy*t);
  p = printtick(buffer, v);
  x = x1+dx*t+1;
  if (dx) {float w = getwidth(p); if (x+w > r.r()) x -= 2+w;}
  y = yt+dy*t;
  if (dy) y += getsize();
  setcolor(textcolor);
  drawtext(p, x,y);
}

void Slider::draw()
{
  Flags flags = this->flags();
  Flags f2 = flags & ~FOCUSED;
  if (pushed()) f2 |= PUSHED;
  flags &= ~HIGHLIGHT;

  Box* box = this->box();
  if (!box->fills_rectangle()) draw_background();
  drawstyle(style(),flags);
  Rectangle r(w(),h());
  box->draw(r);

  // we draw the slot if box() has a zero-sized border:
  Rectangle r1(r); box->inset(r1);
  draw(r1, f2, r1.y()==0);

  // draw the focus indicator inside the box:
  drawstyle(style(),flags);
  box->draw_symbol_overlay(r);
}

/*!
  Subclasses can use this to redraw the moving part.
  Draw everything that is inside the box, such as the tick marks,
  the moving slider, and the "slot". The slot only drawn if \a slot
  is true. You should already have drawn the background of the slider.
*/

bool Slider::draw(const Rectangle& sr, Flags flags, bool slot)
{
  // for back compatability, use type flag to set slider size:
  if (type()&16/*FILL*/) slider_size(0);

  Rectangle r = sr;
  const int slot_size_ = 3; // was 6, really should be a preference?

  // draw the tick marks and inset the slider drawing area to clear them:
  if (tick_size_ && (type()&TICK_BOTH)) {
    Rectangle tr = r;
    if (horizontal()) {
      r.move_b(-tick_size_);
      switch (type()&TICK_BOTH) {
      case TICK_BOTH:
        r.y(r.y()+tick_size_/2);
        break;
      case TICK_ABOVE:
        r.y(r.y()+tick_size_);
        tr.set_b(r.center_y());
        break;
      case TICK_BELOW:
        tr.set_y(r.center_y()+(slot?slot_size_/2:0));
        break;
      }
    } else {
      r.move_r(-tick_size_);
      switch (type()&TICK_BOTH) {
      case TICK_BOTH:
        r.x(r.x()+tick_size_/2);
        break;
      case TICK_ABOVE:
        r.x(r.x()+tick_size_);
        tr.set_r(r.center_x());
        break;
      case TICK_BELOW:
        tr.set_x(r.center_x()+(slot?slot_size_/2:0));
        break;
      }
    }
    draw_ticks(tr, (slider_size()+1)/2);
  }

  if (slot) {
    Rectangle sl;
    int dx = (slider_size_-slot_size_)/2; if (dx < 0) dx = 0;
    if (horizontal()) {
      sl.x(dx+r.x());
      sl.w(r.w()-2*dx);
      sl.y(r.y()+(r.h()-slot_size_+1)/2);
      sl.h(slot_size_);
    } else {
      sl.y(dx+r.y());
      sl.h(r.h()-2*dx);
      sl.x(r.x()+(r.w()-slot_size_+1)/2);
      sl.w(slot_size_);
    }
    setbgcolor(BLACK);
    THIN_DOWN_BOX->draw(sl);
  }

  drawstyle(style(),flags|OUTPUT);
  // if user directly set selected_color we use it:
  if (style()->selection_color_) {
    setbgcolor(style()->selection_color_);
    setcolor(contrast(selection_textcolor(), style()->selection_color_));
  }

  // figure out where the slider should be:
  Rectangle s(r);
  int sglyph = 0; // draw our special glyph
  if (horizontal()) {
    s.x(r.x()+slider_position(value(),r.w()));
    s.w(slider_size_);
    if (!s.w()) { // fill slider
      s.w(s.x()-r.x());
      s.x(r.x());
      sglyph=ALIGN_CENTER; // stops it from drawing divider line
    }
  } else {
    s.y(r.y()+slider_position(value(),r.h()));
    s.h(slider_size_);
    if (!s.h()) { // fill slider
      s.h(r.b()-s.y());
      sglyph=ALIGN_CENTER; // stops it from drawing divider line
    }
  }
  draw_glyph(sglyph, s); // draw slider in new position
  return true;
}

/*! This call is provied so subclasses can draw the moving part inside
  an arbitrary rectangle. */
int Slider::handle(int event, const Rectangle& r) {

  switch (event) {
  case FOCUS:
  case UNFOCUS:
    redraw(DAMAGE_ALL);
    return 1;
  case PUSH:
    redraw(DAMAGE_HIGHLIGHT);
    handle_push();
  case DRAG: {
    // figure out the space the slider moves in and where the event is:
    int w,mx;
    if (horizontal()) {
      w = r.w();
      mx = event_x()-r.x();
    } else {
      w = r.h();
      mx = event_y()-r.y();
    }
    if (w <= slider_size_) return 1;
    static int offcenter;
    int X = slider_position(value(), w);
    if (event == PUSH) {
      offcenter = mx-X;
      // we are done if they clicked on the slider:
      if (offcenter >= (slider_size() ? 0 : -8) && offcenter <= slider_size_)
	return 1;
      if (event_button() > 1) {
	// Move the near end of the slider to the cursor. This is good
	// for scrollbars.
	offcenter = (offcenter < 0) ? 0 : slider_size_;
      } else {
	// Center the slider under the cursor, what most toolkits do
	offcenter = slider_size_/2;
      }
    }
    double v;
  RETRY:
    X = mx-offcenter;
    if (X < 0) {
      X = 0;
      offcenter = mx; if (offcenter < 0) offcenter = 0;
    } else if (X > (w-slider_size_)) {
      X = w-slider_size_;
      offcenter = mx-X; if (offcenter > slider_size_) offcenter = slider_size_;
    }
    v = position_value(X, w);
    handle_drag(v);
    // make sure a click outside the sliderbar moves it:
    if (event == PUSH && value() == previous_value()) {
      offcenter = slider_size_/2;
      event = DRAG;
      goto RETRY;
    }
    return 1;}
  case RELEASE:
    handle_release();
    redraw(DAMAGE_HIGHLIGHT);
    return 1;
  case KEY:
    // Only arrows in the correct direction are used.  This allows the
    // opposite arrows to be used to navigate between a set of parellel
    // sliders.
    switch (event_key()) {
    case UpKey:
    case DownKey:
      if (horizontal()) return 0;
      break;
    case LeftKey:
    case RightKey:
      if (!horizontal()) return 0;
    }
  default:
    return Valuator::handle(event);
  }
}

int Slider::handle(int event) {
  Rectangle r(w(),h());
  box()->inset(r);
  return handle(event,r);
}

static class SliderGlyph : public Symbol {
public:
  void _draw(const Rectangle& rr) const {

    // this stops the scroller from being pushed-in, while leaving the flag
    // set so that an alternative glyph can draw differently when pushed:
    if (!(drawflags()&15)) setdrawflags(drawflags()&~PUSHED);

    // See if anything other than the slider is being drawn, if so
    // call the Widget::default_glyph:
    if (drawflags()&31) {
      Widget::default_glyph->draw(rr);
      return;
    }

    Symbol* box = drawstyle()->buttonbox();
    box->draw(rr);
    Rectangle r = rr; box->inset(r);

    // draw the divider line into slider:
    if (r.w() < 4 || r.h() < 4) return;
    if (!(drawflags()&LAYOUT_VERTICAL)) { // horizontal
      int x = r.x()+(r.w()-1)/2;
      setcolor(GRAY33);
      drawline(x, r.y(), x, r.b());
      setcolor(GRAY99);
      drawline(x+1, r.y(), x+1, r.b());
    } else { // vertical
      int y = r.y()+r.h()/2;
      setcolor(GRAY33);
      drawline(r.x(), y, r.r(), y);
      setcolor(GRAY99);
      drawline(r.x(), y+1, r.r(), y+1);
    }
  }
  SliderGlyph() : Symbol() {}
} glyph;

static void revert(Style *s) {
  s->color_ = GRAY75;
  //s->textcolor_ = GRAY15;
  s->textsize_ = 8;
  s->box_ = FLAT_BOX;
  s->glyph_ = &glyph;
}
static NamedStyle style("Slider", revert, &Slider::default_style);
NamedStyle* Slider::default_style = &::style;

Slider::Slider(int x, int y, int w, int h, const char* l)
: Valuator(x, y, w, h, l) {
  style(default_style);
  tick_size_ = 4;
  slider_size_ = 12;
  //set_vertical();
}

//
// End of "$Id: Slider.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
