/*! \file

The FLTK version number. Allows you to write conditionally compiled
code for different versions of FLTK. This file may be included by C
code in case you need it there.

FL_VERSION is a macro double that describes the major, minor, and
patch version numbers. The integer value is the major number. One
digit is used for the minor number, and three for the "patch" number
which is increased for each binary differnt release (it can go to 999).

Because double cannot be used in #if statements, the integer
numbers are in the FL_MAJOR_VERSION, FL_MINOR_VERSION, and
FL_PATCH_VERSION macro constants.

*/

#ifndef FL_VERSION

# define FL_MAJOR_VERSION 2 //!< The major release number, 1 or 2
# define FL_MINOR_VERSION 1 //!< The minor release number, 0-9
# define FL_PATCH_VERSION 0 //!< The patch number, 0-999
# define FL_VERSION	  2.1000

#if defined(__cplusplus) || defined(DOXYGEN) /* Allow this file to be included by C code */
#include "FL_API.h"
namespace fltk {

/*!
  Returns the value of FL_VERSION that FLTK was compiled with.
  This can be compared to the FL_VERSION macro to see if the shared
  library of fltk your program linked with is up to date.
*/
FL_API double version();

}
#endif

#endif
