/*
  File:       iccCreateProbeImages.cpp
 
  Contains:   Command-line app that takes the pathname of a directory
  and creates two images in it, the pixels of which are
  a flattened 52 x 52 x 52 cube.  The content of the images
  differs only in that one is 8-bit-integer-per-component TIFF,
  and the other is 32-bit-floating-point-per-component TIFF.
 
  These images can be scaled up or down, respectively, to the
  bit depth appropriate for the application (e.g. one could
  read a 32-bit TIFF into Adobe After Effects and write it out as
  a 10-bit-per-component Kodak Cineon or SMPTE DPX file.)
 
  The intent is that these images will be taken through the
  normal path through the non-ICC color management system which
  is being probed; the displayed images will be screen-grabbed;
  and the screen grab will be analyzed with a companion
  program included with this package, allowing for the creation
  of an input profile for those (e.g) Cineon or DPX files
  which when assigned in an ICC-compliant application such
  as Adobe Photoshop will yield a color appearance match to
  the same image displayed in the non-ICC color management system.
 
  At the moment this code is specific to Mac OS X 10.4 and up.
  Contributions of counterpart code for linux and/or Windows
  would be very seriously considered for inclusion here, including
  cross-platform versions using libtiff.
 
  Version:    V1
 
  Copyright:  � see below
*/

/*
 * The ICC Software License, Version 0.2
 *
 *
 * Copyright (c) 2003-2010 The International Color Consortium. All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. In the absence of prior written permission, the names "ICC" and "The
 *    International Color Consortium" must not be used to imply that the
 *    ICC organization endorses or promotes products derived from this
 *    software.
 *
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNATIONAL COLOR CONSORTIUM OR
 * ITS CONTRIBUTING MEMBERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the The International Color Consortium. 
 *
 *
 * Membership in the ICC is encouraged when this software is used for
 * commercial purposes. 
 *
 *  
 * For more information on The International Color Consortium, please
 * see <http://www.color.org/>.
 *  
 * 
 */

////////////////////////////////////////////////////////////////////// 
// HISTORY:
//
// -Initial implementation by Joseph Goldstone spring 2007
//
//////////////////////////////////////////////////////////////////////

#include <iostream>
using namespace std;
#include "Vetters.h"

#ifdef __APPLE__

#include "ApplicationServices/ApplicationServices.h"

CGColorSpaceRef
getDeviceRGBColorSpace()
{
  static CGColorSpaceRef deviceRGB = NULL;
  if (deviceRGB == NULL)
    deviceRGB = CGColorSpaceCreateDeviceRGB();
  return deviceRGB;
}

#define BEST_BYTE_ALIGNMENT 16
#define BEST_BYTES_PER_ROW(bpr)\
( ( (bpr) + (BEST_BYTE_ALIGNMENT-1) ) & ~(BEST_BYTE_ALIGNMENT-1) )

CGContextRef
createRGBBitmapContext(size_t width, size_t height, bool deep)
{
  CGContextRef context;
  
  CGFloat black[4] = { 0.0, 0.0, 0.0, 1.0 };
  size_t bitsPerComponent = deep ? 32 : 8;
  // somewhat problematic: will fail for "thousands of colors" but who uses
  // that setting, these days?
  size_t bytesPerRow  = BEST_BYTES_PER_ROW(width * 4 * bitsPerComponent / 8);
  CGBitmapInfo bitmapInfo = kCGImageAlphaNoneSkipLast
                          | (deep ? kCGBitmapFloatComponents : 0);
  
  unsigned char* data;
  data = static_cast<unsigned char*>(calloc(1, bytesPerRow * height));
  if (data == NULL)
  {
    fprintf(stderr, "Couldn't allocate memory for RGB bitmap context\n");
    return NULL;
  }
  
  context = CGBitmapContextCreate(data, width, height, bitsPerComponent,
                                  bytesPerRow, getDeviceRGBColorSpace(),
                                  bitmapInfo);
  if (context == NULL)
  {
    free(data);
    fprintf(stderr, "Couldn't create RGB bitmap context\n");
    return NULL;
  }
  
  CGColorRef blackColor = CGColorCreate(getDeviceRGBColorSpace(), black);
  CGContextSaveGState(context);
  CGContextSetFillColorWithColor(context, blackColor);
  CGContextFillRect(context, CGRectMake(0, 0, width, height));
  CGContextRestoreGState(context);
  
  return context;
}

void
drawBoundary(CGContextRef context, size_t width, size_t height)
{
  CGContextSetFillColorSpace(context, getDeviceRGBColorSpace());
  CGRect rect;
  rect.origin.x = 0;
  rect.origin.y = 0;
  rect.size.width = width;
  rect.size.height = height;
  CGContextSetRGBFillColor(context, 0.0, 0.0, 0.0, 1.0);
  CGContextFillRect(context, rect);
  rect.origin.x++;
  rect.origin.y++;
  rect.size.width -= 2;
  rect.size.height -= 2;
  CGContextSetRGBFillColor(context, 1.0, 1.0, 1.0, 1.0);
  CGContextFillRect(context, rect);
  rect.origin.x++;
  rect.origin.y++;
  rect.size.width -= 2;
  rect.size.height -= 2;
  CGContextSetRGBFillColor(context, 0.0, 0.0, 0.0, 1.0);
  CGContextFillRect(context, rect);
}

