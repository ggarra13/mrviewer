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

//----------------------------------------------------------------------------
//
//	Timing control for the display thread of the playExr prgram
//
//----------------------------------------------------------------------------

#include <mrvTimer.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)

  int
  gettimeofday (struct timeval *tv, void *tz)
  {
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tv->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tv->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
  } 

#endif


namespace mrv {

Timer::Timer ():
  playState( CMedia::kForwards ),
  _spf (1 / 24.0),
  _timingError (0),
  _framesSinceLastFpsFrame (0),
  _actualFrameRate (0)
{
  gettimeofday (&_lastFrameTime, 0);
  _lastFpsFrameTime = _lastFrameTime;
}


void
Timer::waitUntilNextFrameIsDue ()
{
  if (playState == CMedia::kStopped)
    {
      //
      // If we are not running, reset all timing state
      // variables and return without waiting.
      //
      
      gettimeofday (&_lastFrameTime, 0);
      _timingError = 0;
      _lastFpsFrameTime = _lastFrameTime;
      _framesSinceLastFpsFrame = 0;
      return;
    }

    //
    // If less than _spf seconds have passed since the last frame
    // was displayed, sleep until exactly _spf seconds have gone by.
    //

    timeval now;
    gettimeofday (&now, 0);

    _timeSinceLastFrame =  now.tv_sec  - _lastFrameTime.tv_sec +
                          (now.tv_usec - _lastFrameTime.tv_usec) * 1e-6f;

    double timeToSleep = _spf - _timeSinceLastFrame - _timingError;

    #ifdef _WIN32

	if (timeToSleep > 0)
	    Sleep (int (timeToSleep * 1000.0f));

    #else

	if (timeToSleep > 0)
	{
	    timespec ts;
	    ts.tv_sec = (time_t) timeToSleep;
	    ts.tv_nsec = (long) ((timeToSleep - ts.tv_sec) * 1e9f);
	    nanosleep (&ts, 0);
	}

    #endif

    //
    // If we slept, it is possible that we woke up a little too early
    // or a little too late.  Keep track of the difference between
    // now and the exact time when we wanted to wake up; next time
    // we'll try sleep that much longer or shorter.  This should
    // keep our average frame rate close to one fame every _spf seconds.
    //

    gettimeofday (&now, 0);

    float timeSinceLastSleep = now.tv_sec  - _lastFrameTime.tv_sec +
    		              (now.tv_usec - _lastFrameTime.tv_usec) * 1e-6f;
 
    _timingError += timeSinceLastSleep - _spf;

    if (_timingError < -2 * _spf)
    	_timingError = -2 * _spf;

    if (_timingError >  2 * _spf)
    	_timingError =  2 * _spf;

    _lastFrameTime = now;

    //
    // Calculate our actual frame rate, averaged over several frames.
    //

    if (_framesSinceLastFpsFrame >= 24)
    {
	float t =  now.tv_sec  - _lastFpsFrameTime.tv_sec +
		  (now.tv_usec - _lastFpsFrameTime.tv_usec) * 1e-6f;

	if (t > 0)
	    _actualFrameRate = _framesSinceLastFpsFrame / t;

	_framesSinceLastFpsFrame = 0;
    }

    if (_framesSinceLastFpsFrame == 0)
	_lastFpsFrameTime = now;

    _framesSinceLastFpsFrame += 1;
}

} // namespace mrv
