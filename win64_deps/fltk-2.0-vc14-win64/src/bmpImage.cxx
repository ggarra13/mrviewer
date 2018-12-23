//
// "$Id: bmpImage.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// bmpImage routines.
//
// Copyright 1997-2003 by Easy Software Products.
// Copyright 1998-2006 by Bill Spitzak and others.
// Image support donated by Matthias Melcher, Copyright 2000.
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

#include <config.h>
#include <fltk/SharedImage.h> // defines bmpImage.h
#include <fltk/events.h>
#include <fltk/error.h>
#include <fltk/x.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace fltk;

static FILE *bmpFile;
static const uchar* bmpDatasStart;
static const uchar* bmpDatas;

static uchar GETC()
{
  if (bmpDatas) return *bmpDatas++;
  else return fgetc(bmpFile);
}

#undef FREAD
static int FREAD(void *buf, int size)
{
  if (bmpDatas) {
    memcpy(buf, bmpDatas, size);
    bmpDatas += size;
    return size;
  }
  return fread(buf, 1, size, bmpFile);
}

static int FSEEK(int offset)
{
  if (bmpDatas) {
    bmpDatas = bmpDatasStart + offset;
    return offset;
  }
  return fseek(bmpFile, offset, SEEK_SET);
}

// read 16-bit unsigned integer:
static unsigned short read_word() {
  uchar b0 = GETC();
  uchar b1 = GETC();
  return ((b1 << 8) | b0);
}

