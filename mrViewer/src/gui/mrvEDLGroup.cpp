
#include <limits>

#include <fltk/events.h>
#include <fltk/draw.h>
#include <fltk/Color.h>
#include <fltk/Window.h>

#include "mrViewer.h"
#include "gui/mrvMediaTrack.h"
#include "gui/mrvEDLGroup.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvElement.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"
#include "mrvEDLWindowUI.h"

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
size_t EDLGroup::add_media_track( size_t r )
{
   size_t e = children();

   mrv::Reel reel = browser()->reel( r );
   if (! reel ) return 0;

   mrv::media_track* o = new mrv::media_track(x(), y() + 70 * e,
					      w(), kTrackHeight);
 
   o->main( timeline()->main() );
   o->reel( r );

   this->add( o );

   fltk::Choice* c1 = main()->uiEDLWindow->uiEDLChoiceOne;
   fltk::Choice* c2 = main()->uiEDLWindow->uiEDLChoiceTwo;

   int one = c1->value();
   c1->clear();

   int two = c2->value();
   c2->clear();

   int reels = browser()->number_of_reels();
   for ( int i = 0; i < reels; ++i )
   {
      mrv::Reel track = browser()->reel( i );
      c1->add( track->name.c_str() );
      c2->add( track->name.c_str() );
   }

   if ( e == 0 )
   {
      c1->value( r );
      c2->value( two );
   }
   else
   {
      c1->value( one );
      c2->value( r );
   }

   e = children() - 1;
   
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
size_t EDLGroup::add_audio_track()
{
   // _audio_track.push_back( mrv::audio_track_ptr() );
   return _audio_track.size() - 1;
}

// Return the number of media tracks
size_t EDLGroup::number_of_media_tracks()
{
   return children();
}

// Return the number of audio only tracks
size_t EDLGroup::number_of_audio_tracks()
{
   return _audio_track.size();
}


// Return an audio track at index i
audio_track_ptr& EDLGroup::audio_track( int i )
{
   return _audio_track[i];
}

// Remove a media track at index i
void EDLGroup::remove_media_track( int i )
{
   remove( i );
}

// Remove an audio track at index i
void EDLGroup::remove_audio_track( int i )
{
   _audio_track.erase( _audio_track.begin() + i );
}


void EDLGroup::pan( int diff )
{

   mrv::Timeline* t = timeline();  
 
   double amt = double(diff) / (double) t->w();
   double avg = t->maximum() - t->minimum() + 1;
   amt *= avg;
   
   t->minimum( t->minimum() - amt );
   t->maximum( t->maximum() - amt );
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

	       int idx = _dragY / kTrackHeight;
	       if ( idx < 0 || idx >= children() ) {
		  return 0;
	       }
	       _dragChild = idx;

	       mrv::Timeline* t = timeline();
               if ( !t ) return 0;

	       int ww = t->w();
	       double len = (t->maximum() - t->minimum() + 1);
	       double p = double(_dragX) / double(ww);
	       p = t->minimum() + p * len + 0.5f;

	       mrv::media_track* track = (mrv::media_track*) child(idx);
	       mrv::media m = track->media_at( p );


	       if ( m )
	       {
   		  _drag = ImageBrowser::new_item( m );
		  int j = track->index_for( m );
                  if ( j < 0 || j >= children() ) return 0;
		  assert( j != -1 );

                  view()->stop();
		  browser()->reel( idx );
		  browser()->change_image( j );
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
	       // size_t i = 0;
	       // size_t e = children();

	       // for ( ; i != e; ++i )
	       // {
	       // 	  mrv::media_track* o = (mrv::media_track*)child(i);
	       // 	  mrv::Element* elem = o->selected();
	       // 	  if ( elem )
	       // 	  {
	       // 	     mrv::media m = elem->element();
	       // 	     browser()->reel( o->reel() );
	       // 	     browser()->remove( m );
	       // 	     return 1;
	       // 	  }
	       // }
	       return 0;
	    }

	    if ( key == 'f' || key == 'a' )
	    {
	       size_t i = 0;
	       size_t e = children();

	       int64_t tmin = std::numeric_limits<int64_t>::max();
	       int64_t tmax = std::numeric_limits<int64_t>::min();

	       for ( ; i != e; ++i )
	       {

		  mrv::media_track* o = (mrv::media_track*)child(i);
		  mrv::Reel r = browser()->reel_at( i );
		  mrv::MediaList::iterator i = r->images.begin();
		  mrv::MediaList::iterator e = r->images.end();

		  mrv::media fg = view()->foreground();

		  for ( ; i != e; ++i )
		  {
		     mrv::media m = *i;
		     if (m == fg && key != 'a')
		     {
			int64_t tmi = m->position();
			int64_t tma = m->position() + m->image()->duration();
			if ( tmi < tmin ) tmin = tmi;
			if ( tma > tmax ) tmax = tma;
			break;
		     }
		  }

		  if ( i == e )
		  {
		     int64_t tmi = o->minimum();
		     int64_t tma = o->maximum();

		     if ( tmi == tma ) continue;

		     if ( tmi < tmin ) tmin = tmi;
		     if ( tma > tmax ) tmax = tma;
		  }
	       }

	       mrv::Timeline* t = timeline();
	       t->minimum( tmin );
	       t->maximum( tmax );
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

	    int idx = _dragY / kTrackHeight;
	    if ( idx >= children() ) {
	       delete _drag;
	       _drag = NULL;
	       redraw();
	       return 1;
	    }


	    mrv::media_track* t1 = (mrv::media_track*)child( _dragChild );
	    mrv::media_track* t2 = (mrv::media_track*)child( idx );

	    mrv::Timeline* t = timeline();
	    
	    int ww = t->w();
	    double len = (t->maximum() - t->minimum() + 1);
	    double p = double(_dragX) / double(ww);
	    p = t->minimum() + p * len + 0.5f;


	    mrv::media m = _drag->element();
	    if ( t1 == t2 )
	    {
	       if ( p < m->position() )
	       {
		  t1->remove( m );
		  t1->insert( p, m );
		  t1->refresh();
	       }
	       else
	       {
		  t1->insert( p, m );
		  t1->remove( m );
		  t1->refresh();
	       }
	    }
	    else
	    {
	       t2->insert( p, m );
	       t1->remove( m );
	       t2->refresh();
	    }

	    delete _drag;
	    _drag = NULL;
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
		  add_timeout( 0.1 );
	       }
	       else if ( X <= 128 )
	       {
		  pan(diff * -2);
		  add_timeout( 0.1 );
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

   int64_t tmin = t->minimum();
   int64_t tmax = t->maximum();


   int64_t tdiff = tmax - tmin;
   int64_t tcur = tmin + pct * tdiff;


   tmax *= z;
   tmin *= z;

   int64_t tlen = tmax - tmin;

   tmax = tcur + ( 1.0 - pct ) * tlen;
   tmin = tcur - tlen * pct;


   if ( tmin < 0.0 ) tmin = 0.0;

   t->minimum( tmin );
   t->maximum( tmax );
   t->redraw();


   size_t i = 0;
   size_t e = children();
   for ( ; i != e; ++i )
   {
      mrv::media_track* c = (mrv::media_track*)child(i);
      c->zoom( z );
   }

}

void EDLGroup::refresh()
{
   size_t e = children();
   for ( size_t i = 0; i < e; ++i )
   {
      mrv::media_track* o = (mrv::media_track*) child(i);
      o->refresh();
      o->redraw();
   }
}

void EDLGroup::draw()
{

   fltk::setcolor( fltk::GRAY20 );
   fltk::fillrect( x(), y(), w(), h() );

   fltk::Group::draw();

 
   if ( _drag )
   {
      fltk::push_matrix();
      fltk::translate( _dragX, _dragY );
      _drag->draw();
   //    int ww, hh;
   //    fltk::measure( e->label(), ww, hh );
   //    fltk::drawtext( e->label(), 0, 0 );
   //    fltk::pop_matrix();
   }
}

} // namespace mrv


