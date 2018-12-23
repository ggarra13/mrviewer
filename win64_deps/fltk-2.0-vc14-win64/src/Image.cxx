// "$Id: Image.cxx 8748 2011-05-28 04:47:50Z bgbnbigben $"
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

#include <config.h>
#include <fltk/Image.h>
#include <fltk/Widget.h>
#include <fltk/events.h>
#include <fltk/draw.h>
#include <fltk/x.h>

/*! \class fltk::Image

  A rectangular buffer of pixels that can be efficiently drawn on the
  screen. The draw() functions will copy (or "over" composite if there
  is alpha in the pixeltype()) onto the output, transformed by the
  current transform.

  NOTE: If you already have a set of pixels sitting in your own memory,
  drawimage() can draw it and is \e much easier to use. You should use
  this class \e only if you will be drawing the \e same image multiple
  times, with no changes to the pixels.

  The buffer is created and filled in by setting the type of pixels
  with setpixeltype(), the size with setsize(), and then calling
  buffer() (note that setpixels() calls buffer() for you). The initial
  buffer is filled with undefined contents.

  The best way to put data into the buffer is to make one or more
  calls to setpixels(), to replace rectangular regions.

  You can directly address the buffer() to read and write the
  pixels. The size of the buffer is in buffer_width() and
  buffer_height() (this may be much larger than width() and height())
  and the distance between lines is in buffer_linedelta(). If you
  change any pixels you should call buffer_changed() before the
  next draw().

  Due to operating system limitations, buffer() is usually not an
  array of pixeltype() pixels. Instead setpixels() converts pixels
  into a type the operating system can use. The type of pixels in the
  buffer is retured by buffer_pixeltype(). This is really
  inconvienent, so you can also call the method
  force_ARGB32_on(). This will cause buffer_pixeltype() to return
  ARGB32, so you can assume this at compile time. The implementation
  of Image may be less efficient (actually the overhead is zero on
  Windows and close to zero on most other systems)

  If buffer() has not been called, draw() will call the
  fetch() virtual method. It should call setpixeltype(), setsize() and
  setpixels(). This is used to defer reading image files or
  decompressing data until needed.  fetch() will also restore the
  buffer contents to the original values if you have written to the
  buffer. If fetch() does not allocate a buffer, draw() will draw a
  solid rectangle in the current color.

  Because Image is a subclass of Symbol, it may be used as a
  Widget::image() or as the box() in a Style. If you give it a name it
  can be drawn with "@name;" in a label. If resized, the Symbol
  _draw() method will use the inset() call to decide on edge
  thicknesses and will dice the image up into 9 individually-scaled
  pieces, which is very useful for GUI buttons and background images
  (this is similar to how Flash draws buttons).

  There are a number of subclasses such as jpgImage and pngImage that
  display compressed image data, either from in-memory data buffers or
  from files.
*/

using namespace fltk;

unsigned long Image::memused_;

#if USE_CAIRO || DOXYGEN

// Make the fltk::Picture be a cairo_surface_t:
#define PICTURE ((cairo_surface_t*)picture)
#define SET_PICTURE(x) picture = (fltk::Picture*)x
// special flag to indicate if setimage was used:
#define COPIED_DATA 32

// Return the Cairo image type we will use for the given fltk image type
static cairo_format_t cairo_format(PixelType type) {
  switch (type) {
  case MASK:
    return CAIRO_FORMAT_A8;
  case MONO:
  case RGBx:
  case RGB:
  case RGB32:
    return CAIRO_FORMAT_RGB24;
  case RGBA:
  case ARGB32:
  case RGBM:
  case MRGB32:
  default:
    return CAIRO_FORMAT_ARGB32;
  }
}

