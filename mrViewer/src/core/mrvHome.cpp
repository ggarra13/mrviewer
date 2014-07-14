
#include <stdlib.h>

#include <fltk/filename.h>
#include <fltk/string.h>

#include <boost/filesystem.hpp>

#include "mrvHome.h"

#if defined(_WIN32) && !defined(_WIN64_)
#  include <windows.h>
#endif

namespace fs = boost::filesystem;

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

   char* e = NULL;
   if ( e = getenv("HOME") )
   {
       path = e;
       if ( fs::is_directory( path ) )
           return path;
   }
   else if ( e = getenv("USERPROFILE") )
   {
       path = e;
       if ( fs::is_directory( path ) )
           return path;
   }
   else if ( e = getenv("HOMEDRIVE") )
   {
       path = e;
       path += sgetenv("HOMEPATH");
       if ( fs::is_directory( path ) )
           return path;
   }
   path = "/usr/tmp";
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
