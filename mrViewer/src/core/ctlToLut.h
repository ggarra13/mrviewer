/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

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

#ifndef INCLUDED_CTL_TO_LUT_H
#define INCLUDED_CTL_TO_LUT_H

//-----------------------------------------------------------------------------
//
//	Run a set of CTL transforms to generate a color lookup table.
//
//	Function ctlToLut() applies a series of CTL transforms to a
//	set of input pixel values, pixelValues, producing a color lookup
//	table, lut.  A Cg shader in the display thread of the playexr
//	program will use this lookup table to approximate the effect
//	of applying the CTL transforms directly to the displayed images.
//
//	Parameters:
//
//	transformNames	A list of the names of the CTL transforms that will
//			be applied to the input pixel values.  If this list
//			is empty, ctlToLut() looks for a rendering transform
//			and a display transform:
//
//			If inHeader contains a string attribute called
//			"renderingTransform" then the value of this attribute
//			is the name of the rendering transform.
//			If inHeader contains no such attribute, then the
//			name of the rendering transform is "transform_RRT".
//
//			If the environment variable CTL_DISPLAY_TRANSFORM
//			is set, the value of the environment variable is
//			the name of the display transform.
//			If the environment variable is not set, then the name
//			of the display transform is "transform_display_video".
//
//	inHeader	The header of the first frame of the image sequence
//			displayed by playexr.  The attributes in the header
//			can be read by the CTL transforms.
//
//	lutSize		Number of elements in the pixelValues and lut arrays.
//
//	pixelValues,
//	lut		Input and output pixel data arrays.  Four channels
//			R, G, B and A are interleaved: RGBARGBARGBA...
//			The A channel is only for padding; it cannot be
//			accessed by the CTL transforms.
//
//
//	Function displayVideoGamma() returns 1/g, where g is the display's
//	video gamma.  The value of g is read from the environment variable
//	EXR_DISPLAY_VIDEO_GAMMA.  If the environment variable is not set,
//	then displayVideoGamma() returns 1 / 2.2.
//
//-----------------------------------------------------------------------------

#include <ImfCtlApplyTransforms.h>
#include <string>
#include <vector>
#include <half.h>


extern const char* kRGBChannels[];

void
ctlToLut (std::vector<std::string> transformNames,
	  Imf::Header inHeader,
	  size_t lutSize,
	  const float pixelValues[/*lutSize*/],
#ifdef CTL_GIT
	  float lut[/*lutSize*/],
	  const char* inChannels[4] = kRGBChannels);
#else
          half  lut[/*lutSize*/],
	  const char* inChannels[3] = kRGBChannels);
#endif

void
ctlToLut (std::vector<std::string> transformNames,
	  Imf::Header inHeader,
	  size_t lutSize,
	  const half pixelValues[/*lutSize*/],
	  half lut[/*lutSize*/],
#ifdef CTL_GIT
	  const char* inChannels[4] = kRGBChannels
#else
	  const char* inChannels[3] = kRGBChannels
#endif
	  );


#endif
