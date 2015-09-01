
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

#include <fltk/run.h>

#undef  __STDC_CONSTANT_MACROS
#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/diagnostic_information.hpp>
namespace fs = boost::filesystem;


#include "core/CMedia.h"
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

namespace {

  const char* kModule = "img";

}

#define IMG_WARNING(x) LOG_WARNING( name() << " - " << x ) 
#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )

// #define DEBUG_SEEK
// #define DEBUG_VIDEO_PACKETS
//#define DEBUG_STORES

// #define DEBUG_DECODE
// #define DEBUG_AUDIO_SPLIT




namespace mrv {

static AVRational timeBaseQ = { 1, AV_TIME_BASE };

unsigned    CMedia::_audio_max = 0;
bool        CMedia::_supports_yuv = false;

std::string CMedia::rendering_transform_8bits;
std::string CMedia::rendering_transform_16bits;
std::string CMedia::rendering_transform_32bits;
std::string CMedia::rendering_transform_float;


std::string CMedia::icc_profile_8bits;
std::string CMedia::icc_profile_16bits;
std::string CMedia::icc_profile_32bits;
std::string CMedia::icc_profile_float;

unsigned CMedia::_audio_cache_size = 0;
unsigned CMedia::_video_cache_size = 0;

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

/** 
 * Constructor
 * 
 */
CMedia::CMedia() :
  _w( 0 ),
  _h( 0 ),
  _internal( false ),
  _is_sequence( false ),
  _is_stereo( false ),
  _stereo_type( kNoStereo ),
  _fileroot( NULL ),
  _filename( NULL ),
  _ctime( 0 ),
  _mtime( 0 ),
  _disk_space( 0 ),
  _loop_barrier( NULL ),
  _avdiff( 0.0 ),
  _seek_req( false ),
  _seek_frame( 1 ),
  _channel( NULL ),
  _label( NULL ),
  _real_fps( 0 ),
  _play_fps( 0 ),
  _fps( 0 ),
  _pixel_ratio( 1.0f ),
  _num_channels( 0 ),
  _rendering_intent( kUndefinedIntent ),
  _gamma( 1 ),
  _has_chromaticities( false ),
  _dts( 1 ),
  _audio_frame( 1 ),
  _audio_offset( 0 ),
  _frame( 1 ),
  _expected( 1 ),
  _expected_audio( 1 ),
  _frameStart( 1 ),
  _frameEnd( 1 ),
  _interlaced( kNoInterlace ),
  _image_damage( kNoDamage ),
  _damageRectangle( 0, 0, 0, 0 ),
  _numWindows( 0 ),
  _dataWindow( NULL ),
  _displayWindow( NULL ),
  _dataWindow2( NULL ),
  _displayWindow2( NULL ),
  _left_eye( true ),
  _profile( NULL ),
  _rendering_transform( NULL ),
  _idt_transform( NULL ),
  _playback( kStopped ),
  _aborted( false ),
  _sequence( NULL ),
  _right( NULL ),
  _audio_pts( 0 ),
  _audio_clock( double( av_gettime_relative() )/ 1000000.0 ),
  _video_pts( 0 ),
  _video_clock( double( av_gettime_relative() ) / 1000000.0 ),
  _context(NULL),
  _video_ctx( NULL ),
  _audio_ctx( NULL ),
  _acontext(NULL),
  _audio_codec(NULL),
  _subtitle_index(-1),
  _audio_index(-1),
  _samples_per_sec( 0 ),
  _audio_buf_used( 0 ),
  _audio_last_frame( 0 ),
  _audio_channels( 0 ),
  _audio_format( AudioEngine::kFloatLSB ),
  _audio_buf( NULL ),
  forw_ctx( NULL ),
  _audio_engine( NULL )
{
  audio_initialize();
  _eye[0] = _eye[1] = NULL;
  mrv::PacketQueue::initialize();
}


/** 
 * Copy Constructor.  Copies an image, optionally resizing it.
 * 
 * @param other original image
 * @param ws    new image width
 * @param wh    new image height
 */
CMedia::CMedia( const CMedia* other, int ws, int wh ) :
  _w( 0 ),
  _h( 0 ),
  _is_stereo( false ),
  _stereo_type( kNoStereo ),
  _is_sequence( false ),
  _fileroot( NULL ),
  _filename( NULL ),
  _ctime( 0 ),
  _mtime( 0 ),
  _disk_space( 0 ),
  _loop_barrier( NULL ),
  _seek_req( false ),
  _seek_frame( 1 ),
  _channel( NULL ),
  _label( NULL ),
  _real_fps( 0 ),
  _play_fps( 24.0 ),
  _fps( 0 ),
  _pixel_ratio( 1.0f ),
  _num_channels( 0 ),
  _rendering_intent( kUndefinedIntent ),
  _gamma( 1 ),
  _has_chromaticities( false ),
  _dts( 1 ),
  _audio_frame( 1 ),
  _audio_offset( 0 ),
  _frame( 1 ),
  _expected( 1 ),
  _expected_audio( 1 ),
  _frameStart( 1 ),
  _frameEnd( 1 ),
  _frame_start( 1 ),
  _frame_end( 1 ),
  _interlaced( kNoInterlace ),
  _image_damage( kNoDamage ),
  _damageRectangle( 0, 0, 0, 0 ),
  _numWindows( 0 ),
  _dataWindow( NULL ),
  _displayWindow( NULL ),
  _dataWindow2( NULL ),
  _displayWindow2( NULL ),
  _left_eye( true ),
  _profile( NULL ),
  _rendering_transform( NULL ),
  _idt_transform( NULL ),
  _playback( kStopped ),
  _aborted( false ),
  _sequence( NULL ),
  _right( NULL ),
  _video_ctx( NULL ),
  _audio_ctx( NULL ),
  _context(NULL),
  _acontext(NULL),
  _audio_codec(NULL),
  _subtitle_index(-1),
  _audio_index(-1),
  _samples_per_sec( 0 ),
  _audio_buf_used( 0 ),
  _audio_last_frame( 0 ),
  _audio_channels( other->_audio_channels ),
  _audio_buf( NULL ),
  forw_ctx( NULL ),
  _audio_engine( NULL )
{
  _eye[0] = _eye[1] = NULL;
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

CMedia::CMedia( const CMedia* other, boost::int64_t f ) :
  _w( 0 ),
  _h( 0 ),
  _is_stereo( other->_is_stereo ),
  _stereo_type( other->_stereo_type ),
  _is_sequence( other->_is_sequence ),
  _fileroot( NULL ),
  _filename( NULL ),
  _ctime( other->_ctime ),
  _mtime( other->_mtime ),
  _disk_space( 0 ),
  _loop_barrier( NULL ),
  _seek_req( false ),
  _seek_frame( 1 ),
  _channel( NULL ),
  _label( NULL ),
  _real_fps( 0 ),
  _play_fps( 24.0 ),
  _fps( other->_fps ),
  _pixel_ratio( other->_pixel_ratio ),
  _num_channels( other->_num_channels ),
  _rendering_intent( other->_rendering_intent ),
  _gamma( other->_gamma ),
  _has_chromaticities( other->has_chromaticities() ),
  _chromaticities( other->chromaticities() ),
  _dts( other->_dts ),
  _audio_frame( 0 ),
  _audio_offset( 0 ),
  _frame( other->_frame ),
  _expected( _frame ),
  _expected_audio( 0 ),
  _frameStart( other->_frameStart ),
  _frameEnd( other->_frameEnd ),
  _frame_start( other->_frame_start ),
  _frame_end( other->_frame_end ),
  _interlaced( other->_interlaced ),
  _image_damage( kNoDamage ),
  _damageRectangle( 0, 0, 0, 0 ),
  _numWindows( 0 ),
  _dataWindow( NULL ),
  _displayWindow( NULL ),
  _dataWindow2( NULL ),
  _displayWindow2( NULL ),
_left_eye( false ),
_profile( NULL ),
_rendering_transform( NULL ),
_idt_transform( NULL ),
_playback( kStopped ),
_aborted( false ),
_sequence( NULL ),
_right( NULL ),
_video_ctx( NULL ),
_audio_ctx( NULL ),
_context(NULL),
_acontext(NULL),
_audio_codec(NULL),
_subtitle_index(-1),
_audio_index(-1),
_samples_per_sec( 0 ),
_audio_buf_used( 0 ),
_audio_last_frame( 0 ),
_audio_channels( 0 ),
_audio_buf( NULL ),
forw_ctx( NULL ),
_audio_engine( NULL )
{
  _eye[0] = _eye[1] = NULL;

  _fileroot = strdup( other->fileroot() );
  _filename = strdup( other->filename() );

  fetch( f );
  cache( hires() );
}

boost::int64_t CMedia::get_frame( const AVStream* stream, const AVPacket& pkt )
{
   assert( stream != NULL );
   boost::int64_t frame = 0;
   if ( pkt.pts != MRV_NOPTS_VALUE )
   {
      frame = pts2frame( stream, pkt.pts ); 
   }
   else
   {
       if ( pkt.dts != MRV_NOPTS_VALUE )
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
  if ( _sequence == NULL ) return;

  SCOPED_LOCK( _mutex);

  boost::uint64_t num = _frame_end - _frame_start + 1;
  for ( boost::uint64_t i = 0; i < num; ++i )
    {
        if ( _sequence[i] )
            _sequence[i].reset();
        if ( _right[i] )
            _right[i].reset();
    }

}

/** 
 * Wait for all threads to finish and exit.  Delete them afterwards.
 * 
 */
void CMedia::wait_for_load_threads()
{
  // Wait for all threads to exit
  thread_pool_t::iterator i = _load_threads.begin();
  thread_pool_t::iterator e = _load_threads.end();
  for ( ;i != e; ++i )
    {
      (*i)->join();
      delete *i;
    }

  _load_threads.clear();
}

/** 
 * Wait for all threads to finish and exit.  Delete them afterwards.
 * 
 */
void CMedia::wait_for_threads()
{
  // Wait for all threads to exit
  thread_pool_t::iterator i = _threads.begin();
  thread_pool_t::iterator e = _threads.end();
  for ( ;i != e; ++i )
    {
        (*i)->join();
        delete *i;
    }

  _threads.clear();
}

/** 
 * Destructor
 * 
 */
CMedia::~CMedia()
{
  SCOPED_LOCK( _mutex);
  SCOPED_LOCK( _audio_mutex);

  if ( !stopped() )
    stop();

  image_damage(0);

  free( _channel );
  free( _fileroot );
  free( _filename );
  free( _label );
  free( _profile );
  free( _rendering_transform );

  clear_look_mod_transform();
  free( _idt_transform );


  clear_cache();
  delete [] _sequence;
  delete [] _right;

  if ( has_audio() )
    {
      close_audio();

      delete [] _audio_buf; _audio_buf = NULL;

      close_audio_codec();
    }

  audio_shutdown();

  if ( forw_ctx )
  {
     swr_free( &forw_ctx );
     forw_ctx = NULL;
  }

  if ( _context )
    avformat_close_input( &_context );

  if ( _acontext )
    avformat_close_input( &_acontext );

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
 
/** 
 * Allocate the float pixels for the image
 * 
 * @param ws image width in pixels
 * @param hs image height in pixels
 */
void CMedia::allocate_pixels( const boost::int64_t& frame,
                              const unsigned short channels,
                              const image_type::Format format,
                              const image_type::PixelType pixel_type )
{
  SCOPED_LOCK( _mutex );
  DBG( "allocate pixels frame: " << frame 
       << " channels: " << channels << " format: "
       << format << " pixel type: " << pixel_type );
  _hires.reset( new image_type( frame, width(), height(), 
				channels, format, pixel_type ) );
}


mrv::image_type_ptr CMedia::left() const
{
   boost::uint64_t idx = _frame - _frame_start;
   if ( _eye[0] && _eye[0] != this )
       return _eye[0]->left();
   else if ( _is_sequence && _sequence[idx] )
      return _sequence[idx];
   else
       if ( _stereo[0] )
           return _stereo[0];
       else
           return _hires;
}

mrv::image_type_ptr CMedia::right() const
{
   boost::uint64_t idx = _frame - _frame_start;
   if ( _eye[1] ) {
       return _eye[1]->left();
   }
   if ( _is_sequence && _right[idx] )
      return _right[idx];
   else
      return _stereo[1];
}

mrv::image_type_ptr CMedia::anaglyph( bool left_view ) 
{
   image_type::Pixel* p = (image_type::Pixel*)hires()->data().get();
   image_type::Pixel* l = (image_type::Pixel*)left()->data().get();
   image_type::Pixel* r = (image_type::Pixel*)right()->data().get();
   for (unsigned y = 0; y < height(); ++y )
   {
      for ( unsigned x = 0; x < width(); ++x )
      {
	 if ( left_view )
	 {
	    p[x + y*width() ].r = l[x + y*width() ].r;
	    p[x + y*width() ].g = r[x + y*width() ].g;
	    p[x + y*width() ].b = r[x + y*width() ].b;
	 }
	 else
	 {
	    p[x + y*width() ].r = r[x + y*width() ].r;
	    p[x + y*width() ].g = l[x + y*width() ].g;
	    p[x + y*width() ].b = l[x + y*width() ].b;
	 }
     }
   }
   return hires();
}



static mrv::Recti kNoRect = mrv::Recti(0,0,0,0);



const mrv::Recti& CMedia::display_window( boost::int64_t f ) const
{
    if ( !_displayWindow || _numWindows == 0 ) return kNoRect;

    if ( f == AV_NOPTS_VALUE ) f = _frame;
    boost::int64_t idx = f - _frame_start;
    if ( idx >= (int64_t)_numWindows ) idx = _numWindows-1;
    else if ( idx < 0 ) idx = 0;

    assert( idx <= _frame_end - _frame_start );
    return _displayWindow[idx];
}

const mrv::Recti& CMedia::display_window2( boost::int64_t f ) const
{
    if ( _eye[1] )
        return _eye[1]->display_window(f);

    if ( !_displayWindow2 || _numWindows == 0 ) return kNoRect;

    if ( f == AV_NOPTS_VALUE ) f = _frame;
    boost::int64_t idx = f - _frame_start;
    if ( idx >= (int64_t)_numWindows ) idx = _numWindows-1;
    else if ( idx < 0 ) idx = 0;

    assert( idx <= _frame_end - _frame_start );
    return _displayWindow2[idx];
}

const mrv::Recti& CMedia::data_window( boost::int64_t f ) const
{
    if ( !_dataWindow || _numWindows == 0 ) return kNoRect;

    if ( f == AV_NOPTS_VALUE ) f = _frame;
    boost::uint64_t idx = f - _frame_start;
    if ( idx >= _numWindows ) idx = _numWindows-1;
    else if ( idx < 0 ) idx = 0;

    return _dataWindow[idx];
}

const mrv::Recti& CMedia::data_window2( boost::int64_t f ) const
{
    if ( _eye[1] )
        return _eye[1]->data_window(f);

    if ( !_dataWindow2 || _numWindows == 0 ) return kNoRect;

    if ( f == AV_NOPTS_VALUE ) f = _frame;
    boost::uint64_t idx = f - _frame_start;
    if ( idx >= _numWindows ) idx = _numWindows-1;
    else if ( idx < 0 ) idx = 0;

    return _dataWindow2[idx];
}

void CMedia::display_window( const int xmin, const int ymin,
			     const int xmax, const int ymax )
{
  assert( xmax >= xmin );
  assert( ymax >= ymin );
  _numWindows = _frame_end - _frame_start + 1;
  if ( !_displayWindow )
      _displayWindow = new mrv::Recti[_numWindows];
  boost::uint64_t idx = _frame - _frame_start;
  assert( idx <= _frame_end - _frame_start );
  _displayWindow[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
  image_damage( image_damage() | kDamageData );
  //DBG( "display window frame " << _frame << " is " << _displayWindow[idx] );
}

void CMedia::display_window2( const int xmin, const int ymin,
                              const int xmax, const int ymax )
{
  assert( xmax >= xmin );
  assert( ymax >= ymin );
  _numWindows = _frame_end - _frame_start + 1;
  if ( !_displayWindow2 )
      _displayWindow2 = new mrv::Recti[_numWindows];
  boost::uint64_t idx = _frame - _frame_start;
  assert( idx <= _frame_end - _frame_start );
  _displayWindow2[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
  image_damage( image_damage() | kDamageData );
  DBG( "display window2 frame " << _frame << " is " << _displayWindow2[idx] );
}

void CMedia::data_window( const int xmin, const int ymin,
                          const int xmax, const int ymax )
{
  assert( xmax >= xmin );
  assert( ymax >= ymin );
  _numWindows = _frame_end - _frame_start + 1;
  if ( !_dataWindow )
      _dataWindow = new mrv::Recti[_numWindows];
  boost::uint64_t idx = _frame - _frame_start;
  assert( idx <= _frame_end - _frame_start );
  _dataWindow[idx] = mrv::Recti( xmin, ymin, xmax-xmin+1, ymax-ymin+1 );
  image_damage( image_damage() | kDamageData );
  // DBG( "data window setframe " << _frame << " is " << _dataWindow[idx] );
}


void CMedia::data_window2( const int xmin, const int ymin,
                          const int xmax, const int ymax )
{
  assert( xmax >= xmin );
  assert( ymax >= ymin );
  _numWindows = _frame_end - _frame_start + 1;
  if ( !_dataWindow2 )
      _dataWindow2 = new mrv::Recti[_numWindows];
  boost::uint64_t idx = _frame - _frame_start;
  assert( idx <= _frame_end - _frame_start );
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



void  CMedia::first_frame(boost::int64_t x)
{
   if ( x < _frame_start ) x = _frame_start;
   _frameStart = x;
}

void  CMedia::last_frame(boost::int64_t x)
{
   if ( x > _frame_end ) x = _frame_end;
   _frameEnd = x;
}

/** 
 * Treat image as a sequence of frames
 * 
 * @param fileroot  a fileroot in C format like image.%d.exr
 * @param start     start frame
 * @param end       end   frame
 */
void CMedia::sequence( const char* fileroot, 
		       const boost::int64_t start,
		       const boost::int64_t end,
                       const bool use_thread )
{

  SCOPED_LOCK( _mutex );

  assert( fileroot != NULL );
  assert( start < end );

  if ( _fileroot && strcmp( fileroot, _fileroot ) == 0 && 
       start == _frame_start && end == _frame_end )
    return;


  _fileroot = strdup( fileroot );

  std::string f = _fileroot;
  int idx = f.find( N_("%V") );
  if ( idx != std::string::npos )
  {
     _is_stereo = true;
  }

  if ( _filename ) {
    free( _filename );
    _filename = NULL;
  }


  _is_sequence = true;
  _dts = _frame = start;
  _frameStart = _frame_start = start;
  _frameEnd = _frame_end = end;

  delete [] _sequence;
  _sequence = NULL;
  delete [] _right;
  _right = NULL;

  boost::uint64_t num = _frame_end - _frame_start + 1;
  _sequence = new mrv::image_type_ptr[ unsigned(num) ];
  _right    = new mrv::image_type_ptr[ unsigned(num) ];

  if ( ! initialize() )
    return;

  fetch( start );
  cache( _hires );

  default_icc_profile();
  default_rendering_transform();

  if ( has_audio() )
    {
      decode_audio( _frame );
    }
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

  if ( _fileroot && strcmp( n, _fileroot ) == 0 )
    return;

  SCOPED_LOCK( _mutex);

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

  if ( _filename ) 
  {
     free( _filename );
     _filename = NULL;
    }

  _is_sequence = false;
  _is_stereo = false;
  _dts = _frame = _frameStart = _frameEnd = _frame_start = _frame_end = 1;


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
  strcpy( self->_filename, file.c_str() );
  return _filename;
}



/** 
 * Create and return the current filename of the image for the 
 * current frame.
 * 
 * @return filename of the file for current frame
 */
std::string CMedia::sequence_filename( const boost::int64_t frame )
{
  if ( !is_sequence() ) return _fileroot;

  std::string tmp = parse_view( _fileroot, _left_eye );

  // For image sequences
  char buf[1024];
  sprintf( buf, tmp.c_str(), frame );

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
      std::string file = sequence_filename(_frame);

      int result = stat( file.c_str(), &sbuf );
      if ( (result == -1) || (_frame < _frame_start) ||
			      ( _frame > _frame_end ) ) return false;

      assert( _frame <= _frame_end );
      assert( _frame >= _frame_start );
      boost::uint64_t idx = _frame - _frame_start;

      if ( !_sequence || !_sequence[idx] ) return false;

      if ( _sequence[idx]->mtime() != sbuf.st_mtime )
	{
	   assert( _frame == _sequence[idx]->frame() );
	   // update frame...
	   _sequence[idx].reset();

	   fetch( _frame );
	   cache( _hires );
	   return true;
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
          _mtime = sbuf.st_mtime;
          _ctime = sbuf.st_ctime;
          fetch( _frame );
          cache( _hires );
          return true;
      }
    }
  return false;
}


/** 
 * Change the image size.
 * This function may also set the pixel ratio for some common video
 * formats.
 * 
 * @param w width of image
 * @param h height of image
 */
void CMedia::image_size( int w, int h )
{
  _damageRectangle = mrv::Recti( 0, 0, w, h );
  _pixel_ratio = 1.0f;


  // Derive pixel ratio from common format resolutions
  if ( w == 720 && h == 486 )
    {
      _pixel_ratio = 0.9f;    // 4:3 NTSC
      if ( _fps == 0 ) _fps = _play_fps = 29.97;
    }
  else if ( w == 640 && h == 480 )
    {
      _pixel_ratio = 1.0f;    // 4:3 NTSC
      if ( _fps == 0 ) _fps = _play_fps = 29.97;
    }
  else if ( w == 720 && h == 480 )
    {
      _pixel_ratio = 0.88888f;    // 4:3 NTSC
      if ( _fps == 0 ) _fps = _play_fps = 29.97;
    }
  else if ( w == 512 && h == 486 )
    {
      _pixel_ratio = 1.265f;  // 4:3 Targa 486
      if ( _fps == 0 ) _fps = _play_fps = 29.97;
    }
  else if ( w == 512 && h == 482 )
    {
      _pixel_ratio = 1.255f;  // 4:3 Targa NTSC
      if ( _fps == 0 ) _fps = _play_fps = 29.97;
    }
  else if ( w == 512 && h == 576 )
    {
      _pixel_ratio = 1.5f;    // 4:3 Targa PAL
      if ( _fps == 0 ) _fps = _play_fps = 25;
    }
  else if ( w == 646 && h == 485 )
    {
      _pixel_ratio = 1.001f;  // 4:3 NTSC
      if ( _fps == 0 ) _fps = _play_fps = 29.97;
    }
  else if ( w == 720 && h == 576 )
    {
      _pixel_ratio = 1.066f;  // 4:3 PAL
      if ( _fps == 0 ) _fps = _play_fps = 25;
    }
  else if ( w == 780 && h == 576 )
    {
      _pixel_ratio = 0.984f;  // 4:3 PAL
      if ( _fps == 0 ) _fps = _play_fps = 25;
    }
  else if ( w == 1280 && h == 1024 )
    {
      _pixel_ratio = 1.066f; // HDTV full
      if ( _fps == 0 ) _fps = _play_fps = 29.97;
    }
  else if ( (float)w/(float)h == 1.56 )
    {
      _pixel_ratio = 0.9f;   // HTDV/SD compromise
      if ( _fps == 0 ) _fps = _play_fps = 29.97;
    }

  if ( _fps == 0 )
    {
      _fps = _play_fps = 24; 
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
    if ( _hires ) return _hires->pixel_type();
    else return image_type::kByte;
}

void CMedia::gamma( const float x )
{
    _gamma = x;
    refresh();
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
    SCOPED_LOCK( _mutex );
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
    SCOPED_LOCK( _mutex );
    _layers.push_back( _("Color") );
    _layers.push_back( _("Red") );
    _layers.push_back( _("Green") );
    _layers.push_back( _("Blue") );
    _num_channels += 3;
    image_damage( image_damage() | kDamageLayers | kDamageData );
}

/** 
 * Add r,g,b and lumma to list of layers
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
    SCOPED_LOCK( _mutex );
    _layers.clear();
    _num_channels = 0;
    rgb_layers();
    lumma_layers();
    alpha_layers();
}

/** 
 * Add image's stereo layers
 * 
 */
void CMedia::add_anaglyph_layers()
{
    _layers.push_back( "left.anaglyph" );
    _layers.push_back( "right.anaglyph" );
}

/** 
 * Add image's stereo layers
 * 
 */
void CMedia::add_stereo_layers()
{
    _layers.push_back( "left.anaglyph" );
    _layers.push_back( "right.anaglyph" );
    _layers.push_back( "stereo.horizontal" );
    _layers.push_back( "stereo.crossed" );
}

/** 
 * Change the image's color channel
 * 
 * @param c new image channel
 */
void CMedia::channel( const char* c )
{

  std::string ch( c );

  if ( ch == _("Color") || ch == _("Red") || ch == _("Green") || 
       ch == _("Blue")  ||
       ch == _("Alpha") || ch == _("Alpha Overlay") || ch == _("Lumma") )
      c = NULL;

  bool to_fetch = false;

  if ( _channel != c )
    {
      if ( _channel == NULL || c == NULL )  to_fetch = true;
      else if (strcmp( c, _channel ) != 0 ) to_fetch = true;
    }

  free( _channel );


  if ( c )
    {
      _channel = strdup( c );
    }
  else
    {
      _channel = NULL;
    }

  if (to_fetch) 
    {
       clear_cache();
       SCOPED_LOCK( _mutex );
       fetch(_frame);
    }
  refresh();
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
void CMedia::timestamp(const boost::uint64_t idx )
{
  if ( !_sequence ) return;

  mrv::image_type_ptr pic = _sequence[idx];

  struct stat sbuf;
  int result = stat( sequence_filename( pic->frame() ).c_str(), &sbuf );
  if ( result < 0 ) return;

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
  std::string date( ::ctime( &_ctime ) );
  date = date.substr( 0, date.length() - 1 );
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
  free( _rendering_transform ); _rendering_transform = NULL;
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
  free( _idt_transform ); _idt_transform = NULL;
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
  std::vector< char* >::iterator i = _look_mod_transform.begin();
  std::vector< char* >::iterator e = _look_mod_transform.end();
  for ( ; i != e; ++i )
  {
      free( *i );
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

        for ( size_t j = 0; j < num - 4; ++j )
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
  free( _profile ); _profile = NULL;
  if ( cfile && strlen(cfile) > 0 )
    {
      mrv::colorProfile::add( cfile );
      _profile = strdup( cfile );
    }
  image_damage( image_damage() | kDamageData | kDamageLut );
  refresh();
}


/** 
 * Force exit and delete all threads
 * 
 */
void CMedia::thread_exit()
{

  thread_pool_t::iterator i = _threads.begin();
  thread_pool_t::iterator e = _threads.end();
  for ( ; i != e; ++i )
    {
      delete *i;
    }

  _threads.clear();
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
      if ( _audio_info[i].has_codec ) { valid = true; break; }
    }

  return valid;
}

bool CMedia::valid_subtitle() const
{
  size_t num_streams = number_of_subtitle_streams();

  bool valid = false;
  for ( size_t i = 0; i < num_streams; ++i )
    {
      if ( _subtitle_info[i].has_codec ) { valid = true; break; }
    }

  return valid;
}

/// VCR play (and record if needed) sequence
void CMedia::play(const CMedia::Playback dir, 
		  mrv::ViewerUI* const uiMain,
		  bool fg )
{

    // if ( _playback == kStopped && !_threads.empty() )
    //     return;

    stop();

    _playback = dir;
    _aborted = false;

    assert( uiMain != NULL );
    assert( _threads.size() == 0 );

    if ( _frame < first_frame() ) _frame = first_frame();
    if ( _frame > last_frame() )  _frame = last_frame();

    _audio_frame = _frame;
  // _expected = std::numeric_limits< boost::int64_t >::min();

  _dts = _frame;

  DBG( name() << " Play from frame " << _dts );

  _audio_clock = double( av_gettime_relative() ) / 1000000.0;
  _video_clock = double( av_gettime_relative() ) / 1000000.0;

  _audio_buf_used = 0;


  // clear all packets
  clear_packets();
  clear_stores();

  // This seek is needed to sync audio playback and flush buffers
  if ( dir == kForwards ) _seek_req = true;

  if ( ! seek_to_position( _frame ) )
      IMG_ERROR( _("Could not seek to frame ") << _frame );

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

      delete _loop_barrier;
      _loop_barrier = new Barrier( 1 + valid_a + valid_v + valid_s );

      if ( valid_v )
      {
          video_data = new PlaybackData( *data );
          _threads.push_back( new boost::thread(
                              boost::bind( mrv::video_thread, 
                                           video_data ) ) );
      }

      if ( valid_a )
      {
          // Audio playback thread
          audio_data = new PlaybackData( *data );
          _threads.push_back( new boost::thread(
                              boost::bind( mrv::audio_thread,
                                           audio_data ) ) );
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
          _threads.push_back( new boost::thread( 
                              boost::bind( mrv::decode_thread, 
                                           data ) ) );
      }

      assert( _threads.size() <= ( 1 + valid_a + valid_v + valid_s ) );
  }
  catch( boost::exception& e )
  {
      LOG_ERROR( boost::diagnostic_information(e) );
  }


}


/// VCR stop sequence
void CMedia::stop()
{

    if ( _playback == kStopped && _threads.empty() ) return;


  _playback = kStopped;

  //
  // Notify loop barrier, to exit any wait on a loop
  //
  DBG( name() << " Notify all loop barrier" );
  if ( _loop_barrier ) {
      _loop_barrier->notify_all();
  }

  // Notify packets, to make sure that audio thread exits any wait lock
  // This needs to be done even if no audio is playing, as user might
  // have turned off audio, but audio thread is still active.
  DBG( name() << " Notify all A/V/S" );
  _audio_packets.cond().notify_all();
  _video_packets.cond().notify_all();
  _subtitle_packets.cond().notify_all();


  // Wait for all threads to exit
  DBG( name() << " Wait for threads" );
  wait_for_threads();
  DBG( name() << " Waited for threads" );

  // Clear barrier
  DBG( name() << " Clear barrier" );
  delete _loop_barrier; _loop_barrier = NULL;

  DBG( name() << " Clear packets" );
  // Clear any audio/video/subtitle packets
  clear_packets();


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
bool CMedia::frame( const boost::int64_t f )
{
  assert( _fileroot != NULL );

  if ( _eye[0] && _eye[1] )
  {
      bool l = _eye[0]->frame(f);
      bool r = _eye[1]->frame(f);
      return ( l && r );
  }


//  in ffmpeg, sizes are in bytes...
#define MAX_VIDEOQ_SIZE (5 * 2048 * 1024)
#define MAX_AUDIOQ_SIZE (5 * 60 * 1024)
#define MAX_SUBTITLEQ_SIZE (5 * 30 * 1024)
  if ( _video_packets.bytes() > MAX_VIDEOQ_SIZE ||
       _audio_packets.bytes() > MAX_AUDIOQ_SIZE ||
       _subtitle_packets.bytes() > MAX_SUBTITLEQ_SIZE )
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
  _video_packets.push_back( pkt );

  if ( has_audio() )
  {
      fetch_audio( _dts + _audio_offset );
  }

  _dts = f;
  _expected = _dts + 1;
  _expected_audio = _expected + _audio_offset;

  return true;
}


/** 
 * Seek to a new frame
 * 
 * @param f   new frame
 */
void CMedia::seek( const boost::int64_t f )
{
#ifdef DEBUG_SEEK
  std::cerr << "------- SEEK " << f << std::endl;
#endif


  _seek_req   = true;
  _seek_frame = f;


  if ( stopped() )
    {
      do_seek();
    }

#ifdef DEBUG_SEEK
  std::cerr << "------- SEEK DONE " << f << " _dts: " << _dts << " _frame: " 
	    << _frame << " _expected: " << _expected << std::endl;
#endif
}


/** 
 * Cache picture for sequence.
 * 
 * @param pic       picture to cache
 */
void CMedia::cache( const mrv::image_type_ptr& pic )
{
   assert( pic != NULL );

   if ( ( !is_sequence() ) || ( !_cache_active ) ) 
      return;

   SCOPED_LOCK( _mutex );

  boost::int64_t f = pic->frame();
  if      ( f < _frameStart ) f = _frameStart;
  else if ( f > _frameEnd )   f = _frameEnd;

  boost::int64_t idx = f - _frame_start;
  if ( _sequence[idx] ) return;



  mrv::image_type_ptr np;

  if ( _8bit_cache && pic->pixel_type() != image_type::kByte )
  {

      unsigned w = pic->width();
      unsigned h = pic->height();

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
          _w /= (1 << _cache_scale);
          _h /= (1 << _cache_scale);
          np.reset( np->resize( _w, _h ) );
      }

      _sequence[idx] = np;
  }
  else
  {
      if ( _cache_scale > 0 )
      {
          _w /= (1 << _cache_scale);
          _h /= (1 << _cache_scale);
          np.reset( pic->resize( _w, _h ) );
          _sequence[idx] = np;
      }
      else
      {
          _sequence[idx] = pic;
      }
  }

  if ( _stereo[1] ) {
      _right[idx] = _stereo[1];
  }
  timestamp(idx);

}

/** 
 * Cache picture for sequence.
 * 
 * @param left    left picture to cache
 * @param right   right picture to cache
 */
void CMedia::stereo_cache( const mrv::image_type_ptr& left,
                           const mrv::image_type_ptr& right )
{
   assert( left != NULL && right != NULL );
   assert( left->frame() == right->frame() );

   if ( !is_sequence()) 
      return;

   SCOPED_LOCK( _mutex );

  boost::int64_t f = left->frame();
  if      ( f < _frameStart ) f = _frameStart;
  else if ( f > _frameEnd )   f = _frameEnd;

  boost::int64_t idx = f - _frame_start;
  if ( _sequence[idx] ) return;

  _sequence[idx] = left;
  _right[idx] = right;
  timestamp(idx);

}

/** 
 * Check if cache is already filled for a frame
 * 
 * @param frame   frame to check
 * 
 * @return true or false if cache is already filled.
 */
bool CMedia::is_cache_filled(boost::int64_t frame)
{
    if ( !_sequence ) return false;


  if ( frame < _frameStart ) frame = _frameStart;
  if ( frame > _frameEnd )   frame = _frameEnd;

  SCOPED_LOCK( _mutex );

  boost::uint64_t i = frame - _frame_start;
  if ( !_sequence[i] ) return false;

  return true;
}



/** 
 * Return an EXIF attribute.
 * 
 * @param name   attribute to find
 * 
 * @return  exif attribute value or NULL if not found
 */
const char* const CMedia::exif( const std::string& name ) const
{
  Attributes::const_iterator i = _exif.find( name );
  if ( i == _exif.end() ) return NULL;
  return i->second.c_str();
}


/** 
 * Flushes all caches
 * 
 */
void CMedia::flush_all()
{
    if ( _eye[1] )  _eye[1]->flush_all();
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

	  r += s->width() * s->height() * s->channels() * s->pixel_size();
	}
    }
  else
    {
      if ( hires() )
	{
	  r += ( _hires->width() * _hires->height() * _hires->channels() * 
		 _hires->pixel_size() );
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
	  ascii = false; break;
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
  } else if (enc->codec_id == CODEC_ID_MPEG2TS) {
    /* fake mpeg2 transport stream codec (currently not
       registered) */
     codec_name = N_("mpeg2ts");
  } else {
    /* output avi tags */
    if(   isprint(enc->codec_tag&0xFF) && isprint((enc->codec_tag>>8)&0xFF)
	  && isprint((enc->codec_tag>>16)&0xFF) && 
	  isprint((enc->codec_tag>>24)&0xFF)){
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

  if ( stream->avg_frame_rate.den && stream->avg_frame_rate.num )
  {
      fps = av_q2d( stream->avg_frame_rate );
  }
  else if ( stream->r_frame_rate.den && stream->r_frame_rate.num > 1  )
    {
      fps = av_q2d( stream->r_frame_rate );
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

  AVStream* stream = context->streams[stream_index];
  double time  = av_q2d( stream->time_base );

  if ( stream->start_time == AV_NOPTS_VALUE )
    {
      s.start = 1;
    }
  else
    {
        s.start = ((double) stream->start_time * time);
    }

  if ( stream->duration != AV_NOPTS_VALUE )
    {
        s.duration = ((double) stream->duration * time);
    }
  else
    {
        s.duration = ((double) _context->duration * time);
    }
}



// Convert an FFMPEG pts into a frame number
boost::uint64_t CMedia::frame2pts( const AVStream* stream, 
				   const boost::int64_t frame ) const
{
   assert( frame >= _frame_start );
   assert( frame <= _frame_end );
   long double p = (long double)(frame - 1) / (long double) fps();

   if ( stream )
   {
       // reverse num/den correct here
       p /= stream->time_base.num;
       p *= stream->time_base.den;
   }

   if ( p < 0 )  return AV_NOPTS_VALUE;
   else return uint64_t(p);
}


// Convert an FFMPEG pts into a frame number
boost::int64_t CMedia::pts2frame( const AVStream* stream, 
				  const boost::int64_t dts ) const
{
   static boost::int64_t frame = 0;

   boost::int64_t pts = dts;

   if ( pts == MRV_NOPTS_VALUE ) {
      if ( frame == _frame_end ) frame -= 1;
      pts = frame2pts( stream, frame+1 );
   }


  assert( pts != MRV_NOPTS_VALUE );
  if (!stream) return 0;

  long double p = pts;
  p *= stream->time_base.num;
  p /= stream->time_base.den;
  //  p *= av_q2d( stream->time_base );
  p *= fps();
  frame = boost::int64_t( p + 0.5 ) + 1;
  return frame;
}




// Return the number of frames cached for jog/shuttle
unsigned int CMedia::max_video_frames()
{
    if ( _video_cache_size > 0 )
       return _video_cache_size;
   else
       return unsigned( fps()*2 );
}


void CMedia::loop_at_start( const boost::int64_t frame )
{
   if ( has_picture() )
   {
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


void CMedia::loop_at_end( const boost::int64_t frame )
{


   if ( has_picture() )
   {
      _video_packets.loop_at_end( frame );
   }

  if ( number_of_audio_streams() > 0 )
    {
       _audio_packets.loop_at_end( frame );
    }

  if ( number_of_subtitle_streams() > 0 )
    {
       _subtitle_packets.loop_at_end( frame );
    }
}





void CMedia::preroll( const boost::int64_t f )
{
  // nothing to do for image sequences
  find_image( f );
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
            return;
	}

      CONDITION_WAIT( _video_packets.cond(), vpm );
    }
  return;
}


CMedia::DecodeStatus CMedia::handle_video_seek( boost::int64_t& frame,
                                                const bool is_seek )
{
  Mutex& vpm = _video_packets.mutex();
  SCOPED_LOCK( vpm );

  if ( is_seek && _video_packets.is_seek() )
     _video_packets.pop_front();  // pop seek begin packet
  else if ( !is_seek && _video_packets.is_preroll() )
     _video_packets.pop_front();
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
     _video_packets.pop_front();  // pop seek end packet

      
#ifdef DEBUG_VIDEO_PACKETS
  debug_video_packets(frame, "AFTER HSEEK");
#endif

#ifdef DEBUG_VIDEO_STORES
  debug_video_stores(frame, "AFTER HSEEK");
#endif
  return got_video;
}

CMedia::DecodeStatus CMedia::decode_video( boost::int64_t& frame )
{ 

  mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
  SCOPED_LOCK( vpm );

  if ( _video_packets.empty() )
    return kDecodeMissingFrame;


  DecodeStatus got_video = kDecodeMissingFrame;

  while ( got_video != kDecodeOK && !_video_packets.empty() )
    {
      if ( _video_packets.is_flush() )
	{
	   flush_video();
	  _video_packets.pop_front();
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
          if ( pkt.pts != frame ) return kDecodeOK;

          _video_packets.pop_front();
          return kDecodeLoopStart;
      }
      else if ( _video_packets.is_loop_end() )
      {
          // We check packet integrity as the length of packets is
          // not accurate.
          const AVPacket& pkt = _video_packets.front();
          if ( pkt.pts != frame ) return kDecodeOK;

	  _video_packets.pop_front();
	  return kDecodeLoopEnd;
      }
      else
      {
          _video_packets.pop_front();
          return kDecodeOK;
      }

    }


  return got_video;
}

CMedia::DecodeStatus CMedia::decode_subtitle( const boost::int64_t frame )
{ 
  return kDecodeOK;
}

bool CMedia::find_subtitle( const boost::int64_t f )
{
  return false;
}

bool CMedia::find_image( const boost::int64_t frame )
{ 
  boost::int64_t f = frame;
  if ( f > _frameEnd )       f = _frameEnd;
  else if ( f < _frameStart) f = _frameStart;


  // _video_pts = int64_t( double(f) / _fps * 1000000.0 );
  _video_clock = double(av_gettime_relative()) / 1000000.0;

  // Check if we have a cached frame for this frame
  
  boost::uint64_t idx = f - _frame_start;
  assert( idx <= (_frame_end - _frame_start) );

  image_damage( image_damage() | kDamage3DData );

  if ( _sequence && _sequence[idx] )
    {
        SCOPED_LOCK( _mutex );
        _hires = _stereo[0] = _sequence[idx];
        if ( _right && _right[idx])
            _stereo[1] = _right[idx];

        assert( _hires != NULL );
        _frame = f;

        free(_filename);
        _filename = NULL;

        refresh();
        return true;
    }

  bool should_load = false;

  std::string file;
  {
      SCOPED_LOCK( _mutex );
      file = sequence_filename(f);
  }

  if ( _filename ) 
    {
      if ( file != _filename )
      {
          SCOPED_LOCK( _mutex );
          should_load = true;
          free( _filename );
          _filename = strdup( file.c_str() );
      }
    }
  else
    {
        SCOPED_LOCK( _mutex );
        _filename = strdup( file.c_str() );
        should_load = true;
    }


  if ( should_load )
  {
     if ( fs::exists(file) )
     {
         SCOPED_LOCK( _mutex );
         SCOPED_LOCK( _audio_mutex );
         SCOPED_LOCK( _subtitle_mutex );
         fetch( f );
         cache( _hires );
     }
     else
     {
	if ( ! internal() )
	{
	   LOG_ERROR( file << _(" is missing.") );
	   return false;
	}
     }
  }

  _frame = f;


  refresh();
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

unsigned widthPerThread  = 1024;
unsigned heightPerThread = 20;
unsigned numPixelsPerThread = widthPerThread * heightPerThread;


struct AnaglyphData
{
    bool left_red;
    mrv::Recti daw[2]; // data windows
    mrv::image_type_ptr* stereo;
    mrv::image_type_ptr hires;
};


void anaglyph_cb( AnaglyphData* d )
{
   const image_type_ptr* stereo = d->stereo;
   image_type_ptr& hires = d->hires;

   short idx1 = 0;
   short idx2 = 1;
   if ( !d->left_red )
   {
      idx1 = 1;
      idx2 = 0;
   }


   mrv::Recti daw = d->daw[0];
   daw.merge( d->daw[1] );

   boost::uint8_t* data = (boost::uint8_t*)hires->data().get();
   memset( data, 0, hires->data_size() );


   for ( int y = daw.t(); y < daw.b(); ++y )
   {
       for ( int x = daw.l() ; x < daw.r(); ++x )
       {
           CMedia::Pixel pr, pc;

           int x1 = x - d->daw[idx1].l();
           int y1 = y - d->daw[idx1].t();

           if ( x1 < 0 || y1 < 0 || x1 >= (int)stereo[idx1]->width() ||
                y1 >= (int)stereo[idx1]->height() ) 
               pr = CMedia::Pixel(0,0,0,0);
           else
               pr = stereo[idx1]->pixel( x1, y1 );


           x1 = x - d->daw[idx2].l();
           y1 = y - d->daw[idx2].t();

           
           if ( x1 < 0 || y1 < 0 || x1 >= (int)stereo[idx2]->width() ||
                y1 >= (int) stereo[idx2]->height() ) 
               pc = CMedia::Pixel(0,0,0,0);
           else
               pc = stereo[idx2]->pixel( x1, y1 );
           CMedia::Pixel p = pc;
           p.r = pr.r;

           x1 = x - daw.l();
           y1 = y - daw.t();


           hires->pixel( x1, y1, p );
       }
   }

   delete d;


}


void CMedia::make_anaglyph( bool left_red )
{
   
   if ( ! _stereo[0] || ! _stereo[1] )
   {
      LOG_ERROR( "Stereo image missing" );
      return;
   }
   
   if ( _stereo[0] == _stereo[1] )
   {
      LOG_ERROR( "Stereo images are the same" );
      return;
   }

   const mrv::Recti& dpw = display_window();
   const mrv::Recti& daw = data_window();
   const mrv::Recti& dpw2 = display_window2();
   const mrv::Recti& daw2 = data_window2();

   mrv::Recti d = daw;
   d.merge( daw2 );

   unsigned w = d.w();
   if ( w == 0 ) w = width();
   unsigned h = d.h();
   if ( h == 0 ) h = height();

   _w = w;
   _h = h;
   allocate_pixels( _frame, 4, image_type::kRGBA, image_type::kHalf );



   AnaglyphData* data = new AnaglyphData;
   data->left_red = left_red;
   data->stereo = _stereo;
   data->hires  = _hires;

   data->daw[0] = data_window();
   data->daw[1] = data_window2();
   anaglyph_cb( data );

   cache( _hires );
   data_window( d.x(), d.y(), d.x()+_w-1, d.y()+_h-1 );


   refresh();
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
  if ( stream->codec->codec_type != AVMEDIA_TYPE_VIDEO ) return;

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



void CMedia::debug_video_packets(const boost::int64_t frame, 
				 const char* routine,
				 const bool detail)
{
  if ( !has_video() && !is_sequence() ) return;

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
     if ( _video_packets.is_loop_end( *(last-1) ) ||
	  _video_packets.is_loop_start( *(last-1) ) )
     {
	std::cerr << (*iter).dts;
     }
     else
     {
        if ( stream )
           std::cerr << pts2frame( stream, (*iter).dts ) - _frame_offset;
        else
           std::cerr << (*iter).dts;
     }

     std::cerr << '-';

     if ( _video_packets.is_loop_end( *(last-1) ) ||
	  _video_packets.is_loop_start( *(last-1) ) )
     {
	std::cerr << (*(last-1)).dts;
     }
     else
     {
        if ( stream )
           std::cerr << pts2frame( stream, (*(last-1)).dts ) - _frame_offset;
        else
           std::cerr << (*(last-1)).dts;
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
	   std::cerr << "* "; continue;
	}
	else if ( _video_packets.is_loop_start( *iter ) ||
		  _video_packets.is_loop_end( *iter ) )
	{
	   std::cerr << "L" << (*iter).dts << " "; continue;
	}
	
	assert( (*iter).dts != MRV_NOPTS_VALUE );
	
	boost::int64_t f;
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
	   std::cerr << f << " ";
	}
     }
  }

  std::cerr << std::endl << std::endl;
}


void CMedia::add_shape( mrv::shape_type_ptr s )
{
   _shapes.push_back( s );
   _undo_shapes.clear();
}

} // namespace mrv
