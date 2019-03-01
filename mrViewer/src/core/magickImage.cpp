/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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
 * @file   magickImage.cpp
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 * 
 * @brief  A simple wrapper class to read all of ImageMagick's image formats
 *         using the magick core interface.
 * 
 */

#include <iostream>
using namespace std;

#include <cstdio>

#include <algorithm>

#include "mrvColorProfile.h"
#include "mrvString.h"
#include "magickImage.h"
#include "exrImage.h"
#include "mrvI8N.h"
#include "mrvIO.h"


#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )

#define ThrowWandException(wand) \
  { \
    ExceptionType severity; \
   \
    char* description=MagickGetException(wand,&severity); \
    IMG_ERROR( description ); \
    description=(char *) MagickRelinquishMemory(description); \
    return false; \
  }


namespace 
{
  const char* kModule = "magick";


  
  void magick_warning(const ExceptionType type, const char* reason,
		      const char* description)
  {
    LOG_WARNING( reason << ": " << description ); 
  }

  
  void magick_error(const ExceptionType type, const char* reason,
		    const char* description)
  {
    LOG_ERROR( reason << ": " << description ); 
  }

}


namespace mrv {


  magickImage::magickImage() :
    CMedia(),
    _format( NULL )
  {
  }

  magickImage::~magickImage()
  {
    free( _format );
  }

  void magickImage::initialize_magick()
  {
    if ( IsMagickInstantiated() == MagickFalse )
      { 
	MagickCoreGenesis( NULL, MagickFalse );
	SetWarningHandler( magick_warning );
	SetErrorHandler( magick_error );
	SetFatalErrorHandler( magick_error );
      }
  }

  void magickImage::terminate_magick()
  {
    if ( IsMagickInstantiated() == MagickTrue )
      { 
	MagickCoreTerminus();
      }
  }


  /*! ImageMagick does not allow testing a block of data,
    but allows testing a file or FILE*.
  */
  bool magickImage::test(const char* file)
  {
    MagickBooleanType status = MagickTrue;

    initialize_magick();
    
    ExceptionInfo* exception = AcquireExceptionInfo();
    ImageInfo* info = CloneImageInfo(NULL);
    CopyMagickString( info->filename, file, MaxTextExtent );  
  
    Image* img = PingImage( info, exception );
    DestroyImageInfo( info );

    if ( !img ) 
      {
	status = MagickFalse;
	CatchException( exception );
      }
    else
      {
	DestroyImage(img);
      }

    DestroyExceptionInfo( exception );

    terminate_magick();

    if (status == MagickFalse)
      return false;

    return true;
  }





  bool magickImage::initialize()
  {
    //   MagickWandGenesis();
    return true;
  }

  bool magickImage::release()
  {
    //   MagickWandTerminus();
    return true;
  }


