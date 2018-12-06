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

#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imageio.h>
OIIO_NAMESPACE_USING;

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



#include "core/mrvColorOps.h"
#include "core/mrvMath.h"
#include "core/mrvImageOpts.h"
#include "core/Sequence.h"
#include "core/mrvColorProfile.h"
#include "core/mrvString.h"
#include "core/oiioImage.h"
#include "core/mrvI8N.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvIO.h"
#include "OIIOOptionsUI.h"
#include "mrViewer.h"


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

      std::string f = file;
      std::transform( f.begin(), f.end(), f.begin(), (int(*)(int)) tolower );


      if ( f.rfind(".dpx") != std::string::npos )
      {
          if ( CMedia::load_library != CMedia::kOIIOLibrary )
          {
              return false;
          }
      }

      if ( f.rfind(".psd") != std::string::npos )
          return false;

      std::unique_ptr<ImageInput> in = ImageInput::open( file );
      if(!in)
      {
          return false;
      }
      in->close ();
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
      std::string file = sequence_filename( frame );
      std::unique_ptr<ImageInput> in = ImageInput::open( file.c_str() );
      if (!in)
      {
        std::string err = geterror();
        IMG_ERROR( (err.length() ? err :
                    Strutil::format("Could not open \"%s\"", file.c_str())) );
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
              // Process timecode
              if ( p.name().rfind( N_("TimeCode") ) != std::string::npos )
              {
                  Imf::TimeCode t = CMedia::str2timecode( *(const char **)
                                                          p.data() );
                  process_timecode(t); // turn timecode into a frame offset

                  // Store attribute in image
                  Imf::TimeCodeAttribute attr( t );
                  _attrs.insert( std::make_pair( p.name().c_str(),
                                                 attr.copy() ) );
                  continue;
              }
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

      if ( _num_channels == 0 )
      {
          if ( channels == 1 )
          {
              lumma_layers();
          }
          else if ( channels >= 2 )
          {
              rgb_layers();
              lumma_layers();
          }

          if ( channels >= 4 )
          {
              alpha_layers();
          }

          if ( channels >= 5 )
          {
              _layers.push_back( _("Z") );
              ++_num_channels;
          }

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
          pixel_type = image_type::kShort;
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
      case 2:
      case 3:
          type = image_type::kRGB;   break;
      case 4:
          type = image_type::kRGBA;  break;
      default:
          IMG_ERROR( _("Unknown number of channels") );
          return false;
      }

      if ( allocate_pixels( frame, channels, type, pixel_type, dw, dh ) )
      {
          SCOPED_LOCK( _mutex );

          try
          {
              Pixel* pixels = (Pixel*)_hires->data().get();
              in->read_image (format, &pixels[0]);
          }
          catch( const std::runtime_error& e )
          {
              LOG_ERROR( e.what() );
              return false;
          }
      }
      else
      {
          LOG_ERROR( _("Not enough memory for image") );
          in->close();
          return false;
      }

      in->close();

      return true;
  }