// Convert fltk pixeltypes to the Cairo image type
// Go backwards so the buffers can be shared
static void convert(uchar* to, const uchar* from, PixelType type, int w) {
  U32* t = (U32*)to;
  switch (type) {
  case MONO:
    t += w;
    from += w;
    while (t > (U32*)to)
      *--t = 0x1010101 * *--from;
    break;
  case MASK:
    // fltk mask was designed for black letter images and is therefore
    // inverted from the more sensible usage that Cairo has.
    while (w--) *to++ = ~*from++;
    break;
  case RGBx:
    t += w;
    from += 4*w;
    while (t > (U32*)to) {
      from -= 4;
      *--t = 0xff000000 | (from[0]<<16) | (from[1]<<8) | from[2];
    }
    break;
  case RGB:
    t += w;
    from += 3*w;
    while (t > (U32*)to) {
      from -= 3;
      *--t = 0xff000000 | (from[0]<<16) | (from[1]<<8) | from[2];
    }
    break;
  case RGBA:
  case RGBM:
    t += w;
    from += 4*w;
    while (t > (U32*)to) {
      from -= 4;
      *--t = (from[3]<<24) | (from[0]<<16) | (from[1]<<8) | from[2];
    }
    break;
  case RGB32:
  case ARGB32:
  case MRGB32:
  default:
    if (from != to) memcpy(to, from, 4*w);
    break;
  }
}

/**
  Return the width in pixels of buffer().
*/
int Image::buffer_width() const {
  if (picture) return cairo_image_surface_get_width(PICTURE);
  return w();
}

/**
  Return the height in pixels of buffer();
*/
int Image::buffer_height() const {
  if (picture) return cairo_image_surface_get_height(PICTURE);
  return h();
}

/**
  Return the distance between each row of pixels in buffer().
*/
int Image::buffer_linedelta() const {
  if (picture) return cairo_image_surface_get_stride(PICTURE);
  return (w_*depth()+3)&-4;
}

/**
  Returns the number of bytes per pixel stored in buffer(). This
  is the same as ::depth(buffer_pixeltype()).
*/
int Image::buffer_depth() const {
  switch (pixeltype_) {
  case MASK:
    return 1;
  default:
    return 4;
  }
}

void Image::set_forceARGB32() {
  flags |= FORCEARGB32;
  // It pretty much does this already except for mono
}

void Image::clear_forceARGB32() {
  flags &= ~FORCEARGB32;
}

/**
  Return the type of pixels stored in buffer().  Likely to be
  ARGB32. On older (non-XRender) X system the types 1 and 2 indicate 1
  and 2-byte data, but there is no api to figure out anything more
  about this data.
*/
fltk::PixelType Image::buffer_pixeltype() const {
  switch (pixeltype_) {
  case MASK:
    return MONO;
  case MONO:
  case RGBx:
  case RGB:
  case RGB32:
    return RGB32;
  case RGBA:
  case ARGB32:
  default:
    return ARGB32;
  case RGBM:
  case MRGB32:
    return MRGB32;
  }
}

/**
  Returns how much memory the image is using for buffer() and for
  any other structures it created. Returns zero if buffer() has
  not been called.
*/
unsigned long Image::mem_used() const {
  if (flags & COPIED_DATA)
    return cairo_image_surface_get_stride(PICTURE)*cairo_image_surface_get_height(PICTURE);
  return 0;
}

/**
  This just does a const-cast and calls the non-const version.
*/
const uchar* Image::buffer() const {
  return const_cast<Image*>(this)->buffer();
}

/**
  Creates (if necessary) and returns a pointer to the internal pixel
  buffer. This is probably going to be shared memory with the graphics
  system, it may have a different pixeltype, size, and linedelta than
  the Image. If you are able to figure out the type you can read and
  write the pixels directly.
*/
uchar* Image::buffer() {
  if (!picture) {
    SET_PICTURE(cairo_image_surface_create(cairo_format(pixeltype_), w_, h_));
    flags |= COPIED_DATA;
    memused_ += mem_used();
  }
  return cairo_image_surface_get_data(PICTURE);
}

/**
  Destroys the buffer() and any related system structures.
*/
void Image::destroy() {
  if (!picture) return;
  memused_ -= mem_used();
  cairo_surface_destroy(PICTURE);
  picture = 0;
  flags &= ~(FETCHED|COPIED_DATA);
}

