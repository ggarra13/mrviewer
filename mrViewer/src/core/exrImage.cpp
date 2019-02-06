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
 * @file   exrImage.cpp
 * @author gga
 * @date   Fri Jul 20 04:54:04 2007
 *
 * @brief  Class used to load and save ILM's OpenEXR images
 *
 *
 */


#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>       // for quietNaN

#include <Iex.h>
#include <ImfVersion.h> // for MAGIC
#include <ImfChannelList.h>
#include <ImfPartType.h>
#include <ImfInputFile.h>
#include <ImfInputPart.h>
#include <ImfTiledInputFile.h>
#include <ImfStandardAttributes.h>
#include <ImfDeepScanLineInputPart.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfCompositeDeepScanLine.h>
#include <ImfTiledInputPart.h>
#include <ImfTiledOutputPart.h>
#include <ImfDeepScanLineOutputPart.h>
#include <ImfDeepTiledOutputPart.h>
#include <ImfDeepCompositing.h>
#include <ImfDeepTiledInputPart.h>
#include <ImfOutputFile.h>
#include <ImathMath.h> // for Math:: functions
#include <ImathBox.h>  // for Box2i
#include <ImfIntAttribute.h>
#include <ImfVecAttribute.h>
#include <ImfKeyCodeAttribute.h>
#include <ImfTimeCodeAttribute.h>
#include <ImfFramesPerSecond.h>
#include <ImfRgbaYca.h>

#include "core/mrvACES.h"
#include "core/mrvThread.h"
#include "core/Sequence.h"
#include "core/exrImage.h"
#include "core/mrvImageOpts.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvProgressReport.h"
#include "gui/mrvIO.h"

using namespace Imf;
using namespace Imath;
using namespace std;

#define USE_HASH
#define USE_ALPHA
#define CHANGE_PERIODS_TO_UNDERSCORES

namespace
{
const char* kModule = "exr";
}





