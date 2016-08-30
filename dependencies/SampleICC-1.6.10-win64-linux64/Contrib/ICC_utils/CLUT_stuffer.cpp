/*
  File:       CLUT_stuffer.cpp
 
  Contains:   originally part of iccCreateCLUTProfile command-line tool:
  create aCLUT tag data
 
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
// -broken out of CLUT.h by Joseph Goldstone fall 2006
//
//////////////////////////////////////////////////////////////////////

#include "CLUT_stuffer.h"

#include <iostream>
#include <iomanip>

#include "IccUtil.h"
#include "IccTagLut.h"
#include "IccCmm.h"

CLUT_stuffer::CLUT_stuffer(unsigned int edgeN,
                           const icFloatNumber* const measuredXYZ,
                           const icFloatNumber* flare,
                           const icFloatNumber illuminantY,
                           const CAT* const CATToPCS,
                           const icFloatNumber* adaptedMediaWhite,
                           const bool LABPCS)
  :  m_EdgeN(edgeN),
     m_MeasuredXYZ(measuredXYZ),
     m_IlluminantY(illuminantY),
     m_CAT(CATToPCS),
     m_LABPCS(LABPCS)
{
  for (int i = 0; i < 3; ++i)
  {
    m_Flare[i] = flare[i];
    m_AdaptedMediaWhite[i] = adaptedMediaWhite[i];
  }
}

void
CLUT_stuffer::PixelOp(icFloatNumber* pGridAdr, icFloatNumber* pData)
{
  // 1.  Obtain CIE Tristimulus values for a set of colour patches on the device
  // or media to be profiled.  More information about measurement procedures is 
  // provided in clause D.3.  There should be at least one measurement of the
  // "media white" and the tristimulus values of the illumination source or
  // perfect reflecting diffuser should be specified.
    
  // [We assume this is done upstream, outside of this member function]
    
  unsigned int rIdx = static_cast<unsigned int>(pGridAdr[0] * (m_EdgeN - 1) + 0.5);
  unsigned int gIdx = static_cast<unsigned int>(pGridAdr[1] * (m_EdgeN - 1) + 0.5);
  unsigned int bIdx = static_cast<unsigned int>(pGridAdr[2] * (m_EdgeN - 1) + 0.5);
  unsigned int flattenedIdx = (rIdx * m_EdgeN * m_EdgeN + gIdx * m_EdgeN + bIdx) * 3;
  icFloatNumber measuredXYZ[3];
  for (unsigned int i = 0; i < 3; ++i)
    measuredXYZ[i] = m_MeasuredXYZ[flattenedIdx + i];
    
  // 2. Remove flare from the measured XYZ values as needed to match
  // the PCS measurement conditions, creating flare-free XYZ values (XYZflare-free).
    
  // 3. If necessary, scale the flare-free measurement values so they
  // are relative to the actual illumination source by dividing all
  // values by the measured Y value of the perfect diffuser.
    
  // 4. If the chromaticity of the illumination source is different
  // from that of D50, convert the illuminant-relative XYZ values
  // from the illumination source white point chromaticity to the
  // PCS white point chromaticity using an appropriate chromatic
  // adaptation transform and equation D.9 (which is the same as
  // D.1).  This may be done by applying one of the transformations
  // described in clause D.4 and Annex E.  The transform used must
  // be specified in the chromaticAdaptationTag.
    
  icFloatNumber adaptedXYZ[3];
  CLUT::measuredXYZToAdaptedXYZ(adaptedXYZ, measuredXYZ, m_Flare,
                                m_IlluminantY, m_CAT);
  
  // 5. Record the converted media white point in the mediaWhitePointTag.
  // Optionally, record the converted media black point in the
  // mediaBlackPointTag.
  
  // [This should be done, but not here in per-CLUT-entry land]
  
  // 6. Convert colorimetry from D50 illuminant-relative to
  // mediawhite-relative values, by scaling each value by the ratio of the
  // PCS D50 illuminantion source over the converted media white point, using
  // equation D.10 (which is the same as D.3).  After scaling, the XYZ values
  // for the media white point measurement will be equal to the XYZ values of
  // the PCS D50 illumination source.
  
  icFloatNumber adjustedPCSXYZ[3];
  for (unsigned int i = 0; i < 3; ++i)
    adjustedPCSXYZ[i] = adaptedXYZ[i] * icD50XYZ[i] / m_AdaptedMediaWhite[i];
  
  if (m_LABPCS)
  {
    // 7. Optionally, convert the adjusted PCS XYZ coordinates to PCS L*a*b*
    // as described in Annex A.
    icXYZtoLab(pData, adjustedPCSXYZ, icD50XYZ);
  
    // 8. Encode the PCS XYZ coordinates or the PCS L*a*b* coordinates
    // digitally in 8-bit or 16-bit representations as defined in 6.3.4.
    icLabToPcs(pData);
  
    // 8.5.
    // Since the data type of this tag is 'lut16Type' we use the legacy
    // encoding mentioned in 6.3.4.2 NOTE 4 and in clause 10.8.
    // n.b. 'lut16Type' tags and 'namedColor2Type' tags are the ONLY place
    // you will ever see this.
    CIccPCS::Lab4ToLab2(pData, pData);
  }
  else
  {
    for(unsigned int i = 0; i < 3; ++i)
      pData[i] = adjustedPCSXYZ[i];
    icXyzToPcs(pData);
  }
}