/**
  Change the stored pixeltype. If it is not compatable then
  destroy() is called.
*/
void Image::setpixeltype(PixelType p) {
  if (picture && cairo_format(p) != cairo_image_surface_get_format(PICTURE))
    destroy();
  pixeltype_ = p;
}

/**
  Change the size of the stored image. If it is not compatable with
  the current data size (generally if it is larger) then destroy()
  is called.
*/
void Image::setsize(int w, int h) {
  if (picture && (w > buffer_width() || h > buffer_height())) destroy();
  w_ = w;
  h_ = h;
}

/**
  Return a pointer to a buffer that you can write up to width() pixels
  in pixeltype() to and then call setpixels(buffer,y) with. This can
  avoid doing any copying of the data if the internal format and
  pixeltype() are compatable, because it will return a pointer
  directly into the buffer and setpixels will detect this and do
  nothing.
*/
uchar* Image::linebuffer(int y) {
  buffer();
  return cairo_image_surface_get_data(PICTURE) + y*cairo_image_surface_get_stride(PICTURE);
}

/**
  Replace the given rectangle of buffer() with the supplied data,
  which must be in the pixeltype(). \a linedelta is the distance
  between each row of pixels in \a data. The rectangle is assummed
  to fit inside the width() and height().
*/
void Image::setpixels(const uchar* buf, const fltk::Rectangle& r, int linedelta)
{
  if (r.empty()) return;
  flags &= ~COPIED;
  uchar* to = linebuffer(r.y()) + r.x()*buffer_depth();
  // see if we can do it all at once:
  if (r.w() == buffer_width() && (r.h()==1 || linedelta == buffer_linedelta())) {
    convert(to, buf, pixeltype_, r.w()*r.h());
  } else {
    for (int y = 0; y < r.h(); y++) {
      convert(to, buf, pixeltype_, r.w());
      buf += linedelta;
      to += buffer_linedelta();
    }
  }
}

/** \fn void Image::setpixels(const uchar* data, const fltk::Rectangle& r)
  Figures out the linedelta for you as depth()*r.w().
*/

/**
  Same as setpixels(data,Rectangle(0,y,width(),1)), sets one entire
  row of pixels.
*/
void Image::setpixels(const uchar* buf, int y) {
  convert(linebuffer(y), buf, pixeltype_, width());
  flags &= ~COPIED;
}

void Image::fetch_if_needed() const {
  if (!(flags&FETCHED)) {
    Image* thisimage = const_cast<Image*>(this);
    thisimage->fetch();
    thisimage->flags |= FETCHED;
  }
}

extern void fl_set_cairo_ctm();

/**
  Draws the subrectangle \a from of the image, transformed to fill
  the rectangle \a to (as transformed by the CTM). If the image has
  an alpha channel, an "over" operation is done.

  Due to lame graphics systems, this is not fully operational on all
  systems:
  * X11 without XRender extension: no transformations are done, the
  image is centered in the output area.
  * X11 with XRender: rotations fill the bounding box of the destination
  rectangle, drawing extra triangular areas outside the source rectangle.
  Somewhat bad filtering when making images smaller. xbmImage does
  not transform.
  * Windows: Only scaling, no rotations. Bad filtering. xbmImage does
  not do any transformations.
  * OS/X: works well in all cases.
*/
void Image::draw(const fltk::Rectangle& from, const fltk::Rectangle& to) const {
  fetch_if_needed();
  if (!picture) {fillrect(to); return;}
  cairo_save(cr);
  fl_set_cairo_ctm();
  cairo_translate(cr, to.x(), to.y());
  cairo_scale(cr, double(to.w())/from.w(), double(to.h())/from.h());
  cairo_rectangle(cr, 0, 0, from.w(), from.h());
  switch (pixeltype_) {
  case MASK:
    cairo_clip(cr);
    cairo_mask_surface(cr, PICTURE, -from.x(), -from.y());
    break;
  case RGBM:
  case MRGB32:
    cairo_clip(cr);
    cairo_set_source_surface(cr, PICTURE, -from.x(), -from.y());
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_mask_surface(cr, PICTURE, -from.x(), -from.y());
    break;
  default:
    cairo_set_source_surface(cr, PICTURE, -from.x(), -from.y());
    cairo_fill(cr);
    break;
  }
  cairo_restore(cr);
}

