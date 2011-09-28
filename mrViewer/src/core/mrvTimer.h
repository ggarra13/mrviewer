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

#ifndef mrvTimer_h
#define mrvTimer_h

//----------------------------------------------------------------------------
//
//	Timing control for the display thread of the playExr prgram
//
//----------------------------------------------------------------------------

#if defined(WIN32) || defined(WIN64)
    #include <winsock2.h>  // for timeval
    #include <windows.h>
    #undef max  // love windows
    #undef min
#else
    #include <sys/time.h>
#endif

#include "CMedia.h"

namespace mrv {


  class Timer
  {
  public:

    //------------
    // Constructor
    //------------

    Timer ();


    //--------------------------------------------------------
    // Timing control to maintain the desired frame rate:
    // the redrawWindow() function in the display thread calls
    // waitUntilNextFrameIsDue() before displaying each frame.
    //
    // If playState == RUNNING, then waitUntilNextFrameIsDue()
    // sleeps until the apropriate amount of time has elapsed
    // since the last call to waitUntilNextFrameIsDue().
    // If playState != RUNNING, then waitUntilNextFrameIsDue()
    // returns immediately.
    //--------------------------------------------------------

    void	waitUntilNextFrameIsDue ();


    void     audioTiming( double t ) { _timingError += t; }

    //-------------------------------------------------
    // Set and get the frame rate, in frames per second
    //-------------------------------------------------

    void	setDesiredFrameRate (double fps) { _spf = 1 / fps; }
    inline double actualFrameRate() { return _actualFrameRate; }


    //-------------------------------------------------
    // Get time since last frame
    //-------------------------------------------------
    inline double timeSinceLastFrame() { return _timeSinceLastFrame; }


    inline void setDesiredSecondsPerFrame( double x ) { _spf = x; }

    //-------------------
    // Current play state
    //-------------------
    
    CMedia::Playback	playState;


  private:

    double	_spf;				// desired frame rate,
    						// in seconds per frame

    timeval	_lastFrameTime;			// time when we displayed the
    						// last frame

    double	_timingError;			// cumulative timing error
    double      _timeSinceLastFrame;            // time since last frame

    timeval	_lastFpsFrameTime;		// state to keep track of the
    int		_framesSinceLastFpsFrame;	// actual frame rate, averaged
    double	_actualFrameRate;		// over several frames
  };

}


#endif  // mrvTimer_h

