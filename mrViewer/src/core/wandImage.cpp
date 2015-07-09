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
 * @file   wandImage.cpp
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 * 
 * @brief  A simple wrapper class to read all of ImageMagick's image formats
 *         using the wand interface.
 * 
 */

#include <iostream>
using namespace std;

#include <cstdio>
#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#include <inttypes.h>
#include <math.h>
#ifdef _WIN32
# include <float.h>
# define isfinite(x) _finite(x)
#endif

#include <algorithm>
#include <wand/MagickWand.h>

#include "core/mrvImageOpts.h"
#include "core/Sequence.h"
#include "core/mrvColorProfile.h"
#include "core/mrvString.h"
#include "core/wandImage.h"
#include "core/exrImage.h"
#include "core/aviImage.h"
#include "core/mrvThread.h"
#include "core/mrvI8N.h"
#include "gui/mrvIO.h"


#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )

#define ThrowWandException( wand ) \
  { \
    ExceptionType severity; \
   \
    char* description=MagickGetException(wand,&severity); \
    IMG_ERROR( description ); \
    description=(char *) MagickRelinquishMemory(description); \
    return false; \
  }

#define ThrowWandExceptionPing( wand ) \
  { \
    ExceptionType severity; \
   \
    char* description=MagickGetException(wand,&severity); \
    LOG_ERROR( file << ": " << severity << " " << description ); \
    description=(char *) MagickRelinquishMemory(description); \
    return false; \
  }

namespace 
{
  const char* kModule = "wand";
}


namespace mrv {


  wandImage::wandImage() :
    CMedia(),
    _format( NULL )
  {
  }

  wandImage::~wandImage()
  {
      free( _format );
  }


  /*! ImageMagick does not allow testing a block of data,
    but allows testing a file or FILE*.
  */
  bool wandImage::test(const char* file)
  {
      std::string f = file;
      size_t pos = f.rfind( N_(".") );
      if ( pos != std::string::npos )
      {
          std::string ext = f.substr( pos+1, f.size() );
          if ( ext == "PDF" || ext == "pdf" )
              return false;
      }

    MagickWandGenesis();
    MagickBooleanType status = MagickFalse;

    MagickWand* wand = NewMagickWand();

    status = MagickPingImage( wand, file );

    DestroyMagickWand(wand);


    if (status == MagickFalse )
    {
      return false;
    }

    return true;
  }





  bool wandImage::initialize()
  {
    return true;
  }

  bool wandImage::release()
  {
    return true;
  }


