/*
  File:       CLUT.cpp

  Contains:   originally part of iccCreateCLUTProfile command-line tool:
  chromatic adaptation transforms

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
// -Initial implementation by Joseph Goldstone spring 2006
//
//////////////////////////////////////////////////////////////////////

#ifndef __INCLUDED_CAT_H__
#define __INCLUDED_CAT_H__

#include "IccTag.h"
#include "IccUtil.h"
// icMatrixMultiply3x3

class CAT
{
public:
  CAT(const icFloatNumber* PCS,
      const icFloatNumber* src)
    : m_CAT(new icFloatNumber[9])
  {
    // linearized Bradford transformation, see ICC spec Annex E
    const icFloatNumber fwdLinBfd[] = {  (icFloatNumber)0.8951,  (icFloatNumber)0.2664,  (icFloatNumber)-0.1614,
                                         (icFloatNumber)-0.7502, (icFloatNumber)1.7135,  (icFloatNumber)0.0367,
                                         (icFloatNumber)0.0389,  (icFloatNumber)-0.0685, (icFloatNumber) 1.0296 };
    // inverse is from Mathematica since the spec doesn't provide it
    const icFloatNumber invLinBfd[] = {  (icFloatNumber)0.986993,    (icFloatNumber)-0.147054, (icFloatNumber)0.159963,
                                         (icFloatNumber)0.432305,    (icFloatNumber)0.51836,   (icFloatNumber)0.0492912,
                                         (icFloatNumber)-0.00852866, (icFloatNumber)0.0400428, (icFloatNumber)0.968487 };

    icFloatNumber rhoPCS = fwdLinBfd[0] * PCS[0] + fwdLinBfd[1] * PCS[1] + fwdLinBfd[2] * PCS[2];
    icFloatNumber gamPCS = fwdLinBfd[3] * PCS[0] + fwdLinBfd[4] * PCS[1] + fwdLinBfd[5] * PCS[2];
    icFloatNumber betPCS = fwdLinBfd[6] * PCS[0] + fwdLinBfd[7] * PCS[1] + fwdLinBfd[8] * PCS[2];
    icFloatNumber rhoSrc = fwdLinBfd[0] * src[0] + fwdLinBfd[1] * src[1] + fwdLinBfd[2] * src[2];
    icFloatNumber gamSrc = fwdLinBfd[3] * src[0] + fwdLinBfd[4] * src[1] + fwdLinBfd[5] * src[2];
    icFloatNumber betSrc = fwdLinBfd[6] * src[0] + fwdLinBfd[7] * src[1] + fwdLinBfd[8] * src[2];
    icFloatNumber scaling[9];
    scaling[0] = rhoPCS / rhoSrc;
    scaling[1] = 0;
    scaling[2] = 0;
    scaling[3] = 0;
    scaling[4] = gamPCS / gamSrc;
    scaling[5] = 0;
    scaling[6] = 0;
    scaling[7] = 0;
    scaling[8] = betPCS / betSrc;
    icFloatNumber tmp[9];
    icMatrixMultiply3x3(tmp, scaling, fwdLinBfd);
    icMatrixMultiply3x3(m_CAT, invLinBfd, tmp);
  }

  CAT(const icFloatNumber* contents)
    : m_CAT(new icFloatNumber[9])
  {
    for (unsigned int i = 0; i < 9; ++i)
      m_CAT[i] = contents[i];
  }
  
  ~CAT()
  {
    delete[] m_CAT;
  }

  CAT*
  Inverse() const;

  void Apply(icFloatNumber * const product, const icFloatNumber * const multiplicand) const;

  CIccTagS15Fixed16*
  makeChromaticAdaptationTag() const;

  icFloatNumber* m_CAT;
};

#endif
