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

#include "core/mrvFrame.h"


#include <ctime>

#include <set>
#include <map>
#include <vector>
#include <string>
#include <deque>
#include <limits>
#include <atomic>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <ImfAttribute.h>
#include <ImfChromaticities.h>
#include <ImfTimeCode.h>
#include <ACESclipReader.h>

#include "core/mrvString.h"
#include "core/mrvPacketQueue.h"
#include "core/mrvImagePixel.h"
#include "core/mrvRectangle.h"
#include "core/mrvAudioEngine.h"
#include "core/mrvOS.h"
#include "core/mrvBarrier.h"
#include "core/mrvACES.h"
#include "core/mrvI8N.h"

#include "gui/mrvIO.h"

#include "video/mrvGLShape.h"

#undef min
#undef max

struct Clock {
    Clock() : pts( 0 ),
        pts_drift( 0 ),
        last_updated( 0 ),
        speed(1),
        serial(0),
        paused(0),
        queue_serial( NULL )
    {
    }

    std::atomic<double> pts;           /* clock base */
    std::atomic<double> pts_drift;     /* clock base minus time at which we updated the clock */
    std::atomic<double> last_updated;
    std::atomic<double> speed;
    int serial;           /* clock is based on a packet with this serial */
    int paused;
    int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
};
struct AVFormatContext;
struct AVFrame;
struct AVCodec;
struct AVPacket;
struct AVStream;
struct AVCodecContext;
struct SwrContext;

// Turn this on to print out stats of decoding
// #define DEBUG_DECODE


class ViewerUI;

namespace mrv {

class AudioEngine;
class ImageOpts;

void update_video_pts(CMedia* is, double pts, int64_t pos, int serial);

class CMedia
{
public:
    typedef mrv::ImagePixel    Pixel;
    typedef  std::map< std::string, Imf::Attribute* > Attributes;
    typedef std::map< int64_t, Attributes > AttributesFrame;

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
        bool         play;
        bool         has_codec;
        bool         closed_captions;
        std::string  codec_name;
        std::string  fourcc;
        std::string  language;
        std::string  disposition;
        double       start;
        double       duration;

        StreamInfo() : context( NULL ),
                       stream_index(-1),
                       play( true ),
                       has_codec(false),
                       closed_captions( false ),
                       start(0),
                       duration(0)
        {
        }

        StreamInfo( const StreamInfo& b ) :
            context( b.context ),
            stream_index( b.stream_index ),
            play( b.play ),
            has_codec( b.has_codec ),
            closed_captions( b.closed_captions ),
            codec_name( b.codec_name ),
            fourcc( b.fourcc ),
            language( b.language ),
            disposition( b.disposition ),
            start( b.start ),
            duration( b.duration )
        {
        }

        inline bool has_data( const int64_t& frame, const int64_t offset,
                              const double& fps ) const
        {
            int64_t last = int64_t( ( duration + start ) * fps );
            last -= offset;
            return frame <= last;
        }

    };

    // Video stream data
    struct VideoStream : public StreamInfo
    {
        bool         has_b_frames;
        double       fps;
        std::string  pixel_format;
        size_t       width, height;

        VideoStream() :
            StreamInfo(),
            has_b_frames( false ),
            fps(0),
            width( 0 ),
            height( 0 )
        {
        }

        VideoStream( const VideoStream& b ) :
            StreamInfo( b ),
            has_b_frames( b.has_b_frames ),
            fps( b.fps ),
            pixel_format( b.pixel_format ),
            width( b.width ),
            height( b.height )
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

        AudioStream() : StreamInfo(), channels(0), frequency(0), bitrate(0)
        {
        }

        AudioStream( const AudioStream& b ) :
            StreamInfo( b ),
            channels( b.channels ),
            frequency( b.frequency ),
            bitrate( b.bitrate ),
            format( b.format )
        {
        }
    };
    typedef AudioStream audio_info_t;
    typedef std::vector< audio_info_t >  audio_info_list_t;


    // Subtitle stream data
    struct SubtitleStream : public StreamInfo
    {
        unsigned int bitrate;

        SubtitleStream() : StreamInfo(), bitrate(0)
        {
        }

        SubtitleStream( const SubtitleStream& b ) :
            StreamInfo( b ),
            bitrate( b.bitrate )
        {
        }
    };

    typedef SubtitleStream subtitle_info_t;
    typedef std::vector< subtitle_info_t >   subtitle_info_list_t;


    typedef std::deque< mrv::audio_type_ptr > audio_cache_t;

    struct TransformId
    {
        TransformId() :
            name(NULL), enabled( true ), applied( false ) {};
        TransformId(const char* n) :
            name( av_strdup(n) ), enabled( true ), applied( false ) {};
        TransformId(const TransformId& b) :
            name( av_strdup(b.name) ),
            enabled( b.enabled ),
            applied( b.applied ) {};
        TransformId& operator=(const TransformId& b)
            {
                name = av_strdup(b.name);
                enabled = b.enabled; applied = b.applied;
                return *this;
            }
        ~TransformId() { av_free( name ); }
        char* name;
        bool enabled;
        bool applied;
    };

    typedef std::vector< TransformId >          LMT;
    typedef std::vector< boost::thread* > thread_pool_t;

    enum Looping
    {
        kNoLoop,
        kLoop,
        kPingPong,
        kUnknownLoop
    };

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
        //kScrubbing =  4,
        kSaving    =  16,
    };

    enum Damage {
        kNoDamage        = 0,
        kDamageLayers    = 1 << 0, // 1
        kDamageContents  = 1 << 1, // 2
        kDamageThumbnail = 1 << 2, // 4
        kDamageSubtitle  = 1 << 3, // 8
        kDamageData      = 1 << 4, // 16
        kDamageLut       = 1 << 5, // 32
        kDamage3DData    = 1 << 6, // 64
        kDamageCache     = 1 << 7, // 128
        kDamageTimecode  = 1 << 8, // 256
        kDamageICS       = 1 << 9, // 512
        kDamageAll       = ( kDamageLayers | kDamageContents | kDamageLut |
                             kDamageThumbnail | kDamageData | kDamageSubtitle |
                             kDamage3DData | kDamageCache | kDamageTimecode |
                             kDamageICS )
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
        kDecodeBufferFull = 8,
    };

    enum StereoInput {
        kNoStereoInput = 0,
        kSeparateLayersInput = 1,
        kTopBottomStereoInput = 2,
        kLeftRightStereoInput = 4,
    };

    enum StereoOutput {
        kNoStereo = 0,
        kStereoLeft      = 1,
        kStereoRight     = 2,
        kStereoOpenGL    = 4,
        // these top/bottom bottom/top are reversed on purpose
        kStereoBottomTop = 8,
        kStereoTopBottom = kStereoBottomTop + kStereoRight,
        kStereoSideBySide = 16,
        kStereoCrossed    = kStereoSideBySide + kStereoRight,
        kStereoInterlaced = 32,
        kStereoInterlacedColumns = kStereoInterlaced + 64,
        kStereoCheckerboard = kStereoInterlaced + 128,
        kStereoAnaglyph   = 256,
        kStereoRightAnaglyph = kStereoAnaglyph + kStereoRight,
    };

    enum Cache
    {
        kNoCache = 0,
        kInvalidFrame = 1,
        kLeftCache = 2,
        kStereoCache = 3,
    };

    enum StreamType
    {
        kVideoStream,
        kAudioStream,
        kSubtitleStream
    };

