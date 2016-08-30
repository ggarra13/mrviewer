//
// "$Id: setcolor.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// _WIN32 color functions for the Fast Light Tool Kit (FLTK).
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

// This file does not compile independently, it is included by color.cxx

/** The color picked by the most recent setcolor(Color). */
COLORREF	fltk::current_xpixel;

/**
If non-zero this is the palette alloced by fltk on an 8-bit
screen. Hopefully you can ignore this, I'm not even sure it
works anymore.
*/
HPALETTE	fltk::xpalette;

static HBRUSH	current_brush;
static COLORREF	brush_for_color;
static HDC	brush_for_dc;

static HPEN	current_pen;
static COLORREF	pen_for_color;
static HDC	pen_for_dc;

#undef USE_STOCK_BRUSH
#define USE_STOCK_BRUSH 1

COLORREF fltk::xpixel(Color i) {
  int index = i;
  COLORREF rgb;
  // this is the same as nearest_color(i) but it also sets rgb:
  if (i & 0xffffff00) { // an rgb color
    uchar r = i>>24;
    uchar g = i>>16;
    uchar b = i>> 8;
    rgb = RGB(r,g,b);
    //if (r == g && r == b) index = GRAY00+r*(GRAY99-GRAY00)/256;
    index = (BLACK + (b*5/256 * 5 + r*5/256) * 8 + g*8/256);
  } else {
    unsigned c = cmap[i];
    rgb = RGB(uchar(c>>24), uchar(c>>16), uchar(c>>8));
  }
#if USE_COLORMAP
  if (xpalette) rgb = PALETTEINDEX(index);
#endif
  return rgb;
}

// These are the fltk setting converted into Win32 versions:
static int lstyle = 0;
static DWORD dash_pattern[16];
static int dash_pattern_size = 0;
static int line_width_i = 0;

static void free_pen()
{
  SelectObject(pen_for_dc, GetStockObject(WHITE_PEN)); // Release our pen from dc
  DeleteObject(current_pen);
  current_pen = 0;
}

void fltk::line_style(int style, float width, const char* dashes) {
  line_style_ = style;
  line_width_ = width;
  line_dashes_ = dashes;
  static DWORD Cap[4]= {PS_ENDCAP_ROUND, PS_ENDCAP_FLAT, PS_ENDCAP_ROUND, PS_ENDCAP_SQUARE};
  static DWORD Join[4]={PS_JOIN_ROUND, PS_JOIN_MITER, PS_JOIN_ROUND, PS_JOIN_BEVEL};
  if (dashes && dashes[0]) {
    lstyle = PS_USERSTYLE | Cap[(style>>8)&3] | Join[(style>>12)&3];
    int n; for (n = 0; n < 16 && *dashes;) dash_pattern[n++] = *dashes++;
    dash_pattern_size = n;
    // fix cards that ignore dash pattern for zero width:
    //if (!width) width = 1;
  } else {
    dash_pattern_size = 0;
    // allow any low 8 bits for style
    lstyle = style & 0xff | Cap[(style>>8)&3] | Join[(style>>12)&3];
  }
  // for some reason zero width does not work at all:
  line_width_i = int(width); if (!line_width_i) line_width_i = 1;
  if (current_pen) free_pen();
}

#if USE_STOCK_BRUSH

/* Define these if not already done.*/
#ifndef DC_BRUSH
# define DC_BRUSH 18
#endif
#ifndef DC_PEN
# define DC_PEN 19
#endif

static HPEN stockpen = (HPEN)GetStockObject(DC_PEN);
static HBRUSH stockbrush = (HBRUSH)GetStockObject(DC_BRUSH);

typedef COLORREF (WINAPI *pfSetDCPenColor)(HDC hdc, COLORREF crColor);
typedef COLORREF (WINAPI *pfSetDCBrushColor)(HDC hdc, COLORREF crColor);

static pfSetDCPenColor	 __SetDCPenColor = NULL;
static pfSetDCBrushColor __SetDCBrushColor = NULL;
static bool dc_funcs_init = false;

static void load_dc_funcs()
{
  dc_funcs_init = true;
  HINSTANCE hmod = __LoadLibraryW(L"Gdi32.dll");  
  if(hmod) {
    __SetDCPenColor = (pfSetDCPenColor)GetProcAddress(hmod, "SetDCPenColor");
    __SetDCBrushColor = (pfSetDCBrushColor)GetProcAddress(hmod, "SetDCBrushColor");
    FreeLibrary(hmod);
  }
}