/**
  This is equivalent to:
\code
  setsize(w, h);
  setpixeltype(p);
  setpixels(source, Rectangle(w,h), linedelta);
\endcode
  except, if possible, \a source is used as buffer() (throwing
  away the const!). This will happen if the pixeltype and linedelta
  are of types that it can handle unchanged and if the image memory
  does not need to be allocated by the system.
*/
void Image::setimage(const uchar* source, PixelType p, int w, int h, int ld)
{
  setsize(w,h);
  setpixeltype(p);
  // see if we can directly use the memory:
  if (p == ARGB32 || p == RGB32) {
    cairo_surface_t* new_picture =
      cairo_image_surface_create_for_data((unsigned char*)source, cairo_format(p), w, h, ld);
    // check for no errors due to alignment or stride:
    if (cairo_surface_status(new_picture) == 0) {
      destroy();
      SET_PICTURE(new_picture);
      flags |= FETCHED;
      return;
    }
    cairo_surface_destroy(new_picture);
  }
  // copy the data to a normal cairo_image:
  if (!(flags & COPIED_DATA)) destroy();
  setpixels(source, Rectangle(w, h), ld);
  flags |= FETCHED;
}

/*! \fn void Image::setimage(const uchar* d, PixelType p, int w, int h)
  Figures out linedelta for you as w*depth(p).
*/

void Image::make_current() {
  // TODO - make the cr point at the image_surface!
}

// This function is provided by some backends, and does direct-draw
// from a memory buffer without creating an Image. Returns true if
// it can do it. Returns false if a temporary Image object should be
// used instead to emulate it.
static inline bool innards(const uchar *buf, fltk::PixelType type,
                           const fltk::Rectangle& r1,
                           int linedelta,
                           DrawImageCallback cb, void* userdata)
{
  return false;
}

#elif USE_X11
# include "x11/Image.cxx"
#elif defined(_WIN32)
# include "win32/Image.cxx"
#elif USE_QUARTZ
# include "osx/Image.cxx"
#else
# error
#endif

/*! \fn Image::Image(const char* name)
  The default constructor sets pixeltype() to RGB32 (0x00rrggbb) and
  width() and height() to 12. This means that 12x12 square with the current
  color will be drawn if not able to draw anything else.

  The optional name is passed to the Symbol constructor and allows
  the image to be drawn by putting "@name;" into a label.
*/

/*! \fn Image::Image(int w, int h, const char* name)
  Does setsize(w,h). This causes the width() and height() to return
  the passed values. No buffer is allocated, call buffer() to do that.
  The pixeltype() is set to RGB32 (0x00rrggbb).
*/

/*! \fn Image::Image(PixelType p, int w, int h, const char* name)
  Does pixeltype(p) and setsize(w,h). No buffer is allocated, call
  buffer() to do that.
*/

/*! \fn Image::Image(const uchar* data, PixelType, int w, int h, int linedelta)
  Initializes the size and pixels by doing setimage(). Note that
  the pointer \a data may be used unchanged. If you want to force
  a copy you should use setpixels() instead.
*/

/*! \fn Image::Image(const uchar* data, PixelType, int w, int h)
  Initializes the size and pixels by doing setimage(). Note that
  the pointer \a data may be used unchanged. If you want to force
  a copy you should use setpixels() instead.
*/

/*! \fn PixelType Image::pixeltype() const
  Return the type of pixels that are put into the image with
  setpixels(). You can change this with setpixeltype(). It is
  possible the internal data is in a different type, use
  buffer_pixeltype() to find out what that is.
*/

