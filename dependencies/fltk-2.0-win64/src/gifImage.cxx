// "$Id: gifImage.cxx 5863 2007-05-31 13:38:07Z sanel.z $"

/*! \class fltk::gifImage

  This can either display an image from a .gif image file, or from a
  block of data that is the contents of the image file. By using a
  block of data the image can be compiled into a program, this is whta
  Fluid does to imbed gif images into the source code and is currently
  the most efficient way to get a Fluid program to produce an image.

  To use data, pass a non-null pointer as the second argument to the
  constructor. The filename is ignored in this case, but the "guess image"
  code uses this to identify identical images and reuse them (nyi)
*/

// Extensively modified from original code for gif2ras by
// Patrick J. Naughton of Sun Microsystems.  The original
// copyright notice follows:

/* gif2ras.c - Converts from a Compuserve GIF (tm) image to a Sun Raster image.
 *
 * Copyright (c) 1988 by Patrick J. Naughton
 *
 * Author: Patrick J. Naughton
 * naughton@wind.sun.com
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * Comments and additions should be sent to the author:
 *
 *                     Patrick J. Naughton
 *                     Sun Microsystems, Inc.
 *                     2550 Garcia Ave, MS 14-40
 *                     Mountain View, CA 94043
 * (415) 336-1080 */

#include <config.h>
#include <fltk/SharedImage.h>
#include <fltk/x.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace fltk;

#define NEXTBYTE (dat? *dat++ : getc(GifFile))
#define GETSHORT(var) var = NEXTBYTE; var += NEXTBYTE << 8

/*! Tests block of data to see if it looks like the start of a .gif file. */
bool gifImage::test(const uchar *datas, unsigned size)
{
  return !strncmp((char*) datas,"GIF", 3);
}

