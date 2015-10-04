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

#define __STDC_FORMAT_MACROS

#include <inttypes.h>

#include <fltk/Output.h>
#include <fltk/ProgressBar.h>
#include <fltk/Window.h>
#include <fltk/run.h>
#include <fltk/ask.h>


#include "mrvProgressReport.h"


namespace mrv {

ProgressReport::ProgressReport( fltk::Window* main, boost::int64_t start,
                                boost::int64_t end ) :
_time( 0 ),
_frame( start ),
_end( end )
{
    w = new fltk::Window( main->x(), main->y() + main->h()/2, 
                          main->w(), 120 );
    w->child_of(main);
    w->begin();
    elapsed = new fltk::Output( 120, 80, 150, 20, _("Elapsed") );
    remain = new fltk::Output( 350, 80, 150, 20, _("Remaining") );
    fps = new fltk::Output( 560, 80, 60, 20, _("FPS") );
    progress = new fltk::ProgressBar( 0, 20, w->w(), 40 );
    progress->range( 0, double( end - start + 1) );
    progress->align( fltk::ALIGN_TOP );
    char title[1024];
    sprintf( title, _( "Saving Sequence(s) %" PRId64 " - %" PRId64 ),
             start, end );
    progress->copy_label( title );
    progress->showtext(true);
    w->resizable(progress);
    w->set_modal();
    w->end();

   timer.setDesiredFrameRate( 500.0 );

}

ProgressReport::~ProgressReport()
{
    delete w; w = NULL;
}

void ProgressReport::show()
{
    w->show();
}

void ProgressReport::to_hour_min_sec( const double t, 
                                      int& hour, int& min, int& sec )
{
    hour = int(floor( t / 3600.0 ));
    min = int(floor( t / 60.0 )) % 60;
    sec = int(floor( t )) % 60;
}

bool ProgressReport::tick()
{
    progress->step(1);

    timer.waitUntilNextFrameIsDue();
    ++_frame;
    double now = timer.timeSinceLastFrame();
    _time += now;


    int hour, min, sec;
    to_hour_min_sec( _time, hour, min, sec );

    char buf[120];
    sprintf( buf, "%02d:%02d:%02d", hour, min, sec );
    elapsed->value( buf );

    double r = _time / (double)_frame;
    r *= (_end - _frame);

    to_hour_min_sec( r, hour, min, sec );

    sprintf( buf, "%02d:%02d:%02d", hour, min, sec );
    remain->value( buf );

    sprintf( buf, "%3.2f", timer.actualFrameRate() );
    fps->value( buf );

    fltk::check();

    if ( !w->visible() ) return false;
    return true;
}

}  // namespace mrv
