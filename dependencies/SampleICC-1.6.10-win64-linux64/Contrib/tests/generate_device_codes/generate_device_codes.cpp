/*
  File:       generate_device_codes.cpp
 
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
#include <string>
#include <sstream>
#include <limits>
using namespace std;

#include "Vetters.h"

void
usage(ostream& s, const char* const myName)
{
  s << myName << ": usage is " << myName << " -lattice|-r|-g|-b|-w N" << endl;
}

int
main(int argc, char* argv[])
{
  const char* const myName = path_tail(argv[0]);
  if (argc != 3)
  {
    usage(cout, myName);
    return EXIT_FAILURE;
  }
  string pattern(argv[1]);
  if (pattern != "-lattice"
      && pattern != "-r"
      && pattern != "-g"
      && pattern != "-b"
      && pattern != "-w")
  {
    usage(cout, myName);
    return EXIT_FAILURE;
  }
  const char* const N_chars = argv[2];
  vet_as_int(N_chars, "N", "number of points along line or edge");
  int N = atoi(N_chars);

  if (pattern == "-lattice")
  {
    for (int i = 0; i < N; ++i)
      for (int j = 0; j < N; ++j)
        for (int k = 0; k < N; ++k)
          cout << static_cast<float>(i) / (N - 1) << " "
               << static_cast<float>(j) / (N - 1) << " "
               << static_cast<float>(k) / (N - 1) << endl;

  }
  else if (pattern == "-r")
    for (int i = 0; i < N; ++i)
      cout << static_cast<float>(i) / (N - 1) << " 0 0" << endl;
  else if (pattern == "-g")
    for (int i = 0; i < N; ++i)
      cout << "0 " << static_cast<float>(i) / (N - 1) << " 0" << endl;
  else if (pattern == "-b")
    for (int i = 0; i < N; ++i)
      cout << "0 0 " << static_cast<float>(i) / (N - 1) << endl;
  else if (pattern == "-w")
    for (int i = 0; i < N; ++i)
      cout << static_cast<float>(i) / (N - 1) << " "
           << static_cast<float>(i) / (N - 1) << " "
           << static_cast<float>(i) / (N - 1) << endl;
  else
  {
    cout << "Error: request to generate device codes in pattern other than"
         << " -lattice, -r, -g, -b or -w" << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
