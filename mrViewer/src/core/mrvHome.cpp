
#include <stdlib.h>

#include <fltk/filename.h>
#include <fltk/string.h>

#include "mrvHome.h"

#if defined(_WIN32) && !defined(_WIN64_)
#  include <windows.h>
#endif

namespace mrv
{

std::string sgetenv( const char* const n )
{
   if ( getenv( n ) )
      return getenv( n );
   else
      return std::string();
}

std::string homepath()
{
   std::string path;
#if defined(_WIN32) || defined(_WIN64)
   if ( getenv("HOME") )
       path = getenv("HOME");
   else if ( getenv("USERPROFILE") )
       path = getenv("USERPROFILE");
   else if ( getenv("HOMEDRIVE") )
   {
       path = sgetenv("HOMEDRIVE");
       path += sgetenv("HOMEPATH");
   }
#else
   if ( getenv("HOME" ) )
      path = getenv("HOME");
   else
      path = "/usr/tmp";
#endif
  return path;
}


std::string lockfile()
{
  std::string lockfile = mrv::homepath();
#if defined(_WIN32) || defined(_WIN64)
   lockfile += "/filmaura/mrViewer.lock.prefs";
#else   
   lockfile += "/.fltk/filmaura/mrViewer.lock.prefs";
#endif
   return lockfile;
}


}
