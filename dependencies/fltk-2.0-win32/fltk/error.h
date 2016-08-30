/*! \file
  Functions to report errors and possibly kill the program.
  You can change these pointers from their default values so that
  fltk calls your code instead.
*/

#ifndef fltk_error_h
#define fltk_error_h

#include "FL_API.h"

namespace fltk {

/// \name fltk/error.h
//@{

extern FL_API void (*warning)(const char*, ...);
extern FL_API void (*error)(const char*, ...);
extern FL_API void (*fatal)(const char*, ...);

//@}

}
#endif
