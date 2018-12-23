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
 * @file   mrvIO.h
 * @author gga
 * @date   Wed Oct 25 03:18:47 2006
 *
 * @brief  Routines to handle I/O for mrViewer
 *
 *
 */

#ifndef mrvIO_h
#define mrvIO_h

#include <ostream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include <fltk/ask.h>
#include <boost/thread/recursive_mutex.hpp>

#include "core/mrvI8N.h"

namespace mrv {

// Function used to pass a fltk::alert to main thread
void alert( const char* str );
const char* alert();


namespace io {

typedef
std::basic_stringbuf<char, std::char_traits<char>, std::allocator<char> >
string_stream;

struct logbuffer : public string_stream
{
    static bool _debug;
    static std::fstream out;

    logbuffer() : string_stream() {
        str().reserve(1024);
    };
    virtual ~logbuffer() {
        if (out.is_open()) out.close();
    };

    void debug( bool t ) {
        _debug = t;
        open_stream();
    }
    void open_stream();

    //! from basic_streambuf, stl function used to sync stream
    virtual int sync();

    virtual void print( const char* c ) = 0;

public:
    static boost::recursive_mutex _mutex;
};

struct errorbuffer : public logbuffer
{
    virtual void print( const char* c );
};

struct warnbuffer : public logbuffer
{
    virtual void print( const char* c );
};

struct infobuffer : public logbuffer
{
    virtual void print( const char* c );
};

struct connbuffer : public logbuffer
{
    virtual void print( const char* c );
};


struct  errorstream : public std::ostream
{
    errorstream() : std::ostream( new errorbuffer )
    {
        flags( std::ios::showpoint | std::ios::right | std::ios::fixed );
    };
    ~errorstream() {
        delete rdbuf();
    };
};

struct  warnstream : public std::ostream
{
    warnstream() : std::ostream( new warnbuffer )
    {
        flags( std::ios::showpoint | std::ios::right | std::ios::fixed );
    };
    ~warnstream() {
        delete rdbuf();
    };
};

struct  infostream : public std::ostream
{
    infostream() : std::ostream( new infobuffer )
    {
        flags( std::ios::showpoint | std::ios::right | std::ios::fixed );
    };
    ~infostream() {
        delete rdbuf();
    };
};

struct  connstream : public std::ostream
{
    connstream() : std::ostream( new connbuffer )
    {
        flags( std::ios::showpoint | std::ios::right | std::ios::fixed );
    };
    ~connstream() {
        delete rdbuf();
    };
};


extern connstream  conn;
extern infostream  info;
extern warnstream  warn;
extern errorstream error;

}

} // namespace mrv


#define mrvALERT(x) do { \
    std::ostringstream mErr; \
    mErr << x << std::endl; \
    mrv::alert( mErr.str().c_str() ); \
  } while (0);


#define mrvLOG_ERROR(mod, msg)   do {			\
    mrv::io::error << _("ERROR: ") << N_("[") << mod << N_("] ") << msg; \
  } while(0)
#define mrvLOG_WARNING(mod, msg) do {                               \
    mrv::io::warn << _("WARN : ") << N_("[") << mod << N_("] ") << msg; \
  } while(0)
#define mrvLOG_INFO(mod, msg)    do {                \
    mrv::io::info << _("       ") << N_("[") << mod << N_("] ") << msg; \
  } while(0)
#define mrvCONN_INFO(mod, msg)    do {               \
    mrv::io::conn << _("{conn} ") << N_("[") << mod << N_("] ") << msg; \
  } while(0)

#define LOG_ERROR(msg)   mrvLOG_ERROR( kModule, msg << std::endl )
#define LOG_WARNING(msg) mrvLOG_WARNING( kModule, msg << std::endl )
#define LOG_INFO(msg)    mrvLOG_INFO( kModule, msg << std::endl )
#define LOG_DEBUG(msg)   mrvLOG_INFO( kModule, \
                                    __FUNCTION__ << "(" << __LINE__ << ") " \
                                    << msg << std::endl )
#define LOG_CONN(msg)    mrvCONN_INFO( kModule, msg << std::endl )
#define IMG_ERROR(msg)   do { if( !is_thumbnail() ) LOG_ERROR( this->name() << _(" frame ") << this->frame() << " - " << msg ); } while(0)
#define IMG_WARNING(msg) do { if( !is_thumbnail() ) LOG_WARNING( this->name() << _(" frame ") << this->frame() << " - " << msg ); } while(0)
#define IMG_INFO_F(msg) LOG_INFO( name() << _(" frame ") << this->frame() << " - " << msg )
#define IMG_INFO(msg) LOG_INFO( name() << " - " << msg )

#if 0
#define DBG(msg) do { \
    LOG_DEBUG( msg ); \
} while(0)
#else
#define DBG(msg)
#endif

#define TRACE(msg) do { \
    std::cerr << _("mrViewer TRACE : ") << msg << std::flush << " at "          \
              << __FUNCTION__ << ", " << __LINE__ << std::endl;         \
} while(0)


#endif
