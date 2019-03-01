/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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
/**
 * @file   mrvTimecode.cpp
 * @author
 * @date   Fri Oct 13 07:58:06 2006
 *
 * @brief  an fltk input widget to display its internal value as either
 *         frames (normal Fl_Value_Input) or as timecode.
 *
 *
 */
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

#include <cassert>
#include <cstdio> // for sprintf()
#include <cstring> // for strchr

#include <iostream>
using namespace std;

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Rect.H>

extern "C" {
#include <libavutil/mathematics.h>
}

#include "mrvMath.h"  // for std::abs in some platforms
#include "gui/mrvTimecode.h"
#include "mrViewer.h"
#include "gui/mrvIO.h"

namespace
{
static const char* kModule = "gui";
}


namespace mrv
{

Timecode::Timecode( int x, int y, int w, int h, const char* l ) :
Fl_Float_Input( x, y, w, h, l ),
_display( mrv::Timecode::kFrames ),
_fps( 24.f ),
_frame( 1 ),
_tc_frame( 0 ),
_minimum( 1 ),
_maximum( 50 ),
_step( 1 )
{
    textcolor( FL_BLACK );
}


Timecode::Timecode( int w, int h, const char* l ) :
Fl_Float_Input( 0, 0, w, h, l ),
uiMain( NULL ),
_display( mrv::Timecode::kFrames ),
_fps( 24.f ),
_frame( 1 ),
_tc_frame( 0 ),
_minimum( 1 ),
_maximum( 50 ),
_step( 1 )
{
    textcolor( FL_BLACK );
}

/**
 * Set widget's value from a timecode
 *
 * @param hours  hours in timecode
 * @param mins   minutes in timecode
 * @param secs   seconds in timecode
 * @param frames frames in timecode
 */
void Timecode::value( const int hours, const int mins, const int secs,
                      const int frames )
{
    int64_t x = frames;
    int ifps = (int)round(_fps);
    x += secs * ifps;
    x += mins * 60 * ifps;
    x += hours * 3600 * ifps;
    value( x );
}

bool Timecode::valid_drop_frame( int hours, int mins, int secs, int frames,
                                 double fps )
{
    int ifps = int(fps);
    int maxf = int(2 * ifps / 30);
    if ( secs == 0 && (frames >= 0 && frames <= maxf) )
    {
        if ( (mins % 10) != 0 ) return false;
    }
    return true;
}

int Timecode::handle( int e )
{
    return Fl_Float_Input::handle( e );
    // if ( r != 0 ) return r;
    // return uiMain->uiView->handle( e );
}

int64_t Timecode::value() const
{
    switch( _display )
    {
    case kFrames:
        return atoi( Fl_Input::value() );
    case kSeconds:
        return int64_t( atof( Fl_Input::value() ) * _fps + 0.5 );
    case kTime:
    {
        int hours = 0, mins = 0, secs = 0, msecs = 0;
        int num = sscanf( Fl_Input::value(), "%02d:%02d:%02d.%03d",
                          &hours, &mins, &secs, &msecs );
        if ( num != 4 )
            return atoi( Fl_Input::value() );

        assert( msecs < 1000 );
        assert( secs < 60 );
        assert( mins < 60 );

        int64_t r = int64_t(msecs * _fps / 1000.0);
        r += int64_t(secs * _fps);
        r += int64_t(mins * 60 * _fps);
        r += int64_t(hours * 3600 * _fps);
        return r;
    }
    case kTimecodeNonDrop:
    {
        int hours = 0, mins = 0, secs = 0, frames = 0;
        int num = sscanf( Fl_Input::value(), "%02d:%02d:%02d:%02d",
                          &hours, &mins, &secs, &frames );
        if ( num != 4 )
            return atoi( Fl_Input::value() );

        assert( frames < _fps + 0.5 );
        assert( secs < 60 );
        assert( mins < 60 );

        int ifps = (int)round(_fps);

        int64_t r = frames + 1;  // timecode starts at 0, frames at 1
        r += secs * ifps;
        r += mins * 60 * ifps;
        r += hours * 3600 * ifps;
        r -= _tc_frame;
        return r;
    }
    case kTimecodeDropFrame:
    {
        int hours = 0, mins = 0, secs = 0, frames = 0;
        int num = sscanf( Fl_Input::value(), "%02d;%02d;%02d;%02d",
                          &hours, &mins, &secs, &frames );
        if ( num != 4 )
            return atoi( Fl_Input::value() );

        assert( frames < _fps + 0.5 );
        assert( secs < 60 );
        assert( mins < 60 );

        // Convert current frame value to timecode based on fps
        int ifps = int(_fps);
        int mult = int(30/ifps);
        int frames_per_hour = int(3600 * _fps);
        int64_t r = hours * frames_per_hour;

        if ( mins >= 10 )
        {
            /* there are 1800 frames in the first minutes of every 10
             * minutes, and there are 1798 frames in the remaining 9 minutes
             * of each 10 minutes.  We multiply by ifps to also support 59.94 TC
             * and other multiples.
             */
            int frames_per_ten_mins = mult * (9*1798 + 1800);
            int tmp = mins / 10;
            r += frames_per_ten_mins * tmp;
            mins -= tmp * 10;
        }

        assert( mins < 10 );

        /* minutes 1,2,3,4,5,6,7,8,9 all have a fixed number of frames in them:
         * 1798. So, let's chop time down to something that looks like
         * 00:00:yy:zz
         */
        if ( mins > 0 )
        {
            /* the first minutes of each 10 minutes has 1800 frames.
             */
            r += 1800 * mult;
            --mins;
            /* Chop off the dropped frames--at this point, we need to
             * loose the two frames
             */
            frames -= 2 * mult;

            /* all other minutes of each 10 minutes have 1798 frames
             */
            if ( mins > 0 )
            {
                r += 1798 * mins * mult;
            }
        }

        /* All seconds values except for 00 have 28 frames per seconds, so
         * let's pare down to get something that looks like 00:00:00:zz
         */
        if ( secs > 0 )
        {
            r += 30 * mult;
            --secs;
            if ( secs > 0 )
            {
                r += secs * 30 * mult;
            }
        }

        /* now, we can just add in the frames!!! +1 to compensate timecode
           starting at 0.
        */
        r += frames + 1;

        r -= _tc_frame;
        return r;
    }
    default:
        LOG_ERROR("Unknown timecode format");
        return 0;
    }
}

void Timecode::update()
{
    value( value() );
}

void Timecode::display( Timecode::Display x )
{
    int64_t v = frame();
    _display  = x;


    if ( _display == kFrames || _display == kSeconds )
        type( FL_FLOAT_INPUT );
    else
        type( FL_NORMAL_INPUT );

    value( v );
    redraw();
}


int Timecode::format( char* buf, const mrv::Timecode::Display display,
                      const int64_t& f, const int64_t& tc, const double fps,
                      const bool withFrames )
{
    switch( display )
    {
    case kFrames:
        return sprintf( buf, "%" PRId64, f );
    case kSeconds:
        return sprintf( buf, "%.3f", ((double)f / fps) );
    case kTime:
    {
        int  hours  = 0;
        int  mins   = 0;
        int  secs   = 0;
        int  msecs  = 0;

        // Convert current frame value to time based on fps
        double hourbase = 3600 * fps;
        double minbase  = 60 * fps;
        double secbase  = fps;

        int64_t x = f;
        hours = int( double(x+0.5) / hourbase );
        x -= int64_t( double(hours) * hourbase);
        mins = int( double(x+0.5) / minbase );
        x -= int64_t( double(mins) * minbase);
        secs = int( double(x+0.5) / secbase );
        x -= int64_t( double(secs) * secbase);
        msecs = int(x); // frames
        x -= msecs;

        msecs = int(1000.0 * double(msecs) / fps);

        if ( msecs >= 1000 ) {
            msecs -= 1000;
            ++secs;
        }

        // If negative timecode, make hour negative only
        hours = abs(hours);
        mins = abs(mins);
        secs = abs(secs);
        if ( msecs < 0 ) {
            hours = -hours;
            msecs = -msecs;
        }

        if ( withFrames )
        {
            return sprintf( buf, "%02d:%02d:%02d.%03d",
                            hours, mins, secs, msecs );
        }
        else
        {
            return sprintf( buf, "%02d:%02d:%02d", hours, mins, secs );
        }
    }
    case kTimecodeNonDrop:
    {

        int  hours  = 0;
        int  mins   = 0;
        int  secs   = 0;
        int  frames = 0;

        // Convert current frame value to timecode based on fps
        int ifps = (int)round(fps);
        int frames_per_hour = 3600 * ifps;
        int minbase  = 60 * ifps;
        int secbase  = ifps;


        // timecode starts at 0 so we substract 1 from frame
        int64_t x = f - 1 + tc;
        hours = int( x / frames_per_hour );
        x -= hours * frames_per_hour;
        mins = int( x / minbase );
        x -= mins * minbase;
        secs = int( x / secbase );
        x -= secs * secbase;
        frames = int(x);

        // If negative timecode, add negative sign
        const char* c = "";
        if ( frames < 0 || secs < 0 || mins < 0 || hours < 0 )
            c = "-";

        hours = abs(hours);
        mins = abs(mins);
        secs = abs(secs);
        frames = abs(frames);

        if ( withFrames )
        {
            return sprintf( buf, "%s%02d:%02d:%02d:%02d",
                            c, hours, mins, secs, frames );
        }
        else
        {
            return sprintf( buf, "%s%02d:%02d:%02d",
                            c, hours, mins, secs );
        }
    }
    case kTimecodeDropFrame:
    {
        int ifps = int(fps + 0.5f);
        if ( ifps < 30 || ifps % 30 != 0 )
            return format( buf, kTimecodeNonDrop, f, tc,
                           fps, withFrames );


        int  hours  = 0;
        int  mins   = 0;
        int  secs   = 0;
        int  frames = 0;

        // Convert current frame value to timecode based on fps
        int frames_per_hour = int(3600 * fps);

        int64_t x = f - 1 + tc; // timecode starts at 0
        hours = int( x / frames_per_hour );
        x -= int64_t(hours * frames_per_hour);

        /* there are 1800 frames in the first minutes of every 10
         * minutes, and there are 1798 frames in the remaining 9 minutes
         * of each 10 minutes.  We multiply by ifps to also support 59.94 TC
         * and other multiples.
         */
        int frames_per_ten_mins = ifps * (9*1798 + 1800) / 30;

        mins  = int(x / frames_per_ten_mins);
        x    -= mins * frames_per_ten_mins;
        mins *= 10;

        /* Now we have less than 10 minutes worth of frames
         */

        // 0 min is multiple of 10 and thus 1800 frames
        int fpm = 1800*ifps/30;
        if ( x >= fpm )
        {
            ++mins;
            x -= fpm;

            // Other mins are not multiple of 10 and thus 1798 frames
            fpm = 1798*ifps/30;
            while ( x >= fpm )
            {
                ++mins;
                x -= fpm;
            }
        }

        /* now we have less than 1 minute worth of frames
         */
        int tfps;
        if ( mins % 10 == 0 )
        {
            tfps = int(30 * (ifps / 30));
        }
        else
        {
            tfps = int(28 * (ifps / 30));
        }

        if ( x >= tfps )
        {
            ++secs;
            x -= tfps;

            tfps = int(30 * (ifps / 30));

            while ( x >= tfps ) {
                ++secs;
                x -= tfps;
            }
        }

        /* Now, we have the correct number of frames
         */
        if ( (mins % 10 != 0) && secs == 0 )
        {
            frames  = int(x);
            frames += int(2 * (ifps/30));
        }
        else
        {
            frames = int(x);
        }

        // If negative timecode, add negative sign
        const char* c = "";
        if ( frames < 0 || secs < 0 || mins < 0 || hours < 0 )
            c = "-";

        hours = abs(hours);
        mins = abs(mins);
        secs = abs(secs);
        frames = abs(frames);

        // Sanity check
        assert( valid_drop_frame( hours, mins, secs, frames, fps ) );

        if ( withFrames )
        {
            return sprintf( buf, "%s%02d;%02d;%02d;%02d",
                            c, hours, mins, secs, frames );
        }
        else
        {
            return sprintf( buf, "%s%02d;%02d;%02d",
                            c, hours, mins, secs );
        }
    }
    default:
        LOG_ERROR("Unknown timecode format");
        return 0;
    }
}

void Timecode::value( const int64_t x )
{
    _frame = x;

    char buf[100];
    int n = format( buf, _display, x, _tc_frame, _fps, true );
    Fl_Input::value(buf, n);
}


bool Timecode::replace(int b, int e, const char* text, int ilen) {

    for (int n = 0; n < ilen; n++) {
        char ascii = text[n];
        Fl::compose_reset(); // ignore any foreign letters...

        // Allow only one '.' in FLOAT inputs
        if (input_type()==FL_FLOAT_INPUT && ( ascii=='.' || ascii==',') ) {
            if (!strchr(Fl_Input::value(), ascii))
                continue;
        } else
            // This is complex to allow "00:00:01" or "00;00;00;22" timecode
            // to be typed:
            if (b+n==0 && (ascii == '+' || ascii == '-') ||
                (ascii >= '0' && ascii <= '9') ||
                (input_type()==FL_FLOAT_INPUT && ascii &&
                     strchr(".,eE+-", ascii)) ||
                    ((input_type()==FL_NORMAL_INPUT) &&
                     ascii && (ascii == ';' ||
                               ascii == ':' ||
                               ascii == '.')))
                continue; // it's ok;

        return false;
    }
    return Fl_Input::replace(b,e,text,ilen);
}



}  // namespace mrv