namespace mrv {

float exrImage::_default_gamma = 2.2f;
Imf::Compression exrImage::_default_compression = Imf::PIZ_COMPRESSION;
float exrImage::_default_dwa_compression = 45.0f;

static stringSet ignore;

void fill_ignore_attr()
{
    ignore.insert( "CameraFilmApertureHorizontal" );
    ignore.insert( "CameraFilmApertureVertical" );
    ignore.insert( "CameraFocalLength" );
    ignore.insert( "channels" );
    ignore.insert( "compression" );
    ignore.insert( "dwaCompressionLevel" );
    ignore.insert( "chunkCount" );
    ignore.insert( "dataWindow" );
    ignore.insert( "displayWindow" );
    ignore.insert( "framesPerSecond" );
    ignore.insert( "lineOrder" );
    ignore.insert( "pixelAspectRatio" );
    ignore.insert( "screenWindowCenter" );
    ignore.insert( "screenWindowWidth" );
    ignore.insert( "name" );
    ignore.insert( "type" );
    ignore.insert( "version" );
    ignore.insert( "view" );
    // ignore.insert( "worldToCamera" );
    // ignore.insert( "worldToNDC" );
}


const char* exrImage::kCompression[] = {
    _("None"),
    _("RLE"),
    N_("Zips"),
    N_("Zip"),
    N_("Piz"),
    N_("Pxr24"),
    N_("B44"),
    N_("B44A"),
    N_("DWAA"),
    N_("DWAB"),
    0
};

const char* exrImage::kLineOrder[] = {
    _("Increasing Y"),
    _("Decreasing Y"),
    _("Random Y"),
    0
};

inline image_type::PixelType
exrImage::pixel_type_conversion( Imf::PixelType pixel_type )
{
    switch( pixel_type )
    {
    case Imf::UINT:
        return image_type::kInt;
    case Imf::HALF:
        return image_type::kHalf;
    case Imf::FLOAT:
        return image_type::kFloat;
    default:
        LOG_ERROR( _("Unknown Imf::PixelType") );
        return image_type::kFloat;
    }
}

inline  Imf::PixelType
exrImage::pixel_type_to_exr( image_type::PixelType pixel_type )
{
    switch( pixel_type )
    {
    case image_type::kByte:
    case image_type::kShort:
        return Imf::NUM_PIXELTYPES;
    case image_type::kInt:
        return Imf::UINT;
    case image_type::kHalf:
        return Imf::HALF;
    case image_type::kFloat:
        return Imf::FLOAT;
    default:
        LOG_ERROR("Unknown image_type::PixelType " << pixel_type);
        return Imf::HALF;
    }
}

exrImage::exrImage() :
    CMedia(),
    _levelX( 0 ),
    _levelY( 0 ),
    _multiview( false ),
    _has_alpha( false ),
    _has_yca( false ),
    _use_yca( false ),
    _has_left_eye( NULL ),
    _has_right_eye( NULL ),
    _curpart( -1 ),
    _clear_part( 0 ),
    _numparts( -1 ),
    _read_attr( false ),
    _lineOrder( (Imf::LineOrder) 0 ),
    _compression( (Imf::Compression) 0 ),
    _aces( false )
{
    st[0] = st[1] = -1;

    if ( ignore.empty() ) fill_ignore_attr();
}

exrImage::~exrImage()
{
    free( _has_right_eye );
    free( _has_left_eye );
}

stringArray exrImage::valid_compressions() const
{
    stringArray ret;
    const char** s = kCompression;
    for ( ; *s != 0; ++s )
    {
        ret.push_back( *s );
    }
    return ret;
}

/*! Test a block of data read from the start of the file to see if it
  looks like the start of an .exr file. This returns true if the
  data contains EXR's magic number and a "channels" string in the 8th
  position.
*/
bool exrImage::test(const boost::uint8_t *data, unsigned)
{
    if ( Imf::isImfMagic( (const char*)data ) ) return true;
    return false;
}


bool exrImage::channels_order(
    const boost::int64_t& frame,
    Imf::ChannelList::ConstIterator& s,
    Imf::ChannelList::ConstIterator& e,
    const Imf::ChannelList& channels,
    const Imf::Header& h,
    Imf::FrameBuffer& fb
)
{
    const Box2i& displayWindow = h.displayWindow();
    const Box2i& dataWindow = h.dataWindow();
    int dw = dataWindow.max.x - dataWindow.min.x + 1;
    int dh = dataWindow.max.y - dataWindow.min.y + 1;
    int dx = dataWindow.min.x;
    int dy = dataWindow.min.y;

    // First, count and store the channels
    bool no_layer = false;
    order[0] = order[1] = order[2] = order[3] = -1;

    Imf::PixelType imfPixelType = Imf::UINT;

    typedef std::vector< std::string > LayerList;
    LayerList channelList;
    channelList.reserve(5); // R,G,B,A,Z

    int idx = 0;
    int xsampling[4], ysampling[4];

    bool Zchannel = false;
    std::string c;
    if ( channel() ) c = channel();
    std::string ext = c;
    size_t pos = ext.rfind( '.' );
    if ( pos != std::string::npos )
    {
        ext = ext.substr( pos+1, ext.size() );
    }
    if ( ext == "Z" ) Zchannel = true;

    for (Imf::ChannelList::ConstIterator i = s; i != e; ++i, ++idx )
    {
        const std::string& layerName = i.name();
        const Imf::Channel& ch = i.channel();

        ext = layerName;

        if ( no_layer == false )
        {
            pos = ext.rfind( '.' );
            if ( pos != std::string::npos )
            {
                ext = ext.substr( pos+1, ext.size() );
            }
            else
            {
                no_layer = true;
            }
        }

        std::transform( ext.begin(), ext.end(), ext.begin(),
                        (int(*)(int)) toupper );

        if ( order[0] == -1 && (( ext == N_("R") && Zchannel == false) ||
                                ext == N_("Y") || ext == N_("U") ||
                                ext == N_("X") || ext == N_("Z")) )
        {
            int k = order[0] = (int)channelList.size();
            imfPixelType = ch.type;
            xsampling[k] = ch.xSampling;
            ysampling[k] = ch.ySampling;
            channelList.push_back( layerName );
        }
        else if ( order[1] == -1 && ((ext == N_("G") && Zchannel == false) ||
                                     ext == N_("RY") || ext == N_("V") ||
                                     ext == N_("Y") ) )
        {
            int k = order[1] = (int)channelList.size();
            imfPixelType = ch.type;
            xsampling[k] = ch.xSampling;
            ysampling[k] = ch.ySampling;
            channelList.push_back( layerName );
        }
        else if ( order[2] == -1 && ((ext == N_("B") && Zchannel == false) ||
                                     ext == N_("BY") || ext == N_("W") ||
                                     ext == N_("Z") ) )
        {
            int k = order[2] = (int)channelList.size();
            imfPixelType = ch.type;
            xsampling[k] = ch.xSampling;
            ysampling[k] = ch.ySampling;
            channelList.push_back( layerName );
        }
        else if ( order[3] == -1 && (ext == N_("A") && Zchannel == false) )
        {
            int k = order[3] = (int)channelList.size();
            imfPixelType = ch.type;
            xsampling[k] = ch.xSampling;
            ysampling[k] = ch.ySampling;
            channelList.push_back( layerName );
            _has_alpha = true;
        }
        else if ( Zchannel == false &&
                  order[0] == -1 && order[1] == -1 && order[2] == -1 &&
                  order[3] == -1 && (no_layer || ext.size() > 1) )
        {
            int k = order[0] = (int) channelList.size();
            imfPixelType = ch.type;
            xsampling[k] = ch.xSampling;
            ysampling[k] = ch.ySampling;
            channelList.push_back( layerName );
        }

    }


    size_t numChannels = channelList.size();

    if ( numChannels == 0 )
    {
        if ( channel() )
            IMG_ERROR( _("Image file \"") << filename() <<
                       _("\" has no channels named with prefix \"")
                       << channel() << "\"." );
        else
        {
            IMG_ERROR( _("Image file \"") << filename() <<
                       _("\" has no channels.") );
        }
        return false;
    }


    // Prepare format
    image_type::Format format = VideoFrame::kLumma;
    int offsets[4];
    if (order[0] != -1 ) offsets[order[0]] = 0;

    unsigned sx = 1, sy = 1;

    if ( _has_yca )
    {
        unsigned size  = dw * dh;
        unsigned size2 = dw * dh / 4;

        unsigned off = 0;
        sx = 0;
        unsigned short idx = 0;
        for ( int i = 0; i < 4; ++i )
        {
            int k = order[i];
            if ( k == -1 ) continue;

            if ( idx == 0 ) off = 0;
            else if ( idx == 1 ) off = size;
            else if ( idx == 2 ) off = size + size2;
            else if ( idx == 3 ) off = size + size2 * 2;
            if ( sx == 0 ) {
                sx = xsampling[k];
                sy = ysampling[k];
            }
            offsets[k] = off;
            ++idx;
        }
        if ( numChannels >= 3 && has_alpha() )
        {
            format = VideoFrame::kYByRy420A;
            numChannels = 4;
            offsets[order[3]]  = size + size2 * 2;
        }
        else if ( numChannels >= 2 )
        {
            numChannels = 3;
            format = VideoFrame::kYByRy420;
        }
    }
    else
    {
        if ( order[1] != -1 ) offsets[order[1]]  = 1;
        if ( order[2] != -1 ) offsets[order[2]]  = 2;
        if ( order[3] != -1 ) offsets[order[3]]  = 3;

        if ( numChannels >= 3 && has_alpha() )
        {
            format = VideoFrame::kRGBA;
            numChannels = 4;
        }
        else if ( numChannels >= 2 )
        {
            format = VideoFrame::kRGB;
            numChannels = 3;
        }
    }

    if ( ! allocate_pixels( frame, (unsigned short)numChannels, format,
                            pixel_type_conversion( imfPixelType ),
                            dw / sx, dh / sy ) )
        return false;

    size_t xs[4], ys[4];

    if ( _has_yca )
    {
        size_t t = _hires->pixel_size();

        for ( unsigned j = 0; j < 4; ++j )
        {
            xs[j] = t;
        }

        size_t dw2 = dw / 2;
        if ( order[0] != -1 ) ys[order[0]] = t * dw;
        if ( order[1] != -1 ) ys[order[1]] = t * dw2;
        if ( order[2] != -1 ) ys[order[2]] = t * dw2;
        if ( order[3] != -1 ) ys[order[3]] = t * dw;
    }
    else
    {
        unsigned pixels = (unsigned) (_hires->pixel_size() * numChannels);

        for ( unsigned j = 0; j < 4; ++j )
        {
            int k = order[j];
            if ( k == -1 ) continue;
            xs[k] = pixels;
            ys[k] = pixels * dw;
        }
    }

    char* pixels = (char*)_hires->data().get();
    if (!pixels) return false;
    memset( pixels, 0, _hires->data_size() ); // Needed for lumma pics (Fog.exr)

    // Then, prepare frame buffer for them
    int start = ( (-dx - dy * dw) * _hires->pixel_size() *
                  _hires->channels() );

    char* base = pixels + start;

    for ( int idx = 0; idx < 4; ++idx )
    {
        int k = order[idx];
        if ( k == -1 ) continue;

        char* buf = base + offsets[k] * _hires->pixel_size();

        // std::cerr << "LOAD " << idx << ") " << k << " " << channelList[k]
        //           << " off:" << offsets[k] << " xs,ys "
        //                << std::endl;
        //           << xs[k] << "," << ys[k]
        //           << " sampling " << xsampling[k]
        //           << " " << ysampling[k]
        //           << " buf " << (void*) buf
        //           << std::endl;


        fb.insert( channelList[k],
                   Slice( imfPixelType, buf, xs[k], ys[k],
                          xsampling[k], ysampling[k] ) );
    }

    return true;
}


void exrImage::ycc2rgba( const Imf::Header& hdr, const boost::int64_t& frame )
{

    Imf::Chromaticities cr;
    if ( Imf::hasChromaticities( hdr ) )
        cr = Imf::chromaticities( hdr );

    V3f yw = Imf::RgbaYca::computeYw(cr);

    unsigned w = width();
    unsigned h = height();

    image_type::Format format = ( has_alpha() ?
                                  image_type::kRGBA :
                                  image_type::kRGB );

    mrv::image_type_ptr rgba( new image_type( frame, w, h,
                              (unsigned short)
                              (3 + 1*has_alpha()),
                              format,
                              image_type::kFloat ) );


    for ( unsigned y = 0; y < h; ++y )
    {
        for ( unsigned x = 0; x < w; ++x )
        {
            CMedia::Pixel p = _hires->pixel( x, y );
            rgba->pixel( x, y, p );
        }
    }

    _hires = rgba;
}

/**
 * Fetch the current EXR image
 *
 * @return true if success, false if not
 */
bool exrImage::fetch_mipmap( const boost::int64_t& frame )
{

    try {


        std::string fileName = sequence_filename(frame);

        TiledInputFile in( fileName.c_str() );

        int numXLevels = in.numXLevels();
        int numYLevels = in.numYLevels();
        if (_levelX > numXLevels-1 ) _levelX = numXLevels-1;
        if (_levelY > numYLevels-1 ) _levelY = numYLevels-1;

        if (!in.isValidLevel (_levelX, _levelY))
        {
            THROW (Iex::InputExc, "Level (" << _levelX << ", "
                   << _levelY << ") does "
                   "not exist in file " << fileName << ".");
        }

        Imf::Header h = in.header();
        h.dataWindow() = in.dataWindowForLevel(_levelX, _levelY);
        h.displayWindow() = h.dataWindow();

        const Imath::Box2i& dataWindow = h.dataWindow();
        const Imath::Box2i& displayWindow = h.displayWindow();
        data_window( dataWindow.min.x, dataWindow.min.y,
                     dataWindow.max.x, dataWindow.max.y, frame );

        display_window( displayWindow.min.x, displayWindow.min.y,
                        displayWindow.max.x, displayWindow.max.y, frame );


        read_forced_header_attr( h, frame );

        if ( ! _read_attr )
            read_header_attr( h, frame );


        FrameBuffer fb;
        bool ok = find_channels( h, fb, frame );
        if ( !ok ) return false;

        _pixel_ratio = h.pixelAspectRatio();
        _lineOrder   = h.lineOrder();
        _compression = h.compression();

        in.setFrameBuffer(fb);


        int tx = in.numXTiles( _levelX );
        int ty = in.numYTiles( _levelY );

        //
        // For maximum speed, try to read the tiles in
        // the same order as they are stored in the file.
        //

        if ( _lineOrder == INCREASING_Y )
        {
            for (int y = 0; y < ty; ++y)
                for (int x = 0; x < tx; ++x)
                    in.readTile (x, y, _levelX, _levelY);
        }
        else
        {
            for (int y = ty - 1; y >= 0; --y)
                for (int x = 0; x < tx; ++x)
                    in.readTile (x, y, _levelX, _levelY);
        }

        return true;

    }
    catch( const std::exception& e )
    {
        IMG_ERROR( e.what() );
        _curpart = -1;
        return false;
    }
}

bool exrImage::find_layers( const Imf::Header& h )
{


    const Imf::ChannelList& channels = h.channels();
    if ( layers.empty() || _stereo_output != kNoStereo && _multiview )
    {
        channels.layers( layers );

        free( _has_left_eye );
        _has_left_eye = NULL;
        free( _has_right_eye );
        _has_right_eye = NULL;

        int rightView = -1;
        std::string left = get_long_view( true );
        std::string right = get_long_view( false );
        std::string L = get_short_view( true );
        std::string R = get_short_view( false );

        if ( channel() != NULL )
        {

            std::string prefix, suffix;
            std::string c = channel();
            std::string ch = c;
            if ( c[0] == '#' )
            {
                size_t idx = c.find( ' ' );
                if ( idx != std::string::npos )
                {
                    c = c.substr( idx+1, c.size() );
                }
            }

            std::string match, discard;
            size_t idx = c.find( left );
            if ( idx != std::string::npos )
            {
                _has_left_eye = strdup( c.c_str() );
                rightView = 1;
                match = right;
                discard = left;
                prefix = c.substr( 0, idx );
                suffix = c.substr( idx + left.size(), c.size() );
            }
            else
            {
                idx = c.find( L );
                if ( idx != std::string::npos )
                {
                    _has_left_eye = strdup( c.c_str() );
                    rightView = 1;
                    discard = L;
                    match = R;
                    prefix = c.substr( 0, idx );
                    suffix = c.substr( idx + L.size(), c.size() );
                }
                else
                {
                    idx = c.find( right );
                    if ( idx != std::string::npos )
                    {
                        _has_right_eye = strdup( c.c_str() );
                        rightView = 0;
                        discard = right;
                        match = left;
                        prefix = c.substr( 0, idx );
                        suffix = c.substr( idx + right.size(), c.size() );
                    }
                    else
                    {
                        idx = c.find( R );
                        if ( idx != std::string::npos )
                        {
                            _has_right_eye = strdup( c.c_str() );
                            rightView = 0;
                            discard = R;
                            match = L;
                            prefix = c.substr( 0, idx );
                            suffix = c.substr( idx + R.size(), c.size() );
                        }
                    }
                }
            }

            stringSet::const_iterator i = layers.begin();
            stringSet::const_iterator e = layers.end();

            for ( ; i != e; ++i )
            {
                const std::string& layer = *i;

                idx = layer.find( discard );
                if ( idx != std::string::npos ) {
                    continue;
                }


                if ( !prefix.empty() )
                {
                    idx = layer.find( prefix );
                    if ( idx == std::string::npos ) {
                        continue;
                    }
                }
                if ( !suffix.empty() )
                {
                    idx = layer.rfind( suffix );
                    if ( idx == std::string::npos ) {
                        continue;
                    }
                }

                if ( prefix.empty() && suffix.empty() )
                    continue;

                if ( rightView == 1 )
                {
                    _has_right_eye = strdup( layer.c_str() );
                }
                else if ( rightView == 0 )
                {
                    _has_left_eye = strdup( layer.c_str() );
                }
            }
        }

        if ( rightView == -1 )
        {
            stringSet::const_iterator i = layers.begin();
            stringSet::const_iterator e = layers.end();

            std::string c;
            if ( channel() ) c = channel();

            if ( c.empty() || c == "Z" )
            {
                for ( ; i != e; ++i )
                {
                    const std::string& layer = *i;
                    if ( !_has_right_eye &&
                            ( layer.find( right ) == 0 &&
                              layer.size() == right.size() ) )
                        _has_right_eye = strdup( layer.c_str() );

                    if ( !_has_left_eye &&
                            ( layer.find( left ) == 0 ) &&
                            layer.size() == left.size() )
                    {
                        _has_left_eye = strdup( layer.c_str() );
                    }
                }
            }
            else
            {
                for ( ; i != e; ++i )
                {
                    const std::string& layer = *i;
                    if ( !_has_right_eye &&
                            ( layer.find( right ) != std::string::npos ||
                              layer.find( R ) != std::string::npos ) )
                        _has_right_eye = strdup( layer.c_str() );

                    if ( !_has_left_eye &&
                            ( layer.find( left ) != std::string::npos ||
                              layer.find( L ) != std::string::npos ) )
                    {
                        _has_left_eye = strdup( layer.c_str() );
                    }
                }
            }
        }
        // std::cerr << "_has_left_eye "
        //           << ( _has_left_eye ? _has_left_eye : "NULL" ) << std::endl;
        // std::cerr << "_has_right_eye "
        //           << ( _has_right_eye ? _has_right_eye : "NULL" )
        //           << std::endl;

        if ( !_is_stereo && ( _has_left_eye || _has_right_eye ) )
        {
            _multiview = true;
            _is_stereo = true;
            _stereo_input = kSeparateLayersInput;
        }
    }

    if ( _num_channels == 0 )
    {
        _gamma = _default_gamma;

        bool has_rgb = false;
        _has_alpha = false;
        if ( channels.findChannel( N_("R") ) ||
                channels.findChannel( N_("G") ) ||
                channels.findChannel( N_("B") ) )
        {
            has_rgb = true;
            rgb_layers();
            lumma_layers();
        }

        _has_yca = false;
        _use_yca = false;
        if ( !has_rgb )
        {
            if ( channels.findChannel( N_("Y") ) )
            {
                _layers.push_back( N_("Y") );
                ++_num_channels;
            }
            if ( channels.findChannel( N_("RY") ) )
            {
                _layers.push_back( N_("RY") );
                ++_num_channels;
            }
            if ( channels.findChannel( N_("BY") ) )
            {
                _layers.push_back( N_("BY") );
                ++_num_channels;
            }

            if ( _num_channels != 0 )
            {
                _has_yca = true;
                _use_yca = true;
                rgb_layers();
                _num_channels = (unsigned short) ( _num_channels - 3 );
                lumma_layers();
            }
        }

        if ( channels.findChannel( N_("A") ) )
        {
            _has_alpha = true;
            alpha_layers();
        }



        Imf::ChannelList::ConstIterator i = channels.begin();
        Imf::ChannelList::ConstIterator e = channels.end();

        // Deal with single channels first, like Tag, Z Depth, etc.
        for ( ; i != e; ++i )
        {
            const std::string& layerName = i.name();
            std::string name( layerName );
            // Make names all uppercase, to avoid confusion
            std::transform(name.begin(), name.end(), name.begin(),
                           (int(*)(int)) toupper);
            if ( name == N_("R") || name == N_("G") || name == N_("B") ||
                    name == N_("A") || name == N_("Y") || name == N_("BY") ||
                    name == N_("RY") )
                continue; // these channels are handled in shader directly

            // Don't count layer.channel those are handled later
            if ( name.find( N_(".") ) != string::npos ) continue;

            _layers.push_back( layerName );
            ++_num_channels;

        }


        // Deal with layers next like (Normals, Motion, etc)
        {

            stringSet::const_iterator i = layers.begin();
            stringSet::const_iterator e = layers.end();
            for ( ; i != e; ++i )
            {
                const std::string& name = *i;
                _layers.push_back( name );
                ++_num_channels;

                Imf::ChannelList::ConstIterator x;
                Imf::ChannelList::ConstIterator s;
                Imf::ChannelList::ConstIterator e;
                channels.channelsInLayer( name, s, e );
                stringArray lg;
                lg.reserve(5);
                for ( x = s; x != e; ++x )
                {
                    const std::string& layerName = x.name();
                    lg.push_back( layerName );
                }
                stringArray::iterator ks = lg.begin();
                stringArray::iterator ke = lg.end();
                std::string lcase = *(ks + (lg.size() > 1));
                if ( lcase.rfind( ".g" ) != std::string::npos ||
                        lcase.rfind( ".G" ) != std::string::npos ||
                        lcase.rfind( ".B" ) != std::string::npos ||
                        lcase.rfind( ".b" ) != std::string::npos )
                {
                    std::sort( ks, ke, std::greater<std::string>() );
                }

                for ( ; ks != ke; ++ks )
                {
                    _layers.push_back( *ks );
                    ++_num_channels;
                }
            }
        }
    }

    const Box2i& dataWindow = h.dataWindow();
    int dw  = dataWindow.max.x - dataWindow.min.x + 1;
    int dh  = dataWindow.max.y - dataWindow.min.y + 1;

    image_size( dw, dh );

    return true;
}

bool exrImage::handle_stereo( const boost::int64_t& frame,
                              const Imf::Header& h,
                              Imf::FrameBuffer& fb )
{
    const Imf::ChannelList& channels = h.channels();

    Imf::ChannelList::ConstIterator s;
    Imf::ChannelList::ConstIterator e;
    std::string prefix;
    if ( _has_right_eye ) prefix = _has_right_eye;


    // Find the iterators for a right channel prefix or all channels
    if ( !prefix.empty() )
    {
        channels.channelsWithPrefix( prefix, s, e );
        if ( s == e )
        {
            s = channels.begin();
            e = channels.end();
        }
    }
    else
    {
        s = channels.begin();
        e = channels.end();
    }
    bool ok = channels_order( frame, s, e, channels, h, fb );

    // If 3d is because of different headers exit now
    if ( !_multiview )
    {
        return ok;
    }

    _stereo[1] = _hires;

    //
    // Repeat for left eye
    //
    prefix.clear();
    if ( _has_left_eye ) prefix = _has_left_eye;

    if ( !prefix.empty() )
    {
        channels.channelsWithPrefix( prefix, s, e );
        if ( s == e )
        {
            s = channels.begin();
            e = channels.end();
        }
    }
    else
    {
        s = channels.begin();
        e = channels.end();
    }

    ok = channels_order( frame, s, e, channels, h, fb );
    _stereo[0] = _hires;

    return ok;
}

bool exrImage::find_channels( const Imf::Header& h,
                              Imf::FrameBuffer& fb,
                              const boost::int64_t& frame )
{
    bool ok = find_layers( h );
    if ( !ok ) return false;

    const Imf::ChannelList& channels = h.channels();

    char* channelPrefix = NULL;
    if ( _channel ) channelPrefix = strdup( _channel );


    // If channel starts with #, we are dealing with a multipart exr

    if ( channelPrefix && channelPrefix[0] == '#' )
    {
        _has_alpha = false;
        std::string part = channelPrefix;

        size_t idx = part.find( '.' );

        if ( idx == std::string::npos )
        {
            free( channelPrefix );
            channelPrefix = NULL;
        }
        else
        {
            std::string ext = part.substr( idx+1, part.size() );
            std::string root = ext;

            idx = ext.rfind( '.' );
            if ( idx != std::string::npos )
            {
                ext = ext.substr( idx+1, ext.size() );
            }

            if ( ext == "z" ) ext = "B";  // so it gets removed later

            std::transform( ext.begin(), ext.end(), ext.begin(),
                            (int(*)(int)) toupper );


            // When extension is one of RGBA, XYZ or UVW we want to
            // read all channels together so we remove the extension
            // from prefix.
            if ( ext == "R" || ext == "G" || ext == "B" || ext == "A" ||
                    ext == "X" || ext == "Y" || ext == "U" ||
                    ext == "V" || ext == "W" )
            {
                ext = "";
                if ( idx == std::string::npos || idx == 1 )
                    root = "";
                else
                    root = root.substr( 0, idx );
            }

            if ( !root.empty() )
            {
                free( channelPrefix );
                channelPrefix = strdup( root.c_str() );
            }
        }
    }

    if ( _stereo_output != kNoStereo )
    {
        free( channelPrefix );
        channelPrefix = NULL;
        return handle_stereo(frame, h, fb);
    }
    else if ( channelPrefix != NULL )
    {
        Imf::ChannelList::ConstIterator s;
        Imf::ChannelList::ConstIterator e;
        std::string prefix = channelPrefix;
        size_t pos = prefix.rfind( '.' );
        std::string ext = prefix;
        if ( pos != std::string::npos )
        {
            ext = prefix.substr( pos+1, prefix.size() );
            if ( ext == "z" ) prefix = prefix.substr(0, pos);
        }

        std::transform( ext.begin(), ext.end(), ext.begin(),
                        (int(*)(int)) toupper );


        // If extension is one of a group, load all channels
        if ( ext == "A" || ext == "R" || ext == "G" ||
                ext == "B" || ext == "X" || ext == "Y" ||
                ext == "U" || ext == "V" ||
                ext == "W" ) prefix = prefix.substr(0, pos);

        channels.channelsWithPrefix( prefix, s, e );
        if ( s == e )
        {
            s = channels.begin();
            e = channels.end();
        }

        free( channelPrefix );
        channelPrefix = NULL;

        return channels_order( frame, s, e, channels, h, fb );
    }
    else
    {
        Imf::ChannelList::ConstIterator s = channels.begin();
        Imf::ChannelList::ConstIterator e = channels.end();
        return channels_order( frame, s, e, channels, h, fb );
    }
}

void exrImage::read_forced_header_attr( const Imf::Header& h,
                                        const boost::int64_t& frame )
{

    const Imf::StringAttribute *attr =
        h.findTypedAttribute<Imf::StringAttribute>( N_("capDate") );
    if ( attr )
    {
        _cap_date[frame] = attr->value();
    }

}

void exrImage::read_header_attr( const Imf::Header& h,
                                 const boost::int64_t& frame )
{
    _read_attr = true;

    stringSet attrs;
    attrs.insert( N_("type") );
    attrs.insert( N_("capDate") );

    {
        const Imf::RationalAttribute* attr =
            h.findTypedAttribute<Imf::RationalAttribute>("framesPerSecond");
        if ( attr )
        {
            Imf::Rational r = attr->value();
            _fps = (double) r.n / (double) r.d;
        }

        if ( _play_fps <= 0 ) _orig_fps = _play_fps = _fps.load();
    }


    {
        const Imf::IntAttribute *attr =
            h.findTypedAttribute<Imf::IntAttribute>( N_("acesImageContainerFlag") );
        if ( attr )
        {
            _aces = (bool)attr->value();
            attrs.insert( N_("acesImageContainerFlag") );
        }
    }

    {
        const Imf::StringAttribute* attr =
            h.findTypedAttribute<Imf::StringAttribute>( N_("inputColorSpace") );
        if ( attr && !is_thumbnail() )
        {
            ocio_input_color_space( attr->value() );
            image_damage( image_damage() | kDamageLut );
        }
    }

    {
        const Imf::ChromaticitiesAttribute *attr =
            h.findTypedAttribute<Imf::ChromaticitiesAttribute>( N_("chromaticities") );
        if ( attr )
        {
            chromaticities( attr->value() );
            image_damage( image_damage() | kDamageLut );
        }
    }

    {
        const Imf::StringAttribute *attr =
            h.findTypedAttribute<Imf::StringAttribute>( N_("chromaticitiesName") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Chromaticities Name"),
                                           attr->copy() ) );
            attrs.insert( N_("chromaticitiesName") );
        }
    }

    {
        const Imf::V2fAttribute *attr =
            h.findTypedAttribute<Imf::V2fAttribute>( N_("adoptedNeutral") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Adopted Neutral"),
                                           attr->copy() ) );
            attrs.insert( N_("adoptedNeutral") );
            image_damage( image_damage() | kDamageLut );
        }
    }

    {
        const Imf::IntAttribute *attr =
            h.findTypedAttribute<Imf::IntAttribute>( N_("imageState") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Image State"),
                                           attr->copy() ) );
            attrs.insert( N_("imageState") );
        }
    }

    {
        const Imf::StringAttribute *attr =
            h.findTypedAttribute<Imf::StringAttribute>( N_("owner") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Owner"), attr->copy() ) );
            attrs.insert( N_("owner") );
        }
    }

    {
        const Imf::StringAttribute *attr =
            h.findTypedAttribute<Imf::StringAttribute>(  N_("comments") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Comments"),
                                           attr->copy() ) );
            attrs.insert( N_("comments") );
        }
    }

    {
        const Imf::FloatAttribute *attr =
            h.findTypedAttribute<Imf::FloatAttribute>( N_("utcOffset") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("UTC Offset"), attr->copy() ) );
            attrs.insert( N_("utcOffset") );
        }
    }

    {
        const Imf::FloatAttribute *attr =
            h.findTypedAttribute<Imf::FloatAttribute>( N_("longitude") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Longitude"), attr->copy() ) );
            attrs.insert( N_("longitude") );
        }
    }

    {
        const Imf::FloatAttribute *attr =
            h.findTypedAttribute<Imf::FloatAttribute>( N_("latitude") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Latitude"), attr->copy() ) );
            attrs.insert( N_("latitude") );
        }
    }

    {
        const Imf::FloatAttribute *attr =
            h.findTypedAttribute<Imf::FloatAttribute>( N_("altitude") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Altitude"), attr->copy() ) );
            attrs.insert( N_("altitude") );
        }
    }

    {
        const Imf::FloatAttribute *attr =
            h.findTypedAttribute<Imf::FloatAttribute>( N_("focus") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Focus"), attr->copy() ) );
            attrs.insert( N_("focus") );
        }
    }

    {
        const Imf::FloatAttribute *attr =
            h.findTypedAttribute<Imf::FloatAttribute>( N_("expTime") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Exposure Time"), attr->copy() ) );
            attrs.insert( N_("expTime") );
        }
    }

    {
        const Imf::FloatAttribute *attr =
            h.findTypedAttribute<Imf::FloatAttribute>( N_("aperture") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Aperture"), attr->copy() ) );
            attrs.insert( N_("aperture") );
        }
    }


    {
        const Imf::FloatAttribute *attr =
            h.findTypedAttribute<Imf::FloatAttribute>( N_("isoSpeed") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("ISO Speed"), attr->copy() ) );
            attrs.insert( N_("isoSpeed") );
        }
    }


    {
        const Imf::KeyCodeAttribute* attr =
            h.findTypedAttribute<Imf::KeyCodeAttribute>( N_("keyCode") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( N_("keyCode"), attr->copy() ) );
            attrs.insert( N_("keyCode") );
        }
    }

    {
        const Imf::TimeCodeAttribute *attr =
            h.findTypedAttribute<Imf::TimeCodeAttribute>( N_("timeCode") );
        if ( attr )
        {
            process_timecode( attr->value() );
            _attrs.insert( std::make_pair( N_("timecode"), attr->copy() ));
            attrs.insert( N_("timeCode") );
        }
    }

    {
        const Imf::StringAttribute *attr =
            h.findTypedAttribute<Imf::StringAttribute>( N_("writer") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Writer"), attr->copy() ));
            attrs.insert( N_("writer") );
        }
    }

    {
        const Imf::StringAttribute *attr =
            h.findTypedAttribute<Imf::StringAttribute>( N_("iccProfile") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("ICC Profile"), attr->copy() ) );
            attrs.insert( N_("iccProfile") );
        }
    }

    {
        const Imf::StringAttribute *attr =
            h.findTypedAttribute<Imf::StringAttribute>( N_("wrapmodes") );
        if ( attr )
        {
            _attrs.insert( std::make_pair( _("Wrap Modes"), attr->copy() ) );
            attrs.insert( N_("wrapmodes") );
        }
    }

    if ( h.hasTileDescription() )
    {
        const Imf::TileDescription& desc = h.tileDescription();
        char buf[128];

        switch( desc.mode )
        {
        case Imf::ONE_LEVEL:
            break;
        case Imf::MIPMAP_LEVELS:
        {
            TiledInputFile tin( sequence_filename(frame).c_str() );
            Imf::IntAttribute attr( tin.numLevels() );
            _attrs.insert( std::make_pair( _("Mipmap Levels"),
                                           attr.copy() ) );
            break;
        }
        case Imf::RIPMAP_LEVELS:
        {
            TiledInputFile tin( sequence_filename(frame).c_str() );
            Imf::IntAttribute xat( tin.numXLevels() );
            Imf::IntAttribute yat( tin.numYLevels() );
            _attrs.insert( std::make_pair( _("X Ripmap Levels"),
                                           xat.copy() ));
            _attrs.insert( std::make_pair( _("Y Ripmap Levels"),
                                           yat.copy() ));
            break;
        }
        default:
            IMG_ERROR("Unknown mipmap mode");
            break;
        }

        switch( desc.roundingMode )
        {
        case Imf::ROUND_DOWN:
            _attrs.insert( std::make_pair( _("Rounding Mode"),
                                           new Imf::StringAttribute( _("Down") ) ) );
            break;
        case Imf::ROUND_UP:
            _attrs.insert( std::make_pair( _("Rounding Mode"),
                                           new Imf::StringAttribute( _("Up") ) ) );
            break;
        default:
            IMG_ERROR( _("Unknown rounding mode") );
            break;
        }
    }

    Imf::Header::ConstIterator i = h.begin();
    Imf::Header::ConstIterator e = h.end();

    for ( ; i != e; ++i )
    {
        const std::string& name = i.name();
        if ( attrs.find( name ) != attrs.end() ||
                _attrs.find( name ) != _attrs.end() ||
                ignore.find( name ) != ignore.end() ) continue;

        const Attribute& attr = i.attribute();
        _attrs.insert( std::make_pair( name, attr.copy() ) );
    }
}


