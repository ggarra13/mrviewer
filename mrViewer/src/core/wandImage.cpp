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

#include "core/mrvI8N.h"
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

#include <MagickWand/MagickWand.h>

#include <ImfStandardAttributes.h>
#include <ImfIntAttribute.h>
#include <ImfDoubleAttribute.h>
#include <ImfVecAttribute.h>
#include <ImfMatrixAttribute.h>

#include "core/oiioImage.h"
#include "core/mrvMath.h"
#include "core/mrvImageOpts.h"
#include "core/mrvColorOps.h"
#include "core/Sequence.h"
#include "core/mrvColorProfile.h"
#include "core/mrvString.h"
#include "core/wandImage.h"
#include "core/picImage.h"
#include "core/exrImage.h"
#include "core/aviImage.h"
#include "core/mrvThread.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvPreferences.h"
#include "mrViewer.h"
#include "gui/mrvIO.h"

#undef DBG
#define DBG(x) std::cerr << x << std::endl;

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
    _has_alpha( false ),
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
    size_t pos = f.rfind( '.' );
    if ( pos != std::string::npos && pos != f.size() )
    {
        std::string ext = f.substr( pos+1, f.size() );
        std::transform( ext.begin(), ext.end(), ext.begin(),
                        (int(*)(int))tolower );
        if ( ext == "pdf" || ext == "tx" )
            return false;
    }

    MagickWandGenesis();
    MagickBooleanType status = MagickFalse;

    MagickWand* wand = NewMagickWand();

    try
    {
        status = MagickPingImage( wand, file );
        if (status == MagickFalse )
        {
            ThrowWandExceptionPing( wand );
        }
    }
    catch( const std::exception& e )
    {
        LOG_ERROR( e.what() );
    }

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

