/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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

#define  __STDC_CONSTANT_MACROS

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

#include <FL/Fl.H>

#undef  __STDC_CONSTANT_MACROS

#include <boost/cstdint.hpp>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
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


//#include <MagickWand/MagickWand.h>

#include "core/mrvMath.h"
#include "core/CMedia.h"
#include "core/aviImage.h"
#include "core/R3dImage.h"
#include "core/brawImage.h"
#include "core/clonedImage.h"
#include "core/mrvColorBarsImage.h"
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

#include "mrvVersion.h"

namespace {

const char* kModule = "img";

}


// #define DEBUG_SEEK
// #define DEBUG_VIDEO_PACKETS
//#define DEBUG_STORES

// #define DEBUG_DECODE
// #define DEBUG_AUDIO_SPLIT

#define MEM()
//#define MEM() std::cerr << memory_used << " " << __FUNCTION__ << " " << __LINE__ << std::endl;


namespace mrv {

static AVRational timeBaseQ = { 1, AV_TIME_BASE };

bool       CMedia::_initialize = false;

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


std::atomic<int64_t> CMedia::memory_used( 0 );
double CMedia::thumbnail_percent = 0.0f;

int CMedia::_audio_cache_size = 0;
int CMedia::_video_cache_size = 0;
int CMedia::_image_cache_size = 0;

std::string CMedia::_default_subtitle_font = "Arial";
std::string CMedia::_default_subtitle_encoding = "utf-8";
bool CMedia::_aces_metadata = false;
bool CMedia::_ocio_color_space = false;
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
_cache_full( 0 ),
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
_play_fps( 24.0f ),
_fps( 24.0f ),
_orig_fps( 24.0f ),
_pixel_ratio( NULL ),
_num_channels( 0 ),
_rendering_intent( kUndefinedIntent ),
_gamma( 1 ),
_dissolve( 1 ),
_dissolve_start( std::numeric_limits<int64_t>::min() ),
_dissolve_end( std::numeric_limits<int64_t>::max() ),
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
_frameIn( 1 ),
_frameOut( 1 ),
_frame_start( 1 ),
_frame_end( 1 ),
_start_number( 0 ),
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
_audio_max( 0 ),
_profile( NULL ),
_frame_offset( 0 ),
_playback( kStopped ),
_sequence( NULL ),
_right( NULL ),
_actual_frame_rate( 0 ),
_context(NULL),
_video_ctx( NULL ),
_acontext(NULL),
_audio_ctx( NULL ),
_audio_codec(NULL),
_subtitle_index(-1),
_subtitle_encoding( av_strdup( _default_subtitle_encoding.c_str() ) ),
_subtitle_font( av_strdup( _default_subtitle_font.c_str() ) ),
_owns_right_eye( true ),
_flipX( false ),
_flipY( false ),
_last_audio_cached( false ),
_audio_index(-1),
_samples_per_sec( 0 ),
_audio_buf_used( 0 ),
_audio_last_frame( 0 ),
_audio_channels( 0 ),
_aframe( NULL ),
_audio_format( AudioEngine::kFloatLSB ),
_audio_buf( NULL ),
forw_ctx( NULL ),
_audio_engine( NULL ),
_video_thread( NULL ),
_audio_thread( NULL ),
_decode_thread( NULL )
{

    gettimeofday (&_lastFrameTime, 0);
    _lastFpsFrameTime = _lastFrameTime;
    _framesSinceLastFpsFrame = 0;

    audio_initialize();
    if ( ! _initialize )
    {
        mrv::PacketQueue::initialize();
        _initialize = true;
    }

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
_play_fps( 24.0f ),
_fps( 24.0f ),
_orig_fps( 24.0f ),
_pixel_ratio( NULL ),
_num_channels( 0 ),
_rendering_intent( kUndefinedIntent ),
_gamma( 1 ),
_dissolve( 1 ),
_dissolve_start( std::numeric_limits<int64_t>::min() ),
_dissolve_end( std::numeric_limits<int64_t>::max() ),
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
_frameIn( 1 ),
_frameOut( 1 ),
_frame_start( 1 ),
_frame_end( 1 ),
_start_number( 0 ),
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
_audio_max( 0 ),
_profile( NULL ),
_playback( kStopped ),
_sequence( NULL ),
_right( NULL ),
_actual_frame_rate( 0 ),
_context(NULL),
_video_ctx( NULL ),
_acontext(NULL),
_audio_ctx( NULL ),
_audio_codec(NULL),
_subtitle_index(-1),
_subtitle_encoding( av_strdup( _default_subtitle_encoding.c_str() ) ),
_subtitle_font( av_strdup( _default_subtitle_font.c_str() ) ),
_owns_right_eye( true ),
_flipX( false ),
_flipY( false ),
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
_audio_engine( NULL ),
_video_thread( NULL ),
_audio_thread( NULL ),
_decode_thread( NULL )
{
    gettimeofday (&_lastFrameTime, 0);
    _lastFpsFrameTime = _lastFrameTime;
    _framesSinceLastFpsFrame = 0;

    unsigned int W = other->width();
    unsigned int H = other->height();
    image_size( W, H );

    float xScale = (float)ws / (float)W;
    float yScale = (float)wh / (float)H;

    {
        CMedia* img = const_cast< CMedia* >( other );
        CMedia::Mutex& m = img->video_mutex();
        SCOPED_LOCK(m);
        _hires = img->left();

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
_is_thumbnail( other->_is_thumbnail ),
_is_sequence( other->_is_sequence ),
_is_stereo( other->_is_stereo ),
_stereo_input( other->_stereo_input ),
_stereo_output( other->_stereo_output ),
_looping( other->looping() ),
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
_pixel_ratio( NULL ),
_num_channels( other->_num_channels ),
_rendering_intent( other->_rendering_intent ),
_gamma( other->_gamma ),
_dissolve( other->_dissolve ),
_dissolve_start( other->_dissolve_start ),
_dissolve_end( other->_dissolve_end ),
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
_frameIn( other->_frameIn ),
_frameOut( other->_frameOut ),
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
_audio_max( 0 ),
_profile( NULL ),
_playback( kStopped ),
_sequence( NULL ),
_right( NULL ),
_actual_frame_rate( 0 ),
_context(NULL),
_video_ctx( NULL ),
_acontext(NULL),
_audio_ctx( NULL ),
_audio_codec(NULL),
_subtitle_index(-1),
_subtitle_encoding( av_strdup( other->_subtitle_encoding ) ),
_subtitle_font( av_strdup( other->_subtitle_font ) ),
_owns_right_eye( false ),
_flipX( other->flipX() ),
_flipY( other->flipY() ),
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
_audio_engine( NULL ),
_video_thread( NULL ),
_audio_thread( NULL ),
_decode_thread( NULL )
{
    gettimeofday (&_lastFrameTime, 0);
    _lastFpsFrameTime = _lastFrameTime;
    _framesSinceLastFpsFrame = 0;

    _fileroot = av_strdup( other->fileroot() );
    _filename = av_strdup( other->filename() );

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
        cache( canvas );
        default_color_corrections();
    }
}

int64_t CMedia::get_frame( const AVStream* stream, const AVPacket& pkt )
{
    int64_t frame = AV_NOPTS_VALUE;
    if ( pkt.pts != AV_NOPTS_VALUE )
    {
        frame = pts2frame( stream, pkt.pts );
    }
    else
    {
//         if ( pkt.dts != AV_NOPTS_VALUE )
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

    _cache_full = 0;

    boost::uint64_t num = _frame_end - _frame_start + 1;
    for ( boost::uint64_t i = 0; i < num; ++i )
    {
        if ( _sequence && _sequence[i] )
        {
            _sequence[i].reset();
        }
        if ( _right && _right[i] )
        {
            _right[i].reset();
        }
    }

    if ( _stereo[0] )
    {
        _stereo[0].reset();
    }

    if ( _stereo[1] )
    {
        _stereo[1].reset();
    }

    image_damage( image_damage() | kDamageCache | kDamageContents );

}

void CMedia::update_frame( const int64_t& f )
{
    if ( !_sequence ) return;

    if ( f < _frame_start || f > _frame_end ) return;

    SCOPED_LOCK( _mutex);

    _cache_full = 0;

    boost::uint64_t i = f - _frame_start;
    if ( _sequence[i] )        _sequence[i].reset();
    if ( _right && _right[i] )    _right[i].reset();

    _hires.reset();
    _stereo[0].reset();
    _stereo[1].reset();

    image_type_ptr canvas;
    if ( fetch( canvas, f ) )
    {
        cache( canvas );
        default_color_corrections();
    }

    image_damage( image_damage() | kDamageCache | kDamageContents );

}

/**
 * Wait for all threads to finish and exit.  Delete them afterwards.
 *
 */
void CMedia::wait_for_threads()
{
#if 1
    for ( const auto& i : _threads )
    {
        if ( i->joinable() && i->get_id() != boost::this_thread::get_id() )
            i->join();
        std::string type = "unknown";
        if ( i == _video_thread ) type = "video";
        if ( i == _audio_thread ) type = "audio";
        if ( i == _decode_thread ) type = "decode";
        TRACE2( name() << " Thread " << i << ", type " << type << " returned" );
        delete i;
    }
#else
    for ( const auto& i : _threads )
    {
        boost::posix_time::time_duration timeout =
        boost::posix_time::milliseconds(100000);
        bool ok = false;
        while( !ok )
        {
            ok = i->timed_join(timeout);
            std::string type = "subtitle";
            if ( i == _video_thread ) type = "video";
            if ( i == _audio_thread ) type = "audio";
            if ( i == _decode_thread ) type = "decode";
            TRACE2( name() << " Thread " << i << ", type " << type << " returned " << ok );
        }
        delete i;
    }
#endif

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


    if ( _owns_right_eye )
    {
        delete _right_eye;
    }

    _right_eye = NULL;

    image_damage( kNoDamage );


    av_free( _channel );

    av_free( _label );

    av_free( _profile );


    clear_look_mod_transform();



    delete [] _dataWindow;
    _dataWindow = NULL;

    delete [] _dataWindow2;
    _dataWindow2 = NULL;

    delete [] _displayWindow;
    _displayWindow = NULL;

    delete [] _displayWindow2;
    _displayWindow2 = NULL;

    delete [] _pixel_ratio;
    _pixel_ratio = NULL;


    av_free( _subtitle_encoding );
    _subtitle_encoding = NULL;

    av_free( _subtitle_font );
    _subtitle_font = NULL;

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



    av_free( _fileroot );
    _fileroot = NULL;

    av_free( _filename );
    _filename = NULL;

    if ( _context )
    {

        avformat_close_input( &_context );
    }

    if ( _acontext )
    {

        avformat_close_input( &_acontext );
    }


    {

        for ( const auto& i : _attrs )
        {
            for ( const auto& d : i.second )
            {
                delete d.second;
            }
        }
    }

#ifdef LINUX
    malloc_trim(0);
#endif

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

CMedia::Attributes& CMedia::attributes()  {
    static Attributes empty;
    AttributesFrame::iterator i;
    if ( dynamic_cast< aviImage* >( this )  != NULL ||
         dynamic_cast< R3dImage* >( this )  != NULL ||
         start_frame() == end_frame() )
        i = _attrs.find( start_frame() );
    else
        i = _attrs.find( _frame );
    if ( i != _attrs.end() )
        return i->second;
    return empty;
}

const CMedia::Attributes& CMedia::attributes() const {
    static Attributes empty;
    AttributesFrame::const_iterator i;
    if ( dynamic_cast< const aviImage* const >( this )  != NULL ||
         dynamic_cast< const R3dImage* const >( this )  != NULL ||
         start_frame() == end_frame() )
        i = _attrs.find( start_frame() );
    else
        i = _attrs.find( _frame );
    if ( i != _attrs.end() )
        return i->second;
    return empty;
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
                              size_t w, size_t h)
{
    if ( w == 0 )
    {
        w = width();
        h = height();
    }
    assert( w != 0 && h != 0 );

    //image_damage( image_damage() & ~kDamageContents );
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

/// Set the image pixel ratio
void CMedia::pixel_ratio( int64_t f, double p ) {
  if ( !_pixel_ratio )
    {
      int64_t num;
      if ( dynamic_cast< aviImage* >( this ) != NULL ||
           dynamic_cast< brawImage* >( this ) != NULL ||
           dynamic_cast< R3dImage* >( this ) != NULL ||
           dynamic_cast< clonedImage* >( this ) != NULL||
           dynamic_cast< ColorBarsImage* >( this ) != NULL )
        num = 1;
      else
        num = _frame_end - _frame_start + 1;
      _pixel_ratio = new double[ num ];
      for ( unsigned i = 0; i < num; ++i )
          _pixel_ratio[i] = p;
    }
  if ( f > _frame_end ) f = _frame_end;
  else if ( f < _frame_start ) f = _frame_start;
  int64_t idx = f - _frame_start;
  if ( dynamic_cast< aviImage* >(this) != NULL ||
       dynamic_cast< brawImage* >( this ) != NULL ||
       dynamic_cast< R3dImage* >( this ) != NULL ||
       dynamic_cast< clonedImage* >( this ) != NULL ||
       dynamic_cast< ColorBarsImage* >( this ) != NULL )
    idx = 0;
  _pixel_ratio[idx] = p;
  refresh();
}

double CMedia::pixel_ratio() const    {
  int64_t idx = _frame - _frame_start;
  if ( idx < 0 ) idx = 0;
  else if ( idx > _frame_end - _frame_start ) idx = _frame_end - _frame_start;
  if ( dynamic_cast< const aviImage* >(this) != NULL ||
       dynamic_cast< const brawImage* >( this ) != NULL ||
       dynamic_cast< const R3dImage* >( this ) != NULL ||
       dynamic_cast< const clonedImage* >( this ) != NULL ||
       dynamic_cast< const ColorBarsImage* >( this ) != NULL )
    idx = 0;
  if ( !_pixel_ratio ) return 1.0f;
  return _pixel_ratio[idx];
}

mrv::image_type_ptr CMedia::left() const
{
    Mutex& mtx = const_cast< Mutex& >( _mutex );
    SCOPED_LOCK( mtx );

    int64_t f = _frame;
    int64_t idx = f - _frame_start;

    int64_t num = _frame_end - _frame_start;
    if ( idx > num ) idx = num;
    else if ( idx < 0 ) idx = 0;

    CMedia* img = const_cast< CMedia* >( this );
    mrv::image_type_ptr pic = _hires;

    if ( _is_sequence && _sequence && _sequence[idx] )  pic = _sequence[idx];
    else if ( _stereo[0] )  pic = _stereo[0];
    if ( !pic ) {
        return pic;
    }

    img->_w = pic->width();
    img->_h = pic->height();
    return pic;
}

mrv::image_type_ptr CMedia::right() const
{
    int64_t f = _frame;
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
    {
        _right_eye->stereo_input( stereo_input() );
        return _right_eye->display_window(f);
    }

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
    {
        _right_eye->stereo_input( stereo_input() );
        return _right_eye->data_window(f);
    }

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
    uint64_t num = _frame_end - _frame_start + 1;
    if ( !_displayWindow )
        _displayWindow = new mrv::Recti[ (unsigned) num];

    int64_t f = frame;
    if ( f == AV_NOPTS_VALUE ) f = _frame;

    int64_t idx = f - _frame_start;

    if ( idx >= (int64_t)num || idx < 0 ) return;

    // assert( idx < num );

    SCOPED_LOCK( _data_mutex );
    _displayWindow[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
    image_damage( image_damage() | kDamageData );

}

void CMedia::display_window2( const int xmin, const int ymin,
                              const int xmax, const int ymax,
                              const int64_t& frame )
{
    // assert( xmax >= xmin );
    // assert( ymax >= ymin );
    uint64_t num = _frame_end - _frame_start + 1;
    if ( !_displayWindow2 )
        _displayWindow2 = new mrv::Recti[ (unsigned)num];

    int64_t f = frame;
    if ( f == AV_NOPTS_VALUE ) f = _frame;

    int64_t idx = f - _frame_start;

    if ( idx >= (int64_t) num || idx < 0 ) return;

    // assert( idx < num );

    SCOPED_LOCK( _data_mutex );
    _displayWindow2[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
    image_damage( image_damage() | kDamageData );

}

void CMedia::data_window( const int xmin, const int ymin,
                          const int xmax, const int ymax,
                          const int64_t& frame )
{
    // assert( xmax >= xmin );
    // assert( ymax >= ymin );
    uint64_t num = _frame_end - _frame_start + 1;
    if ( !_dataWindow )
        _dataWindow = new mrv::Recti[ (unsigned) num];

    int64_t f = frame;
    if ( f == AV_NOPTS_VALUE ) f = _frame;

    int64_t idx = f - _frame_start;

    if ( idx >= (int64_t) num || idx < 0 ) return;

    SCOPED_LOCK( _data_mutex );

    // assert( idx < num );

    _dataWindow[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
    image_damage( image_damage() | kDamageData );
    //
}


void CMedia::data_window2( const int xmin, const int ymin,
                           const int xmax, const int ymax,
                           const int64_t& frame )
{
    // assert( xmax >= xmin );
    // assert( ymax >= ymin );
    uint64_t num = _frame_end - _frame_start + 1;
    assert( num > 0 );
    if ( !_dataWindow2 )
        _dataWindow2 = new mrv::Recti[ (unsigned) num];

    int64_t f = frame;
    if ( f == AV_NOPTS_VALUE ) f = _frame;

    int64_t idx = f - _frame_start;

    if ( idx >= (int64_t)num || idx < 0 ) return;

    // assert( idx < num );
    SCOPED_LOCK( _data_mutex );
    _dataWindow2[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
    image_damage( image_damage() | kDamageData );

}


/**
 * Tag a rectangle of the image for refresh
 *
 * @param r rectangle of the image to refresh.
 */
void CMedia::refresh( const mrv::Recti& r )
{
    timeval now;
    gettimeofday(&now, 0 );

    if (_playback == kStopped)
    {
        if (_framesSinceLastFpsFrame >= 24)
        {
            float t =  now.tv_sec  - _lastFpsFrameTime.tv_sec +
                       (now.tv_usec - _lastFpsFrameTime.tv_usec) * 1e-6f;

            if (t > 0)
                _actual_frame_rate = _framesSinceLastFpsFrame / t;

            _framesSinceLastFpsFrame = 0;
        }

        if (_framesSinceLastFpsFrame == 0)
            _lastFpsFrameTime = now;

        static std::atomic<int64_t> old_frame( _frame - 1 );
        int64_t frameDiff = std::abs( _frame - old_frame );
        if ( frameDiff < 10 )
            _framesSinceLastFpsFrame += frameDiff;
        old_frame.store( _frame );
    }


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
//    if ( x < _frame_start ) x = _frame_start;
    assert( x != AV_NOPTS_VALUE );
    _frameStart = _frameIn = x;
    // if ( _frame < _frame_start ) _frame = _frameStart;
}

void  CMedia::last_frame(int64_t x)
{
    assert( x != AV_NOPTS_VALUE );
//    if ( (!_is_sequence || !has_video()) && x > _frame_end ) x = _frame_end;
    _frameEnd = _frameOut = x;
    // if ( _frame > _frame_end ) _frame = _frameEnd;
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

    av_free( _fileroot );
    _fileroot = av_strdup( fileroot );

    std::string f = _fileroot;
    size_t idx = f.find( N_("%V") );
    if ( idx != std::string::npos )
    {
        _is_stereo = true;
    }

    av_free( _filename );
    _filename = NULL;


    _is_sequence = true;
    _dts = _adts = start;
    _frameStart = _frameIn = _frame_start = start;
    _frameEnd = _frameOut = _frame_end = end;


    delete [] _sequence;
    _sequence = NULL;
    delete [] _right;
    _right = NULL;

    uint64_t num = _frame_end - _frame_start + 1;


    if ( dynamic_cast< aviImage* >( this )  == NULL &&
         dynamic_cast< R3dImage* >( this )  == NULL &&
         dynamic_cast< brawImage* >( this ) == NULL )
    {
        _sequence = new mrv::image_type_ptr[ (unsigned) num ];
        _right    = new mrv::image_type_ptr[ (unsigned) num ];
    }


    if ( ! initialize() )
        return;

    image_type_ptr canvas;
    if ( fetch( canvas, start ) )
    {
        cache( canvas );
        refresh();
        default_color_corrections();
    }


    if ( has_audio() )
    {
        int64_t f = start;
        decode_audio( f );
    }
}

//! Adds default OCIO, ICC, and CTL profiles
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

    if ( name.find( "http" )    != 0 &&
         name.find( "rtp" )     != 0 &&
         name.find( "rtmp" )    != 0 &&
         name.find( "youtube" ) != 0 &&
         name.find( "bluray:" ) != 0 &&
         name.find( "dvd:" )    != 0 &&
         name.find( "www." )    != 0 )
        file = fs::absolute( file );

    av_free( _fileroot );

    if ( fs::exists( file ) )
    {
        std::string path = fs::absolute( file ).string();
        _fileroot = av_strdup( path.c_str() );
    }
    else
    {
        _fileroot = av_strdup( file.string().c_str() );
    }


    av_free( _filename );
    _filename = NULL;


    _is_sequence = false;
    _is_stereo = false;


    if ( ! initialize() )
        return;

    if ( fetch( _hires, _frameStart ) )
    {
        cache( _hires );
    }


    timestamp();

    if ( _hires ) _depth = _hires->pixel_type();
    else _depth = image_type::kByte;

    default_color_corrections();
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
    av_free( self->_filename );
    self->_filename = (char*) av_malloc( 1024 * sizeof(char) );
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

        int64_t f = _frame;
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
            int64_t f = _frame;
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
void CMedia::image_size( size_t w, size_t h )
{
    SCOPED_LOCK( _mutex );
    _w = w;
    _h = h;


    pixel_ratio( _frame_start, 1.0f );

    // Derive pixel ratio from common format resolutions
    if ( w == 720 && h == 486 )
      {
        pixel_ratio( _frame_start, 0.9f );    // 4:3 NTSC
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
      }
    else if ( w == 640 && h == 480 )
      {
        pixel_ratio( _frame_start, 1.0f );    // 4:3 NTSC
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
      }
    else if ( w == 720 && h == 480 )
      {
        pixel_ratio( _frame_start, 0.88888f );    // 4:3 NTSC
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
      }
    else if ( w == 512 && h == 486 )
      {
        pixel_ratio( _frame_start, 1.265f );  // 4:3 Targa 486
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
      }
    else if ( w == 512 && h == 482 )
      {
        pixel_ratio( _frame_start, 1.255f );  // 4:3 Targa NTSC
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
      }
    else if ( w == 512 && h == 576 )
      {
        pixel_ratio( _frame_start, 1.5f );    // 4:3 Targa PAL
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 25;
      }
    else if ( w == 646 && h == 485 )
      {
        pixel_ratio( _frame_start, 1.001f );  // 4:3 NTSC
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
      }
    else if ( w == 720 && h == 576 )
      {
        pixel_ratio( _frame_start, 1.066f );  // 4:3 PAL
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 25;
      }
    else if ( w == 780 && h == 576 )
      {
        pixel_ratio( _frame_start, 0.984f );  // 4:3 PAL
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 25;
      }
    else if ( w == 1280 && h == 1024 )
      {
        pixel_ratio( _frame_start, 1.066f ); // HDTV full
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
      }
    else if ( w == 1920 && h == 1080 )
      {
        pixel_ratio( _frame_start, 1.0f ); // HDTV full
      }
    else if ( w == 2048 && h == 1556 )
      {
        pixel_ratio( _frame_start, 1.0f ); // 2K full
      }
    else if ( w == 3840 && h == 2160 )
      {
        pixel_ratio( _frame_start, 1.0f ); // 4K HD
      }
    else if ( w == 4096 && h == 2304 )
      {
        pixel_ratio( _frame_start, 1.0f ); // 4K full
      }
    else if ( w == 7680 && h == 4320 )
      {
        pixel_ratio( _frame_start, 1.0f ); // 8K full
      }
    else if ( w == 8192 && h == 4320 )
      {
        pixel_ratio( _frame_start, 1.0f ); // 8K full
      }
    else if ( (float)w/(float)h == 1.56 )
      {
        pixel_ratio( _frame_start, 0.9f );   // HTDV/SD compromise
        if ( _fps == 0 ) _orig_fps = _fps = _play_fps = 29.97;
      }

    if ( _fps == 0 )
    {
        if ( default_fps > 0 )
            _orig_fps = _fps = _play_fps = default_fps;
        else
            _orig_fps = _fps = _play_fps = 24.0f;
    }

}



image_type::Format CMedia::pixel_format() const
{
    mrv::image_type_ptr pic = left();
    if ( pic )      return pic->format();
    else return image_type::kLumma;
}

const char* const CMedia::pixel_format_name() const
{
    mrv::image_type_ptr pic = left();
    if ( pic )      return pic->pixel_format();
    else return "Lumma";
}

image_type::PixelType CMedia::depth() const
{
    return _depth;
}

/// Returns the pixel type of the image as a string
std::string CMedia::pixel_depth() const
{
    mrv::image_type_ptr pic = left();
    if ( ! pic ) return "unknown";
    return pic->pixel_depth();
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
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
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
        if ( stopped() )
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
                 ch.size() == 1 || ch2.size() == 1 ||
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

    av_free( _channel );
    _channel = NULL;

    // Store channel without r,g,b extension
    if ( !ch.empty() )
    {
        _channel = av_strdup( ch.c_str() );
    }


    if (to_fetch)
    {
        clear_cache();
        int64_t f = _frame;
        image_type_ptr canvas;
        if ( fetch( canvas, f ) )
        {
            if ( !is_sequence() ) _hires = canvas;
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

    DBG3;
    _ctime = sbuf.st_ctime;
    _mtime = sbuf.st_mtime;
    DBG3;
    pic->ctime( sbuf.st_ctime );
    pic->mtime( sbuf.st_mtime );
    DBG3;
    _disk_space += sbuf.st_size;
    DBG3;
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
    mrv::image_type_ptr pic = left();
    if ( is_sequence() && pic )
    {
        time_t t = pic->ctime();
        if ( t != 0 )
        {
            CMedia* img = const_cast< CMedia* >(this);
            img->_ctime = pic->ctime();
            img->_mtime = pic->mtime();
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
    return _rendering_transform.name;
}

/**
 * Change the rendering transform (CTL script) for the image
 *
 * @param cfile  CTL script name
 */
void CMedia::rendering_transform( const char* cfile )
{
    av_free( _rendering_transform.name );
    _rendering_transform.name = NULL;
    if ( cfile && strlen(cfile) > 0 )
    {
        _rendering_transform.name = av_strdup( cfile );
    }
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
    refresh();
}

/**
 * Returns image IDT CTL script (or NULL if no script)
 *
 * @return CTL script or NULL
 */
const char* CMedia::idt_transform()  const
{
    return _idt_transform.name;
}

/**
 * Returns image inverse ouput device CTL script (or NULL if no script)
 *
 * @return CTL script or NULL
 */
const char* CMedia::inverse_odt_transform()  const
{
    return _inverse_odt_transform.name;
}


/**
 * Change the inverse output device transform for the image
 *
 * @param cfile  CTL script name or NULL to clear it
 */
void CMedia::inverse_odt_transform( const char* cfile )
{
    av_free( _inverse_odt_transform.name );
    _inverse_odt_transform.name = NULL;
    if ( cfile && strlen(cfile) > 0 )
        _inverse_odt_transform.name = av_strdup( cfile );
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
    refresh();
}

/**
 * Returns image inverse ouput CTL script (or NULL if no script)
 *
 * @return CTL script or NULL
 */
const char* CMedia::inverse_ot_transform()  const
{
    return _inverse_ot_transform.name;
}


/**
 * Change the inverse output transform for the image
 *
 * @param cfile  CTL script name or NULL to clear it
 */
void CMedia::inverse_ot_transform( const char* cfile )
{
    av_free( _inverse_ot_transform.name );
    _inverse_ot_transform.name = NULL;
    if ( cfile && strlen(cfile) > 0 )
        _inverse_ot_transform.name = av_strdup( cfile );
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
    refresh();
}

/**
 * Returns image inverse reference rendering CTL script (or NULL if no script)
 *
 * @return CTL script or NULL
 */
const char* CMedia::inverse_rrt_transform()  const
{
    return _inverse_rrt_transform.name;
}


/**
 * Change the inverse reference rendering transform for the image
 *
 * @param cfile  CTL script name or NULL to clear it
 */
void CMedia::inverse_rrt_transform( const char* cfile )
{
    av_free( _inverse_rrt_transform.name );
    _inverse_rrt_transform.name = NULL;
    if ( cfile && strlen(cfile) > 0 )
        _inverse_rrt_transform.name = av_strdup( cfile );
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
    refresh();
}

/**
 * Change the IDT transform for the image
 *
 * @param cfile  CTL script name
 */
void CMedia::idt_transform( const char* cfile )
{
    av_free( _idt_transform.name );
    _idt_transform.name = NULL;
    if ( cfile && strlen(cfile) > 0 ) _idt_transform.name = av_strdup( cfile );
    _idt_transform.enabled = true;
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
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
    return _look_mod_transform[idx].name;
}


void CMedia::clear_look_mod_transform()
{
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
        _look_mod_transform.push_back( TransformId( cfile ) );
    else
    {

        size_t idx = _look_mod_transform.size();
        if ( idx == 0 ) return;
        idx -= 1;
        _look_mod_transform.erase( _look_mod_transform.begin() + idx );
    }
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
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
        size_t num = strlen( (*i).name );
        if ( num < 4 ) continue;

        for ( size_t j = 0; j < num - 3; ++j )
        {
            if ( strncmp( "Node", (*i).name + j, 4 ) == 0 )
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
                                    TransformId( cfile ) );
    }
    else
    {
        _look_mod_transform.erase( _look_mod_transform.begin() + idx );
    }
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
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

    if ( cfile && strlen(cfile) > 0 )
    {
        _look_mod_transform[idx] = TransformId( cfile );
    }
    else
    {
        _look_mod_transform.erase( _look_mod_transform.begin() + idx );
    }
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
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
    av_free( _profile );
    _profile = NULL;
    if ( cfile && strlen(cfile) > 0 )
    {
        mrv::colorProfile::add( cfile );
        _profile = av_strdup( cfile );
    }
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
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
                  ViewerUI* const uiMain,
                  bool fg )
{

    if ( saving() ) return;

    assert( dir != kStopped );
    // if ( _playback == kStopped && !_threads.empty() )
    //     return;

    TRACE2( name() << " frame " << frame() << " dir= " << dir
            << " playback= " << _playback << " threads=" << _threads.size() );

    if ( _right_eye && _owns_right_eye ) _right_eye->play( dir, uiMain, fg );

    if ( dir == _playback && !_threads.empty() ) return;

    stop(fg);

    TRACE( name() << " frame " << frame() );
    _playback = dir;

    assert( uiMain != NULL );
    assert( _threads.size() == 0 );

    if ( _frame < in_frame() )  _frame = in_frame();
    else if ( _frame > out_frame() ) _frame = out_frame();

    _audio_frame = _frame.load();
    // _expected = std::numeric_limits< int64_t >::min();

    _dts = _frame.load();



    _audio_clock = double( av_gettime_relative() ) / 1000000.0;
    _video_clock = double( av_gettime_relative() ) / 1000000.0;

    _video_pts = _audio_pts = _frame  / _orig_fps;
    update_video_pts(this, _video_pts, 0, 0);
    set_clock_at(&audclk, _audio_pts, 0, _audio_clock );
    sync_clock_to_slave( &audclk, &extclk );

    _audio_buf_used = 0;

    TRACE( name() << " frame " << frame() );
    // clear all packets
    clear_packets();

    TRACE( name() << " frame " << frame() );
    // This seek is needed to sync audio playback and flush buffers
    if ( dir == kForwards ) _seek_req = true;

    TRACE( name() << " frame " << frame() );
    seek_to_position( _frame );

    TRACE( name() << " frame " << frame() );


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
        if ( _is_stereo && _right_eye && _owns_right_eye )
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
        assert( _loop_barrier );

        if ( valid_v || valid_a )
        {
            video_data = new PlaybackData( *data );
            boost::thread* t = new boost::thread(
                boost::bind( mrv::video_thread,
                             video_data ) );
            _video_thread = t;
            _threads.push_back( t );
            TRACE2( name() << " frame " << frame() << " added video thread "
                   << t );
        }

        if ( valid_a )
        {
            // Audio playback thread
            audio_data = new PlaybackData( *data );
            boost::thread* t = new boost::thread(
                boost::bind( mrv::audio_thread,
                             audio_data ) );
            _audio_thread = t;
            _threads.push_back( t );
            TRACE2( name() << " frame " << frame() << " added audio thread "
                    << t );
        }

        if ( valid_s )
        {
            // Subtitle playback thread
            subtitle_data = new PlaybackData( *data );
            boost::thread* t = new boost::thread(
                boost::bind( mrv::subtitle_thread,
                             subtitle_data ) );
            _threads.push_back( t );
            TRACE2( name() << " frame " << frame() << " added subtitle thread "
                    << t );
        }


        // If something was valid, create decode thread
        if ( valid_a || valid_v || valid_s )
        {
            boost::thread* t = new boost::thread(
                boost::bind( mrv::decode_thread,
                             data ) );
            _decode_thread = t;
            _threads.push_back( t );
            TRACE2( name() << " frame " << frame() << " added decode thread "
                    << t );
        }


        assert( (int)_threads.size() <= ( 1 + 2 * ( valid_a || valid_v ) +
                                          1 * valid_s ) );
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


    if ( _right_eye && _owns_right_eye ) _right_eye->stop(bg);

    TRACE2( name() << " stop at frame " << frame()
            << " playback = " << _playback
            << " has threads? " << _threads.empty() );



    _playback = kStopped;

    //
    //
    //

    //
    // Notify loop barrier, to exit any wait on a loop
    //



    if ( _loop_barrier )  _loop_barrier->notify_all();

    if ( _stereo_barrier ) _stereo_barrier->notify_all();
    if ( _fg_bg_barrier ) _fg_bg_barrier->notify_all();

    // Notify packets, to make sure that audio thread exits any wait lock
    // This needs to be done even if no audio is playing, as user might
    // have turned off audio, but audio thread is still active.

    _audio_packets.cond().notify_all();
    _video_packets.cond().notify_all();
    _subtitle_packets.cond().notify_all();

    TRACE( name() << " frame " << frame() );

    // Wait for all threads to exit
    wait_for_threads();

    TRACE( name() << " frame " << frame() );

    // Clear barrier

    if ( bg ) {
        TRACE( name() << " frame " << frame() );
        delete _fg_bg_barrier;
        _fg_bg_barrier = NULL;
    }

    TRACE( name() << " frame " << frame() );
    // close_audio();


    TRACE( name() << " frame " << frame() );
    // Clear any audio/video/subtitle packets
    clear_packets();

    TRACE( name() << " frame " << frame() );
    // Queue thumbnail for update
    image_damage( image_damage() | kDamageThumbnail | kDamageData );

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
#ifdef _WIN32
        path = fs::absolute( file ).string();
#else
        path = file.string(); // fails on windows mountpoints
#endif
    else
        path = file.string();
    return path;
}



/**
 * Set a new frame for the image sequence
 *  * @param f new frame
 */
bool CMedia::frame( const int64_t f )
{
    assert( _fileroot != NULL );

    if ( stopped() && _right_eye && _owns_right_eye && _stereo_output )
        _right_eye->frame(f);


    if ( Preferences::max_memory <= CMedia::memory_used )
    {
        int64_t max_frames = (int64_t) max_image_frames();
        if ( std::abs( f - _frame ) >= max_frames )
            return false;
        limit_video_store( f );
        if ( has_audio() ) limit_audio_store( f );
    }


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

    timeval now;
    gettimeofday (&now, 0);

    _timeSinceLastFrame =  now.tv_sec  - _lastFrameTime.tv_sec +
                          (now.tv_usec - _lastFrameTime.tv_usec) * 1e-6f;


    if ( f < _frameIn ) _dts = _frameIn;
    else if ( f > _frameOut )  _dts = _frameOut;
    else                       _dts = f;

    AVPacket* pkt;
    pkt = av_packet_alloc();
    pkt->dts = pkt->pts = _dts;
    pkt->size = 0;
    pkt->data = NULL;


    if ( ! is_cache_filled( _dts ) )
    {
        image_type_ptr canvas;
        std::string file = sequence_filename( _dts );

        if ( fs::exists( file ) )
        {
            timeval now;
            gettimeofday (&now, 0);
            _lastFrameTime = now;

            if ( fetch( canvas, _dts ) )
            {
                cache( canvas );
                default_color_corrections();
            }
        }
    }

    _video_packets.push_back( *pkt );

    if ( has_audio() )
    {
        fetch_audio( f + _audio_offset );
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
#define DEBUG_SEEK


    _seek_frame = f;
    _seek_req   = true;
#ifdef DEBUG_SEEK
    TRACE( name() << " frame " << f << " first "
           << _frameIn << " last " << _frameOut );
#endif


    if ( _right_eye && _owns_right_eye )
    {
#ifdef DEBUG_SEEK
        TRACE( name() << " RIGHT EYE frame " << f << " first "
               << _frameIn << " last " << _frameOut );
#endif
        _right_eye->_seek_frame = f;
        _right_eye->_seek_req = true;
    }

    if ( stopped() || saving() )
    {
#ifdef DEBUG_SEEK
        TRACE( name() << " call do_seek to " << f );
#endif
        do_seek();

        image_damage( image_damage() | kDamageData );
    }

#ifdef DEBUG_SEEK
    TRACE( "------- SEEK DONE " << f << " _dts: " << _dts << " _frame: "
           << _frame << " _expected: " << _expected );
#endif
}


void CMedia::update_cache_pic( mrv::image_type_ptr*& seq,
                               const mrv::image_type_ptr& pic )
{
    assert( pic != NULL );
    assert( pic.use_count() >= 1 );



    int64_t f = pic->frame();


    int64_t idx = f - _frame_start;
    int64_t num = _frame_end - _frame_start;
    if ( idx < 0 ) idx = 0;
    else if ( idx > num ) idx = num;

    if ( !seq ) return;


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
            seq[idx] = pic;
            // Use count should be 2, but it fails on stereo images when loaded
            // twice.
            assert( pic.use_count() >= 1 );
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
         dynamic_cast< const aviImage* >( this )  != NULL ||
         dynamic_cast< const R3dImage* >( this )  != NULL ||
         dynamic_cast< const brawImage* >( this ) != NULL )
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
    if ( dynamic_cast< const aviImage* >( this )  != NULL ||
         dynamic_cast< const R3dImage* >( this )  != NULL ||
         dynamic_cast< const brawImage* >( this ) != NULL )
        return;

    if ( !is_sequence() || !_cache_active || !pic )
        return;


    _depth = pic->pixel_type();

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

    if ( _stereo[1] && pic && _stereo[1]->frame() == pic->frame() )
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
    if ( dynamic_cast< aviImage* >( this )  != NULL ||
         dynamic_cast< R3dImage* >( this )  != NULL ||
         dynamic_cast< brawImage* >( this ) != NULL )
        return true;

    if ( _cache_full == 2 ) return true;

    for ( int64_t i = _frame_start; i <= _frame_end; ++i )
    {
        if ( is_cache_filled(i) == kNoCache ) return false;
    }

    _cache_full = 2;
    return true;
}

int64_t CMedia::first_cache_empty_frame()
{
    if ( dynamic_cast< aviImage* >( this )  != NULL ||
         dynamic_cast< R3dImage* >( this )  != NULL ||
         dynamic_cast< brawImage* >( this ) != NULL )
        return frame();

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
    if ( _right_eye && _owns_right_eye ) _right_eye->flush_all();

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
            if ( dynamic_cast< const R3dImage* >( this )  != NULL ||
                 dynamic_cast< const brawImage* >( this ) != NULL )
            {
                r += _hires->data_size() * duration();
            }
            else
            {
                r += _hires->data_size();
            }
        }
        else if ( _stereo[0] )
        {
            r += _stereo[0]->data_size();
            if ( _stereo[1] )
                r += _stereo[1]->data_size();
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
std::string CMedia::codec_name( const AVCodecParameters* par )
{
    const AVCodec* p = avcodec_find_decoder(par->codec_id);
    const char* codec_name;
    char buf[20];

    if (p) {
        codec_name = p->name;
    } else if (par->codec_id == AV_CODEC_ID_MPEG2TS) {
        /* fake mpeg2 transport stream codec (currently not
           registered) */
        codec_name = N_("mpeg2ts");
    } else {
        /* output avi tags */
        if(   isprint(par->codec_tag&0xFF) && isprint((par->codec_tag>>8)&0xFF)
                && isprint((par->codec_tag>>16)&0xFF) &&
                isprint((par->codec_tag>>24)&0xFF)) {
            snprintf(buf, sizeof(buf), N_("%c%c%c%c / 0x%04X"),
                     par->codec_tag & 0xff,
                     (par->codec_tag >> 8) & 0xff,
                     (par->codec_tag >> 16) & 0xff,
                     (par->codec_tag >> 24) & 0xff,
                     par->codec_tag);
        } else {
            snprintf(buf, sizeof(buf), N_("0x%04x"), par->codec_tag);
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
 * @param ctx   codec context
 *
 * @return      name of codec.
 */
std::string CMedia::codec_name( const AVCodecContext* ctx )
{
    const AVCodec* p = avcodec_find_decoder(ctx->codec_id);
    const char* codec_name;
    char buf[20];

    if (p) {
        codec_name = p->name;
    } else if (ctx->codec_id == AV_CODEC_ID_MPEG2TS) {
        /* fake mpeg2 transport stream codec (currently not
           registered) */
        codec_name = N_("mpeg2ts");
    } else {
        /* output avi tags */
        if(   isprint(ctx->codec_tag&0xFF) && isprint((ctx->codec_tag>>8)&0xFF)
                && isprint((ctx->codec_tag>>16)&0xFF) &&
                isprint((ctx->codec_tag>>24)&0xFF)) {
            snprintf(buf, sizeof(buf), N_("%c%c%c%c / 0x%04X"),
                     ctx->codec_tag & 0xff,
                     (ctx->codec_tag >> 8) & 0xff,
                     (ctx->codec_tag >> 16) & 0xff,
                     (ctx->codec_tag >> 24) & 0xff,
                     ctx->codec_tag);
        } else {
            snprintf(buf, sizeof(buf), N_("0x%04x"), ctx->codec_tag);
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
double CMedia::calculate_fps( AVFormatContext* ctx,
                              AVStream* stream )
{
    double fps;
    assert( ctx != NULL );
    assert( stream != NULL );
    AVRational rate = av_guess_frame_rate( ctx, stream, NULL );
    if ( rate.num && rate.den )
    {
        fps = av_q2d( rate );
    }
    else
    {
        fps = 1.0 / av_q2d( stream->time_base );
    }
    if ( fps > 1000 &&
         stream->time_base.den > 10000 && stream->time_base.num < 3 )
    {
        fps = stream->time_base.den / 1000.0;
    }

    if ( fps >= 1000 ) fps = 30.0;  // workaround for some buggy streams
    else if ( fps <= 0 ) fps = 30.0;

    return fps;
}



/**
 * Populate the stream information from a codec context
 *
 * @param s              stream info
 * @param msg            any error message
 * @param context        format context
 * @param par            codec parameters
 * @param stream_index   ffmpeg stream index
 */
void CMedia::populate_stream_info( StreamInfo& s,
                                   std::ostringstream& msg,
                                   const AVFormatContext* context,
                                   const AVCodecParameters* par,
                                   const int stream_index )
{

    bool has_codec = true;

    // Mark streams that we don't have a decoder for
    const AVCodec* codec = avcodec_find_decoder( par->codec_id );
    if ( ! codec )
    {
        has_codec = false;
        const char* type = stream_type( par );
        msg << _("\n\nNot a known codec ") << codec_name(par)
            << _(" for stream #") << stream_index << _(", type ") << type;
    }

    s.context      = context;
    s.stream_index = stream_index;
    s.has_codec    = has_codec;
    s.codec_name   = codec_name( par );
    s.fourcc       = codec_tag2fourcc( par->codec_tag );
    s.duration     = 100;

    AVStream* st = context->streams[stream_index];
    double time  = av_q2d( st->time_base );


    AVDictionaryEntry* lang = av_dict_get(st->metadata, "language", NULL, 0);
    if ( lang && lang->value )
        s.language = lang->value;
    else
        s.language = _("und");

    if ( st->start_time == AV_NOPTS_VALUE )
    {
        if ( context->start_time != AV_NOPTS_VALUE )
            s.start = ((double) context->start_time / ( double )AV_TIME_BASE );
        else
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
        if ( context->duration != AV_NOPTS_VALUE )
            s.duration = ((double) context->duration / ( double )AV_TIME_BASE );
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
unsigned CMedia::max_video_frames()
{
    if ( _video_cache_size > 0 )
        return _video_cache_size;
    else if ( _video_cache_size == 0 )
    {
        return unsigned( fps()*2 );
    }
    else
        return std::numeric_limits<unsigned>::max() / 3;
}


// Return the number of frames cached for jog/shuttle
uint64_t CMedia::max_image_frames()
{
#if 0
    if ( _image_cache_size > 0 )
        return _image_cache_size;
    else if ( _image_cache_size == 0 )
        return int( fps()*2 );
    else
        return std::numeric_limits<int>::max() / 3;
#else
    if ( _hires )
    {
        return (uint64_t)(Preferences::max_memory /
                          (double) _hires->data_size());
    }
    uint64_t i = 0;
    uint64_t num = _frame_end - _frame_start + 1;
    for ( ; i < num; ++i )
    {
        if ( _sequence[i] ) break;
    }

    if ( i >= num ) return std::numeric_limits<int>::max() / 3;
    MEM();

    return (uint64_t) (Preferences::max_memory /
                       (double) _sequence[i]->data_size());
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

#if 1
        mrv::PacketQueue::Mutex& m = _video_packets.mutex();
        SCOPED_LOCK( m );

        AVStream* stream = get_video_stream();

        int64_t limit = frame + _frame_offset;

        mrv::PacketQueue::iterator i = _video_packets.begin();
        for ( ; i != _video_packets.end(); )
        {
            int64_t pktframe;
            if ( _video_packets.is_loop_end(*i) ) pktframe = (*i).dts;
            else pktframe = get_frame( stream, *i );
            if ( pktframe > limit )
            {
                i = _video_packets.erase( i );
            }
            else
            {
                ++i;
            }
        }
#endif

        _video_packets.loop_at_end( frame );
    }

    if ( number_of_audio_streams() > 0 )
    {
        // With loop at end, we can discard all audio packets that go
        // beyond the last frame

        mrv::PacketQueue::Mutex& m = _audio_packets.mutex();
        SCOPED_LOCK( m );

        AVStream* stream = get_audio_stream();

        if ( stream )
        {
            mrv::PacketQueue::iterator i = _audio_packets.begin();
            int64_t limit = frame + _frame_offset + _audio_offset - 1;
            for ( ; i != _audio_packets.end(); )
            {
                int64_t pktframe = get_frame( stream, *i );
                if ( pktframe > limit )
                {
                    i = _audio_packets.erase( i );
                }
                else
                    ++i;
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

    uint64_t max_frames = 1; //max_image_frames();

#undef timercmp
# define timercmp(a, b, CMP)                                                  \
  (((a).tv_sec == (b).tv_sec) ?					\
   ((a).tv_usec CMP (b).tv_usec) :                                          \
   ((a).tv_sec CMP (b).tv_sec))

    struct customMore {
        inline bool operator()( const timeval& a,
                                const timeval& b ) const
        {
            return timercmp( a, b, < );
        }
    };


    uint64_t num  = _frame_end - _frame_start + 1;

    typedef std::map< timeval, uint64_t, customMore > TimedSeqMap;

    TimedSeqMap tmp;
    for ( uint64_t i = 0; i < num; ++i )
    {
        if ( !_sequence[i] || _sequence[i]->data_size() == 0 )
            continue;

        tmp.insert( std::make_pair( _sequence[i]->ptime(), i ) );
    }


    uint64_t image_count = tmp.size();
    TimedSeqMap::iterator it = tmp.begin();

    // std::cerr << "mem used: " << (memory_used / 1024 / 1024) << " max mem "
    //           << (Preferences::max_memory / 1024 / 1024) << std::endl;
    // std::cerr << "image_count: " << image_count << " max frames "
    //           << max_frames << std::endl;

    // Erase enough frames to make sure memory used is less than max memory
    for ( ; it != tmp.end() && memory_used >= Preferences::max_memory; ++it )
    {
        uint64_t idx = it->second;

        if ( image_count <= max_frames ) break;

        if ( _sequence[idx] )
        {
            std::string file = sequence_filename( _sequence[idx]->frame() );
            struct stat sbuf;
            int result = stat( file.c_str(), &sbuf );
            if ( result == 0 ) {
                _disk_space -= sbuf.st_size;
            }
            _sequence[ idx ].reset();
            --image_count;
        }

        if ( _right && _right[idx] ) {
            std::string file = sequence_filename( _right[idx]->frame() );
            struct stat sbuf;
            int result = stat( file.c_str(), &sbuf );
            if ( result == 0 ) {
                _disk_space -= sbuf.st_size;
            }
            _right[ idx ].reset();
        }
    }

#ifdef LINUX
    malloc_trim(0);
#endif

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
              << " A:" << frame << " " << routine;

    if (detail && _sequence )
    {
        std::cerr << " image stores: " << std::endl;
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
    else
    {
        std::cerr << std::endl;
    }
}

CMedia::DecodeStatus CMedia::decode_video( int64_t& frame )
{
    if ( stopped() && _right_eye && _owns_right_eye && _stereo_output ) {
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

            // This is needed to handle loops properly, as packets do not
            // come for all frames, as counted in the playback loop.
            if ( frame > pkt.dts )
            {
                if ( ! is_cache_filled( frame ) )
                    refetch(frame);
                return kDecodeOK;
            }

            _video_packets.pop_front();
            return kDecodeLoopStart;
        }
        else if ( _video_packets.is_loop_end() )
        {
            // We check packet integrity as the length of packets is
            // not accurate.

            const AVPacket& pkt = _video_packets.front();

            // This is needed to handle loops properly, as packets do not
            // come for all frames, as counted in the playback loop.
            if ( frame < pkt.dts )
            {
                if ( ! is_cache_filled( frame ) )
                    refetch(frame);
                return kDecodeOK;
            }

            _video_packets.pop_front();
            return kDecodeLoopEnd;
        }
        else
        {
            assert( !_video_packets.empty() );
            if ( ! is_cache_filled( frame ) )
            {
                refetch(frame);
            }
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



bool CMedia::find_image( const int64_t frame )
{
    if ( stopped() && _right_eye && _owns_right_eye && _stereo_output )
        _right_eye->find_image(frame);


    int64_t f = frame;

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

    // We want to run limit_video_store only once.
    // However, both the video_thread and the preload idle thread
    // might call us on the same frame, leading to shortening of
    // too many frames.  Thus, we use this variable to limit or not
    // the cache.
    bool limit = false;

    if ( _sequence && _sequence[idx] && _sequence[idx]->valid() )
    {
        SCOPED_LOCK( _mutex );

        if ( _frame != frame )
        {
            _frame = frame;
            limit = true;
        }

        if ( _right && _right[idx])
            _stereo[1] = _right[idx];

        av_free(_filename);
        _filename = NULL;

        refresh();
        image_damage( image_damage() | kDamageData | kDamage3DData );
        if ( limit ) limit_video_store( f );
        return true;
    }


    bool should_load = false;

    std::string file = sequence_filename(f);
    std::string old  = sequence_filename(_frame);

    if ( !internal() && file != old  )
    {
        should_load = true;
        av_free( _filename );
        _filename = av_strdup( file.c_str() );
    }

    if ( _frame != f )
    {
        _frame = f;
        limit = true;
    }

    if ( should_load )
    {
        image_type_ptr canvas;
        if ( fs::exists(file) )
        {
            SCOPED_LOCK( _mutex );
            SCOPED_LOCK( _audio_mutex );
            SCOPED_LOCK( _subtitle_mutex );
            timeval now;
            gettimeofday (&now, 0);
            _lastFrameTime = now;
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
                    timeval now;
                    gettimeofday (&now, 0);
                    _lastFrameTime = now;
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
                refresh();
                // if ( limit ) limit_video_store( f );
                // image_damage( image_damage() | kDamageData | kDamage3DData );
                return true;
            }
        }
    }

    if ( limit ) limit_video_store( f );

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


    std::string n = filename();
    OCIO::ConstConfigRcPtr config = Preferences::OCIOConfig();

    if ( ! Preferences::use_ocio ) return;

    if ( config )
    {
        if ( n.rfind( ".braw" ) == std::string::npos )
        {
            std::string cs = config->parseColorSpaceFromString(n.c_str());
            if ( !cs.empty() )
            {
                IMG_INFO( _("Got colorspace '") << cs << _("' from filename") );
                ocio_input_color_space( cs );
                return;
            }
        }
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
    if ( config && ( n.rfind( ".dpx" ) != std::string::npos ||
                     n.rfind( ".cin")  != std::string::npos ) )
    {
        OCIO::ConstColorSpaceRcPtr defaultcs = config->getColorSpace("Cineon");
        if ( ! defaultcs )
            defaultcs = config->getColorSpace("lgf");
        if ( ! defaultcs )
            defaultcs = config->getColorSpace("lg10");
        if ( defaultcs )
        {
            if ( !_is_thumbnail )
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
    image_damage( image_damage() | kDamageData | kDamageLut | kDamageICS );
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
}

// Outputs the indices of the stream
void CMedia::debug_stream_index( const AVStream* stream )
{
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
              << " " << routine << " frame offset: " << _frame_offset
              << " video packets #"
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
            std::cerr << (*iter).dts - _frame_offset;
        }
        else
        {
            if ( stream )
                std::cerr << get_frame( stream, *iter ) - _frame_offset;
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
                std::cerr << get_frame( stream, *(last-1) ) - _frame_offset;
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

const char* const get_error_text(const int error)
{
    static char error_buffer[255];
    if ( error < 0 )
    {
        av_strerror(error, error_buffer, sizeof(error_buffer));
        return error_buffer;
    }
    else
    {
        return _( kDecodeStatus[error] );
    }
}

bool CMedia::refetch( int64_t f )
{
    timeval now;
    gettimeofday (&now, 0);
    _lastFrameTime = now;

    image_type_ptr canvas;
    bool ok = fetch( canvas, f );
    if ( ok )
    {
        cache( canvas );
        default_color_corrections();
        find_image( f );
    }
    return ok;
}

bool CMedia::refetch()
{
    return refetch( _dts );
}

} // namespace mrv
