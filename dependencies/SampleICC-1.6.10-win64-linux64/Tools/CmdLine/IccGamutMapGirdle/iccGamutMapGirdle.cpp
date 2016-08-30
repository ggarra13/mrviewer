/*
    File:       iccGamutMapGirdle.cpp

    Contains:   Console app to parse and display profile round-trip statistics

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
// -Initial implementation by Max Derhak 6-3-2007
//
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <math.h>
#include "IccCmm.h"

int main(int argc, char* argv[])
{
  int nArg = 1;

  if (argc<=1) {
    printf("Usage: iccGamutMapGirdle profile num_hue {rendering_intent=3 {use_mpe=0}}\n");
    printf("  where\n");
	  printf("   num_hue is number of equal spaced hue steps\n");
	  printf("   rendering_intent is (0=perceptual, 1=relative, 2=saturation, 3=absolute)\n");
    return -1;
  }

  icRenderingIntent nIntent = icAbsoluteColorimetric;
  int nUseMPE = 0;
  int nHueSteps = atoi(argv[2]);

  if (argc>3) {
    nIntent = (icRenderingIntent)atoi(argv[3]);
    if (argc>4) {
      nUseMPE = atoi(argv[4]);
    }
  }

  CIccCmm cmm(icSigLabData, icSigLabData, false);

  if (cmm.AddXform(argv[1], nIntent, icInterpTetrahedral, icXformLutColor, nUseMPE==1)!=icCmmStatOk) {
    printf("Unable to add '%s' to cmm\n", argv[1]);
    return -2;
  }
  if (cmm.AddXform(argv[1], nIntent, icInterpTetrahedral, icXformLutColor, nUseMPE==1)!=icCmmStatOk) {
    printf("Unable to append '%s' to cmm\n", argv[1]);
    return -2;
  }

  if (cmm.Begin()!=icCmmStatOk) {
    printf("Unable to begin profile apply\n");
    return -3;
  }
  
  icFloatNumber pixel[16], bestLCH[3];

  icFloatNumber hue, hueStep;
  icFloatNumber Lval, Cval;
  icFloatNumber maxC;

  hueStep = 360.0f /(icFloatNumber)nHueSteps;

  printf("L*\tC*\tH*\n");
  for (hue=0.0; hue<360.0; hue += hueStep) {
    maxC=-10;
    memset(bestLCH, 0, sizeof(bestLCH));
    for (Lval = 100.0; Lval>=0.0; Lval-=.5) {
      pixel[0] = Lval;
      pixel[1] = 128;
      pixel[2] = hue;
      icLch2Lab(pixel);
      icLabToPcs(pixel);

      cmm.Apply(pixel,pixel);

      icLabFromPcs(pixel);
      icLab2Lch(pixel);
      if (pixel[1]>maxC) {
        maxC = pixel[1];
        memcpy(bestLCH, pixel, 3*sizeof(icFloatNumber));
      }
    }
    for (Cval=25; Cval<128; Cval+=.5) {
      pixel[0] = 100;
      pixel[1] = Cval;
      pixel[2] = hue;
      icLch2Lab(pixel);
      icLabToPcs(pixel);

      cmm.Apply(pixel,pixel);

      icLabFromPcs(pixel);
      icLab2Lch(pixel);
      if (pixel[1]>maxC) {
        maxC = pixel[1];
        memcpy(bestLCH, pixel, 3*sizeof(icFloatNumber));
      }

      pixel[0] = 0;
      pixel[1] = Cval;
      pixel[2] = hue;
      icLch2Lab(pixel);
      icLabToPcs(pixel);

      cmm.Apply(pixel,pixel);

      icLabFromPcs(pixel);
      icLab2Lch(pixel);
      if (pixel[1]>maxC) {
        maxC = pixel[1];
        memcpy(bestLCH, pixel, 3*sizeof(icFloatNumber));
      }
    }
    printf("%.2f\t%.2f\t%.2f\n", bestLCH[0], bestLCH[1], bestLCH[2]);
  }

  return 0;
}