bool oiioImage::save( const char* path, const CMedia* img,
                      const OIIOOpts* opts )
{
    std::unique_ptr<ImageOutput> out = ImageOutput::create( path );
    mrv::Recti dpw = img->display_window();
    mrv::Recti daw = img->data_window();
    unsigned dw = daw.w();
    unsigned dh = daw.h();
    std::string ext = path;
    size_t pos = ext.rfind( '.' );
    if ( pos != std::string::npos )
    {
        ext = ext.substr( pos, ext.size() );
    }
    std::transform( ext.begin(), ext.end(), ext.begin(),
                    (int(*)(int)) tolower );

    TypeDesc::BASETYPE type;
    mrv::image_type_ptr pic = img->hires();
    image_type::Format format = pic->format();

    bool must_convert = false;

    image_type::PixelType pt = pic->pixel_type();

    image_type::PixelType st = opts->pixel_type();

    // Constrain some pixel types to the maximum supported by the format
    std::string f = path;
    image_type::PixelType maxPixelType = image_type::kByte;
    if ( ext == ".iff" )
    {
        maxPixelType = image_type::kShort;
    }
    else
    {
        maxPixelType = image_type::kFloat;
    }

    if ( pt < maxPixelType ) maxPixelType = pt;

    switch( st )
    {
        case CharPixel:
            pt = image_type::kByte;
            break;
        case ShortPixel:
            pt = image_type::kShort;
            break;
        case LongPixel:
            pt = image_type::kInt;
            break;
        case DoublePixel:
        case FloatPixel:
            pt = image_type::kFloat;
            break;
    }

    if ( pt > maxPixelType ) pt = maxPixelType;


    switch( pt )
    {
        case image_type::kByte:
            type = TypeDesc::UINT8;
            break;
        case image_type::kShort:
            type = TypeDesc::USHORT;
            break;
        case image_type::kInt:
            type = TypeDesc::UINT;
            break;
        case image_type::kHalf:
            type = TypeDesc::HALF;
            break;
        case image_type::kFloat:
            type = TypeDesc::FLOAT;
            break;
        default:
            LOG_ERROR( img->name() << _(": Unknown pixel type") );
            return false;
    }

    unsigned short channels = pic->channels();

    format = image_type::kLumma;
    if ( channels >= 2 ) format = image_type::kRGB;
    if ( channels >= 4 ) format = image_type::kRGBA;

    if ( pic->pixel_type() > maxPixelType || img->gamma() != 1.0f )
    {
        must_convert = true;
    }
    if ( channels >= 4 && (ext == ".rgbe" || ext == ".hdr" || ext == ".hdri" ))
    {
        LOG_ERROR( _("Too many channels in image.  Saving RGB only") );
        channels = 3; format = image_type::kRGB;
    }

    ImageSpec spec( dw, dh, channels, type );
    spec.full_x = 0;
    spec.full_y = 0;
    spec.full_width = dpw.w();
    spec.full_height = dpw.h();
    spec.x = daw.x();
    spec.y = daw.y();
    spec.width = dw;
    spec.height = dh;


    if ( opts->mipmap() )
    {
        ImageBuf ir( img->name(), spec, (void*)pic->data().get() );
        ImageBufAlgo::MakeTextureMode mode = ImageBufAlgo::MakeTxTexture;
        ImageSpec config;
        config.attribute ( N_("maketx:highlightcomp"), 1);
        config.attribute ( N_("maketx:filtername"), N_("lanczos3") );
        config.attribute ( N_("maketx:opaquedetect"), 1);
        if ( must_convert && Preferences::use_ocio &&
             Preferences::uiMain->uiView->use_lut() )
        {
            config.attribute ( N_("maketx:incolorspace"),
                              img->ocio_input_color_space() );
            const std::string& display = mrv::Preferences::OCIO_Display;
            const std::string& view = mrv::Preferences::OCIO_View;
            config.attribute ( N_("maketx:outcolorspace"), view.c_str() );
        }

        bool ok = ImageBufAlgo::make_texture( mode, ir, path, config, NULL );
        if ( !ok )
        {
            LOG_ERROR( _("ImageBufAlgo::make_texture failed.") );
            return false;
        }
    }
    else
    {

        try
        {
            out->open( path, spec );

            unsigned short pixel_size = pic->pixel_size();
            if ( !must_convert )
            {
                if ( pic->channels() > channels )
                {
                    mrv::image_type_ptr ptr =
                    image_type_ptr( new image_type(
                                                   img->frame(),
                                                   dw, dh, 3,
                                                   image_type::kRGB,
                                                   pic->pixel_type() ) );
                    copy_image( ptr, pic );
                    pic = ptr;
                }

                mrv::aligned16_uint8_t* p = pic->data().get();
                unsigned mult = dw * pic->channels() * pixel_size;
                for ( int y = spec.y; y < spec.y + daw.h(); ++y )
                {
                    void* line = &p[(y-spec.y) * mult ];
                    out->write_scanline( y, 0, type, line );
                }
            }
            else
            {
                prepare_image( pic, img, format, pt );

                mrv::aligned16_uint8_t* p = pic->data().get();
                int yh = spec.y + dh;
                unsigned mult = dw * pic->channels() * pixel_size;
                for ( int y = spec.y; y < yh; ++y )
                {
                    mrv::aligned16_uint8_t* line = p;
                    line += (y-spec.y) * mult;
                    out->write_scanline( y, 0, type, line );
                }
            }
        }
        catch( const std::exception& e )
        {
            LOG_ERROR( e.what() );
            out->close();
            return false;
        }

        out->close();
    }

    return true;
}

} // namespace mrv
