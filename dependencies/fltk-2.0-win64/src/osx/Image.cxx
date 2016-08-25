struct fltk::Picture {
  int w, h, linedelta;
  unsigned long n; // bytes used
  uchar* data;
  int depth;
  CGImageAlphaInfo bitmapInfo;
  CGColorSpaceRef colorspace;

  Picture(PixelType p, int w, int h, int ld, const uchar* d=0) {
    this->w = w;
    this->h = h;
    this->linedelta = ld;
    this->depth = ::depth(p);
    if (d) {n = 0; data = (uchar*)d;}
    else {n = ld*h; data = (uchar*)(new U32[n/4]);}
    setpixeltype(p);
  }

  void setpixeltype(PixelType p) {
    static CGColorSpaceRef rgbcolorspace = 0;
    static CGColorSpaceRef graycolorspace = 0;
    if (!rgbcolorspace) rgbcolorspace = CGColorSpaceCreateDeviceRGB();
    colorspace = rgbcolorspace;
    switch (p) {
    case MASK:
      bitmapInfo = kCGImageAlphaOnly;
      break;
    case MONO:
      bitmapInfo = kCGImageAlphaNone;
      if (!graycolorspace) graycolorspace = CGColorSpaceCreateDeviceGray();
      colorspace = graycolorspace;
      break;
    case RGBx: bitmapInfo = kCGImageAlphaNone; break;
    case RGB: bitmapInfo = kCGImageAlphaNone; break;
    case RGBA: bitmapInfo = kCGImageAlphaPremultipliedLast; break;
    case RGBM: bitmapInfo = kCGImageAlphaLast; break;
#if __BIG_ENDIAN__
# define BE(x) x
#else
# define BE(x) CGImageAlphaInfo(x|(2<<12))
#endif
    case RGB32: bitmapInfo = BE(kCGImageAlphaNoneSkipFirst); break;
    case ARGB32: bitmapInfo = BE(kCGImageAlphaPremultipliedFirst); break;
    case MRGB32: bitmapInfo = BE(kCGImageAlphaFirst); break;
    default: bitmapInfo = kCGImageAlphaNone; break;
    }
  }

  ~Picture() {
    // delete data only if it was not passed to constructor:
    if (n) delete[] (U32*)data;
  }

  // Create and return an image context that matches a given
  // subrectangle:
  CGImageRef img(const fltk::Rectangle rect) {
    uchar* ptr = data+rect.y()*linedelta+rect.x()*depth;

    CGDataProviderRef src =
      CGDataProviderCreateWithData( 0L, ptr, linedelta*rect.h(), 0);

    CGImageRef img;
    if (bitmapInfo == kCGImageAlphaOnly)
      img = CGImageMaskCreate(rect.w(), rect.h(),
                              8, // bitsPerComponent
                              8, // bitsPerPixel
                              linedelta, // bytesPerRow
                              src, // provider
                              0L, // decode array (?)
                              true); // shouldInterpolate
    else
      img = CGImageCreate(rect.w(), rect.h(),
                          8, // bitsPerComponent
                          8*depth, // bitsPerPixel
                          linedelta, // bytesPerRow
                          colorspace,
                          bitmapInfo, // bitmapInfo
                          src, // provider
                          0L, // decode array (?)
                          true, // shouldInterpolate
                          kCGRenderingIntentDefault);
    CGDataProviderRelease(src);
    return img;
  }

  void sync() {}

};

int Image::buffer_width() const {
  if (picture) return picture->w;
  return w();
}

int Image::buffer_height() const {
  if (picture) return picture->h;
  return h();
}

int Image::buffer_linedelta() const {
  if (picture) return picture->linedelta;
  return (w_*depth()+3)&-4;
}

int Image::buffer_depth() const {
  return depth();
}

void Image::set_forceARGB32() {
  flags |= FORCEARGB32;
  // NYI!!!
}

void Image::clear_forceARGB32() {
  flags &= ~FORCEARGB32;
  // NYI!!!
}

