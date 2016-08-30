/*
  File:       iccFlattenAToB0Tag.cpp
 
  Contains:   Utility to flatten out AToB0 contents into a file, suitable as
  the input file to iccCreateCLUTInputProfile.  This is pretty much
  scaffolding for the creation & debugging of
  iccCreateCLUTInputProfile for the moment, but is also not a bad way
  to see how one would probe the CMM.
 
  Version:    V1
 
  Copyright:  © see ICC Software License
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
// -Initial implementation by Joseph Goldstone 11 May 2006
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#include "IccProfile.h"
// OpenICCProfile

#include "IccUtil.h"
// icLabFromPCS

#include "IccCmm.h"
// CIccCmm

#include "Vetters.h"

typedef vector<icFloatNumber> ResultTuple;
typedef vector<ResultTuple> ResultTuples;

void
printTuples(ostream& oS, const ResultTuples& tuples)
{
  for (ResultTuples::const_iterator
         iter = tuples.begin(), endIter = tuples.end();
       iter != endIter; ++iter)
    oS << (*iter)[0] << " " << (*iter)[1] << " " << (*iter)[2] << endl;
}

void
usage(ostream& s, const char* const my_name)
{
  s << my_name << ": usage is " << my_name
  << " profile sampling flattened_contents"
  << endl;
}
  
int
main(const int argc, const char* argv[])
{
  const char* const my_name = path_tail(argv[0]);
  if (argc != 4)
  {  
    usage(cout, my_name);
    return EXIT_FAILURE;
  }

  const char* const profile_path = argv[1];
  vet_input_file_pathname(profile_path, "profile", "the pathname of an ICC"
                          " profile which contains an AToB tag");
  
  const char* const sampling_string = argv[2];
  vet_as_int(sampling_string, "granularity", "the edge sampling of the"
             "3D LUT, e.g. 4 would produce a 4x4x4 sampling resulting in a file"
             " with 64 lines of recovered data");
  unsigned int sampling = atoi(argv[2]);
  
  const char* flattened_contents_path = argv[3];
  vet_output_file_pathname(flattened_contents_path, "flattened_contents",
                           "the pathname of the file that will be created to"
                           " hold the flattened 3D LUT from the supplied"
                           " ICC profile");

  CIccProfile* srcProfile = OpenIccProfile(profile_path);
  if (srcProfile == NULL)
  {
    cout << "Error opening source profile `" << profile_path << "'" << endl;
    return EXIT_FAILURE;
  }
  
  CIccTagXYZ* mediaWhitePointTag
    = static_cast<CIccTagXYZ*>(srcProfile->FindTag(icSigMediaWhitePointTag));
  if (mediaWhitePointTag == NULL)
  {
    cout  << "no white point tag found in source profile `" << profile_path
          << "'" << endl;
    return EXIT_FAILURE;
  }
  icFloatNumber whiteXYZ[3];
  whiteXYZ[0] = icFtoD((*mediaWhitePointTag)[0].X);
  whiteXYZ[1] = icFtoD((*mediaWhitePointTag)[0].Y);
  whiteXYZ[2] = icFtoD((*mediaWhitePointTag)[0].Z);
  
  ResultTuples resultTuples(sampling * sampling * sampling);

  CIccCmm cmm;

  cmm.AddXform(srcProfile, icAbsoluteColorimetric);
  if (cmm.Begin() != icCmmStatOk) {
    cout << "error initializing CMM" << endl;
    return EXIT_FAILURE;
  }
  
  for (unsigned int i = 0; i < sampling; ++i)
    for (unsigned int j = 0; j < sampling; ++j)
      for (unsigned int k = 0; k < sampling; ++k)
      {
        icFloatNumber dstPixel[3];
        icFloatNumber srcPixel[3];
        srcPixel[0] = (icFloatNumber) (i / (sampling - 1.0));
        srcPixel[1] = (icFloatNumber) (j / (sampling - 1.0));
        srcPixel[2] = (icFloatNumber) (k / (sampling - 1.0));
        cmm.Apply(dstPixel, srcPixel);
        if (srcProfile->m_Header.pcs == icSigLabData)
        {
          icLabFromPcs(dstPixel);
          icLabtoXYZ(dstPixel, NULL, whiteXYZ);
        }
        else
          icXyzFromPcs(dstPixel);
        ResultTuple resultTuple(dstPixel, dstPixel + 3);
        resultTuples[i * sampling * sampling + j * sampling + k] = resultTuple;
      }

  if (strcmp(flattened_contents_path,"-") == 0)
    printTuples(cout, resultTuples);
  else
  {
    ofstream oFS(flattened_contents_path);
    printTuples(oFS, resultTuples);
  }

  return EXIT_SUCCESS;
}
