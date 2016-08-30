/*
  File:       Black_scaler.h
 
  Contains:   part of create_CLUT_profile command-line tool:
  scale A2B0 data to mimic A2B1
 
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
// -originally written by Joseph Goldstone fall 2006
//
//////////////////////////////////////////////////////////////////////

#include "Black_scaler.h"

#include <iostream>
#include <cmath>
using namespace std;

Black_scaler::Black_scaler(const unsigned int edgeN,
                           const icFloatNumber* const rawXYZ,
                           const icFloatNumber* const adaptedMediaBlack,
                           const icFloatNumber* const adaptedMediaWhite)
  : m_EdgeN(edgeN), m_rawXYZ(rawXYZ)
{
  unsigned int i;
  for (i = 0; i < 3; ++i)
  {
    m_A2B1BlackXYZ[i] = adaptedMediaBlack[i];
    m_A2B1WhiteXYZ[i] = adaptedMediaWhite[i];
  }
    
  m_A2B0WhiteXYZ[0] = (icFloatNumber)0.9642;
  m_A2B0WhiteXYZ[1] = (icFloatNumber)1.0;
  m_A2B0WhiteXYZ[2] = (icFloatNumber)0.8249;
  
  // Done this way because going forwards from the decimal numbers shown in
  // Table 12 of section 6.3.4.3 of the ISO spec does not map perfectly to
  // the PCS black encoding of 0x0808 0x8080 0x8080, so we back into it.
  icFloatNumber A2B0BlackLab[3]
    = { (icFloatNumber)(0x0808/65535.0),
        (icFloatNumber)(0x8080/65535.0),
        (icFloatNumber)(0x8080/65535.0) };
  icLabFromPcs(A2B0BlackLab);
  icLabtoXYZ(m_A2B0BlackXYZ, A2B0BlackLab, m_A2B0WhiteXYZ);
  
  icFloatNumber A2B0Black_via_A2B0White[3];
  icXYZtoLab(A2B0Black_via_A2B0White, m_A2B0BlackXYZ, m_A2B0WhiteXYZ);
  icLabToPcs(A2B0Black_via_A2B0White);
  
  icFloatNumber A2B0Black_via_A2B1White[3];
  icXYZtoLab(A2B0Black_via_A2B1White, m_A2B0BlackXYZ, m_A2B1WhiteXYZ);
  for (i = 0; i < 3; ++i)
  {
    m_A2B1RangeXYZ[i] = m_A2B1WhiteXYZ[i] - m_A2B1BlackXYZ[i];
    m_A2B0RangeXYZ[i] = m_A2B0WhiteXYZ[i] - m_A2B0BlackXYZ[i];
  }
}

void Black_scaler::PixelOp(icFloatNumber* pGridAdr, icFloatNumber* pData)
{
  // This can be used to take a CLUT containing CIE XYZ data representing
  // colorimetry suitable for the media-relative intent, and then convert it
  // to colorimetry [slightly] more suitable for the perceptual intent.

  // In a better world, one would be able to make an input profile that only
  // had an A2B1 tag but, sadly, we do not live in that better world.  So if
  // a user who really should be using the media relative intent on this input
  // profile in Photoshop [let's say] specifies perceptual intent instead,
  // give them something that, as best we can manage, will produce the same
  // result as relative colorimetric.  A A2B1 tag in A2B0's clothes, in effect.
  
  // One simple heuristic is to take the input device black and pin it to the
  // perceptual reference medium black, which is an achromatic stimulus (see
  // section 6.3.3, "Reference viewing environment", where it says:
  //  "The darkest printable color on this medium is assumed to have a neutral
  //   reflectance of 0,30911% [...]"
  // and pin the input device white to the perceptual reference medium white.
  
  // Because the data are stored in the cube as PCS LAB (with the normal 0-100
  // L* range remapped to 0.0 - 1.0, and comparable hacks done for a* and b*)
  // we transform back to 'normal LAB' and thence to XYZ.
  
  icFloatNumber A2B1StimulusLab[3];
  icFloatNumber A2B1StimulusXYZ[3];
  unsigned int i;
  
  // 1. Extract the PCS colorimetry
  for (i = 0; i < 3; ++i)
    A2B1StimulusLab[i] = pData[i];
  
  // 2. Convert to LAB
  icLabFromPcs(A2B1StimulusLab);
  
  // 3. Convert to XYZ
  icLabtoXYZ(A2B1StimulusXYZ, A2B1StimulusLab, m_A2B1WhiteXYZ);
  
  icFloatNumber A2B1StimulusFrac[3];
  icFloatNumber A2B0StimulusLab[3];
  icFloatNumber A2B0StimulusXYZ[3];

  // 4. Scale the relative-colorimetric XYZ range to the perceptual reference
  //    medium's XYZ range.  This is a hack; normally the contents of an A2B0
  //    tag contains a lot of smarts about the human visual system, the
  //    viewing environment, etc.  At this point, though, we are just putting
  //    something into the A2B0 tag that won't panic an artist if they don't
  //    know color management and load this profile with the default intent
  //    being set in some program to 'perceptual'.
  for (i = 0; i < 3; ++i)
  {
    A2B1StimulusFrac[i] = (A2B1StimulusXYZ[i] - m_A2B1BlackXYZ[i]) / m_A2B1RangeXYZ[i];
    A2B0StimulusXYZ[i] = m_A2B0BlackXYZ[i] + A2B1StimulusFrac[i] * m_A2B0RangeXYZ[i];
  }
  
  // 5. Convert back to Lab colorimetry
  icXYZtoLab(A2B0StimulusLab, A2B0StimulusXYZ, m_A2B1WhiteXYZ);
  
  // 6. Convert back to PCS colorimetry
  icLabToPcs(A2B0StimulusLab);
  
  // 7. Inject the modified PCS colorimetry
  for (i = 0; i < 3; ++i)
    pData[i] = A2B0StimulusLab[i];
}

