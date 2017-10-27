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
#include <ImfFloatAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>

#include <OpenImageIO/imageio.h>
OIIO_NAMESPACE_USING;


#include "core/mrvImageOpts.h"
#include "core/Sequence.h"
#include "core/mrvColorProfile.h"
#include "core/mrvString.h"
#include "core/oiioImage.h"
#include "core/mrvI8N.h"
#include "gui/mrvIO.h"


namespace
{
  const char* kModule = "oiio";

}


namespace mrv {


  oiioImage::oiioImage() :
    CMedia(),
    _format( NULL ),
    _level( -1 ),
    _mipmaps( 0 )
  {
  }

  oiioImage::~oiioImage()
  {
      free( _format );
  }


  /*! LibOiio does not allow testing a block of data,
    but allows testing a file.
  */

  bool oiioImage::test(const char* file)
  {
      if ( file == NULL ) return false;

      ImageInput* in = ImageInput::open( file );
      if(!in)
      {
          return false;
      }
      in->close ();
      ImageInput::destroy (in);
      return true;
  }

  bool oiioImage::initialize()
  {
    return true;
  }

  bool oiioImage::release()
  {
    return true;
  }


  bool oiioImage::fetch( const boost::int64_t frame )
  {

      _layers.clear();
      _num_channels = 0;


      const char* const file = filename();
      ImageInput* in = ImageInput::open( file );
      if (!in)
      {
        std::string err = geterror();
        IMG_ERROR( (err.length() ? err :
                    Strutil::format("Could not open \"%s\"", file)) );
        return false;
      }

      // This is wrong in the demo example.  We need a const free ImageSpec.
      const ImageSpec &s = in->spec();
      ImageSpec& spec = const_cast< ImageSpec& >( s );

      std::string fmt = in->format_name();
      fmt = "OIIO (" + fmt + ")";
      _format = strdup( fmt.c_str() );

      if ( _level < 0 )
      {
          while ( in->seek_subimage( 0, _mipmaps, spec ) )
          {
              ++_mipmaps;
          }

          if ( _mipmaps > 1 )
          {
              Imf::IntAttribute attr( _mipmaps );
              _attrs.insert( std::make_pair( _("Mipmap Levels"),
                                             attr.copy() ) );
          }
          _level = 0;
      }

      if ( ! in->seek_subimage( 0, _level, spec ) )
      {
          IMG_ERROR( _("Invalid mipmap level") );
          return false;
      }

      unsigned dw = spec.width;
      unsigned dh = spec.height;
      int channels = spec.nchannels;
      TypeDesc format = spec.format;
      
      {
          _compression = spec.get_string_attribute( "compression",
                                                    "Zip" );
      }
      for (size_t i = 0; i < spec.extra_attribs.size(); ++i) {
          const ParamValue &p (spec.extra_attribs[i]);
          if ( p.name() == "compression" ) continue;
          if (p.type() == TypeString)
          {
              Imf::StringAttribute attr( *(const char **)p.data() );
              _attrs.insert( std::make_pair( p.name().c_str(), attr.copy() ) );
          }
          else if (p.type() == TypeFloat)
          {
              Imf::FloatAttribute attr( *(const float*)p.data() );
              _attrs.insert( std::make_pair( p.name().c_str(), attr.copy() ) );
          }
          else if (p.type() == TypeInt)
          {
              Imf::IntAttribute attr( *(const int*)p.data() );
              _attrs.insert( std::make_pair( p.name().c_str(), attr.copy() ) );
          }
          else if (p.type() == TypeDesc::UINT)
          {
              Imf::IntAttribute attr( *(const unsigned int*)p.data() );
              _attrs.insert( std::make_pair( p.name().c_str(), attr.copy() ) );
          }
          else if (p.type() == TypeMatrix)
          {
              const float *f = (const float *)p.data();
              Imath::M44f m(f[0], f[1], f[2], f[3],
                            f[4], f[5], f[6], f[7],
                            f[8], f[9], f[10], f[11],
                            f[12], f[13], f[14], f[15]);
              Imf::M44fAttribute attr( m );
              _attrs.insert( std::make_pair( p.name().c_str(), attr.copy() ) );
          }
      }

      if ( channels >= 3 )
      {
          rgb_layers();
          lumma_layers();
      }

      if ( channels >= 4 )
      {
          alpha_layers();
      }
      
      image_size( dw, dh );

      image_type::PixelType pixel_type;

      if ( format == TypeDesc::UINT8 )
      {
          pixel_type = image_type::kByte;
      }
      else if ( format == TypeDesc::HALF )
      {
          pixel_type = image_type::kHalf;
      }
      else if ( format == TypeDesc::FLOAT )
      {
          pixel_type = image_type::kFloat;
      }
      else if ( format == TypeDesc::UINT16 || format == TypeDesc::INT16 )
      {
          pixel_type = image_type::kInt;
      }
      else if ( format == TypeDesc::UINT || format == TypeDesc::INT )
      {
          pixel_type = image_type::kInt;
      }
      else
      {
          IMG_ERROR( _("Unknown pixel type" ) );
          return false;
      }

      image_type::Format type;

      switch( channels )
      {
          case 1:
              type = image_type::kLumma; break;
          case 3:
              type = image_type::kRGB;   break;
          case 4:
              type = image_type::kRGBA;  break;
          default:
              IMG_ERROR( _("Unknown number of channels") );
              return false;
      }

      allocate_pixels( frame, channels, type, pixel_type, dw, dh );

      {
          SCOPED_LOCK( _mutex );

          Pixel* pixels = (Pixel*)_hires->data().get();
          in->read_image (format, &pixels[0]);
      }

      in->close();
      ImageInput::destroy(in);

      return true;
  }




} // namespace mrv