bool wandImage::fetch( mrv::image_type_ptr& canvas, const boost::int64_t frame )
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

    if ( _num_channels == 0 )
    {
        _layers.clear();

        ColorspaceType colorspace = MagickGetImageColorspace( wand );
        switch( colorspace )
        {
        case RGBColorspace:
        case sRGBColorspace:
        case LogColorspace:
        case scRGBColorspace:
        case YCbCrColorspace:
        case YCCColorspace:
        case YIQColorspace:
        case YPbPrColorspace:
        case Rec601YCbCrColorspace:
        case Rec709YCbCrColorspace:
            rgb_layers();
            lumma_layers();
            break;
        default:
            lumma_layers();
            break;
        }

        _has_alpha = false;
        status = MagickGetImageAlphaChannel( wand );
        if ( status == MagickTrue )
        {
            _has_alpha = true;
            alpha_layers();
        }

        size_t profileSize;
        unsigned char* tmp = MagickGetImageProfile( wand, "icc", &profileSize );
        if ( !tmp )    tmp = MagickGetImageProfile( wand, "icm", &profileSize );
        if ( profileSize > 0 )
        {
            _profile = strdup( fileroot() );
            mrv::colorProfile::add( _profile, profileSize, (char*)tmp );
        }

        size_t index = 0;
        unsigned numLayers = (unsigned) MagickGetNumberImages( wand );
        if ( numLayers > 1 )
        {
            const char* channelName = channel();

            std::string layer;

            for ( unsigned i = 0; i < numLayers; ++i )
            {

                char layername[256];

                MagickSetIteratorIndex( wand, i );
                const char* label = MagickGetImageProperty( wand, "label" );
                if ( label == NULL )
                {
                    sprintf( layername, _( "Layer %d" ), i+1 );
                }
                else
                {
                    std::string ly = label;

                    size_t pos;
                    while ( (pos = ly.find( '.' )) != std::string::npos )
                    {
                        std::string n = ly.substr( 0, pos );
                        n += '_';
                        if ( pos != ly.size() )
                            n += ly.substr( pos+1, ly.size() );
                        ly = n;
                    }
                    strcpy( layername, ly.c_str() );
                }

                ColorspaceType colorspace = MagickGetImageColorspace( wand );

                std::string ly = layername;

                _layers.push_back( ly );
                switch( colorspace )
                {
                case sRGBColorspace:
                case RGBColorspace:
                    _layers.push_back( ly + ".R" );
                    _layers.push_back( ly + ".G" );
                    _layers.push_back( ly + ".B" );
                    break;
                case CMYKColorspace:
                    _layers.push_back( ly + ".C" );
                    _layers.push_back( ly + ".M" );
                    _layers.push_back( ly + ".Y" );
                    _layers.push_back( ly + ".K" );
                    break;
                case GRAYColorspace:
                    if ( ( ly.find("Z") != std::string::npos ) ||
                            ( ly.find("depth") != std::string::npos ) )
                        _layers.push_back( ly + ".Z" );
                    else
                        _layers.push_back( ly + ".Y" );
                default:
                    break;
                }

                status = MagickGetImageAlphaChannel( wand );
                if ( status == MagickTrue )
                {
                    _layers.push_back( ly + ".A" );
                }

                ++_num_channels;


                if ( i == 0 )
                {
                    size_t dw = MagickGetImageWidth( wand );
                    size_t dh = MagickGetImageHeight( wand );

                    display_window( 0, 0, (int)dw-1, (int)dh-1, frame );
                }


                if ( channelName && strcmp( layername, channelName ) == 0 )
                {
                    index = i;
                    layer = layername;
                }


            }
        }

        MagickSetIteratorIndex( wand, index );
    }

    /*
      Copy pixels from magick to class
    */
    size_t dw = MagickGetImageWidth( wand );
    size_t dh = MagickGetImageHeight( wand );

    size_t depth = MagickGetImageDepth( wand );

    Image* img = GetImageFromMagickWand( wand );

    // Get the layer data window

    data_window( img->page.x, img->page.y,
                 (img->page.x + dw - 1),
                 (img->page.y + dh - 1), frame );


    _gamma = 1.0f;

    image_type::PixelType pixel_type = image_type::kByte;
    StorageType storage = CharPixel;

    if ( !_8bit_cache )
    {
        if ( depth == 16 && _gamma == 1.0f )
        {
            pixel_type = image_type::kShort;
            storage = ShortPixel;
        }
        else if ( depth >= 32 && _gamma == 1.0f )
        {
            pixel_type = image_type::kFloat;
            storage = FloatPixel;
        }
    }


    image_size( unsigned(dw), unsigned(dh) );

    const char* channels;
    if ( _has_alpha )
    {

        channels = "RGBA";
        if ( ! allocate_pixels( canvas, frame, 4, image_type::kRGBA, pixel_type,
                                unsigned(dw), unsigned(dh) ) )
            return false;
    }
    else
    {

        channels = "RGB";
        if (! allocate_pixels( canvas, frame, 3, image_type::kRGB, pixel_type,
                               unsigned(dw), unsigned(dh) ) )
            return false;
    }

    {
        Pixel* pixels = (Pixel*)canvas->data().get();
        MagickExportImagePixels( wand, 0, 0, dw, dh, channels,
                                 storage, pixels );
    }


    _compression = MagickGetImageCompression( wand );


    double rx, ry, rz, gx, gy, gz, bx, by, bz, wx, wy, wz;
    MagickGetImageRedPrimary( wand, &rx, &ry, &rz );
    MagickGetImageGreenPrimary( wand, &gx, &gy, &gz );
    MagickGetImageBluePrimary( wand, &bx, &by, &bz );
    MagickGetImageWhitePoint( wand, &wx, &wy, &wz );
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

    if ( _attrs.empty() )
    {
        ExceptionInfo* exception = NULL;
        GetImageProperty( img, "exif:*", exception );
        ResetImagePropertyIterator( img );
        const char* property = GetNextImageProperty(img);
        setlocale( LC_NUMERIC, "C" );
        while ( property )
        {

            const char* value = GetImageProperty( img, property, exception );
            if ( value )
            {
                //
                // Format for exif property in imagemagick is like:
                //       "Exif:InteroperabilityVersion"
                //       "dpx:InteroperabilityVersion"
                //
                // Make that string into something prettier
                //
                std::string key = property;

                if ( key.substr(0,4) == "Exif" ||
                        key.substr(0,4) == "exif" )
                {
                    key.clear();
                    // Skip until first ':'
                    const char* p = property;
                    for ( ; *p != ':' && *p != '\0'; ++p ) ;

                    if ( *p != '\0' ) ++p;

                    for ( ; *p != '\0'; ++p )
                    {
                        // Make 'superDad' into 'super Dad'
                        if ( !key.empty() &&
                                (isupper((int) ((unsigned char) *p)) != 0) &&
                                (islower((int) ((unsigned char) *(p+1))) != 0))
                            key += ' ';
                        key += *p;
                    }
                }


                if ( key == "dpx:film.frame_rate" )
                {
                    double g = atof( value );
                    if ( g > 0.0f && g < 250.0f )
                    {
                        _orig_fps = _fps = _play_fps = g;
                    }
                }
                else if ( key == "dpx:television.frame_rate" )
                {
                    double g = atof( value );
                    if ( g > 0.0f && g < 250.0f )
                    {
                        _orig_fps = _fps = _play_fps = g;
                    }
                }
                else if ( key == "dpx:television.gamma" )
                {
                    float g = atof( value );
                    if ( g > 0.0f && g < 128.0f )
                    {
                        _gamma = g;
                    }
                }
                else if ( key == "dpx:television.time.code" ||
                          key.rfind( "timecode" ) != std::string::npos )
                {
                    Imf::TimeCode t = CMedia::str2timecode( value );
                    process_timecode( t );
                    Imf::TimeCodeAttribute attr( t );
                    _attrs.insert( std::make_pair( N_("timecode"),
                                                   attr.copy() ) );
                    property = GetNextImageProperty(img);
                    continue;
                }

                // We always add the EXIF attribute even if it passes
                // other tests so that if user saves the file all is kept
                Imf::StringAttribute attr( value );
                _attrs.insert( std::make_pair( key, attr.copy() ) );
            }
            property = GetNextImageProperty(img);
        }

        setlocale( LC_NUMERIC, "" );

        size_t profileSize;
        unsigned char* tmp = MagickGetImageProfile( wand, "iptc",
                             &profileSize );
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
                case 5:
                    tag = _("Image Name");
                    break;
                case 7:
                    tag = _("Edit Status");
                    break;
                case 10:
                    tag = _("Priority");
                    break;
                case 15:
                    tag = _("Category");
                    break;
                case 20:
                    tag = _("Supplemental Category");
                    break;
                case 22:
                    tag = _("Fixture Identifier");
                    break;
                case 25:
                    tag = _("Keyword");
                    break;
                case 30:
                    tag = _("Release Date");
                    break;
                case 35:
                    tag = _("Release Time");
                    break;
                case 40:
                    tag = _("Special Instructions");
                    break;
                case 45:
                    tag = _("Reference Service");
                    break;
                case 47:
                    tag = _("Reference Date");
                    break;
                case 50:
                    tag = _("Reference Number");
                    break;
                case 55:
                    tag = _("Created Date");
                    break;
                case 60:
                    tag = _("Created Time");
                    break;
                case 65:
                    tag = _("Originating Program");
                    break;
                case 70:
                    tag = _("Program Version");
                    break;
                case 75:
                    tag = _("Object Cycle");
                    break;
                case 80:
                    tag = _("Byline");
                    break;
                case 85:
                    tag = _("Byline Title");
                    break;
                case 90:
                    tag = _("City");
                    break;
                case 95:
                    tag = _("Province State");
                    break;
                case 100:
                    tag = _("Country Code");
                    break;
                case 101:
                    tag = _("Country");
                    break;
                case 103:
                    tag = _("Original Transmission Reference");
                    break;
                case 105:
                    tag = _("Headline");
                    break;
                case 110:
                    tag = _("Credit");
                    break;
                case 115:
                    tag = _("Src");
                    break;
                case 116:
                    tag = _("Copyright String");
                    break;
                case 120:
                    tag = _("Caption");
                    break;
                case 121:
                    tag = _("Local Caption");
                    break;
                case 122:
                    tag = _("Caption Writer");
                    break;
                case 200:
                    tag = _("Custom Field 1");
                    break;
                case 201:
                    tag = _("Custom Field 2");
                    break;
                case 202:
                    tag = _("Custom Field 3");
                    break;
                case 203:
                    tag = _("Custom Field 4");
                    break;
                case 204:
                    tag = _("Custom Field 5");
                    break;
                case 205:
                    tag = _("Custom Field 6");
                    break;
                case 206:
                    tag = _("Custom Field 7");
                    break;
                case 207:
                    tag = _("Custom Field 8");
                    break;
                case 208:
                    tag = _("Custom Field 9");
                    break;
                case 209:
                    tag = _("Custom Field 10");
                    break;
                case 210:
                    tag = _("Custom Field 11");
                    break;
                case 211:
                    tag = _("Custom Field 12");
                    break;
                case 212:
                    tag = _("Custom Field 13");
                    break;
                case 213:
                    tag = _("Custom Field 14");
                    break;
                case 214:
                    tag = _("Custom Field 15");
                    break;
                case 215:
                    tag = _("Custom Field 16");
                    break;
                case 216:
                    tag = _("Custom Field 17");
                    break;
                case 217:
                    tag = _("Custom Field 18");
                    break;
                case 218:
                    tag = _("Custom Field 19");
                    break;
                case 219:
                    tag = _("Custom Field 20");
                    break;
                default:
                    tag = _("unknown");
                    break;
                }
                length = (size_t) (tmp[i++] << 8);
                length |= tmp[i++];
                attribute=(char *) AcquireMagickMemory((length+MaxTextExtent)*
                                                       sizeof(*attribute));
                if (attribute != (char *) NULL)
                {
                    (void) CopyMagickString(attribute,(char *) tmp+i,
                                            length+1);
                    Imf::StringAttribute attr( attribute );
                    _attrs.insert( std::make_pair( tag, attr.copy() ) );
                    attribute=(char *) RelinquishMagickMemory(attribute);
                }
            }
        }
    }


    _rendering_intent = (CMedia::RenderingIntent)
                        MagickGetImageRenderingIntent( wand );

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
    case LongPixel:
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