void exrImage::findZBound( float& zmin, float& zmax, float farPlane,
                           int zsize,
                           Imf::Array<float*>&       zbuff,
                           Imf::Array<unsigned int>& sampleCount )
{
    //
    // find zmax and zmin values of deep data to set bound
    //
    zmax = limits<float>::min();
    zmin = limits<float>::max();

    unsigned maxCount = 0;

    for (int k = 0; k < zsize; k++)
    {
        float* z = zbuff[k];
        unsigned int count = sampleCount[k];

        if (maxCount < count)
            maxCount = count;

        for (unsigned int i = 0; i < count; i++)
        {
            double val = double(z[i]);
            if (val > zmax && val < farPlane)
                zmax = float(val);
            if (val < zmin)
                zmin = float(val);
        }
    }

    if ( zmax < zmin)
    {
        float tmp = zmax;
        zmax = zmin;
        zmin = tmp;
        // cout << "z max: "<< zmax << ", z min: " << zmin << endl;
        // _chart->bounds (zmin, zmax);
    }

}

void exrImage::loadDeepData( int& zsize,
                             Imf::Array<float*>&       zbuff,
                             Imf::Array<unsigned int>& sampleCount )
{

    assert( _curpart >= 0 );

    try {
        Imf::MultiPartInputFile inmaster( sequence_filename(_frame).c_str() );

        if ( ! inmaster.partComplete( _curpart ) ) return;


        const Imf::Header& h = inmaster.header( _curpart );

        _type = SCANLINEIMAGE;
        if ( h.hasType() ) _type = h.type();

        if ( _type == DEEPSCANLINE )
            loadDeepScanlineImage( inmaster, zsize, zbuff, sampleCount, true );
        else if ( _type == DEEPTILE )
            loadDeepTileImage( inmaster, zsize, zbuff, sampleCount, true );
    }
    catch( const std::exception& e )
    {
        IMG_ERROR( "loadDeepData error: " << e.what() );
    }

}


