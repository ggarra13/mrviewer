/*
  File:       Measurement_extractor.cpp
 
  Contains:   Declarations for class to extract from a profile measurements
  that could have been used to create that same profile.
  Requires caller to have chromatic adaptation matrix (usually
  obtained from 'chad' tag) and absolute luminance information
  usually obtained from 'lumi' tag).
 
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
// -originally written by Joseph Goldstone fall 2006
//
//////////////////////////////////////////////////////////////////////

#include "IccCmm.h"
#include "IccUtil.h"
#include "CAT.h"

class Measurement_extractor
{
public:
  
  static
  icColorSpaceSignature
  getPCSSpace(CIccProfile* profile);
  
  static
  icFloatNumber
  getIlluminantY(CIccProfile* profile);
  
  static
  CAT*
  getInverseCAT(CIccProfile* profile);
  
  static
  void
  getAdaptedMediaWhite(icFloatNumber* white, CIccProfile* profile);
  
  // This one requires a luminanceTag to be in the profile
  Measurement_extractor(const char * const profileName,
                        const icFloatNumber* flare);
  
  // This one does not, but you still have to know what the illuminant Y
  // value was, and supply it here.
  Measurement_extractor(const char * const profileName,
                        icFloatNumber illuminantY,
                        const icFloatNumber* flare);
  
  ~Measurement_extractor();
  
  void
  reconstructMeasurement(icFloatNumber* measuredXYZ,
                         icFloatNumber* RGBStimulus);

private:
  CIccProfile*  profile_;
  bool    isLabPCS_;
  icFloatNumber flare_[3];
  icFloatNumber illuminantY_;
  CAT*    inverseCAT_;
  icFloatNumber adaptedMediaWhite_[3];
  CIccCmm   cmm_;
};
