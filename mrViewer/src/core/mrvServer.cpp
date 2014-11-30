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
//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <fstream>
#include <set>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

// #define DEBUG_COMMANDS
// #define BOOST_ASIO_ENABLE_HANDLER_TRACKING 1

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio.hpp>
#include <fltk/Font.h>

#include "mrvClient.h"
#include "mrvServer.h"
#include "gui/mrvEDLGroup.h"
#include "mrvEDLWindowUI.h"
#include "gui/mrvLogDisplay.h"
#include "gui/mrvIO.h"
#include "gui/mrvReel.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvVectorscope.h"
#include "gui/mrvHistogram.h"
#include "gui/mrvTimeline.h"
#include "mrvColorAreaUI.h"
#include "mrViewer.h"

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

//----------------------------------------------------------------------

namespace {
const char* const kModule = "server";
}


//----------------------------------------------------------------------

namespace mrv {


Parser::Parser( boost::asio::io_service& io_service, mrv::ViewerUI* v ) :
connected( false ),
socket_( io_service ),
ui( v )
{
}

Parser::~Parser()
{
   if ( !ui || !ui->uiView ) return;

   ParserList::iterator i = ui->uiView->_clients.begin();
   ParserList::iterator e = ui->uiView->_clients.end();
   for ( ; i != e; ++i  )
   {
      if ( *i == this )
      {
	 ui->uiView->_clients.erase( i );
	 break;
      }
   }
   ui = NULL;
}

void Parser::write( std::string s, std::string id )
{
    if ( !connected || !ui || !ui->uiView ) {
        return;
    }

   ParserList::const_iterator i = ui->uiView->_clients.begin();
   ParserList::const_iterator e = ui->uiView->_clients.end();

   for ( ; i != e; ++i )
   {
      std::string p = boost::lexical_cast<std::string>( (*i)->socket_.remote_endpoint() );
      if ( p == id ) {
	 continue;
      }
      LOG_CONN( s << " sent to " << p );
      (*i)->deliver( s );
   }
}

mrv::ImageView* Parser::view() const
{
   return ui->uiView;
}

mrv::ImageBrowser* Parser::browser() const
{
   return ui->uiReelWindow->uiBrowser;
}

mrv::EDLGroup* Parser::edl_group() const
{
   return ui->uiEDLWindow->uiEDLGroup;
}

bool Parser::parse( const std::string& s )
{
   if ( !connected || !ui || !view() ) return false;


   std::istringstream is( s );
   std::string cmd;
   is >> cmd;

   static mrv::Reel r;
   static mrv::media m;

   bool ok = false;

   mrv::ImageView* v = view();

   ParserList c = v->_clients;
   v->_clients.clear();

   LOG_CONN( "Received: " << s );

#ifdef DEBUG_COMMANDS
   DBG( "received: " << cmd );
#endif

   if ( cmd == N_("GLPathShape") )
   {
      Point xy;
      std::string points;
      GLPathShape* shape = new GLPathShape;
      std::getline( is, points );
      is.str( points );
      is.clear();
      is >> shape->r >> shape->g >> shape->b >> shape->a >> shape->pen_size
	 >> shape->frame;
      while ( is >> xy.x >> xy.y )
      {
	 shape->pts.push_back( xy );
      }
      v->add_shape( mrv::shape_type_ptr(shape) );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("GLErasePathShape") )
   {
      Point xy;
      std::string points;
      GLErasePathShape* shape = new GLErasePathShape;
      is.clear();
      std::getline( is, points );
      is.str( points );
      is.clear();
      is >> shape->pen_size >> shape->frame;
      while ( is >> xy.x >> xy.y )
      {
	 shape->pts.push_back( xy );
      }
      v->add_shape( mrv::shape_type_ptr(shape) );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("GLTextShape") )
   {
      Point xy;
      std::string font, text;
      static string old_text;
      static string old_font;
      unsigned font_size;
      std::getline( is, font, '"' ); // skip first quote
      std::getline( is, font, '"' );
      std::getline( is, text, '^' ); // skip first quote
      std::getline( is, text, '^' );

      GLTextShape* shape;
      mrv::ImageView* view = ui->uiView;
      if ( font == old_font && text == old_text && !view->shapes().empty() )
      {
         shape = dynamic_cast< GLTextShape* >( view->shapes().back().get() );
         if ( shape == NULL ) {
            LOG_ERROR( "Not a GLTextShape as last shape" );
            v->_clients = c;
            return false;
         }
      }
      else
      {
         shape = new GLTextShape;
      }

      shape->text( text );

      fltk::Font** fonts;
      unsigned i;
      unsigned num = fltk::list_fonts(fonts);
      for ( i = 0; i < num; ++i )
      {
         if ( font == fonts[i]->name() ) break;
      }
      if ( i >= num ) i = 0;


      shape->font( fonts[i] );
      is >> font_size >> shape->r >> shape->g >> shape->b >> shape->a
         >> shape->frame;
      is >> xy.x >> xy.y;
      shape->size( font_size );
      shape->pts.clear();
      shape->pts.push_back( xy );

      if ( font != old_font || text != old_text )
      {
         v->add_shape( mrv::shape_type_ptr(shape) );
         old_text = text;
         old_font = font;
      }

      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("FPS") )
   {
      double fps;
      is >> fps;

      v->fps( fps );

      ok = true;
   }
   else if ( cmd == N_("EDL") )
   {
      int b;
      is >> b;
      if ( b )
	 browser()->set_edl();
      else
	 browser()->clear_edl();
      ok = true;
   }
   else if ( cmd == N_("Looping") )
   {
      int i;
      is >> i;

      v->looping( (ImageView::Looping)i );
      ok = true;
   }
   else if ( cmd == N_("Selection") )
   {
      
      double x, y, w, h;
      is >> x >> y >> w >> h;
      v->selection( mrv::Rectd( x, y, w, h ) );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("UndoDraw") )
   {
      v->undo_draw();
      ok = true;
   }
   else if ( cmd == N_("RedoDraw") )
   {
      v->redo_draw();
      ok = true;
   }
   else if ( cmd == N_("Zoom") )
   {
      float z;
      is >> z;
      v->zoom( z );
      ok = true;
   }
   else if ( cmd == N_("Offset") )
   {
      float x, y;
      is >> x >> y;
      v->offset_x( x );
      v->offset_y( y );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("Channel") )
   {
      unsigned short ch;
      is >> ch;
      
      if ( v->foreground() )
	 v->channel( ch );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("FieldDisplay") )
   {
      int field;
      is >> field;
      v->field( (mrv::ImageView::FieldDisplay) field );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("Normalize") )
   {
      int b;
      is >> b;
      v->normalize( ( b != 0 ) );
      ui->uiNormalize->state( (b != 0 ) );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("WipeVertical") )
   {
      float b;
      is >> b;
      v->wipe_direction( ImageView::kWipeVertical );
      v->wipe_amount( b );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("WipeHorizontal") )
   {
      float b;
      is >> b;
      v->wipe_direction( ImageView::kWipeHorizontal );
      v->wipe_amount( b );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("NoWipe") )
   {
      v->wipe_direction( ImageView::kNoWipe );
      v->wipe_amount( 0.0f );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("Gain") )
   {
      float f;
      is >> f;
      v->gain( f );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("Gamma") )
   {
      float f;
      is >> f;
      v->gamma( f );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("Mask") )
   {
      float b;
      is >> b;
      v->masking( b );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("SafeAreas") )
   {
      int b;
      is >> b;
      v->safe_areas( (b != 0) );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("DisplayWindow") )
   {
      int b;
      is >> b;
      v->display_window( (b != 0) );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("Volume") )
   {
      float b;
      is >> b;
      v->volume( b );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("DataWindow") )
   {
      int b;
      is >> b;
      v->data_window( (b != 0) );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("UseLUT") )
   {
      int b;
      is >> b;
      ui->uiLUT->state( (b != 0) );
      v->use_lut( (b != 0) );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("AudioVolume") )
   {
      float t;
      is >> t;

      v->volume( t );
      ok = true;
   }
   else if ( cmd == N_("ShowBG") )
   {
      int b;
      is >> b;
      v->show_background( ( b != 0 ) );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("ShowPixelRatio") )
   {
      int b;
      is >> b;
      v->show_pixel_ratio( (b != 0) );
      v->redraw();
      ok = true;
   }
   else if ( cmd == N_("Reel") )
   {
      std::string name;
      std::getline( is, name, '"' ); // skip first quote
      std::getline( is, name, '"' );

      r = browser()->reel( name.c_str() );
      if (!r) {
	 r = browser()->new_reel( name.c_str() );
      }
      ok = true;
   }
   else if ( cmd == N_("TimelineMax") )
   {
       double x;
       is >> x;
       ui->uiTimeline->maximum( x );
       ui->uiTimeline->redraw();
       ui->uiEndFrame->value( boost::int64_t(x) );
       ui->uiEndFrame->redraw();
       ok = true;
   }
   else if ( cmd == N_("TimelineMin") )
   {
       double x;
       is >> x;
       ui->uiTimeline->minimum( x );
       ui->uiTimeline->redraw();
       ui->uiStartFrame->value( boost::int64_t(x) );
       ui->uiStartFrame->redraw();
       ok = true;
   }
   else if ( cmd == N_("FullScreen") )
   {
       int on;
       is >> on;
       ui->uiView->toggle_fullscreen();
       ok = true;
   }
   else if ( cmd == N_("PresentationMode" ) )
   {
       int on;
       is >> on;
       ui->uiView->toggle_presentation();
       ok = true;
   }
   else if ( cmd == N_("ShiftMediaStart") )
   {
      int reel;
      is >> reel;

      std::string imgname;
      std::getline( is, imgname, '"' ); // skip first quote
      std::getline( is, imgname, '"' );

      boost::int64_t diff;
      is >> diff;

      edl_group()->shift_media_start( reel, imgname, diff );

      ok = true;
   }
   else if ( cmd == N_("ShiftMediaEnd") )
   {
      int reel;
      is >> reel;

      std::string imgname;
      std::getline( is, imgname, '"' ); // skip first quote
      std::getline( is, imgname, '"' );

      boost::int64_t diff;
      is >> diff;

      edl_group()->shift_media_end( reel, imgname, diff );

      ok = true;
   }
   else if ( cmd == N_("CurrentReel") )
   {
      std::string name;
      std::getline( is, name, '"' ); // skip first quote
      std::getline( is, name, '"' );

      mrv::Reel now = browser()->current_reel();
      if ( now && now->name == name )
	 r = now;
      else
	 r = browser()->reel( name.c_str() );
      if (!r) {
	 r = browser()->new_reel( name.c_str() );
      }
      ok = true;
   }
   else if ( cmd == N_("RemoveImage") )
   {
      size_t idx;
      is >> idx;

      r = browser()->current_reel();

      // Store image for insert later
      size_t e = r->images.size();
      int j = 0;
      for ( ; j != e; ++j )
      {
	 if ( j == idx )
	 {
	    m = r->images[j];
	    break;
	 }
      }

      browser()->remove( unsigned(idx) );
      edl_group()->redraw();

      ok = true;
   }
   else if ( cmd == N_("CloneImage") )
   {
      std::string imgname;
      std::getline( is, imgname, '"' ); // skip first quote
      std::getline( is, imgname, '"' );

      if ( r )
      {
	 size_t j;
	 size_t e = r->images.size();

         for ( j = 0; j != e; ++j )
         {
             mrv::media fg = r->images[j];
             if ( fg->image()->fileroot() == imgname )
                 break;
         }

         if ( j != e )
         {
             browser()->value( int(j) );
             browser()->clone_current();

             ok = true;
         }
      }
   }
   else if ( cmd == N_("InsertImage") )
   {
      int idx;
      is >> idx;

      std::string imgname;
      std::getline( is, imgname, '"' ); // skip first quote
      std::getline( is, imgname, '"' );

      r = browser()->current_reel();


      if ( r )
      {
	 int j;
	 size_t e = r->images.size();

	 if ( ! m )
	 {
 	    for ( j = 0; j != e; ++j )
	    {
	       mrv::media fg = r->images[j];
	       if ( fg->image()->fileroot() == imgname )
	       {
		  m = fg;
		  break;
	       }
	    }
	 }

	 if ( m )
	 {
	    
	    for ( j = 0; j != e; ++j )
	    {
	       if ( j == idx && m->image()->fileroot() == imgname )
	       {
		  browser()->insert( idx, m );
		  browser()->change_image( idx );
		  browser()->redraw();
		  edl_group()->refresh();
		  edl_group()->redraw();
		  ok = true;
		  break;
	       }
	    }
	   
	    if ( j == e && m->image()->fileroot() == imgname )
	    {
	       browser()->insert( unsigned(e), m );
	       browser()->change_image( unsigned(e) );
	       browser()->redraw();
	       edl_group()->refresh();
	       edl_group()->redraw();
	       ok = true;
	    }

	    m.reset();
	 }
      }

   }
   else if ( cmd == N_("Image") )
   {
      std::string imgname;
      std::getline( is, imgname, '"' ); // skip first quote
      std::getline( is, imgname, '"' );

      boost::int64_t start, end;
      is >> start;
      is >> end;

      bool found = false;
      if ( r )
      {
	 mrv::MediaList::iterator j = r->images.begin();
	 mrv::MediaList::iterator e = r->images.end();
	 for ( ; j != e; ++j )
	 {
	    mrv::media fg = *j;
	    if ( fg && fg->image()->fileroot() == imgname )
	    {
	       found = true;
	       m = fg;
	    }
	 }
      
	 if (!found)
	 {
	    LoadList files;
	    files.push_back( LoadInfo( imgname, start, end ) );
	   
	    browser()->load( files, false );
	 }
      }

      v->redraw();

      ok = true;
   }
   else if ( cmd == N_("CurrentImage") )
   {
      std::string imgname;
      std::getline( is, imgname, '"' ); // skip first quote
      std::getline( is, imgname, '"' );

      boost::int64_t first, last;
      is >> first;
      is >> last;
 
      if ( r )
      {
	 mrv::MediaList::iterator j = r->images.begin();
	 mrv::MediaList::iterator e = r->images.end();
	 int idx = 0;
	 bool found = false;
	 for ( ; j != e; ++j, ++idx )
	 {
	    if ( !(*j) ) continue;

	    CMedia* img = (*j)->image();
	    if ( img && img->fileroot() == imgname )
	    {
	       img->first_frame( first );
	       img->last_frame( last );
	       browser()->change_image( idx );
	       edl_group()->refresh();
	       edl_group()->redraw();
	       browser()->redraw();
	       m = *j;
	       found = true;
	       break;
	    }
	 }

	 if (! found )
	 {
	    LoadList files;
	    files.push_back( LoadInfo( imgname, first, last ) );

	    browser()->load( files, false );
	    edl_group()->refresh();
	    edl_group()->redraw();
	    browser()->redraw();
	 }
      }

      v->redraw();

      ok = true;
   }
   else if ( cmd == N_("FGReel") )
   {
       int idx;
       is >> idx;

       v->fg_reel( idx );
       ok = true;
   }
   else if ( cmd == N_("BGReel") )
   {
       int idx;
       is >> idx;

       v->bg_reel( idx );
       ok = true;
   }
   else if ( cmd == N_("CurrentBGImage") )
   {
      std::string imgname;
      std::getline( is, imgname, '"' ); // skip first quote
      std::getline( is, imgname, '"' );


      if ( imgname == "" )
      {
          v->background( mrv::media() );
      }
      else
      {

          boost::int64_t first, last;
          is >> first;
          is >> last;

          if ( r )
          {
              mrv::MediaList::iterator j = r->images.begin();
              mrv::MediaList::iterator e = r->images.end();
              int idx = 0;
              for ( ; j != e; ++j, ++idx )
              {
                  if ( !(*j) ) continue;
                  CMedia* img = (*j)->image();
                  if ( img && img->fileroot() == imgname )
                  {
                      img->first_frame( first );
                      img->last_frame( last );
                      v->background( (*j) );
                      ok = true;
                      break;
                  }
              }
          }
      }
      v->redraw();
   }
   else if ( cmd == N_("sync_image") )
   {
      std::string cmd;
      size_t num = browser()->number_of_reels();
      for (size_t i = 0; i < num; ++i )
      {
	 r = browser()->reel( unsigned(i) );
	 cmd = N_("Reel \"");
	 cmd += r->name;
	 cmd += "\"";
	 deliver( cmd );

	 mrv::MediaList::iterator j = r->images.begin();
	 mrv::MediaList::iterator e = r->images.end();
 	 for ( ; j != e; ++j )
	 {
	    cmd = N_("Image \"");
	    cmd += (*j)->image()->directory();
	    cmd += "/";
	    cmd += (*j)->image()->name();
	    cmd += "\" ";

	    char buf[128];
	    boost::int64_t start = (*j)->image()->first_frame();
	    boost::int64_t end   = (*j)->image()->last_frame();

	    sprintf( buf, N_("%") PRId64 N_(" %") PRId64,
		     start, end );
	    cmd += buf;

	    deliver( cmd );

	    boost::int64_t frame = (*j)->image()->frame();
	    sprintf( buf, N_("seek %") PRId64, frame );
	    deliver( buf );
	 }
      }

      if ( num == 0 ) {
          v->_clients = c;
          return true;
      }

      r = browser()->current_reel();
      if (r)
      {
	 cmd = N_("CurrentReel \"");
	 cmd += r->name;
	 cmd += "\"";
	 deliver( cmd );
      }
      if ( r->edl )
      {
	 cmd = N_("EDL 1");
	 deliver( cmd );
      }

      {
          mrv::media m = v->background();
          if ( m )
          {
              cmd = N_("CurrentBGImage \"");

              CMedia* img = m->image();
              cmd += img->fileroot();

              char buf[128];
              sprintf( buf, "\" %" PRId64 " %" PRId64, img->first_frame(),
                       img->last_frame() );
              cmd += buf;
              deliver( cmd );

              boost::int64_t frame = m->image()->frame();
              sprintf( buf, N_("seek %") PRId64, frame );
              deliver( buf );
          }
      }

      mrv::media m = v->foreground();
      if ( m )
      {
          //
          // handle all shapes in images in reels
          //
          size_t num = browser()->number_of_reels();
          for ( size_t r = 0; r < num; ++r )
          {
              mrv::Reel reel = browser()->reel_at( unsigned(r) );
              if ( reel->images.empty() ) continue;

              cmd = N_("CurrentReel \"");
              cmd += reel->name;
              cmd += "\"";
              deliver( cmd );

              size_t n = reel->images.size();
              for ( size_t i = 0; i < n; ++i )
              {
                  const mrv::media& fg = reel->images[i];
                  if (!fg) continue;

                  CMedia* img = fg->image();

                  const mrv::GLShapeList& shapes = img->shapes();
                  if ( shapes.empty() ) continue;


                  cmd = N_("CurrentImage \"");
                  cmd += img->fileroot();

                  char buf[128];
                  sprintf( buf, "\" %" PRId64 " %" PRId64, img->first_frame(),
                           img->last_frame() );
                  cmd += buf;
                  deliver( cmd );

                  mrv::GLShapeList::const_iterator k = shapes.begin();
                  mrv::GLShapeList::const_iterator e = shapes.end();
                  for ( ; k != e; ++k )
                  {
                      std::string s = (*k)->send();
                      deliver( s );
                  }
              }
          }

          CMedia* img = m->image();
          cmd = N_("CurrentImage \"");
          cmd += img->fileroot();

          char buf[128];
          sprintf( buf, "\" %" PRId64 " %" PRId64, img->first_frame(),
                   img->last_frame() );
          cmd += buf;
          deliver( cmd );

          boost::int64_t frame = img->frame() - img->first_frame() + 1;
          sprintf( buf, N_("seek %") PRId64, frame );
          deliver( buf );

      }

      char buf[256];
      sprintf(buf, N_("Zoom %g"), v->zoom() );
      deliver( buf );

      sprintf(buf, N_("Offset %g %g"), v->offset_x(), v->offset_y() );
      deliver( buf );

      sprintf(buf, N_("Channel %d"), v->channel() );
      deliver( buf );
      
      sprintf(buf, N_("UseLUT %d"), (int)v->use_lut() );
      deliver( buf );

      sprintf(buf, N_("SafeAreas %d"), (int)v->safe_areas() );
      deliver( buf );

      sprintf(buf, N_("Volume %g"), v->volume() );
      deliver( buf );
 
      sprintf(buf, N_("Gain %g"), v->gain() );
      deliver( buf );

      sprintf(buf, N_("Gamma %g"), v->gamma() );
      deliver( buf );

      sprintf(buf, N_("ShowPixelRatio %d"), 
	      (int)v->show_pixel_ratio() );
      deliver( buf );

      sprintf(buf, N_("Normalize %d"), (int)v->normalize() );
      deliver( buf );

      sprintf(buf, N_("Mask %g"), v->masking() );
      deliver( buf );

      sprintf( buf, N_("FPS %g"), v->fps() );
      deliver( buf );

      sprintf( buf, N_("Looping %d"), (int)v->looping() );
      deliver( buf );

      const mrv::Rectd& s = v->selection();
      if ( s.w() != 0 )
      {
          sprintf( buf, N_("Selection %g %g %g %g"), s.x(), s.y(), 
                   s.w(), s.h() );
          deliver( buf );
      }


      view()->redraw();

      ok = true;
   }
   else if ( cmd == N_("stop") )
   {
      boost::int64_t f;
      is >> f;
      v->stop();
      ok = true;
   }
   else if ( cmd == N_("playfwd") )
   {
      v->play_forwards();
      ok = true;
   }
   else if ( cmd == N_("playback") )
   {
      v->play_backwards();
      ok = true;
   }
   else if ( cmd == N_("seek") )
   {
      boost::int64_t f;
      is >> f;

      v->seek( f );

      ok = true;
   }
   else if ( cmd == N_("ColorInfoWindow") )
   {
       int x;
       is >> x;
       if ( x ) 
       {
           ui->uiColorArea->uiMain->show();
           v->update_color_info();
       }
       else
       {
           ui->uiColorArea->uiMain->hide();
       }

       ok = true;
   }
   else if ( cmd == N_("GL3dView") )
   {
       int x;
       is >> x;
       if ( x )
       {
           ui->uiGL3dView->uiMain->show();
       }
       else
       {
           ui->uiGL3dView->uiMain->hide();
       }
       ok = true;
   }
   else if ( cmd == N_("HistogramWindow") )
   {
       int x;
       is >> x;
       if ( x ) 
       {
           ui->uiHistogram->uiMain->show();
           v->update_color_info();
       }
       else
       {
           ui->uiHistogram->uiMain->hide();
       }
       ok = true;
   }
   else if ( cmd == N_("VectorscopeWindow") )
   {
       int x;
       is >> x;
       if ( x ) 
       {
           ui->uiVectorscope->uiMain->show();
           v->update_color_info();
       }
       else
       {
           ui->uiVectorscope->uiMain->hide();
       }
       ok = true;
   }

   v->_clients = c;

   return ok;
}


//------------------------ --------------------------

//----------------------------------------------------------------------


tcp_session::tcp_session(boost::asio::io_service& io_service,
			 mrv::ViewerUI* const v) :
input_deadline_(io_service),
non_empty_output_queue_(io_service),
output_deadline_(io_service),
Parser(io_service, v)
{
 
   // boost::asio::socket_base::debug option(true);
   // socket_.set_option( option );

   input_deadline_.expires_at(boost::posix_time::pos_infin);
   output_deadline_.expires_at(boost::posix_time::pos_infin);
   

   // The non_empty_output_queue_ deadline_timer is set to pos_infin 
   // whenever the output queue is empty. This ensures that the output 
   // actor stays asleep until a message is put into the queue.
   non_empty_output_queue_.expires_at(boost::posix_time::pos_infin);
}

tcp_session::~tcp_session()
{
}

tcp::socket& tcp_session::socket()
{
   return socket_;
}

// Called by the server object to initiate the four actors.
void tcp_session::start()
{
   connected = true;

   ui->uiView->_clients.push_back( this );

   start_read();
   
   //	std::cerr << "start1: " << socket_.native_handle() << std::endl;
   // input_deadline_.async_wait(
   //     boost::bind(&tcp_session::check_deadline,
   //     shared_from_this(), &input_deadline_));
   
   await_output();
   
   // std::cerr << "start2: " << socket_.native_handle() << std::endl;
   // output_deadline_.async_wait(
   //     boost::bind(&tcp_session::check_deadline,
   //     shared_from_this(), &output_deadline_));
   
}

bool tcp_session::stopped()
{
   return !socket_.is_open();
}

void tcp_session::deliver(std::string msg)
{
   output_queue_.push_back(msg + "\n");

   // Signal that the output queue contains messages. Modifying the expiry
   // will wake the output actor, if it is waiting on the timer.
   non_empty_output_queue_.expires_at(boost::posix_time::neg_infin);  
   //non_empty_output_queue_.expires_from_now(boost::posix_time::seconds(0));  
}


void tcp_session::stop()
{
   connected = false;
   boost::system::error_code ignored_ec;
   socket_.close(ignored_ec);
   input_deadline_.cancel();
   non_empty_output_queue_.cancel();
   output_deadline_.cancel();
}


void tcp_session::start_read()
{
   // Set a deadline for the read operation.
   
   // input_deadline_.expires_from_now(boost::posix_time::seconds(30));
   // input_deadline_.expires_at(boost::posix_time::pos_infin);
   
   // Start an asynchronous operation to read a newline-delimited message.
   boost::asio::async_read_until(socket(), input_buffer_, '\n',
				 boost::bind(&tcp_session::handle_read, 
					     shared_from_this(), 
					     boost::asio::placeholders::error));
}

void tcp_session::handle_read(const boost::system::error_code& ec)
{
   if (stopped())
      return;
   
   if (!ec)
   {
      // Extract the newline-delimited message from the buffer.
      std::string msg;
      std::istream is(&input_buffer_);
      
      try {
	 if ( std::getline(is, msg) )
	 {
	    if ( msg != "" && msg != "OK" && msg != "Not OK")
	    {
	       if ( parse( msg ) )
	       {
		  // send message to all clients
		  // We need to do this to update multiple clients.
		  // Note that the original client that sent the
		  // message will be skipped as it is IDed.

		  std::string id;
		  id = boost::lexical_cast<std::string>(socket().remote_endpoint() );
		  write( msg, id );  
		  deliver( "OK" );
	       }
	       else
	       {
		  std::string err = "Not OK for '";
		  err += msg;
		  err += "'";
		  deliver( err );
	       }
	    }
	 }
      } 
      catch ( std::ios_base::failure& e )
      {
         LOG_ERROR( "Failure in getline " << e.what() );
      }
      
      
      start_read();
   }
   else
   {
      LOG_ERROR( "ERROR handle_read " << ec );
      stop();
   }
}

void tcp_session::await_output()
{

   if (stopped())
      return;

   
   if (output_queue_.empty())
   {
      // There are no messages that are ready to be sent. The actor goes to
      // sleep by waiting on the non_empty_output_queue_ timer. When a new
      // message is added, the timer will be modified and the actor will
      // wake.
      

      non_empty_output_queue_.async_wait(
					 boost::bind(&tcp_session::await_output,
						     shared_from_this())
					 ); 
      non_empty_output_queue_.expires_at(boost::posix_time::pos_infin);
   }
   else
   {
      start_write();
   }
}

void tcp_session::start_write()
{
   // Start an asynchronous operation to send a message.
   boost::asio::async_write(socket(),
			    boost::asio::buffer(output_queue_.front()),
			    boost::bind(&tcp_session::handle_write, 
					shared_from_this(), _1));
   
}

void tcp_session::handle_write(const boost::system::error_code& ec)
{
   if (stopped())
      return;

   if (!ec)
   {
      output_queue_.pop_front();

      await_output();
   }
   else
   {
      stop();
   }
}

void tcp_session::check_deadline(deadline_timer* deadline)
{
   if (stopped())
      return;
   
   // Check whether the deadline has passed. We compare the deadline against
   // the current time since a new asynchronous operation may have moved the
   // deadline before this actor had a chance to run.
   if (deadline->expires_at() <= deadline_timer::traits_type::now())
   {
      // The deadline has passed. Stop the session. The other actors will
      // terminate as soon as possible.
      stop();
   }
   else
   {
      // Put the actor back to sleep.
      deadline->async_wait(
			   boost::bind(&tcp_session::check_deadline,
				       shared_from_this(), deadline));
   }
}


//----------------------------------------------------------------------

//
// This class manages socket timeouts by applying the concept of a deadline.
// Some asynchronous operations are given deadlines by which they must complete.
// Deadlines are enforced by two "actors" that persist for the lifetime of the
// session object, one for input and one for output:
//
//  +----------------+                     +----------------+
//  |                |                     |                |
//  | check_deadline |<---+                | check_deadline |<---+
//  |                |    | async_wait()   |                |    | async_wait()
//  +----------------+    |  on input      +----------------+    |  on output
//              |         |  deadline                  |         |  deadline
//              +---------+                            +---------+
//
// If either deadline actor determines that the corresponding deadline has
// expired, the socket is closed and any outstanding operations are cancelled.
//
// The input actor reads messages from the socket, where messages are delimited
// by the newline character:
//
//  +------------+
//  |            |
//  | start_read |<---+
//  |            |    |
//  +------------+    |
//          |         |
//  async_- |    +-------------+
//   read_- |    |             |
//  until() +--->| handle_read |
//               |             |
//               +-------------+
//
// The deadline for receiving a complete message is 30 seconds. If a non-empty
// message is received, it is delivered to all subscribers. If a heartbeat (a
// message that consists of a single newline character) is received, a heartbeat
// is enqueued for the client, provided there are no other messages waiting to
// be sent.
//
// The output actor is responsible for sending messages to the client:
//
//  +--------------+
//  |              |<---------------------+
//  | await_output |                      |
//  |              |<---+                 |
//  +--------------+    |                 |
//      |      |        | async_wait()    |
//      |      +--------+                 |
//      V                                 |
//  +-------------+               +--------------+
//  |             | async_write() |              |
//  | start_write |-------------->| handle_write |
//  |             |               |              |
//  +-------------+               +--------------+
//
// The output actor first waits for an output message to be enqueued. It does
// this by using a deadline_timer as an asynchronous condition variable. The
// deadline_timer will be signalled whenever the output queue is non-empty.
//
// Once a message is available, it is sent to the client. The deadline for
// sending a complete message is 30 seconds. After the message is successfully
// sent, the output actor again waits for the output queue to become non-empty.
//


server::server(boost::asio::io_service& io_service,
	       const tcp::endpoint& endpoint,
	       mrv::ViewerUI* v)
: io_service_(io_service),
  acceptor_(io_service),
  ui_( v )
{
   
   acceptor_.open(endpoint.protocol());
   acceptor_.set_option(tcp::acceptor::reuse_address(true));
   acceptor_.bind(endpoint);
   acceptor_.listen();

   start_accept();
}

server::~server()
{
}

void server::start_accept()
{
   //    tcp_session_ptr new_session(new tcp_session(io_service_, ui_));
   tcp_session_ptr new_session(
			       boost::make_shared<tcp_session>(
							       boost::ref(
									  io_service_
									  ),
							       boost::ref(ui_)
							       )
			       );
   

   acceptor_.async_accept(new_session->socket(),
			  boost::bind(&server::handle_accept, this, 
				      new_session, _1));
}

void server::handle_accept(tcp_session_ptr session,
			   const boost::system::error_code& ec)
{
   if (!ec)
   {
      session->start();
   }
   
   start_accept();
}

ConnectionUI* ViewerUI::uiConnection = NULL;

void server::create(mrv::ViewerUI* ui)
{
   unsigned short port = (unsigned short) ui->uiConnection->uiServerPort->value();
   ServerData* data = new ServerData;
   data->port = port;
   data->ui = ui;

   boost::thread t( boost::bind( mrv::server_thread, 
				 data ) );
}

void server::remove( mrv::ViewerUI* ui )
{
   mrv::ImageView* v = ui->uiView;

   ParserList::iterator i = v->_clients.begin();
   ParserList::iterator e = v->_clients.end();

   for ( ; i != e; ++i )
   {
      (*i)->stop();
   }

   ui->uiConnection->uiCreate->label("Create");
   ui->uiConnection->uiClientGroup->activate();

   v->_clients.clear();
   v->_server.reset();
}


//----------------------------------------------------------------------

void server_thread( const ServerData* s )
{
   try
   {

      boost::asio::io_service io_service;

      tcp::endpoint listen_endpoint(tcp::v4(), s->port);

      s->ui->uiView->_server = boost::make_shared< server >( boost::ref(io_service),
							     listen_endpoint,
							     s->ui);

      s->ui->uiConnection->uiCreate->label("Disconnect");
      s->ui->uiConnection->uiClientGroup->deactivate();

      LOG_CONN( "Created server at port " << s->port );

      delete s;


      io_service.run();
   }
   catch (std::exception& e)
   {
      LOG_ERROR( "Exception: " << e.what() );
   }
}

} // namespace mrv


