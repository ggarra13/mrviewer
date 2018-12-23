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
 * @file   rawImage.cpp
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 *
 * @brief  A simple wrapper class to read all of ImageMagick's image formats
 *         using the raw interface.
 *
 */

#include <iostream>
using namespace std;

#include <cstdio>
#define __STDC_LIMIT_MACROS
#include <inttypes.h>
#include <cmath>
#ifdef _WIN32
# include <float.h>
# define isfinite(x) _finite(x)
#endif

#include <algorithm>

#include <ImfStandardAttributes.h>
#include <ImfIntAttribute.h>
#include <ImfFloatAttribute.h>
#include <ImfVecAttribute.h>
#include <ImfMatrixAttribute.h>

#include "core/mrvImageOpts.h"
#include "core/Sequence.h"
#include "core/mrvColorProfile.h"
#include "core/mrvString.h"
#include "core/rawImage.h"
#include "core/mrvI8N.h"
#include "gui/mrvIO.h"


#define HANDLE_ERRORS(ret,verbose) do {                         \
    if(ret)                                                     \
        {                                                       \
            LOG_ERROR( file << ": " << libraw_strerror(ret));     \
            if(LIBRAW_FATAL_ERROR(ret))                         \
                {                                               \
                    libraw_close(iprc);                         \
                    return false;                               \
                }                                               \
        }                                                       \
    } while(0)

namespace
{
  const char* kModule = "raw";

const char *kRawExtensions[] = {
"bay", "bmq", "cr2", "crw", "cs1", "dc2", "dcr", "dng",
"erf", "fff", "k25", "kdc", "mdc", "mos", "mrw",
"nef", "orf", "pef", "pxn", "raf", "raw", "rdc", "sr2",
"srf", "x3f", "arw", "3fr", "cine", "ia", "kc2", "mef",
"nEF", "nrw", "qtk", "rw2", "sti", "rwl", "srw", "drf", "dsc",
"ptx", "cap", "iiq", "rwz", NULL
};

}


namespace mrv {


  rawImage::rawImage() :
    CMedia(),
    _format( NULL ),
    iprc( NULL )
  {
  }

  rawImage::~rawImage()
  {
      free( _format );
      if ( iprc ) libraw_close( iprc );
      iprc = NULL;
  }


  /*! LibRaw does not allow testing a block of data,
    but allows testing a file.
  */

int verbose=1,use_camera_wb=1,use_auto_wb=1,tiff_mode=0;
  bool rawImage::test(const char* file)
  {
      if ( file == NULL ) return false;
      std::string f = file;
      std::transform( f.begin(), f.end(), f.begin(), (int(*)(int)) tolower );

      bool found = false;
      const char** i = kRawExtensions;
      for (  ; *i; ++i )
      {
          const char* ext = *i;
          if ( f.rfind( ext ) == f.size() - strlen( ext ) )
          {
              found = true; break;
          }
      }

      if ( !found ) return false;

      libraw_data_t *in = libraw_init(0);
      if(!in)
      {
          LOG_ERROR( _("Cannot create libraw handle") );
          return false;
      }

      in->params.half_size = 4; /* dcraw -h */
      in->params.use_camera_wb = 0;
      in->params.use_auto_wb = 0;

      int ret = libraw_open_file(in, file);

      if(ret)
      {
          LOG_ERROR( file << ": " << libraw_strerror(ret));
          if(LIBRAW_FATAL_ERROR(ret))
          {
              libraw_close(in);
          }
          return false;
      }

      libraw_close(in);
      return true;
  }

  bool rawImage::initialize()
  {
    return true;
  }

  bool rawImage::release()
  {
    return true;
  }

