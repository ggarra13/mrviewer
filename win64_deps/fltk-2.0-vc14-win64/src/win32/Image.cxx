// Extra bitblt functions:
#define NOTSRCAND       0x00220326 /* dest = (NOT source) AND dest */
#define NOTSRCINVERT    0x00990066 /* dest = (NOT source) XOR dest */
#define SRCORREVERSE    0x00DD0228 /* dest = source OR (NOT dest) */
#define SRCNAND         0x007700E6 /* dest = NOT (source AND dest) */
#define MASKPAT         0x00E20746 /* dest = (src & pat) | (!src & dst) */
#define COPYFG          0x00CA0749 /* dest = (pat & src) | (!pat & dst) */
#define COPYBG          0x00AC0744 /* dest = (!pat & src) | (pat & dst) */

# if defined(__MINGW32__) || defined(__CYGWIN__) // || defined(__BORLANDC__)
// AlphaBlend IS declared in these but only when WINVER is >= 500
extern "C" {
  WINGDIAPI BOOL  WINAPI AlphaBlend(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);
}
#  define AC_SRC_ALPHA		  0x01
# endif

// Link against msimg32 lib to get the AlphaBlend function:
#if defined(_MSC_VER)
# pragma comment(lib, "msimg32.lib")
#endif

// DM C/C++ compiler comes with very old Win32 API headers. Modifying existing
// ones (from MS PSDK for example) is time-consuming job, so I have decided
// to declare necessary stuff here.
#if __DMC__ || __SC__ || __RCC__
  typedef struct _BLENDFUNCTION {
    BYTE BlendOp;
    BYTE BlendFlags;
    BYTE SourceConstantAlpha;
    BYTE AlphaFormat;
  } BLENDFUNCTION,*PBLENDFUNCTION,*LPBLENDFUNCTION;
  extern "C" {
    WINGDIAPI BOOL WINAPI AlphaBlend(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);
  }
  #define AC_SRC_OVER   0x00
  #define AC_SRC_ALPHA  0x01
#endif // __DMC__

using namespace fltk;

static int syncnumber = 1;

struct fltk::Picture {
  int w, h, linedelta;
  unsigned long n; // bytes used
  uchar* data;
  HBITMAP bitmap;
  int syncro;
  Picture(int w, int h) {
    this->w = w;
    this->h = h;
    linedelta = w*4;
    n = linedelta*h;
    //picture = CreateCompatibleBitmap(getDC(), data_w_, data_h_);
    // Use CreateDIBSection instead, it seems to be only reliable way to
    // make AlphaBlend function working correctly always..
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = n;
    void* p;
    bitmap = CreateDIBSection(getDC(), &bmi, DIB_RGB_COLORS, &p, NULL, 0);
    data = (uchar*)p;
    syncro=0;
  }

  void sync() {
    if (syncro == syncnumber) {++syncnumber; GdiFlush();}
  }

  ~Picture() {
    sync();
    DeleteObject(bitmap);
  }

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
  return w_*4;
}

int Image::buffer_depth() const {
  return 4;
}

PixelType Image::buffer_pixeltype() const {
  return ARGB32;
}

void Image::set_forceARGB32() {
  flags |= FORCEARGB32;
  // This does nothing, as it always is ARGB32
}

