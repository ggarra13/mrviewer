//
// "$Id: list_fonts.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// _WIN32 font utilities for the Fast Light Tool Kit (FLTK).
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

#include <fltk/events.h>
#include <fltk/utf.h>
#include <fltk/x.h>
#include <ctype.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>

using namespace fltk;

extern int has_unicode();

int Font::encodings(const char**& arrayp) {
  // CET - FIXME - What about this encoding stuff?
  // WAS: we need some way to find out what charsets are supported
  // and turn these into ISO encoding names, and return this list.
  // This is a poor simulation:
  static const char* simulation[] = {"iso10646-1", 0};
  arrayp = simulation;
  return 1;
}

////////////////////////////////////////////////////////////////
// List sizes:

static int nbSize;
//static int cyPerInch;
#define MAX_SIZES 16
static int sizes[MAX_SIZES];

static int CALLBACK EnumSizeCb(CONST LOGFONTW* lpelf,
			       CONST TEXTMETRICW* lpntm,
			       DWORD fontType,
			       LPARAM p)
{
  if ((fontType & RASTER_FONTTYPE) == 0) {
    // Scalable font
    sizes[0] = 0;
    nbSize = 1;
    return 0;
  }

  int add = lpntm->tmHeight - lpntm->tmInternalLeading;
  //add = MulDiv(add, 72, cyPerInch); // seems to be correct before this

  int start = 0;
  while ((start < nbSize) && (sizes[start] < add)) start++;

  if ((start < nbSize) && (sizes[start] == add)) return (1);

  for (int i=nbSize; i>start; i--) sizes[i] = sizes[i - 1];

  sizes[start] = add;
  nbSize++;

  // Stop enum if buffer overflow
  return (nbSize < MAX_SIZES);
}

int Font::sizes(int*& sizep) {
  nbSize = 0;
  HDC dc = getDC();
  //cyPerInch = GetDeviceCaps(dc, LOGPIXELSY);
  //if (cyPerInch < 1) cyPerInch = 1;
  if (has_unicode()) {
    wchar_t ucs[1024];
    utf8towc(name_, strlen(name_), ucs, 1024);
#if defined(__BORLANDC__) || defined(__DMC__)
    EnumFontFamiliesW(dc, ucs, (FONTENUMPROCA)EnumSizeCb, 0);
#else
    EnumFontFamiliesW(dc, ucs, EnumSizeCb, 0);
#endif
  } else {
    EnumFontFamiliesA(dc, name_, (FONTENUMPROCA)EnumSizeCb, 0);
  }
  sizep = ::sizes;
  return nbSize;
}

////////////////////////////////////////////////////////////////
// list fonts:

extern "C" {
static int sort_function(const void *aa, const void *bb) {
  fltk::Font* a = *(fltk::Font**)aa;
  fltk::Font* b = *(fltk::Font**)bb;
  int ret = stricmp(a->name_, b->name_); if (ret) return ret;
  return a->attributes_ - b->attributes_;
}}

extern Font* fl_make_font(const char* name, int attrib);

static Font** font_array = 0;
static int num_fonts = 0;
static int array_size = 0;

static int CALLBACK enumcbW(CONST LOGFONTW* lplf,
                           CONST TEXTMETRICW* lpntm,
                           DWORD fontType,
                           LPARAM p)
{
  // we need to do something about different encodings of the same font
  // in order to match X!  I can't tell if each different encoding is
  // returned sepeartely or not.  This is what fltk 1.0 did:
  if (lplf->lfCharSet != ANSI_CHARSET) return 1;
  const wchar_t *name = lplf->lfFaceName;
  //const char *name = (const char*)(((ENUMLOGFONT *)lplf)->elfFullName);
  char buffer[1024];
  utf8fromwc(buffer, 1024, name, wcslen(name));
  // ignore mystery garbage font names:
  if (buffer[0] == '@') return 1;

  if (num_fonts >= array_size) {
    array_size = array_size ? 2*array_size : 128;
    font_array = (Font**)realloc(font_array, array_size*sizeof(Font*));
  }
  int attrib = 0;
//    if (lplf->lfWeight > 400 || strstr(name, " Bold") == name+strlen(name)-5)
//      attrib = BOLD;
  font_array[num_fonts++] = fl_make_font(buffer, attrib);
  return 1;
}

static int CALLBACK enumcbA(CONST LOGFONT* lplf,
                           CONST TEXTMETRIC* lpntm,
                           DWORD fontType,
                           LPARAM p)
{
  // we need to do something about different encodings of the same font
  // in order to match X!  I can't tell if each different encoding is
  // returned sepeartely or not.  This is what fltk 1.0 did:
  //if (lplf->lfCharSet != ANSI_CHARSET) return 1;
  const char *name = lplf->lfFaceName;
  if (num_fonts >= array_size) {
    array_size = array_size ? 2*array_size : 128;
    font_array = (Font**)realloc(font_array, array_size*sizeof(Font*));
  }
  int attrib = 0;
  font_array[num_fonts++] = fl_make_font(name, attrib);
  return 1;
}


int fltk::list_fonts(Font**& arrayp) {
  if (font_array) {arrayp = font_array; return num_fonts;}
  HDC dc = getDC();

  if (has_unicode()) {
    LOGFONTW lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;
    EnumFontFamiliesExW(dc, &lf, (FONTENUMPROCW)enumcbW, 0, 0);
  } else {
    LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfCharSet = DEFAULT_CHARSET;
    EnumFontFamiliesExA(dc, &lf, (FONTENUMPROCA)enumcbA, 0, 0);
  }
  ReleaseDC(0, dc);
  qsort(font_array, num_fonts, sizeof(*font_array), sort_function);
  arrayp = font_array;
  return num_fonts;
}

////////////////////////////////////////////////////////////////
// This function apparently is needed to translate some font names
// stored in setup files to the actual name of a font. Currently
// font(name) calls this.

#if defined(WIN32) && !defined(__CYGWIN__)
static const char* GetFontSubstitutes(const char* name,int& len)
{
  static char subst_name[1024]; //used BUFLEN from bool fltk_theme()

  if ( strstr(name,"MS Shell Dlg") ||  strstr(name,"Helv")  ||
       strstr(name,"Tms Rmn")) {
    DWORD type = REG_SZ;
    LONG  err;
    HKEY  key;
    char truncname[1024];
    strncpy(truncname, name, len);
    truncname[len] = 0;
    err = RegOpenKey(
      HKEY_LOCAL_MACHINE,
      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes",
      &key );

    if (err == ERROR_SUCCESS) {
      DWORD L=1024;
      err = RegQueryValueEx( key, truncname, 0L, &type, (BYTE*)
                             subst_name, &L);
      RegCloseKey(key);
      if ( err == ERROR_SUCCESS ) {
        len = L;
        return subst_name;
      }
    }
  }
  return name;
}
#endif

//
// End of "$Id: list_fonts.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
