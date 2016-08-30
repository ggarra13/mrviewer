/*
  File:       reconstruct_measurements.cpp
 
  Contains:   .
 
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

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#include "ICC_tool_exception.h"
#include "Measurement_extractor.h"
#include "Vetters.h"

void
processMeasurements(istream& in_s,
                    Measurement_extractor& extractor,
                    const char* const XYZMeasurementsFilename)
{
  ofstream out_s(XYZMeasurementsFilename);
  if (! out_s)
  {
    ostringstream s;
    s << "Could not open file `" << XYZMeasurementsFilename << "' for output";
    throw ICC_tool_exception(s.str());
  }
  
  while (! in_s.eof())
  {
    string line("");
    getline(in_s, line);
    if (line == "")
      break;
    istringstream l_s(line);
    icFloatNumber RGBStimulus[3];
    l_s >> RGBStimulus[0] >> RGBStimulus[1] >> RGBStimulus[2];
    icFloatNumber measuredXYZ[3];
    extractor.reconstructMeasurement(measuredXYZ, RGBStimulus);
    out_s << measuredXYZ[0] << " "
          << measuredXYZ[1] << " "
          << measuredXYZ[2] << endl;
  }
}

void
usage(ostream& s, const char* const myName)
{
  s << myName << ": usage is " << myName <<
    " stimuli profile measurements [Y]\n"
  << "where\n"
  << " stimuli is the set of RGB values for which you wish to reconstruct"
  << " measurements\n"
  << " profile is the ICC Profile from which the measurements will be"
  << " reconstructed\n"
  << " measurements is the pathname of the file into which the reconstructed"
  << " measurements will be written\n"
  << " Y, if given, will be the Y value of the reconstructed illuminant\n"
  << endl;
}

int
main(int argc, char* argv[])
{
  try
  {
    const char* const myName = path_tail(argv[0]);
    if (argc < 4 || argc > 5)
    {
      usage(cout, myName);
      return EXIT_FAILURE;
    }
    const char* const RGBStimuliFilename = argv[1];
    vet_input_file_pathname(RGBStimuliFilename, "stimuli", "the pathname of"
                            " a file containing RGB stimuli");
    ifstream in_s(RGBStimuliFilename);
    if (! in_s)
    {
      ostringstream s;
      s << "Could not open file `" << RGBStimuliFilename << "'";
      throw ICC_tool_exception(s.str());
    }
    const char* const ICCProfileFilename = argv[2];
    vet_input_file_pathname(ICCProfileFilename, "profile", "the pathname of an"
                            " ICC profile from which measurements will be"
                            " extracted which could reconstruct that profile");
    icFloatNumber flare[3] = {0, 0, 0};
    
    char* XYZMeasurementsFilename = argv[3];
    vet_output_file_pathname(XYZMeasurementsFilename, "measurements",
                             "the pathname of the file into which the"
                             " reconstructed XYZ measurements will be written");
    if (argc == 5)
    {
      const char* const Y_chars = argv[4];
      vet_as_float(Y_chars, "Y", "desired Y value of the reconstructed"
                   " illuminant");
      icFloatNumber illuminantY = (icFloatNumber)atof(Y_chars);
      Measurement_extractor extractor(ICCProfileFilename,
                                      illuminantY,
                                      flare);
      processMeasurements(in_s, extractor, XYZMeasurementsFilename);
    }
    else
    {
      Measurement_extractor extractor(ICCProfileFilename,
                                      flare);
      processMeasurements(in_s, extractor, XYZMeasurementsFilename);
    }
    return EXIT_SUCCESS;
  }
  catch (const exception& e)
  {
    cout << "Error: " << e.what() << endl;
    return EXIT_FAILURE;
  }
}
