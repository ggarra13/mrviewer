/**
 * @file   mrvTimer.h
 * @author gga
 * @date   Mon Jul 16 04:46:45 2007
 * 
 * @brief  windows specific timer stuff
 * 
 * 
 */


#ifndef win32_mrvTimer_h
#define win32_mrvTimer_h

#include <windows.h>
#undef min
#undef max


#define USE_GETTICKCOUNT

namespace mrv {

#ifdef USE_GETTICKCOUNT
  typedef DWORD timer_t;
#endif

#ifdef USE_QUERYPERFORNACECOUNTER
  typedef LARGE_INTEGER timer_t;
#endif

#ifdef USE_TIMEGETTIME
  typedef DWORD timer_t;
#endif

  inline void initialize_timer()
  {
#ifdef USE_TIMEGETTIME
    timeBeginPeriod(1);	// 1ms periods
#endif
  }

  inline void release_timer()
  {
#ifdef USE_TIMEGETTIME
    timeEndPeriod(1);	// 1ms periods
#endif
  }


  inline timer_t time(void)
  {
#ifdef USE_GETTICKCOUNT
    return GetTickCount();
#endif

#ifdef USE_QUERYPERFORMANCECOUNTER
    LARGE_INTEGER ret;
    QueryPerformanceCounter(&ret);
    return ret;
#endif

#ifdef USE_TIMEGETTIME
    return timeGetTime();
#endif
  }


  inline double timespan( const timer_t& now, const timer_t& start )
  {
#ifdef USE_GETTICKCOUNT
    timer_t ticks;
    if ( now < start ) {
      static const DWORD TIME_WRAP_VALUE = (~(DWORD)0);
      ticks = (TIME_WRAP_VALUE - start) + now;
    } else {
      ticks = (now - start);
    }
    return ticks;
#endif

#ifdef USE_QUERYPERFORMANCECOUNTER
    LARGE_INTEGER winclock_frequency;
    QueryPerformanceFrequency( &winclock_frequency );
    return ((double)now.QuadPart - (double)start.QuadPart)/((double)winclock_frequency.QuadPart);
#endif

#ifdef USE_TIMEGETTIME
    timer_t ticks;
    if ( now < start ) {
      static const DWORD TIME_WRAP_VALUE = (~(DWORD)0);
      ticks = (TIME_WRAP_VALUE - start) + now;
    } else {
      ticks = (now - start);
    }
    return ticks;
#endif
  }

  inline void delay( unsigned int ms )
  {
    Sleep(ms);
  }

} // namespace mrv



#endif // win32_mrvTimer_h
