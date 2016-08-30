//
// "$Id: ShapedWindow.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

/*! \class fltk::ShapedWindow

This window's shape is clipped to an area defined by the alpha from
an Image object. Current implementation insists that this be an
fltk::xbmImage, which limits you to 1-bit alpha which must be supplied
by the program. It should not be hard to modify this on newer
systems to accept an arbitrary Image.

The layout and widgets inside are unaware of the mask shape, and most will
act as though the bounding box is available to them. Therefore this
window type is usally sublassed or occupied by a single widget.

If the window will be short-lived and does not have to move, you may
be much better off using an fltk::MenuWindow. This is a normal window
but with no border and no pixels are changed unless you draw into
them. Thus you can get arbitrary shapes by the simple expediency of
not drawing where it should be "transparent".

The window borders and caption created by the window system are turned
off by default for a ShapedWindow object .  They can be re-enabled by
calling void Window::border(bool set).

*/

#include <config.h>
#include <fltk/ShapedWindow.h>
#include <fltk/x.h>

#if USE_X11
# define Window XWindow
# include <X11/extensions/shape.h>
# undef Window
#elif defined(_WIN32)
static HRGN bitmap2region(fltk::xbmImage*);
#elif defined(__APPLE__)
// Not yet implemented for Apple
#endif

using namespace fltk;

/*! \fn void ShapedWindow::shape(xbmImage* b)

The alpha channel of the supplied image is used as the shape of the
window. A pointer to the image is stored, so it must remain in existence
until shape() is called again or the ShapedWindow is destroyed.

If you want your window to resize you should subclass and make a
layout() method that draws a new image and calls shape().

*/

// maybe one day we'll want to be able to resize the clip mask
// when the window resized
static xbmImage* resize_bitmap(xbmImage*, int, int);

void ShapedWindow::draw() {
  if ((lw != w() || lh != h() || changed) && shape_) {
    // size of window has change since last time
    lw = w(); lh = h();
    xbmImage* mask = resize_bitmap(shape_, w(), h());
#if USE_X11
    Pixmap pmask = XCreateBitmapFromData(xdisplay, xid(this),
					 (const char*)mask->array,
					 mask->width(), mask->height());
    hide();
    XShapeCombineMask(xdisplay, xid(this), ShapeBounding, 0, 0,
                      pmask, ShapeSet);
    show();
    if (pmask != None) XFreePixmap(xdisplay, pmask);
#elif defined(_WIN32)
    HRGN region = bitmap2region(mask);
    SetWindowRgn(xid(this), region, TRUE);
#elif defined(__APPLE__)
	mask = 0; // hopefully will shut up compiler
    // not yet implemented for Apple
#else
#endif
    changed = 0;
  }
  Window::draw();
}

// maybe one day we'll want to be able to resize the clip mask
// bitmap when the window is resized
static xbmImage* resize_bitmap(xbmImage* bitmap, int /*W*/, int /*H*/) {
  return bitmap; // CET - FIXME - someday...
}

#ifdef _WIN32
#include <malloc.h>
static inline BYTE bit(int x) { return (BYTE)(1 << (x % 8)); }

// Windows uses regions only to specify the clip mask of a window therefore
// we must convert our bitmap to a region.
// Much of this code is "borrowed" from the Windows version of GTK
// (also LGPLed).  Their code was based on code originally written by
// Jean-Edouard Lachand-Robert.  Ain't open source great?
//
// Modified by me to use an xbmImage, to not hog memory, to not leak memory
// (I hope) and to allow bitmaps of arbitrary dimensions. -CET
static HRGN bitmap2region(xbmImage* bitmap) {
  HRGN hRgn = 0;
  /* For better performances, we will use the ExtCreateRegion()
   * function to create the region. This function take a RGNDATA
   * structure on entry. We will add rectangles by amount of
   * ALLOC_UNIT number in this structure.
   */
# define ALLOC_UNIT  100
  DWORD maxRects = ALLOC_UNIT;

  RGNDATA* pData = (RGNDATA*)malloc(sizeof(RGNDATAHEADER)+(sizeof(RECT)*maxRects));
  pData->rdh.dwSize = sizeof(RGNDATAHEADER);
  pData->rdh.iType = RDH_RECTANGLES;
  pData->rdh.nCount = pData->rdh.nRgnSize = 0;
  SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

  const int bpl = (bitmap->width()+7)/8; // number of bytes per line of pixels
  BYTE* p8 = (BYTE*)bitmap->array;
  BYTE* p;
  for (int y = 0; y < bitmap->height(); y++) {
    /* Scan each bitmap row from left to right*/
    for (int x = 0; x < bitmap->width(); x++) {
      /* Search for a continuous range of "non transparent pixels"*/
      int x0 = x;
      while (x < bitmap->width()) {
        p = p8 + x / 8;
        if (!((*p) & bit(x))) break; /* This pixel is "transparent"*/
        x++;
      }

      if (x > x0) {
        RECT* pr;
        /* Add the pixels (x0, y) to (x, y+1) as a new rectangle
         * in the region
         */
        if (pData->rdh.nCount >= maxRects) {
          maxRects += ALLOC_UNIT;
          pData = (RGNDATA*)realloc(pData, sizeof(RGNDATAHEADER)
                                    + (sizeof(RECT)*maxRects));
    	}
        pr = (RECT*)&pData->Buffer;

#ifdef WIN32
	// fabien : we must take in account the border+caption x and y offsets of the Window
	POINT pt={GetSystemMetrics(SM_CXBORDER)+GetSystemMetrics(SM_CXEDGE),GetSystemMetrics(SM_CYBORDER)+GetSystemMetrics(SM_CXEDGE)+GetSystemMetrics(SM_CYCAPTION)}; 
#else
	// please check some if some adjustement is necessary on Unix(-like) platforms
#endif
	
        SetRect(&pr[pData->rdh.nCount], x0+pt.x, y+pt.y, x+pt.x, y+pt.y+1);
        if (x0 < pData->rdh.rcBound.left)
          pData->rdh.rcBound.left = x0;
        if (y < pData->rdh.rcBound.top)
          pData->rdh.rcBound.top = y;
        if (x > pData->rdh.rcBound.right)
          pData->rdh.rcBound.right = x;
        if (y+1 > pData->rdh.rcBound.bottom)
          pData->rdh.rcBound.bottom = y+1;
        pData->rdh.nCount++;

        /* On Windows98, ExtCreateRegion() may fail if the
         * number of rectangles is too large (ie: >
         * 4000). Therefore, we have to create the region by
         * multiple steps.
         */
        if (pData->rdh.nCount == 2000) {
          HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER)
                                   + (sizeof(RECT)*maxRects), pData);
          if (hRgn) {
            CombineRgn(hRgn, hRgn, h, RGN_OR);
            DeleteObject(h);
          } else hRgn = h;
          pData->rdh.nCount = 0;
          SetRect(&pData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
        }
      }
    }

    /* Go to next row */
    p8 += bpl;
  }

  /* Create or extend the region with the remaining rectangles*/
  HRGN h = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER)
		      + (sizeof(RECT)*maxRects), pData);
  if (hRgn) {
    CombineRgn(hRgn, hRgn, h, RGN_OR);
    DeleteObject(h);
  } else hRgn = h;

  free(pData); // I've created the region so I can free this now, right?

  return hRgn;

}
#endif

//
// End of "$Id: ShapedWindow.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