/*! \fn int Image::depth() const
  Same as ::depth(pixeltype()), this returns how many bytes
  each pixel takes in the buffer sent to setpixels().
*/

/*! \fn int Image::width() const
  Return the width of the image in pixels. You can change this with
  setsize().
*/

/*! \fn int Image::height() const
  Return the height of the image in pixels. You can change this with
  setsize().
*/

/*! \fn void Image::buffer_changed()
  Call this if you modify the contents of buffer(). On some systems
  the memory is not actually shared with the window system, and this
  will cause draw() to copy the buffer to the system's memory.
  setpixels() calls this for you.
*/

/*! The destructor calls destroy() */
Image::~Image() {
  destroy();
}

/*!
  This is called by the draw() functions once after the Image is
  created or after refetch() has been called.  This allows subclasses
  to defer reading files and calling setpixels() calls until the first
  draw() or measure(). This should return true if successful, false on
  any error (though fltk does not do anything useful with errors).

  The base class does nothing and returns true, thus leaving the
  image unchanged.

  Sample implementation:
\code
  bool MyImage::fetch() {
    setsize(get_width(file), get_height(file));
    setpixeltype(my_pixeltype);
    for (int y=0; y<height(); y++) {
      uchar* buffer = linebuffer(y);
      get_line_of_pixels(file, buffer, y);
      setpixels(buffer, y);
    }
    return true;
  }
\endcode
*/
bool Image::fetch() {return true;}

/*! \fn void Image::refetch()
  Cause fetch() to be called again. This is useful for a file image
  if the file name or contents have changed.
*/

/*! \fn void Image::fetch_if_needed() const
  Call fetch() if it has not been called or if refetch() was called.
*/

/**
  Returns width() and height().

  The default constructor sets an internal flag that indicates that
  fetch() must be called before the width() and height() are known.
  This is useful for subclasses that read a file and figure out both
  the size and pixels at the same time.
*/
void Image::_measure(int& W, int& H) const {
  if (flags & MEASUREFETCH)
    fetch_if_needed();
  W = w_;
  H = h_;
}

/*! \fn bool Image::fills_rectangle() const
  Returns true if the pixeltype does not support alpha.
*/
bool Image::fills_rectangle() const {
  return
    pixeltype_ != MASK &&
    pixeltype_ != RGBA &&
    pixeltype_ < ARGB32;
}

#include <fltk/Widget.h>

/**
  This is a 1.1 back-compatability function. It is the same as
  doing widget->image(this) and widget->label(0).
*/
void Image::label(Widget* o) {
  o->image(this);
  o->label(0);
}

/**
  Does measure() and then draw(Rectangle(0,0,w,h),Rectangle(x,y,w,h).
  Thus the top-left corner is placed at x,y and no scaling (other
  than due to the current transformation) is done.
*/
void Image::draw(int x, int y) const {
  int w,h; measure(w,h);
  draw(fltk::Rectangle(0,0,w,h), fltk::Rectangle(x,y,w,h));
}

