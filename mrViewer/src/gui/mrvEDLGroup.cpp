
#include <limits>

#include <fltk/events.h>
#include <fltk/draw.h>
#include <fltk/Color.h>
#include <fltk/Window.h>

#include "mrViewer.h"
#include "gui/mrvHotkey.h"
#include "gui/mrvMediaTrack.h"
#include "gui/mrvEDLGroup.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvElement.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"
#include "mrvEDLWindowUI.h"

namespace {
const char* kModule = "edl";
}

namespace mrv {

static int kTrackHeight = 68;
static int kXOffset = 64;

EDLGroup::EDLGroup(int x, int y, int w, int h) :
fltk::Group(x,y,w,h),
_drag( NULL ),
_dragX( 0 ),
_dragY( 0 )
{
}

EDLGroup::~EDLGroup()
{
   for ( int i = 0; i < children(); ++i )
   {
      delete child(i);
      remove( i );
   }

   _audio_track.clear();
}

ImageBrowser* EDLGroup::browser() const
{ 
   return uiMain->uiReelWindow->uiBrowser; 
}

ImageView* EDLGroup::view() const
{ 
   return uiMain->uiView; 
}

// Add a media track and return its index
unsigned EDLGroup::add_media_track( int r )
{
   unsigned e = children();

   mrv::media_track* o = new mrv::media_track(x(), y() + 78 * e,
					      w(), kTrackHeight);
 
   o->main( timeline()->main() );
   this->add( o );

   o->reel( r );

   return e;
}

bool  EDLGroup::shift_media_start( unsigned reel_idx, std::string s, 
				   boost::int64_t f )
{

   for ( int i = 0; i < 2; ++i )
   {
      mrv::media_track* t = (mrv::media_track*)child(i);
      if ( t->reel() == reel_idx )
      {
	 int idx = t->index_for(s);
	 mrv::media m = t->media( idx );
	 m->image()->first_frame( f );
	 t->refresh();
	 return true;
      }
   }
   return false;
}

bool  EDLGroup::shift_media_end( unsigned reel_idx, std::string s, 
				 boost::int64_t f )
{

   for ( int i = 0; i < 2; ++i )
   {
      mrv::media_track* t = (mrv::media_track*)child(i);
      if ( t->reel() == reel_idx )
      {
	 int idx = t->index_for(s);
	 mrv::media m = t->media( idx );
	 m->image()->last_frame( f );
	 t->refresh();
	 return true;
      }
   }
   return false;
}

// Add an audio only track and return its index
unsigned EDLGroup::add_audio_track()
{
   // _audio_track.push_back( mrv::audio_track_ptr() );
    return unsigned( _audio_track.size() - 1 );
}

// Return the number of media tracks
unsigned EDLGroup::number_of_media_tracks()
{
    return children();
}

// Return the number of audio only tracks
unsigned EDLGroup::number_of_audio_tracks()
{
    return (unsigned) _audio_track.size();
}


// Return an audio track at index i
audio_track_ptr& EDLGroup::audio_track( unsigned i )
{
   return _audio_track[i];
}

// Remove a media track at index i
void EDLGroup::remove_media_track( unsigned i )
{
   if ( children() == 1 )
   {
       mrv::media_track* track = (mrv::media_track*) child(i);
       mrv::Reel reel = browser()->reel_at( track->reel() );
       reel->edl = false;
   }
   remove( i );
}

// Remove an audio track at index i
void EDLGroup::remove_audio_track( unsigned i )
{
   _audio_track.erase( _audio_track.begin() + i );
}


void EDLGroup::pan( int diff )
{

   mrv::Timeline* t = timeline();  
 
   double amt = double(diff) / (double) t->w();
   double tmax = t->maximum();
   double tmin = t->minimum();
   double avg = tmax - tmin + 1;
   amt *= avg;
   
   t->minimum( tmin - amt );
   if ( t->minimum() < 0 ) t->minimum( 0 );

   t->maximum( tmax - amt );
   t->redraw();
   redraw();
   
}

int EDLGroup::handle( int event )
{
   switch( event )
   {
      case fltk::PUSH:
	 {
	    if ( fltk::event_key() == fltk::MiddleButton )
	    {
	       _dragX = fltk::event_x();
	       return 1;
	    }

	    if ( fltk::event_key() == fltk::LeftButton )
	    {
	       _dragX = fltk::event_x();
	       _dragY = fltk::event_y();

	       // LIMITS
	       if ( _dragX < 8 ) _dragX = 8;
	       if ( _dragY < 33 ) _dragY = 33;

	       int idx = int( _dragY / kTrackHeight );
	       if ( idx < 0 || idx >= children() ) {
		  return 0;
	       }
	       _dragChild = idx;

	       mrv::Timeline* t = timeline();
               if ( !t ) return 0;

	       int ww = t->w();
	       double len = (t->maximum() - t->minimum() + 1);
	       double p = double(_dragX) / double(ww);
	       p = t->minimum() + p * len;
               int64_t pt = int64_t( p );

	       mrv::media_track* track = (mrv::media_track*) child(idx);
	       mrv::media m = track->media_at( pt );

	       if ( m )
	       {
   		  _drag = ImageBrowser::new_item( m );
		  int j = track->index_for( m );
                  if ( j < 0 ) {
                     return 0;
                  }

                  browser()->reel( track->reel() );
                  DBG("Change  image " << j );
                  // browser()->change_image( j );
                  view()->stop();
                  view()->seek( pt );
                  DBG("Changed image " << j );
                  browser()->redraw();
		  return 1;
	       }
	    }

	    for ( int i = 0; i < children(); ++i )
	    {
	       fltk::Widget* c = this->child(i);
	       if (fltk::event_x() < c->x() - kXOffset) continue;
	       if (fltk::event_x() >= c->x()+c->w()) continue;
	       if (fltk::event_y() < c->y() - y() ) continue;
	       if (fltk::event_y() >= c->y() - y() +c->h()) continue;

	       if ( c->send( event ) ) return 1;
	    }
	    return 0;
	    break;
	 }
      case fltk::ENTER:
	 focus(this);
	 window()->show();
	 return 1;
	 break;
      case fltk::FOCUS:
	 return 1;
	 break;
      case fltk::KEY:
	 {
	    int key = fltk::event_key();
	
	    if ( key == fltk::DeleteKey )
	    {
	       browser()->remove_current();
	       return 1;
	    }

            if ( kPlayBack.match( key ) )
            {

                mrv::ImageView* v = view();
                mrv::media fg = v->foreground();
                if ( ! fg ) return 1;

                const CMedia* img = fg->image();
                double FPS = 24;
                if ( img ) FPS = img->play_fps();
                v->fps( FPS );

                if ( v->playback() != ImageView::kStopped )
                    v->stop();
                else
                    v->play_backwards();
                return 1;
            }
            else if ( kPlayFwd.match( key ) )
            {

                mrv::ImageView* v = view();
                mrv::media fg = v->foreground();
                if ( ! fg ) return 1;

                const CMedia* img = fg->image();
                double FPS = 24;
                if ( img ) FPS = img->play_fps();
                v->fps( FPS );

                if ( v->playback() != ImageView::kStopped )
                    v->stop();
                else
                    v->play_forwards();
                return 1;
            }
            else if ( kFrameStepFwd.match(key) )
            {
                view()->step_frame( 1 );
                return 1;
            }
            else if ( kFrameStepBack.match(key) )
            {
                view()->step_frame( -1 );
                return 1;
            }
	    else if ( key == 'f' || key == 'a' )
	    {
	       unsigned i = 0;
	       unsigned e = children();

	       int64_t tmin = std::numeric_limits<int64_t>::max();
	       int64_t tmax = std::numeric_limits<int64_t>::min();

               fltk::Choice* c1 = main()->uiEDLWindow->uiEDLChoiceOne;
               fltk::Choice* c2 = main()->uiEDLWindow->uiEDLChoiceTwo;

               int one = c1->value();
               int two = c2->value();

               if ( one == -1 && two == -1 )
               {
                  tmin = 1;
                  tmax = 100;
               }

	       for ( ; i != e; ++i )
	       {

                  if ( i != one && i != two ) continue;

		  mrv::media_track* o = (mrv::media_track*)child(i);
		  mrv::Reel r = browser()->reel_at( i );
                  if (!r) continue;

		  mrv::MediaList::iterator j = r->images.begin();
		  mrv::MediaList::iterator k = r->images.end();

		  mrv::media fg = view()->foreground();

                  if ( key == 'f' )
                  {
                     for ( ; j != k; ++j )
                     {
                        const mrv::media& m = *j;
                        if (m == fg)
                        {
                           int64_t tmi = m->position();
                           int64_t tma = m->position() + m->image()->duration();
                           if ( tmi < tmin ) tmin = tmi;
                           if ( tma > tmax ) tmax = tma;
                           break;
                        }
                     }

                     if ( j == k )
                     {
                        tmin = 1;
                        tmax = 100;
                     }

                  }
                  else
                  {

                     for ( ; j != k; ++j )
                     {
                        const mrv::media& m = *j;
                        int64_t tmi = m->position();
                        int64_t tma = m->position() + m->image()->duration();
                        if ( tmi < tmin ) tmin = tmi;
                        if ( tma > tmax ) tmax = tma;
                     }
                  }
               }

	       mrv::Timeline* t = timeline();
	       t->minimum( double(tmin) );
	       t->maximum( double(tmax) );
	       t->redraw();
	       redraw();
	       return 1;
	    }
	 }
	 break;
      case fltk::MOUSEWHEEL:
	if ( fltk::event_dy() < 0.f )
	  {
	     zoom( 0.5 );
	  }
	else
	  {
	     zoom( 2.0 );
	  }
	return 1;
	break;
      case fltk::RELEASE:
	 if ( fltk::event_key() == fltk::LeftButton )
	 {
	    _dragX = fltk::event_x();
	    _dragY = fltk::event_y();

	    int idx = int( _dragY / kTrackHeight );
	    if ( idx < 0 || idx >= 2 || idx >= children() ) {
	       delete _drag;
	       _drag = NULL;
	       redraw();
	       return 1;
	    }


	    mrv::media_track* t1 = (mrv::media_track*)child( _dragChild );
	    mrv::media_track* t2 = (mrv::media_track*)child( idx );

            if ( t2->reel() == -1 ) {
	       delete _drag;
	       _drag = NULL;
	       redraw();
               return 1;
            }

	    mrv::Timeline* t = timeline();
            if (!t) return 0;
	    
	    int ww = t->w();
	    double len = (t->maximum() - t->minimum() + 1);
	    double p = double(_dragX) / double(ww);
	    p = t->minimum() + p * len;
            int64_t pt = int64_t( p );


            mrv::Reel r = browser()->current_reel();
	    mrv::media m = _drag->element();
	    if ( t1 == t2 )
	    {
	       if ( pt < m->position() )
	       {
		  t1->remove( m );
		  t1->insert( pt, m );
		  t1->refresh();
	       }
	       else
	       {
		  t1->insert( pt, m );
		  t1->remove( m );
		  t1->refresh();
	       }

               browser()->reel( r->name.c_str() );
               browser()->redraw();
	    }
	    else
	    {
                t2->insert( pt, m );
                t1->remove( m );
                t2->refresh();

                browser()->reel( r->name.c_str() );
                browser()->redraw();
	    }

            timeline()->value( pt );
            view()->seek( pt );
	    delete _drag;
	    _drag = NULL;
            _dragChild = -1;
	    redraw();
	    return 1;
	 }
	 break;
      case fltk::TIMEOUT:
	 {
	    int X = fltk::event_x();
	    if ( X >= w()-128 ) {
	       pan(-1);
	       repeat_timeout( 0.25 );
	    }
	    else if ( X <= 128 )
	    {
	       pan(1);
	       repeat_timeout( 0.25 );
	    }
	    redraw();
	    return 1;
	 }
      case fltk::DRAG:
	 {
	    int diff = ( fltk::event_x() - _dragX );

	    if ( fltk::event_key() == fltk::LeftButton )
	    {
	       int X = fltk::event_x();
	       _dragY = fltk::event_y();

	       // LIMITS
	       if ( _dragY < 33 ) _dragY = 33;
	       if ( X < 8 ) X = 8;


	       if ( X >= w()-128 ) {
		  pan(diff * 2);
		  add_timeout( 0.1f );
	       }
	       else if ( X <= 128 )
	       {
		  pan(diff * -2);
		  add_timeout( 0.1f );
	       }
	       
	       _dragX = X;
	       

	       redraw();
	       return 1;
	    }
	    else if ( fltk::event_key() == fltk::MiddleButton )
	    {
	       pan(diff * 2);
	       _dragX = fltk::event_x();
	       return 1;
	    }
	 } 
	 break;
      default:
	 break;
   }

   return fltk::Group::handle( event );
}

void EDLGroup::zoom( double z )
{

    mrv::Timeline* t = timeline();

    double pct = (double) fltk::event_x() / (double)w();

    int64_t tmin = int64_t( t->minimum() );
    int64_t tmax = int64_t( t->maximum() );


   int64_t tdiff = tmax - tmin;
   int64_t tcur = tmin + int64_t( pct * double(tdiff) );


   tmax = int64_t( double(tmax) * z );
   tmin = int64_t( double(tmin) * z );

   int64_t tlen = tmax - tmin;

   tmax = tcur + int64_t( ( 1.0 - pct ) *  double(tlen));
   tmin = tcur - int64_t( double(tlen) * pct );


   if ( tmin < 0.0 ) tmin = 0.0;

   t->minimum( double(tmin) );
   t->maximum( double(tmax) );
   t->redraw();


   unsigned e = children();
   for ( unsigned i = 0; i < e; ++i )
   {
      mrv::media_track* c = (mrv::media_track*)child(i);
      c->zoom( z );
   }

}

void EDLGroup::cut( boost::int64_t frame )
{
    fltk::Choice* c1 = main()->uiEDLWindow->uiEDLChoiceOne;
    int c = c1->value();
    if ( c < 0 ) return;

    mrv::Reel r = browser()->reel_at(c);
    if (!r) return;

    CMedia* img = r->image_at( frame );
    if ( !img ) return;

    size_t idx = r->index( frame );
    int64_t f = r->global_to_local( frame );

    if ( img->first_frame() == f || img->last_frame() == f )
        return;


    CMedia* right = CMedia::guess_image( img->fileroot(), NULL, 0, f,
                                         img->last_frame() );
    if (!right) return;

    mrv::media m( new mrv::gui::media( right ) );

    right->last_frame( img->last_frame() );
    img->last_frame( f-1 );
    right->first_frame( f );
    right->fetch( f );

    browser()->insert( unsigned(idx+1), m );
    browser()->value(idx+1);
    refresh();
    redraw();
}

void EDLGroup::refresh()
{
   unsigned e = children();
   for ( unsigned i = 0; i < e; ++i )
   {
      DBG( "REFRESH MEDIA TRACK " << i );
      mrv::media_track* o = (mrv::media_track*) child(i);
      o->refresh();
      o->redraw();
   }
}

void EDLGroup::draw()
{

   fltk::setcolor( fltk::GRAY20 );
   fltk::fillrect( 0, 0, w(), h() );


   fltk::Group::draw();

   mrv::Timeline* t = uiMain->uiTimeline;
   double frame = t->value();
   t = timeline();
   t->value( frame );
   // double p = double(frame - t->minimum()) / 
   //            double(t->maximum() - t->minimum());
   // p *= t->w();

   int p = t->slider_position( double(frame), t->w() );
   p += int( t->slider_size()/2.0 );


   fltk::setcolor( fltk::YELLOW );
   fltk::push_clip( 0, 0, w(), h() );
   fltk::drawline( p, 0, p, h() );
   fltk::pop_clip();

   if ( _drag )
   {
      fltk::push_matrix();
      fltk::translate( _dragX, _dragY );
      _drag->draw();
   //    int ww, hh;
   //    fltk::measure( e->label(), ww, hh );
   //    fltk::drawtext( e->label(), 0, 0 );
      fltk::pop_matrix();
   }
}

} // namespace mrv


