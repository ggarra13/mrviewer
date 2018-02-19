//
// "$Id: list_fonts.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// MacOS font utilities for the Fast Light Tool Kit (FLTK).
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

#include <fltk/x.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <fltk/string.h>

using namespace fltk;

int Font::encodings(const char**& arrayp) {
  // CET - FIXME - What about this encoding stuff?
  // WAS: we need some way to find out what charsets are supported
  // and turn these into ISO encoding names, and return this list.
  // This is a poor simulation:
  static const char* simulation[] = {"iso8859-1", 0};
  arrayp = simulation;
  return 1;
}

////////////////////////////////////////////////////////////////
// List sizes:

int Font::sizes(int*& sizep) {
  static int simulation[1] = {0};
  sizep = simulation;
  return 1;
}

#if 0
// Code used by fltk1.1:
static int array[128];
int Fl::get_font_sizes(Fl_Font fnum, int*& sizep) {
  Fl_Fontdesc *s = fl_fonts+fnum;
  if (!s->name) s = fl_fonts; // empty slot in table, use entry 0

  Str255 name;
  int len = strlen( s->name ); strncpy( (char*)(name+1), s->name+1, len ); name[0] = len-1;
  FMFontFamily family = FMGetFontFamilyFromName( name );
  if ( family == kInvalidFontFamily ) return 0;

  sizep = array;
  FMFont font;
  FMFontStyle style, fStyle;
  switch ( s->name[0] ) {
    default :
      fStyle=0;
      break;
    case 'B' : 
      fStyle=1;
      break;
    case 'I' : 
      fStyle=2;
      break;
    case 'P' :
      fStyle=3;
      break;
  }
  FMFontSize size, pSize = -1;
  FMFontFamilyInstanceIterator ffiIterator;
  FMCreateFontFamilyInstanceIterator( family, &ffiIterator );
  int cnt = 0;
  OSStatus listInstances;
  for (;;)
  {
    listInstances = FMGetNextFontFamilyInstance( &ffiIterator, &font, &style, &size );
    if ( listInstances != 0 ) break;
    if ( style==fStyle )
    {
      if ( size>pSize ) 
      {
        array[ cnt++ ] = size;
        pSize = size;
      }
    }
  }
  FMDisposeFontFamilyInstanceIterator( &ffiIterator );

  return cnt;
}
#endif

////////////////////////////////////////////////////////////////
// list fonts:

extern "C" {
static int sort_function(const void *aa, const void *bb) {
  fltk::Font* a = *(fltk::Font**)aa;
  fltk::Font* b = *(fltk::Font**)bb;
  int ret = strcasecmp(a->name_, b->name_); if (ret) return ret;
  return a->attributes_ - b->attributes_;
}}

extern Font* fl_make_font(const char* name, int attrib);

static Font** font_array = 0;
static int num_fonts = 0;
static int array_size = 0;

int fltk::list_fonts(Font**& arrayp) {
  if (font_array) {arrayp = font_array; return num_fonts;}
  //FMFontFamilyInstanceIterator ffiIterator;
  //FMFont font;
  //FMFontStyle style; // bits 0..6: bold, italic, underline, outline, shadow, condens, extended (FLTK supports 0 and 1 )
  //FMFontSize size;
  //FMFilter filter; // do we need to set a specific (or multiple) filter(s) to get ALL fonts?
  
  //filter.format = kFMCurrentFilterFormat;
  //filter.selector = kFMGenerationFilterSelector;
  //filter.filter.generationFilter = 

  FMFontFamilyIterator ffIterator;
  FMCreateFontFamilyIterator(NULL,NULL, kFMUseGlobalScopeOption, &ffIterator);
  for (;;) {
    FMFontFamily family;
    if (FMGetNextFontFamily( &ffIterator, &family ) != 0) break;
    Str255 buf;
    FMGetFontFamilyName( family, buf );
    buf[ buf[0]+1 ] = 0;
    if (!isalpha(buf[1])) continue;
    //printf( "Font Family: %s\n", buf+1 );
    if (num_fonts >= array_size) {
      array_size = array_size ? 2*array_size : 128;
      font_array = (Font**)realloc(font_array, array_size*sizeof(Font*));
    }
    // Style enumeration could be used here to figure out if there is
    // a bold or italic version, if not fl_make_font can be altered to
    // not create them:
    font_array[num_fonts++] = fl_make_font((char*)buf+1, 0);
  }
  FMDisposeFontFamilyIterator( &ffIterator );
  qsort(font_array, num_fonts, sizeof(*font_array), sort_function);
  arrayp = font_array;
  return num_fonts;
}

//
// End of "$Id: list_fonts.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
