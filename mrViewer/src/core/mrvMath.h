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
