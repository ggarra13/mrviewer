/*
  File:       round_trip_PCS_LAB.cpp
 
  Contains:   

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
// -Initial implementation by Joseph Goldstone late spring 2006
//
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
using namespace std;

#include "IccUtil.h"
#include "ICC_tool_exception.h"
#include "Vetters.h"

icFloatNumber
deltaE(icFloatNumber* first_LAB, icFloatNumber* second_LAB)
{
	icFloatNumber dL = second_LAB[0] - first_LAB[0];
	icFloatNumber da = second_LAB[1] - first_LAB[1];
	icFloatNumber db = second_LAB[2] - first_LAB[2];
  return sqrt(dL * dL + da * da + db * db);
}

void
usage(ostream& s, const char* const my_name)
{
  s << my_name << ": usage is " << my_name << " file\n"
  " where file is the pathname of a file containing whitespace-seperated"
  " triplets of CIE LAB data.  For each triplet a line summarizing the"
  " CIE 1976 delta E of that triplet and its round-tripped value through"
  " the CIE LAB PCS is printed on standard output";
}

int
main(int argc, char* argv[])
{
  const char* const my_name = path_tail(argv[0]);
  if (argc != 2)
  {
    usage(cout, my_name);
    return EXIT_FAILURE;
  }
  char* LAB_pre_encoding_filename = argv[1];
  vet_input_file_pathname(LAB_pre_encoding_filename, "file", "the pathname of a"
                          " file containing whitespace-seperated triplets of"
                          " CIE LAB data");
  ifstream in_s(LAB_pre_encoding_filename);
  if (! in_s)
  {
    ostringstream s;
    s << "Could not open file `" << LAB_pre_encoding_filename << "' for input";
    throw ICC_tool_exception(s.str());
  }
  
  while (! in_s.eof())
	{
		string line("");
		getline(in_s, line);
		if (line == "")
			break;
		istringstream l_s(line);
		icFloatNumber orig_LAB[3];
		icFloatNumber pre_round_trip_LAB[3];
		icFloatNumber post_round_trip[3];
		l_s >> orig_LAB[0] >> orig_LAB[1] >> orig_LAB[2];
		for (unsigned int i = 0; i < 3; ++i)
			pre_round_trip_LAB[i] = orig_LAB[i];
		icLabToPcs(pre_round_trip_LAB);
		for (unsigned int i = 0; i < 3; ++i)
		{
			icUInt16Number as_in_file
				= (icUInt16Number)(max((icFloatNumber)0.0,
															 min((icFloatNumber)1.0,
																	 pre_round_trip_LAB[i])) * 65535 + 0.0);
			post_round_trip[i]
				= (icFloatNumber)((icFloatNumber)as_in_file / 65535.0);
		}
		icLabFromPcs(post_round_trip);
		cout << deltaE(orig_LAB, post_round_trip) << " "
				 << orig_LAB[0] << " "
				 << orig_LAB[1] << " "
				 << orig_LAB[2] << " "
				 << post_round_trip[0] << " "
				 << post_round_trip[1] << " "
				 << post_round_trip[2] << endl;
	}
}