/**
  Cubes an image and draws the 4 corners.
  It then expands the middle of the cube as far as needed, whilst
  only expanding the top and side edges horizontally/vertically as needed.
  With an image that has pixels set out as such:
  ABA
  CDC
  ABA
  The final image looks as follows, and is expanded to fit the required rectangle:
  AABBBBBAA
  AABBBBBAA
  CCDDDDDCC
  CCDDDDDCC
  CCDDDDDCC
  CCDDDDDCC
  CCDDDDDCC
  AABBBBBAA
  AABBBBBAA
  This function is mostly called from within FLTK's themes.
*/
void Image::draw_diced(const fltk::Rectangle& R) {
//  fetch_if_needed();
  int w, h; measure(w, h);
  int cw = w / 3;            // Corner width 
  int lw = w - 2*cw;         // Top width
  int ch = h / 3;            // Corner height
  int lh = h - 2*ch;         // Top height
  int dx = R.x();
  int dy = R.y();
  int dlw = R.w() - 2*cw;
  int dlh = R.h() - 2*ch;
  draw(fltk::Rectangle(0, 0, cw, ch), 			fltk::Rectangle(dx, dy, cw, ch));
  draw(fltk::Rectangle(cw, 0, lw, ch),			fltk::Rectangle(dx + cw, dy, dlw, ch));
  draw(fltk::Rectangle(cw + lw, 0, cw, ch), 		fltk::Rectangle(dx + cw + dlw, dy, cw, ch));
  draw(fltk::Rectangle(0, ch, cw, lh),			fltk::Rectangle(dx, dy + ch, cw, dlh));
  draw(fltk::Rectangle(cw + lw, ch, cw, lh),		fltk::Rectangle(dx + cw + dlw, dy + ch, cw, dlh));
  draw(fltk::Rectangle(0, ch + lh, cw, ch),		fltk::Rectangle(dx, dy + ch + dlh, cw, ch));
  draw(fltk::Rectangle(cw + lw, ch + lh, cw, ch), 	fltk::Rectangle(dx + cw + dlw, dy + ch + dlh, cw, ch));
  draw(fltk::Rectangle(cw, ch + lh, lw, ch), 		fltk::Rectangle(dx + cw, dy + ch + dlh, dlw, ch));
  draw(fltk::Rectangle(cw, ch, lw, lh), 		fltk::Rectangle(dx + cw, dy + ch, dlw, dlh));
};


/**
  Resizes the image to fit in the rectangle. This is the virtual
  method from the Symbol base class, so this is what is called if
  the image is used as a label or box type.

  If the destination rectangle is not the same size, inset() is
  used to figure out the edge thicknesses. The image is then
  diced into 9 rectangles in a 3x3 grid by the insets, and
  each piece is scaled individually. This is very useful for
  scaling paintings of buttons. Note that if the insets are zero
  (the default) then the whole image is scaled as one piece.
  If you want, inset() can return different thicknesses depending
  on the size, producing very interesting scaling.

  It is possible this will use drawflags(INACTIVE) to gray out
  the image in a system-specific way. NYI.
*/
void Image::_draw(const fltk::Rectangle& r) const
{
  if (r.empty()) return;
  int w,h; measure(w,h);
  // quickly handle no-scaling:
  if (r.w()==w && r.h()==h) {
  NOINSETS:
    draw(fltk::Rectangle(0,0,w,h), r);
    return;
  }
  // now check the insets and use them to scale pieces individually:
  fltk::Rectangle in(0,0,w,h); this->inset(in);
  if (!in.x() && !in.y() && in.w()==w && in.h()==h) goto NOINSETS;
  int fx[4]; fx[0]=0; fx[1]=in.x(); fx[2]=in.r(); fx[3]=w;
  int fy[4]; fy[0]=0; fy[1]=in.y(); fy[2]=in.b(); fy[3]=h;
  fltk::Rectangle out(r); this->inset(out);
  int tx[4]; tx[0]=r.x(); tx[1]=out.x(); tx[2]=out.r(); tx[3]=r.r();
  int ty[4]; ty[0]=r.y(); ty[1]=out.y(); ty[2]=out.b(); ty[3]=r.b();
  for (int y=0; y<3; y++) if (fy[y+1]>fy[y] && ty[y+1]>ty[y]) {
    for (int x=0; x<3; x++) if (fx[x+1]>fx[x] && tx[x+1]>tx[x]) {
      draw(fltk::Rectangle(fx[x],fy[y],fx[x+1]-fx[x],fy[y+1]-fy[y]),
	   fltk::Rectangle(tx[x],ty[y],tx[x+1]-tx[x],ty[y+1]-ty[y]));
    }
  }
}

/*! \fn unsigned long Image::total_mem_used()
  Sum of all mem_used() calls to all Images.  This is used by
  SharedImage to decide when to clear out cached images.
*/

////////////////////////////////////////////////////////////////
// drawimage()

static Image* reused_image;