  bool magickImage::fetch( const boost::int64_t frame ) 
  {
    MagickBooleanType status = MagickTrue;

    initialize_magick();


    ExceptionInfo* exception = AcquireExceptionInfo();
    ImageInfo* info = CloneImageInfo((ImageInfo *) NULL);
    strcpy( info->filename, sequence_filename(frame).c_str() );

    Image* img = ReadImage( info, exception );
    if ( !img )
      {
	 IMG_ERROR("Could not open image " << sequence_filename(frame) );
	 DestroyImageInfo( info );
	 CatchException( exception );
	 return false;
      }


    /*
      Read an image.
    */

    _format = strdup( AcquireString( img->magick ) );

    _layers.clear();
    _num_channels = 0;

    rgb_layers();
    lumma_layers();

    unsigned numLayers = GetImageListLength( img );
    if ( numLayers > 1 )
      {
	const char* channelName = channel();

	int index = 0;
	for ( unsigned i = 1; i < numLayers; ++i )
	  {
	    char layername[256];

	    Image* subimg = GetImageFromList( img, i );
	    const char* label = ConstantString( GetImageProperty( subimg, 
								  "label" ) );
	    if ( label == NULL )
	      {
		sprintf( layername, _("Layer %d"), i+1 );
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

      }
    

    bool has_alpha = false;
    unsigned long matte = img->matte;
    if ( matte == MagickTrue )
      {
	has_alpha = true;
	alpha_layers();
      }

    /*
      Copy pixels from magick to class
    */
    unsigned int dw = img->columns;
    unsigned int dh = img->rows;

    unsigned long depth = img->depth;
    
    image_type::PixelType pixel_type = image_type::kByte;
    StorageType storage = CharPixel;


    if ( depth == 16 ) 
      {
	pixel_type = image_type::kShort;
	storage = ShortPixel;
      }
    
    if ( depth >= 32 ) 
      {
	pixel_type = image_type::kFloat;
	storage = FloatPixel;
      }



    image_size( dw, dh );

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
    
    PixelType* pixels = (PixelType*)_hires->data().get();

    status = ExportImagePixels( img, 0, 0, dw, dh, channels, storage, pixels,
				exception );
    if ( status == MagickFalse )
      {
	CatchException(exception);
	DestroyImage( img );
	DestroyImageInfo( info );
	DestroyExceptionInfo( exception );
	terminate_magick();
	return false;
      }

    if ( has_alpha )
      {
	for ( unsigned y = 0; y < dh; ++y )
	  {
	    for ( unsigned x = 0; x < dw; ++x )
	      {
		PixelType p = _hires->pixel( x, y );
		p.r *= p.a;
		p.g *= p.a;
		p.b *= p.a;
		_hires->pixel( x, y, p );
	      }
	  }
      }

    _compression = img->compression;

    _gamma = (float) img->gamma;
    if (_gamma <= 0.f ) _gamma = 1.0f;

  
    double rx, ry, gx, gy, bx, by, wx, wy;
    rx = img->chromaticity.red_primary.x;
    ry = img->chromaticity.red_primary.y;
    gx = img->chromaticity.green_primary.x;
    gy = img->chromaticity.green_primary.y;
    bx = img->chromaticity.blue_primary.x;
    by = img->chromaticity.blue_primary.y;
    wx = img->chromaticity.white_point.x;
    wy = img->chromaticity.white_point.y;
    

    if ( rx > 0.0 && ry > 0.0 )
      {
	_chromaticities.red.x = rx;
	_chromaticities.red.y = ry;
	_chromaticities.green.x = gx;
	_chromaticities.green.y = gy;
	_chromaticities.blue.x = bx;
	_chromaticities.blue.y = by;
	_chromaticities.white.x = wx;
	_chromaticities.white.y = wy;
      }

    _exif.clear();
    _iptc.clear();
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
	    //
	    // Make that string into something prettier
	    //
	    bool add = false;
	    if ( strncmp( property, "exif:", 5 ) == 0 )
	      {
		add = true; property += 5;
	      }

	    if ( strcmp( property, "Comment" ) == 0 )   add = true;


	    if ( add )
	      {
		std::string key;
		for (const char* p = property; *p != '\0'; ++p)
		  {
		    if ( (isupper((int) ((unsigned char) *p)) != 0) &&
			 (islower((int) ((unsigned char) *(p+1))) != 0))
		      key += ' ';
		    key += *p;
		  }
		_exif.insert( std::make_pair( key, value ) );
	      }
	  }
	property = GetNextImageProperty(img);
      }

    if ( img->profiles )
      {
	const StringInfo* prof = GetImageProfile( img, "icc" );
	if ( !prof )      prof = GetImageProfile( img, "icm" );
	if ( prof )
	  {
	    _profile = strdup( fileroot() );
	    mrv::colorProfile::add( _profile, GetStringInfoLength(prof), 
				    (char*)GetStringInfoDatum(prof) );
	  }
      }

    _rendering_intent = (CMedia::RenderingIntent) img->rendering_intent;

    const StringInfo* iptc = GetImageProfile( img, "iptc" );
    if ( iptc )
      {
	unsigned profileSize = GetStringInfoLength(iptc);
	const char* tmp = (char*)GetStringInfoDatum(iptc);

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


    DestroyImage( img );
    DestroyImageInfo( info );
    DestroyExceptionInfo( exception );
    terminate_magick();

    return true;
  }


#if 0

  
  bool CMedia::save( const char* file ) const
  {
    if ( strncmp( file + strlen(file) - 4, ".exr", 4 ) == 0 )
      {
	return exrImage::save( file, this );
      }

    MagickBooleanType status = MagickTrue; 

    /*
      Write out an image.
    */
    magickImage::initialize_magick();


    MagickImg* img = NewMagickImg();

    mrv::image_type_ptr frame = hires();

    const bool  has_alpha = frame->has_alpha();

    bool must_convert = false;
    const char* channels;
    switch ( frame->format() )
      {
      case image_type::kRGB:
	channels = "RGB"; break;
      case image_type::kRGBA:
	channels = "RGBA"; break;
      case image_type::kBGRA:
	channels = "BGRA"; break;
      case image_type::kBGR:
	channels = "BGR"; break;
      case image_type::kLumma:
	channels = "I"; break;
      case image_type::kLummaA:
	channels = "IA"; break;
      default:
	must_convert = true;
	channels = "RGB";
	if ( has_alpha ) channels = "RGBA";
	break;
      }


    StorageType storage = CharPixel;
    switch( frame->pixel_type() )
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

    // Set matte (alpha)
    MagickBooleanType matte = MagickFalse;
    if ( has_alpha ) matte = MagickTrue;
    MagickSetImageMatte( img, matte );

    /**
     * Load image onto img
     * 
     */
    boost::uint8_t* pixels = NULL;
    if ( must_convert )
      {
	unsigned pixel_size = 1;
	switch( storage )
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
	  default:
	  case CharPixel:
	    pixel_size = sizeof(char);
	    break;
	  }

	pixels = new boost::uint8_t[ width() * height() * 
				     frame->channels() * pixel_size ];
      }
    else
      {
	pixels = (boost::uint8_t*)frame->data().get();
      }

    status = MagickConstituteImage( img, width(), height(), 
				    channels, storage, pixels );
    if (status == MagickFalse)
      {
	if ( must_convert ) delete [] pixels;
	ThrowImgException(img);
      }

    MagickSetImageGamma( img, gamma() );

    if ( must_convert )
      {
	unsigned int dh = height();
	unsigned int dw = width();
	for ( unsigned y = 0; y < dh; ++y )
	  {
	    for ( unsigned x = 0; x < dw; ++x )
	      {
		CMedia::PixelType p = frame->pixel( x, y );
		MagickSetImagePixels(img, x, y, 1, 1,"RGBA", FloatPixel, &p );
	      }
	  }
      }
    

    //
    // Store EXIF and IPTC data (if any)
    //

    /**
     * Write out image
     * 
     */
    status = MagickWriteImage( img, file );


    if (status == MagickFalse)
      {
	if ( must_convert ) delete [] pixels;
	ThrowImgException( img );
      }

    if ( must_convert ) delete [] pixels;

    DestroyMagickImg( img );


    magickImage::terminate_magick();

    return true;
  }

#endif


} // namespace mrv

