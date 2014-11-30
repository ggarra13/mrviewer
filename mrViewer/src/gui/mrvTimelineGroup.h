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
/**
 * @file   mrvTimelineGroup.h
 * @author gga
 * @date   Sat Oct 14 22:13:01 2006
 * 
 * @brief  Group encompassing a timeline, current frame, start frame,
 *         end frame, video controls and frame rate controls. 
 *         Changing timeline automatically updates current frame, start
 *         frame, end frame, etc.
 * 
 */

#ifndef mrvTimelineGroup_h
#define mrvTimelineGroup_h

#include <fltk/Group.h>


namespace mrv 
{

class Timeline;
class Timecode;

class TimelineGroup : public fltk::Group
{
public:
  TimelineGroup( int x, int y, int w, int h, char* l = 0);
  TimelineGroup( int w, int h, char* l = 0 );

  void   minimum( double x );
  double minimum() { return timeline_->minimum(); }

  void   maximum( double x );
  double maximum() { return timeline_->maximum(); }

  double value();
  void   value( double x ) { timeline_->value(x); }

  double fps() { return timeline_->fps(); }
  void   fps( double x );

protected:
  Timeline* timeline_;
  Timecode* frame_;
  Timecode* start_;
  Timecode* end_;
};


} // namespace mrv


#endif // mrvTimelineGroup_h
