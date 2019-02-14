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
 * @file   CMedia.cpp
 * @author gga
 * @date   Sat Aug 25 00:14:54 2007
 *
 * @brief
 *
 *
 */

#include <sys/stat.h>

#if defined(WIN32) || defined(WIN64)
#  include <direct.h>
#  define getcwd _getcwd
#else
#  include <unistd.h>
#endif

#include <cstdio>     // for snprintf
#include <cassert>    // for assert


extern "C" {
#include <libavutil/time.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}



#include <iostream>
#include <algorithm>  // for std::min, std::abs
#include <limits>

#include <thread>
#include <mutex>

#include <fltk/run.h>

#undef  __STDC_CONSTANT_MACROS

#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/diagnostic_information.hpp>
namespace fs = boost::filesystem;

#include <ImfBoxAttribute.h>
#include <ImfChromaticitiesAttribute.h>
#include <ImfDoubleAttribute.h>
#include <ImfFloatAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfKeyCodeAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfRationalAttribute.h>
#include <ImfStringAttribute.h>
#include <ImfTimeCodeAttribute.h>
#include <ImfVecAttribute.h>

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

#include <MagickWand/MagickWand.h>

#include "core/mrvMath.h"
#include "core/CMedia.h"
#include "core/aviImage.h"
#include "core/Sequence.h"
#include "core/mrvFrameFunctors.h"
#include "core/mrvPlayback.h"
#include "core/mrvColorProfile.h"
#include "core/mrvException.h"
#include "core/mrvThread.h"
#include "core/mrvI8N.h"
#include "core/mrvOS.h"
#include "core/mrvTimer.h"
#include "gui/mrvIO.h"
#include "gui/mrvPreferences.h"

namespace {

const char* kModule = "img";

}


#undef DBG
#define DBG(x)

// #define DEBUG_SEEK
// #define DEBUG_VIDEO_PACKETS
//#define DEBUG_STORES

// #define DEBUG_DECODE
// #define DEBUG_AUDIO_SPLIT




namespace mrv {

static AVRational timeBaseQ = { 1, AV_TIME_BASE };

bool       CMedia::_initialize = false;

unsigned    CMedia::_audio_max = 0;
bool        CMedia::_supports_yuv = false;
bool        CMedia::_supports_yuva = false;
bool        CMedia::_uses_16bits = false;
CMedia::LoadLib CMedia::load_library = CMedia::kFFMPEGLibrary;

double      CMedia::default_fps = 24.f;

std::string CMedia::ocio_8bits_ics;
std::string CMedia::ocio_16bits_ics;
std::string CMedia::ocio_32bits_ics;
std::string CMedia::ocio_float_ics;

std::string CMedia::rendering_transform_8bits;
std::string CMedia::rendering_transform_16bits;
std::string CMedia::rendering_transform_32bits;
std::string CMedia::rendering_transform_float;


std::string CMedia::icc_profile_8bits;
std::string CMedia::icc_profile_16bits;
std::string CMedia::icc_profile_32bits;
std::string CMedia::icc_profile_float;


int CMedia::_audio_cache_size = 0;
int CMedia::_video_cache_size = 0;
int CMedia::_image_cache_size = 0;

std::string CMedia::_default_subtitle_font = "Arial";
std::string CMedia::_default_subtitle_encoding = "iso-8859-1";
bool CMedia::_aces_metadata = false;
bool CMedia::_all_layers = false;
bool CMedia::_cache_active = true;
bool CMedia::_preload_cache = true;
bool CMedia::_8bit_cache = false;
int  CMedia::_cache_scale = 0;

static const char* const kDecodeStatus[] = {
    _("Decode Missing Frame"),
    _("Decode OK"),
    _("Decode Done"),
    _("Decode Error"),
    _("Decode Missing Samples"),
    _("Decode No Stream"),
    _("Decode Loop Start"),
    _("Decode Loop End"),
    _("Decode Buffer Full"),
};


const char* const CMedia::decode_error( DecodeStatus err )
{
    return _( kDecodeStatus[err] );
}

std::string CMedia::attr2str( const Imf::Attribute* attr )
{
    std::string r;
    char buf[256];

    const Imf::Box2iAttribute* b2i =
        dynamic_cast< const Imf::Box2iAttribute* >( attr );
    const Imf::Box2fAttribute* b2f =
        dynamic_cast< const Imf::Box2fAttribute* >( attr );
    const Imf::KeyCodeAttribute* kc =
        dynamic_cast< const Imf::KeyCodeAttribute* >( attr );
    const Imf::TimeCodeAttribute* tm =
        dynamic_cast< const Imf::TimeCodeAttribute* >( attr );
    const Imf::M33dAttribute* m33d =
        dynamic_cast< const Imf::M33dAttribute* >( attr );
    const Imf::M33fAttribute* m33f =
        dynamic_cast< const Imf::M33fAttribute* >( attr );
    const Imf::M44dAttribute* m44d =
        dynamic_cast< const Imf::M44dAttribute* >( attr );
    const Imf::M44fAttribute* m44f =
        dynamic_cast< const Imf::M44fAttribute* >( attr );
    const Imf::V3dAttribute* v3d =
        dynamic_cast< const Imf::V3dAttribute* >( attr );
    const Imf::V3fAttribute* v3f =
        dynamic_cast< const Imf::V3fAttribute* >( attr );
    const Imf::V3iAttribute* v3i =
        dynamic_cast< const Imf::V3iAttribute* >( attr );
    const Imf::RationalAttribute* rat =
        dynamic_cast< const Imf::RationalAttribute* >( attr );
    const Imf::V2dAttribute* v2d =
        dynamic_cast< const Imf::V2dAttribute* >( attr );
    const Imf::V2fAttribute* v2f =
        dynamic_cast< const Imf::V2fAttribute* >( attr );
    const Imf::V2iAttribute* v2i =
        dynamic_cast< const Imf::V2iAttribute* >( attr );
    const Imf::Box2iAttribute* box2i =
        dynamic_cast< const Imf::Box2iAttribute* >( attr );
    const Imf::Box2fAttribute* box2f =
        dynamic_cast< const Imf::Box2fAttribute* >( attr );
    const Imf::IntAttribute* intg =
        dynamic_cast< const Imf::IntAttribute* >( attr );
    const Imf::FloatAttribute* flt =
        dynamic_cast< const Imf::FloatAttribute* >( attr );
    const Imf::DoubleAttribute* dbl =
        dynamic_cast< const Imf::DoubleAttribute* >( attr );
    const Imf::StringAttribute* str =
        dynamic_cast< const Imf::StringAttribute* >( attr );

    if ( b2i )
    {
        const Imath::Box2i& t = b2i->value();
        sprintf( buf, "%d %d  %d %d", t.min.x, t.min.y, t.max.x, t.max.y );
        r = buf;
    }
    else if ( b2f )
    {
        const Imath::Box2f& t = b2f->value();
        sprintf( buf, "%g %g  %g %g", t.min.x, t.min.y, t.max.x, t.max.y );
        r = buf;
    }
    else if ( kc )
    {
        const Imf::KeyCode& k = kc->value();
        sprintf( buf, "%d %d %d %d %d %d %d", k.filmMfcCode(),
                 k.filmType(), k.prefix(), k.count(), k.perfOffset(),
                 k.perfsPerFrame(), k.perfsPerCount() );
        r = buf;
    }
    else if ( tm )
    {
        const Imf::TimeCode& t = tm->value();
        if ( t.dropFrame() )
        {
            sprintf( buf, "%02d;%02d;%02d;%02d", t.hours(),
                     t.minutes(), t.seconds(), t.frame() );
        }
        else
        {
            sprintf( buf, "%02d:%02d:%02d:%02d", t.hours(),
                     t.minutes(), t.seconds(), t.frame() );
        }
        r = buf;
    }
    else if ( m33d )
    {
        const Imath::M33d& m = m33d->value();
        sprintf( buf, "%lg %lg %lg %lg %lg %lg %lg %lg %lg",
                 m[0][0], m[0][1], m[0][2],
                 m[1][0], m[1][1], m[1][2],
                 m[2][0], m[2][1], m[2][2] );
        r = buf;
    }
    else if ( m33f )
    {
        const Imath::M33f& m = m33f->value();
        sprintf( buf, "%g %g %g %g %g %g %g %g %g",
                 m[0][0], m[0][1], m[0][2],
                 m[1][0], m[1][1], m[1][2],
                 m[2][0], m[2][1], m[2][2] );
        r = buf;
    }
    else if ( m44d )
    {
        const Imath::M44d& m = m44d->value();
        sprintf( buf, "%lg %lg %lg %lg %lg %lg %lg %lg "
                 "%lg %lg %lg %lg %lg %lg %lg %lg",
                 m[0][0], m[0][1], m[0][2], m[0][3],
                 m[1][0], m[1][1], m[1][2], m[1][3], m[2][0], m[2][1],
                 m[2][2], m[2][3], m[3][0], m[3][1], m[3][2], m[3][3]);
        r = buf;
    }
    else if ( m44f )
    {
        const Imath::M44f& m = m44f->value();
        sprintf( buf, "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
                 m[0][0], m[0][1], m[0][2], m[0][3],
                 m[1][0], m[1][1], m[1][2], m[1][3], m[2][0], m[2][1],
                 m[2][2], m[2][3], m[3][0], m[3][1], m[3][2], m[3][3]);
        r = buf;
    }
    else if ( v3d )
    {
        const Imath::V3d& v = v3d->value();
        sprintf( buf, "%lg %lg %lg", v.x, v.y, v.z );
        r = buf;
    }
    else if ( v3f )
    {
        const Imath::V3f& v = v3f->value();
        sprintf( buf, "%g %g %g", v.x, v.y, v.z );
        r = buf;
    }
    else if ( v3i )
    {
        const Imath::V3i& v = v3i->value();
        char buf[64];
        sprintf( buf, "%d %d %d", v.x, v.y, v.z );
        r = buf;
    }
    else if ( rat )
    {
        const Imf::Rational& rt = rat->value();
        sprintf( buf, "%d/%d", rt.n, rt.d );
        r = buf;
    }
    else if ( v2d )
    {
        const Imath::V2d& v = v2d->value();
        char buf[64];
        sprintf( buf, "%lg %lg", v.x, v.y );
        r = buf;
    }
    else if ( v2f )
    {
        const Imath::V2f& v = v2f->value();
        sprintf( buf, "%g %g", v.x, v.y );
        r = buf;
    }
    else if ( v2i )
    {
        const Imath::V2i& v = v2i->value();
        sprintf( buf, "%d %d", v.x, v.y );
        r = buf;
    }
    else if ( intg )
    {
        sprintf( buf, "%d", intg->value() );
        r = buf;
    }
    else if ( dbl )
    {
        sprintf( buf, "%.8lg", dbl->value() );
        r = buf;
    }
    else if ( flt )
    {
        sprintf( buf, "%.8g", flt->value() );
        r = buf;
    }
    else if ( str )
    {
        r = str->value();
    }
    return r;
}


/**
 * Constructor
 *
 */
CMedia::CMedia() :
    av_sync_type( CMedia::AV_SYNC_AUDIO_MASTER ),
    _has_deep_data( false ),
    _w( 0 ),
    _h( 0 ),
    _internal( false ),
    _is_thumbnail( false ),
    _is_sequence( false ),
    _is_stereo( false ),
    _stereo_input( kSeparateLayersInput ),
    _stereo_output( kNoStereo ),
    _looping( kUnknownLoop ),
    _fileroot( NULL ),
    _filename( NULL ),
    _ctime( 0 ),
    _mtime( 0 ),
    _disk_space( 0 ),
    _colorspace_index( -1 ),
    _avdiff( 0.0 ),
    _loop_barrier( NULL ),
    _stereo_barrier( NULL ),
    _fg_bg_barrier( NULL ),
    _audio_start( true ),
    _seek_req( false ),
    _seek_frame( 1 ),
    _pos( 1 ),
    _channel( NULL ),
    _label( NULL ),
    _real_fps( 0 ),
    _play_fps( 0 ),
    _fps( 0 ),
    _orig_fps( 0 ),
    _pixel_ratio( 1.0f ),
    _num_channels( 0 ),
    _rendering_intent( kUndefinedIntent ),
    _gamma( 1 ),
    _has_chromaticities( false ),
    _dts( 1 ),
    _adts( 1 ),
    _audio_frame( 1 ),
    _audio_offset( 0 ),
    _frame( AV_NOPTS_VALUE ),
    _tc_frame( 0 ),
    _expected( 1 ),
    _expected_audio( 1 ),
    _frameStart( 1 ),
    _frameEnd( 1 ),
    _frame_start( 1 ),
    _frame_end( 1 ),
    _start_number( 0 ),
    _loop_start( 1 ),
    _audio_pts( 0 ),
    _audio_clock( double( av_gettime_relative() )/ 1000000.0 ),
    _video_pts( 0 ),
    _video_clock( double( av_gettime_relative() ) / 1000000.0 ),
    _interlaced( kNoInterlace ),
    _image_damage( kNoDamage ),
    _damageRectangle( 0, 0, 0, 0 ),
    _x( 0 ),
    _y( 0 ),
    _scale_x( 1.0 ),
    _scale_y( 1.0 ),
    _rot_z( 0 ),
    _dataWindow( NULL ),
    _displayWindow( NULL ),
    _dataWindow2( NULL ),
    _displayWindow2( NULL ),
    _is_left_eye( true ),
    _right_eye( NULL ),
    _eye_separation( 0.0f ),
    _profile( NULL ),
    _rendering_transform( NULL ),
    _idt_transform( NULL ),
    _frame_offset( 0 ),
    _playback( kStopped ),
    _sequence( NULL ),
    _right( NULL ),
    _context(NULL),
    _video_ctx( NULL ),
    _acontext(NULL),
    _audio_ctx( NULL ),
    _audio_codec(NULL),
    _subtitle_index(-1),
    _subtitle_encoding( strdup( _default_subtitle_encoding.c_str() ) ),
    _subtitle_font( strdup( _default_subtitle_font.c_str() ) ),
    _last_audio_cached( false ),
    _audio_index(-1),
    _samples_per_sec( 0 ),
    _audio_buf_used( 0 ),
    _audio_last_frame( 0 ),
    _audio_channels( 0 ),
    _aframe( NULL ),
    audio_callback_time( 0 ),
    _audio_format( AudioEngine::kFloatLSB ),
    _audio_buf( NULL ),
    forw_ctx( NULL ),
    _audio_engine( NULL )
{
    _aframe = av_frame_alloc();

    audio_initialize();
    if ( ! _initialize )
    {
        mrv::PacketQueue::initialize();
        _initialize = true;
    }
    // std::cerr << "CMedia " << this << std::endl;
}


/**
 * Copy Constructor.  Copies an image, optionally resizing it.
 *
 * @param other original image
 * @param ws    new image width
 * @param wh    new image height
 */
CMedia::CMedia( const CMedia* other, int ws, int wh ) :
    av_sync_type( other->av_sync_type ),
    _has_deep_data( other->_has_deep_data ),
    _w( 0 ),
    _h( 0 ),
    _is_stereo( false ),
    _stereo_input( kSeparateLayersInput ),
    _stereo_output( kNoStereo ),
    _looping( kUnknownLoop ),
    _is_thumbnail( false ),
    _is_sequence( false ),
    _fileroot( NULL ),
    _filename( NULL ),
    _ctime( 0 ),
    _mtime( 0 ),
    _disk_space( 0 ),
    _loop_barrier( NULL ),
    _stereo_barrier( NULL ),
    _fg_bg_barrier( NULL ),
    _audio_start( true ),
    _seek_req( false ),
    _seek_frame( 1 ),
    _pos( 1 ),
    _channel( NULL ),
    _label( NULL ),
    _real_fps( 0 ),
    _play_fps( 0 ),
    _fps( 0 ),
    _orig_fps( 0 ),
    _pixel_ratio( 1.0f ),
    _num_channels( 0 ),
    _rendering_intent( kUndefinedIntent ),
    _gamma( 1 ),
    _has_chromaticities( false ),
    _dts( 1 ),
    _adts( 1 ),
    _audio_frame( 1 ),
    _audio_offset( 0 ),
    _frame( AV_NOPTS_VALUE ),
    _tc_frame( 0 ),
    _expected( 1 ),
    _expected_audio( 1 ),
    _frameStart( 1 ),
    _frameEnd( 1 ),
    _frame_start( 1 ),
    _frame_end( 1 ),
    _start_number( 0 ),
    _loop_start( 1 ),
    _interlaced( kNoInterlace ),
    _image_damage( kNoDamage ),
    _damageRectangle( 0, 0, 0, 0 ),
    _x( 0 ),
    _y( 0 ),
    _scale_x( 1.0 ),
    _scale_y( 1.0 ),
    _rot_z( 0 ),
    _dataWindow( NULL ),
    _displayWindow( NULL ),
    _dataWindow2( NULL ),
    _displayWindow2( NULL ),
    _is_left_eye( true ),
    _right_eye( NULL ),
    _eye_separation( 0.0f ),
    _profile( NULL ),
    _rendering_transform( NULL ),
    _idt_transform( NULL ),
    _playback( kStopped ),
    _sequence( NULL ),
    _right( NULL ),
    _context(NULL),
    _video_ctx( NULL ),
    _acontext(NULL),
    _audio_ctx( NULL ),
    _audio_codec(NULL),
    _subtitle_index(-1),
    _subtitle_encoding( strdup( _default_subtitle_encoding.c_str() ) ),
    _subtitle_font( strdup( _default_subtitle_font.c_str() ) ),
    _last_audio_cached( false ),
    _audio_index(-1),
    _samples_per_sec( 0 ),
    _audio_buf_used( 0 ),
    _audio_last_frame( 0 ),
    _audio_channels( other->_audio_channels.load() ),
    _aframe( NULL ),
    _audio_format( other->_audio_format.load() ),
    _audio_buf( NULL ),
    forw_ctx( NULL ),
    _audio_engine( NULL )
{
    _aframe = av_frame_alloc();
    unsigned int W = other->width();
    unsigned int H = other->height();
    image_size( W, H );

    float xScale = (float)ws / (float)W;
    float yScale = (float)wh / (float)H;

    {
        CMedia* img = const_cast< CMedia* >( other );
        CMedia::Mutex& m = img->video_mutex();
        SCOPED_LOCK(m);
        _hires = img->hires();

        if ( xScale != 1.0f || yScale != 1.0f )
        {
            _hires.reset( _hires->scaleX( xScale ) );
            _hires.reset( _hires->scaleY( yScale ) );
            _w = _hires->width();
            _h = _hires->height();
            _damageRectangle = mrv::Recti(0, 0, _w, _h);
        }
    }
}

CMedia::CMedia( const CMedia* other, int64_t f ) :
    av_sync_type( other->av_sync_type ),
    _has_deep_data( other->_has_deep_data ),
    _w( 0 ),
    _h( 0 ),
    _is_stereo( other->_is_stereo ),
    _stereo_input( other->_stereo_input ),
    _stereo_output( other->_stereo_output ),
    _looping( other->looping() ),
    _is_thumbnail( other->_is_thumbnail ),
    _is_sequence( other->_is_sequence ),
    _fileroot( NULL ),
    _filename( NULL ),
    _ctime( other->_ctime ),
    _mtime( other->_mtime ),
    _disk_space( 0 ),
    _loop_barrier( NULL ),
    _stereo_barrier( NULL ),
    _fg_bg_barrier( NULL ),
    _audio_start( true ),
    _seek_req( false ),
    _seek_frame( 1 ),
    _pos( 1 ),
    _channel( NULL ),
    _label( NULL ),
    _real_fps( 0 ),
    _play_fps( other->_play_fps.load() ),
    _fps( other->_fps.load() ),
    _orig_fps( other->_orig_fps.load() ),
    _pixel_ratio( other->_pixel_ratio ),
    _num_channels( other->_num_channels ),
    _rendering_intent( other->_rendering_intent ),
    _gamma( other->_gamma ),
    _has_chromaticities( other->has_chromaticities() ),
    _chromaticities( other->chromaticities() ),
    _dts( other->_dts.load() ),
    _adts( other->_adts.load() ),
    _audio_frame( other->_audio_frame.load() ),
    _audio_offset( other->_audio_offset ),
    _frame( f ),
    _tc_frame( other->_tc_frame ),
    _expected( f+1 ),
    _expected_audio( 0 ),
    _frameStart( other->_frameStart ),
    _frameEnd( other->_frameEnd ),
    _frame_start( other->_frame_start ),
    _frame_end( other->_frame_end ),
    _start_number( other->_start_number ),
    _interlaced( other->_interlaced ),
    _image_damage( kNoDamage ),
    _damageRectangle( 0, 0, 0, 0 ),
    _x( 0 ),
    _y( 0 ),
    _scale_x( 1.0 ),
    _scale_y( 1.0 ),
    _rot_z( 0 ),
    _dataWindow( NULL ),
    _displayWindow( NULL ),
    _dataWindow2( NULL ),
    _displayWindow2( NULL ),
    _is_left_eye( other->_is_left_eye ),
    _right_eye( NULL ),
    _eye_separation( 0.0f ),
    _profile( NULL ),
    _rendering_transform( NULL ),
    _idt_transform( NULL ),
    _playback( kStopped ),
    _sequence( NULL ),
    _right( NULL ),
    _context(NULL),
    _video_ctx( NULL ),
    _acontext(NULL),
    _audio_ctx( NULL ),
    _audio_codec(NULL),
    _subtitle_index(-1),
    _subtitle_encoding( strdup( other->_subtitle_encoding ) ),
    _subtitle_font( strdup( other->_subtitle_font ) ),
    _last_audio_cached( false ),
    _audio_index( other->_audio_index ),
    _samples_per_sec( 0 ),
    _audio_buf_used( 0 ),
    _audio_last_frame( 0 ),
    _audio_channels( 0 ),
    _aframe( NULL ),
    _audio_format( AudioEngine::kFloatLSB ),
    _audio_buf( NULL ),
    forw_ctx( NULL ),
    _audio_engine( NULL )
{

    _aframe = av_frame_alloc();
    _fileroot = strdup( other->fileroot() );
    _filename = strdup( other->filename() );

    // TRACE( "copy constructor" );

    audio_initialize();
    if ( ! _initialize )
    {
        mrv::PacketQueue::initialize();
        _initialize = true;
    }

    if ( other->audio_file().empty() )
        audio_file( NULL );
    else
        audio_file( other->audio_file().c_str() );
    _audio_offset = other->audio_offset() + f - 1;
    audio_stream( other->_audio_index );

    image_type_ptr canvas;
    if ( fetch( canvas, f ) )
    {
        _hires = canvas;
        cache( canvas );
        default_color_corrections();
    }
}

int64_t CMedia::get_frame( const AVStream* stream, const AVPacket& pkt )
{
    assert( stream != NULL );
    int64_t frame = AV_NOPTS_VALUE;
    if ( pkt.pts != AV_NOPTS_VALUE )
    {
        frame = pts2frame( stream, pkt.pts );
    }
    else
    {
        if ( pkt.dts != AV_NOPTS_VALUE )
            frame = pts2frame( stream, pkt.dts );
    }
    return frame;
}


/**
 * Clears cache of frames.  @todo: refactor
 *
 */
void CMedia::clear_cache()
{
    if ( !_sequence ) return;

    SCOPED_LOCK( _mutex);

    boost::uint64_t num = _frame_end - _frame_start + 1;
    for ( boost::uint64_t i = 0; i < num; ++i )
    {
        if ( _sequence && _sequence[i] )
            _sequence[i].reset();
        if ( _right && _right[i] )
            _right[i].reset();
    }

    _stereo[0].reset();
    _stereo[1].reset();

    image_damage( image_damage() | kDamageCache | kDamageContents );

}

void CMedia::update_frame( const int64_t& f )
{
    if ( !_sequence ) return;

    if ( f < _frame_start || f > _frame_end ) return;

    SCOPED_LOCK( _mutex);

    boost::uint64_t i = f - _frame_start;
    if ( _sequence[i] )        _sequence[i].reset();
    if ( _right && _right[i] )    _right[i].reset();

    _hires.reset();
    _stereo[0].reset();
    _stereo[1].reset();

    image_type_ptr canvas;
    if ( fetch( canvas, f ) )
    {
        _hires = canvas;
        cache( canvas );
        default_color_corrections();
    }

    image_damage( image_damage() | kDamageCache | kDamageContents );

}

/**
 */
void CMedia::wait_for_load_threads()
{
}

/**
 * Wait for all threads to finish and exit.  Delete them afterwards.
 *
 */
void CMedia::wait_for_threads()
{
    for ( const auto& i : _threads )
    {
        i->join();
        delete i;
    }

    _threads.clear();
}

/**
 * Destructor
 *
 */
CMedia::~CMedia()
{

    SCOPED_LOCK( _mutex );
    SCOPED_LOCK( _audio_mutex );
    SCOPED_LOCK( _subtitle_mutex );


    if ( !stopped() )
        stop();

    delete _right_eye;
    _right_eye = NULL;

    image_damage( kNoDamage );

    free( _channel );
    free( _label );
    free( _profile );
    free( _rendering_transform );

    clear_look_mod_transform();
    free( _idt_transform );

    delete [] _dataWindow;
    _dataWindow = NULL;
    delete [] _dataWindow2;
    _dataWindow2 = NULL;
    delete [] _displayWindow;
    _displayWindow = NULL;
    delete [] _displayWindow2;
    _displayWindow2 = NULL;

    _hires.reset();

    clear_cache();
    delete [] _sequence;
    _sequence = NULL;
    delete [] _right;
    _right = NULL;



    delete [] _audio_buf;
    _audio_buf = NULL;

    if ( has_audio() )
    {
        close_audio();

        close_audio_codec();

    }

    audio_shutdown();

    if ( forw_ctx )
    {
        swr_free( &forw_ctx );
        forw_ctx = NULL;
    }

    flush_audio();

    if ( _aframe )
    {
        av_frame_unref(_aframe);
        av_frame_free(&_aframe);
    }

    free( _fileroot );
    _fileroot = NULL;
    free( _filename );
    _filename = NULL;

    if ( _context )
    {
        avformat_close_input( &_context );
    }

    if ( _acontext )
    {
        avformat_close_input( &_acontext );
    }

    free( _subtitle_font );
    _subtitle_font = NULL;
    free( _subtitle_encoding);
    _subtitle_encoding = NULL;

    {
        for ( const auto& i : _attrs )
        {
            delete i.second;
        }
    }

    _context = _acontext = NULL;
}


void CMedia::hires( const mrv::image_type_ptr pic)
{
    _hires = pic;
    _frame = pic->frame();
    _w = pic->width();
    _h = pic->height();
    refresh();
}

int CMedia::from_stereo_output( CMedia::StereoOutput x )
{
    switch( x )
    {
    case kStereoLeft:
        return 1;
    case kStereoRight:
        return 2;
    case kStereoOpenGL:
        return 3;
    case kStereoTopBottom:
        return 4;
    case kStereoBottomTop:
        return 5;
    case kStereoSideBySide:
        return 6;
    case kStereoCrossed:
        return 7;
    case kStereoInterlaced:
        return 8;
    case kStereoInterlacedColumns:
        return 9;
    case kStereoCheckerboard:
        return 10;
    case kStereoAnaglyph:
        return 11;
    case kStereoRightAnaglyph:
        return 12;
    default:
        return 0;
    }
}

CMedia::StereoOutput CMedia::to_stereo_output( int x )
{
    switch( x )
    {
    case 1:
        return kStereoLeft;
    case 2:
        return kStereoRight;
    case 3:
        return kStereoOpenGL;
    case 4:
        return kStereoTopBottom;
    case 5:
        return kStereoBottomTop;
    case 6:
        return kStereoSideBySide;
    case 7:
        return kStereoCrossed;
    case 8:
        return kStereoInterlaced;
    case 9:
        return kStereoInterlacedColumns;
    case 10:
        return kStereoCheckerboard;
    case 11:
        return kStereoAnaglyph;
    case 12:
        return kStereoRightAnaglyph;
    case 0:
    default:
        return kNoStereo;
    }
}

int CMedia::from_stereo_input( CMedia::StereoInput x )
{
    switch( x )
    {
    case kSeparateLayersInput:
        return 0;
    case kTopBottomStereoInput:
        return 1;
    case kLeftRightStereoInput:
        return 2;
    case kNoStereoInput:
    default:
        return 0;
    }
}

CMedia::StereoInput CMedia::to_stereo_input( int x )
{
    switch( x )
    {
    case 0:
        return kSeparateLayersInput;
    case 1:
        return kTopBottomStereoInput;
    case 2:
        return kLeftRightStereoInput;
    default:
        return kNoStereoInput;
    }
}

/**
 * Allocate the float pixels for the image
 *
 * @param canvas canvas where allocated picture will be stored.
 * @param frame image frame this picture belongs to.
 * @param channels number of channels in this image.
 * @param format   format of image (RGBA, A, Luma, YUV, etc).
 * @param pixel_type  pixel type of image (8 bits, 16 bits, half, float).
 * @param w image width in pixels.
 * @param h image height in pixels.
 *
 * @returns true for success, false for failure
 */
bool CMedia::allocate_pixels( image_type_ptr& canvas,
                              const int64_t& frame,
                              const unsigned short channels,
                              const image_type::Format format,
                              const image_type::PixelType pixel_type,
                              unsigned w, unsigned h)
{
    if ( w == 0 )
    {
        w = width();
        h = height();
    }
    assert( w != 0 && h != 0 );
    // LOG_INFO( "allocate pixels " << w << " " << h << " frame: " << frame
    //           << " channels: " << channels << " format: "
    //           << format << " pixel type: " << pixel_type );

    image_damage( image_damage() & ~kDamageContents );
    try {
        canvas.reset( new image_type( frame, w, h,
                                      channels, format, pixel_type ) );
    }
    catch( const std::bad_alloc& e )
    {
        LOG_ERROR( e.what() );
        return false;
    }
    catch( const std::runtime_error& e )
    {
        LOG_ERROR( e.what() );
        return false;
    }

    if (! canvas->data().get() )
    {
        IMG_ERROR( _("Out of memory") );
        return false;
    }

    return true;
}


mrv::image_type_ptr CMedia::left() const
{
    int64_t f = handle_loops( _frame );
    int64_t idx = f - _frame_start;

    int64_t num = _frame_end - _frame_start + 1;
    if ( idx > num ) idx = num - 1;
    else if ( idx < 0 ) idx = 0;

    Mutex& mtx = const_cast< Mutex& >( _mutex );
    SCOPED_LOCK( mtx );
    CMedia* img = const_cast< CMedia* >( this );
    mrv::image_type_ptr pic = _hires;

    if ( _is_sequence && _sequence && _sequence[idx] )  pic = _sequence[idx];
    else if ( _stereo[0] )  pic = _stereo[0];
    if ( !pic ) return pic;

    img->_w = pic->width();
    img->_h = pic->height();
    return pic;
}

mrv::image_type_ptr CMedia::right() const
{
    int64_t f = handle_loops( _frame );
    int64_t idx = f - _frame_start;
    if ( _right_eye ) {
        return _right_eye->left();
    }

    Mutex& mtx = const_cast< Mutex& >( _mutex );
    SCOPED_LOCK( mtx );

    if ( stereo_input() == kTopBottomStereoInput ||
            stereo_input() == kLeftRightStereoInput )
        return _stereo[0] ? _stereo[0] : _hires;

    int64_t num = _frame_end - _frame_start + 1;

    if ( idx >= num )   idx = num - 1;
    else if ( idx < 0 ) idx = 0;

    if ( _is_sequence && _right && _right[idx] )
    {
        return _right[idx];
    }
    else
    {
        return _stereo[1];
    }
}






const mrv::Recti CMedia::display_window( int64_t f ) const
{
    int dw = width();
    int dh = height();

    int64_t num = _frame_end - _frame_start + 1;

    if ( !_displayWindow || num == 0 )
    {
        if ( stereo_input() & kTopBottomStereoInput ) dh /= 2;
        else if ( stereo_input() & kLeftRightStereoInput ) dw /= 2;
        return mrv::Recti( 0, 0, dw, dh );
    }

    if ( f == AV_NOPTS_VALUE ) f = _frame;

    f = handle_loops( f );
    int64_t idx = f - _frame_start;


    if ( idx >= num )   idx = num - 1;
    else if ( idx < 0 ) idx = 0;

    Mutex& mtx = const_cast< Mutex& >( _data_mutex );
    SCOPED_LOCK( mtx );
    if ( stereo_input() & kTopBottomStereoInput ) {
        mrv::Recti r = _displayWindow[idx];
        r.h( r.h() / 2 );
        return r;
    }
    else if ( stereo_input() & kLeftRightStereoInput ) {
        mrv::Recti r = _displayWindow[idx];
        r.w( r.w() / 2 );
        return r;
    }
    assert( idx < num );
    return _displayWindow[idx];
}

const mrv::Recti CMedia::display_window2( int64_t f ) const
{
    if ( _right_eye )
        return _right_eye->display_window(f);

    int dw = width();
    int dh = height();

    int64_t num = _frame_end - _frame_start + 1;

    if ( !_displayWindow2 || num == 0 )
    {
        if ( stereo_input() & kTopBottomStereoInput ) {
            dh /= 2;
        }
        else if ( stereo_input() & kLeftRightStereoInput ) {
            dw /= 2;
        }
        return mrv::Recti( 0, 0, dw, dh );
    }

    if ( f == AV_NOPTS_VALUE ) f = _frame;

    f = handle_loops( f );
    int64_t idx = f - _frame_start;


    if ( idx >= num )   idx = num - 1;
    else if ( idx < 0 ) idx = 0;

    Mutex& mtx = const_cast< Mutex& >( _data_mutex );
    SCOPED_LOCK( mtx );
    if ( stereo_input() & kTopBottomStereoInput ) {
        mrv::Recti r = _displayWindow2[idx];
        r.h( r.h() / 2 );
        return r;
    }
    else if ( stereo_input() & kLeftRightStereoInput ) {
        mrv::Recti r = _displayWindow2[idx];
        r.w( r.w() / 2 );
        return r;
    }
    assert( idx < num );
    return _displayWindow2[idx];
}

const mrv::Recti CMedia::data_window( int64_t f ) const
{
    int dw = width();
    int dh = height();

    int64_t num = _frame_end - _frame_start + 1;

    if ( !_dataWindow || num == 0 )
    {
        if ( stereo_input() & kTopBottomStereoInput )      dh /= 2;
        else if ( stereo_input() & kLeftRightStereoInput ) dw /= 2;
        return mrv::Recti( 0, 0, dw, dh );
    }

    if ( f == AV_NOPTS_VALUE ) f = _frame;

    f = handle_loops( f );
    int64_t idx = f - _frame_start;


    if ( idx >= num )   idx = num - 1;
    else if ( idx < 0 ) idx = 0;

    Mutex& mtx = const_cast< Mutex& >( _data_mutex );
    SCOPED_LOCK( mtx );

    if ( stereo_input() & kTopBottomStereoInput ) {
        mrv::Recti r = _dataWindow[idx];
        r.h( r.h() / 2 );
        return r;
    }
    else if ( stereo_input() & kLeftRightStereoInput ) {
        mrv::Recti r = _dataWindow[idx];
        r.w( r.w() / 2 );
        return r;
    }
    return _dataWindow[idx];
}

const mrv::Recti CMedia::data_window2( int64_t f ) const
{
    if ( _right_eye )
        return _right_eye->data_window(f);

    int64_t num = _frame_end - _frame_start + 1;

    int dw = width();
    int dh = height();
    if ( !_dataWindow2 || num == 0 )
    {
        if ( stereo_input() & kTopBottomStereoInput )      {
            dh /= 2;
        }
        else if ( stereo_input() & kLeftRightStereoInput ) {
            dw /= 2;
        }
        return mrv::Recti( 0, 0, dw, dh );
    }

    if ( f == AV_NOPTS_VALUE ) f = _frame;

    f = handle_loops( f );
    int64_t idx = f - _frame_start;

    if ( idx >= num )   idx = num - 1;
    else if ( idx < 0 ) idx = 0;

    Mutex& mtx = const_cast< Mutex& >( _data_mutex );
    SCOPED_LOCK( mtx );

    if ( stereo_input() & kTopBottomStereoInput ) {
        mrv::Recti r = _dataWindow2[idx];
        r.h( r.h() / 2 );
        return r;
    }
    else if ( stereo_input() & kLeftRightStereoInput ) {
        mrv::Recti r = _dataWindow2[idx];
        r.w( r.w() / 2 );
        return r;
    }
    return _dataWindow2[idx];
}

void CMedia::display_window( const int xmin, const int ymin,
                             const int xmax, const int ymax,
                             const int64_t& frame )
{
    // assert( xmax >= xmin );
    // assert( ymax >= ymin );
    int64_t num = _frame_end - _frame_start + 1;
    if ( !_displayWindow )
        _displayWindow = new mrv::Recti[num];

    int64_t f = handle_loops( frame );
    int64_t idx = f - _frame_start;

    if ( idx >= num || idx < 0 ) return;

    // assert( idx < num );

    SCOPED_LOCK( _data_mutex );
    _displayWindow[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
    image_damage( image_damage() | kDamageData );
    DBG( "display window frame " << _frame << " is " << _displayWindow[idx] );
}

void CMedia::display_window2( const int xmin, const int ymin,
                              const int xmax, const int ymax,
                              const int64_t& frame )
{
    // assert( xmax >= xmin );
    // assert( ymax >= ymin );
    int64_t num = _frame_end - _frame_start + 1;
    if ( !_displayWindow2 )
        _displayWindow2 = new mrv::Recti[num];

    int64_t f = handle_loops( frame );
    int64_t idx = f - _frame_start;

    if ( idx >= num || idx < 0 ) return;

    // assert( idx < num );

    SCOPED_LOCK( _data_mutex );
    _displayWindow2[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
    image_damage( image_damage() | kDamageData );
    DBG( "display window2 frame " << _frame << " is " << _displayWindow2[idx] );
}

void CMedia::data_window( const int xmin, const int ymin,
                          const int xmax, const int ymax,
                          const int64_t& frame )
{
    // assert( xmax >= xmin );
    // assert( ymax >= ymin );
    int64_t num = _frame_end - _frame_start + 1;
    if ( !_dataWindow )
        _dataWindow = new mrv::Recti[num];

    int64_t f = handle_loops( frame );
    int64_t idx = f - _frame_start;

    if ( idx >= num || idx < 0 ) return;

    SCOPED_LOCK( _data_mutex );

    // assert( idx < num );

    _dataWindow[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
    image_damage( image_damage() | kDamageData );
    // DBG( "data window setframe " << _frame << " is " << _dataWindow[idx] );
}


void CMedia::data_window2( const int xmin, const int ymin,
                           const int xmax, const int ymax,
                           const int64_t& frame )
{
    // assert( xmax >= xmin );
    // assert( ymax >= ymin );
    int64_t num = _frame_end - _frame_start + 1;
    assert( _num > 0 );
    if ( !_dataWindow2 )
        _dataWindow2 = new mrv::Recti[num];

    int64_t f = handle_loops( frame );
    int64_t idx = f - _frame_start;

    if ( idx >= num || idx < 0 ) return;

    // assert( idx < num );
    SCOPED_LOCK( _data_mutex );
    _dataWindow2[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
    image_damage( image_damage() | kDamageData );
    DBG( "data window2 frame " << _frame << " is " << _dataWindow2[idx] );
}


/**
 * Tag a rectangle of the image for refresh
 *
 * @param r rectangle of the image to refresh.
 */
void CMedia::refresh( const mrv::Recti& r )
{
    // Merge the bounding box of area to update
    _damageRectangle.merge( r );
    image_damage( image_damage() | kDamageContents );
}


/**
 * Tag the whole image for refresh
 *
 */
void CMedia::refresh()
{
    refresh( mrv::Recti(0, 0, width(), height()) );
}



void  CMedia::first_frame(int64_t x)
{
    if ( x < _frame_start ) x = _frame_start;
    _frameStart = x;
    if ( _frame < _frameStart ) _frame = _frameStart;
}

void  CMedia::last_frame(int64_t x)
{
    if ( (!_is_sequence || !has_video()) && x > _frame_end ) x = _frame_end;
    _frameEnd = x;
    if ( _frame > _frameEnd ) _frame = _frameEnd;
}

/**
 * Treat image as a sequence of frames
 *
 * @param fileroot  a fileroot in C format like image.%d.exr
 * @param start     start frame
 * @param end       end   frame
 */
void CMedia::sequence( const char* fileroot,
                       const int64_t start,
                       const int64_t end,
                       const bool use_thread )
{

    SCOPED_LOCK( _mutex );

    assert( fileroot != NULL );
    assert( start <= end );

    if ( strncmp( fileroot, "file:", 5 ) == 0 )
        fileroot += 5;

    if ( _fileroot && strcmp( fileroot, _fileroot ) == 0 &&
            start == _frame_start && end == _frame_end )
        return;


    _fileroot = strdup( fileroot );

    std::string f = _fileroot;
    size_t idx = f.find( N_("%V") );
    if ( idx != std::string::npos )
    {
        _is_stereo = true;
    }

    free( _filename );
    _filename = NULL;


    _is_sequence = true;
    _dts = _adts = start;
    _frameStart = _frame_start = start;
    _frameEnd = _frame_end = end;


    delete [] _sequence;
    _sequence = NULL;
    delete [] _right;
    _right = NULL;

    boost::uint64_t num = _frame_end - _frame_start + 1;

    if ( dynamic_cast< aviImage* >( this ) == NULL )
    {
        _sequence = new mrv::image_type_ptr[ unsigned(num) ];
        _right    = new mrv::image_type_ptr[ unsigned(num) ];
    }

    if ( ! initialize() )
        return;

    image_type_ptr canvas;
    if ( fetch( canvas, start ) )
    {
        _hires = canvas;
        _depth = _hires->pixel_type();
        cache( canvas );
        refresh();
        default_color_corrections();
    }


    if ( has_audio() )
    {
        int64_t f = _frame.load();
        decode_audio( f );
    }
}

// Adds default OCIO, ICC, and CTL profiles
void CMedia::default_color_corrections()
{
    default_icc_profile();
    default_rendering_transform();
    default_ocio_input_color_space();
}

/**
 * Set a new filename for a single image. If dealing with an image sequence,
 * sequence() should be called instead.
 *
 * @param n new filename
 */
void CMedia::filename( const char* n )
{
    assert( n != NULL );

    if ( strncmp( n, "file:", 5 ) == 0 )
        n += 5;

    if ( _fileroot && strcmp( n, _fileroot ) == 0 )
        return;


    std::string name = n;
    if ( name.substr(0, 6) == "Slate " )
        name = name.substr(6, name.size() );


    fs::path file = fs::path( name );
    file = fs::absolute( file );

    if ( fs::exists( file ) )
    {
        std::string path = fs::canonical( file ).string();
        _fileroot = strdup( path.c_str() );
    }
    else
    {
        _fileroot = strdup( file.string().c_str() );
    }

    free( _filename );
    _filename = NULL;

    _is_sequence = false;
    _is_stereo = false;

    image_type_ptr canvas;
    if ( fetch( canvas, 1 ) )
    {
        _hires = canvas;
        _depth = canvas->pixel_type();
        default_color_corrections();
    }

    _dts = _adts = _frameStart = _frameEnd = _frame_start = _frame_end = 1;


    timestamp();

    if ( ! initialize() )
        return;



}


/**
 * Create and return the current filename of the image for the
 * current frame.
 *
 *
 * @return filename of the file for current frame
 */
const char* const CMedia::filename() const
{
    if ( !is_sequence() ) return _fileroot;
    if ( _filename ) return _filename;

    Mutex& vpm = const_cast< Mutex& >( _mutex );
    SCOPED_LOCK( vpm );

    CMedia* self = const_cast< CMedia* >(this);
    std::string file = self->sequence_filename( _dts );
    self->_filename = (char*) malloc( 1024 * sizeof(char) );
    strncpy( self->_filename, file.c_str(), 1023 );
    return _filename;
}



/**
 * Create and return the current filename of the image for the
 * current frame.
 *
 * @return filename of the file for current frame
 */
std::string CMedia::sequence_filename( const int64_t frame ) const
{
    if ( !is_sequence() ) return _fileroot;

    std::string tmp = parse_view( _fileroot, _is_left_eye );

    int64_t f = frame;
    if ( f > _frame_end ) f = _frame_end;
    else if ( f < _frame_start ) f = _frame_start;

    // For image sequences
    char buf[1024];
    sprintf( buf, tmp.c_str(), f );

    return std::string( buf );
}





/**
 * Returns whether the file has changed on disk and reloads it.
 * Note:  This function will fail if the file types of the images are
 *        different (for example, an .exr file replaced by a .dpx file ).
 *
 * @return true if it has changed, false if not.
 */
bool CMedia::has_changed()
{
    struct stat sbuf;

    SCOPED_LOCK( _mutex );

    if ( is_sequence() )
    {
        if ( !_sequence ) return false;

        int64_t f = handle_loops( _frame );
        std::string file = sequence_filename(f);


        int result = stat( file.c_str(), &sbuf );
        if ( (result == -1) || (f < _frame_start) ||
                ( f > _frame_end ) ) return false;

        int64_t idx = f - _frame_start;

        int64_t num = _frame_end - _frame_start + 1;

        if ( idx >= num )   idx = num - 1;
        else if ( idx < 0 ) idx = 0;

        if ( !_sequence[idx] ||
             _sequence[idx]->mtime() != sbuf.st_mtime ||
             _sequence[idx]->ctime() != sbuf.st_ctime )
        {
            assert( f == _sequence[idx]->frame() );
            // update frame...
            _sequence[idx].reset();

            _is_thumbnail = true;  // to avoid printing errors
            image_type_ptr canvas;
            if ( fetch( canvas, f ) )
            {
                _is_thumbnail = false;
                _mtime = sbuf.st_mtime;
                _ctime = sbuf.st_ctime;
                cache( canvas );
                refresh();
                return true;
            }
            _is_thumbnail = false;
            return false;
        }
    }
    else
    {
        if ( !_fileroot ) return false;

        int result = stat( _fileroot, &sbuf );
        if ( result == -1 ) return false;

        if ( ( _mtime != sbuf.st_mtime ) ||
                ( _ctime != sbuf.st_ctime ) )
        {
            int64_t f = handle_loops( _frame );
            _is_thumbnail = true; // to avoid printing errors
            image_type_ptr canvas;
            if ( fetch( canvas, f ) )
            {
                _mtime = sbuf.st_mtime;
                _ctime = sbuf.st_ctime;
                cache( canvas );
                refresh();
                _is_thumbnail = false;
                return true;
            }
            _is_thumbnail = false;
            return false;
        }
    }
    return false;
}


/**
 * Change the image size.
 * This function may also set the pixel ratio for some common video
 * formats and also their fps if fps == 0.
 *
 * @param w width of image
 * @param h height of image
 */
void CMedia::image_size( int w, int h )
{
    SCOPED_LOCK( _mutex );
    _pixel_ratio = 1.0f;

    // Derive pixel ratio from common format resolutions
    if ( w == 720 && h == 486 )
    {
        _pixel_ratio = 0.9f;    // 4:3 NTSC
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
    }
    else if ( w == 640 && h == 480 )
    {
        _pixel_ratio = 1.0f;    // 4:3 NTSC
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
    }
    else if ( w == 720 && h == 480 )
    {
        _pixel_ratio = 0.88888f;    // 4:3 NTSC
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
    }
    else if ( w == 512 && h == 486 )
    {
        _pixel_ratio = 1.265f;  // 4:3 Targa 486
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
    }
    else if ( w == 512 && h == 482 )
    {
        _pixel_ratio = 1.255f;  // 4:3 Targa NTSC
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
    }
    else if ( w == 512 && h == 576 )
    {
        _pixel_ratio = 1.5f;    // 4:3 Targa PAL
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 25;
    }
    else if ( w == 646 && h == 485 )
    {
        _pixel_ratio = 1.001f;  // 4:3 NTSC
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
    }
    else if ( w == 720 && h == 576 )
    {
        _pixel_ratio = 1.066f;  // 4:3 PAL
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 25;
    }
    else if ( w == 780 && h == 576 )
    {
        _pixel_ratio = 0.984f;  // 4:3 PAL
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 25;
    }
    else if ( w == 1280 && h == 1024 )
    {
        _pixel_ratio = 1.066f; // HDTV full
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
    }
    else if ( w == 1920 && h == 1080 )
    {
        _pixel_ratio = 1.0f; // HDTV full
    }
    else if ( w == 2048 && h == 1556 )
    {
        _pixel_ratio = 1.0f; // 2K full
    }
    else if ( w == 3840 && h == 2160 )
    {
        _pixel_ratio = 1.0f; // 4K HD
    }
    else if ( w == 4096 && h == 2304 )
    {
        _pixel_ratio = 1.0f; // 4K full
    }
    else if ( w == 7680 && h == 4320 )
    {
        _pixel_ratio = 1.0f; // 8K full
    }
    else if ( (float)w/(float)h == 1.56 )
    {
        _pixel_ratio = 0.9f;   // HTDV/SD compromise
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
    }

    if ( _fps == 0 )
    {
        if ( default_fps > 0 )
            _orig_fps =_fps = _play_fps = default_fps;
        else
            _orig_fps =_fps = _play_fps = 24.0f;
    }

    _w = w;
    _h = h;

}



image_type::Format CMedia::pixel_format() const
{
    if ( _hires )      return _hires->format();
    else return image_type::kLumma;
}

const char* const CMedia::pixel_format_name() const
{
    if ( _hires ) return _hires->pixel_format();
    else return "Lumma";
}

image_type::PixelType CMedia::depth() const
{
    return _depth;
}


void CMedia::gamma( const float x )
{
    CMedia::StereoInput  stereo_in  = stereo_input();
    CMedia::StereoOutput stereo_out = stereo_output();
    if ( _right_eye ) _right_eye->gamma( x );
    if ( stereo_in != kSeparateLayersInput || stereo_out != kStereoRight )
    {
        _gamma = x;
        refresh();
    }
}


void CMedia::chromaticities( const Imf::Chromaticities& c )
{
    _chromaticities = c;
    _has_chromaticities = true;
    image_damage( image_damage() | kDamageData | kDamageLut );
    refresh();
}

/**
 * Add alpha and alpha overlay layers to list of layers
 *
 */
void CMedia::alpha_layers()
{
    _layers.push_back( _("Alpha") );
    if ( _num_channels != 0 )
        _layers.push_back( _("Alpha Overlay") );
    ++_num_channels;
    image_damage( image_damage() | kDamageLayers | kDamageData );
}

/**
 * Add color and r,g,b to list of layers
 *
 */
void CMedia::rgb_layers()
{
    _layers.push_back( _("Color") );
    _layers.push_back( _("Red") );
    _layers.push_back( _("Green") );
    _layers.push_back( _("Blue") );
    _num_channels += 3;

    image_damage( image_damage() | kDamageLayers | kDamageData );
}

/**
 * Add lumma to list of layers
 *
 */
void CMedia::lumma_layers()
{
    _layers.push_back( _("Lumma") );
    image_damage( image_damage() | kDamageLayers | kDamageData );
}

/**
 * Add image's default RGBA layers
 *
 */
void CMedia::default_layers()
{
    _layers.clear();
    _num_channels = 0;
    rgb_layers();
    lumma_layers();
    alpha_layers();
}

void CMedia::stereo_output( StereoOutput x )
{
    if ( _stereo_output != x )
    {
        _stereo_output = x;
        if ( is_sequence() ) clear_cache();
        if ( playback() == kStopped )
        {
            image_type_ptr canvas;
            if ( fetch(canvas, _frame) )
            {
                cache( canvas );
            }
        }
        refresh();
    }
}

/**
 * Change the image's color channel
 *
 * @param c new image channel
 */
void CMedia::channel( const char* c )
{
    if ( _right_eye )  _right_eye->channel( c );

    std::string ch;
    std::string ext;

    if (c)
    {

        ch = c;


        if ( ch == _("Color") || ch == _("Red") || ch == _("Green") ||
                ch == _("Blue")  || ch == "" ||
                ch == _("Alpha") || ch == _("Alpha Overlay") || ch == _("Lumma") )
        {
            c = NULL;
            ch = "";
        }

    }

    bool to_fetch = false;


    if ( _channel != c )
    {
        std::string ch2, ext2;
        if ( _channel ) ch2 = _channel;

        // If stereo not set, don't fetch anything.
        if ( _channel == NULL && _right_eye &&
                _stereo_output != kNoStereo ) to_fetch = false;
        else
        {
            // No easy case.  Check the root names to see if one of them
            // contains the other
            size_t pos = ch.rfind( '.' );
            if ( pos != std::string::npos && pos != ch.size() )
            {
                ext = ch.substr( pos+1, ch.size() );
                ch = ch.substr( 0, pos );
                if ( ext.size() > 1 ) ch += '.' + ext;
            }

            pos = ch2.rfind( '.' );
            if ( pos != std::string::npos && pos != ch2.size() )
            {
                ext2 = ch2.substr( pos+1, ch2.size() );
                ch2 = ch2.substr( 0, pos );
                if ( ext2.size() > 1 ) ch2 += '.' + ext2;
            }

            //
            // Compare root name of layer to be more than 3 letters, to avoid
            // #0 N.N.Z (for example) from triggering a recache.
            //
            std::string chl = ch;
            pos = ch.find( '#' );
            if ( pos == 0 )
            {
                pos = ch.find( ' ' );
                chl = ch.substr( pos+1, ch.size() );
                pos = chl.find( '.' );
                if ( pos != std::string::npos )
                {
                    chl = chl.substr( 0, pos-1 );
                }
            }

            std::string chl2 = ch2;
            pos = ch2.find( '#' );
            if ( pos == 0 )
            {
                pos = ch2.find( ' ' );
                chl2 = ch2.substr( pos+1, ch2.size() );
                pos = chl2.find( '.' );
                if ( pos != std::string::npos )
                {
                    chl2 = chl2.substr( 0, pos-1 );
                }
            }

            if ( ( ch == "Y" || ch == "RY" || ch == "BY" ) ||
                    ( ch.find(ch2) == std::string::npos &&
                      ch2.find(ch) == std::string::npos ) ||
                    ( ext == "Z" && chl.size() > 1 ) ||
                    ( ext2 == "Z" && chl2.size() > 1 ) ||
                    _channel == NULL || c == NULL)
            {
                // To fetch Z channel, we must make it part of the channel name
                if ( ext == "Z" ) ch += '.' + ext;
                to_fetch = true;
            }
        }
    }

    free( _channel );
    _channel = NULL;

    // Store channel without r,g,b extension
    if ( !ch.empty() )
    {
        _channel = strdup( ch.c_str() );
    }


    if (to_fetch)
    {
        clear_cache();
        int64_t f = handle_loops( _frame );
        image_type_ptr canvas;
        if ( fetch( canvas, f ) )
        {
            _hires = canvas;
            cache( canvas );
            default_color_corrections();
        }
        refresh();
    }
}


/**
 * Retrieve the timestamp of the image on disk
 *
 */
void CMedia::timestamp()
{
    struct stat sbuf;
    int result = stat( filename(), &sbuf );
    if ( result == -1 ) return;
    _ctime = sbuf.st_ctime;
    _mtime = sbuf.st_mtime;
    _disk_space = sbuf.st_size;
}

/**
 * Store the timestamp for a cached sequence image
 *
 * @param idx index of cached image in sequence list
 */
void CMedia::timestamp(const boost::uint64_t idx,
                       mrv::image_type_ptr*& seq )
{
    if ( !seq ) return;

    mrv::image_type_ptr pic = seq[idx];

    struct stat sbuf;
    int result = stat( sequence_filename( pic->frame() ).c_str(), &sbuf );
    if ( result < 0 ) return;

    _ctime = sbuf.st_ctime;
    _mtime = sbuf.st_mtime;
    pic->ctime( sbuf.st_ctime );
    pic->mtime( sbuf.st_mtime );
    _disk_space += sbuf.st_size;
    image_damage( image_damage() | kDamageData );
}


/**
 * Retrieve the creation date of the image as ascii text.
 *
 *
 * @return date of creation of file
 */
const std::string CMedia::creation_date() const
{
    if ( is_sequence() && hires() )
    {
        time_t t = hires()->ctime();
        if ( t != 0 )
        {
            CMedia* img = const_cast< CMedia* >(this);
            img->_ctime = hires()->ctime();
            img->_mtime = hires()->mtime();
        }
    }

    std::string date( ::ctime( &_ctime ) );
    date = date.substr( 0, date.size() - 1 ); // eliminate \n
    return date;
}

/**
 * Returns image CTL script (or NULL if no script)
 *
 *
 * @return CTL script or NULL
 */
const char* CMedia::rendering_transform()  const
{
    return _rendering_transform;
}

/**
 * Change the rendering transform (CTL script) for the image
 *
 * @param cfile  CTL script name
 */
void CMedia::rendering_transform( const char* cfile )
{
    free( _rendering_transform );
    _rendering_transform = NULL;
    if ( cfile && strlen(cfile) > 0 )
    {
        _rendering_transform = strdup( cfile );
    }
    image_damage( image_damage() | kDamageData | kDamageLut );
    refresh();
}

/**
 * Returns image IDT CTL script (or NULL if no script)
 *
 * @return CTL script or NULL
 */
const char* CMedia::idt_transform()  const
{
    return _idt_transform;
}


/**
 * Change the IDT transform for the image
 *
 * @param cfile  CTL script name
 */
void CMedia::idt_transform( const char* cfile )
{
    free( _idt_transform );
    _idt_transform = NULL;
    if ( cfile && strlen(cfile) > 0 ) _idt_transform = strdup( cfile );
    image_damage( image_damage() | kDamageData | kDamageLut );
    refresh();
}


/**
 * Returns image LMT CTL script (or NULL if no script)
 *
 *
 * @return CTL script or NULL
 */
const char* CMedia::look_mod_transform( const size_t idx )  const
{
    if ( idx >= _look_mod_transform.size() ) return NULL;
    return _look_mod_transform[idx];
}


void CMedia::clear_look_mod_transform()
{
    for ( const auto& i : _look_mod_transform )
    {
        free( i );
    }
    _look_mod_transform.clear();

}

/**
 * Change the look mod transform for the image
 *
 * @param cfile  CTL script name
 */
void CMedia::append_look_mod_transform( const char* cfile )
{


    if ( cfile && strlen(cfile) > 0 )
        _look_mod_transform.push_back( strdup( cfile ) );
    else
    {

        size_t idx = _look_mod_transform.size();
        if ( idx == 0 ) return;
        idx -= 1;
        free( _look_mod_transform[idx] );
        _look_mod_transform.erase( _look_mod_transform.begin() + idx );
    }
    image_damage( image_damage() | kDamageData | kDamageLut );
    refresh();
}

// Count how many LMTs are GradeRef's components (Nodes)
size_t  CMedia::number_of_grade_refs() const
{
    size_t c = 0;

    LMT::const_iterator i = _look_mod_transform.begin();
    LMT::const_iterator e = _look_mod_transform.end();
    for ( ; i != e; ++i )
    {
        size_t num = strlen( *i );
        if ( num < 4 ) continue;

        for ( size_t j = 0; j < num - 3; ++j )
        {
            if ( strncmp( "Node", (*i) + j, 4 ) == 0 )
            {
                ++c;
                break;
            }
        }
    }

    return c;
}

/**
 * Change the look mod transform for the image
 *
 * @param cfile  CTL script name
 */
void CMedia::insert_look_mod_transform( const size_t idx, const char* cfile )
{
    if ( idx >= _look_mod_transform.size() ) return;

    if ( cfile && strlen(cfile) > 0 )
    {
        _look_mod_transform.insert( _look_mod_transform.begin() + idx,
                                    strdup( cfile ) );
    }
    else
    {
        _look_mod_transform.erase( _look_mod_transform.begin() + idx );
    }
    image_damage( image_damage() | kDamageData | kDamageLut );
    refresh();
}

/**
 * Change the look mod transform for the image
 *
 * @param cfile  CTL script name
 */
void CMedia::look_mod_transform( const size_t idx, const char* cfile )
{
    if ( idx >= _look_mod_transform.size() ) return;

    free( _look_mod_transform[idx] );
    if ( cfile && strlen(cfile) > 0 )
    {
        _look_mod_transform[idx] = strdup( cfile );
    }
    else
    {
        _look_mod_transform.erase( _look_mod_transform.begin() + idx );
    }
    image_damage( image_damage() | kDamageData | kDamageLut );
    refresh();
}

/**
 * Returns image color profile information (or NULL if no profile)
 *
 *
 * @return color profile or NULL
 */
const char* CMedia::icc_profile()  const
{
    return _profile;
}


/**
 * Change ICC profile of image
 *
 * @param cfile   color profile of image
 */
void CMedia::icc_profile( const char* cfile )
{
    free( _profile );
    _profile = NULL;
    if ( cfile && strlen(cfile) > 0 )
    {
        mrv::colorProfile::add( cfile );
        _profile = strdup( cfile );
    }
    image_damage( image_damage() | kDamageData | kDamageLut );
    refresh();
}



// If sequence or has picture return true
bool CMedia::valid_video() const
{
    if ( is_sequence() || has_picture() )
    {
        return true;
    }

    return false;
}

bool CMedia::valid_audio() const
{
    size_t num_streams = number_of_audio_streams();

    bool valid = false;
    for ( size_t i = 0; i < num_streams; ++i )
    {
        if ( _audio_info[i].has_codec ) {
            valid = true;
            break;
        }
    }

    return valid;
}

bool CMedia::valid_subtitle() const
{
    size_t num_streams = number_of_subtitle_streams();

    bool valid = false;
    for ( size_t i = 0; i < num_streams; ++i )
    {
        if ( _subtitle_info[i].has_codec ) {
            valid = true;
            break;
        }
    }

    return valid;
}

/// VCR play (and record if needed) sequence
void CMedia::play(const CMedia::Playback dir,
                  mrv::ViewerUI* const uiMain,
                  bool fg )
{

    assert0( dir != kStopped );
    // if ( _playback == kStopped && !_threads.empty() )
    //     return;

    if ( _right_eye ) _right_eye->play( dir, uiMain, fg );

    TRACE("");
    stop();

    TRACE("");
    _playback = dir;

    assert( uiMain != NULL );
    assert( _threads.size() == 0 );

    if ( _frame < first_frame() ) _frame = first_frame();
    if ( _frame > last_frame() )  _frame = last_frame();

    _audio_frame = _frame.load();
    // _expected = std::numeric_limits< int64_t >::min();

    _dts = _frame.load();

    DBG( name() << " Play from frame " << _dts << " expected " << _expected );

    _audio_clock = double( av_gettime_relative() ) / 1000000.0;
    _video_clock = double( av_gettime_relative() ) / 1000000.0;

    _video_pts = _audio_pts = _frame  / _orig_fps;
    update_video_pts(this, _video_pts, 0, 0);
    set_clock_at(&audclk, _audio_pts, 0, _audio_clock );
    sync_clock_to_slave( &audclk, &extclk );

    _audio_buf_used = 0;


    TRACE("");
    // clear all packets
    clear_packets();

    TRACE("");
    // This seek is needed to sync audio playback and flush buffers
    if ( dir == kForwards ) _seek_req = true;

    TRACE("");
    if ( ! seek_to_position( _frame ) )
        IMG_ERROR( _("Could not seek to frame ") << _frame );

    TRACE("");

    // Start threads
    PlaybackData* data = new PlaybackData( fg, uiMain, this );  //for decode
    assert( data != NULL );
    assert( data->uiMain != NULL );
    assert( data->image != NULL );

    PlaybackData* video_data, *audio_data, *subtitle_data;

    try {
        // If there's at least one valid video stream, create video thread
        bool valid_v = valid_video();
        // If there's at least one valid audio stream, create audio thread
        bool valid_a = valid_audio();
        // If there's at least one valid subtitle stream, create subtitle thread
        bool valid_s = valid_subtitle();

        // When a single image with no audio is present jump to our single frame
        if ( !valid_a && !has_video() && !is_sequence() )
        {
            frame( _frame );
        }

        // std::cerr << name()
        //           << " is_stereo? " << _is_stereo << " left_eye? "
        //           << _is_left_eye
        //           << " output " << _stereo_output << std::endl;
        // if ( _is_stereo && _is_left_eye == false &&
        //      _stereo_output != kNoStereo )
        if ( _is_stereo && _right_eye )
        {
            delete _right_eye->_stereo_barrier;
            if ( number_of_video_streams() % 2 == 0 )
                _right_eye->_stereo_barrier = new Barrier( 2 );
            else
                _right_eye->_stereo_barrier = new Barrier( 1 );

        }

        if ( !fg && !_fg_bg_barrier )
        {
            _fg_bg_barrier = new Barrier( valid_v + valid_a );
        }
        else if ( !fg && _fg_bg_barrier )
        {
            _fg_bg_barrier->notify_all();
            _fg_bg_barrier->threshold( valid_v + valid_a );
        }
        else
        {
            _fg_bg_barrier = NULL;
        }

        unsigned num = 1 + valid_a + valid_v + valid_s;
        delete _loop_barrier;
        _loop_barrier = new Barrier( num );

        if ( valid_v || valid_a )
        {
            video_data = new PlaybackData( *data );
            boost::thread* t = new boost::thread(
                boost::bind( mrv::video_thread,
                             video_data ) );
            _threads.push_back( t );
        }

        if ( valid_a )
        {
            // Audio playback thread
            audio_data = new PlaybackData( *data );
            boost::thread* t = new boost::thread(
                boost::bind( mrv::audio_thread,
                             audio_data ) );
            _threads.push_back( t );
        }

        if ( valid_s )
        {
            // Subtitle playback thread
            subtitle_data = new PlaybackData( *data );
            _threads.push_back( new boost::thread(
                                    boost::bind( mrv::subtitle_thread,
                                                 subtitle_data ) ) );
        }


        // If something was valid, create decode thread
        if ( valid_a || valid_v || valid_s )
        {
            boost::thread* t = new boost::thread(
                boost::bind( mrv::decode_thread,
                             data ) );
            _threads.push_back( t );
        }


        assert0( _threads.size() <= ( 1 + 2 * ( valid_a || valid_v ) + valid_s ) );
    }
    catch( boost::exception& e )
    {
        LOG_ERROR( boost::diagnostic_information(e) );
    }


}


/// VCR stop sequence
void CMedia::stop(const bool bg)
{


    if ( _playback == kStopped && _threads.empty() ) return;

    if ( _right_eye ) _right_eye->stop();

    TRACE("");


    _playback = kStopped;

    //
    //
    //

    //
    // Notify loop barrier, to exit any wait on a loop
    //
    TRACE("");
    DBG( name() << " Notify all loop barriers" );
    if ( _loop_barrier )  _loop_barrier->notify_all();
    DBG( name() << " Notify stereo barrier" );
    TRACE("");
    if ( _stereo_barrier ) _stereo_barrier->notify_all();
    if ( _fg_bg_barrier ) _fg_bg_barrier->notify_all();

    TRACE("");
    // Notify packets, to make sure that audio thread exits any wait lock
    // This needs to be done even if no audio is playing, as user might
    // have turned off audio, but audio thread is still active.
    DBG( name() << " Notify all A/V/S" );
    TRACE("");
    _audio_packets.cond().notify_all();
    TRACE("");
    _video_packets.cond().notify_all();
    TRACE("");
    _subtitle_packets.cond().notify_all();
    TRACE("");

    // Wait for all threads to exit
    DBG( name() << " Wait for threads" );
    wait_for_threads();
    DBG( name() << " Waited for threads" );

    // Clear barrier
    DBG( name() << " Clear barrier" );
    delete _loop_barrier;
    TRACE("");
    _loop_barrier = NULL;
    TRACE("");
    delete _stereo_barrier;
    _stereo_barrier = NULL;

#if 0
    if ( bg ) {
    TRACE("");
        delete _fg_bg_barrier;
        _fg_bg_barrier = NULL;
    }
#endif

    TRACE("");
    close_audio();

    DBG( name() << " Clear packets" );
    TRACE("");
    // Clear any audio/video/subtitle packets
    clear_packets();

    TRACE("");
    // Queue thumbnail for update
    image_damage( image_damage() | kDamageThumbnail );

}


/**
 *
 * @return the image's name sans directory
 */
std::string CMedia::name() const
{
    fs::path file = fs::path( fileroot() );
    return file.leaf().string();
}


/**
 *
 * @return  the image's directory
 */
std::string CMedia::directory() const
{
    std::string name = fileroot();
    if ( name.substr(0, 6) == "Slate " )
        name = name.substr(6, name.size() );
    fs::path file = fs::path( name );
    file = fs::absolute( file.branch_path() );
    std::string path;
    if ( fs::exists( file ) )
        path = fs::canonical( file ).string();
    else
        path = file.string();
    return path;
}



/**
 * Set a new frame for the image sequence
 *  * @param f new frame
 */
bool CMedia::frame( int64_t f )
{
    assert( _fileroot != NULL );

    if ( ( playback() == kStopped ) && _right_eye && _stereo_output )
        _right_eye->frame(f);

//  in ffmpeg, sizes are in bytes...
#define MAX_VIDEOQ_SIZE (5 * 2048 * 1024)
#define MAX_AUDIOQ_SIZE (5 * 60 * 1024)
#define MAX_SUBTITLEQ_SIZE (5 * 30 * 1024)
    if (
        _video_packets.bytes() > MAX_VIDEOQ_SIZE ||
        _audio_packets.bytes() > MAX_AUDIOQ_SIZE ||
        _subtitle_packets.bytes() > MAX_SUBTITLEQ_SIZE  )
    {
        return false;
    }


    if ( f < _frameStart )     _dts = _frameStart;
    else if ( f > _frameEnd )  _dts = _frameEnd;
    else                       _dts = f;

    AVPacket pkt;
    av_init_packet( &pkt );
    pkt.dts = pkt.pts = _dts;
    pkt.size = 0;
    pkt.data = NULL;

    if ( ! is_cache_filled( _dts ) )
    {
        image_type_ptr canvas;
        std::string file = sequence_filename( _dts );

        if ( fs::exists( file ) )
        {
            if ( fetch( canvas, _dts ) )
            {
                _hires = canvas;
                cache( canvas );
                default_color_corrections();
            }
        }
    }

    _video_packets.push_back( pkt );

    if ( has_audio() )
    {
        _adts = _dts + _audio_offset;
        fetch_audio( _adts );
    }

    _expected = _dts + 1;
    _expected_audio = _expected + _audio_offset;


    return true;
}


/**
 * Seek to a new frame
 *
 * @param f   new frame
 */
void CMedia::seek( const int64_t f )
{
// #define DEBUG_SEEK
#ifdef DEBUG_SEEK
    std::cerr << "------- SEEK " << f << " " << name() << " stopped? "
              << stopped() << std::endl;
#endif


    _seek_frame = f;
    _seek_req   = true;


    if ( _right_eye )
    {
#ifdef DEBUG_SEEK
        std::cerr << "------- SEEK RIGHT EYE " << f << " " << name() << " stopped? "
                  << stopped() << std::endl;
#endif
        _right_eye->_seek_frame = f;
        _right_eye->_seek_req = true;
    }

    if ( stopped() || saving() )
    {
#ifdef DEBUG_SEEK
        std::cerr << "------ SEEK STOPPED OR SAVING DO ACTUAL SEEK" << std::endl;
#endif
        do_seek();
    }

#ifdef DEBUG_SEEK
    std::cerr << "------- SEEK DONE " << f << " _dts: " << _dts << " _frame: "
              << _frame << " _expected: " << _expected << std::endl;
#endif
}


void CMedia::update_cache_pic( mrv::image_type_ptr*& seq,
                               const mrv::image_type_ptr& pic )
{
    assert0( seq != NULL );
    assert0( pic != NULL );
    assert0( pic.use_count() >= 1 );


    
    int64_t f = pic->frame();
    _depth = pic->pixel_type();

    int64_t idx = f - _frame_start;
    int64_t num = _frame_end - _frame_start + 1;
    if ( idx < 0 ) idx = 0;
    else if ( idx > num ) idx = num - 1;

    if ( !seq || seq[idx] ) return;

    
    
    mrv::image_type_ptr np;

    unsigned w = pic->width();
    unsigned h = pic->height();

    if ( _8bit_cache && pic->pixel_type() != image_type::kByte )
    {
        np.reset( new image_type( pic->frame(), w, h, pic->channels(),
                                  pic->format(), image_type::kByte,
                                  pic->repeat(), pic->pts() ) );

        for ( unsigned y = 0; y < h; ++y )
        {
            for ( unsigned x = 0; x < w; ++x )
            {
                ImagePixel p = pic->pixel( x, y );

                if ( p.r > 1.0f ) p.r = 1.0f;
                else if ( p.r < 0.0f ) p.r = 0.f;

                if ( p.g > 1.0f ) p.g = 1.0f;
                else if ( p.g < 0.0f ) p.g = 0.f;

                if ( p.b > 1.0f ) p.b = 1.0f;
                else if ( p.b < 0.0f ) p.b = 0.f;

                if ( p.a > 1.0f ) p.a = 1.0f;
                else if ( p.a < 0.0f ) p.a = 0.f;

                if ( p.r > 0.f )
                    p.r = powf( p.r, 1.0f / gamma() );
                if ( p.g > 0.f )
                    p.g = powf( p.g, 1.0f / gamma() );
                if ( p.b > 0.f )
                    p.b = powf( p.b, 1.0f / gamma() );

                np->pixel( x, y, p );
            }
        }

        if ( _cache_scale > 0 )
        {
            w /= (1 << _cache_scale);
            h /= (1 << _cache_scale);
            np.reset( np->resize( w, h ) );
        }

        seq[idx] = np;
    }
    else
    {
        if ( _cache_scale > 0 )
        {
            w /= (1 << _cache_scale);
            h /= (1 << _cache_scale);
            np.reset( pic->resize( w, h ) );
            seq[idx] = np;
        }
        else
        {
            //DBG( "cache seq " << idx << " pic " << pic );
            seq[idx] = pic;
            assert0( pic.use_count() >= 2 );
        }
    }

    _w = w;
    _h = h;

    timestamp(idx, seq);
}

/**
 * Cache picture for sequence.
 *
 * @param frame       frame to retrieve cache from
 */
mrv::image_type_ptr CMedia::cache( int64_t frame ) const
{

    mrv::image_type_ptr r;

    if ( !is_sequence() || !_cache_active ||
         dynamic_cast< const aviImage* >( this ) != NULL )
        return r;

    int64_t idx = frame - _frame_start;
    int64_t num = _frame_end - _frame_start + 1;
    {
        //SCOPED_LOCK( _mutex );
        if ( idx < 0 ) idx = 0;
        else if ( idx >= num ) idx = num - 1;
    }

    return _sequence[idx];
}
/**
 * Cache picture for sequence.
 *
 * @param pic       picture to cache
 */
void CMedia::cache( mrv::image_type_ptr& pic )
{
    if ( dynamic_cast< const aviImage* >( this ) != NULL )
        return;

    assert0( pic != NULL );
    assert0( pic.use_count() != 0 );


    if ( !is_sequence() || !_cache_active || !pic )
        return;

    if ( _stereo[0] && _stereo[0]->frame() == pic->frame() )
    {
        update_cache_pic( _sequence, _stereo[0] );
        _stereo[0].reset();
    }
    else
    {
        update_cache_pic( _sequence, pic );
        pic.reset();
    }

    if ( _stereo[1] && _stereo[1]->frame() == pic->frame() )
    {
        update_cache_pic( _right, _stereo[1] );
        _stereo[1].reset();
    }

}


/**
 * Check if cache is already filled for a frame
 *
 * @param frame   frame to check
 *
 * @return true or false if cache is already filled.
 */
CMedia::Cache CMedia::is_cache_filled(int64_t frame)
{
    if ( !_sequence ) return kNoCache;

    SCOPED_LOCK( _mutex );

    if ( frame > _frame_end ) return kNoCache;
    else if ( frame < _frame_start ) return kNoCache;

    boost::int64_t i = frame - _frame_start;

    CMedia::Cache cache = kNoCache;
    mrv::image_type_ptr pic = _sequence[i];
    if ( !pic ) return cache;

    if ( !pic->valid() ) return kInvalidFrame;

    cache = kLeftCache;

    if ( _stereo_output != kNoStereo )
    {
        if ( _stereo_input  == kSeparateLayersInput &&
                _right && _right[i] ) cache = kStereoCache;
        else if ( _stereo_input != kSeparateLayersInput && cache == kLeftCache )
            cache = kStereoCache;
    }

    return cache;
}


bool CMedia::is_cache_full()
{
    if ( dynamic_cast< aviImage* >( this ) != NULL )
        return true;

    for ( int64_t i = _frame_start; i <= _frame_end; ++i )
    {
        if ( is_cache_filled(i) == kNoCache ) return false;
    }
    return true;
}

int64_t CMedia::first_cache_empty_frame()
{
    if ( dynamic_cast< aviImage* >( this ) != NULL )
        return first_frame();

    for ( int64_t i = _frame_start; i <= _frame_end; ++i )
    {
        if ( is_cache_filled(i) == kNoCache ) {
            return i;
        }
    }

    return last_frame();
}
/**
 * Flushes all caches
 *
 */
void CMedia::flush_all()
{
    if ( _right_eye ) _right_eye->flush_all();

    if ( has_video() )
        flush_video();
    if ( has_audio() )
        flush_audio();
}

/// Returns the size in memory of image or sequence, based on original
/// bit depth.
size_t CMedia::memory() const
{
    size_t r = 0;
    if ( _sequence )
    {
        boost::uint64_t frames = _frame_end - _frame_start + 1;
        for ( boost::uint64_t i = 0; i < frames; ++i )
        {
            mrv::image_type_ptr s = _sequence[i];
            if ( !s ) continue;

            r += s->data_size();
        }
    }
    else
    {
        if ( hires() )
        {
            r += _hires->data_size();
        }
    }

    return r;
}



///////////////////////////////
// FFMPEG routines
///////////////////////////////


/**
 * Given a codec_context, returns the type of stream.
 *
 * @param codec_context
 *
 * @return stream type as a character string.
 */
const char* CMedia::stream_type( const AVCodecParameters* codecpar )
{
    const char* stream;
    switch( codecpar->codec_type )
    {
    case AVMEDIA_TYPE_VIDEO:
        stream = _("video");
        break;
    case AVMEDIA_TYPE_AUDIO:
        stream = _("audio");
        break;
    case AVMEDIA_TYPE_DATA:
        stream = _("data");
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        stream = _("subtitle");
        break;
    default:
        stream = _("unknown");
        break;
    }
    return stream;
}

/**
 * Given a codec_context, returns the type of stream.
 *
 * @param codec_context
 *
 * @return stream type as a character string.
 */
const char* CMedia::stream_type( const AVCodecContext* codec_context )
{
    const char* stream;
    switch( codec_context->codec_type )
    {
    case AVMEDIA_TYPE_VIDEO:
        stream = _("video");
        break;
    case AVMEDIA_TYPE_AUDIO:
        stream = _("audio");
        break;
    case AVMEDIA_TYPE_DATA:
        stream = _("data");
        break;
    case AVMEDIA_TYPE_SUBTITLE:
        stream = _("subtitle");
        break;
    default:
        stream = _("unknown");
        break;
    }
    return stream;
}



/**
 * Given a codec tag as unsigned integer, return the FOURCC if valid or
 * an hex representation of the tag.
 *
 * @param codec_tag
 *
 * @return
 */
std::string CMedia::codec_tag2fourcc( unsigned int codec_tag )
{
    char buf[80];
    memset( buf, 0, 5 );
    memcpy( buf, &codec_tag, 4 );

    bool ascii = true;
    for ( int i = 0; i < 4; ++i )
    {
        if ( !isprint( buf[i] ) )
        {
            ascii = false;
            break;
        }
    }


    std::string fourcc;
    if ( !ascii )
    {
        sprintf( buf, N_("0x%08x"), codec_tag );
    }

    fourcc = buf;
    return fourcc;
}



/**
 * Given a codec parameter, return the name of the codec if possible.
 *
 * @param enc   codec context
 *
 * @return      name of codec.
 */
std::string CMedia::codec_name( const AVCodecParameters* enc )
{
    AVCodec* p = avcodec_find_decoder(enc->codec_id);
    const char* codec_name;
    char buf[20];

    if (p) {
        codec_name = p->name;
    } else if (enc->codec_id == AV_CODEC_ID_MPEG2TS) {
        /* fake mpeg2 transport stream codec (currently not
           registered) */
        codec_name = N_("mpeg2ts");
    } else {
        /* output avi tags */
        if(   isprint(enc->codec_tag&0xFF) && isprint((enc->codec_tag>>8)&0xFF)
                && isprint((enc->codec_tag>>16)&0xFF) &&
                isprint((enc->codec_tag>>24)&0xFF)) {
            snprintf(buf, sizeof(buf), N_("%c%c%c%c / 0x%04X"),
                     enc->codec_tag & 0xff,
                     (enc->codec_tag >> 8) & 0xff,
                     (enc->codec_tag >> 16) & 0xff,
                     (enc->codec_tag >> 24) & 0xff,
                     enc->codec_tag);
        } else {
            snprintf(buf, sizeof(buf), N_("0x%04x"), enc->codec_tag);
        }
        codec_name = buf;
    }

    if ( codec_name )
        return std::string( codec_name );
    else
        return "";
}

/**
 * Given a codec context, return the name of the codec if possible.
 *
 * @param enc   codec context
 *
 * @return      name of codec.
 */
std::string CMedia::codec_name( const AVCodecContext* enc )
{
    AVCodec* p = avcodec_find_decoder(enc->codec_id);
    const char* codec_name;
    char buf[20];

    if (p) {
        codec_name = p->name;
    } else if (enc->codec_id == AV_CODEC_ID_MPEG2TS) {
        /* fake mpeg2 transport stream codec (currently not
           registered) */
        codec_name = N_("mpeg2ts");
    } else {
        /* output avi tags */
        if(   isprint(enc->codec_tag&0xFF) && isprint((enc->codec_tag>>8)&0xFF)
                && isprint((enc->codec_tag>>16)&0xFF) &&
                isprint((enc->codec_tag>>24)&0xFF)) {
            snprintf(buf, sizeof(buf), N_("%c%c%c%c / 0x%04X"),
                     enc->codec_tag & 0xff,
                     (enc->codec_tag >> 8) & 0xff,
                     (enc->codec_tag >> 16) & 0xff,
                     (enc->codec_tag >> 24) & 0xff,
                     enc->codec_tag);
        } else {
            snprintf(buf, sizeof(buf), N_("0x%04x"), enc->codec_tag);
        }
        codec_name = buf;
    }

    if ( codec_name )
        return std::string( codec_name );
    else
        return "";
}


/**
 * Given an FFMPEG stream, try to calculate the FPS speed for it.
 *
 * @param stream
 *
 * @return FPS (frames per second)
 */
double CMedia::calculate_fps( const AVStream* stream )
{
    double fps;

    assert( stream != NULL );
    const AVRational& rate = av_stream_get_r_frame_rate( stream );

    if ( stream->avg_frame_rate.den && stream->avg_frame_rate.num )
    {
        fps = av_q2d( stream->avg_frame_rate );
    }
    else if ( rate.den && rate.num )
    {
        fps = av_q2d( rate );
    }
    else
    {
        fps = 1.0 / av_q2d( stream->time_base );
    }

    // Some codecs/movies seem to return fps as a time_base > 10000, like:
    //           24895 / 2   for 24.895 fps.  Not sure why /2, thou.
    if ( fps > 1000 &&
            stream->time_base.den > 10000 && stream->time_base.num < 3 )
    {
        fps = stream->time_base.den / 1000.0;
    }

    if ( fps >= 1000 )
    {
        fps = 30.0f;  // workaround for some buggy streams
    }

    return fps;
}



/**
 * Populate the stream information from a codec context
 *
 * @param s              stream info
 * @param msg            any error message
 * @param codec_context  codec context
 * @param stream_index   ffmpeg stream index
 */
void CMedia::populate_stream_info( StreamInfo& s,
                                   std::ostringstream& msg,
                                   const AVFormatContext* context,
                                   const AVCodecParameters* ctx,
                                   const int stream_index )
{

    bool has_codec = true;

    // Mark streams that we don't have a decoder for
    AVCodec* codec = avcodec_find_decoder( ctx->codec_id );
    if ( ! codec )
    {
        has_codec = false;
        const char* type = stream_type( ctx );
        msg << _("\n\nNot a known codec ") << codec_name(ctx)
            << _(" for stream #") << stream_index << _(", type ") << type;
    }

    s.context      = context;
    s.stream_index = stream_index;
    s.has_codec    = has_codec;
    s.codec_name   = codec_name( ctx );
    s.fourcc       = codec_tag2fourcc( ctx->codec_tag );

    AVStream* st = context->streams[stream_index];
    double time  = av_q2d( st->time_base );


    AVDictionaryEntry* lang = av_dict_get(st->metadata, "language", NULL, 0);
    if ( lang && lang->value )
        s.language = lang->value;
    else
        s.language = _("und");

    if ( st->start_time == AV_NOPTS_VALUE )
    {
        s.start = 1;
    }
    else
    {
        s.start = ((double) st->start_time * time);
    }

    if ( st->duration != AV_NOPTS_VALUE )
    {
        s.duration = ((double) st->duration * time);
    }
    else
    {
        s.duration = ((double) _context->duration / ( double )AV_TIME_BASE );
    }

    if (st->disposition & AV_DISPOSITION_DEFAULT)
        s.disposition = _("default");
    if (st->disposition & AV_DISPOSITION_DUB)
        s.disposition = _("dub");
    if (st->disposition & AV_DISPOSITION_ORIGINAL)
        s.disposition = _("original");
    if (st->disposition & AV_DISPOSITION_COMMENT)
        s.disposition = _("comment");
    if (st->disposition & AV_DISPOSITION_LYRICS)
        s.disposition = _("lyrics");
    if (st->disposition & AV_DISPOSITION_KARAOKE)
        s.disposition = _("karaoke");
    if (st->disposition & AV_DISPOSITION_FORCED)
        s.disposition = _("forced");
    if (st->disposition & AV_DISPOSITION_HEARING_IMPAIRED)
        s.disposition = _("hearing impaired");
    if (st->disposition & AV_DISPOSITION_VISUAL_IMPAIRED)
        s.disposition = _("visual impaired");
    if (st->disposition & AV_DISPOSITION_CLEAN_EFFECTS)
        s.disposition = _("clean effects");
}


/**
 * Populate the stream information from a codec context
 *
 * @param s              stream info
 * @param msg            any error message
 * @param codec_context  codec context
 * @param stream_index   ffmpeg stream index
 */
void CMedia::populate_stream_info( StreamInfo& s,
                                   std::ostringstream& msg,
                                   const AVFormatContext* context,
                                   const AVCodecContext* ctx,
                                   const int stream_index )
{

    bool has_codec = true;

    // Mark streams that we don't have a decoder for
    AVCodec* codec = avcodec_find_decoder( ctx->codec_id );
    if ( ! codec )
    {
        has_codec = false;
        const char* type = stream_type( ctx );
        msg << _("\n\nNot a known codec ") << codec_name(ctx)
            << _(" for stream #") << stream_index << _(", type ") << type;
    }

    s.context      = context;
    s.stream_index = stream_index;
    s.has_codec    = has_codec;
    s.codec_name   = codec_name( ctx );
    s.fourcc       = codec_tag2fourcc( ctx->codec_tag );

    AVStream* st = context->streams[stream_index];
    double time  = av_q2d( st->time_base );


    AVDictionaryEntry* lang = av_dict_get(st->metadata, "language", NULL, 0);
    if ( lang && lang->value )
        s.language = lang->value;
    else
        s.language = _("und");

    if ( st->start_time == AV_NOPTS_VALUE )
    {
        s.start = 1;
    }
    else
    {
        s.start = ((double) st->start_time * time);
    }

    if ( st->duration != AV_NOPTS_VALUE )
    {
        s.duration = ((double) st->duration * time);
    }
    else
    {
        s.duration = ((double) _context->duration / ( double )AV_TIME_BASE );
    }

    if (st->disposition & AV_DISPOSITION_DEFAULT)
        s.disposition = _("default");
    if (st->disposition & AV_DISPOSITION_DUB)
        s.disposition = _("dub");
    if (st->disposition & AV_DISPOSITION_ORIGINAL)
        s.disposition = _("original");
    if (st->disposition & AV_DISPOSITION_COMMENT)
        s.disposition = _("comment");
    if (st->disposition & AV_DISPOSITION_LYRICS)
        s.disposition = _("lyrics");
    if (st->disposition & AV_DISPOSITION_KARAOKE)
        s.disposition = _("karaoke");
    if (st->disposition & AV_DISPOSITION_FORCED)
        s.disposition = _("forced");
    if (st->disposition & AV_DISPOSITION_HEARING_IMPAIRED)
        s.disposition = _("hearing impaired");
    if (st->disposition & AV_DISPOSITION_VISUAL_IMPAIRED)
        s.disposition = _("visual impaired");
    if (st->disposition & AV_DISPOSITION_CLEAN_EFFECTS)
        s.disposition = _("clean effects");
}

// Convert an FFMPEG pts into a frame number
int64_t CMedia::frame2pts( const AVStream* stream,
                           const int64_t frame ) const
{
    long double p = (long double)(frame - 1) / (long double) fps();

    if ( stream )
    {
        // reverse num/den correct here
        p /= stream->time_base.num;
        p *= stream->time_base.den;
    }

    return int64_t(p);
}


// Convert an FFMPEG pts into a frame number
int64_t CMedia::pts2frame( const AVStream* stream,
                           const int64_t dts ) const
{


    //assert( dts != AV_NOPTS_VALUE );
    if (!stream || dts == AV_NOPTS_VALUE) return 0;

    long double p = (long double) dts;
    p *= stream->time_base.num;
    p /= stream->time_base.den;
    p *= _orig_fps;
    int64_t pts = int64_t( p + 0.5 ) + 1;
    return pts;
}




// Return the number of frames cached for jog/shuttle
int CMedia::max_video_frames()
{
    if ( _video_cache_size > 0 )
        return _video_cache_size;
    else if ( _video_cache_size == 0 )
        return int( fps()*2 );
    else
        return std::numeric_limits<int>::max() / 3;
}


// Return the number of frames cached for jog/shuttle
int CMedia::max_image_frames()
{
#if 0
    if ( _image_cache_size > 0 )
        return _image_cache_size;
    else if ( _image_cache_size == 0 )
        return int( fps()*2 );
    else
        return std::numeric_limits<int>::max() / 3;
#else
    if ( !_hires ) return std::numeric_limits<int>::max() / 3;
    return (Preferences::max_memory / _hires->data_size());
#endif
}

void CMedia::loop_at_start( const int64_t frame )
{
    if ( has_picture() )
    {
        // With loop at start we cannot discard previous frames as they are
        // part of one or multiple prerolls

        _video_packets.loop_at_start( frame );
    }

    if ( number_of_audio_streams() > 0 )
    {
        _audio_packets.loop_at_start( frame );
    }

    if ( number_of_subtitle_streams() > 0 )
    {
        _subtitle_packets.loop_at_start( frame );
    }
}


void CMedia::loop_at_end( const int64_t frame )
{


    if ( has_picture() )
    {
        // With loop at end, we can discard all video packets that go
        // beyond the last frame
        // SCOPED_LOCK( _mutex ); // unneeded

        mrv::PacketQueue::Mutex& m = _video_packets.mutex();
        SCOPED_LOCK( m );

        mrv::PacketQueue::reverse_iterator i = _video_packets.rbegin();
        mrv::PacketQueue::reverse_iterator e = _video_packets.rend();
        AVStream* stream = get_video_stream();
        for ( ; i != e; ++i )
        {
            if ( get_frame( stream, *i ) >= frame )
            {
                mrv::PacketQueue::iterator it = (i+1).base();
                _video_packets.erase( it );
            }
        }

        _video_packets.loop_at_end( frame );
    }

    if ( number_of_audio_streams() > 0 )
    {
        // With loop at end, we can discard all audio packets that go
        // beyond the last frame

        // SCOPED_LOCK( _audio_mutex ); // unneeded

        mrv::PacketQueue::Mutex& m = _audio_packets.mutex();
        SCOPED_LOCK( m );

        mrv::PacketQueue::reverse_iterator i = _audio_packets.rbegin();
        mrv::PacketQueue::reverse_iterator e = _audio_packets.rend();
        AVStream* stream = get_audio_stream();
        for ( ; i != e; ++i )
        {
            int64_t pktframe = get_frame( stream, *i );
            if ( pktframe >= frame )
            {
                mrv::PacketQueue::iterator it = (i+1).base();
                _audio_packets.erase( it );
            }
        }

        _audio_packets.loop_at_end( frame );
    }

    if ( number_of_subtitle_streams() > 0 )
    {
        _subtitle_packets.loop_at_end( frame );
    }
}




void CMedia::limit_video_store( const int64_t f )
{
    SCOPED_LOCK( _mutex );


    if ( !_sequence ) return;

    
#undef timercmp
# define timercmp(a, b, CMP)                                                  \
  (((a).tv_sec == (b).tv_sec) ?					\
   ((a).tv_usec CMP (b).tv_usec) :                                          \
   ((a).tv_sec CMP (b).tv_sec))

    struct customMore {
        inline bool operator()( const timeval& a,
                                const timeval& b ) const
        {
            return timercmp( a, b, > );
        }
    };


    boost::uint64_t num  = _frame_end - _frame_start + 1;

    typedef std::multimap< timeval, uint64_t, customMore > TimedMoreSeqMap;
    
    TimedMoreSeqMap tmp;
    for ( uint64_t i = 0; i < num; ++i )
    {
        if ( !_sequence[i] || _sequence[i]->frame() == f ) continue;
        tmp.insert( std::make_pair( _sequence[i]->ptime(), i ) );
    }


    
    uint64_t count = 0;
    uint64_t max_frames = max_image_frames();

    {
	TimedMoreSeqMap::iterator it = tmp.begin();
	for ( ; it != tmp.end(); )
	{
	    ++count;
	    if ( count > max_frames )
	    {
		uint64_t idx = it->second;
		_sequence[ idx ].reset();
		if ( _right && _right[idx] ) _right[ idx ].reset();
	    
		it = tmp.erase(it);
	    }
	    else
	    {
		++it;
	    }
	}
    }
    


}

void CMedia::preroll( const int64_t f )
{
    // nothing to do for image sequences
    int64_t frame = f;
    find_image( frame );
}


void CMedia::wait_image()
{
    mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
    SCOPED_LOCK( vpm );

    for(;;)
    {
        if ( stopped() ) break;

        if ( ! _video_packets.empty() )
        {
            break;
        }

        CONDITION_WAIT( _video_packets.cond(), vpm );
    }
}


CMedia::DecodeStatus CMedia::handle_video_seek( int64_t& frame,
        const bool is_seek )
{
    Mutex& vpm = _video_packets.mutex();
    SCOPED_LOCK( vpm );

    if ( is_seek && _video_packets.is_seek() )
    {
        assert( !_video_packets.empty() );
        _video_packets.pop_front();  // pop seek begin packet
    }
    else if ( !is_seek && _video_packets.is_preroll() )
    {
        assert( !_video_packets.empty() );
        _video_packets.pop_front();
    }
    else
        IMG_ERROR( "handle_video_packet_seek error - no seek/preroll packet" );

    DecodeStatus got_video = kDecodeMissingFrame;
    unsigned count = 0;

    while ( !_video_packets.empty() && !_video_packets.is_seek_end() )
    {
        count += 1;

        if ( !is_seek && playback() == kBackwards )
        {
            got_video = kDecodeOK;
        }
        else
        {
            got_video = kDecodeOK;
        }

        assert( !_video_packets.empty() );
        _video_packets.pop_front();
    }

    if ( _video_packets.empty() ) return kDecodeError;

    if ( count != 0 && is_seek )
    {
        const AVPacket& pkt = _video_packets.front();
        AVStream* stream = get_video_stream();
        if ( stream )
            frame = pts2frame( stream, pkt.dts );
        else
            frame = pkt.dts;
    }

    if ( _video_packets.is_seek_end() )
    {
        assert( !_video_packets.empty() );
        _video_packets.pop_front();  // pop seek end packet
    }

#ifdef DEBUG_VIDEO_PACKETS
    debug_video_packets(frame, "AFTER HSEEK");
#endif

#ifdef DEBUG_VIDEO_STORES
    debug_video_stores(frame, "AFTER HSEEK");
#endif
    return got_video;
}

void CMedia::debug_video_stores(const int64_t frame,
                                const char* routine,
                                const bool detail )
{
    std::cerr << this << std::dec << " " << name()
              << " S:" << _frame << " D:" << _dts
              << " A:" << frame << " " << routine << " image stores "
              << ": ";

    if (detail && _sequence )
    {
        uint64_t i = 0;
        uint64_t num = _frame_end - _frame_start + 1;
        for ( ; i < num; ++i )
        {
            int64_t f = i + _frame_start - 1;
            if ( f == frame )  std::cerr << "P";
            if ( f == _dts )   std::cerr << "D";
            if ( f == _frame ) std::cerr << "F";
            if ( _sequence[i] )
            {
                std::cerr << i << " ";
            }
        }
        std::cerr << std::endl;
    }
}

CMedia::DecodeStatus CMedia::decode_video( int64_t& frame )
{
    if ( ( playback() == kStopped ) && _right_eye && _stereo_output ) {
        int64_t f = frame;
        _right_eye->decode_video(f);
    }

    mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
    SCOPED_LOCK( vpm );

    if ( _video_packets.empty() )
        return kDecodeMissingFrame;



    while ( !_video_packets.empty() )
    {
        if ( _video_packets.is_flush() )
        {
            flush_video();
            _video_packets.pop_front();
            continue;
        }
        else if ( _video_packets.is_seek() )
        {
            handle_video_seek( frame, true );
            continue;
        }
        else if ( _video_packets.is_preroll() )
        {
            handle_video_seek( frame, false );
            continue;
        }
        else if ( _video_packets.is_loop_start() )
        {
            // We check packet integrity as the length of packets is
            // not accurate.
            const AVPacket& pkt = _video_packets.front();
            if ( frame > pkt.pts ) {
                // std::cerr << name() << " frame " << frame
                //        << " pkt.pts " << pkt.pts
                //        << std::endl;
                return kDecodeOK;
            }

            // std::cerr << name() << " frame " << frame << " pkt.pts " <<  pkt.pts
            //        << " OK!!!" << std::endl;

            _video_packets.pop_front();
            return kDecodeLoopStart;
        }
        else if ( _video_packets.is_loop_end() )
        {
            // We check packet integrity as the length of packets is
            // not accurate.
            const AVPacket& pkt = _video_packets.front();
            if ( frame < pkt.pts ) return kDecodeOK;

            _video_packets.pop_front();
            return kDecodeLoopEnd;
        }
        else
        {
            assert( !_video_packets.empty() );
            _video_packets.pop_front();
            return kDecodeOK;
        }

    }


    return kDecodeMissingFrame;
}

CMedia::DecodeStatus CMedia::decode_subtitle( const int64_t frame )
{
    return kDecodeOK;
}

bool CMedia::find_subtitle( const int64_t f )
{
    return false;
}

int64_t CMedia::loops_offset( int64_t f,
                              const int64_t frame ) const
{
    if ( looping() == kLoop )
    {
        while ( frame < f )
        {
            int64_t len = duration();
            f += len;
        }
        // std::cerr << frame << " len " << len << " f " << f << std::endl;
    }
    else if ( looping() == kPingPong )
    {
        int64_t len = duration();
        while ( frame < f )
        {
            int64_t len = duration();
            f += len;
        }
    }
    return f;
}


int64_t CMedia::handle_loops( const int64_t frame ) const
{
    int64_t f = frame;

    if ( looping() == kLoop )
    {
        int64_t len = duration();
        if ( len > 0 )
        {
            f = modE(f-_frameStart, len) + _frameStart;
        }
    }
    else if ( looping() == kPingPong )
    {
        int64_t len = duration();
        if ( len > 0 )
        {
            f -= _frameStart;
            int64_t v   = f / len;
            f = modE( f, len ) + _frameStart;
            if ( modE( v, 2 ) == 1 )
            {
                f = len - f + _frame_start;
            }
        }
    }
    else
    {
        if ( f > _frameEnd )       f = _frameEnd;
        else if ( f < _frameStart) f = _frameStart;
    }

    return f;
}

bool CMedia::find_image( const int64_t frame )
{
    if ( ( playback() == kStopped ) && _right_eye && _stereo_output )
        _right_eye->find_image(frame);


    int64_t f = handle_loops( frame );

    _video_pts   = f / _orig_fps;
    _video_clock = double(av_gettime_relative()) / 1000000.0;
    update_video_pts(this, _video_pts, 0, 0);

    // Check if we have a cached frame for this frame

    int64_t idx = f - _frame_start;
    int64_t num = _frame_end - _frame_start + 1;
    {
        //SCOPED_LOCK( _mutex );
        if ( idx < 0 ) idx = 0;
        else if ( idx >= num ) idx = num - 1;
    }

    if ( _sequence && _sequence[idx] )
    {
        SCOPED_LOCK( _mutex );
        _hires = _sequence[idx];

        if ( _right && _right[idx])
            _stereo[1] = _right[idx];

        _frame = frame;

        free(_filename);
        _filename = NULL;

        refresh();
        image_damage( image_damage() | kDamageData | kDamage3DData );
        limit_video_store(f);
        return true;
    }


    bool should_load = false;

    std::string file = sequence_filename(f);
    std::string old  = sequence_filename(_frame);

    if ( file != old )
    {

        should_load = true;
        free( _filename );
        _filename = strdup( file.c_str() );

    }

    _frame = f;

    if ( should_load )
    {
        image_type_ptr canvas;
        if ( fs::exists(file) )
        {
            SCOPED_LOCK( _mutex );
            SCOPED_LOCK( _audio_mutex );
            SCOPED_LOCK( _subtitle_mutex );
            if ( fetch( canvas, f ) )
            {
                cache( canvas );
                default_color_corrections();
            }
            else
            {
                if ( idx > 1 && _sequence[idx-1] )
                {
                    // If we run out of memory, make sure we sweep the
                    // frames we have in memory.
                    size_t data_size = _sequence[idx-1]->data_size();
                    int64_t maxmem = (idx-2) * data_size;
                    Preferences::max_memory = maxmem;
                    LOG_INFO( "[mem] Max memory is now " << maxmem );
                    limit_video_store( frame );
                    if ( fetch( canvas, f ) )
                    {
                        cache( canvas );
                        default_color_corrections();
                    }
                }
            }
        }
        else
        {
            if ( ! internal() )
            {
                if ( Preferences::missing_frame == Preferences::kBlackFrame )
                {
                    canvas =
                    mrv::image_type_ptr(
                                        new image_type( frame,
                                                        width(),
                                                        height(),
                                                        1,
                                                        image_type::kLumma,
                                                        image_type::kByte ) );
                    memset( canvas->data().get(), 0x0, canvas->data_size() );
                    canvas->valid( false ); // mark this frame as invalid
                    cache( canvas );
                    default_color_corrections();
                    IMG_WARNING( file << _(" is missing.") );
                }
                else
                {
                    // REPEATS LAST FRAME
                    if ( idx >= 0 && idx < num )
                    {
                        for ( ; ( !_sequence[idx] ||
                                  ( _sequence[idx] && !_sequence[idx]->valid() ));
                        --idx )
                        {
                        }


                        if ( idx >= 0 )
                        {
                            const mrv::image_type_ptr old = _sequence[idx];
                            canvas =
                            mrv::image_type_ptr( new image_type( *old ) );
                            canvas->frame( f );
                            canvas->valid( false ); // mark this frame as invalid
                            cache( canvas );
                            default_color_corrections();
                            IMG_WARNING( file << _(" is missing. Choosing ")
                                         << old->frame() << "." );
                        }
                    }
                }
		_hires = canvas;
                refresh();
                return true;
            }
        }
    }


    limit_video_store( f );

    refresh();
    image_damage( image_damage() | kDamageData | kDamage3DData );
    return true;
}


void CMedia::default_icc_profile()
{
    if ( icc_profile() ) return;

    if ( internal() ) return;

    switch( depth() )
    {
    case image_type::kByte:
        if ( !icc_profile_8bits.empty() )
            icc_profile( icc_profile_8bits.c_str() );
        break;
    case image_type::kShort:
        if ( !icc_profile_16bits.empty() )
            icc_profile( icc_profile_16bits.c_str() );
        break;
    case image_type::kInt:
        if ( !icc_profile_32bits.empty() )
            icc_profile( icc_profile_32bits.c_str() );
        break;
    case image_type::kHalf:
    case image_type::kFloat:
        if ( !icc_profile_float.empty() )
            icc_profile( icc_profile_float.c_str() );
        break;
    default:
        IMG_ERROR("default_icc_profile - unknown bit depth");
        break;
    }
}


void CMedia::default_ocio_input_color_space()
{
    if ( !ocio_input_color_space().empty() ) return;

    if ( internal() ) return;

    if ( ! Preferences::use_ocio ) return;

    std::string n = filename();
    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();
    std::string cs = config->parseColorSpaceFromString(n.c_str());
    if ( !cs.empty() )
    {
        IMG_INFO( _("Got colorspace '") << cs << _("' from filename") );
        ocio_input_color_space( cs );
        return;
    }

    const char* bit_depth = _("unknown");
    switch( depth() )
    {
    case image_type::kByte:
        if ( !ocio_8bits_ics.empty() )
        {
            bit_depth = _("8 bits");
            ocio_input_color_space( ocio_8bits_ics.c_str() );
        }
        break;
    case image_type::kShort:
        if ( !ocio_16bits_ics.empty() )
        {
            bit_depth = _("16 bits");
            ocio_input_color_space( ocio_16bits_ics.c_str() );
        }
        break;
    case image_type::kInt:
        if ( !ocio_32bits_ics.empty() )
        {
            bit_depth = _("32 bits");
            ocio_input_color_space( ocio_32bits_ics.c_str() );
        }
        break;
    case image_type::kHalf:
    case image_type::kFloat:
        if ( !ocio_float_ics.empty() )
        {
            bit_depth = _("half/float");
            ocio_input_color_space( ocio_float_ics.c_str() );
        }
        break;
    default:
        IMG_ERROR( _("default_ocio_input_color_space - "
                     "unknown bit depth") );
        break;
    }

    if (! ocio_input_color_space().empty() )
    {
        IMG_INFO( _("Got colorspace '") << ocio_input_color_space()
                  << _("' from bitdepth ") << bit_depth << _(" as default") );
        return;
    }
    if ( n.rfind( ".dpx" ) != std::string::npos ||
            n.rfind( ".cin")  != std::string::npos )
    {
        OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();
        OCIO::ConstColorSpaceRcPtr defaultcs = config->getColorSpace("Cineon");
        if ( ! defaultcs )
            defaultcs = config->getColorSpace("lgf");
        if ( ! defaultcs )
            defaultcs = config->getColorSpace("lg10");
        if ( defaultcs )
        {
            IMG_INFO( "Got colorspace '" << defaultcs->getName()
                      << "' for dpx/cin extension");
            ocio_input_color_space( defaultcs->getName() );
        }
    }
}

void CMedia::ocio_input_color_space( const std::string& n )
{
    if ( _input_color_space == n || is_thumbnail() ) return;

    _input_color_space = n;
    image_damage( image_damage() | kDamageData | kDamageLut );
}

void CMedia::default_rendering_transform()
{
    if ( rendering_transform() ) return;

    if ( internal() ) return;

    switch( depth() )
    {
    case image_type::kByte:
        if ( !rendering_transform_8bits.empty() )
            rendering_transform( rendering_transform_8bits.c_str() );
        break;
    case image_type::kShort:
        if ( !rendering_transform_16bits.empty() )
            rendering_transform( rendering_transform_16bits.c_str() );
        break;
    case image_type::kInt:
        if ( !rendering_transform_32bits.empty() )
            rendering_transform( rendering_transform_32bits.c_str() );
        break;
    case image_type::kHalf:
    case image_type::kFloat:
        if ( !rendering_transform_float.empty() )
            rendering_transform( rendering_transform_float.c_str() );
        break;
    default:
        IMG_ERROR("default_rendering_tranform - unknown bit depth");
        break;
    }
}

// Outputs the indices of the stream that are keyframes
// (keyframes are used only for video streams)
void CMedia::debug_stream_keyframes( const AVStream* stream )
{
    if ( stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO ) return;

    int64_t  max_distance  = 0;
    unsigned num_keyframes = 0;

    int num = stream->nb_index_entries;
    LOG_INFO( "# indices: " << num );
    if ( num <= 0 ) return;

    int64_t last_frame = pts2frame( stream,
                                    stream->index_entries[0].timestamp );

    for ( int i = 0; i < num; ++i )
    {
        const AVIndexEntry& e = stream->index_entries[i];
        if ( ! (e.flags & AVINDEX_KEYFRAME) ) continue;

        int64_t frame = pts2frame( stream, e.timestamp );
        int64_t dist  = frame - last_frame;
        if ( dist > max_distance ) max_distance = dist;
        last_frame = frame;

        ++num_keyframes;
    }

    LOG_INFO( "# keyframes: " << num_keyframes );
    LOG_INFO( " max frames: " << max_distance );

    for ( int i = 0; i < num; ++i )
    {
        const AVIndexEntry& e = stream->index_entries[i];
        if ( ! (e.flags & AVINDEX_KEYFRAME) ) continue;

        LOG_INFO( "\t#" << i
                  << " pos: " << e.pos << " pts: " << e.timestamp
                  << " frame: " << pts2frame( stream, e.timestamp ) );
        LOG_INFO( "\t\tflags: "<< e.flags
                  << " size: " << e.size << " dist: " << e.min_distance );
    }

}

// Outputs the indices of the stream
void CMedia::debug_stream_index( const AVStream* stream )
{
    int num = stream->nb_index_entries;
    LOG_INFO( "# indices: " << num );
    for ( int i = 0; i < num; ++i )
    {
        const AVIndexEntry& e = stream->index_entries[i];
        LOG_INFO( "\t#" << i
                  << ( e.flags & AVINDEX_KEYFRAME ? "K" : " " )
                  << " pos: " << e.pos << " pts: " << e.timestamp
                  << " frame: " << pts2frame( stream, e.timestamp ) );
        LOG_INFO( "\t\tflags: "<< e.flags
                  << " size: " << e.size
                  << " dist: " << e.min_distance );
    }
}



void CMedia::debug_video_packets(const int64_t frame,
                                 const char* routine,
                                 const bool detail)
{

    mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
    SCOPED_LOCK( vpm );

    mrv::PacketQueue::const_iterator iter = _video_packets.begin();
    mrv::PacketQueue::const_iterator last = _video_packets.end();
    std::cerr << name() << " S:" << _frame << " D:" << _dts << " V:" << frame
              << " " << routine << " video packets #"
              << _video_packets.size() << " (bytes:" << _video_packets.bytes() << "): ";

    AVStream* stream = get_video_stream();

    if ( iter == last )
    {
        std::cerr << std::endl << "***EMPTY***";
    }
    else
    {
        std::cerr << std::dec;
        if ( _video_packets.is_loop_end( *iter ) ||
             _video_packets.is_loop_start( *iter ) )
        {
            std::cerr << (*iter).dts;
        }
        else
        {
            if ( stream )
                std::cerr << pts2frame( stream, (*iter).dts ) - _frame_offset;
            else
                std::cerr << (*iter).dts - _frame_offset;
        }

        std::cerr << '-';

        if ( _video_packets.is_loop_end( *(last-1) ) ||
             _video_packets.is_loop_start( *(last-1) ) )
        {
            std::cerr << (*(last-1)).dts - _frame_offset;
        }
        else
        {
            if ( stream )
                std::cerr << pts2frame( stream, (*(last-1)).dts ) - _frame_offset;
            else
                std::cerr << (*(last-1)).dts - _frame_offset;
        }

        std::cerr << std::endl;
    }



    if ( detail )
    {
        bool in_preroll = false;
        bool in_seek = false;
        for ( ; iter != last; ++iter )
        {
            if ( _video_packets.is_flush( *iter ) )
            {
                std::cerr << "* ";
                continue;
            }
            else if ( _video_packets.is_loop_start( *iter ) ||
                      _video_packets.is_loop_end( *iter ) )
            {
                std::cerr << "L" << (*iter).dts << " ";
                continue;
            }

            assert( (*iter).dts != MRV_NOPTS_VALUE );

            int64_t f;
            if ( (*iter).pts != MRV_NOPTS_VALUE )
                f = (*iter).pts;
            else
            {
                f = (*iter).dts;
            }

            if ( stream )   f = pts2frame( stream, f );

            if ( _video_packets.is_seek_end( *iter ) )
            {
                if ( in_preroll )
                {
                    std::cerr << "[PREROLL END: " << f << "]";
                    in_preroll = false;
                }
                else if ( in_seek )
                {
                    std::cerr << "<SEEK END:" << f << ">";
                    in_seek = false;
                }
                else
                {
                    std::cerr << "+ERROR:" << f << "+";
                }
            }
            else if ( _video_packets.is_seek( *iter ) )
            {
                std::cerr << "<SEEK:" << f << ">";
                in_seek = true;
            }
            else if ( _video_packets.is_preroll( *iter ) )
            {
                std::cerr << "[PREROLL:" << f << "]";
                in_preroll = true;
            }
            else
            {
                if ( f == frame )  std::cerr << "S";
                if ( f == _dts )   std::cerr << "D";
                if ( f == _frame ) std::cerr << "F";
                std::cerr << f - _frame_offset << " ";
            }
        }
        std::cerr << std::endl << std::endl;
    }

}


void CMedia::add_shape( mrv::shape_type_ptr s )
{
    _shapes.push_back( s );
    _undo_shapes.clear();
}

char *const get_error_text(const int error)
{
    static char error_buffer[255];
    if ( error < 0 )
        av_strerror(error, error_buffer, sizeof(error_buffer));
    else
    {
        switch( error )
        {
        case CMedia::kDecodeMissingFrame:
            strcpy( error_buffer, _( "Decode Missing Frame" ) );
            break;
        case CMedia::kDecodeOK:
            strcpy( error_buffer, _( "Decode OK" ) );
            break;
        case CMedia::kDecodeDone:
            strcpy( error_buffer, _( "Decode Done" ) );
            break;
        case CMedia::kDecodeError:
            strcpy( error_buffer, _( "Decode Error" ) );
            break;
        case CMedia::kDecodeMissingSamples:
            strcpy( error_buffer, _( "Decode Missing Samples" ) );
            break;
        case CMedia::kDecodeNoStream:
            strcpy( error_buffer, _( "Decode No Stream" ) );
            break;
        case CMedia::kDecodeLoopStart:
            strcpy( error_buffer, _( "Decode Loop Start" ) );
            break;
        case CMedia::kDecodeLoopEnd:
            strcpy( error_buffer, _( "Decode Loop End" ) );
            break;
        case CMedia::kDecodeBufferFull:
            strcpy( error_buffer, _( "Decode Buffer Full" ) );
            break;
        }
    }
    return error_buffer;
}

} // namespace mrv
