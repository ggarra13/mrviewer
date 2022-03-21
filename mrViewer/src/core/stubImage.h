/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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
 * @file   stubImage.h
 * @author gga
 * @date   Sat Aug 25 17:42:12 2007
 *
 * @brief  Custom image reader for mental ray stub files.
 *
 *
 */

#ifndef stubImage_h
#define stubImage_h

#include <ctime>
#include <sys/stat.h>

#include "mrSocket.h"
#include "CMedia.h"

namespace mrv {

class stubImage : public CMedia
{
    stubImage();


    ~stubImage();

    static CMedia* create() {
        return new stubImage();
    }

public:
    stubImage( const CMedia* other );

    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
        return CMedia::get(create, name, datas);
    }
    bool fetch( mrv::image_type_ptr& canvas, const int64_t frame);

    const char* host() {
        return _host;
    }

    unsigned int port() {
        return _portB;
    }

    inline bool aborted() {
        return _aborted;
    }

    virtual bool has_changed();

    virtual const char* const format() const {
        return "mental images stub";
    }

    /// Mark the framebuffer data with a "pattern"
    void crosshatch( const mrv::Recti& r );
    void crosshatch() {
        crosshatch( mrv::Recti( width(), height() ) );
    }

    /// Parse a mray stub file and extract host, ports, image res, etc.
    void parse_stub( mrv::image_type_ptr& canvas );

    void add_layer( const char* name, const int fb );

    ///
    virtual void channel( const char* c );
    virtual const char* channel() const {
        return _channel;
    };

    ////////////////// Set the frame for the current image (sequence)
    virtual bool     frame( const int64_t f );
    virtual int64_t  frame() const {
        return _frame;
    }

    void clear_buffers();
    void resize_buffers( const unsigned int ws, const unsigned int hs );
    void new_buffer( const unsigned int fb, const unsigned int ws,
                     const unsigned int hs );

    mrv::image_type_ptr frame_buffer( int idx );

    // Sets a pixel of frame buffer fb.
    bool set_float_pixel( const unsigned int fb,
                          const unsigned int x,
                          const unsigned int y,
                          const Pixel& c );


    void start_timer();
    void end_timer();
    void wait_for_rthreads();
    void thread_exit();

protected:

    void clear_to_NANs();

    static unsigned int init;

    time_t _startRenderTime;

    thread_pool_t  _rthreads;  //!< render threads used to monitor
    bool  _aborted;
    float _version;
    unsigned int   _portA;
    unsigned int   _portB;
    char* _host;

    MR_SOCKET id;

    // for last mtime
    struct stat _sbuf;
};

}


#endif // stubImage_h

