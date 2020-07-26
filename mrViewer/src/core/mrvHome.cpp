/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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

#include <stdlib.h>

#include <FL/filename.H>
#include <FL/fl_utf8.h>

#include <boost/filesystem.hpp>

#include "mrvHome.h"

#if defined(_WIN32) && !defined(_WIN64_)
#  include <Shlobj.h>
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <pwd.h>
#endif

namespace fs = boost::filesystem;

namespace mrv
{

std::string sgetenv( const char* const n )
{
   if ( fl_getenv( n ) )
      return fl_getenv( n );
   else
      return std::string();
}

std::string homepath()
{
   std::string path;

#ifdef _WIN32
  char dir[MAX_PATH + 1];
  if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, dir)))
    {
        path = std::string(dir) + "/";
        return path;
    }
  else
  {
   char* e = NULL;
   if ( (e = fl_getenv("HOME")) )
   {
       path = e;
       if ( fs::is_directory( path ) )
           return path;
   }
   else if ( (e = fl_getenv("USERPROFILE")) )
   {
       path = e;
       if ( fs::is_directory( path ) )
           return path;
   }
   else if ( (e = fl_getenv("HOMEDRIVE")) )
   {
       path = e;
       path += sgetenv("HOMEPATH");
       path += "/" + sgetenv("USERNAME");
       if ( fs::is_directory( path ) )
           return path;
   }
  }
#else
   char* e = NULL;
   if ( (e = fl_getenv("HOME")) )
   {
       path = e;
       if ( fs::is_directory( path ) )
           return path;
   }
   else
   {
     e = getpwuid( getuid() )->pw_dir;
     if ( e ) {
       path = e;
       return path;
     }
   }
#endif

   path = "/usr/tmp";
   return path;
}


std::string prefspath()
{
  std::string lockfile = mrv::homepath();
   lockfile += "/.filmaura/";
   return lockfile;
}

std::string lockfile()
{
    std::string lockfile = mrv::homepath();
    lockfile += "/.filmaura/mrViewer.lock.prefs";
    return lockfile;
}


}
