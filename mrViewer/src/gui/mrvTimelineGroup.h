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
