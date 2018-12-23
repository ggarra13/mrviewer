using namespace fltk;

uchar *				// O - Pixel buffer or NULL if failed
fltk::readimage(uchar *p,	// I - Pixel buffer or NULL to allocate
	PixelType type,		// Type of pixels to store (RGB and RGBA only now)
	const Rectangle& r,	// area to read
	int linedelta
) {
  int   X = r.x();
  int   Y = r.y();
  int   w = r.w();
  int   h = r.h();
  int delta = depth(type);

  int	x, y;			// Looping vars

  // None of these read alpha yet, so set the alpha to 1 everywhere.
  if (type > 3) {
    for (int y = 0; y < h; y++) {
      uchar *line = p + y * linedelta + 3;
      for (int x = 0; x < w; x++) {*line = 0xff; line += delta;}
    }
  }


  // Grab all of the pixels in the image, one at a time...
  // MRS: there has to be a better way than this!
  for (y = 0; y < h; y ++) {
    uchar *ptr = p + y*linedelta;
    for (x = 0; x < w; x ++, ptr += delta) {
      COLORREF c = GetPixel(dc, X + x, Y + y);
      ptr[0] = (uchar)c;
      c >>= 8;
      ptr[1] = (uchar)c;
      c >>= 8;
      ptr[2] = (uchar)c;
    }
  }

  return p;
}

