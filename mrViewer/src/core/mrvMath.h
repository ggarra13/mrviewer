/**
 * @file   mrvMath.h
 * @author gga
 * @date   Wed Aug 22 02:40:11 2007
 * 
 * @brief  Auxialiary math routines
 * 
 * 
 */


#ifndef mrvMath_h
#define mrvMath_h

#include <cmath>


namespace mrv {

  inline bool is_equal( const double a, const double b,
			const double epsilon = 1e-5 )
  {
    // this is faster but inaccurate
    //    return ( (a - epsilon) < b) && (b < ( a + epsilon) );

    // this is slower but more accurate
    return std::abs( a - b ) <= epsilon * std::abs(a);
  }

}

#endif // mrvMath_h