public:

    virtual DecodeStatus decode_eof( int64_t frame ) { return kDecodeOK; }


    inline void dissolve( float f )
        {
            _dissolve = f;
        }

    inline float dissolve() const
        {
            return _dissolve;
        }





    /// Fetch (load) the image for a frame
    virtual bool fetch(mrv::image_type_ptr& canvas, const int64_t frame) {
        return true;
    }

    bool refetch( const int64_t frame );
    bool refetch();

    inline void actual_frame_rate( double fps ) { _actual_frame_rate = fps; }
    inline double actual_frame_rate() const { return _actual_frame_rate; }

    /// Constructor used to create a resized image from another image.
    CMedia( const CMedia* other, int nw, int nh );

    /// Constructor used to split a video or sequence in two
    //  (cut_frame - last_frame)
    CMedia( const CMedia* other, int64_t cut_frame );


    virtual ~CMedia();


    /// Save the image under a new filename, with options opts
    bool save( const char* filename, const ImageOpts* const opts ) const;

  void pixel_ratio( int64_t f, double p );


  /// Set the image pixel ratio
  inline void  pixel_ratio( double f ) {
      pixel_ratio( _frame, f );
  }

    /////////////////// Set the image size, allocating a 4-float buffer
    void image_size( size_t w, size_t h );

    /////////////// Set to true if image is internal and no filename is used
    inline bool internal() const {
        return _internal;
    }
    inline const void internal(bool t) {
        _internal = t;
    }

    ///
    virtual void channel( const char* c );
    virtual const char* channel() const {
        return _channel;
    };


    // Add default Color, Red, Green, Blue
    void rgb_layers();

    // Add Lumma layer
    void lumma_layers();

    // Add alpha layers (Alpha and, optionally, Overlay)
    void alpha_layers();

    // Add default Color, Red, Green, Blue, Alpha, Overlay, Lumma layers
    void default_layers();



    const mrv::Recti data_window( int64_t f = AV_NOPTS_VALUE) const;
    const mrv::Recti display_window( int64_t f = AV_NOPTS_VALUE) const;
    const mrv::Recti data_window2(int64_t f = AV_NOPTS_VALUE) const;
    const mrv::Recti display_window2(int64_t f = AV_NOPTS_VALUE) const;

    void data_window( const int xmin, const int ymin,
                      const int xmax, const int ymax,
                      const int64_t& frame );
    void display_window( const int xmin, const int ymin,
                         const int xmax, const int ymax,
                         const int64_t& frame );
    void data_window2( const int xmin, const int ymin,
                       const int xmax, const int ymax,
                       const int64_t& frame );
    void display_window2( const int xmin, const int ymin,
                          const int xmax, const int ymax,
                          const int64_t& frame );

    ////////////////// Return the list of available layers in the image
    inline const stringArray& layers() const {
        return _layers;
    }

    ////////////////// Return the list of available layers in the image
    inline stringArray& layers() {
        return _layers;
    }

    ////////////////// Return the current damage area in the image
    inline const mrv::Recti& damage_rectangle() const
    {
        return _damageRectangle;
    }

    ////////////////// Mark an image rectangle for refreshing
    void refresh( const mrv::Recti& r );

    ////////////////// Mark the whole image for refreshing
    void refresh();

    ////////////////// Return if the image is damaged
    inline Damage image_damage() const {
        return _image_damage;
    };

    ////////////////// Set the image damage (for update)
    inline void  image_damage( int x )
    {
        _image_damage = (Damage) x;
    };


    /// Return the name of the image (sans directory)
    std::string name() const;

    /// Return the directory of the image
    std::string directory() const;

    ////////////////// Return the image filename expression
    inline const char* fileroot() const {
        if ( !_fileroot ) return filename();
        return _fileroot;
    };

    std::string name_prefix() const;
    std::string name_suffix() const;
    int         frame_padding() const;

    ////////////////// Return the image filename for current frame
    const char* const filename() const;

    ////////////////// Set the image filename (can be a C/C++
    ////////////////// sprintf expression like image.%d.exr) to handle
    ////////////////// sequences.
    void filename( const char* n );

    virtual void sequence( const char* fileroot,
                           const int64_t start,
                           const int64_t end,
                           const bool use_threads );

    ////////////////// Check if image has changed on disk or network
    virtual bool has_changed();

    // Clear the sequence 8-bit cache
    virtual void clear_cache();

    // Notify barrier to end playback
    void notify_barriers();

    // Clear one frame of the sequence 8-bit cache
    void update_frame( const int64_t& frame );

    inline bool has_sequence() const {
        return (_sequence != NULL);
    }

    // Returns true if cache for the frame is already filled, false if not
    virtual Cache is_cache_filled(int64_t frame);

    // For sequences, returns true if cache is all filled, false if not
    // For videos, returns false always
    bool is_cache_full();

    // For sequences, returns the first empty cache frame if any.
    // For videos, returns current frame.
    int64_t first_cache_empty_frame();

    // Store a frame in sequence cache
    void cache( mrv::image_type_ptr& pic );

    // Return a frame from cache
    mrv::image_type_ptr cache( int64_t frame ) const;

    inline PacketQueue& video_packets() {
        return _video_packets;
    }
    inline PacketQueue& audio_packets() {
        return _audio_packets;
    }
    inline PacketQueue& subtitle_packets() {
        return _subtitle_packets;
    }

    inline uint64_t duration() const {
        return _frameEnd - _frameStart + 1;
    }

    inline void avdiff( const double x ) {
        _avdiff = x;
    }
    inline double avdiff() const {
        return _avdiff;
    }

    ////////////////// Return the hi-res image
    inline mrv::image_type_ptr hires() const {
        Mutex& mtx = const_cast< Mutex& >( _mutex );
        SCOPED_LOCK( mtx );
        return _hires;
    }
    inline mrv::image_type_ptr hires()       {
        SCOPED_LOCK( _mutex );
        return _hires;
    }
    void hires( const mrv::image_type_ptr pic);

    inline void is_stereo( bool x ) {
        _is_stereo = x;
    }
    inline bool  is_stereo() const {
        return _is_stereo;
    }

    inline void stereo_input( StereoInput x ) {
        _stereo_input = x;
        refresh();
    }
    inline StereoInput stereo_input() const {
        return _stereo_input;
    }

    void stereo_output( StereoOutput x );
    inline StereoOutput stereo_output() const {
        return _stereo_output;
    }

    mrv::image_type_ptr left() const;

    mrv::image_type_ptr right() const;

    ////////////////// Return the 8-bits subtitle image
    inline mrv::image_type_ptr subtitle() const {
        return _subtitle;
    }
    inline mrv::image_type_ptr subtitle()       {
        return _subtitle;
    }

    ////////////////// Set the label for the current image
    inline const char* label() const {
        return _label;
    }
    inline void label( const char* x ) {
        _label = av_strdup(x);
    }

    ////////////////// Set the frame for the current image (sequence)
    virtual bool    frame( const int64_t frame );
    inline int64_t frame() const {
        return _frame.load();
    }

    ///////////////// Decoding time stamp
    inline int64_t   dts()                      {
        return _dts;
    }

    inline void      dts( const int64_t frame ) { _dts = frame; _expected = _dts + 1; _expected_audio = _expected + _audio_offset; }

    inline int64_t expected() const {
        return _expected;
    }

    ///////////////// Audio frame
    inline void audio_frame( const int64_t f ) {
        _audio_frame = f;
    }
    inline int64_t   audio_frame() const {
        return _audio_frame;
    }

    inline aligned16_uint8_t* audio_buffer() const {
        return _audio_buf;
    }


    ////////////////// Seek to a specific frame in current image.
    ////////////////// Frame is local to the video/sequence, not timeline.
    void seek( const int64_t frame );

    ////////////////// Pre-roll sequence for playback
    ////////////////// Frame is local to the video/sequence, not timeline.
    virtual void preroll( const int64_t frame );

    ////////////////// Add a loop to packet lists
    ////////////////// Frame is local to the video/sequence, not timeline.
    virtual void loop_at_start( const int64_t frame );
    virtual void loop_at_end( const int64_t frame );

    ////////////////// Static functions
    // Main entry function for guessing an image/movie type.
    // Defined in guessImage.cpp
    static CMedia* guess_image( const char* name,
                                const boost::uint8_t* datas = NULL,
                                const int size = 0,
                                const bool is_thumbnail = false,
                                int64_t first = AV_NOPTS_VALUE,
                                int64_t   end = AV_NOPTS_VALUE,
                                const bool avoid_seq = false );

    ////////////////////////
    // Image information
    ////////////////////////

    inline void width( size_t x )  {
        _w = x;
    }
    inline void height( size_t y ) {
        _h = y;
    }

    /// Return the image width
    inline size_t  width() const  {
        return _w;
    }

    /// Return the image height
    inline size_t  height() const {
        return _h;
    }



  /// Return the image pixel ratio
  double pixel_ratio() const;

    /// Returns the file format of the image
    virtual const char* const format() const {
        return _("Unknown");
    };

    /// Set the image compression
    virtual void compression( const unsigned idx ) { };

    /// Returns the image compression (if any)
    virtual const char* const compression() const {
        return _("None");
    };

    /// Returns the video's 4-letter codec identifier
    virtual const char* const fourcc() const {
        return "";
    }

    /// Returns the valid compressions for this format
    virtual stringArray valid_compressions() const {
        return stringArray();
    }

    /// Returns the image line order (if any)
    virtual const char* const line_order() const {
        return _("Unknown");
    };

    void rendering_intent( RenderingIntent r ) {
        _rendering_intent = r;
    }

    RenderingIntent rendering_intent() const {
        return _rendering_intent;
    }

    //   virtual const char* const rendering_intent() const;

    /// Returns the image original colorspace (RGB, YUV, CMYK, etc)
    virtual const char* const colorspace() const {
        return "RGB";
    };

    virtual size_t const colorspace_index() const {
        return 2;
    }
    virtual void colorspace_index( int x ) {
        _colorspace_index = x;
    }

    /// Returns the disk space used by image or movie (in bytes)
    inline size_t disk_space() const {
        return _disk_space;
    }

    /// Returns the size in memory of image or sequence (in bytes)
    size_t memory() const;


    // Returns the creation date as a string
    virtual const std::string creation_date() const;

    /// Returns image ICC color profile if present or NULL
    const char* icc_profile() const;

    /// Assigns a new color profile to the image or NULL to clear it
    void icc_profile( const char* cfile );

    /// Returns rendering transform name ( CTL script )
    const char* rendering_transform() const;

    /// Assigns a new rendering transform ( CTL script ) or NULL to remove it
    void rendering_transform( const char* cfile );

    // Adds default OCIO, ICC, and CTL profiles
    void default_color_corrections();

    /// Add default OCIO Input Color Space for this image bit depth.
    void default_ocio_input_color_space();

    /// Add default rendering transform (CTL script) for this image bit depth.
    void default_rendering_transform();

    /// Add default icc profile for this image bit depth.
    void default_icc_profile();

    inline std::string ocio_input_color_space() const {
        return _input_color_space;
    }

    // Assign an OCIO input color space or "" for clearing it.
    void ocio_input_color_space( const std::string& n );


    /// Returns input device transform id ( CTL script and attributes )
    const TransformId& idt_transform_id() const {return _idt_transform;}

    /// Returns input device transform id ( CTL script and attributes )
    TransformId& idt_transform_id() {return _idt_transform;}

    /// Returns input device transform name ( CTL script )
    const char* idt_transform() const;

    /// Assigns a new input device transform ( CTL script ) or NULL to clear it
    void idt_transform( const char* cfile );



    /// Returns inverse output transform name ( CTL script and attributes )
    TransformId& inverse_ot_transform_id() { return _inverse_ot_transform; }

    /// Returns inverse output transform name ( CTL script )
    const char* inverse_ot_transform() const;

    /// Assigns a new inverse output transform ( CTL script ) or NULL to clear it
    void inverse_ot_transform( const char* cfile );

    /// Returns inverse output transform name ( CTL script and attributes )
    TransformId& inverse_odt_transform_id() { return _inverse_odt_transform; }

    /// Returns inverse output device transform name ( CTL script )
    const char* inverse_odt_transform() const;

    /// Assigns a new inverse output device transform ( CTL script ) or NULL to clear it
    void inverse_odt_transform( const char* cfile );

    /// Returns inverse output transform name ( CTL script and attributes )
    TransformId& inverse_rrt_transform_id() { return _inverse_rrt_transform; }

    /// Returns inverse reference rendering transform name ( CTL script )
    const char* inverse_rrt_transform() const;

    /// Assigns a new inverse reference rendering transform ( CTL script ) or NULL to clear it
    void inverse_rrt_transform( const char* cfile );

    /// Returns the number of Look Mod Transforms ( CTL scripts )
    inline size_t number_of_lmts() const {
        return _look_mod_transform.size();
    }

    /// Returns look mod transform name ( CTL script and attributes ) or
    /// an exception if index is out of range
    const TransformId& look_mod_transform_id( const size_t idx ) const {
        if ( idx >= number_of_lmts() )
            throw std::runtime_error("idx > number of look mod transforms" );
        return _look_mod_transform[idx];
    }

    /// Returns look mod transform name ( CTL script and attributes ) or
    /// an exception if index is out of range
    TransformId& look_mod_transform_id( const size_t idx ) {
        if ( idx >= number_of_lmts() )
            throw std::runtime_error("idx > number of look mod transforms" );
        return _look_mod_transform[idx];
    }

    /// Returns look mod transform name ( CTL script ) or NULL if not present
    const char* look_mod_transform( const size_t idx ) const;

    /// Clears all the look mod transforms.
    void clear_look_mod_transform();

    /// Appends a new look mod transform ( CTL script )
    /// cfile cannot be NULL.
    void append_look_mod_transform( const char* cfile );

    /// Assigns a new look mod transform to a certain index ( CTL script )
    /// and moves all other indexes down.
    /// If cfile is NULL, the look mod at the index is removed.
    void insert_look_mod_transform( const size_t idx, const char* cfile );

    /// Assigns a new look mod transform to a certain index ( CTL script )
    /// If cfile is NULL, the look mod at the index is removed.
    void look_mod_transform( const size_t idx, const char* cfile );

    void asc_cdl( const ACES::ASC_CDL& o )
    {
        _sops = o;
    }

    size_t number_of_grade_refs() const;


    inline const ACES::ASC_CDL& asc_cdl() const {
        return _sops;
    }
    inline ACES::ASC_CDL& asc_cdl() {
        return _sops;
    }

    /// True if image represents a sequence
    inline bool is_sequence() const {
        return _is_sequence;
    }

    virtual bool has_picture() const {
        return true;
    }

    /// True if image represents a video sequence
    virtual bool has_video() const {
        return false;
    }

    // Returns the video information for a certain (i) video stream
    virtual const video_info_t& video_info( unsigned int i ) const
    {
        throw "No video stream";
    }

    // Sets the video stream to a new index (x)
    virtual void video_stream( int x ) {}

    // Returns the video stream index or -1 if no video stream (images)
    virtual int video_stream() const {
        return -1;
    }

    // Returns the number of video streams (0 or more)
    virtual size_t number_of_video_streams() const {
        return 0;
    }

    /// Sets the first frame in the transition of image (< first_frame() )
    void in_frame( int64_t x ) { _frameIn = x; }

    inline int64_t in_frame() const {
        return _frameIn;
    }

    /// Sets the last frame in the transition of image (> last_frame() )
    void out_frame( int64_t x ) { _frameOut = x; }

    inline int64_t out_frame() const {
        return _frameOut;
    }

    /// Sets the first frame in the range of playback
    void  first_frame(int64_t x);

    /// Sets the last frame in the range of playback
    void  last_frame(int64_t x);

    /// Returns the first frame in the range of playback
    inline int64_t   first_frame() const {
        return _frameStart;
    }

    /// Returns the last frame in the range of playback
    inline int64_t   last_frame()  const {
        return _frameEnd;
    }

    /// Returns the first frame in the video or sequence
    inline void  start_frame( const int64_t& x ) {
        _frame_start = x;
    }

    /// Returns the last frame in the video or sequence
    inline void   end_frame( const int64_t& x ) {
        _frame_end = x;
    }

    /// Returns the first frame in the video or sequence
    inline int64_t   start_frame() const {
        return _frame_start;
    }

    /// Returns the last frame in the video or sequence
    inline int64_t   end_frame()  const {
        return _frame_end;
    }

    // Returns the video interlace type if any
    inline InterlaceType interlaced() const {
        return _interlaced;
    }

    /// Returns the number of channels in the image
    inline unsigned number_of_channels() const {
        return _num_channels;
    }

    /// Returns the pixel format of the image
    image_type::Format pixel_format() const;

    // Returns the pixel format of the image as a string
    const char* const pixel_format_name() const;

    /// Returns the pixel type of the image
    image_type::PixelType depth() const;

    /// Returns the pixel type of the image as a string
    std::string pixel_depth() const;

    /// Stores an embedded gamma value
    void gamma(const float g);

    /// Returns an embedded gamma value (if any)
    inline float gamma() const {
        return _gamma;
    }

    inline bool has_chromaticities() const {
        return _has_chromaticities;
    }

    inline const Imf::Chromaticities& chromaticities() const
    {
        return _chromaticities;
    }

    void chromaticities( const Imf::Chromaticities& c );

    /// Returns true if image has an alpha channel
    virtual bool  has_alpha() const {
        return true;
    }

    // Return creation time of image
    inline time_t ctime() const {
        return _ctime;
    }

    // Return modification time of image
    inline time_t mtime() const {
        return _mtime;
    }



    // return true if one valid video stream or sequence is found
    virtual bool valid_video() const;

    // return true if one valid audio stream is found
    bool valid_audio() const;

    // return true if one valid subtitle stream is found
    bool valid_subtitle() const;



    /// VCR play (and cache frames if needed) sequence
    void play( const Playback dir, ViewerUI* const uiMain,
               const bool fg );



    /// VCR stop sequence
    virtual void stop( const bool fg = true );

    inline Playback playback() const         {
        return _playback;
    }
    inline void playback( const Playback p ) {
        _playback = p;
    }

    inline bool stopped() const {
        return ( _playback == kStopped );
    }

    inline bool saving() const {
        return ( _playback == kSaving );
    }

    /// Original play rate of movie
    inline double fps() const {
        return _fps;
    }

    /// Original play rate of movie
    inline void fps( double fps ) {
        _fps = fps;
    }

    /// Change real play rate of movie
    inline void real_fps( double fps ) {
        _real_fps = fps;
    }

    /// Return current actual play rate of movie
    inline double real_fps() const {
        return _real_fps;
    }

    /// Change play rate of movie
    inline void play_fps( double fps ) {
        if ( fps < 1.0 ) fps = 1.0;
        _play_fps = fps;
    }

    /// Return current play rate of movie
    inline double play_fps() const {
        return _play_fps;
    }

    /// Return current play rate of movie
    inline double orig_fps() const {
        return _orig_fps;
    }


    /// Return otio play rate of movie
    inline double otio_fps() const {
        return _otio_fps;
    }

    inline void otio_fps( const double x ) {
        _otio_fps = x;
    }


    /// Change audio volume
    void volume( float v );
    float volume() const;

    inline bool has_subtitle() const
    {
        return ( _subtitle_index >= 0 &&
                 _subtitle_info[ _subtitle_index ].has_codec &&
                 _subtitle_info[ _subtitle_index ].play );
    }

    inline const subtitle_info_t& subtitle_info( unsigned int i )
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
                 && _audio_info[ _audio_index ].has_data( _frame,
                         _audio_offset,
                         _fps ) );
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

    inline AudioEngine::AudioFormat audio_format() const {
        return _audio_format;
    }

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

    virtual void audio_stream( int idx );

    inline int audio_stream() const
    {
        return _audio_index;
    }



    inline static AVRational swap( const AVRational& value )
    {
        return AVRational({ value.den, value.num });
    }

    inline static void image_cache_size( int x ) {
        _image_cache_size = x;
    }

    inline static void video_cache_size( int x ) {
        _video_cache_size = x;
    }

    inline static void audio_cache_size( int x ) {
        _audio_cache_size = x;
    }

    void audio_file( const char* file );
    std::string audio_file() const {
        return _audio_file;
    }

    audio_type_ptr get_audio_frame( const int64_t f );  // const

    void close_audio();

    void    wait_audio();
    bool    find_audio( const int64_t frame);

    virtual int64_t wait_subtitle() {
        return _frameStart;
    }
    virtual bool find_subtitle( const int64_t frame );
    virtual void wait_image();
    virtual bool find_image( const int64_t frame );


    Attributes& attributes();

    const Attributes& attributes() const;

    inline const Attributes& clip_attributes() const {
        return _clip_attrs;
    }

    inline const AttributesFrame attrs_frames() const {
        return _attrs;
    }

    static void default_profile( const char* c );

    void flush_all();

    inline void seek_request( bool b ) {
        _seek_req = b;
    }
    inline bool seek_request()         {
        return _seek_req;
    }

    // Convert a frame into stream's pts
    int64_t frame2pts( const AVStream* stream,
                       const int64_t frame ) const;

    virtual void do_seek();

    virtual DecodeStatus decode_audio( int64_t& frame );
    DecodeStatus handle_video_seek( int64_t& frame,
                                    const bool is_seek = true );
    virtual DecodeStatus decode_video( int64_t& frame );
    virtual DecodeStatus decode_subtitle( const int64_t frame );


    inline float eye_separation() const {
        return _eye_separation;
    }
    inline void eye_separation(float b) {
        _eye_separation = b;
        refresh();
    }

    inline Barrier* stereo_barrier()     {
        if (_right_eye && _is_stereo)
            return _right_eye->_stereo_barrier;
        return _stereo_barrier;
    }
    inline void fg_bg_barrier( Barrier* b ) {
        _fg_bg_barrier = b;
    }
    inline Barrier* fg_bg_barrier()         {
        return _fg_bg_barrier;
    }
    inline Barrier* loop_barrier()          {
        return _loop_barrier;
    }
    inline Mutex& data_mutex()             {
        return _data_mutex;
    };
    inline Mutex& video_mutex()             {
        return _mutex;
    };
    inline Mutex& audio_mutex()             {
        return _audio_mutex;
    };
    inline Mutex& subtitle_mutex()          {
        return _subtitle_mutex;
    };

    void clear_audio_packets();
    void clear_video_packets();
    void clear_packets();


    /**
     * Given a frame number, returns whether audio for that frame is already
     * cached in store
     *
     * @param frame  frame to check
     *
     * @return true or false
     */
    bool in_audio_store( const int64_t frame );


    void debug_audio_packets(const int64_t frame,
                             const char* routine = "",
                             const bool detail = true);
    virtual void debug_video_packets(const int64_t frame,
                                     const char* routine = "",
                                     const bool detail = false);
    virtual void debug_video_stores(const int64_t frame,
                                    const char* routine = "",
                                    const bool detail = false);
    virtual void debug_subtitle_stores(const int64_t frame,
                                       const char* routine = "",
                                       const bool detail = false) {};

    virtual void probe_size( unsigned p ) {}

    inline mrv::AudioEngine* audio_engine() const {
        return _audio_engine;
    }

    // Return this image as the left eye
    inline CMedia* left_eye() {
        return this;
    }

    inline bool owns_right_eye() const {
        return _owns_right_eye;
    }

    // Set an image as the right eye for stereo decoding
    inline void owns_right_eye( bool c ) {
        _owns_right_eye = c;
    }

    // Set an image as the right eye for stereo decoding
    inline void right_eye( CMedia* c ) {
        _right_eye = c;
        refresh();
    }

    // Return the image as the right eye for stereo decoding or NULL if none
    inline CMedia* right_eye() const {
        return _right_eye;
    }

    // Return if this image is the left eye on stereo decoding
    inline bool is_left_eye() const {
        return _is_left_eye;
    }

    // Set this image as the left (true) or right (false) eye on stereo decoding
    inline void is_left_eye( bool left ) {
        _is_left_eye = left;
    }

    inline Looping looping() const {
        return _looping;
    }
    inline void looping( Looping x ) {
        _looping = x;
    }

    // Return the sequence filename for frame 'frame'
    std::string sequence_filename( const int64_t frame ) const;

    // Return the video clock as a double
    double video_clock() const {
        return _video_clock;
    }
    // Return the audio clock as a double
    double audio_clock() const {
        return _audio_clock;
    }

    // Return the video stream being decoded or NULL if none
    virtual AVStream* get_video_stream() const {
        return NULL;
    } ;
    // Return the subtitlee stream being decoded or NULL if none
    virtual AVStream* get_subtitle_stream() const {
        return NULL;
    } ;

    // Return offset in timeline
    inline void position( int64_t x ) {
        _pos = x;
    }

    // Set offset in timeline
    inline int64_t position() const {
        return _pos;
    }

    // Return the video pts as a double
    inline double video_pts() const {
        return _video_pts;
    }

    // Return FFMPEG's start number of sequences (PNG loader)
    inline int64_t start_number() const {
        return _start_number;
    }

    // Return the audio pts as a double
    inline double audio_pts() const {
        return _audio_pts;
    }

    // Return the shape list
    inline const GLShapeList& shapes() const {
        return _shapes;
    }
    // Return the shape list
    inline GLShapeList& shapes() {
        return _shapes;
    }

    // Return the undo shape list
    inline const GLShapeList& undo_shapes() const {
        return _undo_shapes;
    }

    // Return the undo shape list
    inline GLShapeList& undo_shapes() {
        return _undo_shapes;
    }

    // Add a GL drawn shape to image
    void add_shape( shape_type_ptr s );

    // Return the maximum number of audio frames cached for jog/shuttle
    // or 0 for no cache or numeric_limits<unsigned>max() for full cache
    unsigned max_audio_frames();
    // Return the maximum number of video frames cached for jog/shuttle
    // or 0 for no cache or numeric_limits<unsigned>max() for full cache
    unsigned max_video_frames();
    // Return the maximum number of video frames cached for jog/shuttle
    // or 0 for no cache or numeric_limits<int>max() for full cache
    uint64_t max_image_frames();

    virtual void limit_video_store( const int64_t frame );

    inline void is_thumbnail(bool t) {
        _is_thumbnail = t;
    }
    inline bool is_thumbnail() const {
        return _is_thumbnail;
    }

    inline void    timecode( const int64_t& f ) {
        _tc_frame = 0;
    }

    inline int64_t timecode() const {
        return _tc_frame;
    }

    inline void x( double t ) {
        _x = t;
        if (_right_eye) _right_eye->x(t);
        refresh();
    }
    inline void y( double t ) {
        _y = t;
        if (_right_eye) _right_eye->y(t);
        refresh();
    }
    inline double x() const {
        return _x;
    }
    inline double y() const {
        return _y;
    }

    inline void rotate( float t ) {
        _rot_z += t;
    }
    inline void rot_z( float t ) {
        _rot_z = t;
    }
    inline double rot_z() const {
        return _rot_z;
    }

    inline void flipX(bool t) { _flipX = t; }
    inline void flipY(bool t) { _flipY = t; }

    inline bool flipX() const { return _flipX; }
    inline bool flipY() const { return _flipY; }

    inline void scale_x( double t ) {
        _scale_x = t;
    }
    inline double scale_x() const {
        return _scale_x;
    }

    inline void scale_y( double t ) {
        _scale_y = t;
    }
    inline double scale_y() const {
        return _scale_y;
    }


    // Process a timecode object unto _tc_frame
    void process_timecode( const Imf::TimeCode& tc );

    // Conver a string like HH:MM:SS:FF or HH;MM;SS;FF into Imf::TimeCode
    static Imf::TimeCode str2timecode( const std::string text );

    // Initialize audio engine
    void audio_initialize();

    void fetch_audio( const int64_t frame );


    // Wait for all threads to exit
    void wait_for_threads();

    inline void audio_offset( const int64_t f ) {
        _audio_offset = f;
    }
    inline int64_t audio_offset() const {
        return _audio_offset;
    }

    inline void audio_muted( const bool b ) { _audio_muted = b; }

    void notify_all()
        {
            _video_packets.cond().notify_all();
            _audio_packets.cond().notify_all();
            _subtitle_packets.cond().notify_all();
        }

    void debug_audio_stores(const int64_t frame,
                            const char* routine = "",
                            const bool detail = true);

    inline bool has_deep_data() const {
        return _has_deep_data;
    }

    static int from_stereo_input( StereoInput x );
    static StereoInput to_stereo_input( int x );

    static int from_stereo_output( StereoOutput x );
    static StereoOutput to_stereo_output( int x );

    static inline bool supports_yuv()         {
        return _supports_yuv;
    }
    static inline void supports_yuv( bool x ) {
        _supports_yuv = x;
    }

    static inline bool supports_yuva()         {
        return _supports_yuva;
    }
    static inline void supports_yuva( bool x ) {
        _supports_yuva = x;
    }

    static inline bool uses_16bits()         {
        return _uses_16bits;
    }
    static inline void uses_16bits( bool x ) {
        _uses_16bits = x;
    }

    static void default_subtitle_encoding( const char* f )
    {
        if (f) _default_subtitle_encoding = f;
    }

    static void default_subtitle_font( const char* f )
    {
        if (f) _default_subtitle_font = f;
    }

    static inline bool aces_metadata()         {
        return _aces_metadata;
    }
    static inline void aces_metadata( bool x ) {
        _aces_metadata = x;
    }

    static inline bool ocio_color_space()         {
        return _ocio_color_space;
    }
    static inline void ocio_color_space( bool x ) {
        _ocio_color_space = x;
    }

    static inline bool all_layers()         {
        return _all_layers;
    }
    static inline void all_layers( bool x ) {
        _all_layers = x;
    }

    static void eight_bit_caches( bool x ) {
        _8bit_cache = x;
    }
    static bool eight_bit_caches() {
        return _8bit_cache;
    }

    static void preload_cache( bool x ) {
        _preload_cache = x;
    }
    static bool preload_cache() {
        return _preload_cache;
    }

    static void cache_active( bool x ) {
        _cache_active = x;
    }
    static bool cache_active() {
        return _cache_active;
    }

    static void cache_scale( int x ) {
        _cache_scale = x;
    }
    static int  cache_scale() {
        return _cache_scale;
    }


    static int colorspace_override; //!< Override YUV Hint always with this

    static double default_fps;     //!< Default FPS when not selected
    static std::string ocio_8bits_ics;
    static std::string ocio_16bits_ics;
    static std::string ocio_32bits_ics;
    static std::string ocio_float_ics;

    static std::string rendering_transform_8bits;
    static std::string rendering_transform_16bits;
    static std::string rendering_transform_32bits;
    static std::string rendering_transform_float;

    static std::string icc_profile_8bits;
    static std::string icc_profile_16bits;
    static std::string icc_profile_32bits;
    static std::string icc_profile_float;

    static std::string attr2str( const Imf::Attribute* attr );


