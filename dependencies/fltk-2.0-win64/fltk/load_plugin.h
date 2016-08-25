/*! \file
  This is a convienence function for portable loading of a plugin
  and possibly returing a symbol in that plugin. The function is
  \e not in the fltk namespace!
*/

#ifndef fl_load_plugin_h
#define fl_load_plugin_h

#include "FL_API.h"

FL_API void* load_plugin(const char* name, const char* symbol);

#endif
