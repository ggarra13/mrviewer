/**
 * @file   mrvAlignedData.h
 * @author gga
 * @date   Sun Jan 27 10:12:19 2008
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvAlignedData_h
#define mrvAlignedData_h

#include <malloc.h>

#include <boost/cstdint.hpp>

#if defined(WIN32) || defined(WIN64)
#  define memalign( b, a )   _aligned_malloc( a, b )
#  define memalign_free( a ) _aligned_free( a ) 
#else
#  define memalign_free( a ) free( a )
#endif


namespace mrv {


  struct aligned16_uint8_t
  {
    boost::uint8_t x;

    inline void* operator new(size_t size)
    {      
      return memalign( 16, size*sizeof(aligned16_uint8_t) );
    }

    inline void operator delete( void* ptr )
    {
      memalign_free( ptr );
    }

    inline void* operator new[](size_t size)
    {      
      return memalign( 16, size*sizeof(aligned16_uint8_t) );
    }

    inline void operator delete[]( void* ptr )
    {
      memalign_free( ptr );
    }
  };


}  // namespace mrv


#endif // mrvAlignedData