void
drawBackground(CGContextRef context, size_t width, size_t height)
{
  CGContextSetFillColorSpace(context, getDeviceRGBColorSpace());
  CGRect rect;
  rect.origin.x = 0;
  rect.origin.y = 0;
  rect.size.width = width;
  rect.size.height = height;
  // Why Magenta? Because it's not a commonly-used UI color on the Mac.
  // (I almost said the same thing about cyan, but consider Aqua UI controls.)
  CGContextSetRGBFillColor(context, 1.0, 0.0, 1.0, 1.0);
  CGContextFillRect(context, rect);
}

void
drawContents(CGContextRef context, CGRect contentRect, size_t edgeSize)
{
  CGContextSetFillColorSpace(context, getDeviceRGBColorSpace());
  size_t r = 0;
  size_t g = 0;
  size_t b = 0;
  CGRect pixelRect;
  pixelRect.origin.x = contentRect.origin.x;
  pixelRect.origin.y = contentRect.origin.y + contentRect.size.height - 1;
  pixelRect.size.width = 1.0;
  pixelRect.size.height = 1.0;
  for (r = 0; r < edgeSize; ++r)
  {
    float rFrac = r / (edgeSize - 1.0);
    for (g = 0; g < edgeSize; ++g)
    {
      float gFrac = g / (edgeSize - 1.0);
      for (b = 0; b < edgeSize; ++b)
      {
        float bFrac = b / (edgeSize - 1.0);
        CGContextSetRGBFillColor(context, rFrac, gFrac, bFrac, 1.0);
        CGContextFillRect(context, pixelRect);
        ++pixelRect.origin.x;
        if (pixelRect.origin.x - contentRect.origin.x >= contentRect.size.width)
        {
          pixelRect.origin.x = contentRect.origin.x;
          --pixelRect.origin.y;
        }
      }
    }
  }
  // Now leave a marker showing the length of the (now flattened) cube
  pixelRect.origin.x = contentRect.origin.x;
  --pixelRect.origin.y;
  for (size_t x = 0; x < edgeSize; ++x)
  {
    CGContextSetRGBFillColor(context, 1.0, 1.0, 1.0, 1.0);
    CGContextFillRect(context, pixelRect);
    ++pixelRect.origin.x;
  }
}

void
exportImage(CGImageRef image, CFURLRef url, CFStringRef outputFormat)
{
  CGImageDestinationRef imageDestination
    = CGImageDestinationCreateWithURL(url, outputFormat, 1, NULL);
  if (imageDestination == NULL)
  {
    fprintf(stderr, "Couldn't create image destination.\n");
    return;
  }
  CGImageDestinationAddImage(imageDestination, image, NULL);
  CGImageDestinationFinalize(imageDestination);
  CFRelease(imageDestination);
}

void
usage(ostream& s, const char* const myName, unsigned int N)
{
  cout << myName << ": usage is " << myName << " [N]\n"
       << "where N is an optional argument indicating the number of"
       << " points along the edge of the color cube being flattened into the"
       << " probe image; if not specified, N defaults to " << N << "\n"
       << "Two files are created in /var/tmp:\n"
       << "  32bpcProbe.tiff (32-bit-per-component unwrapped NxNxN cube)\n"
       << "and\n"
       << "  8bpcProbe.tiff (8-bit-per-component unwrapped NxNxN cube)\n"
       << "A simple After Effects script, or some other tested tool, should"
       << " be used to turn one of these into a 10-bit log DPX or Cineon file."
       << endl;
}
#endif /* __APPLE__ */

int
main(int argc, char* argv[])
{
#ifdef __APPLE__
  const unsigned int DEFAULT_N = 52;
  
  const char* const myName = path_tail(argv[0]);
  if (argc > 2)
  {
    usage(cout, myName, DEFAULT_N);
    return EXIT_FAILURE;
  }
  bool deep = false;
  
  size_t borderSize = 20;
  size_t contentWidth = 560;
  size_t contentHeight = 280;
  size_t edgeSize = argc == 3 ? atoi(argv[2]) : 52;
  
  size_t bitmapWidth = contentWidth + 2 * borderSize;
  size_t bitmapHeight = contentHeight + 2 * borderSize;
  
  CGContextRef bitmapContext = createRGBBitmapContext(bitmapWidth,
                                                      bitmapHeight, deep);
  if (bitmapContext == NULL)
  {
    fprintf(stderr, "Couldn't create bitmapContext.\n");
    return EXIT_FAILURE;
  }
  
  CGContextSetAllowsAntialiasing(bitmapContext, false);
//  drawBoundary(bitmapContext, bitmapWidth, bitmapHeight);
  drawBackground(bitmapContext, bitmapWidth, bitmapHeight);
  CGRect contentRect = CGRectMake(borderSize, borderSize, contentWidth,
                                  contentHeight);
  drawContents(bitmapContext, contentRect, edgeSize);
  
  CGImageRef image = CGBitmapContextCreateImage(bitmapContext);
  CFStringRef    deepURLString = CFSTR("file:///var/tmp/32bpcProbe.tiff");
  CFStringRef shallowURLString = CFSTR("file:///var/tmp/8bpcProbe.tiff");
  CFURLRef    deepURL = CFURLCreateWithString(NULL,    deepURLString, NULL);
  CFURLRef shallowURL = CFURLCreateWithString(NULL, shallowURLString, NULL);
  exportImage(image,    deepURL, kUTTypeTIFF);
  exportImage(image, shallowURL, kUTTypeTIFF);
#endif /* __APPLE__ */
  return EXIT_SUCCESS;
}