void
exrImage::loadDeepTileImage( Imf::MultiPartInputFile& inmaster,
                             int &zsize,
                             Imf::Array<float*>&       zbuff,
                             Imf::Array<unsigned int>& sampleCount,
                             bool deepComp )
{
    _has_deep_data = true;

    DeepTiledInputPart in (inmaster, _curpart);
    const Imf::Header& header = in.header();

    const Box2i &dataWindow = header.dataWindow();
    const Box2i &displayWindow = header.displayWindow();
    int dw = dataWindow.max.x - dataWindow.min.x + 1;
    int dh = dataWindow.max.y - dataWindow.min.y + 1;
    int dx = dataWindow.min.x;
    int dy = dataWindow.min.y;

    data_window( dataWindow.min.x, dataWindow.min.y,
                 dataWindow.max.x, dataWindow.max.y, _frame );
    display_window( displayWindow.min.x, displayWindow.min.y,
                    displayWindow.max.x, displayWindow.max.y, _frame );

    if ( !_hires || dw*dh*sizeof(Imf::Rgba) != _hires->data_size() )
    {
        if (! allocate_pixels( _frame, 4, image_type::kRGBA,
                               image_type::kHalf ) )
            return;
    }

    // display black right now
    Imf::Rgba* pixels = (Imf::Rgba*)_hires->data().get();
    if (!pixels) return;

    memset( pixels, 0, _hires->data_size() ); // Needed


    Array< half* > dataR;
    Array< half* > dataG;
    Array< half* > dataB;

    Array< float* > zback;
    Array< half* > alpha;

    zsize = dw * dh;
    zbuff.resizeErase (zsize);
    zback.resizeErase (zsize);
    alpha.resizeErase (zsize);

    dataR.resizeErase (zsize);
    dataG.resizeErase (zsize);
    dataB.resizeErase (zsize);

    sampleCount.resizeErase (zsize);

    int rgbflag = 0;
    int deepCompflag = 0;

    const char* channelPrefix = channel();
    if ( channelPrefix == NULL || strcmp( channelPrefix, "Z" ) != 0 )
    {
        if (header.channels().findChannel ("R"))
        {
            rgbflag = 1;
        }
        else if (header.channels().findChannel ("B"))
        {
            rgbflag = 1;
        }
        else if (header.channels().findChannel ("G"))
        {
            rgbflag = 1;
        }

        if (header.channels().findChannel ("Z") &&
                header.channels().findChannel ("A") &&
                deepComp)
        {
            deepCompflag = 1;
        }
    }

    DeepFrameBuffer fb;

    fb.insertSampleCountSlice (Slice (Imf::UINT,
                                      (char *) (&sampleCount[0]
                                              - dx- dy * dw),
                                      sizeof (unsigned int) * 1,
                                      sizeof (unsigned int) * dw));

    fb.insert ("Z",
               DeepSlice (Imf::FLOAT,
                          (char *) (&zbuff[0] - dx- dy * dw),
                          sizeof (float *) * 1,    // xStride for pointer array
                          sizeof (float *) * dw,   // yStride for pointer array
                          sizeof (float) * 1));    // stride for z data sample
    fb.insert ("ZBack",
               DeepSlice (Imf::FLOAT,
                          (char *) (&zback[0] - dx- dy * dw),
                          sizeof (float *) * 1,    // xStride for pointer array
                          sizeof (float *) * dw,   // yStride for pointer array
                          sizeof (float) * 1));    // stride for z data sample

    if (rgbflag)
    {
        fb.insert ("R",
                   DeepSlice (Imf::HALF,
                              (char *) (&dataR[0] - dx- dy * dw),
                              sizeof (half *) * 1,
                              sizeof (half *) * dw,
                              sizeof (half) * 1));

        fb.insert ("G",
                   DeepSlice (Imf::HALF,
                              (char *) (&dataG[0] - dx- dy * dw),
                              sizeof (half *) * 1,
                              sizeof (half *) * dw,
                              sizeof (half) * 1));

        fb.insert ("B",
                   DeepSlice (Imf::HALF,
                              (char *) (&dataB[0] - dx- dy * dw),
                              sizeof (half *) * 1,
                              sizeof (half *) * dw,
                              sizeof (half) * 1));


    }

    fb.insert ("A",
               DeepSlice (Imf::HALF,
                          (char *) (&alpha[0] - dx- dy * dw),
                          sizeof (half *) * 1,    // xStride for pointer array
                          sizeof (half *) * dw,   // yStride for pointer array
                          sizeof (half) * 1,      // stride for z data sample
                          1, 1,                   // xSampling, ySampling
                          1.0));                  // fillValue

    in.setFrameBuffer (fb);

    int numXTiles = in.numXTiles(0);
    int numYTiles = in.numYTiles(0);

    in.readPixelSampleCounts (0, numXTiles - 1, 0, numYTiles - 1);

    for (int i = 0; i < zsize; i++)
    {
        int num = sampleCount[i];
        zbuff[i] = new float[num];
        zback[i] = new float[num];
        alpha[i] = new half[num];
        if (rgbflag)
        {
            dataR[i] = new half[num];
            dataG[i] = new half[num];
            dataB[i] = new half[num];
        }
    }

    in.readTiles (0, numXTiles - 1, 0, numYTiles - 1);

    if (deepCompflag)
    {
        int count = 0;
        // Loop over all the pixels and comp manually
        // @ToDo implent deep compositing for the DeepTile case
        for (int i=0; i<zsize; ++i)
        {
            if ( sampleCount[i] == 0 ) continue;

            pixels[i].a = alpha[i][0];
            pixels[i].r = dataR[i][0];
            pixels[i].g = dataG[i][0];
            pixels[i].b = dataB[i][0];

            for(unsigned s=1; s<sampleCount[i]; s++)
            {
                float a = pixels[i].a;
                if( a>=1.f)
                    break;

                pixels[i].r += (1.f - a) * dataR[i][s];
                pixels[i].g += (1.f - a) * dataG[i][s];
                pixels[i].b += (1.f - a) * dataB[i][s];
                pixels[i].a += (1.f - a) * alpha[i][s];
            }
        }
    }
    else
    {
        for (int i = 0; i < zsize; i++)
        {
            if (sampleCount[i] > 0)
            {
                pixels[i].a = alpha[i][0];
                if (rgbflag)
                {
                    pixels[i].r = dataR[i][0];
                    pixels[i].g = dataG[i][0];
                    pixels[i].b = dataB[i][0];
                }
                else
                {
                    pixels[i].r = zbuff[i][0];
                    pixels[i].g = zbuff[i][0];
                    pixels[i].b = zbuff[i][0];
                }
            }
        }
    }

}

void
exrImage::loadDeepScanlineImage ( Imf::MultiPartInputFile& inmaster,
                                  int &zsize,
                                  Imf::Array<float*>&       zbuff,
                                  Imf::Array<unsigned int>& sampleCount,
                                  bool deepComp)
{

    _has_deep_data = true;

    DeepScanLineInputPart in (inmaster, _curpart);
    const Imf::Header& header = in.header();

    const Box2i &dataWindow = header.dataWindow();
    int dw = dataWindow.max.x - dataWindow.min.x + 1;
    int dh = dataWindow.max.y - dataWindow.min.y + 1;
    int dx = dataWindow.min.x;
    int dy = dataWindow.min.y;



    Array< float* > zback;
    Array< half* > alpha;

    zsize = dw * dh;
    zbuff.resizeErase (zsize);

    sampleCount.resizeErase (zsize);

    int rgbflag = 0;
    int deepCompflag = 0;

    if (header.channels().findChannel ("Z") &&
            header.channels().findChannel ("A") &&
            deepComp)
    {
        deepCompflag = 1;
    }

    DeepFrameBuffer fb;

    fb.insertSampleCountSlice (Slice (Imf::UINT,
                                      (char *) (&sampleCount[0]
                                              - dx- dy * dw),
                                      sizeof (unsigned int) * 1,
                                      sizeof (unsigned int) * dw));

    fb.insert ("Z",
               DeepSlice (Imf::FLOAT,
                          (char *) (&zbuff[0] - dx- dy * dw),
                          sizeof (float *) * 1,    // xStride for pointer array
                          sizeof (float *) * dw,   // yStride for pointer array
                          sizeof (float) * 1));    // stride for z data sample

    in.setFrameBuffer (fb);

    in.readPixelSampleCounts (dataWindow.min.y, dataWindow.max.y);

    for (int i = 0; i < dh * dw; i++)
    {
        zbuff[i] = new float[sampleCount[i]];
    }

    in.readPixels (dataWindow.min.y, dataWindow.max.y);


}


