
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
   char filename[PATH_MAX];
#  define FLPREFS_RESOURCE	"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
   size_t appDataLen = strlen("filmaura") + strlen("mrViewer") + 8;
#if defined( _WIN32 ) || defined ( _WIN64 )
   DWORD type, nn;
   LONG err;
   HKEY key;
   err = RegOpenKey( HKEY_CURRENT_USER, FLPREFS_RESOURCE, &key );
   if (err == ERROR_SUCCESS) {
      nn = PATH_MAX - (DWORD) appDataLen;
      err = RegQueryValueEx( key, "AppData", 0L, &type, (BYTE*)filename, &nn );
      if ( ( err != ERROR_SUCCESS ) && ( type == REG_SZ ) )
      {
	 err = RegQueryValueEx( key, "Personal", 0L, &type, (BYTE*)filename, &nn );
	 if ( ( err != ERROR_SUCCESS ) && ( type == REG_SZ ) )
	    filename[0] = 0;
      }
      RegCloseKey(key);

      path = filename;

      if ( path.empty() )
      {
	 if ( getenv("HOME") )
	    path = getenv("HOME");
	 else if ( getenv("USERPROFILE") )
	    path = getenv("USERPROFILE");
	 else if ( getenv("HOMEDRIVE") )
	 {
	    path = sgetenv("HOMEDRIVE");
	    path += sgetenv("HOMEPATH");
	 }
      }
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
