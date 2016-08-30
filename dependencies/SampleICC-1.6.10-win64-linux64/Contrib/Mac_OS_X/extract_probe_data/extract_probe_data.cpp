/*
  File:       iccExtractValuesFromGrabbedProbeImage.cpp
 
  Contains:   Command-line app that takes the pathname of a screen grab
  of a probe image, and the pixels coordinates within that image
  of the white border of the content area, and extracts from the
  pixels within the border area the non-ICC-color-managed values
  of the (known) probe pixel values, establishing a relationship
  between that non-ICC system's un-color-managed and color-managed
  pixel values.
 
  At the moment this code is specific to Mac OS X 10.4 and up.
  Contributions of counterpart code for linux and/or Windows
  would be very seriously considered for inclusion here, including
  cross-platform versions using libtiff.
 
  Version:    V1
 
  Copyright:  © see below
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

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>

// ------- Mac-specific color space hackery, so that we don't end up having
// the system distort the colors we are reading out of the image in the name
// of its own 'color management'.

// from Gelphman & Laden, pp. 390-391, Listing 12.10
CGColorSpaceRef
getTheDisplayColorSpace()
{
  static CGColorSpaceRef displayCS = NULL;
  if (displayCS == NULL)
  {
    CMProfileRef displayProfile = NULL;
    // Get the display ID of the main display.
    
    // For displays other than the main display, use
    // the functions CGGetDisplaysWithPoint,
    // CGGetDisplaysWithRect, etc. in CGDirectDisplay.h
    CGDirectDisplayID displayID = CGMainDisplayID();
    // The CGDirectDisplayID is the same as the CMDisplayIDType
    // passed to CMGetProfileByAVID
    CMError err = CMGetProfileByAVID((CMDisplayIDType)displayID,
                                     &displayProfile);
    if (err || displayProfile == NULL)
    {
      fprintf(stderr, "Got error %d when getting profile for main display!\n",
              static_cast<int>(err));
      return NULL;
    }
    
    displayCS = CGColorSpaceCreateWithPlatformColorSpace(displayProfile);
    CMCloseProfile(displayProfile);
  }
  return displayCS;
}

// ------- Utilities for finding horizontal or vertical bands of continuous
// pixels. The presumption is that the user, when they grab the probe image,
// has managed to have the grab outline completely within the magenta image
// border region. This should be easy as it is 20 pixels wide...

bool
equalPixels(const unsigned char* const firstPixel,
            const unsigned char* const secondPixel)
{
  return firstPixel[0] == secondPixel[0]
      && firstPixel[1] == secondPixel[1]
      && firstPixel[2] == secondPixel[2];
}

void
getPixel(unsigned const char* const data, size_t fullImageWidth,
         size_t fullImageHeight, size_t bytesPerRow,
         size_t bytesPerPixel, size_t x, size_t y, unsigned char* pixel)
{
  const unsigned char* const p = data + y * bytesPerRow + x * bytesPerPixel;
  pixel[0] = p[0];
  pixel[1] = p[1];
  pixel[2] = p[2];
}

bool
isHorizontalBorderRow(const unsigned char* const data, size_t fullImageWidth,
                      size_t fullImageHeight, size_t bytesPerRow,
                      size_t bytesPerPixel, unsigned char* const borderPixel,
                      size_t y)
{
  unsigned char pixel[3];
  for (size_t x = 0; x < fullImageWidth; ++x)
  {
    getPixel(data, fullImageWidth, fullImageHeight, bytesPerRow, bytesPerPixel,
             x, y, pixel);
    if (! equalPixels(pixel, borderPixel))
      return false;
  }
  return true;
}

bool
isVerticalBorderRow(const unsigned char* const data, size_t fullImageWidth,
                    size_t fullImageHeight, size_t bytesPerRow,
                    size_t bytesPerPixel, unsigned char* const borderPixel,
                    size_t x)
{
  unsigned char pixel[3];
  for (size_t y = 0; y < fullImageHeight; ++y)
  {
    getPixel(data, fullImageWidth, fullImageHeight, bytesPerRow,
             bytesPerPixel, x, y, pixel);
    if (! equalPixels(pixel, borderPixel))
      return false;
  }
  return true;
}

// the bitmap data are arrayed such that lower-index data are at the top
// of the image, and higher-index data at the bottom. So increasing Y values
// from 0 help us find the start of the content, and decreasing Y values from
// fullImageHeight - 1 helps us find the last of the content.

// The last line of the content is not part of the flattened cube, but rather
// is a white strip whose length is the same as the edge size. This lets us
// deduce edge size rather than passing it as a command-line parameter.

void
getContentBoundaries(const unsigned char* const data, size_t fullImageWidth,
                     size_t fullImageHeight, size_t bytesPerRow,
                     size_t bytesPerPixel, size_t* firstContentRow,
                     size_t* lastContentRow, size_t* firstContentColumn,
                     size_t* lastContentColumn)
{
  unsigned char borderPixel[3];
  getPixel(data, fullImageWidth, fullImageHeight, bytesPerRow,
           bytesPerPixel, 0, 0, borderPixel);

  size_t y = 0;
  while (isHorizontalBorderRow(data, fullImageWidth, fullImageHeight,
                               bytesPerRow, bytesPerPixel, borderPixel, y)
         && y < fullImageHeight - 1)
    ++y;
  *firstContentRow = y;
  
  y = fullImageHeight - 1;
  while (isHorizontalBorderRow(data, fullImageWidth, fullImageHeight,
                               bytesPerRow, bytesPerPixel, borderPixel, y)
         && y > 0)
    --y;
  *lastContentRow = y;
  
  size_t x = 0;
  while (isVerticalBorderRow(data, fullImageWidth, fullImageHeight,
                             bytesPerRow, bytesPerPixel, borderPixel, x)
         && x < fullImageWidth - 1)
    ++x;
  *firstContentColumn = x;
  
  x = fullImageWidth - 1;
  while (isVerticalBorderRow(data, fullImageWidth, fullImageHeight,
                             bytesPerRow, bytesPerPixel, borderPixel, x)
         && x > 0)
    --x;
  *lastContentColumn = x;
}

size_t
getEdgeSize(const unsigned char* const data, size_t fullImageWidth,
            size_t fullImageHeight, size_t bytesPerRow, size_t bytesPerPixel,
            size_t firstContentColumn, size_t lastContentColumn,
            size_t lastContentRow)
{
  // not necc. (1.0, 1.0, 1.0) after any color transform
  unsigned char whitePixel[3];
  getPixel(data, fullImageWidth, fullImageHeight, bytesPerRow,
           bytesPerPixel, firstContentColumn, lastContentRow, whitePixel);
  size_t N = 0;
  for (size_t x = firstContentColumn; x <= lastContentColumn; ++x)
  {
    unsigned char pixel[3];
    getPixel(data, fullImageWidth, fullImageHeight, bytesPerRow, bytesPerPixel,
             x, lastContentRow, pixel);
    if (equalPixels(pixel, whitePixel))
      ++N;
    else
      break;
  }
  return N;
}

#define BEST_BYTE_ALIGNMENT 16
#define COMPUTE_BEST_BYTES_PER_ROW(bpr)\
( ( (bpr) + (BEST_BYTE_ALIGNMENT-1) ) & ~(BEST_BYTE_ALIGNMENT-1) )

void
usage(ostream& s, const char* const myName)
{
  s << myName << ": usage is " << myName << " image extracted_data"
  << " where\n"
  << "image is the pathname of the file containing the grabbed probe"
  << " frame\n"
  << "extracted_data is the pathname to which the extracted data will"
  << " be written\n"
  << "example:\n"
  << " extract_probe_data grabbed_frame.tiff extracted_data.txt"
  << endl;
}

#endif /* __APPLE__ */

