/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃ±o

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

#include "core/mrvTimer.h"

class Fl_Window;
class Fl_Progress;
class Fl_Output;

namespace mrv {

class ProgressReport
{
public:
    ProgressReport( Fl_Window* main, boost::int64_t start,
                    boost::int64_t end );
    ~ProgressReport();

    Fl_Window* window() const {
        return w;
    }

    bool tick();

    void show();

protected:
    //! Convert a double in seconds to hour, minutes and seconds
    void to_hour_min_sec( const double t, int& hour, int& min, int& sec );

protected:
    Fl_Window* w;
    Fl_Progress* progress;
    Fl_Output* elapsed;
    Fl_Output* remain;
    Fl_Output* fps;
    mrv::Timer timer;

    boost::int64_t _frame;
    boost::int64_t _end;
    double _time;
};

}
