/*
File:       iccStripUnknownTags.cpp

Contains:   Console app to parse and remove unknown tag elements

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
// -Initial implementation by Max Derhak Oct-3-2003
//
//////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include "IccProfile.h"
#include "IccTag.h"
#include "IccTagFactory.h"
#include "IccUtil.h"

int main(int argc, char* argv[])
{
  CIccProfile *pIcc;

  if (argc<=2) {
    printf("Usage: iccStripUnknownTags src_profile result_profile\n");
    return -1;
  }

  pIcc = ReadIccProfile(argv[1]);

  if (!pIcc) {
    printf("Unable to parse '%s'\n", argv[1]);
    return -1;
  }

  int n;
  bool done = false;
  TagEntryList::iterator i;
  CIccInfo Fmt;

  while (!done) {
    done = true;
    for (n=0, i=pIcc->m_Tags->begin(); i!=pIcc->m_Tags->end(); i++, n++) {
      IccTagEntry *pEntry = &(*i);
      if (!CIccTagCreator::GetTagSigName(pEntry->TagInfo.sig)) {
        printf("Removing '%s'\n", Fmt.GetTagSigName(pEntry->TagInfo.sig));
        done = false;
        pIcc->DeleteTag(pEntry->TagInfo.sig);
        break;
      }
    }
  }

  if (!SaveIccProfile(argv[2], pIcc)) {
    printf("Unable to save '%s'\n", argv[2]);
    return -1;
  }

  return 0;
}