public:
    AV_SYNC_TYPE av_sync_type;
    Clock audclk;
    Clock vidclk;
    Clock extclk;

    bool _has_deep_data;
    enum LoadLib
    {
        kFFMPEGLibrary = 0,
        kOIIOLibrary = 1,
        kImageMagickLibrary = 2
    };
    static LoadLib load_library;
    static std::atomic<int64_t> memory_used;
    static double thumbnail_percent;

protected:


    /**
     * Given a frame number, returns another frame number taking into
     * account the loops in the sequence.
     *
     * @param f      frame in local units of image
     * @param frame  frame to handle in loops
     *
     * @return offset to bring local frame as returned by handle_loops
     *                to timeline frame
     */
    int64_t loops_offset( int64_t f,
                          const int64_t frame ) const;

    /**
     * Given a frame number, returns whether subtitle for that frame is already
     * in packet queue.
     *
     * @param frame  frame to check
     *
     * @return true or false
     */
    bool in_subtitle_packets( const int64_t frame );

    /**
     * Given a frame number, returns whether video for that frame is already
     * in packet queue.
     *
     * @param frame  frame to check
     *
     * @return true or false
     */
    bool in_video_packets( const int64_t frame );


    /**
     * Handles skipping a bunch of packets due to a seek or play in reverse.
     *
     * @param frame       current frame being played (changed is is_seek is true)
     * @param is_seek     set to true to handle a seek, false for preroll for
     *                    playback in reverse
     *
     * @return true if packets could be skipped, false if not.
     */
    DecodeStatus handle_audio_packet_seek( int64_t& frame,
                                           const bool is_seek );



    /**
     * Given a picture, scale it and convert it to 8 bits if user
     * preferences set it so.
     *
     * @param seq  sequence cache ( _sequence or _right ).
     * @param pic  picture to update
     *
     */
    void update_cache_pic( mrv::image_type_ptr*& seq,
                           const mrv::image_type_ptr& pic );

    /**
     * Given a frame number, returns whether audio for that frame is already
     * in packet queue.
     *
     * @param frame  frame to check
     *
     * @return true or false
     */
    bool in_audio_packets( const int64_t frame );

    // Given a frame and a packet, return got frame and a filled frame if
    // possible
    int decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame,
               AVPacket *pkt, bool& eof);

    /**
     * Given an audio packet, decode it into internal buffer
     *
     * @param ptsframe   frame of packet
     * @param frame      frame to use if packet has no frame
     * @param pkt        audio packet
     *
     * @return decode status for packet
     */
    DecodeStatus decode_audio_packet( int64_t& ptsframe,
                                      const int64_t frame,
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
    DecodeStatus decode_audio( const int64_t frame, const AVPacket& pkt );



    void debug_stream_keyframes( const AVStream* stream );
    void debug_stream_index( const AVStream* stream );

    CMedia();

    static CMedia* get(CMedia* (*create)(),
                       const char* name, const boost::uint8_t* datas = 0) {
        CMedia* img = (CMedia*)create();
        return (CMedia*) img;
    }

    /// Initialize format's global resources
    virtual bool initialize() {
        return true;
    };

    /// Release format's global resources
    virtual bool release() {
        return true;
    };

    /// Get and store the timestamp for a frame in sequence
    void timestamp( boost::uint64_t idx,
                    mrv::image_type_ptr*& seq );

    /// Get time stamp of file on disk
    void timestamp();

    /// Allocate hires image pixels
    bool allocate_pixels( mrv::image_type_ptr& canvas,
                          const int64_t& frame,
                          const unsigned short channels = 4,
                          const image_type::Format format = image_type::kRGBA,
                          const image_type::PixelType pixel_type = image_type::kFloat,
                          size_t w = 0, size_t h = 0);

    int audio_bytes_per_frame();

    void audio_shutdown();

    // Extract frame from pts or dts
    int64_t get_frame( const AVStream* s, const AVPacket& pkt );

    // Convert an FFMPEG pts into a frame number
    int64_t pts2frame( const AVStream* stream,
                       const int64_t pts ) const;

    void open_audio_codec();
    void close_audio_codec();
    bool open_audio( const short channels, const unsigned samples );
    bool play_audio( const mrv::audio_type_ptr result );

    int audio_stream_index() const;
    AVStream* get_audio_stream() const;

    void populate_audio();

    void populate_stream_info( StreamInfo& s,
                               std::ostringstream& msg,
                               const AVFormatContext* context,
                               const AVCodecParameters* codecpar,
                               const int stream_index );

    void timed_limit_audio_store( const int64_t frame );
    void limit_audio_store( const int64_t frame );
    void clear_stores();

    /**
     * Store an audio frame in cache
     *
     * @param audio_frame  audio frame to store
     * @param buf          audio data
     * @param samples      number of audio samples
     *
     * @return number of bytes stored
     */
    unsigned int store_audio( const int64_t audio_frame,
                              const uint8_t* buf, const unsigned int size );


    virtual bool seek_to_position( const int64_t frame );



    virtual void flush_video() {};
    void flush_audio();

    void dump_metadata( AVDictionary* m, const std::string prefix = "" );

    // Auxiliary function to handle decoding audio in messy new api.
    int decode_audio3(AVCodecContext* avctx, int16_t* samples,
                      int* frame_size_ptr, AVPacket* avpkt);




protected:

    void fill_rectangle( uint8_t* buf, int xh, int yh, int xl, int yl );

    int64_t queue_packets( const int64_t frame,
                           const bool is_seek,
                           bool& got_video,
                           bool& got_audio,
                           bool& got_subtitle );

    static const char* stream_type( const AVCodecContext* );
    static const char* stream_type( const AVCodecParameters* );

    static std::string codec_tag2fourcc( unsigned int );
    static std::string codec_name( const AVCodecParameters* enc );
    static std::string codec_name( const AVCodecContext* enc );
    static unsigned int calculate_bitrate( const AVStream* stream,
                                           const AVCodecParameters* enc );
    static double calculate_fps( AVFormatContext* ctx,
                                 AVStream* stream );

protected:
    static bool _supports_yuv;          //!< display supports yuv
    static bool _supports_yuva;         //!< display supports yuva
    static bool _uses_16bits;         //!< display supports 16 bits movies

    static int _image_cache_size;
    static int _video_cache_size;
    static int _audio_cache_size;

    thread_pool_t  _threads;         //!< any threads associated with process


    std::atomic<size_t>  _w, _h;     //!< width and height of image
    short   _cache_full;   // 0, 1 or 2
    bool   _internal;      //!< image is internal with no filename
    bool   _is_thumbnail;     //!< image is a thumbnail (no printing of errors)
    bool   _is_sequence;      //!< true if a sequence
    bool   _is_stereo;        //!< true if part of stereo pair of images
    StereoInput  _stereo_input; //!< Stereo input (feed)
    StereoOutput _stereo_output; //!< Stereo output display
    Looping      _looping;   //!< End behavior of playback (loop, stop, swing)
    char*  _fileroot;         //!< root name of image sequence
    char*  _filename;         //!< generated filename of a frame
    time_t _ctime, _mtime;    //!< creation and modification time of image
    size_t _disk_space;       //!< disk space used by image

    Mutex  _mutex;          //!< to mark image routines
    Mutex  _subtitle_mutex; //!< to mark subtitle routines
    Mutex  _audio_mutex;    //!< to mark audio routines
    Mutex  _data_mutex;     //!< to mark data routines (data/displaywindow)


    int _colorspace_index;    //!< YUV Hint for conversion

    std::atomic<double>    _avdiff;      //!< Audio-Video Difference
    Barrier*  _loop_barrier;   //!< Barrier used to sync loops across threads
    Barrier*  _stereo_barrier; //!< Barrier used to sync stereo threads
    Barrier*  _fg_bg_barrier;   //!< Barrier used to sync fg/bg threads

    bool    _audio_muted;     //!< to avoid opening audio file descriptor
    bool    _seek_req;        //!< set internally for seeking
    int64_t _seek_frame;      //!< seek frame requested
    std::atomic<int64_t> _pos;  //!< position offset in timeline

    char*  _channel;          //!< current channel/layer being shown

    char*  _label;            //!< optional label drawn superimposed

    double               _otio_fps;  //!< fps in source range of clip in otio
    std::atomic<double>  _real_fps;  //!< actual play rate of movie
                                     //!  (if frames are dropped)
    std::atomic<double>  _play_fps;  //!< current desired play speed
    std::atomic<double>  _fps;       //!< movie's original play speed
                                     //!  (set by user on sequences,
                                     //!   automatically set on movies)

    std::atomic<double> _orig_fps;   //!< movie's original play speed

    double*         _pixel_ratio;  //!< pixel ratio of image
    unsigned        _num_channels; //!< number of channels

    // mostly unused --- keep?
    RenderingIntent _rendering_intent;
    float     _gamma;
    float     _dissolve;
    bool                _has_chromaticities;
    Imf::Chromaticities _chromaticities;

    std::atomic<int64_t>   _dts;   //!< decoding time stamp (current fetch pkt)
    std::atomic<int64_t>   _adts;  //!< decoding time stamp of audio
    //   (current fetch pkt)
    std::atomic<int64_t> _audio_frame; //!< presentation time stamp (current audio)
    int64_t   _audio_offset;//!< offset of additional audio

    std::atomic<int64_t>   _frame; //!< presentation time stamp (current video)
    int64_t   _tc_frame;    //!< timecode frame offset
    std::atomic<int64_t>   _expected;    //!< expected next dts fetch
    std::atomic<int64_t>   _expected_audio; //!< expected next frame fetch

    int64_t   _frameStart;  //!< user start frame for sequence or movie
    int64_t   _frameEnd;    //!< user end frame for sequence or movie

    int64_t   _frameIn;     //!< start frame of transition (dissolve)
    int64_t   _frameOut;    //!< end frame of transition (dissolve)

    int64_t   _frame_start; //!< real start frame for sequence or movie
    int64_t   _frame_end;   //!< real end frame for sequence or movie

    int64_t   _start_number; //!< ffmpeg's start number offset

    int64_t   _loop_start;   //!< loop start when playing backwards
    int64_t   _loop_end;     //!< loop end when playing forwards

    std::atomic<double>      _audio_pts;
    std::atomic<double>      _audio_clock;
    std::atomic<double>     _video_pts;
    std::atomic<double>     _video_clock;

    InterlaceType _interlaced;     //!< image is interlaced?

    std::atomic<Damage> _image_damage;     //!< flag specifying image damage
    mrv::Recti  _damageRectangle;  //!< rectangle that changed

    double      _x, _y;             //!< x,y coordinates in canvas
    double      _scale_x, _scale_y; //!< x,y scale in canvas
    double      _rot_z;             //!< z quad rotation in canvas
    mrv::Recti* _dataWindow;        //!< data window of sequence
    mrv::Recti* _displayWindow;     //!< display window of sequence
    mrv::Recti* _dataWindow2;       //!< data window of stereo sequence
    mrv::Recti* _displayWindow2;    //!< display window of stereo sequence

    // Hi-res/quality image (usually floating point)
    mrv::image_type_ptr _hires;
    mrv::image_type_ptr _stereo[2]; // stereo image
    mrv::image_type_ptr _subtitle;  // subtitle image (when decoding VOBs)

    bool      _is_left_eye;         // is this image the left eye?
    CMedia*   _right_eye;           // Pointer to right class in stereo

    float     _eye_separation;      // eye separation when stereo is on

    // Audio file when different from video
    std::string _audio_file;
    size_t      _audio_max;        //!< max size of audio buf

    // Image color profile for ICC
    char*     _profile;

    // Rendering transform for CTL
    TransformId     _rendering_transform;

    // Look Mod Transform(s) for CTL
    LMT   _look_mod_transform;

    // Input Device Transform for CTL
    TransformId     _idt_transform;


    TransformId     _inverse_odt_transform;
    TransformId     _inverse_ot_transform;
    TransformId     _inverse_rrt_transform;

    // OCIO
    std::string _input_color_space;

    unsigned int _frame_offset;      //!< hack to get ffmpeg to behave correctly

    std::atomic<Playback> _playback;        //!< playback direction or stopped


    mrv::image_type_ptr* _sequence; //!< For sequences, holds each float frame
    mrv::image_type_ptr* _right;    //!< For stereo sequences, holds each
    //!  right float frame
    ACES::ASC_CDL _sops;            //!< Slope,Offset,Pivot,Saturation
    ACES::ACESclipReader::GradeRefs _grade_refs; //!< SOPS Nodes in ASCII


    std::atomic<double> _actual_frame_rate;
    image_type::PixelType _depth;

    stringArray  _layers;                //!< list of layers in file
    PixelBuffers _pixelBuffers;          //!< float pixel buffers
    LayerBuffers _layerBuffers;          //!< mapping of layer to pixel buf.

    Attributes      _clip_attrs;
    AttributesFrame _attrs;                    //!< All attributes

    // Audio/Video
    AVFormatContext* _context;           //!< current read file context
    AVCodecContext* _video_ctx;          //!< current video context
    AVFormatContext* _acontext;          //!< current audio read file context
    AVCodecContext* _audio_ctx;          //!< current audio context

    // Drawings
    GLShapeList      _shapes;
    GLShapeList      _undo_shapes;

    PacketQueue      _video_packets;
    PacketQueue      _audio_packets;
    PacketQueue      _subtitle_packets;

    AVCodec*         _audio_codec;       //!< current audio codec for play

    int              _subtitle_index;
    subtitle_info_list_t _subtitle_info;   //!< list of subtitle stream infos
    char*                _subtitle_encoding;
    char*                _subtitle_font;

    bool              _owns_right_eye;
    bool              _flipX;
    bool              _flipY;

    bool              _last_audio_cached;
    int               _audio_index;   //!< current audio active stream
    audio_info_list_t _audio_info;   //!< list of audio stream infos
    unsigned         _samples_per_sec;   //!< last samples per sec
    audio_cache_t    _audio;
    unsigned         frequency;
    int              _audio_buf_used;    //!< amount used of reading cache
    int64_t          _audio_last_frame;  //!< last audio frame decoded
    std::atomic<unsigned short>   _audio_channels;
    AVFrame*         _aframe;   //!< audio ffmpeg frame

    std::atomic<mrv::AudioEngine::AudioFormat> _audio_format;
    mrv::aligned16_uint8_t*  _audio_buf; //!< temporary audio reading cache (aligned16)
    float     _timeSinceLastFrame;
    int       _framesSinceLastFpsFrame;
    timeval    _lastFrameTime;
    timeval    _lastFpsFrameTime;
    SwrContext* forw_ctx;
    mrv::AudioEngine*  _audio_engine;

#if 1
    boost::thread* _video_thread;
    boost::thread* _audio_thread;
    boost::thread* _decode_thread;
#endif


    static std::string _default_subtitle_font;
    static std::string _default_subtitle_encoding;
    static bool _aces_metadata;
    static bool _ocio_color_space;
    static bool _all_layers;
    static bool _8bit_cache;
    static bool _cache_active;
    static bool _preload_cache;
    static int  _cache_scale;
    static bool _initialize;
};


void verify_stereo_resolution( const CMedia* const image,
                               const CMedia* const right );
uint64_t get_valid_channel_layout(uint64_t channel_layout, int channels);
const char* const get_error_text(const int error);

//   typedef boost::shared_ptr< CMedia > Image_ptr;
typedef CMedia*                     Image_ptr;
typedef std::vector< Image_ptr >       ImageList;

}  // namespace mrv



#endif // CMedia_h
