/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//
//	Run a set of CTL transforms to generate a color lookup table.
//
//-----------------------------------------------------------------------------

#include <cassert>
#include <iostream>

#include <ctlToLut.h>

#include "core/mrvOS.h"
#undef snprintf

#include <CtlSimdInterpreter.h>
#include <ImfStandardAttributes.h>
#include <ImfHeader.h>
#include <ImfFrameBuffer.h>

#include "gui/mrvPreferences.h"
#include "core/CMedia.h"

using namespace std;
using namespace Ctl;
using namespace Imf;
using namespace Imath;


#define WARNING(message) (cerr << "Warning: " << message << endl)


namespace {

void
initializeEnvHeader (Header &envHeader)
{
    //
    // Initialize the "environment header" for the CTL
    // transforms by adding displayChromaticities,
    // displayWhiteLuminance and displaySurroundLuminance
    // attributes.
    //

    //
    // Get the chromaticities of the display's primaries and
    // white point from an environment variable.  If this fails,
    // assume chromaticities according to Rec. ITU-R BT.709.
    //
    const Chromaticities& c = mrv::Preferences::ODT_CTL_chromaticities;
    envHeader.insert ("displayChromaticities", ChromaticitiesAttribute (c));

    float wl = mrv::Preferences::ODT_CTL_white_luminance;
    envHeader.insert ("displayWhiteLuminance", FloatAttribute (wl));

    //
    // Get the display's surround luminance from an environment variable.
    // If this fails, assume 10% of the display's white luminance.
    // (Recommended setup according to SMPTE RP 166.)
    //
    float sl = mrv::Preferences::ODT_CTL_surround_luminance;
    envHeader.insert ("displaySurroundLuminance", FloatAttribute (sl));
}



} // namespace


const char* kRGBChannels[] = { "rIn", "gIn", "bIn", "aIn" };

void
ctlToLut (std::vector<std::string> transformNames,
	  Header inHeader,
	  size_t lutSize,
	  const float pixelValues[/*lutSize*/],
	  float lut[/*lutSize*/],
	  const char* inChannels[4]
)
{

    //
    // Initialize an input and an environment header:
    // Make sure that the headers contain information about the primaries
    // and the white point of the image files an the display, and about
    // the display's white luminance and surround luminance.
    //

    Header envHeader;
    Header outHeader;

    if (!hasChromaticities (inHeader))
      {
	addChromaticities (inHeader, Chromaticities());
      }

    initializeEnvHeader (envHeader);

    //
    // Set up input and output FrameBuffer objects for the CTL transforms.
    //

    assert (lutSize % 4 == 0);

    FrameBuffer inFb;

    inFb.insert (inChannels[0],
		 Slice (Imf::FLOAT,			// type
		        (char *)pixelValues,		// base
			4 * sizeof (float),		// xStride
			0));				// yStride

    inFb.insert (inChannels[1],
		 Slice (Imf::FLOAT,			// type
			(char *)(pixelValues + 1),	// base
			4 * sizeof (float),		// xStride
			0));				// yStride

    inFb.insert (inChannels[2],
		 Slice (Imf::FLOAT,			// type
			(char *)(pixelValues + 2),	// base
			4 * sizeof (float),		// xStride
			0));				// yStride

    inFb.insert (inChannels[3],
		 Slice (Imf::FLOAT,			// type
			(char *)(pixelValues + 3),	// base
			4 * sizeof (float),		// xStride
			0));

    FrameBuffer outFb;

    outFb.insert ("rOut",
		  Slice (Imf::FLOAT,			// type
			 (char *)lut,			// base
			 4 * sizeof (float),		// xStride
			 0));				// yStride

    outFb.insert ("gOut",
		  Slice (Imf::FLOAT,			// type
			 (char *)(lut + 1),		// base
			 4 * sizeof (float),		// xStride
			 0));				// yStride

    outFb.insert ("bOut",
		  Slice (Imf::FLOAT,			// type
			 (char *)(lut + 2),		// base
			 4 * sizeof (float),		// xStride
			 0));				// yStride

    outFb.insert ("aOut",
		  Slice (Imf::FLOAT,			// type
			 (char *)(lut + 3),		// base
			 4 * sizeof (float),		// xStride
			 0));				// yStride

    //
    // Run the CTL transforms.
    //

    SimdInterpreter interpreter;

    #ifdef CTL_MODULE_BASE_PATH

	//
	// The configuration scripts has defined a default
	// location for CTL modules.  Include this location
	// in the CTL module search path.
	//

	std::vector< std::string > paths = interpreter.modulePaths();
	paths.push_back (CTL_MODULE_BASE_PATH);
	interpreter.setModulePaths (paths);

    #endif

    ImfCtl::applyTransforms (interpreter,
			     transformNames,
			     Box2i (V2i (0, 0), V2i (unsigned(lutSize) / 4, 0)),
			     envHeader,
			     inHeader,
			     inFb,
			     outHeader,
			     outFb);
}