int
main(int argc, const char * argv[]) {
#ifdef __APPLE__
  const char* const my_name = path_tail(argv[0]);
  if (argc != 3)
  {
    usage(cout, my_name);
    return EXIT_FAILURE;
  }
  const char* const inputImagePath = argv[1];
  vet_input_file_pathname(inputImagePath, "image", "the pathname of a file"
                          " containing the grabbed image of the probe frame");
  
  const char* const outputTextPath = argv[2];
  vet_output_file_pathname(outputTextPath, "extracted_data", "the pathanme of a"
                           " file to which the data extracted from the image"
                           " will be written");
  
  // first arg is name of probe frame file
  // if it's not there, error out.
  // from this compute minimum number of pixels required to contain data.
  // and from that, compute (via sqrt) min run size for edge
  
  // next, read in the image.  (does this have a failure mode?  Don't think so.)
  CFStringRef URLString = CFStringCreateWithCString(NULL,
                                                    inputImagePath,
                                                    kCFStringEncodingASCII);
  
  CFURLRef URL = CFURLCreateWithFileSystemPath(NULL, URLString,
                                               kCFURLPOSIXPathStyle, false);
  if (URL == NULL)
  {
    fprintf(stderr, "Can't create URL.\n");
    return EXIT_FAILURE;
  }
  
  CGImageSourceRef imageSource = CGImageSourceCreateWithURL(URL, NULL);
  if (imageSource == NULL)
  {
    fprintf(stderr, "Couldn't create image source from URL.\n");
    return EXIT_FAILURE;
  }
  CGImageRef fullImage = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
  if (fullImage == NULL)
  {
    fprintf(stderr, "Couldn't create full image from image source.\n");
    return EXIT_FAILURE;
  }
  
  size_t fullImageWidth  = CGImageGetWidth(fullImage);
  size_t fullImageHeight = CGImageGetHeight(fullImage);
  size_t bitsPerComponent = CGImageGetBitsPerComponent(fullImage);
  size_t bytesPerPixel = 4;
  CGBitmapInfo bitmapInfo = CGImageGetBitmapInfo(fullImage);

  size_t fullImageBytesPerRow;
  switch (bitmapInfo)
  {
    case kCGImageAlphaNone:
    case kCGImageAlphaNoneSkipFirst:
    case kCGImageAlphaNoneSkipLast:
    case kCGImageAlphaPremultipliedFirst:
    case kCGImageAlphaPremultipliedLast:
      fullImageBytesPerRow = COMPUTE_BEST_BYTES_PER_ROW(4 * fullImageWidth);
      break;
    case kCGImageAlphaNone | kCGBitmapFloatComponents:
    case kCGImageAlphaNoneSkipFirst | kCGBitmapFloatComponents:
    case kCGImageAlphaNoneSkipLast | kCGBitmapFloatComponents:
    case kCGImageAlphaPremultipliedFirst | kCGBitmapFloatComponents:
    case kCGImageAlphaPremultipliedLast | kCGBitmapFloatComponents:
      fullImageBytesPerRow = COMPUTE_BEST_BYTES_PER_ROW(16 * fullImageHeight);
      break;
    default:
      fprintf(stderr, "Unknown bitmap info - unable to figure out optimal bytes"
              " per full row.\n");
			return EXIT_FAILURE;
  }
    
  double maxValue = (1 << bitsPerComponent) - 1.0;
  
  CGColorSpaceRef colorSpace = getTheDisplayColorSpace();
  if (colorSpace == NULL)
  {
    fprintf(stderr, "Couldn't get display color space.\n");
    return EXIT_FAILURE;
  }
  
  
  unsigned char* data
    = static_cast<unsigned char*>(calloc(1, fullImageBytesPerRow * fullImageHeight));
  
  CGContextRef bitmapContext
    = CGBitmapContextCreate(data, fullImageWidth, fullImageHeight,
                            bitsPerComponent, fullImageBytesPerRow, colorSpace,
                            kCGImageAlphaNoneSkipLast);
  if (bitmapContext == NULL)
  {
    fprintf(stderr, "Can't create bitmap context from image attributes.\n");
    return EXIT_FAILURE;
  }
  
  CGRect fullImageRect = CGRectMake(0, 0, fullImageWidth, fullImageHeight);
  CGContextDrawImage(bitmapContext, fullImageRect, fullImage);
  
  size_t firstContentRow;
  size_t lastContentRow;
  size_t firstContentColumn;
  size_t lastContentColumn;
  getContentBoundaries(data, fullImageWidth, fullImageHeight,
                       fullImageBytesPerRow, bytesPerPixel, &firstContentRow,
                       &lastContentRow, &firstContentColumn,
                       &lastContentColumn);
  if (! (lastContentRow - firstContentRow > 0 &&
         lastContentColumn - firstContentColumn > 0))
  {
    fprintf(stderr, "error: could not find borders of probe content\n");
    return EXIT_FAILURE;
  }
  
  size_t N = getEdgeSize(data, fullImageWidth, fullImageHeight,
                         fullImageBytesPerRow, bytesPerPixel,
                         firstContentColumn, lastContentColumn, lastContentRow);
  
  FILE* file = fopen(outputTextPath, "w");
  if (file == NULL)
  {
    fprintf(stderr, "error: %s: %s\n", my_name, strerror(errno));
    return EXIT_FAILURE;
  }
  
  size_t x = firstContentColumn;
  size_t y = firstContentRow;
  size_t r;
  size_t g;
  size_t b;
  for (r = 0; r < N; ++r)
    for (g = 0; g < N; ++g)
      for (b = 0; b < N; ++b)
      {
        unsigned char* row = data + y * fullImageBytesPerRow;
        unsigned char* pixel = row + x * 4;
        fprintf(file, "%f %f %f\n",
                *(pixel + 0) / maxValue,
                *(pixel + 1) / maxValue,
                *(pixel + 2) / maxValue);
        ++x;
        if (x > lastContentColumn)
        {
          x = firstContentColumn;
          ++y;
        }
      }

  int closeStatus = fclose(file);
  if (closeStatus != 0)
  {
    fprintf(stderr, "%s: error: %s.\n", my_name, strerror(errno));
    return EXIT_FAILURE;
  }
  cout << N << endl;
#endif /* __APPLE__ */
  return EXIT_SUCCESS;
}
