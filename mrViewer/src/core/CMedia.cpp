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
#include <libavformat/avformat.h>
}



#include <iostream>
#include <algorithm>  // for std::min, std::abs
#include <limits>

#undef  __STDC_CONSTANT_MACROS
#include <boost/cstdint.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
namespace fs = boost::filesystem;


#include "CMedia.h"

#include "Sequence.h"
#include "mrvColorProfile.h"
#include "mrvIO.h"
#include "mrvException.h"
#include "mrvPlayback.h"
#include "mrvFrameFunctors.h"
#include "mrvThread.h"
#include "mrvI8N.h"
#include "mrvOS.h"


namespace {

  const char* kModule = "img";

}

#define IMG_WARNING(x) LOG_WARNING( name() << " - " << x ) 
#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )

// #define DEBUG_SEEK
// #define DEBUG_PACKETS
// #define DEBUG_STORES

// #define DEBUG_DECODE
// #define DEBUG_AUDIO_SPLIT




namespace mrv {


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


/** 
 * Constructor
 * 
 */
CMedia::CMedia() :
  _w( 0 ),
  _h( 0 ),
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
  _play_fps( 0 ),
  _fps( 0 ),
  _pixel_ratio( 1.0f ),
  _num_channels( 0 ),
  _rendering_intent( kUndefinedIntent ),
  _gamma( 1 ),
  _dts( 1 ),
  _audio_frame( 1 ),
  _frame( 1 ),
  _frameStart( 1 ),
  _frameEnd( 1 ),
  _interlaced( kNoInterlace ),
  _image_damage( kNoDamage ),
  _damageRectangle( 0, 0, 0, 0 ),
  _profile( NULL ),
  _rendering_transform( NULL ),
  _look_mod_transform( NULL ),
  _playback( kStopped ),
  _sequence( NULL ),
  _video_clock( av_gettime() / 1000000.0 ),
  _context(NULL),
  _audio_codec(NULL),
  _subtitle_index(-1),
  _audio_index(-1),
  _samples_per_sec( 0 ),
  _audio_buf_used( 0 ),
  _video_pts( 0 ),
  _audio_pts( 0 ),
  _audio_clock( av_gettime() / 1000000.0 ),
  _audio_buf( NULL ),
  _audio_engine( NULL )
{
  audio_initialize();
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
  _play_fps( 0 ),
  _fps( 0 ),
  _pixel_ratio( 1.0f ),
  _num_channels( 0 ),
  _rendering_intent( kUndefinedIntent ),
  _gamma( 1 ),
  _dts( 1 ),
  _audio_frame( 1 ),
  _frame( 1 ),
  _frameStart( 1 ),
  _frameEnd( 1 ),
  _interlaced( kNoInterlace ),
  _image_damage( kNoDamage ),
  _damageRectangle( 0, 0, 0, 0 ),
  _profile( NULL ),
  _rendering_transform( NULL ),
  _look_mod_transform( NULL ),
  _playback( kStopped ),
  _sequence( NULL ),
  _context(NULL),
  _audio_codec(NULL),
  _subtitle_index(-1),
  _audio_index(-1),
  _samples_per_sec( 0 ),
  _audio_buf_used( 0 ),
  _audio_buf( NULL ),
  _audio_engine( NULL )
{
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
  }

  if ( xScale != 1.0f || yScale != 1.0f )
    {
      _hires.reset( _hires->scaleX( xScale ) );
      _hires.reset( _hires->scaleY( yScale ) );
      _w = _hires->width();
      _h = _hires->height();
      _damageRectangle = mrv::Recti(0, 0, _w, _h);
    }

  _disk_space = 0;
}



boost::int64_t CMedia::get_frame( const AVStream* stream, const AVPacket& pkt )
{
   assert( stream != NULL );
   boost::int64_t pts;
   if ( pkt.pts != MRV_NOPTS_VALUE )
   {
      pts = pts2frame( stream, pkt.pts );
   }
   else
   {
      assert( pkt.dts != MRV_NOPTS_VALUE );
      pts = pts2frame( stream, pkt.dts );
   }
   return pts;
}


/** 
 * Clears cache of frames.  @todo: refactor
 * 
 */
void CMedia::clear_sequence()
{
  if ( _sequence == NULL ) return;

  SCOPED_LOCK( _mutex);

  boost::uint64_t num = _frameEnd - _frameStart + 1;
  for ( boost::uint64_t i = 0; i < num; ++i )
    {
      _sequence[i].reset();
    }
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

  free( _fileroot );
  free( _filename );
  free( _label );
  free( _profile );
  free( _rendering_transform );
  free( _look_mod_transform );


  clear_sequence();
  delete [] _sequence;

  if ( has_audio() )
    {
      close_audio();

      delete [] _audio_buf; _audio_buf = NULL;

      close_audio_codec();
    }

  audio_shutdown();

  if ( _context )
    av_close_input_file( _context );

  _context = NULL;
}


/** 
 * Allocate the float pixels for the image
 * 
 * @param ws image width in pixels
 * @param hs image height in pixels
 */
void CMedia::allocate_pixels( const boost::int64_t& frame,
				 const unsigned int channels,
				 const image_type::Format format,
				 const image_type::PixelType pixel_type )
{
  SCOPED_LOCK( _mutex );
  _hires.reset( new image_type( frame, width(), height(), 
				channels, format, pixel_type ) );
}


void CMedia::display_window( const int xmin, const int ymin,
				const int xmax, const int ymax )
{
  assert( xmax >= xmin );
  assert( ymax >= ymin );
  _displayWindow = mrv::Recti( xmin, ymin, xmax, ymax );
}

void CMedia::data_window( const int xmin, const int ymin,
			     const int xmax, const int ymax )
{
  assert( xmax >= xmin );
  assert( ymax >= ymin );
  _dataWindow = mrv::Recti( xmin, ymin, xmax, ymax );
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
   _frameStart = x;
   if ( is_sequence() )
   {
      delete [] _sequence;
      _sequence = NULL;
      
      boost::uint64_t num = _frameEnd - _frameStart + 1;
      _sequence = new mrv::image_type_ptr[num];
   }
}

void  CMedia::last_frame(boost::int64_t x)
{
   _frameEnd = x;
   if ( is_sequence() )
   {
      delete [] _sequence;
      _sequence = NULL;
      
      boost::uint64_t num = _frameEnd - _frameStart + 1;
      _sequence = new mrv::image_type_ptr[num];
   }
}

/** 
 * Treat image as a sequence of frames
 * 
 * @param fileroot  a fileroot in C format like crash.%d.exr
 * @param start     start frame
 * @param end       end   frame
 */
void CMedia::sequence( const char* fileroot, const boost::int64_t start,
		       const boost::int64_t end )
{
  SCOPED_LOCK( _mutex );

  assert( fileroot != NULL );
  assert( start < end );

  if ( _fileroot && strcmp( fileroot, _fileroot ) == 0 && 
       start == _frameStart && end == _frameEnd )
    return;

  _fileroot = strdup( fileroot );

  if ( _filename ) {
    free( _filename );
    _filename = NULL;
  }


  _is_sequence = true;
  _dts = _frame = start;
  _frameStart = start;
  _frameEnd = end;

  delete [] _sequence;
  _sequence = NULL;

  boost::uint64_t num = _frameEnd - _frameStart + 1;
  _sequence = new mrv::image_type_ptr[num];

  // timestamp all pictures
  struct stat sbuf;

  for ( int64_t f = _frameStart; f <= _frameEnd; ++f )
    {
      int result = stat( sequence_filename( f ).c_str(), &sbuf );
      if ( result < 0 ) return;
    }


  if ( ! initialize() )
    return;
  
  fetch(_dts);

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

  _fileroot = strdup( n );

  if ( _filename ) 
    {
      free( _filename );
      _filename = NULL;
    }

  _is_sequence = false;
  _dts = _frame = _frameStart = _frameEnd = 1;


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
const char* CMedia::filename() const
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
 * 
 * @return filename of the file for current frame
 */
std::string CMedia::sequence_filename( const boost::int64_t frame )
{
  if ( !is_sequence() ) return _fileroot;

  // For image sequences
  char buf[1024];
  sprintf( buf, _fileroot, frame );
  return std::string( buf );
}



/** 
 * Returns whether the file has changed on disk.
 * 
 * 
 * @return true if it has changed, false if not.
 */
bool CMedia::has_changed()
{
  struct stat sbuf;

  if ( is_sequence() ) 
    {
      SCOPED_LOCK( _mutex );

      std::string file = sequence_filename(_frame);

      int result = stat( file.c_str(), &sbuf );
      if ( result == -1 ) return false;

      boost::uint64_t idx = _frame - _frameStart;

      if ( !_sequence || !_sequence[idx] ) return false;

      if ( _sequence[idx]->mtime() != sbuf.st_mtime )
	{
	  assert( _frame == _sequence[idx]->frame() );
	  // update frame...
	  _sequence[idx].reset();
	  fetch( _frame );
	  return false;
	}
    }
  else
    {
      if ( !_fileroot ) return false;

      int result = stat( _fileroot, &sbuf );
      if ( result == -1 ) return false;

      if ( _mtime != sbuf.st_mtime )
	return true;

      if ( _ctime != sbuf.st_ctime )
	return true;

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

image_type::PixelType CMedia::depth() const
{
  if ( _hires )      return _hires->pixel_type();
  else return image_type::kByte;
}

void CMedia::gamma( const float x )
{
  _gamma = x;
  image_damage( image_damage() | kDamageData );
  refresh();
}


void CMedia::chromaticities( const Imf::Chromaticities& c )
{
  _chromaticities = c;
  image_damage( image_damage() | kDamageData | kDamageLut );
  refresh();
}

/** 
 * Add alpha and alpha overlay layers to list of layers
 * 
 */
void CMedia::alpha_layers()
{
  _layers.push_back( "Alpha" );
  if ( _num_channels != 0 )
    _layers.push_back( "Alpha Overlay" );
  ++_num_channels;
  image_damage( image_damage() | kDamageLayers | kDamageData );
}

/** 
 * Add color and r,g,b to list of layers
 * 
 */
void CMedia::rgb_layers()
{
  _layers.insert( _layers.begin(), "Color" );
  _layers.push_back( "Red" );
  _layers.push_back( "Green" );
  _layers.push_back( "Blue" );
  _num_channels += 3;
  image_damage( image_damage() | kDamageLayers | kDamageData );
}

/** 
 * Add r,g,b and lumincan to list of layers
 * 
 */
void CMedia::lumma_layers()
{
  _layers.push_back( "Lumma" );
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


/** 
 * Change the image's color channel
 * 
 * @param chinput new image channel
 */
void CMedia::channel( const char* chinput )
{
  const char* c = chinput;
  std::string ch( chinput );
  if ( ch == "Color" || ch == "Red" || ch == "Green" || ch == "Blue" ||
       ch == "Alpha" || ch == "Alpha Overlay" || ch == "Lumma" )
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
      clear_sequence();
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
  if ( _disk_space == 0 ) _disk_space = sbuf.st_size;
  else _disk_space = (_disk_space + sbuf.st_size)/2;
  _disk_space *= size_t( last_frame() - first_frame() + 1 );
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
  clear_sequence();
  image_damage( image_damage() | kDamageData | kDamageLut );
  refresh();
}

/** 
 * Returns image CTL script (or NULL if no script)
 * 
 * 
 * @return CTL script or NULL
 */
const char* CMedia::look_mod_transform()  const
{
  return _look_mod_transform;
}


/** 
 * Change the look mod transform for the image
 * 
 * @param cfile  CTL script name
 */
void CMedia::look_mod_transform( const char* cfile )
{
  free( _look_mod_transform ); _look_mod_transform = NULL;
  if ( cfile && strlen(cfile) > 0 ) _look_mod_transform = strdup( cfile );
  image_damage( image_damage() | kDamageData | kDamageLut );
  clear_sequence();
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
  clear_sequence();
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


/// VCR play (and record if needed) sequence
void CMedia::play(const CMedia::Playback dir, 
		     mrv::ViewerUI* const uiMain)
{

  if ( dir == _playback && !_threads.empty() ) return;

  stop();


  _playback = dir;

  assert( uiMain != NULL );


  _dts = _audio_frame = _frame;
  _expected = std::numeric_limits< boost::int64_t >::min();

  _audio_clock = 0.0;
  _video_clock = 0.0;

  _audio_buf_used = 0;


  // clear all packets
  clear_packets();
  clear_stores(); // clear audio store


  // Start threads
  PlaybackData* data = new PlaybackData( uiMain, this );
  assert( data != NULL );
  assert( data->uiMain != NULL );
  assert( data->image != NULL );

  PlaybackData* video_data, *audio_data, *subtitle_data;

#if 0
  bool     valid_video = false;
  unsigned num_streams = number_of_video_streams();
  for ( unsigned i = 0; i < num_streams; ++i )
    {
      if ( _video_info[i].has_codec ) { valid_video = true; break; }
    }

  // If there's at least one valid video stream, create video thread
  if ( valid_video || is_sequence() )
    {
      video_data = new PlaybackData( *data );
      _threads.push_back( new boost::thread( boost::bind( mrv::video_thread, 
							  video_data ) ) );
    }
#else
  bool     valid_video = number_of_video_streams() > 0;
  unsigned num_streams;

  // If there's at least one valid video stream, create video thread
  if ( valid_video || is_sequence() )
    {
      valid_video = true;
      video_data = new PlaybackData( *data );
      _threads.push_back( new boost::thread( boost::bind( mrv::video_thread, 
							  video_data ) ) );
    }
#endif

  bool valid_audio = false;
  num_streams = number_of_audio_streams();
  for ( unsigned i = 0; i < num_streams; ++i )
    {
      if ( _audio_info[i].has_codec ) { valid_audio = true; break; }
    }


  // If there's at least one valid audio stream, create audio thread
  if ( valid_audio )
    {
      // Audio playback thread
      audio_data = new PlaybackData( *data );
      _threads.push_back( new boost::thread( boost::bind( mrv::audio_thread,
							  audio_data ) ) );
    }

  bool valid_subtitle = false;
  num_streams = number_of_subtitle_streams();
  for ( unsigned i = 0; i < num_streams; ++i )
    {
      if ( _subtitle_info[i].has_codec ) { valid_subtitle = true; break; }
    }


  // If there's at least one valid subtitle stream, create subtitle thread
  if ( valid_subtitle )
    {
      // Subtitle playback thread
      subtitle_data = new PlaybackData( *data );
      _threads.push_back( new boost::thread( boost::bind( mrv::subtitle_thread,
							  subtitle_data ) ) );
    }

// #if defined(FOUR_THREADS)
//   // Playback thread
//   PlaybackData* play_data = new PlaybackData( *data );
//   _threads.push_back( new boost::thread( boost::bind( mrv::playback_thread, 
// 					                 play_data ) ) );
// #endif

#if 1
  // Decoding thread
  if ( valid_audio || valid_video || valid_subtitle )
    {
      _loop_barrier = new Barrier( 1 + valid_audio + valid_video + 
				   valid_subtitle );
      _threads.push_back( new boost::thread( boost::bind( mrv::decode_thread, 
							  data ) ) );
    }
#else
  if ( is_sequence() )
    {
      delete data;
    }
  else
    {
    }
#endif

}

/// VCR stop sequence
void CMedia::stop()
{

  if ( _playback != kStopped ) {
    _playback = kStopped;

    //
    // Notify loop barrier, to exit any wait on a loop
    //
    if ( _loop_barrier ) _loop_barrier->notify_all();

    // Notify packets, to make sure that audio thread exits any wait lock
    // This needs to be done even if no audio is playing, as user might
    // have turned off audio, but audio thread is still active.
    _audio_packets.cond().notify_one();
    _video_packets.cond().notify_one();
    _subtitle_packets.cond().notify_one();
    _video_cond.notify_one();

    // Wait for all threads to exit
    wait_for_threads();

    // Clear barrier
    delete _loop_barrier; _loop_barrier = NULL;

    // Clear any audio/video/subtitle packets
    clear_packets();

    // Queue thumbnail for update
    image_damage( image_damage() | kDamageThumbnail );
  }


}



/** 
 * 
 * @return the image's name sans directory
 */
std::string CMedia::name() const
{
  fs::path file = fs::path( fileroot(), fs::native );
  return file.leaf();
}


/** 
 * 
 * @return  the image's directory
 */
std::string CMedia::directory() const
{
  fs::path file = fs::path( fileroot(), fs::native );
  std::string path( file.branch_path().string() );
  char buf[1024];
  if ( path == "" ) {
    file = fs::path( getcwd(buf,1024), fs::native );
    path = file.string();
  }
  return path;
}



/** 
 * Set a new frame for the image sequence
 * 
 * @param f new frame
 */
bool CMedia::frame( const boost::int64_t f )
{
  assert( _fileroot != NULL );

  if ( !is_sequence() ) return true;

//  in ffmpeg, sizes are in bytes...
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)
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

  mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
  SCOPED_LOCK( vpm );

  if ( _dts != _expected )
    {
      _video_packets.seek_begin(_dts);
      _video_packets.seek_end(_dts);
    }

  AVPacket pkt;
  av_init_packet( &pkt );
  pkt.dts = pkt.pts = _dts;
  pkt.size = 0; // width() * height() * 16; 
  pkt.data = NULL;
  pkt.destruct = av_destruct_packet; // prevent av_dup_packet()
  _video_packets.push_back( pkt );

  _expected = _dts + 1;

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
 * @param pi       picture to cache
 */
void CMedia::cache( const mrv::image_type_ptr& pic )
{
  boost::int64_t f = pic->frame();
  if      ( f < _frameStart ) f = _frameStart;
  else if ( f > _frameEnd )   f = _frameEnd;

  boost::uint64_t idx = f - _frameStart;
  if ( _sequence[idx] ) return;

  _sequence[idx] = pic;
  timestamp(idx);
}


/** 
 * Check if cache is already filled for a frame
 * 
 * @param frame   frame to check
 * 
 * @return true or false if cache is already filled.
 */
bool CMedia::is_cache_filled(boost::int64_t frame) const
{
  if ( !_sequence ) return false;

  if ( frame < _frameStart ) frame = _frameStart;
  if ( frame > _frameEnd )   frame = _frameEnd;

  boost::uint64_t i = frame - _frameStart;
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
      boost::uint64_t frames = _frameEnd - _frameStart + 1;
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
    case CODEC_TYPE_VIDEO:
      stream = "video";
      break;
    case CODEC_TYPE_AUDIO:
      stream = "audio";
      break;
    case CODEC_TYPE_DATA:
      stream = "data";
      break;
    case CODEC_TYPE_SUBTITLE:
      stream = "subtitle";
      break;
    default:
      stream = "unknown";
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
      sprintf( buf, "0x%08x", codec_tag );
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
    if (enc->codec_id == CODEC_ID_MP3) {
      if (enc->sub_id == 2)
	codec_name = "mp2";
      else if (enc->sub_id == 1)
	codec_name = "mp1";
    }
  } else if (enc->codec_id == CODEC_ID_MPEG2TS) {
    /* fake mpeg2 transport stream codec (currently not
       registered) */
    codec_name = "mpeg2ts";
  } else if (enc->codec_name[0] != '\0') {
    codec_name = enc->codec_name;
  } else {
    /* output avi tags */
    if(   isprint(enc->codec_tag&0xFF) && isprint((enc->codec_tag>>8)&0xFF)
	  && isprint((enc->codec_tag>>16)&0xFF) && 
	  isprint((enc->codec_tag>>24)&0xFF)){
      snprintf(buf, sizeof(buf), "%c%c%c%c / 0x%04X",
	       enc->codec_tag & 0xff,
	       (enc->codec_tag >> 8) & 0xff,
	       (enc->codec_tag >> 16) & 0xff,
	       (enc->codec_tag >> 24) & 0xff,
	       enc->codec_tag);
    } else {
      snprintf(buf, sizeof(buf), "0x%04x", enc->codec_tag);
    }
    codec_name = buf;
  }
  
  return std::string( codec_name );
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

  if ( stream->r_frame_rate.den && stream->r_frame_rate.num > 1  )
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
				      const AVCodecContext *codec_context, 
				      const int stream_index )
{

  bool has_codec = true;

  // Mark streams that we don't have a decoder for
  AVCodec* codec = avcodec_find_decoder( codec_context->codec_id );
  if ( ! codec )
    {
      has_codec = false;
      const char* type = stream_type( codec_context );
      msg << "\n\nNot a known codec " << codec_name(codec_context) 
	  << " for " << " stream #" << stream_index << ", type " << type;
    }

  s.stream_index = stream_index;
  s.has_codec    = has_codec;
  s.codec_name = codec_name( codec_context );
  s.fourcc     = codec_tag2fourcc( codec_context->codec_tag );

  AVStream* stream = _context->streams[stream_index];
  double time  = av_q2d( stream->time_base );

  if ( stream->start_time == MRV_NOPTS_VALUE )
    {
      s.start = 1;
    }
  else
    {
      s.start = stream->start_time * time;
    }

  if ( stream->duration == MRV_NOPTS_VALUE )
    {
      s.duration = _context->duration * time;
    }
  else
    {
      s.duration = stream->duration * time;
    }
}



// Convert an FFMPEG pts into a frame number
boost::uint64_t CMedia::frame2pts( const AVStream* stream, 
				   const boost::int64_t frame ) const
{
   double p = (double)(frame - 1) / fps();
   if ( stream ) p /= av_q2d( stream->time_base );
   if ( p < 0 )  return AV_NOPTS_VALUE;
   else return uint64_t(p);
}


// Convert an FFMPEG pts into a frame number
boost::int64_t CMedia::pts2frame( const AVStream* stream, 
				     const boost::int64_t pts ) const
{
  assert( pts != MRV_NOPTS_VALUE );

  double p = av_q2d( stream->time_base ) * pts;
  return boost::int64_t( p * fps() + 0.5 ) + 1; // _frameStart; //is wrong

}




// Return the number of frames cached for jog/shuttle
unsigned int CMedia::max_video_frames()
{
  return unsigned( fps() * 2 );
  // return int( fps() * 0.5 );
  // return 15;
}


void CMedia::loop_at_start( const boost::int64_t frame )
{
  boost::uint64_t pts = frame2pts( NULL, frame );
  _video_packets.loop_at_start( pts );

  if ( number_of_audio_streams() > 0 )
    {
      boost::uint64_t pts = frame2pts( get_audio_stream(), frame );
      _audio_packets.loop_at_start( pts );
    }

  if ( number_of_subtitle_streams() > 0 )
    {
      boost::uint64_t pts = frame2pts( NULL, frame );
      _subtitle_packets.loop_at_start( pts );
    }
}



void CMedia::loop_at_end( const boost::int64_t frame )
{
  _video_packets.loop_at_end( frame );

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


int64_t CMedia::wait_image()
{
  mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
  SCOPED_LOCK( vpm );

  for(;;)
    {
      if ( stopped() ) break;

      if ( ! _video_packets.empty() )
	{
	  const AVPacket& pkt = _video_packets.front();

	  boost::int64_t pktframe;
	  if ( pkt.pts != MRV_NOPTS_VALUE )
	     return pkt.pts;
	  else
	     return pkt.dts;
	}

      CONDITION_WAIT( _video_packets.cond(), vpm );
    }
  return _frame;
}

CMedia::DecodeStatus CMedia::decode_video( boost::int64_t& frame )
{ 
  mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
  SCOPED_LOCK( vpm );
  if ( _video_packets.empty() )
    return kDecodeMissingFrame;

  DecodeStatus got_image = kDecodeMissingFrame;
  while ( got_image != kDecodeOK && !_video_packets.empty() )
    {
      if ( _video_packets.is_flush() )
	{
	  flush_video();
	  _video_packets.pop_front();
	  continue;
	}
      else if ( _video_packets.is_loop_start() )
	{
	  _video_packets.pop_front();
	  return kDecodeLoopStart;
	}
      else if ( _video_packets.is_loop_end() )
	{
	  _video_packets.pop_front();
	  return kDecodeLoopEnd;
	}
      else if ( _video_packets.is_seek() || _video_packets.is_preroll() )
	{
	  _video_packets.pop_front();  // remove first
	  assert( !_video_packets.empty() );
	  _video_packets.pop_front();  // remove end
	  assert( !_video_packets.empty() );
	  AVPacket& pkt = _video_packets.front();
	  frame = pkt.pts;
	}
      got_image = kDecodeOK;
      _video_packets.pop_front();
    }


  return kDecodeOK;
}

CMedia::DecodeStatus CMedia::decode_subtitle( boost::int64_t& frame )
{ 
  if ( frame > _frameEnd )       frame = _frameEnd;
  else if ( frame < _frameStart) frame = _frameStart;
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

  // Check if we have a cached frame for this frame
  
  boost::uint64_t idx = f - _frameStart;
  assert( idx <= (_frameEnd - _frameStart) );

  if ( _sequence && _sequence[idx] )
    {

      {
	SCOPED_LOCK( _mutex );
	_hires = _sequence[idx];
	assert( _hires != NULL );
	_frame = f;

	free(_filename);
	_filename = NULL;
      }

      refresh();
      return true;
    }

  bool should_load = true;

  if ( _filename ) 
    {
      char* old = _filename;
      _filename = NULL;
      if ( strcmp( filename(), old ) != 0 )
	should_load = true;
      free( old );
    }
  else
    {
      should_load = true;
    }


  if ( should_load )
  {
     _dts = f;
     if ( fs::exists( filename() ) )
     {
	SCOPED_LOCK( _audio_mutex );
	SCOPED_LOCK( _mutex );
	SCOPED_LOCK( _subtitle_mutex );
	fetch(f);
     }
     else
     {
	return false;
     }
  }

  _frame = f;
    

  refresh();
  return true;
  // return frame( f );
}


void CMedia::default_icc_profile()
{
  if ( icc_profile() ) return;

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
      IMG_ERROR("default_rendering_tranform - unknown bit depth");
      break;
    }
}
void CMedia::default_rendering_transform()
{
  if ( rendering_transform() ) return;

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
  if ( stream->codec->codec_type != CODEC_TYPE_VIDEO ) return;

  unsigned max_distance  = 0;
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
				    const char* routine)
{
  if ( !has_video() && !is_sequence() ) return;

  mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
  SCOPED_LOCK( vpm );

  mrv::PacketQueue::const_iterator iter = _video_packets.begin();
  mrv::PacketQueue::const_iterator last = _video_packets.end();
  std::cerr << name() << " S:" << _frame << " D:" << _dts << " V:" << frame 
	    << " " << routine << " video packets #" 
	    << _video_packets.size() << " (" << _video_packets.bytes() << "): "
	    << std::endl;

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
	  std::cerr << "L "; continue;
	}

      assert( (*iter).dts != MRV_NOPTS_VALUE );
      boost::int64_t f = (*iter).dts;
      if ( _video_packets.is_seek( *iter ) )
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
	      std::cerr << "<SEEK:" << f << ">";
	      in_seek = true;
	    }
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
  std::cerr << std::endl;
}


} // namespace mrv
