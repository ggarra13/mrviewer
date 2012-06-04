/**
 * @file   aviImage.h
 * @author gga
 * @date   Tue Jul 17 10:00:02 2007
 * 
 * @brief  A class to read movie and avi files
 * 
 * 
 */

#ifndef aviImage_h
#define aviImage_h

extern "C" {
#include "libswscale/swscale.h"
}

#include "CMedia.h"


namespace mrv {

  class aviImage : public CMedia 
  {
    aviImage();

    static CMedia* create() { return new aviImage(); }

  public:
    typedef std::vector< mrv::image_type_ptr > video_cache_t;
    typedef std::vector< mrv::image_type_ptr > subtitle_cache_t;

  public:
    static bool test(const boost::uint8_t *data, unsigned len);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual ~aviImage();


    virtual const char* const format()      const { return _format.c_str(); }
    virtual const char* const compression() const { return _compression.c_str(); }

    ////////////////// Set the frame for the current image (sequence)
    virtual bool           frame( const boost::int64_t f );
    virtual boost::int64_t frame() const { return _frame; }

    /// Returns true if image has an alpha channel
    virtual bool  has_alpha() const { return (_num_channels == 4); };

    ////////////////// Pre-roll sequence for playback
    virtual void preroll( const boost::int64_t frame );
    virtual bool fetch( const boost::int64_t frame );
    /// VCR play (and cache frames if needed) sequence
    virtual void play( const Playback dir,  mrv::ViewerUI* const uiMain);

    virtual boost::int64_t wait_subtitle();

    virtual boost::int64_t wait_image();

    virtual bool    find_image( const boost::int64_t frame );

    virtual boost::uint64_t video_pts() { 
       return frame2pts( get_video_stream(), _frame ); 
    }

    virtual bool has_video() const;

    virtual bool has_picture() const { return has_video(); }

    virtual void video_stream( int x );

    virtual const video_info_t& video_info( unsigned int i ) const
    {
      assert( i < _video_info.size() );
      return _video_info[i];
    }

    virtual unsigned int number_of_video_streams() const { 
      return _video_info.size(); 
    }

    virtual void sequence( const char* fileroot, 
			   const boost::int64_t start, 
			   const boost::int64_t end ) {}

    ////////////////// Add a loop to packet lists
    virtual void loop_at_start( const boost::int64_t frame ); 
    virtual void loop_at_end( const boost::int64_t frame ); 

    virtual void flush_video();
    virtual DecodeStatus decode_video( boost::int64_t& frame );
    virtual DecodeStatus decode_subtitle( boost::int64_t& frame );

    virtual void clear_packets();

    virtual void subtitle_stream( int idx );

    void debug_video_stores(const boost::int64_t frame, 
			    const char* routine = "");

    void debug_subtitle_packets(const boost::int64_t frame, 
				const char* routine = "");
    void debug_subtitle_stores(const boost::int64_t frame, 
			       const char* routine = "");

  protected:

    void open_video_codec();
    void close_video_codec();

    DecodeStatus handle_video_packet_seek( boost::int64_t& frame, 
					   const bool is_seek );

    // Check if a frame is already in video store.
    bool in_video_store( const boost::int64_t frame );

    /** 
     * Decode and store an image from a packet if possible
     * 
     * @param frame frame to use (if packet has no dts)
     * @param pkt   packet to decode
     * 
     * @return true if decoded and stored, false if not
     */
    DecodeStatus decode_image( const boost::int64_t frame, 
			       const AVPacket& pkt );

    void open_subtitle_codec();
    void close_subtitle_codec();
    void subtitle_rect_to_image( const AVSubtitleRect& rect );

    DecodeStatus decode_subtitle( const boost::int64_t frame, 
				  const AVPacket& pkt );

    virtual bool initialize();

    virtual void do_seek();

    void populate();

    /** 
     * Store an image frame in cache
     * 
     * @param frame        video frame to store
     * @param pts          video pts to store
     * 
     */
       void store_image( const boost::int64_t frame,
			 const boost::int64_t pts );

    /** 
     * Decode a video packet, returning its frame
     * 
     * @param pktframe frame of packet (calculated)
     * @param frame    frame to use if packet lacks dts
     * @param pkt      packet
     * 
     * @return true if image was decoded, false if not
     */
    DecodeStatus decode_video_packet( boost::int64_t& pktframe,
				      const boost::int64_t frame,
				      const AVPacket& pkt
				      );

    void limit_video_store( const boost::int64_t frame );
    void limit_subtitle_store( const boost::int64_t frame );

    AVStream* get_video_stream() const;
    bool seek_to_position( const boost::int64_t frame, const int flags = 0 );
    int video_stream_index() const;
       mrv::image_type_ptr allocate_image(const boost::int64_t& frame,
					  const double pts );


    AVStream* get_subtitle_stream() const;
    void flush_subtitle();
    bool in_subtitle_store( const boost::int64_t frame );
    int  subtitle_stream_index() const;
       void store_subtitle( const boost::int64_t frame,
			    const boost::int64_t repeat );
    DecodeStatus handle_subtitle_packet_seek( boost::int64_t& frame, 
					      const bool is_seek );
    DecodeStatus decode_subtitle_packet( boost::int64_t& pktframe,
					 boost::int64_t& repeat,
					 const boost::int64_t frame,
					 const AVPacket& pkt
					 );
    virtual bool    find_subtitle( const boost::int64_t frame );

       void dump_metadata( AVDictionary* m );
  protected:

    std::string _format;
    std::string _compression;

    int              _video_index;
    PixelFormat      _av_dst_pix_fmt;
    VideoFrame::Format _pix_fmt;
    AVFrame*         _av_frame;
    AVCodec*         _video_codec;
    SwsContext*      _convert_ctx;

    video_info_list_t    _video_info;

    unsigned int         _video_images;
    video_cache_t        _images;
    unsigned int         _max_images;


    AVCodec*         _subtitle_codec;
    subtitle_cache_t _subtitles;
    unsigned int     _subtitle_didx;
    unsigned int     _subtitle_sidx;
    AVSubtitle       _sub;
    

  };

} // namespace mrv


#endif // aviImage_h
