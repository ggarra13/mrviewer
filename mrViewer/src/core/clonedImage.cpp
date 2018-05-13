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
 * @file   clonedImage.cpp
 * @author gga
 * @date   Thu Nov 02 05:41:09 2006
 *
 * @brief  Cloned image class
 *
 *
 */

#include <iostream>
using namespace std;

#include <ctime>
#include <cstdio>

#include "mrvThread.h"
#include "clonedImage.h"


namespace mrv {

  clonedImage::clonedImage( const CMedia* other ) :
    CMedia()
  {
      _frame_start = other->start_frame();
      _frame_end   = other->end_frame();
      _frameStart = other->first_frame();
      _frameEnd   = other->last_frame();
      
      char* file = strdup( other->filename() );
      char* orig = file;
      for ( char* s = orig; *s != 0; ++s )
      {
          if ( *s == '\t' )
          {
              orig = s + 1; break;
          }
      }

    char now[128];
    _ctime = time(NULL);
    strftime( now, 127, "%H:%M:%S - %d %b", localtime(&_ctime) );

    char name[256];
    sprintf( name, "%s\t%s", now, orig );
    filename( name );

    _filename = strdup( other->filename() );

    unsigned int ws = other->width();
    unsigned int wh = other->height();
    _gamma = other->gamma();

    image_size( ws, wh );
    _pixel_ratio = other->pixel_ratio();

    _num_channels = 1;
    const char* channel = other->channel();
    if ( channel )
      {
        _layers.push_back( channel );
        ++_num_channels;
        _channel = strdup( channel );
      }
    else
      {
        default_layers();
        _num_channels = 4;
      }

    {
      CMedia* img = const_cast< CMedia* >( other );
      CMedia::Mutex& m = img->video_mutex();
      SCOPED_LOCK(m);
      _hires.reset( new mrv::image_type( other->frame(),
					 other->width(),
					 other->height(),
					 other->hires()->channels(),
					 other->hires()->format(),
					 other->hires()->pixel_type() ) );
      copy_image( _hires, img->hires() );
    }


    //   setsize( -1, -1 );

    { // Copy attributes
        const CMedia::Attributes& attrs = other->attributes();
        CMedia::Attributes::const_iterator i = attrs.begin();
        CMedia::Attributes::const_iterator e = attrs.end();
        for ( ; i != e; ++i )
        {
            _attrs.insert( std::make_pair( i->first, i->second->copy() ) );
        }
    }

    const char* profile = other->icc_profile();
    if ( profile )  icc_profile( profile );


    size_t num = other->number_of_lmts();
    size_t i = 0;
    for ( ; i < num; ++i )
    {
        _look_mod_transform.push_back( strdup(
                                       other->look_mod_transform( i ) ) );
    }

    const char* transform = other->idt_transform();
    if ( transform )  idt_transform( strdup(transform) );

    transform = other->rendering_transform();
    if ( transform )  rendering_transform( strdup(transform) );

    const char* lbl = other->label();
    if ( lbl )  _label = strdup( lbl );

    free( file );
    _disk_space = 0;

    // thumbnail_create();
    // _thumbnail_frozen = true;
  }


} // namespace mrv
