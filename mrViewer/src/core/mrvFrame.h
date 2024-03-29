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

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <windows.h>
#undef min
#undef max
#pragma warning( disable: 4267 )
#pragma warning( disable: 4244 )
#endif


#include <cstring>
#include <ctime>                  // for time_t
#ifdef LINUX
#include <sys/time.h>             // for timeval, gettimeofday
#endif
#include <stdexcept>              // for std::runtime_error


#include <boost/cstdint.hpp>      // for int64_t and uint8_t
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

#include "core/mrvAssert.h"
#include "core/mrvAlignedData.h"
#include "core/mrvImagePixel.h"

struct SwsContext;

// Defined in mrvTimer.cpp
#if defined(_WIN32) || defined(_WIN64)
int gettimeofday(struct timeval * tp, void * tzp);
#endif



namespace mrv
{

class VideoFrame
{
public:
    static const char* const fmts[];
    static const char* const ptype[];

    //! Channel formats
    enum Format {
        kLumma,
        kLummaA,

        kBGR,
        kBGRA,
        kRGB,
        kRGBA,

        kYUV,
        kYUVA,

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
    size_t                      _width;
    size_t                      _height;
    bool                        _valid;   //! invalid frame
    short unsigned int          _channels; //!< number of channels of image
    time_t                      _ctime;  //!< creation time of frame
    time_t                      _mtime;  //!< modification time of frame
    timeval                     _ptime;  //!< pos. time to erase in timeline
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
        _valid( true ),
        _channels( 0 ),
        _ctime( 0 ),
        _mtime( 0 ),
        _format( kRGBA ),
        _type( kByte )
    {
        gettimeofday( &_ptime, NULL );
    }

    VideoFrame( const VideoFrame& b ) :
        _frame( b._frame ),
        _pts( b._pts ),
        _repeat( b._repeat ),
        _width( b._width ),
        _height( b._height ),
        _valid( true ),
        _channels( b._channels ),
        _ctime( b._ctime ),
        _mtime( b._mtime ),
        _format( b._format ),
        _type( b._type )
    {
        gettimeofday( &_ptime, NULL );
        allocate();
        memcpy( _data.get(), b.data().get(), b.data_size() );
    }

    VideoFrame( const boost::int64_t& frame,
                const size_t w, const size_t h,
                const unsigned short c = 4,
                const Format format  = kRGBA,
                const PixelType type = kByte,
                const boost::int64_t repeat = 0,
                const boost::int64_t pts = 0,
                const bool valid = true ) :
        _frame( frame ),
        _pts( pts ),
        _repeat( repeat ),
        _width( w ),
        _height( h ),
        _valid( valid ),
        _channels( c ),
        _ctime( 0 ),
        _mtime( 0 ),
        _format( format ),
        _type( type )
    {
        gettimeofday( &_ptime, NULL );
        allocate();
    }

    ~VideoFrame();

    void allocate();

    self& operator=( const self& b );

    inline const timeval& ptime() const {
        return _ptime;
    }

    inline void width( const size_t w ) {
        _width = w;
    }
    inline size_t width()  const    {
        return _width;
    }

    inline void height( const size_t h ) {
        _height = h;
    }
    inline size_t height() const     {
        return _height;
    }

    inline void repeat( const int64_t& r )      {
        _repeat = r;
    }
    inline boost::int64_t repeat() const          {
        return _repeat;
    }

    inline void channels( const short unsigned c ) {
        _channels = c;
    }
    inline short unsigned channels() const     {
        return _channels;
    }

    bool has_alpha() const;


    inline void      format( Format f ) {
        _format = f;
    }
    inline Format    format()     const {
        return _format;
    }

    inline const char* const pixel_format() const {
        return fmts[_format];
    }
    inline const char* const pixel_depth() const {
        return ptype[_type];
    }

    inline void      pixel_type( PixelType t ) {
        _type = t;
    }
    inline PixelType pixel_type() const        {
        return _type;
    }
    size_t           line_size() const;
    unsigned short   pixel_size() const;

    inline void frame( const boost::int64_t& f ) {
        _frame = f;
    }
    inline boost::int64_t frame() const          {
        return _frame;
    }

    inline void    ctime(const time_t c) {
        _ctime = c;
    }
    inline time_t  ctime() const         {
        return _ctime;
    }

    inline void    mtime(const time_t c) {
        _mtime = c;
    }
    inline time_t  mtime() const         {
        return _mtime;
    }

    inline boost::int64_t pts() const {
        return _pts;
    }
    inline void pts(const int64_t& p) {
        _pts = p;
    }

    size_t data_size() const;

    // inline void data(const PixelData& d) { _data = d; }

    inline void valid( bool b ) {
        _valid = b;
    }
    inline bool valid() const {
        return _valid;
    }

    inline const PixelData data() const {
        return _data;
    }

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
    timeval                _ptime; //!< audio creation time
    boost::int64_t         _frame;  //!< position in audio stream
    short        _channels;  //!< number of channels
    unsigned int     _freq;  //!< audio frequency
    unsigned int     _size;  //!< size of data (in bytes)
    mrv::aligned16_uint8_t*  _data;  //!< audio data of size _size

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
        _data( new mrv::aligned16_uint8_t[size] )
    {
        gettimeofday( &_ptime, NULL );
        memcpy( _data, data, size );
        sum_memory();
    }

    void sum_memory() noexcept;


    ~AudioFrame();


    inline const timeval&   ptime() const {
        return _ptime;
    }
    inline boost::int64_t   frame() const {
        return _frame;
    }

    inline void frame( int64_t f ) { _frame = f; }

    inline unsigned int frequency() const {
        return _freq;
    }
    inline short         channels() const {
        return _channels;
    }
    inline unsigned int      size() const {
        return _size;
    }
    inline const uint8_t*  data() const {
        return (uint8_t*) _data;
    }

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

// Copy an image picture (video frame) to another, taking into account
// the pixel type.  The pictures have to have the same width/height and
// only (optionally) the same pixel and format type.
// If the same pixel and format type, memcpy is used for speed.
// If not the same format, a temporary image is used and sws_scale is called
// to convert to RGBA (byte pixel type).
// If not the same pixel type a loop is used and a float pixel is used
// as intermediary.
    void copy_image( image_type_ptr& dst, const image_type_ptr& src,
                     SwsContext** sws );

} // namespace mrv


#endif // mrvFrame_h
