/*
    File:       CmdApplyProfiles.cpp

    Contains:   Console app that applies profiles

    Version:    V1

    Copyright:  © see below
*/

/*
 * The ICC Software License, Version 0.2
 *
 *
 * Copyright (c) 2003-2015 The International Color Consortium. All rights 
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
// -Initial implementation by Max Derhak 5-15-2003
//
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include "IccCmm.h"
#include "IccUtil.h"
#include "TiffImg.h"

static icFloatNumber UnitClip(icFloatNumber v)
{
  if (v<0.0)
    return 0.0;
  if (v>1.0)
    return 1.0;
  return v;
}

bool Apply(const char *szSrcImage,
           const char *szSrcProfile,
           const char *szDstProfile,
           const char *szDstImage,
           int nIntent)
{
  unsigned long i, j, k, sn, sphoto, dn, photo, space;
  CTiffImg SrcImg, DstImg;
  CIccCmm cmm;
  unsigned char *sptr, *dptr;
  bool bSuccess = true;
  bool bConvert = false;

  if (cmm.AddXform(szSrcProfile, nIntent<0 ? icUnknownIntent : (icRenderingIntent)nIntent/*, icInterpTetrahedral*/)) {
    printf("Invalid Profile:  %s\n", szSrcProfile);
    return false;
  }
  
  if (szDstProfile && *szDstProfile && cmm.AddXform(szDstProfile/*, icUnknownIntent, icInterpTetrahedral*/)) {
    printf("Invalid Profile:  %s\n", szDstProfile);
    return false;
  }

  if (cmm.Begin() != icCmmStatOk) {
    printf("Invalid Profile:\n  %s\n  %s'\n", szSrcProfile, szDstProfile);
    return false;
  }

  if (!SrcImg.Open(szSrcImage)) {
    printf("Invalid Tiff file - '%s'\n", szSrcImage);
    return false;
  }
  sn = SrcImg.GetSamples();
  sphoto = SrcImg.GetPhoto();
  space = cmm.GetSourceSpace();

  if (SrcImg.GetBitsPerSample()!=8 ||
      !((space==icSigRgbData && sn==3 && sphoto==PHOTO_MINISBLACK) ||
        (space==icSigLabData && sn==3 && sphoto==PHOTO_CIELAB) ||
        (space==icSigXYZData && sn==3 && sphoto==PHOTO_CIELAB) ||
        (space==icSigCmykData && sn==4 && sphoto==PHOTO_MINISWHITE) ||
        (space==icSigMCH4Data && sn==4 && sphoto==PHOTO_MINISWHITE) ||
        (space==icSigMCH5Data && sn==5 && sphoto==PHOTO_MINISWHITE) ||
        (space==icSigMCH6Data && sn==6 && sphoto==PHOTO_MINISWHITE))) {
    printf("Invalid source profile/image pixel format\n");
    return false;
  }

  switch (cmm.GetDestSpace()) {
  case icSigRgbData:
    photo = PHOTO_MINISBLACK;
    dn = 3;
    break;

  case icSigCmyData:
    photo = PHOTO_MINISWHITE;
    dn = 3;
    break;

  case icSigXYZData:
    bConvert = true;
    //Fall through - No break here

  case icSigLabData:
    photo = PHOTO_CIELAB;
    dn = 3;
    break;

  case icSigCmykData:
  case icSig4colorData:
    photo = PHOTO_MINISWHITE;
    dn = 4;
    break;

  case icSig5colorData:
    photo = PHOTO_MINISWHITE;
    dn = 5;
    break;

  case icSig6colorData:
    photo = PHOTO_MINISWHITE;
    dn = 6;
    break;

  case icSig7colorData:
    photo = PHOTO_MINISWHITE;
    dn = 7;
    break;

  case icSig8colorData:
    photo = PHOTO_MINISWHITE;
    dn = 8;
    break;

  default:
    printf("Invalid destination profile/image pixel format\n");
    return false;
  }

  if (!DstImg.Create(szDstImage, SrcImg.GetWidth(), SrcImg.GetHeight(), 8, photo, dn, SrcImg.GetXRes(), SrcImg.GetYRes(), false)) {
    printf("Unable to create Tiff file - '%s'\n", szDstImage);
    return false;
  }

  unsigned char *pSBuf = (unsigned char *)malloc(SrcImg.GetBytesPerLine());
  unsigned char *pDBuf = (unsigned char *)malloc(DstImg.GetBytesPerLine());
  icFloatNumber Pixel[16];

  if (!pSBuf) {
    printf("Out of Memory!\n");
    return false;
  }

  if (!pDBuf) {
    printf("Out of Memory!\n");
    free(pSBuf);
    return false;
  }

  for (i=0; i<SrcImg.GetHeight(); i++) {
    if (!SrcImg.ReadLine(pSBuf)) {
      bSuccess = false;
      break;
    }
    for (sptr=pSBuf, dptr=pDBuf, j=0; j<SrcImg.GetWidth(); j++, sptr+=sn, dptr+=dn) {
      if (sphoto==PHOTO_CIELAB) {
        Pixel[0] = (icFloatNumber)sptr[0] / 255.0f;
        Pixel[1] = ((icFloatNumber)((signed char)sptr[1]) + 128.0f) / 255.0f;
        Pixel[2] = ((icFloatNumber)((signed char)sptr[2]) + 128.0f) / 255.0f;

        if (space==icSigXYZData) {
          icLabFromPcs(Pixel);

          icLabtoXYZ(Pixel);

          icXyzToPcs(Pixel);
        }
      }
      else {
        for (k=0; k<sn; k++) {
          Pixel[k] = (icFloatNumber)sptr[k] / 255.0f;
        }
      }

      cmm.Apply(Pixel, Pixel);

      if (photo==PHOTO_CIELAB) {
        if (bConvert) {
          icXyzFromPcs(Pixel);

          icXYZtoLab(Pixel);

          icLabToPcs(Pixel);
        }
        dptr[0] = (unsigned char)(UnitClip(Pixel[0]) * 255.0 + 0.5);
        dptr[1] = (unsigned char)(UnitClip(Pixel[1]) * 255.0 - 128.0);
        dptr[2] = (unsigned char)(UnitClip(Pixel[2]) * 255.0 - 128.0);
      }
      else {
        for (k=0; k<dn; k++) {
          dptr[k] = (unsigned char)(UnitClip(Pixel[k]) * 255.0 + 0.5);
        }
      }
    }
    if (!DstImg.WriteLine(pDBuf)) {
      bSuccess = false;
      break;
    }
  }

  SrcImg.Close();

  free(pSBuf);
  free(pDBuf);

  return bSuccess;
}


int main(int argc, char* argv[])
{
  int nIntent = -1;

  if (argc<5) {
    printf("Usage: iccApplyProfiles src_img_path src_profile_path dest_profile_path dest_image_path {Rendering_intent}\n\n");
    printf("  For Rendering_intent:\n");
    printf("    0 - Perceptual\n");
    printf("    1 - Relative Colorimetric\n");
    printf("    2 - Saturation\n");
    printf("    3 - Absolute Colorimetric\n");

    return -1;
  }

  if (argc>5)
    sscanf(argv[5], "%d", &nIntent);

  if (Apply(argv[1], argv[2], argv[3], argv[4], nIntent)) {
    printf("Profiles successfully applied\n");
  }
  
  return 0;
}

