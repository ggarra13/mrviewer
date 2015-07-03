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
 * @file   mrvFrame.h
 * @author 
 * @date   Sun Oct 15 15:41:43 2006
 * 
 * @brief  Some simple classes to represent an audio and video frame.
 * 
 * 
 */


#ifndef mrvFrame_h
#define mrvFrame_h

#include <cstring>
#include <ctime>                  // for time_t
#include <stdexcept>              // for std::runtime_error

#include <boost/cstdint.hpp>      // for int64_t and uint8_t
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

#include "mrvAlignedData.h"
#include "mrvImagePixel.h"


namespace mrv
{

  class VideoFrame
  {
  public:
       static const char* const fmts[];

    //! Channel formats
    enum Format {
      kLumma,
      kLummaA,

      kBGR,
      kBGRA,
      kRGB,
      kRGBA,
      
      kITU_601_YCbCr410,
      kITU_601_YCbCr410A, // @todo: not done
      kITU_601_YCbCr420,
      kITU_601_YCbCr420A, // @todo: not done
      kITU_601_YCbCr422,
      kITU_601_YCbCr422A, // @todo: not done
      kITU_601_YCbCr444,
      kITU_601_YCbCr444A, // @todo: not done

      kITU_709_YCbCr410,
      kITU_709_YCbCr410A, // @todo: not done
      kITU_709_YCbCr420,
      kITU_709_YCbCr420A, // @todo: not done
      kITU_709_YCbCr422,
      kITU_709_YCbCr422A, // @todo: not done
      kITU_709_YCbCr444,
      kITU_709_YCbCr444A, // @todo: not done

      kYByRy410,
      kYByRy410A, // @todo: not done
      kYByRy420,
      kYByRy420A, // @todo: not done
      kYByRy422,
      kYByRy422A, // @todo: not done
      kYByRy444,
      kYByRy444A, // @todo: not done
    };

    //! Pixel Type
    enum PixelType {
      kByte,
      kShort,
      kInt,
      kHalf,
      kFloat
    };

    typedef VideoFrame       self;
    typedef ImagePixel       Pixel;
    typedef boost::shared_array< mrv::aligned16_uint8_t > PixelData;

  private:
    boost::int64_t              _frame;  //!< position in video stream
    boost::int64_t              _pts;  //!< video pts in ffmpeg
    boost::int64_t              _repeat;  //!< number of frames to repeat
    unsigned int                _width;
    unsigned int                _height;
    short unsigned int          _channels; //!< number of channels of image
    time_t                      _mtime;  //!< creation time of frame
    Format                      _format; //!< rgb/yuv format
    PixelType                   _type;   //!< pixel type
    PixelData                   _data;   //!< video data

  public:


    VideoFrame() :
      _frame( 0 ),
      _pts( 0 ),
      _repeat( 0 ),
      _width( 0 ),
      _height( 0 ),
      _channels( 0 ),
      _mtime( 0 ),
      _format( kRGBA ),
      _type( kByte )
    {
    }
    
    VideoFrame( const boost::int64_t& frame,
		const unsigned int w, const unsigned int h, 
		const unsigned short c = 4,
		const Format format  = kRGBA,
		const PixelType type = kByte,
		const boost::int64_t repeat = 0,
		const boost::int64_t pts = 0 ) :
      _frame( frame ),
      _pts( pts ),
      _repeat( repeat ),
      _width( w ),
      _height( h ),
      _channels( c ),
      _mtime( 0 ),
      _format( format ),
      _type( type )
    {
      allocate();
    }

    ~VideoFrame()
    {
    }
    
    void allocate();

    self& operator=( const self& b );

    inline void width( const unsigned w ) { _width = w;  }
    inline unsigned int width()  const    { return _width;  }

    inline void height( const unsigned h ) { _height = h;  }
    inline unsigned int height() const     { return _height; }

    inline boost::int64_t repeat() const              { return _repeat; }

    inline void channels( const short unsigned c ) { _channels = c;  }
    inline short unsigned channels() const     { return _channels; }

    bool has_alpha() const;

    inline void      format( Format f ) { _format = f; }
    inline Format    format()     const { return _format; }

    inline const char* const pixel_format() const { return fmts[_format]; } 

    inline void      pixel_type( PixelType t ) { _type = t; }
    inline PixelType pixel_type() const        { return _type; }
    unsigned short   pixel_size();

    inline void frame( const boost::int64_t& f ) { _frame = f; }
    inline boost::int64_t frame() const          { return _frame; }

    inline void    mtime(const time_t c) { _mtime = c; }
    inline time_t  mtime() const         { return _mtime; }

