/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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

    char* orig = strdup( other->filename() );
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

    _fileroot = strdup( other->fileroot() );

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
      _hires.reset( new mrv::image_type() );
      *_hires = *(img->left());
    }


    { // Copy attributes
        for ( const auto& i : other->attrs_frames() )
        {
            int64_t frame = i.first;
            _attrs.insert( std::make_pair( frame, Attributes() ) );
            for ( const auto& j: i.second )
            {
                _attrs[frame][j.first] = j.second->copy();
            }
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


    std::string ocio = other->ocio_input_color_space();
    ocio_input_color_space( ocio );

    _label = NULL;
    const char* lbl = other->label();
    if ( lbl )  _label = strdup( lbl );

    free( orig );
    _disk_space = 0;

    // thumbnail_create();
    // _thumbnail_frozen = true;
  }

    bool clonedImage::fetch( int64_t frame )
    {
        return true;
    }

} // namespace mrv
