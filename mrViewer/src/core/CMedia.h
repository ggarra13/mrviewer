

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
 * @file   CMedia.h
 * @author 
 * @date   Sat Sep 16 01:42:12 2006
 * 
 * @brief  Base class for mrViewer image readers
 * 
 * 
 */

#ifndef CMedia_h
#define CMedia_h

#ifdef _WIN32
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#pragma warning(disable: 4800)
#endif


#include <ctime>

#include <set>
#include <map>
#include <vector>
#include <string>
#include <deque>
#include <limits>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <ImfChromaticities.h>
#include <ACESclipReader.h>

#include "core/mrvAlignedData.h"
#include "core/mrvFrame.h"
#include "core/mrvString.h"
#include "core/mrvPacketQueue.h"
#include "core/mrvImagePixel.h"
#include "core/mrvRectangle.h"
#include "core/mrvAudioEngine.h"
#include "core/mrvOS.h"
#include "core/mrvBarrier.h"
#include "core/mrvACES.h"
#include "core/mrvI8N.h"


#include "video/mrvGLShape.h"

#undef min
#undef max

typedef struct Clock {
    double pts;           /* clock base */
    double pts_drift;     /* clock base minus time at which we updated the clock */
    double last_updated;
    double speed;
    int serial;           /* clock is based on a packet with this serial */
    int paused;
    int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;
struct AVFormatContext;
struct AVFrame;
struct AVCodec;
struct AVPacket;
struct AVStream;
struct AVCodecContext;
struct SwrContext;

// Turn this on to print out stats of decoding
// #define DEBUG_DECODE



namespace mrv {

class ViewerUI;
class AudioEngine;
class ImageOpts;

void update_video_pts(CMedia* is, double pts, int64_t pos, int serial);

class CMedia
{
  public:
    typedef mrv::ImagePixel    Pixel;
    typedef  std::map< std::string, std::string > Attributes;

    typedef boost::recursive_mutex              Mutex;
    typedef boost::condition_variable_any       Condition;
    typedef mrv::barrier                        Barrier;

    typedef std::map< int, mrv::image_type_ptr > PixelBuffers;
    typedef std::map< std::string, int > LayerBuffers;



    enum AV_SYNC_TYPE {
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
    };

    //   typedef std::deque< AVPacket > PacketQueue;
    typedef mrv::PacketQueue PacketQueue;

    struct StreamInfo
    {
        const AVFormatContext* context;
        int          stream_index;
        bool         has_codec;
        std::string  codec_name;
        std::string  fourcc;
        double       start;
        double       duration;

        StreamInfo() : context( NULL ),
                       stream_index(-1), 
                       has_codec(false), 
                       start(0), 
                       duration(0) 
        {
        }

        StreamInfo( const StreamInfo& b ) :
        context( b.context ),
        stream_index( b.stream_index ), 
        has_codec( b.has_codec ),
        codec_name( b.codec_name ),
        fourcc( b.fourcc ),
        start( b.start ),
        duration( b.duration )
        {
        }

        bool has_data( boost::int64_t frame ) const
        {
            const AVStream* stream = context->streams[stream_index];
            double time  = av_q2d( stream->time_base );
            return frame <= boost::int64_t( ( duration - start ) * time );
        }

    };

    // Video stream data
    struct VideoStream : public StreamInfo
    {
        bool         has_b_frames;
        double       fps;
        std::string  pixel_format;

        VideoStream() : StreamInfo(), 
                        has_b_frames( false ),
                        fps(0)
        {
        }
    
        VideoStream( const VideoStream& b ) :
	StreamInfo( b ),
	has_b_frames( b.has_b_frames ),
	fps( b.fps ),
	pixel_format( b.pixel_format )
        {
        }

    };
    typedef VideoStream video_info_t;
    typedef std::vector< video_info_t > video_info_list_t;

    // Audio stream data
    struct AudioStream : public StreamInfo
    {
        unsigned int channels;
        unsigned int frequency;
        unsigned int bitrate;
        std::string format;
        std::string language;

        AudioStream() : StreamInfo(), channels(0), frequency(0), bitrate(0)
        {
        }

        AudioStream( const AudioStream& b ) :
	StreamInfo( b ),
	channels( b.channels ),
	frequency( b.frequency ),
	bitrate( b.bitrate ),
	format( b.format ),
	language( b.language )
        {
        }
    };
    typedef AudioStream audio_info_t;
    typedef std::vector< audio_info_t >  audio_info_list_t;


    // Subtitle stream data
    struct SubtitleStream : public StreamInfo
    {
        unsigned int bitrate;
        std::string  language;

        SubtitleStream() : StreamInfo(), bitrate(0)
        {
        }

        SubtitleStream( const SubtitleStream& b ) :
	StreamInfo( b ),
	bitrate( b.bitrate ),
	language( b.language )
        {
        }
    };

    typedef SubtitleStream subtitle_info_t;
    typedef std::vector< subtitle_info_t >   subtitle_info_list_t;


    typedef std::deque< mrv::audio_type_ptr > audio_cache_t;

    typedef std::vector< char* >          LMT;
    typedef std::vector< boost::thread* > thread_pool_t;

    enum InterlaceType
    {
    kNoInterlace,
    kTopFieldFirst,
    kBottomFieldFirst,
    };

    enum RenderingIntent
    {
    kUndefinedIntent,
    kSaturationIntent,
    kPerceptualIntent,
    kAbsoluteIntent,
    kRelativeIntent,
    };

    enum Playback
    {
    kBackwards = -1,
    kStopped   =  0,
    kForwards  =  1,
    };

    enum Damage {
    kNoDamage        = 0,
    kDamageLayers    = 1 << 1,
    kDamageContents  = 1 << 2,
    kDamageThumbnail = 1 << 3,
    kDamageSubtitle  = 1 << 4,
    kDamageData      = 1 << 5,
    kDamageLut       = 1 << 6,
    kDamage3DData    = 1 << 7,
    kDamageAll       = (kDamageLayers | kDamageContents | kDamageLut | 
                        kDamageThumbnail | kDamageData | kDamageSubtitle |
                        kDamage3DData)
    };

    enum DecodeStatus {
    kDecodeMissingFrame = 0,
    kDecodeOK = 1,
    kDecodeDone = 2,
    kDecodeError = 3,
    kDecodeMissingSamples = 4,
    kDecodeNoStream = 5,
    kDecodeLoopStart = 6,
    kDecodeLoopEnd = 7,
    kDecodeBufferFull = 8
    };

    enum StereoType {
    kNoStereo = 0,
    kStereoSideBySide = 1,
    kStereoRight      = 2,
    kStereoCrossed    = 1 + 2,
    kStereoInterlaced = 4,
    kStereoOpenGL     = 8,
    kStereoAnaglyph   = 16,
    kStereoRightAnaglyph = 16 + 2,
    kStereoInterlacedColumns = 4 + 32,
    kStereoCheckerboard = 4 + 64,
    };


  public:
    /// Fetch (load) the image for a frame
    virtual bool fetch(const boost::int64_t frame) { return true; }

    /// Constructor used to create a resized image from another image.
    CMedia( const CMedia* other, int nw, int nh );
    
    /// Constructor used to split a video or sequence in two
    //  (cut_frame - last_frame)
    CMedia( const CMedia* other, boost::int64_t cut_frame );


    virtual ~CMedia();

      
    /// Save the image under a new filename, with options opts
    bool save( const char* filename, const ImageOpts* const opts ) const;

    /// Set the image pixel ratio
    inline void  pixel_ratio( double f ) { _pixel_ratio = f; refresh(); }

    /////////////////// Set the image size, allocating a 4-float buffer
    void image_size( int w, int h );

    /////////////// Set to true if image is internal and no filename is used
    bool internal() const { return _internal; }
    const void internal(bool t) { _internal = t; }

    ///
    virtual void channel( const char* c );
    virtual const char* channel() const { return _channel; };


    // Add default Color, Red, Green, Blue
    void rgb_layers();

    // Add Lumma layer
    void lumma_layers();

    // Add alpha layers (Alpha and, optionally, Overlay) 
    void alpha_layers();

    // Add default Color, Red, Green, Blue, Alpha, Overlay, Lumma layers
    void default_layers();


    // Add default stereo.horizontal, stereo.crossed
    void add_stereo_layers();

    const mrv::Recti data_window( boost::int64_t f = AV_NOPTS_VALUE) const;
    const mrv::Recti display_window( boost::int64_t f = AV_NOPTS_VALUE) const;
    const mrv::Recti data_window2(boost::int64_t f = AV_NOPTS_VALUE) const;
    const mrv::Recti display_window2(boost::int64_t f = AV_NOPTS_VALUE) const;

    void data_window( const int xmin, const int ymin,
		      const int xmax, const int ymax );
    void display_window( const int xmin, const int ymin,
			 const int xmax, const int ymax );
    void data_window2( const int xmin, const int ymin,
                       const int xmax, const int ymax );
    void display_window2( const int xmin, const int ymin,
                          const int xmax, const int ymax );

    ////////////////// Return the list of available layers in the image
    inline const stringArray& layers() const { return _layers; }

    ////////////////// Return the current damage area in the image
    inline const mrv::Recti& damage_rectangle() const 
    { return _damageRectangle; }

    ////////////////// Mark an image rectangle for refreshing
    void refresh( const mrv::Recti& r );

    ////////////////// Mark the whole image for refreshing
    void refresh();

    ////////////////// Return if the image is damaged
    inline Damage image_damage() const { return _image_damage; };

    ////////////////// Set the image damage (for update)
    inline void  image_damage( int x ) 
    { _image_damage = (Damage) x; };

    /// Return the name of the image (sans directory)
    std::string name() const;

    /// Return the directory of the image
    std::string directory() const;

    ////////////////// Return the image filename expression
    inline const char* fileroot() const { return _fileroot; };

    ////////////////// Return the image filename for current frame
    const char* const filename() const;

    ////////////////// Set the image filename (can be a C/C++ 
    ////////////////// sprintf expression like image.%d.exr) to handle
    ////////////////// sequences.
    void filename( const char* n );

    virtual void sequence( const char* fileroot, 
			   const boost::int64_t start, 
			   const boost::int64_t end,
                           const bool use_threads );

    ////////////////// Check if image has changed on disk or network
    virtual bool has_changed();

    // Clear the sequence 8-bit cache
    virtual void clear_cache();

    inline bool has_sequence() const { return (_sequence != NULL); }

    // Returns true if cache for the frame is already filled, false if not
    virtual bool is_cache_filled(int64_t frame);

    // Store a frame in sequence cache
    void cache( const mrv::image_type_ptr& pic );
    void stereo_cache( const mrv::image_type_ptr& left, 
                       const mrv::image_type_ptr& right );

    inline PacketQueue& video_packets() { return _video_packets; }

    inline uint64_t duration() const { return _frameEnd - _frameStart + 1; }

    inline void avdiff( const double x ) { _avdiff = x; }
    inline double avdiff() const { return _avdiff; }

    ////////////////// Return the hi-res image
    inline mrv::image_type_ptr hires() const { return _hires; }
    inline mrv::image_type_ptr hires()       { return _hires; }
    void hires( const mrv::image_type_ptr pic);

    inline void is_stereo( bool x ) { _is_stereo = x; }
    inline bool  is_stereo() const { return _is_stereo; }

    inline void stereo_type( StereoType x ) { _stereo_type = x; }
    inline StereoType stereo_type() const { return _stereo_type; }

    mrv::image_type_ptr left() const;

    mrv::image_type_ptr right() const;

    ////////////////// Return the 8-bits subtitle image
    inline mrv::image_type_ptr subtitle() const { return _subtitle; }
    inline mrv::image_type_ptr subtitle()       { return _subtitle; }

    ////////////////// Set the label for the current image
    inline const char* label() const { return _label; }
    inline void label( const char* x ) { _label = strdup(x); }

    ////////////////// Set the frame for the current image (sequence)
    virtual bool    frame( const boost::int64_t frame );
    virtual boost::int64_t frame() const { return _frame; }

    ///////////////// Decoding time stamp
    inline boost::int64_t   dts()                      { return _dts; }
    inline void      dts( const boost::int64_t frame ) { _dts = frame; _expected = _dts + 1; _expected_audio = _expected + _audio_offset; } 

    ///////////////// Audio frame
    inline void audio_frame( const boost::int64_t f ) { _audio_frame = f; }
    inline boost::int64_t   audio_frame() const { return _audio_frame; }

    ////////////////// Seek to a specific frame in current image. 
    ////////////////// Frame is local to the video/sequence, not timeline.
    void seek( const boost::int64_t frame );

    ////////////////// Pre-roll sequence for playback
    ////////////////// Frame is local to the video/sequence, not timeline.
    virtual void preroll( const boost::int64_t frame );

    ////////////////// Add a loop to packet lists
    ////////////////// Frame is local to the video/sequence, not timeline.
    virtual void loop_at_start( const boost::int64_t frame ); 
    virtual void loop_at_end( const boost::int64_t frame ); 

    static const char* const decode_error( DecodeStatus err );

    ////////////////// Static functions
    static CMedia* guess_image( const char* name,
				const boost::uint8_t* datas = NULL,
				const int size = 0,
				const boost::int64_t 
				first = std::numeric_limits<boost::int64_t>::max(),
				const boost::int64_t 
				end = std::numeric_limits<boost::int64_t>::min(),
                                const bool use_threads = false );

    ////////////////////////
    // Image information
    ////////////////////////

    inline void width( unsigned int x )  { _w = x; }
    inline void height( unsigned int y ) { _h = y; }

    /// Return the image width
    inline unsigned int  width() const  {
        return _w;
    }

    /// Return the image height
    inline unsigned int  height() const {
        return _h;
    }

    /// Return the image pixel ratio
    inline double pixel_ratio() const    { return _pixel_ratio; }

    /// Returns the file format of the image
    virtual const char* const format() const { return _("Unknown"); };

    /// Set the image compression
    virtual void compression( const unsigned idx ) { };

    /// Returns the image compression (if any)
    virtual const char* const compression() const { return _("None"); };

    /// Returns the video's 4-letter codec identifier
    virtual const char* const fourcc() const { return ""; } 

    /// Returns the valid compressions for this format
    virtual stringArray valid_compressions() const {
        return stringArray();
    }

    /// Returns the image line order (if any)
    virtual const char* const line_order() const { return _("Unknown"); };

    void rendering_intent( RenderingIntent r ) { _rendering_intent = r; }

    RenderingIntent rendering_intent() const { return _rendering_intent; }

    //   virtual const char* const rendering_intent() const;

    /// Returns the image original colorspace (RGB, YUV, CMYK, etc)
    virtual const char* const colorspace() const { return "RGB"; };

    /// Returns the disk space used by image or movie (in bytes)
    inline size_t disk_space() const { return _disk_space; }

    /// Returns the size in memory of image or sequence (in bytes)
    size_t memory() const;



    virtual std::string const creation_date() const;

    /// Returns image ICC color profile
    const char* icc_profile() const;

    /// Assigns a new color profile to the image
    void icc_profile( const char* cfile );

    /// Returns rendering transform name ( CTL script )
    const char* rendering_transform() const;

    /// Assigns a new rendering transform ( CTL script ) 
    void rendering_transform( const char* cfile );

    /// Add default rendering transform (CTL script) for this image bit depth. 
    void default_rendering_transform();

    /// Add default icc profile for this image bit depth.
    void default_icc_profile();

    /// Returns input device transform name ( CTL script )
    const char* idt_transform() const;

    /// Assigns a new input device transform ( CTL script ) 
    void idt_transform( const char* cfile );

    /// Returns the number of Look Mod Transforms ( CTL scripts )
    inline size_t number_of_lmts() const { return _look_mod_transform.size(); }

    /// Returns look mod transform name ( CTL script )
    const char* look_mod_transform( const size_t idx ) const;

    /// Clears all the look mod transforms.
    void clear_look_mod_transform();

    /// Appends a new look mod transform ( CTL script ) 
    /// cfile cannot be NULL.
    void append_look_mod_transform( const char* cfile );

    /// Assigns a new look mod transform to a certain index ( CTL script )
    /// If cfile is NULL, the look mod at the index is removed.
    void look_mod_transform( const size_t idx, const char* cfile );

    void asc_cdl( const ACES::ASC_CDL& o )
    {
        _sops = o;
    }

    size_t number_of_grade_refs() const;

    const ACES::ASC_CDL& asc_cdl() const { return _sops; }

    /// True if image represents a sequence
    bool is_sequence() const { return _is_sequence; }

    virtual bool has_picture() const { return true; }

    /// True if image represents a video sequence
    virtual bool has_video() const { return false; }

    virtual const video_info_t& video_info( unsigned int i ) const
    {
        throw "No video stream";
    }

    virtual void video_stream( int x ) {}

    virtual size_t number_of_video_streams() const { return 0; }


    /// Sets the first frame in the sequence
    void  first_frame(boost::int64_t x);

    /// Sets the first frame in the sequence
    void  last_frame(boost::int64_t x);

    /// Returns the first frame in the range of playback
    inline boost::int64_t   first_frame() const { return _frameStart; }

    /// Returns the last frame in the range of playback
    inline boost::int64_t   last_frame()  const { return _frameEnd; }

    /// Returns the first frame in the video or sequence
    inline boost::int64_t   start_frame() const { return _frame_start; }

    /// Returns the last frame in the video or sequence
    inline boost::int64_t   end_frame()  const { return _frame_end; }

    inline InterlaceType interlaced() const { return _interlaced; }

    /// Returns the number of channels in the image
    inline unsigned number_of_channels() const { return _num_channels; }

    /// Returns the pixel format of the image
    image_type::Format pixel_format() const;

    const char* const pixel_format_name() const;

    /// Returns the pixel type of the image
    image_type::PixelType depth() const;

    /// Stores an embedded gamma value
    void gamma(const float g);

    /// Returns an embedded gamma value (if any)
    inline float gamma() const { return _gamma; }

    inline bool has_chromaticities() const { return _has_chromaticities; }

    inline const Imf::Chromaticities& chromaticities() const
    { return _chromaticities; }

    void chromaticities( const Imf::Chromaticities& c );

    /// Returns true if image has an alpha channel
    virtual bool  has_alpha() const { return true; }

    // Return creation time of image
    inline time_t ctime() const { return _ctime; }

    // Return modification time of image
    inline time_t mtime() const { return _mtime; }



    // return true if one valid video stream or sequence is found
    virtual bool valid_video() const;

    // return true if one valid audio stream is found
    bool valid_audio() const;

    // return true if one valid subtitle stream is found
    bool valid_subtitle() const;



    /// VCR play (and cache frames if needed) sequence
    virtual void play( const Playback dir,
                       mrv::ViewerUI* const uiMain,
                       const bool fg );

    // Abort playback suddenly
    void abort(bool t) { _aborted = t; }

    // Is playback aborted?
    bool aborted() { return _aborted; }


    /// VCR stop sequence
    virtual void stop();

    inline Playback playback() const         { return _playback; }
    inline void playback( const Playback p ) { _playback = p; }

    inline bool stopped() const { return ( _playback == kStopped ); }

    /// Original play rate of movie
    inline double fps() const { return _fps; }

    /// Original play rate of movie
    inline void fps( double fps ) { _fps = fps; }

    /// Change real play rate of movie
    inline void real_fps( double fps ) { _real_fps = fps; }

    /// Return current actual play rate of movie
    inline double real_fps() const { return _real_fps; }

    /// Change play rate of movie
    inline void play_fps( double fps ) { _play_fps = fps; }

    /// Return current play rate of movie
    inline double play_fps() const { return _play_fps; }


    /// Change audio volume
    virtual void volume( float v );


    inline bool has_subtitle() const
    {
        return ( _subtitle_index >= 0 && 
                 _subtitle_info[ _subtitle_index ].has_codec );
    }

    const subtitle_info_t& subtitle_info( unsigned int i )
    {
        assert( i < _subtitle_info.size() );
        return _subtitle_info[ i ];
    }

    inline size_t number_of_subtitle_streams() const 
    { 
        return _subtitle_info.size(); 
    }

    inline bool has_audio_data() const
    {
        return ( _audio_index >= 0 && _audio_info[ _audio_index ].has_codec
                 && _audio_info[ _audio_index ].has_data( _frame ) );
    }

    inline bool has_audio() const
    {
        return ( _audio_index >= 0 && _audio_info[ _audio_index ].has_codec );
    }

    inline const audio_info_t& audio_info( unsigned int i ) const
    {
        assert( i < _audio_info.size() );
        return _audio_info[ i ];
    }

    inline const AVFormatContext* audio_context() const
    {
        if ( _audio_index < 0 ) return NULL;
        return _audio_info[ _audio_index ].context;
    }

    AudioEngine::AudioFormat audio_format() const { return _audio_format; }

    inline const std::string& audio_codec()  const 
    { 
        return _audio_info[ _audio_index ].codec_name;
    }

    inline unsigned audio_channels()  const
    {
        return _audio_info[ _audio_index ].channels;
    }

    inline unsigned audio_frequency() const
    {
        return _audio_info[ _audio_index ].frequency;
    }

    inline size_t number_of_audio_streams() const 
    { 
        return _audio_info.size(); 
    }

    virtual void subtitle_stream( int idx ) {};

    inline int subtitle_stream() const
    {
        return _subtitle_index;
    }

    void audio_stream( int idx );

    inline int audio_stream() const
    {
        return _audio_index;
    }


    static void video_cache_size( unsigned x ) { _video_cache_size = x; }

    static void audio_cache_size( unsigned x ) { _audio_cache_size = x; }

    void audio_file( const char* file = "" );
    std::string audio_file() const { return _audio_file; }

    audio_type_ptr get_audio_frame( const boost::int64_t f );  // const

    void close_audio();

    const char* const exif( const std::string& name ) const;

    void    wait_audio();
    bool    find_audio( const boost::int64_t frame);

    virtual boost::int64_t wait_subtitle() { return _frameStart; }
    virtual bool find_subtitle( const boost::int64_t frame );
    virtual void wait_image();
    virtual bool find_image( const boost::int64_t frame );


    void thread_exit();

    const Attributes& exif() const { return _exif; }
    const Attributes& iptc() const { return _iptc; }

    static void default_profile( const char* c );

    void flush_all();

    void seek_request( bool b ) { _seek_req = b; }
    bool seek_request()         { return _seek_req; }

    // Convert a frame into stream's pts
    boost::uint64_t frame2pts( const AVStream* stream,
			       const boost::int64_t frame ) const;

    virtual void do_seek();

    DecodeStatus decode_audio( boost::int64_t& frame );
    DecodeStatus handle_video_seek( boost::int64_t& frame, 
                                    const bool is_seek = true );
    virtual DecodeStatus decode_video( boost::int64_t& frame );
    virtual DecodeStatus decode_subtitle( const boost::int64_t frame );


    float eye_separation() const { return _eye_separation; }
    void eye_separation(float b) { _eye_separation = b; refresh(); }

    Barrier* loop_barrier()       { return _loop_barrier; }
    Mutex& decode_mutex()         { return _decode_mutex; }
    Mutex& video_mutex()          { return _mutex; };

    virtual void clear_packets();

    /** 
     * Given a frame number, returns whether audio for that frame is already
     * cached in store
     * 
     * @param frame  frame to check 
     * 
     * @return true or false
     */
    bool in_audio_store( const boost::int64_t frame );


    void debug_audio_packets(const boost::int64_t frame, 
			     const char* routine = "",
			     const bool detail = true);
    virtual void debug_video_packets(const boost::int64_t frame, 
				     const char* routine = "",
				     const bool detail = false);
    virtual void debug_video_stores(const boost::int64_t frame, 
				    const char* routine = "",
				    const bool detail = false) {};
    virtual void debug_subtitle_stores(const boost::int64_t frame, 
				       const char* routine = "",
				       const bool detail = false) {};

    virtual void probe_size( unsigned p ) {}
    inline mrv::AudioEngine* audio_engine() const { return _audio_engine; }

    CMedia* left_eye() { return this; }

    void right_eye( CMedia* c ) { _right_eye = c; refresh(); }
    CMedia* right_eye() const { return _right_eye; }

    bool is_left_eye() const { return _is_left_eye; }
    void is_left_eye( bool left ) { _is_left_eye = left; }

    std::string sequence_filename( const boost::int64_t frame );

    double video_clock() const { return _video_clock; }
    
    double audio_clock() const { return _audio_clock; }
    
    virtual AVStream* get_video_stream() const { return NULL; } ;
    virtual AVStream* get_subtitle_stream() const { return NULL; } ;

    double video_pts() const { return _video_pts; }
    double audio_pts() const { return _audio_pts; }

    const GLShapeList& shapes() const { return _shapes; }
    GLShapeList& shapes() { return _shapes; }

    const GLShapeList& undo_shapes() const { return _undo_shapes; }

    GLShapeList& undo_shapes() { return _undo_shapes; }
    void add_shape( shape_type_ptr s );

    void fetch_audio( const boost::int64_t frame );
    void wait_for_load_threads();

    void wait_for_threads();

    void audio_offset( const boost::int64_t& f ) { _audio_offset = f; }
    boost::int64_t audio_offset() const { return _audio_offset; }

    void debug_audio_stores(const boost::int64_t frame,
			    const char* routine = "",
			    const bool detail = true);

    static bool supports_yuv()         { return _supports_yuv; }
    static void supports_yuv( bool x ) { _supports_yuv = x; }

    static void eight_bit_caches( bool x ) { _8bit_cache = x; }
    static bool eight_bit_caches() { return _8bit_cache; }

    static void preload_cache( bool x ) { _preload_cache = x; }
    static bool preload_cache() { return _preload_cache; }

    static void cache_active( bool x ) { _cache_active = x; }
    static bool cache_active() { return _cache_active; }

    static void cache_scale( int x ) { _cache_scale = x; }
    static int  cache_scale() { return _cache_scale; }

       
    static std::string rendering_transform_8bits;
    static std::string rendering_transform_16bits;
    static std::string rendering_transform_32bits;
    static std::string rendering_transform_float;

    static std::string icc_profile_8bits;
    static std::string icc_profile_16bits;
    static std::string icc_profile_32bits;
    static std::string icc_profile_float;

  public:
    AV_SYNC_TYPE av_sync_type;
    Clock audclk;
    Clock vidclk;
    Clock extclk;

  protected:

    /** 
     * Given a frame number, returns whether subtitle for that frame is already
     * in packet queue.
     * 
     * @param frame  frame to check 
     * 
     * @return true or false
     */
    bool in_subtitle_packets( const boost::int64_t frame );

    /** 
     * Given a frame number, returns whether video for that frame is already
     * in packet queue.
     * 
     * @param frame  frame to check 
     * 
     * @return true or false
     */
    bool in_video_packets( const boost::int64_t frame );


    /** 
     * Handles skipping a bunch of packets due to a seek or play in reverse.
     * 
     * @param frame       current frame being played (changed is is_seek is true)
     * @param is_seek     set to true to handle a seek, false for preroll for
     *                    playback in reverse
     * 
     * @return true if packets could be skipped, false if not.
     */
    DecodeStatus handle_audio_packet_seek( boost::int64_t& frame, 
					   const bool is_seek );


    /** 
     * Given a frame number, returns whether audio for that frame is already
     * in packet queue.
     * 
     * @param frame  frame to check 
     * 
     * @return true or false
     */
    bool in_audio_packets( const boost::int64_t frame );

    /** 
     * Given an audio packet, decode it into internal buffer
     * 
     * @param ptsframe   frame of packet
     * @param frame      frame to use if packet has no frame
     * @param pkt        audio packet
     * 
     * @return decode status for packet
     */
    DecodeStatus decode_audio_packet( boost::int64_t& ptsframe, 
                                      const boost::int64_t frame, 
				      const AVPacket& pkt );

    /** 
     * Given an audio packet, try to decode the audio into one or more
     * audio frames
     * 
     * @param frame       frame to decode packet for
     * @param pkt         Audio packet 
     * 
     * @return true if we get audio for current frame
     */
    DecodeStatus decode_audio( const boost::int64_t frame, const AVPacket& pkt );



    void debug_stream_keyframes( const AVStream* stream );
    void debug_stream_index( const AVStream* stream );

    CMedia();

    static CMedia* get(CMedia* (*create)(),
		       const char* name, const boost::uint8_t* datas = 0) {
        CMedia* img = (CMedia*)create();
        return (CMedia*) img;
    }

    /// Initialize format's global resources
    virtual bool initialize() { return true; };

    /// Release format's global resources
    virtual bool release() { return true; };

    /// Get and store the timestamp for a frame in sequence
    void timestamp( boost::uint64_t idx );

    /// Get time stamp of file on disk
    void timestamp();

    /// Allocate hires image pixels
    void allocate_pixels( const boost::int64_t& frame,
			  const unsigned short channels = 4,
			  const image_type::Format format = image_type::kRGBA,
			  const image_type::PixelType pixel_type = image_type::kFloat ); 


    unsigned int audio_bytes_per_frame();

    void audio_initialize();
    void audio_shutdown();



    // Extract frame from pts or dts
    boost::int64_t get_frame( const AVStream* s, const AVPacket& pkt );

    // Convert an FFMPEG pts into a frame number
    boost::int64_t pts2frame( const AVStream* stream, 
			      const boost::int64_t pts ) const;

    void open_audio_codec();
    void close_audio_codec();
    bool open_audio( const short channels, const unsigned samples );
    bool play_audio( const mrv::audio_type_ptr& result );

    int audio_stream_index() const;
    AVStream* get_audio_stream() const;

    void populate_audio();

    void populate_stream_info( StreamInfo& s, 
			       std::ostringstream& msg,
			       const AVFormatContext* context,
			       const AVCodecContext* ctx, 
			       const int stream_index );

    void limit_audio_store( const boost::int64_t frame );
    void clear_stores();
    // Return the maximum number of audio frames cached for jog/shuttle
    unsigned int max_audio_frames();
    // Return the maximum number of video frames cached for jog/shuttle
    unsigned int max_video_frames();

    /** 
     * Store an audio frame in cache
     * 
     * @param audio_frame  audio frame to store 
     * @param buf          audio data
     * @param samples      number of audio samples
     * 
     * @return number of bytes stored
     */
    unsigned int store_audio( const boost::int64_t audio_frame, 
			      const boost::uint8_t* buf, const unsigned int size );


    virtual bool seek_to_position( const boost::int64_t frame );


    virtual void flush_video() {};
    void flush_audio();

    void dump_metadata( AVDictionary* m, const std::string prefix = "" );

    // Auxiliary function to handle decoding audio in messy new api.
    int decode_audio3(AVCodecContext* avctx, int16_t* samples,
                      int* frame_size_ptr, AVPacket* avpkt);



  protected:

    boost::int64_t queue_packets( const boost::int64_t frame,
                                  const bool is_seek,
                                  bool& got_video,
                                  bool& got_audio,
                                  bool& got_subtitle );

    static const char* stream_type( const AVCodecContext* );

    static std::string codec_tag2fourcc( unsigned int );
    static std::string codec_name( const AVCodecContext* enc );
    static unsigned int calculate_bitrate( const AVCodecContext* enc );
    static double calculate_fps( const AVStream* stream );

  protected:
    static unsigned  _audio_max;        //!< max size of audio buf
    static bool _supports_yuv;          //!< display supports yuv

    static unsigned _video_cache_size;
    static unsigned _audio_cache_size;


    unsigned int  _w, _h;     //!< width and height of image
    bool   _internal;      //!< image is internal with no filename
    bool   _is_sequence;      //!< true if a sequence
    bool   _is_stereo;        //!< true if part of stereo pair of images
    StereoType   _stereo_type;//!< Stereo type
    char*  _fileroot;         //!< root name of image sequence
    char*  _filename;         //!< generated filename of a frame
    time_t _ctime, _mtime;    //!< creation and modification time of image
    size_t _disk_space;       //!< disk space used by image

    Mutex     _mutex;          //!< to mark image routines
    Mutex     _subtitle_mutex; //!< to mark subtitle routines
    Mutex     _audio_mutex;    //!< to mark audio routines
    Mutex     _decode_mutex;   //!< to mark looping section routines

    AVFrame*   _a_frame;

    double    _avdiff;      //!< Audio-Video Difference
    Barrier*  _loop_barrier;   //!< Barrier used to sync loops across threads
    static Barrier*  _bg_barrier;     //!< Barrier to sync bg and fg images

    bool    _seek_req;        //!< set internally for seeking
    boost::int64_t _seek_frame;      //!< seek frame requested

    char*  _channel;          //!< current channel/layer being shown
    char*  _old_channel;      //!< previously shown channel/layer

    char*  _label;            //!< optional label drawn superimposed

    double  _real_fps;        //!< actual play rate of movie
    double  _play_fps;        //!< current desired play speed
    double  _fps;             //!< movie's original play speed

    double          _pixel_ratio;  //!< pixel ratio of image
    unsigned        _num_channels; //!< number of channels

    // mostly unused --- keep?
    RenderingIntent _rendering_intent;
    float     _gamma;
    bool                _has_chromaticities;
    Imf::Chromaticities _chromaticities;

    boost::int64_t   _dts;         //!< decoding time stamp (current fetch pkt)
    boost::int64_t   _audio_frame; //!< presentation time stamp (current audio)
    boost::int64_t   _audio_offset;//!< offset of additional audio
    int _audio_hw_buf_size;

    boost::int64_t   _frame;       //!< presentation time stamp (current video)
    boost::int64_t   _expected;    //!< expected next dts fetch
    boost::int64_t   _expected_audio; //!< expected next frame fetch

    boost::int64_t   _frameStart;  //!< start frame for sequence or movie
    boost::int64_t   _frameEnd;    //!< end frame for sequence or movie

    boost::int64_t   _frame_start;
    boost::int64_t   _frame_end;

    double     _audio_pts;
    double     _audio_clock;
    double     _video_pts;
    double     _video_clock;

    InterlaceType _interlaced;     //!< image is interlaced?

    Damage           _image_damage;     //!< flag specifying image damage
    mrv::Recti  _damageRectangle;  //!< rectangle that changed

    boost::uint64_t _numWindows;   //!< number of data/display windows
    mrv::Recti* _dataWindow;       //!< data window of sequence
    mrv::Recti* _displayWindow;    //!< display window of sequence
    mrv::Recti* _dataWindow2;       //!< data window of stereo sequence
    mrv::Recti* _displayWindow2;    //!< display window of stereo sequence

    // Hi-res/quality image (usually floating point)
    mrv::image_type_ptr _hires;
    mrv::image_type_ptr _stereo[2]; // stereo image
    mrv::image_type_ptr _subtitle;

    bool      _is_left_eye;
    CMedia*   _right_eye;

    float     _eye_separation;

    //
    std::string _audio_file;

    // Image color profile for ICC
    char*     _profile;

    // Rendering transform for CTL
    char*     _rendering_transform;

    // Look Mod Transform for CTL
    LMT   _look_mod_transform;

    // Input Device Transform for CTL
    char*     _idt_transform;

    unsigned int _frame_offset;      //!< hack to get ffmpeg to behave correctly

    Playback       _playback;        //!< playback direction or stopped
    bool        _aborted;
    
    thread_pool_t  _threads;         //!< any threads associated with process
    thread_pool_t  _load_threads;    //!< loading threads if any

    mrv::image_type_ptr* _sequence; //!< For sequences, holds each float frame
    mrv::image_type_ptr* _right;    //!< For stereo sequences, holds each 
                                    //!  right float frame
    ACES::ASC_CDL _sops;            //!< Slope,Offset,Pivot,Saturation
    ACES::ACESclipReader::GradeRefs _grade_refs; 

    stringArray  _layers;                //!< list of layers in file
    PixelBuffers _pixelBuffers;          //!< float pixel buffers
    LayerBuffers _layerBuffers;          //!< mapping of layer to pixel buf.

    Attributes _exif;                    //!< EXIF attributes
    Attributes _iptc;                    //!< IPTC attributes

    // Audio/Video
    AVFormatContext* _context;           //!< current read file context
    AVCodecContext* _video_ctx;          //!< current video context
    AVFormatContext* _acontext;          //!< current audio file context
    AVCodecContext* _audio_ctx;          //!< current video context

    // Drawings
    GLShapeList      _shapes;
    GLShapeList      _undo_shapes;

    PacketQueue      _video_packets;
    PacketQueue      _audio_packets;
    PacketQueue      _subtitle_packets;

    AVCodec*         _audio_codec;       //!< current audio codec for play

    int              _subtitle_index;
    subtitle_info_list_t _subtitle_info;   //!< list of subtitle stream infos

    int               _audio_index;   //!< current audio active stream
    audio_info_list_t _audio_info;   //!< list of audio stream infos
    unsigned         _samples_per_sec;   //!< last samples per sec
    audio_cache_t    _audio;
    unsigned         _audio_buf_used;    //!< amount used of reading cache
    boost::int64_t   _audio_last_frame;  //!< last audio frame decoded
    unsigned short   _audio_channels;
    AVFrame*         _aframe;   //!< audio ffmpeg frame
    boost::int64_t   next_pts;
    AVRational       next_pts_tb;
    int64_t          audio_callback_time;

    mrv::AudioEngine::AudioFormat _audio_format;
    mrv::aligned16_uint8_t*  _audio_buf; //!< temporary audio reading cache (aligned16)


    SwrContext* forw_ctx;
    mrv::AudioEngine*  _audio_engine;

    static bool _8bit_cache;
    static bool _cache_active;
    static bool _preload_cache;
    static int  _cache_scale;
};


void verify_stereo_resolution( const CMedia* const image,
                               const CMedia* const right );
uint64_t get_valid_channel_layout(uint64_t channel_layout, int channels);
char *const get_error_text(const int error);


//   typedef boost::shared_ptr< CMedia > Image_ptr;
typedef CMedia*                     Image_ptr;
typedef std::vector< Image_ptr >       ImageList;

}  // namespace mrv



#endif // CMedia_h