// read 32-bit unsigned integer:
static unsigned int read_dword() {
  uchar b0 = GETC();
  uchar b1 = GETC();
  uchar b2 = GETC();
  uchar b3 = GETC();
  return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

//
// BMP definitions...
//

bool bmpImage::test(const uchar* buffer, unsigned size)
{
  return !strncmp((char*)buffer, "BM", size<2? size:2);
}

//
// Load a BMP image file.
//
bool bmpImage::fetch()
{
  bmpDatas = (uchar*)datas;
  bmpDatasStart = bmpDatas;
  if (!datas) {
    bmpFile = fopen(get_filename(), "rb");
    if (!bmpFile) return false;
  }

  unsigned
		bitsperpixel,
		depth = 3,	// bytes per output pixel
		compression,	// Type of compression
		colors_used,	// Number of colors used
		repcount,	// Number of times to repeat
		temp,		// Temporary color
		align,		// Alignment bytes
		dataSize;	// number of bytes in image data set
  bool		flip = false;	// if true, image is top-to-bottom
  uchar		colormap[256][3];// Colormap
  bool		mask = false;	// single bit mask follows image data
  bool		use_5_6_5 = false; // Use 565 instead of 555 for 16-bit data

  // Get the header...
  uchar byte = GETC();	// Check "BM" sync chars
  uchar bit  = GETC();
  if (byte != 'B' || bit != 'M') {
    if (!datas) fclose(bmpFile);
    return false;
  }

  read_dword();		// Skip size
  read_word();		// Skip reserved stuff
  read_word();
  unsigned offbits = read_dword();// Read offset to image data

  // Then the bitmap information...
  unsigned info_size = read_dword();

  //printf("offbits = %u, info_size = %d\n", offbits, info_size);

  if (info_size < 40) {
    // Old Windows/OS2 BMP header...
    // It is unclear if negative numbers or numbers > 32767 are correct:
    int w = read_word(); //if (w&0x8000) w = (w^0xffff)+1;
    int h = read_word(); //if (h&0x8000) {h = (h^0xffff)+1; flip=true;}
    read_word();
    bitsperpixel = read_word();
    compression = 0;
    colors_used = 0;
    repcount = info_size - 12;
    this->setsize(w,h);
    this->setpixeltype(fltk::RGB);
  } else {
    // New BMP header...
    // Must take absolute value:
    unsigned w = read_dword();
    if (w&0x80000000) w = (w^0xffffffff)+1;
    unsigned h = read_dword();
    if (h&0x80000000) {h = (h^0xffffffff)+1; flip=true; }
    read_word();
    bitsperpixel = read_word();
    compression = read_dword();
    dataSize = read_dword();
    read_dword();
    read_dword();
    colors_used = read_dword();
    read_dword();

    repcount = info_size - 40;
    if (!compression && bitsperpixel>=8 && w>32/bitsperpixel) {
      unsigned maskSize =
	(((w*(bitsperpixel/8)+3)&~3)*h) + (((((w+7)/8)+3)&~3)*h);
      if (maskSize==2*dataSize) {
	mask = true;
	h /= 2;
	depth = 4;
      }
    }
    this->setsize(w,h);
    this->setpixeltype(mask ? fltk::RGBA : fltk::RGB);
  }

  //printf("w %d, h %d, bitsperpixel %d, compression %d, colors_used %d, repcount %d\n", w(), h(), bitsperpixel, compression, colors_used, repcount);

  // Skip remaining header bytes...
  while (repcount > 0) {
    GETC();
    repcount --;
  }

  // Check header data...
  if (!w() || !h() || !bitsperpixel) {
    if (!datas) fclose(bmpFile);
    return false;
  }

  // Get colormap...
  if (colors_used == 0 && bitsperpixel <= 8)
    colors_used = 1 << bitsperpixel;

  for (repcount = 0; repcount < colors_used; repcount ++) {
    // Read BGR color...
    FREAD(colormap[repcount], 3);
    //fread(colormap[repcount], 1, 3, );

    // Skip pad byte for new BMP files...
    if (info_size > 12) GETC();
  }

  // Read first dword of colormap. It tells us if 5:5:5 or 5:6:5 for 16 bit
  if (bitsperpixel == 16)
    use_5_6_5 = (read_dword() == 0xf800);

  // Setup image and buffers...
  if (offbits) FSEEK(offbits);

  uchar *array = new uchar[w() * h() * depth];
  //alloc_array = 1;

  // Read the image data...
  int color = 0;
  repcount = 0;
  align = 0;
  temp  = 0;

  for (int y = 0; y < h(); y++) {
    uchar* ptr = array + (flip ? y : h()-y-1) * w() * depth;

    int x;

    switch (bitsperpixel)
    {
      case 1 : // Bitmap
	  for (x = w(), bit = 128; x > 0; x --) {
	    if (bit == 128) byte = (uchar)GETC();

	    if (byte & bit) {
	      *ptr++ = colormap[1][2];
	      *ptr++ = colormap[1][1];
	      *ptr++ = colormap[1][0];
	    } else {
	      *ptr++ = colormap[0][2];
	      *ptr++ = colormap[0][1];
	      *ptr++ = colormap[0][0];
	    }

	    if (bit > 1)
	      bit >>= 1;
	    else
	      bit = 128;
	  }

	  // Read remaining bytes to align to 32 bits...
	  for (temp = (w() + 7) / 8; temp & 3; temp ++) {
	    GETC();
	  }
	  break;

      case 4 : // 16-color
	  for (x = w(), bit = 0xf0; x > 0; x --) {
	    // Get a new repcount as needed...
	    if (repcount == 0) {
	      if (!compression) {
		repcount = 2;
		color = -1;
	      } else {
		while (align > 0) {
		  align --;
		  GETC();
		}

		if ((repcount = GETC()) == 0) {
		  if ((repcount = GETC()) == 0) {
		    // End of line...
		    x ++;
		    continue;
		  } else if (repcount == 1) {
		    // End of image...
		    break;
		  } else if (repcount == 2) {
		    // Delta...
		    repcount = GETC() * GETC() * w();
		    color = 0;
		  } else {
		    // Absolute...
		    color = -1;
		    align = ((4 - (repcount & 3)) / 2) & 1;
		  }
		} else {
		  color = GETC();
		}
	      }
	    }

	    // Get a new color as needed...
	    repcount --;

	    // Extract the next pixel...
	    if (bit == 0xf0) {
	      // Get the next color byte as needed...
	      if (color < 0) temp = GETC();
	      else temp = color;

	      // Copy the color value...
	      *ptr++ = colormap[(temp >> 4) & 15][2];
	      *ptr++ = colormap[(temp >> 4) & 15][1];
	      *ptr++ = colormap[(temp >> 4) & 15][0];

	      bit  = 0x0f;
	    } else {
	      bit  = 0xf0;

	      // Copy the color value...
	      *ptr++ = colormap[temp & 15][2];
	      *ptr++ = colormap[temp & 15][1];
	      *ptr++ = colormap[temp & 15][0];
	    }

	  }

	  if (!compression) {
	    // Read remaining bytes to align to 32 bits...
	    for (temp = (w() + 1) / 2; temp & 3; temp ++) {
	      GETC();
	    }
	  }
	  break;

      case 8 : // 256-color
	  for (x = w(); x > 0; x --, ptr += depth) {
	    // Get a new repcount as needed...
	    if (!compression) {
	      repcount = 1;
	      color = -1;
	    }

	    if (repcount == 0) {
	      while (align > 0) {
		align --;
		GETC();
	      }

	      if ((repcount = GETC()) == 0) {
		if ((repcount = GETC()) == 0) {
		  // End of line...
		  x ++;
		  continue;
		} else if (repcount == 1) {
		  // End of image...
		  break;
		} else if (repcount == 2) {
		  // Delta...
		  repcount = GETC() * GETC() * w();
		  color = 0;
		} else {
		  // Absolute...
		  color = -1;
		  align = (2 - (repcount & 1)) & 1;
		}
	      } else {
		color = GETC();
	      }
	    }

	    // Get a new color as needed...
	    if (color < 0) temp = GETC();
	    else temp = color;

	    repcount --;

	    // Copy the color value...
	    ptr[0] = colormap[temp][2];
	    ptr[1] = colormap[temp][1];
	    ptr[2] = colormap[temp][0];
	  }

	  if (!compression) {
	    // Read remaining bytes to align to 32 bits...
	    for (temp = w(); temp & 3; temp ++) {
	      GETC();
	    }
	  }
	  break;

      case 16 : // 16-bit, either 555 or 565 rgb bits:
	  for (x = w(); x > 0; x --, ptr += depth) {
	    uchar b = GETC(), a = GETC() ;
	    if (use_5_6_5) {
	      ptr[0] = (uchar)(( b << 3 ) & 0xf8);
	      ptr[1] = (uchar)(((a << 5) & 0xe0) | ((b >> 3) & 0x1c));
	      ptr[2] = (uchar)(a & 0xf8);
	    } else {
	      ptr[2] = (uchar)((b << 3) & 0xf8);
	      ptr[1] = (uchar)(((a << 6) & 0xc0) | ((b >> 2) & 0x38));
	      ptr[0] = (uchar)((a<<1) & 0xf8);
	    }
	  }

	  // Read remaining bytes to align to 32 bits...
	  for (temp = w() * 3; temp & 3; temp ++) {
	    GETC();
	  }
	  break;

      case 24 : // 24-bit RGB
	  for (x = w(); x > 0; x --, ptr += depth) {
	    ptr[2] = GETC();
	    ptr[1] = GETC();
	    ptr[0] = GETC();
	  }

	  // Read remaining bytes to align to 32 bits...
	  for (temp = w() * 3; temp & 3; temp ++) {
	    GETC();
	  }
	  break;
    }
  }

  if (mask) {
    for (int y = 0; y < h(); y++) {
      uchar* ptr = array + (flip ? y : h()-y-1) * w() * depth + 3;
      for (int x = w(), bit = 128; x > 0; x --, ptr += 4) {
	if (bit == 128) byte = (uchar)GETC();
	if (byte & bit)
	  *ptr = 0;
	else
	  *ptr = 255;
	if (bit > 1)
	  bit >>= 1;
	else
	  bit = 128;
      }
      // Read remaining bytes to align to 32 bits...
      for (temp = (w() + 7) / 8; temp & 3; temp ++)
	GETC();
    }
  }

  setpixels(array, Rectangle(w(), h()), depth*w());

  // Close the file and return...
  if (!datas) fclose(bmpFile);

  delete []array;
  return true;
}

//
// End of "$Id: bmpImage.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