  bool rawImage::fetch( const boost::int64_t frame )
  {
      if ( !iprc )
      {
          iprc = libraw_init(0);
          if(!iprc)
          {
              LOG_ERROR( _("Cannot create libraw handle") );
              return false;
          }
      }

      iprc->params.half_size = 0; /* no dcraw -h */
      iprc->params.use_camera_wb = use_camera_wb;
      iprc->params.use_auto_wb = use_auto_wb;

      _layers.clear();
      _num_channels = 0;

      const char* const file = filename();
      int ret = libraw_open_file(iprc, file);
      HANDLE_ERRORS( ret, true );

      char buf[256];
      sprintf( buf, "%s/%s",iprc->idata.make, iprc->idata.model );
      _format = strdup( buf );

      unsigned dw = 0, dh = 0;

#if 1
      if ( is_thumbnail() )
      {
          //@todo: extract thumb image if present
          ret = libraw_unpack_thumb(iprc);
          HANDLE_ERRORS( ret, false );

          LibRaw_thumbnail_formats tformat = iprc->thumbnail.tformat;
          dw = iprc->thumbnail.twidth;
          dh = iprc->thumbnail.theight;

          if ( tformat == LIBRAW_THUMBNAIL_BITMAP )
          {
              rgb_layers();
              lumma_layers();
              alpha_layers();
              image_size( dw, dh );
              const image_type::PixelType pixel_type = image_type::kByte;
              allocate_pixels( frame, 4, image_type::kRGBA, pixel_type,
                               dw, dh );

              {
                  SCOPED_LOCK( _mutex );

                  Pixel* pixels = (Pixel*)_hires->data().get();
                  memcpy( pixels, iprc->thumbnail.thumb, dw*dh*4 );
              }
          }
          else if ( tformat == LIBRAW_THUMBNAIL_JPEG )
          {
              // @todo: decompress in memory the jpeg thumbnail
              is_thumbnail( false );
              iprc->params.half_size = 1; /* dcraw -h */
          }
          else
          {
              is_thumbnail( false );
              iprc->params.half_size = 1; /* dcraw -h */
          }
      }
#endif

      {
          ret = libraw_unpack(iprc);
          HANDLE_ERRORS( ret, true );

          ret = libraw_dcraw_process(iprc);
          HANDLE_ERRORS( ret, true );

          if ( !iprc->image )
          {
              IMG_ERROR( _("No image was decoded") );
              return false;
          }

          const libraw_imgother_t& o = iprc->other;

          {
              Imf::FloatAttribute attr( o.iso_speed );
              _attrs.insert( std::make_pair( "Exif:ISOSpeedRatings",
                                             attr.copy() ) );
          }
          {
              Imf::FloatAttribute attr( o.shutter );
              _attrs.insert( std::make_pair( "ExposureTime",
                                             attr.copy() ) );
          }
          {
              Imf::FloatAttribute attr( -log2f(o.shutter) );
              _attrs.insert( std::make_pair( "Exif:ShutterSpeedValue",
                                             attr.copy() ) );
          }
          {
              //   Imf::FloatAttribute attr( o.aperture );
              Imf::RationalAttribute attr( Imf::Rational( o.aperture*100,
                                                          100 ) );
              _attrs.insert( std::make_pair( "F Number", attr.copy() ) );
          }
          {
              Imf::FloatAttribute attr( 2.0f * log2f(o.aperture) );
              _attrs.insert( std::make_pair( "Exif:ApertureValue",
                                             attr.copy() ) );
          }
          {
              Imf::FloatAttribute attr( o.focal_len );
              _attrs.insert( std::make_pair( "Exif:FocalLength",
                                             attr.copy() ) );
          }
          {
              struct tm * m_tm = localtime(&o.timestamp);
              char datetime[20];
              strftime (datetime, 20, "%Y-%m-%d %H:%M:%S", m_tm);
              Imf::StringAttribute attr( datetime );
              _attrs.insert( std::make_pair( "DateTime", attr.copy() ) );
          }
          if ( o.desc[0] )
          {
              Imf::StringAttribute attr( o.desc );
              _attrs.insert( std::make_pair( "ImageDescription",
                                             attr.copy() ) );
          }

          if ( o.artist[0] )
          {
              Imf::StringAttribute attr( o.artist );
              _attrs.insert( std::make_pair( "Artist", attr.copy() ) );
          }
          {
              const libraw_colordata_t &color (iprc->color);
              Imf::FloatAttribute attr( color.flash_used );
              _attrs.insert( std::make_pair( "Exif:Flash",
                                             attr.copy() ) );

              if ( color.model2[0] )
              {
                  Imf::StringAttribute attr( color.model2 );
                  _attrs.insert( std::make_pair( "Software", attr.copy() ) );
              }
          }

          dw = libraw_get_iwidth( iprc );
          dh = libraw_get_iheight( iprc );


          rgb_layers();
          lumma_layers();
          alpha_layers();
          pixel_ratio( iprc->sizes.pixel_aspect );
          image_size( dw, dh );


          const image_type::PixelType pixel_type = image_type::kShort;

          image_type::Format type = image_type::kRGBA;
          allocate_pixels( frame, _num_channels, type, pixel_type, dw, dh );

          {
              SCOPED_LOCK( _mutex );

              Pixel* pixels = (Pixel*)_hires->data().get();
              memcpy( pixels, iprc->image, dw*dh*4*sizeof(short) );
          }
      }

      libraw_recycle(iprc);

      return true;
  }




} // namespace mrv