bool exrImage::fetch_multipart( Imf::MultiPartInputFile& inmaster,
                                const boost::int64_t& frame )
{
    if (  _numparts > 1 )
    {
        std::string left = get_long_view( true );
        std::string right = get_long_view( false );
        std::string L = get_short_view( true );
        std::string R = get_short_view( false );
        std::string prefix, suffix;


        std::string c;

        const Imf::Header& h = inmaster.header(0);
        if ( h.hasName() ) c = h.name();

        if  ( channel() ) c = channel();


        if ( !c.empty() )
        {
            if ( c[0] == '#' )
            {
                size_t idx = c.find( ' ' );
                if ( idx != std::string::npos )
                {
                    c = c.substr( idx+1, c.size() );
                }
            }

            size_t idx = c.find( left );
            if ( idx != std::string::npos )
            {
                suffix = c.substr( idx+left.size(), c.size() );
            }
            else
            {
                idx = c.find( L );
                if ( idx != std::string::npos )
                {
                    suffix = c.substr( idx+L.size(), c.size() );
                }
                else
                {
                    idx = c.find( right );
                    if ( idx != std::string::npos )
                    {
                        suffix = c.substr( idx+right.size(), c.size() );
                    }
                    else
                    {
                        idx = c.find( R );
                        if ( idx != std::string::npos )
                        {
                            suffix = c.substr( idx+R.size(), c.size() );
                        }
                    }
                }
            }

            prefix = c.substr( 0, idx-1 );
        }

        if ( _layers.empty() )
        {
            st[0] = st[1] = -1;

            for ( unsigned i = 0; i < _numparts; ++i )
            {
                const Header& header = inmaster.header(i);
                if ( ! header.hasType() )
                    _type = SCANLINEIMAGE;
                else
                    _type = header.type();

                if ( _type != SCANLINEIMAGE &&
                        _type != TILEDIMAGE &&
                        _type != DEEPSCANLINE &&
                        _type != DEEPTILE ) {
                    continue;
                }

                if ( _type == DEEPSCANLINE || _type == DEEPTILE )
                {
                    _has_deep_data = true;
                    image_damage( image_damage() | kDamage3DData );
                }

                read_forced_header_attr( header, frame );

                if ( ! _read_attr )
                    read_header_attr( header, frame );


                _pixel_ratio = header.pixelAspectRatio();
                _lineOrder   = header.lineOrder();
                _compression = header.compression();

                const Imf::ChannelList& channels = header.channels();


                char buf[256];
                std::string name;
                if ( header.hasName() ) name = header.name();



                // If layer name is empty it is the one with "Color".  We set it
                // as default.
                if ( name.empty() && _curpart == -1 )
                {
                    _curpart = _clear_part = i;
                }

                //
                // For simplicity sake, we don't accept periods in header name
                //
#ifdef CHANGE_PERIODS_TO_UNDERSCORES
                size_t pos;
                while ( (pos = name.find( '.' )) != std::string::npos )
                {
                    std::string n = name.substr( 0, pos );
                    n += '_';
                    if ( pos != name.size()-1 )
                        n += name.substr( pos+1, name.size() );
                    name = n;
                }
#endif

                buf[0] = 0;
                if ( !name.empty() )
                {
                    sprintf( buf, "#%d %s", i, name.c_str() );
                    _layers.push_back( buf );
                }
                else
                {
                    bool has_rgb = false;
                    _has_yca = false;
                    _use_yca = false;

                    if ( !has_rgb &&
                            (( channels.findChannel( N_("R") ) ) ||
                             ( channels.findChannel( N_("G") ) ) ||
                             ( channels.findChannel( N_("B") ) )) )
                    {
                        has_rgb = true;
                        rgb_layers();
                        lumma_layers();
                    }
                    else if ( !has_rgb )
                    {
                        if ( channels.findChannel( N_("Y") ) )
                        {
                            _layers.push_back( N_("Y") );
                            ++_num_channels;
                        }
                        if ( channels.findChannel( N_("RY") ) )
                        {
                            _layers.push_back( N_("RY") );
                            ++_num_channels;
                        }
                        if ( channels.findChannel( N_("BY") ) )
                        {
                            _layers.push_back( N_("BY") );
                            ++_num_channels;
                        }

                        if ( _num_channels != 0 )
                        {
                            has_rgb = true;
                            _has_yca = true;
                            _use_yca = true;
                            rgb_layers();
                            _num_channels = (unsigned short) ( _num_channels - 3 );
                            lumma_layers();
                        }
                    }
                    if ( has_rgb && channels.findChannel( N_("A") ) )
                    {
                        alpha_layers();
                        _has_alpha = true;
                    }
                    if ( has_rgb && channels.findChannel( N_("Z") ) )
                    {
                        _layers.push_back( N_("Z") );
                        ++_num_channels;
                    }
                    continue;
                }


                std::string ext = name;
                if ( header.hasView() ) ext = header.view();

                if ( !name.empty() || !ext.empty() )
                {
                    std::string first;
                    if ( !_layers.empty() ) first = *(_layers.end() - 1);

                    Imf::ChannelList::ConstIterator s = channels.begin();
                    Imf::ChannelList::ConstIterator e = channels.end();
                    for ( ; s != e; ++s )
                    {
                        std::string layerName = buf;
                        if ( !layerName.empty() ) layerName += '.';
                        std::string channelName = s.name();
                        if ( channelName.find( name ) == std::string::npos )
                        {
                            layerName += name;
                            layerName += '.';
                        }
                        layerName += s.name();
                        _layers.push_back( layerName );
                    }
                    stringArray::iterator it = std::find( _layers.begin(),
                                                          _layers.end(),
                                                          first );
                    if ( it != _layers.end() )
                    {
                        const std::string& layerName = *(it+1);
                        // Handle sorting RGB
                        if ( layerName.find( ".B" ) != std::string::npos &&
                                (_layers.end() - it == 4 ) )
                            std::sort( it+1, _layers.end(),
                                       std::greater<std::string>() );
                        // Handle sorting XYZ
                        else if ( layerName.find(".Z") != std::string::npos &&
                                  (_layers.end() - it == 4 ) )
                        {
                            std::sort( it+1, _layers.end(),
                                       std::less<std::string>() );
                        }
                        // Handle sorting RGBA
                        else if ( layerName.find(".A") != std::string::npos &&
                                  (_layers.end() - it == 5 ) )
                        {
                            std::sort( it+1, _layers.end(),
                                       std::greater<std::string>() );
                        }
                        // Handle sorting RGBAZ
                        else if ( ( layerName.find(".A") != std::string::npos ||
                                    layerName.find(".Z") != std::string::npos )
                                  && (_layers.end() - it > 5 ) )
                        {
                            std::sort( it+1, _layers.end()-1,
                                       std::greater<std::string>() );
                        }
                    }
                }

                if ( st[1] == -1 &&
                        ( ext.find( right ) != std::string::npos ||
                          ext.find( R ) != std::string::npos ) )
                {
                    if ( !prefix.empty() )
                    {
                        if ( prefix != "Z" &&
                                name.find( prefix ) == std::string::npos )
                            continue;
                    }
                    if ( !suffix.empty() )
                    {
                        if ( suffix != ".Z" &&
                                name.rfind( suffix ) == std::string::npos )
                            continue;
                    }
                    st[1] = i;
                    _has_right_eye = strdup( name.c_str() );
                    _is_stereo = true;
                    continue;
                }
                if ( st[0] == -1 &&
                        ( ext.find( left ) != std::string::npos ||
                          ext.find( L ) != std::string::npos ) )
                {
                    if ( !prefix.empty() )
                    {
                        if ( prefix != "Z" &&
                                name.find( prefix ) == std::string::npos )
                            continue;
                    }
                    if ( !suffix.empty() )
                    {
                        if ( suffix != ".Z" &&
                                name.rfind( suffix ) == std::string::npos )
                            continue;
                    }
                    _has_left_eye = strdup( name.c_str() );
                    st[0] = i;
                    _is_stereo = true;
                    continue;
                }

            }
        }
    }
    else if ( _numparts == 1 )
    {
        const Imf::Header& header = inmaster.header(0);
        if ( header.hasType() ) _type = header.type();

        if ( _type == DEEPSCANLINE || _type == DEEPTILE )
        {
            _has_deep_data = true;
            image_damage( image_damage() | kDamage3DData );
        }

        read_forced_header_attr( header, frame );

        if ( ! _read_attr )
            read_header_attr( header, frame );

        if ( _multiview )
        {
            st[0] = st[1] = 0;
        }
    }



    if ( _is_stereo && _multiview && ( st[0] == -1 || st[1] == -1 ) )
    {
        IMG_ERROR( _("Could not find both stereo images in multiview file") );
        if ( st[0] != -1 ) st[1] = st[0];
        else if ( st[1] != -1 ) st[0] = st[1];
        if ( st[0] == -1 ) return false;
    }
    else if ( _is_stereo && stereo_output() != CMedia::kNoStereo &&
              ( st[0] == -1 || st[1] == -1 ) )
    {
        if ( st[0] == -1 ) st[0] = 0;
        else if ( st[1] == -1 ) st[1] = 0;
        if ( st[0] == st[1] )
        {
            std::string root( _fileroot );
            if ( root.find( "%V" ) != std::string::npos ||
                    root.find( "%v" ) != std::string::npos )
                return true;
            IMG_ERROR( _("Could not find both stereo images in file") );
            return false;
        }
    }

    const char* channelPrefix = channel();
    if ( channelPrefix != NULL )
    {
        if ( channelPrefix[0] == '#' )
        {
            std::string part = channelPrefix;

            size_t pos = part.find( ' ' );
            part = part.substr( 1, pos );

            _curpart = (int) strtoul( part.c_str(), NULL, 10 );
        }
        else
        {
            _curpart = _clear_part;
        }
    }
    else
    {
        _curpart = _clear_part;
    }

    if ( _is_stereo )
    {
        for ( int i = 0; i < 2; ++i )
        {
            if ( _stereo_output != kNoStereo && st[i] >= 0 &&
                    _right_eye == NULL )
                _curpart = st[i];


            const Header& header = inmaster.header(_curpart);

            const Box2i& displayWindow = header.displayWindow();
            const Box2i& dataWindow = header.dataWindow();

            if ( i == 0 || _stereo_output == kNoStereo )
            {
                data_window( dataWindow.min.x, dataWindow.min.y,
                             dataWindow.max.x, dataWindow.max.y, frame );

                display_window( displayWindow.min.x, displayWindow.min.y,
                                displayWindow.max.x, displayWindow.max.y,
                                frame );
            }
            else
            {
                data_window2( dataWindow.min.x, dataWindow.min.y,
                              dataWindow.max.x, dataWindow.max.y, frame );

                display_window2( displayWindow.min.x, displayWindow.min.y,
                                 displayWindow.max.x, displayWindow.max.y,
                                 frame );
            }


            FrameBuffer fb;

            bool ok = find_channels( header, fb, frame );
            if (!ok) {
                IMG_ERROR( _("Could not locate channels in header") );
                return false;
            }

            _pixel_ratio = header.pixelAspectRatio();
            _lineOrder   = header.lineOrder();
            _compression = header.compression();

            try
            {
                InputPart in( inmaster, _curpart );
                in.setFrameBuffer(fb);
                in.readPixels( dataWindow.min.y, dataWindow.max.y );
            }
            catch( const std::exception& e )
            {
                IMG_ERROR( e.what() );
                return false;
            }

            // Quick exit if stereo is off or multiview
            if ( _stereo_output == kNoStereo ) break;

            if ( st[0] != st[1] )
            {
                _stereo[i] = _hires;
            }
        }
    }
    else
    {

        int oldpart = _curpart;

        const Header& header = inmaster.header(_curpart);


        if ( _type == DEEPTILE )
        {
            if ( ! find_layers( header ) )
                return false;

            read_forced_header_attr( header, frame );

            if ( ! _read_attr )
                read_header_attr( header, frame );

            int zsize;
            Imf::Array< float* > zbuff;
            Imf::Array< unsigned > sampleCount;
            loadDeepTileImage( inmaster, zsize, zbuff, sampleCount, true );
            return true;
        }


        InputPart in (inmaster, _curpart);
        const Box2i& dataWindow = header.dataWindow();
        const Box2i& displayWindow = header.displayWindow();

        data_window( dataWindow.min.x, dataWindow.min.y,
                     dataWindow.max.x, dataWindow.max.y, frame );

        display_window( displayWindow.min.x, displayWindow.min.y,
                        displayWindow.max.x, displayWindow.max.y, frame );

        FrameBuffer fb;
        bool ok = find_channels( header, fb, frame );
        if (!ok) {
            IMG_ERROR( _("Could not locate channels in header") );
            return false;
        }


        _pixel_ratio = header.pixelAspectRatio();
        _lineOrder   = header.lineOrder();
        _compression = header.compression();

        if ( _curpart != oldpart )
        {
            InputPart in (inmaster, _curpart);
            const Header& header = in.header();


            const Box2i& dataWindow = header.dataWindow();
            const Box2i& displayWindow = header.displayWindow();

            data_window( dataWindow.min.x, dataWindow.min.y,
                         dataWindow.max.x, dataWindow.max.y, frame );

            display_window( displayWindow.min.x, displayWindow.min.y,
                            displayWindow.max.x, displayWindow.max.y, frame );

            FrameBuffer fb;
            bool ok = find_channels( header, fb, frame );
            if (!ok) {
                IMG_ERROR( _("Could not locate channels in header") );
                return false;
            }

            _pixel_ratio = header.pixelAspectRatio();
            _lineOrder   = header.lineOrder();
            _compression = header.compression();

            try
            {
                in.setFrameBuffer(fb);
                in.readPixels( dataWindow.min.y, dataWindow.max.y );
            }
            catch( const std::exception& e )
            {
                IMG_ERROR( e.what() );
                return false;
            }
        }
        else
        {
            try
            {
                in.setFrameBuffer(fb);
                in.readPixels( dataWindow.min.y, dataWindow.max.y );
            }
            catch( const std::exception& e )
            {
                IMG_ERROR( e.what() );
                return false;
            }
        }

    }

    return true;
}

