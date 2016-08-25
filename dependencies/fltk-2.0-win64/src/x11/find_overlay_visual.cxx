//
// "$Id: find_overlay_visual.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// X overlay support for the Fast Light Tool Kit (FLTK).
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

// Return an overlay visual, if any.  Also allocate a colormap and
// record the depth for color() to use.
// Another disgusting X interface, based on code extracted and
// purified with great difficulty from XLayerUtil.C:

// This code must somehow be included if USE_OVERLAY or USE_GL_OVERLAY are
// are set in config.h!

#include <fltk/events.h>
#include <fltk/x.h>
using namespace fltk;

// SERVER_OVERLAY_VISUALS property element:
struct OverlayInfo {
  long overlay_visual;
  long transparent_type;
  long value;
  long layer;
};

extern Colormap fl_overlay_colormap;
extern XVisualInfo* fl_overlay_visual;
extern ulong fl_transparent_pixel;

XVisualInfo *fl_find_overlay_visual() {
  static bool beenhere;
  if (beenhere) return fl_overlay_visual;
  beenhere = true;

  open_display();
  Atom overlayVisualsAtom =
    XInternAtom(xdisplay,"SERVER_OVERLAY_VISUALS",1);
  if (!overlayVisualsAtom) return 0;
  OverlayInfo *overlayInfo;
  ulong sizeData, bytesLeft;
  Atom actualType;
  int actualFormat;
  if (XGetWindowProperty(xdisplay, RootWindow(xdisplay, xscreen),
			 overlayVisualsAtom, 0L, 10000L, False,
			 overlayVisualsAtom, &actualType, &actualFormat,
			 &sizeData, &bytesLeft,
			 (unsigned char **) &overlayInfo)) return 0;

  if (actualType == overlayVisualsAtom && actualFormat == 32) {
    int n = int(sizeData/4);
    XVisualInfo* v = 0;
    // find the greatest depth that has a transparent pixel:
    for (int i = 0; i < n; i++) {
      if (overlayInfo[i].transparent_type != 1) continue;
      if (overlayInfo[i].layer <= 0) continue;
      XVisualInfo templt;
      templt.visualid = overlayInfo[i].overlay_visual;
      int num;
      XVisualInfo *v1=XGetVisualInfo(xdisplay, VisualIDMask, &templt, &num);
      if (v1->screen == xscreen && v1->c_class == PseudoColor
	  && (!v || v1->depth > v->depth && v1->depth <= 8)) {
	if (v) XFree((char*)v);
	v = v1;
	fl_transparent_pixel = overlayInfo[i].value;
      } else {
	XFree((char*)v1);
      }
    }
    if (v) {
      fl_overlay_visual = v;
      fl_overlay_colormap = 
	XCreateColormap(xdisplay, RootWindow(xdisplay, xscreen),
			v->visual, AllocNone);
    }
  }
  XFree((char*)overlayInfo);
  //printf("overlay visual %d selected\n", overlay_visual->visualid);
  return fl_overlay_visual;
}

//
// End of "$Id: find_overlay_visual.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
