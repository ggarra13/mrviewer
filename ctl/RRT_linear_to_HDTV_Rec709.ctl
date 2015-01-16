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

//
// A simple rendering transform that converts the pixels
// of an ACES RGB image into OCES XYZ pixels for display.
//
// This rendering transform is a passthru for HDTV.
//
// This transform does not claim to be optimal
// in any sense, or to be an approximation of any RRT candidate
// under consideration.
//
// The transform consists of two steps:
//
//	- convert from the input image's RGB space to RGB with
//	  primaries and white point according to Rec. 709
//
//	- convert from Rec. 709 RGB to CIE XYZ.
//


import "utilities";


void 
RT_linear_to_HDTV_Rec709 
    (varying half R,				// ACES RGB pixels
     varying half G,
     varying half B,
     uniform Chromaticities chromaticities,	// RGB space of input image
     output varying half X_OCES,		// OCES XYZ pixels
     output varying half Y_OCES,
     output varying half Z_OCES)
{
    float toRec709[4][4] = mult_f44_f44 (RGBtoXYZ (chromaticities, 1.0), 
					 XYZtoRGB (rec709Chromaticities, 1.0));

    float RGB[3] = {R, G, B};
    RGB = mult_f3_f44 (RGB, toRec709);

    convertRGBtoXYZ_h (rec709Chromaticities, 1.0, RGB, X_OCES, Y_OCES, Z_OCES);
}