/**
 * Fetch the current EXR image
 *
 *
 * @return true if success, false if not
 */
bool exrImage::fetch( const boost::int64_t frame )
{

    try
    {

        if ( _levelX > 0 || _levelY > 0 )
        {
            return fetch_mipmap( frame );
        }

        MultiPartInputFile inmaster( sequence_filename(frame).c_str() );
        _numparts = inmaster.parts();



        if ( _numparts > 0 )
        {

            if ( !  fetch_multipart( inmaster, frame ) )
                return false;

            if ( _use_yca && !supports_yuv() )
            {
                const Imf::Header& h = inmaster.header(0);
                ycc2rgba( h, frame );
            }
        }
    }
    catch( const std::exception& e )
    {
        IMG_ERROR( e.what() );
        _curpart = -1;
        image_size( _w, _h );
        return false;
    }

    return true;
}

static
void save_attributes( const CMedia* img, Header& hdr,
                      const EXROpts* opts )
{
    stringSet attrs;
    attrs.insert( "type" );
    attrs.insert( "compression" );
    attrs.insert( "channels" );

    if ( opts->ACES_metadata() )
    {
        Imf::IntAttribute attr = 1;
        hdr.insert( N_("acesImageContainerFlag"), attr );
    }

    if ( img->has_chromaticities() )
    {
        Imf::ChromaticitiesAttribute attr( img->chromaticities() );
        hdr.insert( N_("Chromaticities"), attr );
        attrs.insert( N_("Chromaticities") );
    }


    const exrImage* exr = dynamic_cast< const exrImage* >( img );
    if ( exr )
    {
        const std::string& date = exr->capture_date( img->frame() );
        Imf::StringAttribute attr( date );
        hdr.insert( N_("capDate"), attr );
    }

    const CMedia::Attributes& attributes = img->attributes();
    CMedia::Attributes::const_iterator it = attributes.find( _( "UTC Offset" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("utcOffset"), *it->second );
        attrs.insert( _("UTC Offset") );
    }

    it = attributes.find( _( "Longitude" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("longitude"), *it->second );
        attrs.insert( _("Longitude") );
    }

    it = attributes.find( _( "Latitude" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("latitude"), *it->second );
        attrs.insert( _("Latitude") );
    }

    it = attributes.find( _( "Altitude" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("altitude"), *it->second );
        attrs.insert( _("Altitude") );
    }

    if ( ! img->ocio_input_color_space().empty() )
    {
        Imf::StringAttribute attr( img->ocio_input_color_space() );
        hdr.insert( N_("inputColorSpace"), attr );
        attrs.insert( _("Input Color Space") );
    }

    it = attributes.find( _( "Chromaticities Name" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("Chromaticities Name"), *it->second );
        attrs.insert( _("Chromaticities Name") );
    }

    it = attributes.find( _( "Comments" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( _("Comments"), *it->second );
        attrs.insert( _("Comments") );
    }

    it = attributes.find( _( "Adopted Neutral" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("adoptedNeutral"), *it->second );
        attrs.insert( _("Adopted Neutral") );
    }

    it = attributes.find( _( "Image State" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("imageState"), *it->second );
        attrs.insert( _("Image State") );
    }

    it = attributes.find( _( "Owner" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("owner"), *it->second );
        attrs.insert( _("Owner") );
    }

    it = attributes.find( _( "Comments" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("comments"), *it->second );
        attrs.insert( _("Comments") );
    }

    it = attributes.find( _( "Capture Date" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("capDate"), *it->second );
        attrs.insert( _("Capture Date") );
    }

    it = attributes.find( _( "UTC Offset") );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("utcOffset"), *it->second );
        attrs.insert( _("UTC Offset") );
    }

    it = attributes.find( _( "Longitude") );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("longitude"), *it->second );
        attrs.insert( _("Longitude") );
    }

    it = attributes.find( _( "Latitude") );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("latitude"), *it->second );
        attrs.insert( _("Latitude") );
    }


    it = attributes.find( _( "Altitude") );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("altitude"), *it->second );
        attrs.insert( _("Altitude") );
    }


    it = attributes.find( _( "Focus") );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("focus"), *it->second );
        attrs.insert( _("Focus") );
    }


    it = attributes.find( _( "Exposure Time") );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("expTime"), *it->second );
        attrs.insert( _("Exposure Time") );
    }

    it = attributes.find( _( "Aperture") );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("aperture"), *it->second );
        attrs.insert( _("Aperture") );
    }

    it = attributes.find( _( "ISO Speed") );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("isoSpeed"), *it->second );
        attrs.insert( _("ISO Speed") );
    }

    {
        Imf::Rational r( int( img->fps() * 1000 ), 1000 );
        Imf::RationalAttribute attr( r );
        hdr.insert( N_("framesPerSecond"), attr );
        attrs.insert( N_("framesPerSecond") );
    }

    it = attributes.find( _( "Film Manufacturer Code" ) );
    if ( it != attributes.end() )
    {
        attrs.insert( _("Film Manufacturer Code") );
        hdr.insert( N_("keyCode"), *it->second );
        attrs.insert( _("keyCode") );
    }


    it = attributes.find( N_( "timecode" ) );
    if ( it != attributes.end() )
    {

        Imf::TimeCodeAttribute* attr =
            dynamic_cast< Imf::TimeCodeAttribute* >( it->second );

        if ( attr )
        {
            Imf::TimeCode t( attr->value() );

            mrv::Timecode::Display d = mrv::Timecode::kTimecodeNonDrop;
            if ( t.dropFrame() ) d = mrv::Timecode::kTimecodeDropFrame;
            char buf[64];
            mrv::Timecode::format( buf, d, img->frame(), img->timecode(),
                                   img->fps(), true );
            int hours, minutes, seconds, frames;
            if ( t.dropFrame() )
            {
                sscanf( buf, "%02d;%02d;%02d;%02d",
                        &hours, &minutes, &seconds, &frames );
            }
            else
            {
                sscanf( buf, "%02d:%02d:%02d:%02d",
                        &hours, &minutes, &seconds, &frames );
            }

            t.setHours( hours );
            t.setMinutes( minutes );
            t.setSeconds( seconds );
            t.setFrame( frames );

            attrs.insert( N_("timecode") );

            Imf::TimeCodeAttribute val(t);
            hdr.insert( N_("timeCode"), val );
        }
    }

    it = attributes.find( _( "Writer" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("writer"), *it->second );
        attrs.insert( _("Writer") );
    }

    it = attributes.find( _( "ICC Profile" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("iccProfile"), *it->second );
        attrs.insert( _("ICC Profile") );
    }

    it = attributes.find( _( "Wrap Modes" ) );
    if ( it != attributes.end() )
    {
        hdr.insert( N_("wrapmodes"), *it->second );
        attrs.insert( _("Wrap Modes") );
    }

    CMedia::Attributes::const_iterator i = attributes.begin();
    CMedia::Attributes::const_iterator e = attributes.end();
    stringSet::const_iterator end = attrs.end();
    for ( ; i != e; ++i )
    {
        const std::string& name = i->first;
        // Avoid adding attributes we already parsed
        if ( attrs.find( name ) != end ) continue;
        const Imf::Attribute& attr = *(i->second);
        hdr.insert( name, attr );
    }
}


typedef std::vector< Imf::Header > HeaderList;
typedef std::vector< Imf::FrameBuffer > FrameBufferList;
typedef std::vector< Imf::DeepFrameBuffer > DeepFrameBufferList;
typedef std::set< std::string > PartNames;
typedef std::vector< std::string >   LayerList;


void exrImage::copy_pixel_data( mrv::image_type_ptr pic,
                                Imf::PixelType save_type,
                                uint8_t* base,
                                size_t total_size,
                                bool use_alpha )
{
    Imf::PixelType pt = pixel_type_to_exr( pic->pixel_type() );
    unsigned dh = pic->height();
    unsigned dw = pic->width();
    unsigned channels = pic->channels();

    if ( pt == save_type )
    {
        memcpy( base, pic->data().get(), total_size );
    }
    else
    {
        for ( unsigned y = 0; y < dh; ++y )
        {
            for ( unsigned x = 0; x < dw; ++x )
            {
                CMedia::Pixel p = pic->pixel( x, y );
                unsigned yh = channels * ( x + y * dw );
                if ( save_type == Imf::HALF )
                {
                    half* s = (half*) base;
                    s[ yh ]   = p.r;
                    if ( channels == 1 ) continue;
                    s[ yh + 1 ] = p.g;
                    s[ yh + 2 ] = p.b;
                    if ( use_alpha )
                        s[ yh + 3 ] = p.a;
                }
                else if ( save_type == Imf::FLOAT )
                {
                    CMedia::Pixel* s = (CMedia::Pixel*) base;
                    if ( use_alpha )
                        s[ yh + 3 ] = p;
                    else
                    {
                        float* s = (float*) base;
                        s[ yh ] = p.r;
                        if ( channels == 1 ) continue;
                        s[ yh + 1 ] = p.g;
                        s[ yh + 2 ] = p.b;
                    }
                }
                else if ( save_type == Imf::UINT )
                {
                    unsigned* s = (unsigned*) base;
                    if ( p.r > 1.0f ) p.r = 1.0f;
                    else if ( p.r < 0.0f ) p.r = 0.0f;
                    s[ yh ] = (unsigned) (p.r * 4294967295.0f);
                    if ( channels == 1 ) continue;
                    if ( p.g > 1.0f ) p.g = 1.0f;
                    else if ( p.g < 0.0f ) p.g = 0.0f;
                    if ( p.b > 1.0f ) p.b = 1.0f;
                    else if ( p.b < 0.0f ) p.b = 0.0f;
                    s[ yh + 1 ] = (unsigned) (p.g * 4294967295.0f);
                    s[ yh + 2 ] = (unsigned) ( p.b * 4294967295.0f );
                    if ( use_alpha )
                        s[ yh + 3 ] = (unsigned) (p.a * 4294967295.0f);
                }
            }
        }
    }
}

