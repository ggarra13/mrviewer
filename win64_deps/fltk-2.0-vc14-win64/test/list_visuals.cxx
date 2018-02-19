//
// "$Id: list_visuals.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Visual list utility for the Fast Light Tool Kit (FLTK).
//
// List all the visuals on the screen, and dumps anything interesting
// about them to stdout.
//
// Does not use fltk.
//
// This file may be #included in another program to make a function to
// call to list the visuals.  Fl.h must be included first to indicate this.
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

#include <config.h>

#if !(USE_X11)
#include <fltk/error.h>

int main(int, char**) {
  fltk::fatal("Only X has this sillyness.");
  return 1;
}

#else

#include <config.h>
#include <fltk/x.h>
#include <stdio.h>
#include <stdlib.h>

const char *ClassNames[] = {
  "StaticGray ",
  "GrayScale  ",
  "StaticColor",
  "PseudoColor",
  "TrueColor  ",
  "DirectColor"
};

// SERVER_OVERLAY_VISUALS property element:
typedef struct _OverlayInfo {
  long overlay_visual;
  long transparent_type;
  long value;
  long layer;
} OverlayInfo;

#if HAVE_MULTIBUF
#define Window XWindow
#include <X11/extensions/multibuf.h>
#undef Window
#endif

#if HAVE_XDBE
#define Window XWindow
#include <X11/extensions/Xdbe.h>
#undef Window
#endif


static void print_mask(XVisualInfo* p) {
  int n = 0;
  int what = 0;
  int print_anything = 0;
  char buf[20];
  char *q = buf;
  *q = 0;
  int b; unsigned int m; for (b=32,m=0x80000000; ; b--,m>>=1) {
    int new_what = 0;
    if (p->red_mask&m) new_what = 'r';
    else if (p->green_mask&m) new_what = 'g';
    else if (p->blue_mask&m) new_what = 'b';
    else new_what = '?';
    if (new_what != what) {
      if (what && (what != '?' || print_anything)) {
	q += sprintf(q,"%d%c", n, what);
	print_anything = 1;
      }
      what = new_what;
      n = 1;
    } else {
      n++;
    }
    if (!b) break;
  }
  printf("%7s", buf);
}

void list_visuals() {
  using namespace fltk;
  fltk::open_display();
  XVisualInfo vTemplate;
  int num;
  XVisualInfo *visualList = XGetVisualInfo(xdisplay,0,&vTemplate,&num);

  XPixmapFormatValues *pfvlist;
  static int numpfv;
  pfvlist = XListPixmapFormats(xdisplay, &numpfv);

  OverlayInfo *overlayInfo = 0;
  int numoverlayinfo = 0;
  Atom overlayVisualsAtom = XInternAtom(xdisplay,"SERVER_OVERLAY_VISUALS",1);
  if (overlayVisualsAtom) {
    unsigned long sizeData, bytesLeft;
    Atom actualType;
    int actualFormat;
    if (!XGetWindowProperty(xdisplay, RootWindow(xdisplay, xscreen),
			   overlayVisualsAtom, 0L, 10000L, False,
			   overlayVisualsAtom, &actualType, &actualFormat,
			   &sizeData, &bytesLeft,
			   (unsigned char **)(void*)&overlayInfo))
      numoverlayinfo = int(sizeData/4);
  }

#if HAVE_MULTIBUF
  int event_base, error_base;
  XmbufBufferInfo *mbuf, *sbuf;
  int nmbuf = 0, nsbuf = 0;
  if (XmbufQueryExtension(xdisplay,&event_base, &error_base)) {
    XmbufGetScreenInfo(xdisplay,RootWindow(xdisplay,xscreen),
		       &nmbuf, &mbuf, &nsbuf, &sbuf);
  }
#endif

#if HAVE_XDBE
  int event_base, error_base;
  int numdouble = 0;
  XdbeVisualInfo *dbe = 0;
  if (XdbeQueryExtension(xdisplay, &event_base, &error_base)) {
    XWindow root = RootWindow(xdisplay,xscreen);
    int numscreens = 1;
    XdbeScreenVisualInfo *a = XdbeGetVisualInfo(xdisplay,&root,&numscreens);
    if (!a) printf("error getting double buffer visuals\n");
    else {
      dbe = a->visinfo;
      numdouble = a->count;
    }
  }
#endif

  for (int i=0; i<num; i++) {
    XVisualInfo *p = visualList+i;

    XPixmapFormatValues *pfv;
    for (pfv = pfvlist; ; pfv++) {
      if (pfv >= pfvlist+numpfv) {pfv = 0; break;} // should not happen!
      if (pfv->depth == p->depth) break;
    }

    int j = pfv ? pfv->bits_per_pixel : 0;
    printf(" %2ld: %s %2d/%d", p->visualid, ClassNames[p->c_class],
	   p->depth, j);
    if (j < 10) putchar(' ');

    print_mask(p);

    for (j=0; j<numoverlayinfo; j++) {
      OverlayInfo *o = &overlayInfo[j];
      if (o->overlay_visual == long(p->visualid)) {
	printf(" overlay(");
	if (o->transparent_type==1) printf("transparent pixel %ld, ",o->value);
	else if (o->transparent_type==2) printf("transparent mask %ld, ",o->value);
	else printf("opaque, ");
	printf("layer %ld)", o->layer);
      }
    }

#if HAVE_MULTIBUF
    for (j=0; j<nmbuf; j++) {
      XmbufBufferInfo *m = &mbuf[j];
      if (m->visualid == p->visualid)
	printf(" multibuffer(%d)", m->max_buffers);
    }
    for (j=0; j<nsbuf; j++) {
      XmbufBufferInfo *m = &sbuf[j];
      if (m->visualid == p->visualid)
	printf(" stereo multibuffer(%d)", m->max_buffers);
    }
#endif

#if HAVE_XDBE
    for (j = 0; j < numdouble; j++) if (dbe[j].visual == p->visualid)
      printf(" doublebuf(perflevel %d)",dbe[j].perflevel);
#endif

    if (p->visualid==XVisualIDFromVisual(DefaultVisual(xdisplay,xscreen)))
      printf(" (default visual)");

    putchar('\n');
  }
}

#ifndef fltk_run_h
#include <fltk/run.h>
int main(int argc, char **argv) {
  if (argc == 1);
  else if (argc == 2 && argv[1][0]!='-') fltk::display(argv[1]);
  else {fprintf(stderr,"usage: %s <display>\n",argv[0]); exit(1);}
  list_visuals();
  return 0;
}
#endif
#endif

//
// End of "$Id: list_visuals.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
