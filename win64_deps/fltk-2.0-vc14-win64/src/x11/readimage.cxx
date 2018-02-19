//
// "$Id: readimage.cxx,v 1.1 2005/02/05 00:26:11 spitzak Exp $"
//
// X11 image reading routines for the Fast Light Tool Kit (FLTK).
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

#include <fltk/string.h>

#ifdef DEBUG
#  include <stdio.h>
#endif // DEBUG

#  include <X11/Xutil.h>
#  ifdef __sgi
#    define Window XWindow
#    include <X11/extensions/readdisplay.h>
#    undef Window
#  endif // __sgi

using namespace fltk;

uchar *				// O - Pixel buffer or NULL if failed
fltk::readimage(uchar *p,	// I - Pixel buffer or NULL to allocate
	PixelType type,		// Type of pixels to store (RGB and RGBA only now)
	const Rectangle& r,	// area to read
		int linedelta	// pointer increment per line
) {
  int X = r.x();
  int Y = r.y();
  int w = r.w();
  int h = r.h();
  int d = depth(type);		// pointer increment per pixel
  XImage	*image;		// Captured image
  int		i, maxindex;	// Looping vars
  int           x, y;		// Current X & Y in image
  unsigned char *line,		// Array to hold image row
		*line_ptr;	// Pointer to current line image
  unsigned char	*pixel;		// Current color value
  XColor	colors[4096];	// Colors from the colormap...
  unsigned char	cvals[4096][3];	// Color values from the colormap...
  unsigned	index_mask,
		index_shift,
		red_mask,
		red_shift,
		green_mask,
		green_shift,
		blue_mask,
		blue_shift;

  //
  // Under X11 we have the option of the XGetImage() interface or SGI's
  // ReadDisplay extension which does all of the really hard work for
  // us...
  //

#  ifdef __sgi
  if (XReadDisplayQueryExtension(xdisplay, &i, &i)) {
    image = XReadDisplay(xdisplay, xwindow, X, Y, w, h, 0, NULL);
  } else
#  else
  image = 0;
#  endif // __sgi

  if (!image) {
    image = XGetImage(xdisplay, xwindow, X, Y, w, h, AllPlanes, ZPixmap);
  }

  if (!image) return 0;

#ifdef FLTK_DEBUG
  printf("width            = %d\n", image->width);
  printf("height           = %d\n", image->height);
  printf("xoffset          = %d\n", image->xoffset);
  printf("format           = %d\n", image->format);
  printf("data             = %p\n", image->data);
  printf("byte_order       = %d\n", image->byte_order);
  printf("bitmap_unit      = %d\n", image->bitmap_unit);
  printf("bitmap_bit_order = %d\n", image->bitmap_bit_order);
  printf("bitmap_pad       = %d\n", image->bitmap_pad);
  printf("depth            = %d\n", image->depth);
  printf("bytes_per_line   = %d\n", image->bytes_per_line);
  printf("bits_per_pixel   = %d\n", image->bits_per_pixel);
  printf("red_mask         = %08x\n", image->red_mask);
  printf("green_mask       = %08x\n", image->green_mask);
  printf("blue_mask        = %08x\n", image->blue_mask);
#endif // FLTK_DEBUG

  // None of these read alpha yet, so set the alpha to 1 everywhere.
  if (type > 3) {
    for (int y = 0; y < h; y++) {
      line = p + y * linedelta + 3;
      for (int x = 0; x < w; x++) {*line = 0xff; line += d;}
    }
  }

  if (!image->red_mask && image->bits_per_pixel > 12) {
    // Greater than 12 bits must be TrueColor...
    image->red_mask   = xvisual->visual->red_mask;
    image->green_mask   = xvisual->visual->green_mask;
    image->blue_mask   = xvisual->visual->blue_mask;
  }

  // Check if we have colormap image...
  if (image->red_mask == 0) {
    // Get the colormap entries for this window...
    maxindex = xvisual->visual->map_entries;

    for (i = 0; i < maxindex; i ++) colors[i].pixel = i;

    XQueryColors(xdisplay, xcolormap, colors, maxindex);

    for (i = 0; i < maxindex; i ++) {
      cvals[i][0] = colors[i].red >> 8;
      cvals[i][1] = colors[i].green >> 8;
      cvals[i][2] = colors[i].blue >> 8;
    }

    // Read the pixels and output an RGB image...
    for (y = 0; y < image->height; y ++) {
      pixel = (unsigned char *)(image->data + y * image->bytes_per_line);
      line  = p + y * linedelta;

      switch (image->bits_per_pixel) {
	case 1 :
	  for (x = image->width, line_ptr = line, index_mask = 128;
	       x > 0;
	       x --, line_ptr += d) {
	    if (*pixel & index_mask) {
	      line_ptr[0] = cvals[1][0];
	      line_ptr[1] = cvals[1][1];
	      line_ptr[2] = cvals[1][2];
	    } else {
	      line_ptr[0] = cvals[0][0];
	      line_ptr[1] = cvals[0][1];
	      line_ptr[2] = cvals[0][2];
	    }

	    if (index_mask > 1) {
	      index_mask >>= 1;
	    } else {
	      index_mask = 128;
	      pixel ++;
	    }
	  }
	  break;

	case 2 :
	  for (x = image->width, line_ptr = line, index_shift = 6;
	       x > 0;
	       x --, line_ptr += d) {
	    i = (*pixel >> index_shift) & 3;

	    line_ptr[0] = cvals[i][0];
	    line_ptr[1] = cvals[i][1];
	    line_ptr[2] = cvals[i][2];

	    if (index_shift > 0) {
	      index_mask >>= 2;
	      index_shift -= 2;
	    } else {
	      index_mask  = 192;
	      index_shift = 6;
	      pixel ++;
	    }
	  }
	  break;

	case 4 :
	  for (x = image->width, line_ptr = line, index_shift = 4;
	       x > 0;
	       x --, line_ptr += d) {
	    if (index_shift == 4) i = (*pixel >> 4) & 15;
	    else i = *pixel & 15;

	    line_ptr[0] = cvals[i][0];
	    line_ptr[1] = cvals[i][1];
	    line_ptr[2] = cvals[i][2];

	    if (index_shift > 0) {
	      index_shift = 0;
	    } else {
	      index_shift = 4;
	      pixel ++;
	    }
	  }
	  break;

	case 8 :
	  for (x = image->width, line_ptr = line;
	       x > 0;
	       x --, line_ptr += d, pixel ++) {
	    line_ptr[0] = cvals[*pixel][0];
	    line_ptr[1] = cvals[*pixel][1];
	    line_ptr[2] = cvals[*pixel][2];
	  }
	  break;

	case 12 :
	  for (x = image->width, line_ptr = line, index_shift = 0;
	       x > 0;
	       x --, line_ptr += d) {
	    if (index_shift == 0) {
	      i = ((pixel[0] << 4) | (pixel[1] >> 4)) & 4095;
	    } else {
	      i = ((pixel[1] << 8) | pixel[2]) & 4095;
	    }

	    line_ptr[0] = cvals[i][0];
	    line_ptr[1] = cvals[i][1];
	    line_ptr[2] = cvals[i][2];

	    if (index_shift == 0) {
	      index_shift = 4;
	    } else {
	      index_shift = 0;
	      pixel += 3;
	    }
	  }
	  break;
      }
    }
  } else {
    // RGB(A) image, so figure out the shifts & masks...
    red_mask  = image->red_mask;
    red_shift = 0;

    while ((red_mask & 1) == 0) {
      red_mask >>= 1;
      red_shift ++;
    }

    green_mask  = image->green_mask;
    green_shift = 0;

    while ((green_mask & 1) == 0) {
      green_mask >>= 1;
      green_shift ++;
    }

    blue_mask  = image->blue_mask;
    blue_shift = 0;

    while ((blue_mask & 1) == 0) {
      blue_mask >>= 1;
      blue_shift ++;
    }

    // Read the pixels and output an RGB image...
    for (y = 0; y < image->height; y ++) {
      pixel = (unsigned char *)(image->data + y * image->bytes_per_line);
      line  = p + y * linedelta;

      switch (image->bits_per_pixel) {
	case 8 :
	  for (x = image->width, line_ptr = line;
	       x > 0;
	       x --, line_ptr += d, pixel ++) {
	    i = *pixel;

	    line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	    line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	    line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	  }
	  break;

	case 12 :
	  for (x = image->width, line_ptr = line, index_shift = 0;
	       x > 0;
	       x --, line_ptr += d) {
	    if (index_shift == 0) {
	      i = ((pixel[0] << 4) | (pixel[1] >> 4)) & 4095;
	    } else {
	      i = ((pixel[1] << 8) | pixel[2]) & 4095;
	    }

	    line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	    line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	    line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;

	    if (index_shift == 0) {
	      index_shift = 4;
	    } else {
	      index_shift = 0;
	      pixel += 3;
	    }
	  }
	  break;

	case 16 :
	  if (image->byte_order == LSBFirst) {
	    // Little-endian...
	    for (x = image->width, line_ptr = line;
		 x > 0;
		 x --, line_ptr += d, pixel += 2) {
	      i = (pixel[1] << 8) | pixel[0];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  } else {
	    // Big-endian...
	    for (x = image->width, line_ptr = line;
		 x > 0;
		 x --, line_ptr += d, pixel += 2) {
	      i = (pixel[0] << 8) | pixel[1];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  }
	  break;

	case 24 :
	  if (image->byte_order == LSBFirst) {
	    // Little-endian...
	    for (x = image->width, line_ptr = line;
		 x > 0;
		 x --, line_ptr += d, pixel += 3) {
	      i = (((pixel[2] << 8) | pixel[1]) << 8) | pixel[0];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  } else {
	    // Big-endian...
	    for (x = image->width, line_ptr = line;
		 x > 0;
		 x --, line_ptr += d, pixel += 3) {
	      i = (((pixel[0] << 8) | pixel[1]) << 8) | pixel[2];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  }
	  break;

	case 32 :
	  if (image->byte_order == LSBFirst) {
	    // Little-endian...
	    for (x = image->width, line_ptr = line;
		 x > 0;
		 x --, line_ptr += d, pixel += 4) {
	      i = (((((pixel[3] << 8) | pixel[2]) << 8) | pixel[1]) << 8) | pixel[0];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  } else {
	    // Big-endian...
	    for (x = image->width, line_ptr = line;
		 x > 0;
		 x --, line_ptr += d, pixel += 4) {
	      i = (((((pixel[0] << 8) | pixel[1]) << 8) | pixel[2]) << 8) | pixel[3];

	      line_ptr[0] = 255 * ((i >> red_shift) & red_mask) / red_mask;
	      line_ptr[1] = 255 * ((i >> green_shift) & green_mask) / green_mask;
	      line_ptr[2] = 255 * ((i >> blue_shift) & blue_mask) / blue_mask;
	    }
	  }
	  break;
      }
    }
  }

  // Destroy the X image we've read and return the RGB(A) image...
  XDestroyImage(image);

  return p;
}

//
// End of "$Id: readimage.cxx,v 1.1 2005/02/05 00:26:11 spitzak Exp $".
//