typedef std::vector< uint8_t* > Buffers;

static void destroyPixels( Buffers& bufs )
{
    Buffers::iterator i = bufs.begin();
    Buffers::iterator e = bufs.end();
    for ( ; i != e; ++i )
    {
        delete [] *i;
    }
}

static void save_attribute( const CMedia* img,
                            MagickWand* wand,
                            const CMedia::Attributes::const_iterator& i )
{
    char buf[256];
    MagickBooleanType status;
    std::string key = i->first;
    // size_t pos;
    // while ( ( pos = key.find(' ') ) != std::string::npos )
    // {
    //     key = key.substr( 0, pos ) + key.substr( pos+1, key.size() );
    // }
    // key = "exif:" + key;

    {
        Imf::StringAttribute* attr =
            dynamic_cast< Imf::StringAttribute* >( i->second );
        if ( attr )
        {
            status = MagickSetImageProperty( wand, key.c_str(),
                                             attr->value().c_str() );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::IntAttribute* attr =
            dynamic_cast< Imf::IntAttribute* >( i->second );
        if ( attr )
        {
            sprintf( buf, "%d", attr->value() );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::FloatAttribute* attr =
            dynamic_cast< Imf::FloatAttribute* >( i->second );
        if ( attr )
        {
            sprintf( buf, "%g", attr->value() );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::DoubleAttribute* attr =
            dynamic_cast< Imf::DoubleAttribute* >( i->second );
        if ( attr )
        {
            sprintf( buf, "%lg", attr->value() );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::V2iAttribute* attr =
            dynamic_cast< Imf::V2iAttribute* >( i->second );
        if ( attr )
        {
            const Imath::V2i& v = attr->value();
            sprintf( buf, "%d %d", v.x, v.y );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::V2fAttribute* attr =
            dynamic_cast< Imf::V2fAttribute* >( i->second );
        if ( attr )
        {
            const Imath::V2f& v = attr->value();
            sprintf( buf, "%g %g", v.x, v.y );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::V2dAttribute* attr =
            dynamic_cast< Imf::V2dAttribute* >( i->second );
        if ( attr )
        {
            const Imath::V2d& v = attr->value();
            sprintf( buf, "%lg %lg", v.x, v.y );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::V3iAttribute* attr =
            dynamic_cast< Imf::V3iAttribute* >( i->second );
        if ( attr )
        {
            const Imath::V3i& v = attr->value();
            sprintf( buf, "%d %d %d", v.x, v.y, v.z );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::V3fAttribute* attr =
            dynamic_cast< Imf::V3fAttribute* >( i->second );
        if ( attr )
        {
            const Imath::V3f& v = attr->value();
            sprintf( buf, "%g %g %g", v.x, v.y, v.z );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::V3dAttribute* attr =
            dynamic_cast< Imf::V3dAttribute* >( i->second );
        if ( attr )
        {
            const Imath::V3d& v = attr->value();
            sprintf( buf, "%lg %lg %lg", v.x, v.y, v.z );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::Box2iAttribute* attr =
            dynamic_cast< Imf::Box2iAttribute* >( i->second );
        if ( attr )
        {
            const Imath::Box2i& v = attr->value();
            sprintf( buf, "%d %d  %d %d", v.min.x, v.min.y, v.max.x, v.max.y );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::Box2fAttribute* attr =
            dynamic_cast< Imf::Box2fAttribute* >( i->second );
        if ( attr )
        {
            const Imath::Box2f& v = attr->value();
            sprintf( buf, "%f %f  %f %f", v.min.x, v.min.y, v.max.x, v.max.y );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::KeyCodeAttribute* attr =
            dynamic_cast< Imf::KeyCodeAttribute* >( i->second );
        if ( attr )
        {
            const Imf::KeyCode& k = attr->value();
            std::string key;

#define SET_KEYCODE( attr ) \
            key = i->first + "." + #attr; \
            sprintf( buf, "%d", k.attr() ); \
            status = MagickSetImageProperty( wand, key.c_str(), buf ); \
            if ( status != MagickTrue ) \
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );

            SET_KEYCODE( filmMfcCode );
            SET_KEYCODE( filmType );
            SET_KEYCODE( prefix );
            SET_KEYCODE( count );
            SET_KEYCODE( perfOffset );
            SET_KEYCODE( perfsPerFrame );
            SET_KEYCODE( perfsPerCount );
#undef SET_KEYCODE
            return;
        }
    }
    {
        Imf::ChromaticitiesAttribute* attr =
            dynamic_cast< Imf::ChromaticitiesAttribute* >( i->second );
        if ( attr )
        {
            const Imf::Chromaticities& v = attr->value();
            sprintf( buf,
                     "%g %g  "
                     "%g %g  "
                     "%g %g  "
                     "%g %g",
                     v.red.x, v.red.y,
                     v.green.x, v.green.y,
                     v.blue.x, v.blue.y,
                     v.white.x, v.white.y );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::M33fAttribute* attr =
            dynamic_cast< Imf::M33fAttribute* >( i->second );
        if ( attr )
        {
            const Imath::M33f& v = attr->value();
            sprintf( buf,
                     "%g %g %g  "
                     "%g %g %g  "
                     "%g %g %g",
                     v[0][0], v[0][1], v[0][2],
                     v[1][0], v[1][1], v[1][2],
                     v[2][0], v[2][1], v[2][2] );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::M33dAttribute* attr =
            dynamic_cast< Imf::M33dAttribute* >( i->second );
        if ( attr )
        {
            const Imath::M33d& v = attr->value();
            sprintf( buf,
                     "%lg %lg %lg  "
                     "%lg %lg %lg  "
                     "%lg %lg %lg",
                     v[0][0], v[0][1], v[0][2],
                     v[1][0], v[1][1], v[1][2],
                     v[2][0], v[2][1], v[2][2] );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::M44fAttribute* attr =
            dynamic_cast< Imf::M44fAttribute* >( i->second );
        if ( attr )
        {
            const Imath::M44f& v = attr->value();
            sprintf( buf,
                     "%g %g %g %g  "
                     "%g %g %g %g  "
                     "%g %g %g %g  "
                     "%g %g %g %g",
                     v[0][0], v[0][1], v[0][2], v[0][3],
                     v[1][0], v[1][1], v[1][2], v[1][3],
                     v[2][0], v[2][1], v[2][2], v[2][3],
                     v[3][0], v[3][1], v[3][2], v[3][3] );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            return;
        }
    }
    {
        Imf::M44dAttribute* attr =
            dynamic_cast< Imf::M44dAttribute* >( i->second );
        if ( attr )
        {
            const Imath::M44d& v = attr->value();
            sprintf( buf,
                     "%lg %lg %lg %lg  "
                     "%lg %lg %lg %lg  "
                     "%lg %lg %lg %lg  "
                     "%lg %lg %lg %lg",
                     v[0][0], v[0][1], v[0][2], v[0][3],
                     v[1][0], v[1][1], v[1][2], v[1][3],
                     v[2][0], v[2][1], v[2][2], v[2][3],
                     v[3][0], v[3][1], v[3][2], v[3][3] );
            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    {
        Imf::TimeCodeAttribute* attr =
            dynamic_cast< Imf::TimeCodeAttribute* >( i->second );
        if ( attr )
        {
            Imf::TimeCode t( attr->value() );

            mrv::Timecode::Display d = mrv::Timecode::kTimecodeNonDrop;
            if ( t.dropFrame() ) d = mrv::Timecode::kTimecodeDropFrame;
            char buf[64];
            mrv::Timecode::format( buf, d, img->frame(), img->timecode(),
                                   img->fps(), true );

            status = MagickSetImageProperty( wand, key.c_str(), buf );
            if ( status != MagickTrue )
                LOG_ERROR( _("Could not set ") << key << _(" attribute") );
            return;
        }
    }
    mrvALERT( _("Unknown data type to convert to string") );
}

bool CMedia::save( const char* file, const ImageOpts* opts ) const
{
    if ( dynamic_cast< const EXROpts* >( opts ) != NULL )
    {
        return exrImage::save( file, this, opts );
    }

    std::string f = file;
    std::transform( f.begin(), f.end(), f.begin(), (int(*)(int)) tolower );

    if ( f.substr( f.size()-4, f.size() ) == ".pic" )
    {
        return picImage::save( file, this, opts );
    }

    if ( dynamic_cast< const OIIOOpts* >( opts ) != NULL )
    {
        OIIOOpts* o = (OIIOOpts*) opts;
        return oiioImage::save( file, this, o );
    }


    if ( dynamic_cast< const WandOpts* >( opts ) == NULL )
    {
        LOG_ERROR( _("Unknown image format to save") );
        return false;
    }

    WandOpts* o = (WandOpts*) opts;

    MagickBooleanType status;
    std::string filename = file;


    /*
      Write out an image.
    */
    MagickWand* wand = NewMagickWand();

    CMedia* p = const_cast< CMedia* >( this );

    const char* old_channel = channel();

    bool has_composite = false;

    stringArray::const_iterator i = p->layers().begin();
    stringArray::const_iterator s = i;
    stringArray::const_iterator e = p->layers().end();
    if ( !opts->all_layers() )
    {
        for ( ; i != e; ++i )
        {
            if ( ( old_channel && *i == old_channel ) || *i == _("Color") )
            {
                e = i+1;
                break;
            }
        }
        if ( i == e )
        {
            i = p->layers().begin();
            e = i+1;
        }
    }

    CompressionType compression = RLECompression;
    std::transform( filename.begin(), filename.end(), filename.begin(),
                    (int(*)(int)) tolower );


    if ( filename.rfind( ".tif" ) != std::string::npos )
    {
        has_composite = true;
        compression = LZWCompression;
    }
    else if ( filename.rfind( ".psd" ) != std::string::npos )
    {
        has_composite = true;
        compression = RLECompression;
    }

    Buffers bufs;

    std::string root = "ZXVCW#!";
    mrv::Recti dpw = display_window();

    for ( ; i != e; ++i )
    {
        std::string x = *i;
        // std::cerr << "layer " << x << std::endl;

        if ( x == _("Lumma") || x == _("Alpha Overlay") ||
                x == _("Red") || x == _("Green") ||
                x == _("Blue") || x == _("Alpha") ||
                x == N_("RY") || x == N_("BY") )
        {
            continue;
        }

        std::string ext = x;

        size_t pos = ext.rfind( '.' );
        if ( pos != std::string::npos && pos != ext.size() )
        {
            ext = ext.substr( pos+1, ext.size() );
        }

        std::transform( ext.begin(), ext.end(), ext.begin(),
                        (int(*)(int)) toupper);

        if ( x.find(root) == 0 && root != "Z" ) continue;

        root = x;

        if ( x == _("Color") ) x = "";


        p->channel( x.c_str() );

        mrv::image_type_ptr pic = hires();

        mrv::Recti daw = data_window();


        image_type::Format format = pic->format();

        bool  has_alpha = pic->has_alpha();


        bool must_convert = false;
        const char* channels;
        switch ( format )
        {
        case image_type::kRGB:
            channels = N_("RGB");
            break;
        case image_type::kRGBA:
            channels = N_("RGBA");
            break;
        case image_type::kBGRA:
            channels = N_("BGRA");
            break;
        case image_type::kBGR:
            channels = N_("BGR");
            break;
        case image_type::kLumma:
            channels = N_("I");
            break;
        case image_type::kLummaA:
            channels = N_("IA");
            break;
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
            storage = LongPixel;
            break;
        case image_type::kFloat:
            storage = FloatPixel;
            break;
        case image_type::kHalf:
            storage = FloatPixel;
            must_convert = true;
            break;
        case image_type::kByte:
        default:
            storage = CharPixel;
            break;
        }

        if ( o->pixel_type() != storage && pic->frame() == first_frame() )
        {
            LOG_INFO( _("Original pixel type is ")
                      << pic->pixel_depth()
                      << _(".  Saving pixel type is ")
                      << pixel_storage( o->pixel_type() )
                      << "." );
            must_convert = true;
        }

        // if ( gamma() != 1.0 )
        //    must_convert = true;


        const std::string& display = mrv::Preferences::OCIO_Display;
        const std::string& view = mrv::Preferences::OCIO_View;

        boost::uint8_t* pixels = NULL;
        unsigned pixel_size;

        if ( Preferences::use_ocio && !display.empty() && !view.empty() &&
                Preferences::uiMain->uiView->use_lut() )
        {
            must_convert = true;
        }

        if ( opts->opengl() )
            must_convert = false;

        /**
         * Load image onto wand
         *
         */
        switch( o->pixel_type() )
        {
        case ShortPixel:
            pixel_size = sizeof(short);
            break;
        case LongPixel:
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

        if ( must_convert )
        {
            // Memory is kept until we save the image
            size_t data_size = width()*height()*pic->channels()*sizeof(float);
            pixels = new boost::uint8_t[ data_size ];
            bufs.push_back( pixels );
        }
        else
        {
            // Memory is handled by the mrv::image_ptr pic class.  No
            // need to delete it here.
            pixels = (boost::uint8_t*)pic->data().get();
        }

        unsigned dw = pic->width();
        unsigned dh = pic->height();

        MagickWand* w = NewMagickWand();

        ColorspaceType colorspace = sRGBColorspace;
        MagickSetImageColorspace( w, colorspace );

        status = MagickConstituteImage( w, dw, dh, channels,
                                        o->pixel_type(), pixels );
        if (status == MagickFalse)
        {
            destroyPixels(bufs);
            ThrowWandException( wand );
        }

        if ( !must_convert )
        {
            if ( pic->frame() == first_frame() )
            {
                LOG_INFO( _("No conversion needed.  Gamma: ") << _gamma );
            }
            MagickSetImageGamma( w, _gamma );
        }
        else
        {
            if ( pic->frame() == first_frame() )
            {
                LOG_INFO( _("Conversion needed.  Gamma: 1.0") );
            }
            MagickSetImageGamma( w, 1.0 );
        }

        if ( must_convert )
        {


            image_type_ptr ptr = image_type_ptr( new image_type(
                    pic->frame(),
                    dw, dh, 4,
                    image_type::kRGBA,
                    image_type::kFloat
                                                 ) );
            copy_image( ptr, pic );

            const std::string& display = mrv::Preferences::OCIO_Display;
            const std::string& view = mrv::Preferences::OCIO_View;

            if ( Preferences::use_ocio && !display.empty() && !view.empty() &&
                    Preferences::uiMain->uiView->use_lut() )
            {
                try
                {
                    bake_ocio( ptr, this );
                }
                catch( const std::exception& e )
                {
                    LOG_ERROR( "[ocio] " << e.what() );
                    return false;
                }
            }


            // LOG_INFO( "Converting..." );
            if ( ! mrv::is_equal( _gamma, 1.0f ) )
            {
                float one_gamma = 1.0f / _gamma;
                for ( unsigned y = 0; y < dh; ++y )
                {
                    for ( unsigned x = 0; x < dw; ++x )
                    {
                        ImagePixel p = ptr->pixel( x, y );

                        // This code is equivalent to p.r = powf( p.r, gamma )
                        // but faster
                        if ( p.r > 0.f && isfinite(p.r) )
                            p.r = expf( logf(p.r) * one_gamma );
                        if ( p.g > 0.f && isfinite(p.g) )
                            p.g = expf( logf(p.g) * one_gamma );
                        if ( p.b > 0.f && isfinite(p.b) )
                            p.b = expf( logf(p.b) * one_gamma );
                        if ( !has_alpha ) p.a = 1.0f;

                        p.clamp();
                        ptr->pixel( x, y, p );
                    }
                }
            }

            ImagePixel* p = (ImagePixel*) ptr->data().get();
            status = MagickImportImagePixels(w, 0, 0, dw, dh, "RGBA",
                                             FloatPixel, &p[0] );
            if (status == MagickFalse)
            {
                destroyPixels(bufs);
                ThrowWandException( wand );
            }
        }


        MagickSetLastIterator( wand );


        MagickSetImageDepth( w, pixel_size*8 );
        if ( has_alpha )
        {
            MagickSetImageAlphaChannel( w, ActivateAlphaChannel );
        }


        std::string label = x;
        if ( label[0] == '#' )
        {
            pos = label.find( ' ' );
            if ( pos != std::string::npos && pos != label.size() )
            {
                label = label.substr( pos+1, label.size() );
            }
        }

        if ( !label.empty() )
            MagickSetImageProperty( w, "label", label.c_str() );
        else
            MagickSetImageProperty( w, "label", NULL );

        Image* img = GetImageFromMagickWand( w );

        img->resolution.x = img->resolution.y = 72;  // so GIMP doesn't bark

        img->page.x = daw.x();
        img->page.y = daw.y();
        img->page.width = daw.w();
        img->page.height = daw.h();

        // We set the compression albeit ImageMagick seems not to
        // compress the files.
        MagickSetImageCompression( w, compression );
        MagickAddImage( wand, w );
        MagickSetImageCompression( wand, compression );

        DestroyMagickWand( w );


        if ( label == "" && has_composite )
        {
            w = NewMagickWand();

            MagickSetImageColorspace( w, colorspace );
            unsigned width = dpw.w();
            unsigned height = dpw.h();

            unsigned data_size = width*height*3;
            pixels = new boost::uint8_t[ data_size ];
            memset( pixels, 0, data_size );
            bufs.push_back( pixels );

            status = MagickConstituteImage( w, width, height, "RGB",
                                            CharPixel, pixels );
            if (status == MagickFalse)
            {
                DestroyMagickWand( w );
                destroyPixels(bufs);
                ThrowWandException( wand );
            }

            mrv::image_type_ptr ptr;
            ptr = mrv::image_type_ptr( new image_type( pic->frame(),
                                       dw, dh, 4,
                                       mrv::image_type::kRGBA,
                                       mrv::image_type::kFloat
                                                     ) );

            float one_gamma = 1.0f / _gamma;
            for ( unsigned y = 0; y < dh; ++y )
            {
                for ( unsigned x = 0; x < dw; ++x )
                {
                    ImagePixel p = pic->pixel( x, y );

                    if ( p.r > 0.f && isfinite(p.r) )
                        p.r = expf( logf(p.r) * one_gamma );
                    if ( p.g > 0.f && isfinite(p.g) )
                        p.g = expf( logf(p.g) * one_gamma );
                    if ( p.b > 0.f && isfinite(p.b) )
                        p.b = expf( logf(p.b) * one_gamma );
                    if ( !has_alpha ) p.a = 1.0f;

                    ptr->pixel( x, y, p );
                }
            }

            ImagePixel* p = (ImagePixel*)ptr->data().get();

            if ( Preferences::use_ocio && !display.empty() && !view.empty() &&
                    Preferences::uiMain->uiView->use_lut() )
            {

                try
                {
                    bake_ocio( ptr, this );
                }
                catch( const std::exception& e )
                {
                    LOG_ERROR( "[ocio] " << e.what() );
                    return false;
                }
            }

            status = MagickImportImagePixels( w, daw.x(), daw.y(), dw, dh,
                                              "RGBA", FloatPixel, &p[0] );
            if (status == MagickFalse)
            {
                DestroyMagickWand( w );
                destroyPixels(bufs);
                ThrowWandException( wand );
            }

            status = MagickSetImageGamma( w, 1.0 );
            if (status == MagickFalse)
            {
                DestroyMagickWand( w );
                destroyPixels(bufs);
                ThrowWandException( wand );
            }

            status = MagickSetImageProperty( w, "label", "Composite" );
            if (status == MagickFalse)
            {
                DestroyMagickWand( w );
                destroyPixels(bufs);
                ThrowWandException( wand );
            }

            status = MagickSetImageCompression( w, compression );
            if (status == MagickFalse)
            {
                DestroyMagickWand( w );
                destroyPixels(bufs);
                ThrowWandException( wand );
            }

            status = MagickSetImageDepth( w, 8 );
            if (status == MagickFalse)
            {
                DestroyMagickWand( w );
                destroyPixels(bufs);
                ThrowWandException( wand );
            }

            Image* img = GetImageFromMagickWand( w );
            img->resolution.x = img->resolution.y = 72;  // so GIMP doesn't bark

            // This is the composite channel, add it as first channel.
            // This is needed to have Photoshop distinguish it and set
            // its canvas.
            MagickSetFirstIterator( wand );
            MagickAddImage( wand, w );

            DestroyMagickWand( w );
        }

    }

    //
    // Store EXIF and IPTC data (if any)
    //
    {
        setlocale( LC_NUMERIC, "C" );  // Set locale to C
        Attributes::const_iterator i = _attrs.begin();
        Attributes::const_iterator e = _attrs.end();
        for ( ; i != e; ++i )
        {
            save_attribute( this, wand, i );
        }

        char buf[32];
        sprintf( buf, "%2.4f", _fps.load() );

        if ( filename.rfind( ".dpx" ) != std::string::npos )
        {
            MagickSetImageProperty( wand, "dpx:film.frame_rate", buf );
            MagickSetImageProperty( wand, "dpx:television.frame_rate", buf );
        }

        setlocale( LC_NUMERIC, "" );  // Return to our locale

        //sprintf( buf, "%2.4f", _gamma );
        //MagickSetImageProperty( wand, "dpx:television.gamma", buf );
    }

    /**
     * Write out image layer(s)
     *
     */
    status = MagickWriteImages( wand, file, MagickTrue );
    if ( status == MagickFalse )
        ThrowWandException( wand );

    destroyPixels(bufs);
    DestroyMagickWand( wand );

    if (status == MagickFalse)
    {
        ThrowWandException( wand );
    }

    p->channel( old_channel );

    return true;
}


} // namespace mrv
