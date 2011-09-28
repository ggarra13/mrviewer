/**
 * @file   mrvTimer.h
 * @author gga
 * @date   Mon Jul 16 04:48:56 2007
 * 
 * @brief  Generic Unix timer functions
 * 
 * 
 */


#ifndef unix_mrvTimer_h
#define unix_mrvTimer_h


#define HAVE_CLOCK_GETTIME

#ifdef HAVE_CLOCK_GETTIME
  #include <errno.h>
  #include <time.h>
#else
  #include <sys/time.h>
#endif

namespace mrv {

#ifdef HAVE_CLOCK_GETTIME
  typedef struct timespec timer_t;

  inline timespec time(void)
  {
    timespec ret;
    clock_gettime( CLOCK_MONOTONIC, &ret );
    return ret;
  }

  // calculate a difference between two times in milliseconds 
  inline double timespan( const timespec& now, const timespec& start )
  {
    return ( (now.tv_sec  - start.tv_sec) * 1000 + 
	     (now.tv_nsec - start.tv_nsec)/1000000.0 );
  }

#else

  typedef struct timeval timer_t;

  inline timeval time(void)
  {
    timezone tz;
    timeval ret;
    gettimeofday(&ret, &tz);
    return ret;
  }


  // calculate a difference between two times in milliseconds 
  inline double timespan( const timeval& now, const timeval& start )
  {
    return ( (now.tv_sec  - start.tv_sec) * 1000 + 
	     (now.tv_usec - start.tv_usec)/1000000.0 );
  }

#endif

  inline void delay( const unsigned int ms )
  {
    if ( ms <= 1 ) return;

    mrv::timer_t start = mrv::time();

    mrv::timer_t now;
    do {
      now = mrv::time();
    } while ( timespan( now, start ) < ms );

  }


} // namepace mrv

#endif // unix_mrvTimer_h