/**
  Draw a image (a rectangle of pixels) stored in your program's
  memory. The current transformation (scale, rotate) is applied.

  - \a pointer points at the first byte of the top-left pixel.
  - \a type describes how to interpret the bytes of each pixel.
  - \a rectangle: the image is put in the top-left corner
  and the width and height declare how many pixels the image has.
  - \a line_delta is how much to add to \a pointer to go 1 pixel down.
  By setting \a line_delta to larger than \a depth(type)*r.w() you can
  crop a picture out of a larger buffer. You can also set it negative
  for images that are stored with bottom-to-top in memory,
  notice that in this case \a pointer still points at the top-left
  pixel, which is at the \e end of your buffer minus one line_delta.

  The X version of FLTK will abort() if the default visual is one
  it cannot use for images. To avoid this call fltk::visual(fltk::RGB)
  at the start of your program.
*/
void fltk::drawimage(const uchar* pointer, fltk::PixelType type,
		     const Rectangle& r,
		     int line_delta) {
  if (innards(pointer, type, r, line_delta, 0, 0)) return;
  // Fake it using a temporary Image
  if (!reused_image) reused_image = new Image();
  reused_image->setimage(pointer, type, r.w(), r.h(), line_delta);
  reused_image->draw(Rectangle(r.w(),r.h()), r);
}

/**
  Same except \a line_delta is set to <i>r</i>.w() times
  depth(<i>type</i>), indicating the rows are packed together one
  after another with no gap.
*/
void fltk::drawimage(const uchar* pointer, fltk::PixelType type,
		     const Rectangle& r) {
  drawimage(pointer, type, r, depth(type)*r.w());
}

/** \typedef fltk::DrawImageCallback

  Type of function passed to drawimage(). It must return a pointer
  to a horizontal row of \a w pixels, starting with the pixel at
  \a x and \a y (relative to the top-left corner of the image, not
  to the coordinate space drawimage() is called in). These pixels
  must be in the format described by \a type passed to drawimage()
  and must be the \a delta apart passed to drawimage().

  \a userdata is the same as the argument passed to drawimage().
  This can be used to point at a structure of information about
  the image.

  Due to cropping, less than the whole image may be requested. So the
  callback may get an \a x greater than zero, the first \a y passed to
  it may be greater than zero, and \a x+w may be less than the width
  of the image.  The passed \a buffer contains room for at least the
  number of pixels specified by the width passed to drawimage(). You
  can use this as temporary storage to construct a row of the image,
  and return a pointer offset by \a x into it.

*/

/**
  Call the passed function to provide each scan line of the
  image. This lets you generate the image as it is being drawn, or do
  arbitrary decompression of stored data (provided it can be
  decompressed to individual scan lines easily).

  \a callback is called with the void* \a data argument (this can be
  used to point at a structure of information about the image), the x,
  y, and number of pixels desired from the image, measured from the
  upper-left corner of the image. It is also given a buffer of at
  least \a w pixels that can be used as temporary storage, for
  instance to decompress a line read from a file. You can then return
  a pointer to this buffer, or to somewhere inside it.

  The callback must return n pixels of the format described by \a
  type.

  The \a xywh rectangle describes the area to draw. The callback is
  called with y values between 0 and h-1. Due to cropping not
  all pixels may be asked for. You can assumme y will be asked for in
  increasing order.
*/
void fltk::drawimage(DrawImageCallback cb,
		     void* userdata, fltk::PixelType type,
		     const Rectangle& r) {
  if (innards(0, type, r, 0, cb, userdata)) return;
  // Fake it using a temporary Image
  if (!reused_image) reused_image = new Image();
  reused_image->setpixeltype(type);
  reused_image->setsize(r.w(), r.h());
  for (int y=0; y<r.h(); y++)
    reused_image->setpixels(cb(userdata, 0, y, r.w(), reused_image->linebuffer(y)), y);
  reused_image->draw(Rectangle(r.w(),r.h()), r);
}

//
// End of "$Id: Image.cxx 8748 2011-05-28 04:47:50Z bgbnbigben $".
//