#endif /* USE_STOCK_BRUSH */

/** Set the current "pen" in the DC to match the most recent setcolor()
    and line_style() calls. This is stupid-expensive on Windows so
    we defer it until the pen is needed.
*/
HPEN fltk::setpen() {
#if USE_STOCK_BRUSH
  if (!lstyle && line_width_i <= 1) {
    if (!dc_funcs_init) load_dc_funcs();
    if (__SetDCPenColor) {
      SelectObject(dc, stockpen);
      (*__SetDCPenColor)(dc, current_xpixel);
      if (current_pen) free_pen();
      return stockpen;
    }
  }
#endif

  if (!current_pen) goto J1;
  if (pen_for_color != current_xpixel || pen_for_dc != dc) {
    free_pen();
  J1:
    if (lstyle) {
      LOGBRUSH penbrush = {BS_SOLID, current_xpixel, 0};
      current_pen = ExtCreatePen(lstyle|PS_GEOMETRIC, line_width_i, &penbrush,
				 dash_pattern_size,
				 dash_pattern_size ? dash_pattern : 0);
    } else {
      current_pen = CreatePen(PS_SOLID, line_width_i, current_xpixel);
    }
    pen_for_dc = dc;
    pen_for_color = current_xpixel;
  }

  SelectObject(dc, current_pen);	
  return current_pen;
}

static void free_brush()
{
  SelectObject(brush_for_dc, GetStockObject(WHITE_BRUSH)); // Release our brush from dc	
  DeleteObject(current_brush);
  current_brush = 0;
}

/** Set the current "brush" in the DC to match the most recent setcolor()
    and line_style() calls. This is stupid-expensive on Windows so
    we defer it until the brush is needed.
*/
HBRUSH fltk::setbrush() {
#if USE_STOCK_BRUSH
  if (!dc_funcs_init) load_dc_funcs();
  if (__SetDCBrushColor) {
    SelectObject(dc, stockbrush);
    (*__SetDCBrushColor)(dc, current_xpixel); 
    if (current_brush) free_brush();
    return current_brush;
  }
#endif

  if (!current_brush) goto J1;
  if (brush_for_color != current_xpixel || brush_for_dc != dc) {
    free_brush();
  J1:
    current_brush = CreateSolidBrush(current_xpixel);
    brush_for_color = current_xpixel;
    brush_for_dc = dc;
  }

  SelectObject(dc, current_brush);
  return current_brush;
}

void fltk::setcolor(Color i) {
  if (current_color_ != i) {
    current_color_ = i;
    current_xpixel = xpixel(i);
  }
}

// alpha color is not implemented
void fltk::setcolor_alpha(Color color, float alpha) {
  setcolor(color);
}

#if USE_COLORMAP

// 'select_palette()' - Make a color palette for 8-bit displays if necessary
// Thanks to Michael Sweet @ Easy Software Products for this

HPALETTE
fl_select_palette(HDC dc)
{
  static char beenhere;
  if (!beenhere) {
    beenhere = 1;

    //if (GetDeviceCaps(dc, BITSPIXEL) > 8) return NULL;
    int nColors = GetDeviceCaps(dc, SIZEPALETTE);
    if (nColors <= 0 || nColors > 256) return NULL;
    // this will try to work on < 256 color screens, but will probably
    // come out quite badly.

    // I lamely try to get this variable-sized object allocated on stack:
    unsigned long foo[(sizeof(LOGPALETTE)+256*sizeof(PALETTEENTRY))/sizeof(unsigned long)+1];
    LOGPALETTE *pPal = (LOGPALETTE*)foo;

    pPal->palVersion    = 0x300;
    pPal->palNumEntries = nColors;

    // Build 256 colors from the standard FLTK colormap...

    for (int i = 0; i < nColors; i ++) {
      pPal->palPalEntry[i].peRed   = (cmap[i] >> 24) & 255;
      pPal->palPalEntry[i].peGreen = (cmap[i] >> 16) & 255;
      pPal->palPalEntry[i].peBlue  = (cmap[i] >>  8) & 255;
      pPal->palPalEntry[i].peFlags = 0;
    };

    // Create the palette:
    xpalette = CreatePalette(pPal);
  }
  if (xpalette) {
    SelectPalette(dc, xpalette, FALSE);
    RealizePalette(dc);
  }
  return xpalette;
}

#endif

//
// End of "$Id: setcolor.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