static
void add_layer( HeaderList& headers, FrameBufferList& fbs,
                PartNames& names, LayerList& layers,
                Imf::PixelType save_type,
                const CMedia* img, const EXROpts* opts,
                std::string& root, const std::string& layer,
                const std::string& x )
{

    Header hdr;
    hdr.setVersion( 1 );
    hdr.setType( SCANLINEIMAGE );

    // Avoid repeated names
    while ( names.find( root ) != names.end() )
        root += "_2";
    names.insert( root );

    hdr.setName( root );

    std::string left = get_long_view( true );
    std::string right = get_long_view( false );

    if ( root.find( right ) != std::string::npos )
        hdr.setView( right );

    if ( root.find( left ) != std::string::npos )
        hdr.setView( left );


    if ( x == N_("Z") || x == N_("Y") )
    {
        hdr.channels().insert( x,
                               Channel( save_type,1,1 ) );
    }
    else if ( x == _("YBYRY") )
    {
        hdr.channels().insert( N_("Y"),
                               Channel( save_type, 1, 1 ) );
        hdr.channels().insert( N_("BY"),
                               Channel( save_type, 2, 2 ) );
        hdr.channels().insert( N_("RY"),
                               Channel( save_type, 2, 2 ) );
    }
    else if ( x == _("Color") || x == N_("ColorZ") || x == N_("ColorZBack") )
    {
        hdr.channels().insert( N_("R"),
                               Channel( save_type, 1, 1 ) );
        hdr.channels().insert( N_("G"),
                               Channel( save_type, 1, 1 ) );
        hdr.channels().insert( N_("B"),
                               Channel( save_type, 1, 1 ) );

        if ( img->has_alpha() )
            hdr.channels().insert( N_("A"),
                                   Channel( save_type, 1, 1 ) );

        if ( x == N_("ColorZ") || x == N_("ColorZBack") )
            hdr.channels().insert( N_("Z"),
                                   Channel( save_type, 1, 1 ) );

        if ( x == N_("ColorZBack") )
            hdr.channels().insert( N_("ZBack"),
                                   Channel( save_type, 1, 1 ) );
    }
    else
    {
        // this breaks saving of multipart OpenEXR
        //hdr.channels().insert( x, Channel( save_type, 1, 1 ) );
    }

    Imf::Compression comp = opts->compression();
    if ( comp >= NUM_COMPRESSION_METHODS )
    {
        LOG_WARNING( _("Compression method not available. "
                       "Using PIZ.") );
        comp = Imf::PIZ_COMPRESSION;
    }
    hdr.compression() = comp;

    if ( comp == Imf::DWAA_COMPRESSION ||
            comp == Imf::DWAB_COMPRESSION )
    {
        Imf::addDwaCompressionLevel( hdr,
                                     opts->compression_level() );
    }


    layers.push_back( layer );
    headers.push_back( hdr );
    FrameBuffer fb;
    fbs.push_back( fb );
}



bool save_deep_data(const char* file, const CMedia* img,
                    const EXROpts* opts )
{
    using namespace Imf;
    using namespace std;


    std::string input = img->sequence_filename( img->frame() );
    MultiPartInputFile in( input.c_str() );
    int numParts = in.parts();
    vector <Header> headers;

    for (int part = 0; part < numParts; ++part)
    {
        const Header& hdr = in.header (part);
        Header& h = const_cast< Header& >( hdr );
        {
            Header::Iterator i = h.begin();
            Header::Iterator e = h.end();
            stringSet names;
            for ( ; i != e; ++i )
            {
                const std::string& name = i.name();
                if ( ignore.find(name) != ignore.end() )
                    continue;
                names.insert( name );
            }
            stringSet::const_iterator j = names.begin();
            stringSet::const_iterator je = names.end();
            for ( ; j != je; ++j )
            {
                h.erase( *j );
            }
        }

        save_attributes( img, h, opts );

        headers.push_back(h);
    }

    if ( headers.empty() )
    {
        LOG_ERROR( "Empty headers for " << img->name() );
        return false;
    }


    try
    {

        MultiPartOutputFile out (file, &headers[0], numParts);
        for (int p = 0; p < numParts; ++p)
        {
            const Header &h = in.header (p);
            const string &type = h.type();

            if (type == SCANLINEIMAGE)
            {
                InputPart  inPart  (in,  p);
                OutputPart outPart (out, p);
                outPart.copyPixels (inPart);
            }
            else if (type == TILEDIMAGE)
            {
                TiledInputPart  inPart  (in,  p);
                TiledOutputPart outPart (out, p);
                outPart.copyPixels (inPart);
            }
            else if (type == DEEPSCANLINE)
            {
                DeepScanLineInputPart  inPart  (in,  p);
                DeepScanLineOutputPart outPart (out, p);
                outPart.copyPixels (inPart);
            }
            else if (type == DEEPTILE)
            {
                DeepTiledInputPart  inPart  (in,  p);
                DeepTiledOutputPart outPart (out, p);
                outPart.copyPixels (inPart);
            }
        }
    }
    catch ( const std::exception& e )
    {
        LOG_ERROR( img->name() << ": Could not save file. " << e.what() );
    }

    return true;
}

