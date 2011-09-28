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

    static CMedia* create() { return new stubImage(); }

  public:
    stubImage( const CMedia* other );

    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }
    bool fetch(const int64_t frame);

    const char* host() { return _host; }

    unsigned int port() { return _portB; }

    inline bool aborted() { return _aborted; }

    virtual bool has_changed();

    virtual const char* const format() const { return "mental images stub"; }

    /// Mark the framebuffer data with a "pattern"
    void crosshatch( const mrv::Recti& r );
    void crosshatch() { crosshatch( mrv::Recti( width(), height() ) ); }

    /// Parse a mray stub file and extract host, ports, image res, etc.
    void parse_stub();

    void add_layer( const char* name, const int fb );

    ///
    virtual void channel( const char* c );
    virtual const char* channel() const { return _channel; };

    ////////////////// Set the frame for the current image (sequence)
    virtual bool     frame( const int64_t f );
    virtual int64_t  frame() const { return _frame; }

    void clear_buffers();
    void resize_buffers( const unsigned int ws, const unsigned int hs );
    void new_buffer( const unsigned int fb, const unsigned int ws, 
		     const unsigned int hs );

    mrv::image_type_ptr frame_buffer( int idx );

    // Sets a pixel of frame buffer fb.
    bool set_float_pixel( const unsigned int fb, 
			  const unsigned int x,
			  const unsigned int y, 
			  const PixelType& c );


    void start_timer();
    void end_timer();

  protected:

    void clear_to_NANs();

    static unsigned int init;

    time_t _startRenderTime;

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

