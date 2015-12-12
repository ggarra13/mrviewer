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

  class AviSaveUI;
  class ViewerUI;

  class aviImage : public CMedia 
  {
    aviImage();

    static CMedia* create() { return new aviImage(); }

  public:
    typedef std::vector< mrv::image_type_ptr > video_cache_t;
    typedef std::vector< mrv::image_type_ptr > subtitle_cache_t;

  public:
       static bool test_filename( const char* filename );
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
       virtual void play( const Playback dir, mrv::ViewerUI* const uiMain,
			  const bool fg );

      virtual void clear_cache();

      virtual bool is_cache_filled( int64_t frame );

    virtual boost::int64_t wait_subtitle();

    virtual void wait_image();

      virtual bool find_image( const boost::int64_t frame );

    virtual boost::uint64_t video_pts() { 
       return frame2pts( get_video_stream(), _frame ); 
    }

    virtual bool has_video() const;

    // virtual bool has_picture() const { return has_video(); }

    virtual void video_stream( int x );

    virtual AVStream* get_video_stream() const;

      virtual bool valid_video() const;

    virtual const video_info_t& video_info( unsigned int i ) const
    {
      assert( i < _video_info.size() );
      return _video_info[i];
    }

    virtual size_t number_of_video_streams() const { 
      return _video_info.size(); 
    }

      static bool open_movie( const char* filename, const CMedia* img,
                              const AviSaveUI* opts );
      static bool save_movie_frame( CMedia* img );
       static bool close_movie( const CMedia* img );

       bool save_frame( const mrv::image_type_ptr pic );



    virtual void flush_video();
    virtual DecodeStatus decode_video( boost::int64_t& frame );
    virtual DecodeStatus decode_subtitle( const boost::int64_t frame );

    virtual void clear_packets();

    virtual void subtitle_stream( int idx );

      const char* const colorspace();
      const char* const color_range();

       virtual void probe_size( unsigned p );

    void debug_video_stores(const boost::int64_t frame, 
			    const char* routine = "",
			    const bool detail = false);

    void debug_subtitle_packets(const boost::int64_t frame, 
				const char* routine = "",
				const bool detail = false);
    void debug_subtitle_stores(const boost::int64_t frame, 
			       const char* routine = "",
			       const bool detail = false);

  protected:

       boost::int64_t queue_packets( const boost::int64_t frame,
				     const bool is_seek,
				     bool& got_video,
				     bool& got_audio,
				     bool& got_subtitle );

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
			       AVPacket& pkt );

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
       void store_image( boost::int64_t frame,
			 const boost::int64_t pts );

    /** 
     * Decode a video packet, returning success or not.
     * Checks cache to see if frame already exists after decoding.
     * 
     * @param pktframe frame of packet (calculated)
     * @param frame    frame to use if packet lacks dts
     * @param pkt      packet
     * 
     * @return DecodeOK if decoded, other if not
     */
    DecodeStatus decode_vpacket( boost::int64_t& pktframe,
                                 const boost::int64_t& frame,
                                 const AVPacket& pkt );

    /** 
     * Decode a video packet, returning its frame
     * 
     * @param pktframe frame of packet (calculated)
     * @param frame    frame to use if packet lacks dts
     * @param pkt      packet
     * 
     * @return DecodeOK if decoded, other if not
     */
    DecodeStatus decode_video_packet( boost::int64_t& pktframe,
				      const boost::int64_t frame,
				      const AVPacket& pkt
				      );

    void limit_video_store( const boost::int64_t frame );
    void limit_subtitle_store( const boost::int64_t frame );

    virtual bool seek_to_position( const boost::int64_t frame );
    int video_stream_index() const;
       mrv::image_type_ptr allocate_image(const boost::int64_t& frame,
					  const boost::int64_t& pts);


    AVStream* get_subtitle_stream() const;
    void flush_subtitle();
    bool in_subtitle_store( const boost::int64_t frame );
    int  subtitle_stream_index() const;
       void store_subtitle( const boost::int64_t& frame,
			    const boost::int64_t& repeat );
    DecodeStatus handle_subtitle_packet_seek( boost::int64_t& frame, 
					      const bool is_seek );
    DecodeStatus decode_subtitle_packet( boost::int64_t& pktframe,
					 boost::int64_t& repeat,
					 const boost::int64_t frame,
					 const AVPacket& pkt
					 );
    virtual bool    find_subtitle( const boost::int64_t frame );


  protected:

    std::string _format;
    std::string _compression;

    int                _video_index;
    AVPixelFormat      _av_dst_pix_fmt;
    VideoFrame::Format _pix_fmt;
       VideoFrame::PixelType _ptype;
    AVFrame*         _av_frame;
    AVCodec*         _video_codec;
    AVCodecContext* _subtitle_ctx;           //!< current video context
    SwsContext*      _convert_ctx;

    video_info_list_t    _video_info;

    unsigned int         _video_images;
    video_cache_t        _images;
    unsigned int         _max_images;


    AVCodec*         _subtitle_codec;
    subtitle_cache_t _subtitles;
    AVSubtitle       _sub;
  };

} // namespace mrv


#endif // aviImage_h