bool exrImage::save( const char* file, const CMedia* img,
                     const ImageOpts* const ipts )
{

    const EXROpts* const opts = dynamic_cast< const EXROpts* >( ipts );
    if (!opts)
    {
        LOG_ERROR( _("Options for EXR were empty") );
        return false;
    }

    if ( opts->save_deep_data() && img->has_deep_data() )
    {
        return save_deep_data( file, img, opts );
    }

    std::string old_channel;
    const char* orig = img->channel();
    if ( orig ) old_channel = orig;


    CMedia* p = const_cast< CMedia* >( img );


    HeaderList headers;
    FrameBufferList fbs;
    Buffers bufs;
    PartNames names;
    LayerList layers;

    Imf::PixelType save_type = opts->pixel_type();



    if ( opts->all_layers() )
    {

        stringArray::const_iterator i = img->layers().begin();
        stringArray::const_iterator e = img->layers().end();
        stringArray::const_iterator s = i;

        bool has_z = false;
        bool has_zback = false;
        bool has_y = false;
        bool has_yca = false;
        for ( ; i != e; ++i )
        {
            const std::string& x = *i;
            if ( x == "Y" )
            {
                has_y = true;
            }
            else if ( x == "Z" )
            {
                has_z = true;
            }
            else if ( x == "ZBack" )
            {
                has_zback = true;
            }
            else if ( x == "RY" || x == "BY" )
            {
                has_yca = true;
            }
        }

        std::string x = N_("ZVCF!@");
        for ( i = s; i != e; ++i )
        {
            const std::string& name = *i;

            if ( name.find(x + '.') == 0 )
            {
                std::string root = name;

                if ( root[0] == '#' && x.size() < name.size() )
                {
                    root = name.substr( x.size()+1, name.size() );
                }

                size_t pos = root.rfind( '.' );
                std::string suffix;
                if ( pos != std::string::npos && pos != root.size() )
                {
                    suffix = root.substr( pos+1, root.size() );
                    root = root.substr( 0, pos );
                    if ( suffix.size() == 1 )
                    {
                        std::transform( suffix.begin(), suffix.end(),
                                        suffix.begin(),
                                        (int(*)(int)) toupper );
                    }
                }

                Header& hdr = headers.back();

                if ( !suffix.empty() )
                {
                    hdr.channels().insert( root + '.' + suffix,
                                           Channel( save_type,1,1 ) );
                }
                else
                {
                    hdr.channels().insert( root,
                                           Channel( save_type,1,1 ) );
                }
            }
            else
            {
                x = name;

                if ( x == _("Lumma") || x == _("Alpha Overlay") ||
                        x == _("Red") || x == _("Green") ||
                        x == _("Blue") || x == _("Alpha")  )
                {
                    x = _("Color");
                    continue;
                }

                if ( x == N_("RY") || x == N_("BY") || x == N_("Z") ||
                        x == N_("ZBack") )
                    continue;

                if ( x == _("Color") && has_y ) continue;

                if ( x == N_("Y") )
                {
                    if ( has_yca )
                        x = N_("YBYRY");
                    else
                        x = N_("Y");
                }

                std::string root = x;

                if ( root == _("Color") ||
                        root == N_("YBYRY") ||
                        root == N_("Y") ) root = "";

                // If R,G,B,A,Z,ZBack exists, mark it as ColorZBack
                if ( x == _("Color") && has_zback ) x = N_("ColorZBack");
                // If R,G,B,A,Z exists, mark it as ColorZ
                else if ( x == _("Color") && has_z ) x = N_("ColorZ");

                // If dealing with a multipart remove the #number from root
                // name
                if ( root[0] == '#' )
                {
                    size_t pos = root.find( ' ' );
                    if ( pos != std::string::npos )
                        root = root.substr( pos+1, root.size() );
                }


                add_layer( headers, fbs, names, layers,
                           save_type, img, opts, root, name, x );

                if ( headers.size() == 1 )
                    save_attributes( img, headers[0], opts );
            }
        }
    }
    else
    {
        const char* layer = img->channel();
        std::string root, x;
        if ( layer ) root = layer;
        else layer = "";

        mrv::image_type_ptr pic = img->hires();

        if ( root.find( "Z" ) != std::string::npos ) x = "Z";
        else
        {
            image_type::Format pf = pic->format();
            if ( pf == image_type::kLumma )
                x = N_("Y" );
            else if ( pf >= image_type::kYByRy420 )
                x = N_("YBYRY" );
            else
                x = _("Color");
        }

        add_layer( headers, fbs, names, layers,
                   save_type, img, opts, root, layer, x );
        save_attributes( img, headers[0], opts );
    }


    unsigned part = 0;
    unsigned numParts = (unsigned)fbs.size();

    p->channel( NULL );
    const mrv::Recti& dpw = img->display_window();

    for ( ; part < numParts; ++part )
    {
        Header& h = headers[part];
        FrameBuffer& fb = fbs[part];

        const std::string& layer = layers[part];

        if ( layer == "Y" )
        {
            p->channel( NULL );
        }
        else
        {
            p->channel( layer.c_str() );
        }

        const char* ch = p->channel();
        if ( ( layer == _("Color") && ch != NULL ) ||
                ( ch && layer != ch ) )
            LOG_ERROR( _("Failed setting layer to ") << layer );

        // std::cerr << "Changed to layer "
        //           << ( p->channel() ? p->channel() : "NULL" ) << std::endl;

        const mrv::Recti& daw = img->data_window();


        Box2i bdpw( V2i(dpw.x(), dpw.y()),
                    V2i(dpw.x() + dpw.w() - 1,
                        dpw.y() + dpw.h() - 1) );
        Box2i bdaw( V2i(daw.x(), daw.y()),
                    V2i(daw.x() + daw.w() - 1,
                        daw.y() + daw.h() - 1) );

        h.displayWindow() = bdpw;
        h.dataWindow() = bdaw;


        int dx = daw.x();
        int dy = daw.y();

        mrv::image_type_ptr pic = img->hires();
        if (!pic) return false;

        unsigned dw = pic->width();
        unsigned dh = pic->height();

        int xsampling[6], ysampling[6];
        for ( int i = 0; i < 6; ++i )
        {
            xsampling[i] = ysampling[i] = 1;
        }
        unsigned offsets[6];
        offsets[0] = offsets[1] = offsets[2] = offsets[3] =
                offsets[4] = offsets[5] = 0;

        size_t size;
        switch( save_type )
        {
        case Imf::UINT:
            size = sizeof(unsigned);
            break;
        case Imf::FLOAT:
            size = sizeof(float);
            break;
        default:
        case Imf::HALF:
            size = sizeof(half);
            break;
        }

        image_type::Format pf = pic->format();
        unsigned channels = pic->channels();

        size_t total_size = 0;
        bool has_yca = false;
        switch( pf )
        {
        case image_type::kLumma:
            total_size = dw*dh*size;
            offsets[3] = (unsigned)total_size;
            offsets[1] = offsets[2] = 0;
            break;
        case image_type::kLummaA:
            offsets[3] = dw*dh*(unsigned)size;
            offsets[1] = offsets[2] = 0;
            total_size = offsets[3]*2;
            break;
        // case image_type::kITU_601_YCbCr410:
        // case image_type::kITU_709_YCbCr410:
        // case image_type::kYByRy410:
        //     {
        //         //@todo:
        //         has_yca = true;
        //         total_size = dw*dh*size*4;
        //         offsets[1] = 1; offsets[2] = 2; offsets[3] = 3;
        //         break;
        //     }
        // case image_type::kITU_601_YCbCr410A:
        // case image_type::kITU_709_YCbCr410A:
        // case image_type::kYByRy410A:
        //     {
        //         //@todo:
        //         has_yca = true;
        //         total_size = dw*dh*size*4;
        //         offsets[1] = 1; offsets[2] = 2; offsets[3] = 3;
        //         break;
        //     }
        // case image_type::kITU_601_YCbCr420:
        // case image_type::kITU_709_YCbCr420:
        case image_type::kYByRy420:
        {
            has_yca = true;
            unsigned int Ylen    = dw * dh;
            unsigned int w2      = (dw + 1) / 2;
            unsigned int h2      = (dh + 1) / 2;
            unsigned int Cblen   = w2 * h2;
            offsets[1] = Ylen;
            offsets[2] = Ylen + Cblen;
            xsampling[1] = ysampling[1] = 2;
            xsampling[2] = ysampling[2] = 2;
            total_size = size * (Ylen + Cblen*2);
            break;
        }
        // case image_type::kITU_601_YCbCr420A:
        // case image_type::kITU_709_YCbCr420A:
        case image_type::kYByRy420A:
        {
            has_yca = true;
            unsigned int Ylen    = dw * dh;
            unsigned int w2      = (dw + 1) / 2;
            unsigned int h2      = (dh + 1) / 2;
            unsigned int Cblen   = w2 * h2;
            offsets[1] = Ylen;
            offsets[2] = Ylen + Cblen;
            offsets[3] = Ylen + Cblen*2;
            xsampling[1] = ysampling[1] = 2;
            xsampling[2] = ysampling[2] = 2;
            total_size = size * (Ylen*2 + Cblen*2);
            break;
        }
        // case image_type::kITU_709_YCbCr422:
        // case image_type::kITU_601_YCbCr422:
        case image_type::kYByRy422:
        {
            has_yca = true;
            unsigned int  Ylen = dw * dh;
            unsigned int w2 = (dw + 1) / 2;
            unsigned int Cblen = w2 * dh;
            offsets[1] = Ylen;
            offsets[2] = Ylen + Cblen;
            total_size = size * (Ylen + Cblen*2);
            break;
        }
        // case image_type::kITU_709_YCbCr422A:
        // case image_type::kITU_601_YCbCr422A:
        case image_type::kYByRy422A:
        {
            has_yca = true;
            unsigned int  Ylen = dw * dh;
            unsigned int w2 = (dw + 1) / 2;
            unsigned int Cblen = w2 * dh;
            offsets[1] = Ylen;
            offsets[2] = Ylen + Cblen;
            offsets[3] = Ylen + Cblen * 2;
            total_size = size * (Ylen*2 + Cblen*2);
            break;
        }
        // case image_type::kITU_709_YCbCr444:
        // case image_type::kITU_601_YCbCr444:
        // case image_type::kITU_709_YCbCr444A:
        // case image_type::kITU_601_YCbCr444A:
        case image_type::kYByRy444:
        case image_type::kYByRy444A:
        {
            has_yca = true;
            unsigned int  Ylen = dw * dh;
            offsets[1] = Ylen;
            offsets[2] = Ylen * 2;
            offsets[3] = Ylen * 3;
            total_size = size * Ylen * 4;
            break;
        }
        case image_type::kITU_601_YCbCr410:
        case image_type::kITU_709_YCbCr410:
        case image_type::kITU_601_YCbCr410A:
        case image_type::kITU_709_YCbCr410A:
        case image_type::kITU_601_YCbCr420:
        case image_type::kITU_709_YCbCr420:
        case image_type::kITU_601_YCbCr420A:
        case image_type::kITU_709_YCbCr420A:
        case image_type::kITU_709_YCbCr422:
        case image_type::kITU_601_YCbCr422:
        case image_type::kITU_709_YCbCr422A:
        case image_type::kITU_601_YCbCr422A:
        case image_type::kITU_709_YCbCr444:
        case image_type::kITU_601_YCbCr444:
        case image_type::kITU_709_YCbCr444A:
        case image_type::kITU_601_YCbCr444A:
        case image_type::kBGR:
        case image_type::kBGRA:
        case image_type::kRGB:
        case image_type::kRGBA:
        default:
            offsets[1] = 1;
            offsets[2] = 2;
            offsets[3] = 3;
            total_size = dw*dh*size*channels;
            break;
        }


        uint8_t* base = (uint8_t*) new uint8_t[total_size];
        bufs.push_back( base );


        bool use_alpha = false;
        if ( channels == 4 ) use_alpha = true;

        copy_pixel_data( pic, save_type, base, total_size, use_alpha );

        size_t t = size;

        int order[6]; // R,G,B,A,Z,Zback
        order[0] = order[1] = order[2] = order[3] = order[4] = order[5] = -1;


        bool no_layer = false;

        ChannelList::ConstIterator ci = h.channels().begin();
        ChannelList::ConstIterator ce = h.channels().end();
        ChannelList::ConstIterator cs = ci;

        for ( int idx = 0; ci != ce; ++ci, ++idx )
        {
            const std::string& name = ci.name();


            std::string ext = name;

            if ( no_layer == false )
            {
                size_t pos = ext.rfind( '.' );
                if ( pos != std::string::npos )
                {
                    ext = ext.substr( pos+1, ext.size() );
                }
                else
                {
                    no_layer = true;
                }
            }

            std::transform( ext.begin(), ext.end(), ext.begin(),
                            (int(*)(int)) toupper);


            if ( order[0] == -1 && (ext == N_("R") ||
                                    ext == N_("Y") || ext == N_("U") ||
                                    ext == N_("X") || ext == N_("Z")) )
            {
                order[0] = idx;
            }
            else if ( order[1] == -1 && (ext == N_("G")  ||
                                         ext == N_("RY") || ext == N_("V") ||
                                         ext == N_("Y") ) )
            {
                order[1] = idx;
            }
            else if ( order[2] == -1 && (ext == N_("B") ||
                                         ext == N_("BY") || ext == N_("W") ||
                                         ext == N_("Z") ) )
            {
                order[2] = idx;
            }
            else if ( order[3] == -1 && ext == N_("A") )
            {
                order[3] = idx;
            }
            else if ( order[4] == -1 && ext == N_("Z") )
            {
                order[4] = idx;
            }
            else if ( order[5] == -1 && ext == N_("ZBACK") )
            {
                order[5] = idx;
            }
            else if ( order[0] == -1 && order[1] == -1 && order[2] == -1 &&
                      order[3] == -1 && order[4] == -1 && order[5] == -1 &&
                      (no_layer || ext.size() > 1) )
            {
                order[0] = idx;
            }
        }


        size_t xs[6], ys[6];

        if ( has_yca )
        {
            size_t t = size;

            for ( unsigned j = 0; j < 6; ++j )
            {
                xs[j] = t;
            }

            size_t dw2 = dw / 2;
            if ( order[0] != -1 ) ys[0] = t * dw; // for Y channel
            if ( order[1] != -1 ) ys[1] = t * dw2; // for BY channel
            if ( order[2] != -1 ) ys[2] = t * dw2; // for RY channel
            if ( order[3] != -1 ) ys[3] = t * dw; // for Alpha channel
            if ( order[4] != -1 ) ys[4] = t * dw; // for Z channel
            if ( order[5] != -1 ) ys[5] = t * dw; // for ZBack channel
        }
        else
        {
            unsigned pixels = (unsigned)size * channels;

            // xs[4], ys[4], xs[5], ys[5] are set later
            for ( unsigned j = 0; j < 4; ++j )
            {
                int k = order[j];
                if ( k == -1 ) continue;
                xs[k] = pixels;
                ys[k] = pixels * dw;
            }
        }


        int start = (-dx - dy * dw) * (int)size * channels;
        base += start;

        int idx = 0;
        for ( ci = cs; ci != ce; ++ci, ++idx )
        {
            int k = order[idx];
            if ( k == -1 ) continue;

            const std::string& name = ci.name();

            // std::cerr << "SAVE " << idx << ") " << name << " has order " << k
            //        << std::endl;
            //           << " offset " << offsets[k]
            //           << " xs " << xs[k] << " ys "
            //           << ys[k] << " sampling "
            //           << xsampling[k] << ", " << ysampling[k] << std::endl;

            //
            // When idx == 4 or idx == 5, we are dealing with RGBAZ in one layer
            // so get Z channel and save it in same layer.
            //
            if ( idx == 4 || idx == 5 ) {
                std::string c;
                if ( layer[0] == '#' )
                    c = layer + '.' + name;
                else
                    c = name;

                p->channel( c.c_str() );

                pic = img->hires();

                channels = pic->channels();
                if ( channels != 1 )
                {
                    const char* ch = p->channel();
                    LOG_ERROR( "Failed setting layer to " << c
                               << " returned " << ch );
                }

                dw = pic->width();
                dh = pic->height();
                total_size = dw*dh*size; // *1 (channels)

                base = (uint8_t*) new uint8_t[total_size];
                bufs.push_back( base );

                copy_pixel_data( pic, save_type, base, total_size, false );

                int start = (int)( (-dx - dy * dw) * size );
                base += start;

                xs[k] = size;
                ys[k] = size * dw;
            }


            char* buf =  (char*) base + offsets[k] * size;
            fb.insert( name,
                       Slice( save_type, buf, xs[k], ys[k],
                              xsampling[k], ysampling[k] ) );

        }



    }

    assert( headers.size() == fbs.size() );
    assert( fbs.size() == numParts );

    try {
        MultiPartOutputFile multi( file, &(headers[0]),
                                   (int)headers.size() );
        for ( unsigned part = 0; part < numParts; ++part )
        {
            OutputPart out( multi, part );
            const Header& h = multi.header(part);
            const Box2i& daw = h.dataWindow();
            out.setFrameBuffer( fbs[part] );
            out.writePixels( daw.max.y - daw.min.y + 1 );
        }
    }
    catch( std::exception& e )
    {
        LOG_ERROR( e.what() );
        return false;
    }

    Buffers::iterator i = bufs.begin();
    Buffers::iterator e = bufs.end();
    for ( ; i != e; ++i )
    {
        delete [] *i;
    }
    bufs.clear();
    fbs.clear();
    headers.clear();

    p->channel( old_channel.c_str() );

    return true;
}

} // namespace mrv