void Image::clear_forceARGB32() {
  flags &= ~FORCEARGB32;
  // This does nothing, as it always is ARGB32
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
    picture = new Picture(w_,h_);
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

// Convert fltk pixeltypes to ARGB32:
// Go backwards so the buffers can be shared
static void convert(uchar* to, const uchar* from, PixelType type, int w) {
  U32* t = (U32*)to;
  switch (type) {
  case MONO:
    t += w;
    from += w;
    while (t > (U32*)to)
      *--t = 0xff000000 | (*--from * 0x10101);
    break;
  case MASK: {
    uchar mr,mg,mb;
    fltk::split_color(fltk::getcolor(),mr,mg,mb);
    t += w;
    from += w;
    while (t > (U32*)to) {
      uchar a = 255 - *--from;
      *--t = (a<<24) | (((mr*a)<<8)&0xff0000) | ((mg*a)&0xff00) | ((mb*a)>>8);
    }
    break;}
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
    t += w;
    from += 4*w;
    while (t > (U32*)to) {
      from -= 4;
      *--t = (from[3]<<24) | (from[0]<<16) | (from[1]<<8) | from[2];
    }
    break;
  case RGB32:
  case ARGB32:
    if (from != to) memcpy(to, from, 4*w);
    break;
  case RGBM:
    t += w;
    from += 4*w;
    while (t > (U32*)to) {
      from -= 4;
      uchar a = from[3];
      *--t = (a<<24) | (((from[0]*a)<<8)&0xff0000) | ((from[1]*a)&0xff00) | ((from[2]*a)>>8);
    }
    break;
  case MRGB32:
    t += w;
    from += 4*w;
    while (t > (U32*)to) {
      from -= 4;
      U32 v = *(U32*)from;
      uchar a = v>>24;
      *--t = (v&0xff000000) |
          ((((v&0xff0000)*a)>>8) & 0xff0000) |
          ((((v&0xff00)*a)>>8) & 0xff00) |
          ((((v&0xff)*a)>>8) & 0xff);
    }
    break;
  }
}

void Image::setpixels(const uchar* buf, int y) {
  buffer();
  flags &= ~COPIED;
  uchar* to = picture->data+y*picture->linedelta;
  convert(to, buf, pixeltype_, width());
}

void Image::setpixels(const uchar* buf, const fltk::Rectangle& r, int linedelta)
{
  if (r.empty()) return;
  buffer();
  flags &= ~COPIED;
  uchar* to = picture->data+r.y()*picture->linedelta+r.x()*buffer_depth();
  // see if we can do it all at once:
  if (r.w() == picture->w && (r.h()==1 || linedelta == picture->linedelta)) {
    convert(to, buf, pixeltype_, r.w()*r.h());
  } else {
    for (int y = 0; y < r.h(); y++) {
      convert(to, buf, pixeltype_, r.w());
      buf += linedelta;
      to += picture->linedelta;
    }
  }
}

void Image::fetch_if_needed() const {
  if (pixeltype_==MASK) {
    int fg = fltk::get_color_index(fltk::getcolor())&0xffffff00;
    if ((flags & 0xffffff00) != fg)
      (const_cast<Image*>(this))->flags = (flags&0xff&~FETCHED)|fg;
  }
  if (!(flags&FETCHED)) {
    Image* thisimage = const_cast<Image*>(this);
    thisimage->fetch();
    thisimage->flags |= FETCHED;
  }
}

void Image::draw(const fltk::Rectangle& from, const fltk::Rectangle& to) const {
  fetch_if_needed();
  if (!picture) {fillrect(to); return;}
  // unfortunately rotation does not work. Pick nearest scaled size:
  fltk::Rectangle R; fltk::transform(to,R);
  HDC tempdc = CreateCompatibleDC(dc);
  SelectObject(tempdc, picture->bitmap);
  if (fills_rectangle() && R.w()==from.w() && R.h()==from.h()) {
    BitBlt(dc, R.x(), R.y(), from.w(), from.h(), tempdc, from.x(), from.y(), SRCCOPY);
  } else {
    BLENDFUNCTION m_bf;
    m_bf.BlendOp = AC_SRC_OVER;
    m_bf.BlendFlags = 0;
    m_bf.AlphaFormat = fills_rectangle() ? 0 : 1; //AC_SRC_ALPHA;
    m_bf.SourceConstantAlpha = 0xFF;
    AlphaBlend(dc, R.x(), R.y(), R.w(), R.h(), tempdc,
               from.x(), from.y(), from.w(), from.h(), m_bf);
  }
  picture->syncro = syncnumber;
  DeleteDC(tempdc);
}

void Image::setimage(const uchar* d, PixelType p, int w, int h, int ld) {
  setsize(w,h);
  setpixeltype(p);
  setpixels(d, Rectangle(w,h), ld); flags = 0;
}

void Image::make_current() {
  buffer();
  draw_into(picture->bitmap, picture->w, picture->h);
}

////////////////////////////////////////////////////////////////

// drawimage() calls this to see if a direct draw will work. Returns
// true if successful, false if an Image must be used to emulate it.

static bool innards(const uchar *buf, PixelType type,
		    const fltk::Rectangle& r1,
		    int linedelta,
		    DrawImageCallback cb, void* userdata)
{
  // We can directly draw RGB32.
  // May be able to draw RGB and RGBx, not sure.
  if (!buf || type != RGB32) return false;

  int W = r1.w();
  int H = r1.h();
  fltk::Rectangle r; transform(r1,r);
  // scaling requires an Image object:
  // (rotation does not work so I use this code in that case...)
  if (r.w() != W || r.h() != H) return false;

  BITMAPINFO bmi;
  memset(&bmi, 0, sizeof(BITMAPINFO));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  if (linedelta >= 0) {
    bmi.bmiHeader.biWidth = linedelta/4;
    bmi.bmiHeader.biHeight = -H;
    bmi.bmiHeader.biSizeImage = linedelta*H;
  } else {
    bmi.bmiHeader.biWidth = -linedelta/4;
    bmi.bmiHeader.biHeight = H;
    buf += linedelta*(H-1);
    bmi.bmiHeader.biSizeImage = -linedelta*H;
  }

  SetDIBitsToDevice(dc, r.x(), r.y(), W, H, 0, 0, 0, H,
                    (LPSTR)buf,
                    &bmi,
                    DIB_RGB_COLORS
                    );
  return true;
}

//
// End of "$Id: Image.cxx 1477 2006-12-30 03:42:46Z spitzak $".
//