    inline boost::int64_t pts() const { return _pts; }

    size_t data_size();

    inline void data(const PixelData& d) { _data = d; }

    inline const PixelData& data() const { return _data; }

    ImagePixel pixel( const unsigned int x, const unsigned int y ) const;
    void pixel( const unsigned int x, const unsigned int y, 
		const ImagePixel& p );

    inline bool operator==( const self& b ) const
    {
      return _frame == b.frame();  // should never happen
    }

    inline bool operator<( const self& b ) const
    {
      return _frame < b.frame();
    }

    inline bool operator>( const self& b ) const
    {
      return _frame > b.frame();
    }


    inline bool operator>( const boost::int64_t b ) const
    {
      return _frame > b;
    }

    inline bool operator==( const boost::int64_t b ) const
    {
      return _frame == b;
    }

    inline bool operator<=( const boost::int64_t b ) const
    {
      return _frame <= b;
    }

    inline bool operator>=( const boost::int64_t b ) const
    {
      return _frame >= b;
    }

    inline bool operator<( const boost::int64_t b ) const
    {
      return _frame < b;
    }

    VideoFrame* quick_resize( unsigned int w, unsigned int h ) const;
    VideoFrame* resize( unsigned int w, unsigned int h ) const;

    VideoFrame* scaleX(float t) const;
    VideoFrame* scaleY(float t) const;

  private:
    VideoFrame( const VideoFrame& b ) { }

    ImagePixel pixel_u8( const unsigned int x, const unsigned int y ) const;
    void pixel_u8( const unsigned int x, const unsigned int y, 
		   const ImagePixel& p );

    ImagePixel pixel_u16( const unsigned int x, const unsigned int y ) const;
    void pixel_u16( const unsigned int x, const unsigned int y, 
		    const ImagePixel& p );

    ImagePixel pixel_u32( const unsigned int x, const unsigned int y ) const;
    void pixel_u32( const unsigned int x, const unsigned int y, 
		    const ImagePixel& p );

    ImagePixel pixel_h16( const unsigned int x, const unsigned int y ) const;
    void pixel_h16( const unsigned int x, const unsigned int y, 
		    const ImagePixel& p );

    ImagePixel pixel_f32( const unsigned int x, const unsigned int y ) const;
    void pixel_f32( const unsigned int x, const unsigned int y, 
		    const ImagePixel& p );
  };


  class AudioFrame
  {
    boost::int64_t         _frame;  //!< position in audio stream
    short        _channels;  //!< number of channels
    unsigned int     _freq;  //!< audio frequency
    unsigned int     _size;  //!< size of data (in bytes)
    boost::uint8_t*  _data;  //!< audio data of size _size

  public:
    typedef AudioFrame       self;


  public:
    AudioFrame( const boost::int64_t frame, 
		const int freq, const short channels, 
		const boost::uint8_t* data, const unsigned int size ) :
      _frame( frame ),
      _channels( channels ),
      _freq( freq ),
      _size( size ),
      _data( (boost::uint8_t*)new mrv::aligned16_uint8_t[size] )
    {
      memcpy( _data, data, size );
    }


    ~AudioFrame()
    {
      delete [] _data; _data = NULL;
    }


    inline boost::int64_t   frame() const { return _frame; }
    inline unsigned int frequency() const { return _freq; }
    inline short         channels() const { return _channels; }
    inline unsigned int      size() const { return _size; }
    inline const boost::uint8_t*  data() const { return _data; }

    inline bool operator==( const self& b ) const
    {
      return _frame == b.frame();  // should never happen
    }

    inline bool operator<( const self& b ) const
    {
      return _frame < b.frame();
    }

    inline bool operator>( const self& b ) const
    {
      return _frame > b.frame();
    }

    inline bool operator>( const boost::int64_t b ) const
    {
      return _frame > b;
    }

    inline bool operator==( const boost::int64_t b ) const
    {
      return _frame == b;
    }

    inline bool operator<( const boost::int64_t b ) const
    {
      return _frame < b;
    }

    inline bool operator<=( const boost::int64_t b ) const
    {
      return _frame <= b;
    }

    inline bool operator>=( const boost::int64_t b ) const
    {
      return _frame >= b;
    }

  private:
    AudioFrame( const AudioFrame& b ) { }
  };


  typedef VideoFrame image_type;
  typedef AudioFrame audio_type;

  typedef boost::shared_ptr< VideoFrame > image_type_ptr;
  typedef boost::shared_ptr< AudioFrame > audio_type_ptr;

} // namespace mrv


#endif // mrvFrame_h