bool gifImage::fetch()
{
  const uchar* dat = datas;
  FILE *GifFile=0;

  if (dat) {
    dat += 6;
  } else { // set up to read from file, quit silently on any errors:
    GifFile=fopen(get_filename(), "rb");
    if (!GifFile)
      return false;
    char b[6];
    if (!GifFile || fread(b,1,6,GifFile) < 6 ||
	b[0]!='G' || b[1]!='I' || b[2] != 'F') {
      fclose(GifFile);
      return false;
    }
  }

  int inumber=0; // which image from animated gif file to read

  int Width; GETSHORT(Width);
  int Height; GETSHORT(Height);
  if (Width <= 0 || Height <= 0) {
    fclose(GifFile);
    return false;
  }

  uchar ch = NEXTBYTE;
  char HasColormap = ((ch & 0x80) != 0);
  int BitsPerPixel = (ch & 7) + 1;
  int ColorMapSize = 1 << BitsPerPixel;
  // int OriginalResolution = ((ch>>4)&7)+1;
  // int SortedTable = (ch&8)!=0;
  NEXTBYTE; // Background Color index
  NEXTBYTE; // Aspect ratio is N/64

  // Read in global colormap:
  uchar transparent_pixel = 0;
  char has_transparent = 0;
  U32 colormap[256];

  if (HasColormap) {
    for (int i=0; i < ColorMapSize; i++) {
      U32 r = NEXTBYTE;
      U32 g = NEXTBYTE;
      U32 b = NEXTBYTE;
      colormap[i] = 0xff000000|(r<<16)|(g<<8)|b;
    }
  } else {
    //    fprintf(stderr,"%s does not have a colormap.\n", infname);
    for (int i = 0; i < ColorMapSize; i++)
      colormap[i] = 0xff000000|(0x10101*i);
  }

  int CodeSize;		/* Code size, init from GIF header, increases... */
  char Interlace;

  for (;;) {

    int i = NEXTBYTE;
    if (i<0) {
      // fprintf(stderr,"%s: unexpected EOF\n",infname);
      return false;
    }
    int blocklen;

    //  if (i == 0x3B) return;  eof code

    if (i == 0x21) {		// a "gif extension"

      ch = NEXTBYTE;
      blocklen = NEXTBYTE;

      if (ch==0xF9 && blocklen==4) { // Netscape animation extension

	char bits;
	bits = NEXTBYTE;
	NEXTBYTE; NEXTBYTE; // GETSHORT(delay);
	transparent_pixel = NEXTBYTE;
	if (bits & 1) has_transparent = 1;
	blocklen = NEXTBYTE;

      } else if (ch == 0xFF) { // Netscape repeat count
	;

      } else if (ch == 0xFE) { //Gif Comment
#if 0
	if(blocklen>0) {
	  char *comment=new char[blocklen+1];
	  int l;
	  for(l=0;blocklen;l++,blocklen--)
	    comment[l]=NEXTBYTE;
	  comment[l]=0;
	  //fprintf(stderr,"%s: Gif Comment: '%s'\n", infname, comment);
	  delete[] comment;
	  NEXTBYTE; //End marker
	}
#endif
      } else {
	//fprintf(stderr,"%s: unknown gif extension 0x%02x\n", infname, ch);

      }

    } else if (i == 0x2c) {	// an image

      NEXTBYTE; NEXTBYTE; // GETSHORT(x_position);
      NEXTBYTE; NEXTBYTE; // GETSHORT(y_position);
      GETSHORT(Width);
      GETSHORT(Height);
      ch = NEXTBYTE;
      Interlace = ((ch & 0x40) != 0);
      if (ch&0x80) {
	// read local color map
	int n = 2 << (ch&7);
	for (i=0; i < n; i++) {
          U32 r = NEXTBYTE;
          U32 g = NEXTBYTE;
          U32 b = NEXTBYTE;
          colormap[i] = 0xff000000|(r<<16)|(g<<8)|b;
	}
      }
      CodeSize = NEXTBYTE+1;
      if (!inumber--) break; // okay, this is the image we want
      blocklen = NEXTBYTE;

    } else {
      //fprintf(stderr,"%s: unknown gif code 0x%02x\n", infname, i);
      blocklen = 0;
    }

    // skip the data:
    while (blocklen>0) {while (blocklen--) {NEXTBYTE;} blocklen=NEXTBYTE;}
  }

  if (BitsPerPixel >= CodeSize) {
    // Workaround for broken GIF files...
    BitsPerPixel = CodeSize - 1;
    ColorMapSize = 1 << BitsPerPixel;
  }

  uchar *Image = new uchar[Width*Height];
  if (!Image) {
    //fprintf (stderr, "Insufficient memory\n");
    return false;
  }
  int YC = 0, Pass = 0; /* Used to de-interlace the picture */
  uchar *p = Image;
  uchar *eol = p+Width;

  int InitCodeSize = CodeSize;
  int ClearCode = (1 << (CodeSize-1));
  int EOFCode = ClearCode + 1;
  int FirstFree = ClearCode + 2;
  int FinChar = 0;
  int ReadMask = (1<<CodeSize) - 1;
  int FreeCode = FirstFree;
  int OldCode = ClearCode;

  // tables used by LZW decompresser:
  short int Prefix[4096];
  uchar Suffix[4096];

  int blocklen = NEXTBYTE;
  uchar thisbyte = NEXTBYTE; blocklen--;
  int frombit = 0;

  for (;;) {

/* Fetch the next code from the raster data stream.  The codes can be
 * any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
 * maintain our location as a pointer and a bit offset.
 * In addition, gif adds totally useless and annoying block counts
 * that must be correctly skipped over. */
    int CurCode = thisbyte;
    if (frombit+CodeSize > 7) {
      if (blocklen <= 0) {
	blocklen = NEXTBYTE;
	if (blocklen <= 0) break;
      }
      thisbyte = NEXTBYTE; blocklen--;
      CurCode |= thisbyte<<8;
    }
    if (frombit+CodeSize > 15) {
      if (blocklen <= 0) {
	blocklen = NEXTBYTE;
	if (blocklen <= 0) break;
      }
      thisbyte = NEXTBYTE; blocklen--;
      CurCode |= thisbyte<<16;
    }
    CurCode = (CurCode>>frombit)&ReadMask;
    frombit = (frombit+CodeSize)%8;

    if (CurCode == ClearCode) {
      CodeSize = InitCodeSize;
      ReadMask = (1<<CodeSize) - 1;
      FreeCode = FirstFree;
      OldCode = ClearCode;
      continue;
    }

    if (CurCode == EOFCode) break;

    uchar OutCode[1025]; // temporary array for reversing codes
    uchar *tp = OutCode;
    int i;
    if (CurCode < FreeCode) i = CurCode;
    else if (CurCode == FreeCode) {*tp++ = FinChar; i = OldCode;}
    else {/*fprintf(stderr,"%s : LZW Barf!\n",infname);*/ break;}

    while (i >= ColorMapSize) {*tp++ = Suffix[i]; i = Prefix[i];}
    *tp++ = FinChar = i;
    while (tp > OutCode) {
      *p++ = *--tp;
      if (p >= eol) {
	if (!Interlace) YC++;
	else switch (Pass) {
	case 0: YC += 8; if (YC >= Height) {Pass++; YC = 4;} break;
	case 1: YC += 8; if (YC >= Height) {Pass++; YC = 2;} break;
	case 2: YC += 4; if (YC >= Height) {Pass++; YC = 1;} break;
	case 3: YC += 2; break;
	}
	if (YC>=Height) YC=0; /* cheap bug fix when excess data */
	p = Image + YC*Width;
	eol = p+Width;
      }
    }

    if (OldCode != ClearCode) {
      Prefix[FreeCode] = OldCode;
      Suffix[FreeCode] = FinChar;
      FreeCode++;
      if (FreeCode > ReadMask) {
	if (CodeSize < 12) {
	  CodeSize++;
	  ReadMask = (1 << CodeSize) - 1;
	}
	else FreeCode--;
      }
    }
    OldCode = CurCode;
  }
  if (!datas) fclose(GifFile);

  setsize(Width,Height);
  if (has_transparent) colormap[transparent_pixel] = 0;
  setpixeltype(has_transparent ? fltk::ARGB32 : fltk::RGB32);
  for (int y=0; y<Height; y++) {
    U32* to = (U32*)(linebuffer(y));
    p = Image+y*Width;
    for (int x=0; x<Width; x++) to[x] = colormap[*p++];
    setpixels((uchar*)to, y);
  }
  delete[] Image;
  return true;
}

//
// End of "$Id: gifImage.cxx 5863 2007-05-31 13:38:07Z sanel.z $"
//