  bool wandImage::fetch( const boost::int64_t frame ) 
  {

     MagickBooleanType status;


     /*
       Read an image.
     */
     MagickWand* wand = NewMagickWand(); 
     status = MagickReadImage( wand, sequence_filename(frame).c_str() );
     if (status == MagickFalse)
	ThrowWandException( wand );

     _format = strdup( MagickGetImageFormat( wand ) );

     _layers.clear();
     _num_channels = 0;

     rgb_layers();
     lumma_layers();

     size_t numLayers = MagickGetNumberImages( wand );
     if ( numLayers > 1 )
     {
	const char* channelName = channel();

	size_t index = 0;
	for ( size_t i = 1; i < numLayers; ++i )
	{

	   char layername[256];

	   MagickSetIteratorIndex( wand, i );
	   const char* label = MagickGetImageProperty( wand, "label" );
	   if ( label == NULL )
	   {
	      sprintf( layername, _("Layer %" PRId64 ), i+1 );
	   }
	   else
	   {
	      strcpy( layername, label );
	   }

	   _layers.push_back( layername );
	   ++_num_channels;

	   if ( channelName && strcmp( layername, channelName ) == 0 )
	   {
	      index = i;
	   } 
	}

	MagickSetIteratorIndex( wand, index );
     }


     bool has_alpha = false;
     MagickBooleanType alpha = MagickGetImageAlphaChannel( wand );
     if ( alpha == MagickTrue )
     {
	has_alpha = true;
	alpha_layers();
     }

     /*
       Copy pixels from magick to class
     */
     size_t dw = MagickGetImageWidth( wand );
     size_t dh = MagickGetImageHeight( wand );


     // This depth call in ImageMagick is pretty slow.  Seems to be scanning all
     // pixels, for some weird reason.

#if 1
     Image* img = GetImageFromMagickWand( wand );
     size_t depth = img->depth;
#else
     unsigned long depth = MagickGetImageDepth( wand );
#endif


     _gamma = 1.0f;
     // _gamma = (float) MagickGetImageGamma( wand );
     // if (_gamma <= 0.f || !isfinite(_gamma) ) {
     //     LOG_ERROR( _("Image gamma ") << _gamma << _(" invalid.  Using 1.0") );
     //     _gamma = 1.0f;
     // }
     // _gamma = 1.0f / _gamma;

     image_type::PixelType pixel_type = image_type::kByte;
     StorageType storage = CharPixel;

     if ( !_8bit_cache && depth == 16 && _gamma == 1.0f ) 
     {
	pixel_type = image_type::kShort;
	storage = ShortPixel;
     }

     if ( !_8bit_cache && depth >= 32 && _gamma == 1.0f ) 
     {
	pixel_type = image_type::kFloat;
	storage = FloatPixel;
     }


     image_size( unsigned(dw), unsigned(dh) );

     const char* channels;
     if ( has_alpha )
     {
	channels = "RGBA";
	allocate_pixels( frame, 4, image_type::kRGBA, pixel_type );
     }
     else
     {
	channels = "RGB";
	allocate_pixels( frame, 3, image_type::kRGB, pixel_type );
     }

     {
	SCOPED_LOCK( _mutex );

	Pixel* pixels = (Pixel*)_hires->data().get();
	MagickExportImagePixels( wand, 0, 0, dw, dh, channels, 
				 storage, pixels );
     }

     _compression = MagickGetImageCompression( wand );


     double rx, ry, gx, gy, bx, by, wx, wy;
     MagickGetImageRedPrimary( wand, &rx, &ry );
     MagickGetImageGreenPrimary( wand, &gx, &gy );
     MagickGetImageBluePrimary( wand, &bx, &by );
     MagickGetImageWhitePoint( wand, &wx, &wy );
     if ( rx > 0.0 && ry > 0.0 )
     {
	_chromaticities.red.x = float(rx);
	_chromaticities.red.y = float(ry);
	_chromaticities.green.x = float(gx);
	_chromaticities.green.y = float(gy);
	_chromaticities.blue.x = float(bx);
	_chromaticities.blue.y = float(by);
	_chromaticities.white.x = float(wx);
	_chromaticities.white.y = float(wy);
     }

     if ( _exif.empty() )
     {

	GetImageProperty( img, "exif:*" );
	ResetImagePropertyIterator( img );
	const char* property = GetNextImageProperty(img);
	while ( property )
	{
	   const char* value = GetImageProperty( img, property );
	   if ( value )
	   {
	      //
	      // Format for exif property in imagemagick is like:
	      //       "Exif:InteroperabilityVersion"
	      //       "dpx:InteroperabilityVersion"
	      //
	      // Make that string into something prettier
	      //
	      std::string key;

              // Skip until first ':'
              const char* p = property;
              for ( ; *p != ':' && *p != '\0'; ++p ) ;

              if ( *p != '\0' ) ++p;


	      for ( ; *p != '\0'; ++p )
	      {
		 if ( (isupper((int) ((unsigned char) *p)) != 0) &&
		      (islower((int) ((unsigned char) *(p+1))) != 0))
		    key += ' ';
		 key += *p;
	      }
	      _exif.insert( std::make_pair( key, value ) );
	   }
	   property = GetNextImageProperty(img);
	}
     }

     size_t profileSize;
     unsigned char* tmp = MagickGetImageProfile( wand, "icc", &profileSize );
     if ( !tmp )    tmp = MagickGetImageProfile( wand, "icm", &profileSize );
     if ( profileSize > 0 )
     {
	_profile = strdup( fileroot() );
	mrv::colorProfile::add( _profile, profileSize, (char*)tmp );
     }

     _rendering_intent = (CMedia::RenderingIntent) 
     MagickGetImageRenderingIntent( wand );

     if ( _iptc.empty() )
     {

	tmp = MagickGetImageProfile( wand, "iptc", &profileSize );
	if ( profileSize > 0 )
	{
	   char* attribute;
	   const char* tag;
	   long dataset, record, sentinel;

	   size_t i;
	   size_t length;

	   /*
	     Identify IPTC data.
	   */
	   for (i=0; i < profileSize; i += length)
	   {
	      length=1;
	      sentinel = tmp[i++];
	      if (sentinel != 0x1c)
		 continue;
	      dataset = tmp[i++];
	      record  = tmp[i++];
	      switch (record)
	      {
		 case 5: tag = _("Image Name"); break;
		 case 7: tag = _("Edit Status"); break;
		 case 10: tag = _("Priority"); break;
		 case 15: tag = _("Category"); break;
		 case 20: tag = _("Supplemental Category"); break;
		 case 22: tag = _("Fixture Identifier"); break;
		 case 25: tag = _("Keyword"); break;
		 case 30: tag = _("Release Date"); break;
		 case 35: tag = _("Release Time"); break;
		 case 40: tag = _("Special Instructions"); break;
		 case 45: tag = _("Reference Service"); break;
		 case 47: tag = _("Reference Date"); break;
		 case 50: tag = _("Reference Number"); break;
		 case 55: tag = _("Created Date"); break;
		 case 60: tag = _("Created Time"); break;
		 case 65: tag = _("Originating Program"); break;
		 case 70: tag = _("Program Version"); break;
		 case 75: tag = _("Object Cycle"); break;
		 case 80: tag = _("Byline"); break;
		 case 85: tag = _("Byline Title"); break;
		 case 90: tag = _("City"); break;
		 case 95: tag = _("Province State"); break;
		 case 100: tag = _("Country Code"); break;
		 case 101: tag = _("Country"); break;
		 case 103: tag = _("Original Transmission Reference"); break;
		 case 105: tag = _("Headline"); break;
		 case 110: tag = _("Credit"); break;
		 case 115: tag = _("Src"); break;
		 case 116: tag = _("Copyright String"); break;
		 case 120: tag = _("Caption"); break;
		 case 121: tag = _("Local Caption"); break;
		 case 122: tag = _("Caption Writer"); break;
		 case 200: tag = _("Custom Field 1"); break;
		 case 201: tag = _("Custom Field 2"); break;
		 case 202: tag = _("Custom Field 3"); break;
		 case 203: tag = _("Custom Field 4"); break;
		 case 204: tag = _("Custom Field 5"); break;
		 case 205: tag = _("Custom Field 6"); break;
		 case 206: tag = _("Custom Field 7"); break;
		 case 207: tag = _("Custom Field 8"); break;
		 case 208: tag = _("Custom Field 9"); break;
		 case 209: tag = _("Custom Field 10"); break;
		 case 210: tag = _("Custom Field 11"); break;
		 case 211: tag = _("Custom Field 12"); break;
		 case 212: tag = _("Custom Field 13"); break;
		 case 213: tag = _("Custom Field 14"); break;
		 case 214: tag = _("Custom Field 15"); break;
		 case 215: tag = _("Custom Field 16"); break;
		 case 216: tag = _("Custom Field 17"); break;
		 case 217: tag = _("Custom Field 18"); break;
		 case 218: tag = _("Custom Field 19"); break;
		 case 219: tag = _("Custom Field 20"); break;
		 default: tag = _("unknown"); break;
	      }
	      length = (size_t) (tmp[i++] << 8);
	      length |= tmp[i++];
	      attribute=(char *) AcquireMagickMemory((length+MaxTextExtent)*
						     sizeof(*attribute));
	      if (attribute != (char *) NULL)
	      {
		 (void) CopyMagickString(attribute,(char *) tmp+i,
					 length+1);
		 _iptc.insert( std::make_pair( tag, attribute ) );
		 attribute=(char *) RelinquishMagickMemory(attribute);
	      }
	   }
	}
     }
     DestroyMagickWand( wand );

     return true;
  }


const char* const pixel_type( image_type::PixelType t )
{
    switch( t )
    {
        case image_type::kByte:
            return _("Byte");
        case image_type::kShort:
            return _("Short");
        case image_type::kInt:
            return _("Int");
        case image_type::kFloat:
            return _("Float");
        default:
        case image_type::kHalf:
            return _("Half");
    }
}


const char* const pixel_storage( StorageType storage )
{
    switch( storage )
    {
        case ShortPixel:
            return _("Short");
        case IntegerPixel:
            return _("Int");
        case FloatPixel:
            return _("Float");
        case DoublePixel:
            return _("Double");
        default:
        case CharPixel:
            return _("Byte");
    }
}



bool CMedia::save( const char* file, const ImageOpts* opts ) const
{
    std::string tmp = file;
    std::transform( tmp.begin(), tmp.end(), tmp.begin(),
		     (int(*)(int)) tolower);
    
    if ( strncmp( tmp.c_str() + tmp.size() - 4, ".exr", 4 ) == 0 ||
         strncmp( tmp.c_str() + tmp.size() - 4, ".sxr", 4 ) == 0 )
    {
	return exrImage::save( file, this, opts );
    }

    WandOpts* o = (WandOpts*) opts;

    MagickBooleanType status;

    /*
      Write out an image.
    */
    MagickWand* wand = NewMagickWand();

    mrv::image_type_ptr pic = hires();

    const bool  has_alpha = pic->has_alpha();

    bool must_convert = false;
    const char* channels;
    switch ( pic->format() )
      {
      case image_type::kRGB:
          channels = N_("RGB"); break;
      case image_type::kRGBA:
          channels = N_("RGBA"); break;
      case image_type::kBGRA:
          channels = N_("BGRA"); break;
      case image_type::kBGR:
          channels = N_("BGR"); break;
      case image_type::kLumma:
          channels = N_("I"); break;
      case image_type::kLummaA:
          channels = N_("IA"); break;
      default:
	must_convert = true;
	channels = N_("RGB");
	if ( has_alpha ) channels = N_("RGBA");
	break;
      }


    StorageType storage = CharPixel;
    switch( pic->pixel_type() )
      {
      case image_type::kShort:
	storage = ShortPixel;
	break;
      case image_type::kInt:
	storage = IntegerPixel;
	break;
      case image_type::kFloat:
	storage = FloatPixel;
	break;
      case image_type::kHalf:
	storage = ShortPixel;
	must_convert = true;
	break;
      case image_type::kByte:
      default:
	storage = CharPixel;
	break;
      }

    if ( o->pixel_type() != storage )
    {
        LOG_INFO( _("Original pixel type is ") 
                  << pixel_storage( storage )
                  << (".  Saving pixel type is ")
                  << pixel_storage( o->pixel_type() )
                  << "." );
        must_convert = true;
    }

    // if ( gamma() != 1.0 )
    //    must_convert = true;

    if ( opts->opengl() )
        must_convert = false;

    // Set matte (alpha)
    MagickBooleanType matte = MagickFalse;
    if ( has_alpha ) matte = MagickTrue;
    MagickSetImageMatte( wand, matte );

    /**
     * Load image onto wand
     * 
     */
    boost::uint8_t* pixels = NULL;
    if ( must_convert )
    {
	unsigned pixel_size = 1;
	switch( o->pixel_type() )
        {
            case ShortPixel:
                pixel_size = sizeof(short);
                break;
            case IntegerPixel:
                pixel_size = sizeof(int);
                break;
            case FloatPixel:
                pixel_size = sizeof(float);
                break;
            case DoublePixel:
                pixel_size = sizeof(double);
                break;
            default:
            case CharPixel:
                pixel_size = sizeof(char);
                break;
	  }

        unsigned data_size = width()*height()*pic->channels()*pixel_size;
	pixels = new boost::uint8_t[ data_size ];
        memset( pixels, 0, data_size );
      }
    else
      {
	pixels = (boost::uint8_t*)pic->data().get();
      }



    status = MagickConstituteImage( wand, pic->width(), pic->height(), 
				    channels, o->pixel_type(), pixels );
    if (status == MagickFalse)
      {
	if ( must_convert ) delete [] pixels;
	ThrowWandException( wand );
      }



    if ( !must_convert )
    {
        LOG_INFO( _("No conversion needed.  Gamma: ") << _gamma );
        MagickSetImageGamma( wand, _gamma );
    }
    else
    {
        LOG_INFO( _("Conversion needed.  Gamma: 1.0") );
        MagickSetImageGamma( wand, 1.0 );
    }

    if ( must_convert )
      {
	unsigned int dh = pic->height();
	unsigned int dw = pic->width();
        float one_gamma = 1.0f/_gamma;
	for ( unsigned y = 0; y < dh; ++y )
	  {
	    for ( unsigned x = 0; x < dw; ++x )
	      {
		ImagePixel p = pic->pixel( x, y );

                if ( p.r > 0.f && isfinite(p.r) )
                    p.r = pow( p.r, one_gamma );
                if ( p.g > 0.f && isfinite(p.g) )
                    p.g = pow( p.g, one_gamma );
                if ( p.b > 0.f && isfinite(p.b) )
                    p.b = pow( p.b, one_gamma );

		status = MagickImportImagePixels(wand, x, y, 1, 1, channels, 
                                                 FloatPixel, &p[0] );

	      }
	  }

        if (status == MagickFalse)
        {
            delete [] pixels;
            ThrowWandException( wand );
        }
      }



    //
    // Store EXIF and IPTC data (if any)
    //

    /**
     * Write out image
     * 
     */
    status = MagickWriteImage( wand, file );

    if ( must_convert ) delete [] pixels;

    DestroyMagickWand( wand );

    if (status == MagickFalse)
      {
	ThrowWandException( wand );
      }


    return true;
  }


} // namespace mrv

