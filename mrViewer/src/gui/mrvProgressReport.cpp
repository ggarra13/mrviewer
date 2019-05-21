/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuno

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

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

#include <FL/Fl_Output.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>


#include "mrvProgressReport.h"


namespace mrv {

ProgressReport::ProgressReport( Fl_Window* main, boost::int64_t start,
                                boost::int64_t end ) :
    _time( 0 ),
    _frame( start ),
    _end( end )
{
    Fl_Group::current(main);
    w = new Fl_Window( main->x() + main->w() / 2 - 320,
                       main->y() + main->h()/2,
                       640, 120 );
    w->size_range( 640, 120 );
    w->begin();
    Fl_Group* g = new Fl_Group( 0, 0, w->w(), 120 );
    g->begin();
    g->box( FL_UP_BOX );
    progress = new Fl_Progress( 0, 20, g->w(), 40 );
    progress->minimum( 0 );
    progress->maximum( double( end - start + 1) );
    progress->align( FL_ALIGN_TOP );
    char title[1024];
    sprintf( title, _( "Saving Sequence(s) %" PRId64 " - %" PRId64 ),
             start, end );
    progress->copy_label( title );
    // progress->showtext(true);
    elapsed = new Fl_Output( 120, 80, 120, 20, _("Elapsed") );
    elapsed->labelsize( 16 );
    elapsed->box( FL_FLAT_BOX );
    elapsed->textcolor( FL_BLACK );
    elapsed->set_output(); // needed so no selection appears
    remain = new Fl_Output( 350, 80, 120, 20, _("Remaining") );
    remain->labelsize( 16 );
    remain->box( FL_FLAT_BOX );
    remain->textcolor( FL_BLACK );
    remain->set_output(); // needed so no selection appears
    fps = new Fl_Output( 550, 80, 60, 20, _("FPS") );
    fps->labelsize( 16 );
    fps->box( FL_FLAT_BOX );
    fps->textcolor( FL_BLACK );
    fps->set_output(); // needed so no selection appears
    g->end();
    w->resizable(w);
    w->set_modal();
    w->end();
    Fl_Group::current(0);

    timer.setDesiredFrameRate( 500.0 );

}

ProgressReport::~ProgressReport()
{
    delete w;
    w = NULL;
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
    progress->value( progress->value() + 1);

    timer.waitUntilNextFrameIsDue();
    double now = timer.timeSinceLastFrame();
    _time += now;


    int hour, min, sec;
    to_hour_min_sec( _time, hour, min, sec );

    char buf[120];
    sprintf( buf, " %02d:%02d:%02d", hour, min, sec );
    elapsed->value( buf );

    double r = _time / (double)_frame;
    r *= (_end - _frame);


    to_hour_min_sec( r, hour, min, sec );

    sprintf( buf, " %02d:%02d:%02d", hour, min, sec );
    remain->value( buf );

    sprintf( buf, " %3.2f", timer.actualFrameRate() );
    fps->value( buf );

    Fl::check();
    ++_frame;

    if ( !w->visible() ) return false;
    return true;
}

}  // namespace mrv
