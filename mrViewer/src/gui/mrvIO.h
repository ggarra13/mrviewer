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

#include <fltk/ask.h>
#include <boost/thread/recursive_mutex.hpp>

#include "mrvI8N.h"

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
      logbuffer() : string_stream() { str().reserve(1024); };
      virtual ~logbuffer() {};

      //! from basic_streambuf, stl function used to sync stream
      virtual int sync();

      virtual void print( const char* c ) = 0;

    protected:
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
      ~errorstream() { delete rdbuf(); };
    };

    struct  warnstream : public std::ostream
    {
      warnstream() : std::ostream( new warnbuffer )
      { 
	flags( std::ios::showpoint | std::ios::right | std::ios::fixed );
      };
      ~warnstream() { delete rdbuf(); };
    };

    struct  infostream : public std::ostream
    {
      infostream() : std::ostream( new infobuffer )
      { 
	flags( std::ios::showpoint | std::ios::right | std::ios::fixed );
      };
      ~infostream() { delete rdbuf(); };
    };

    struct  connstream : public std::ostream
    {
      connstream() : std::ostream( new connbuffer )
      { 
	flags( std::ios::showpoint | std::ios::right | std::ios::fixed );
      };
      ~connstream() { delete rdbuf(); };
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


#define mrvLOG_ERROR(m, x)   do {			\
    mrv::io::error << _("ERROR: ") << N_("[") << m << N_("] ") << x; \
  } while(0)
#define mrvLOG_WARNING(m, x) do {		 \
    mrv::io::warn << _("WARN : ") << N_("[") << m << N_("] ") << x; \
  } while(0)
#define mrvLOG_INFO(m, x)    do {		 \
    mrv::io::info << _("       ") << N_("[") << m << N_("] ") << x; \
  } while(0)
#define mrvCONN_INFO(m, x)    do {		 \
    mrv::io::conn << _("{conn} ") << N_("[") << m << N_("] ") << x; \
  } while(0)

#define LOG_ERROR(x)   mrvLOG_ERROR( kModule, x << std::endl )
#define LOG_WARNING(x) mrvLOG_WARNING( kModule, x << std::endl )
#define LOG_INFO(x)    mrvLOG_INFO( kModule, x << std::endl )
#define LOG_CONN(x)    mrvCONN_INFO( kModule, x << std::endl )
#define IMG_ERROR(x)   LOG_ERROR( name() << " - " << x )
#define IMG_WARNING(x) LOG_WARNING( name() << " - " << x ) 

#ifdef DEBUG
#define DBG(x) do { \
    mrv::io::info << _("mrViewer DBG : ") << x << " at "            \
		  << __FUNCTION__ << ", " << __LINE__ << std::endl; \
  } while(0)
#else
#define DBG(x)
#endif


#endif 