fltk::PixelType Image::buffer_pixeltype() const {
  return pixeltype();
}

unsigned long Image::mem_used() const {
  if (picture) return picture->n;
  return 0;
}

const uchar* Image::buffer() const {
  if (picture) return picture->data;
  return const_cast<Image*>(this)->buffer();
}

uchar* Image::buffer() {
  if (picture) {
    picture->sync();
  } else {
    open_display();
    picture = new Picture(pixeltype_, w_, h_, (w_*depth()+3)&-4);
    memused_ += picture->n;
  }
  return picture->data;
}

void Image::destroy() {
  if (!picture) return;
  if (picture->n > memused_) memused_ -= picture->n; else memused_ = 0;
  delete picture;
  picture = 0;
  flags &= ~FETCHED;
}

void Image::setpixeltype(PixelType p) {
  if (picture) {
    if (::depth(p) != picture->depth) destroy();
    else picture->setpixeltype(p);
  }
  pixeltype_ = p;
}

void Image::setsize(int w, int h) {
  if (picture && (w > picture->w || h > picture->h)) destroy();
  w_ = w;
  h_ = h;
}

uchar* Image::linebuffer(int y) {
  buffer();
  return picture->data+y*picture->linedelta;
}

void Image::setpixels(const uchar* buf, int y) {
  buffer();
  flags &= ~COPIED;
  uchar* to = picture->data+y*picture->linedelta;
  if (buf != to) memcpy(to, buf, width()*depth());
}

void Image::setpixels(const uchar* buf, const fltk::Rectangle& r, int linedelta)
{
  if (r.empty()) return;
  buffer();
  flags &= ~COPIED;
  uchar* to = picture->data+r.y()*picture->linedelta+r.x()*buffer_depth();
  // see if we can do it all at once:
  if (r.w() == picture->w && (r.h()==1 || linedelta == picture->linedelta)) {
    memcpy(to, buf, r.h()*linedelta);
  } else {
    for (int y = 0; y < r.h(); y++) {
      memcpy(to, buf, r.w()*depth());
      buf += linedelta;
      to += picture->linedelta;
    }
  }
}

void Image::fetch_if_needed() const {
  if (!(flags&FETCHED)) {
    Image* thisimage = const_cast<Image*>(this);
    thisimage->fetch();
    thisimage->flags |= FETCHED;
  }
}

extern void fl_set_quartz_ctm();

void Image::draw(const fltk::Rectangle& from, const fltk::Rectangle& to) const {
  fetch_if_needed();
  if (!picture) {fillrect(to); return;}
  CGContextSaveGState(quartz_gc);
  fl_set_quartz_ctm();
  CGImageRef img = picture->img(from);
  CGRect rect = {{to.x(), -to.y()}, {to.w(), -to.h()}};
  CGContextDrawImage(quartz_gc, rect, img);
  CGImageRelease(img);
  CGContextRestoreGState(quartz_gc);
}

void Image::setimage(const uchar* source, PixelType p, int w, int h, int ld)
{
  setsize(w,h);
  setpixeltype(p);
  if (ld & 3) {
    // can't handle linedelta that is not a multiple of 4?
    if (picture && !(picture->n)) destroy();
    setpixels(source, Rectangle(w, h), ld);
  } else {
    destroy();
    picture = new Picture(p, w, h, ld, source);
    flags |= FETCHED;
  }
}

void Image::make_current() {
  // TODO - see x11/Image.cxx for implementation clues
}

////////////////////////////////////////////////////////////////

// drawimage() calls this to see if a direct draw will work. Returns
// true if successful, false if an Image must be used to emulate it.

static bool innards(const uchar *buf, fltk::PixelType type,
		    const fltk::Rectangle& r1,
		    int linedelta,
		    DrawImageCallback cb, void* userdata)
{
  return false;
}

//
// End of "$Id: Image.cxx 1488 2007-01-05 02:31:53Z spitzak $".
//
