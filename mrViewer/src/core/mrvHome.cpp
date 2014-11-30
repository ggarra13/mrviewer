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
