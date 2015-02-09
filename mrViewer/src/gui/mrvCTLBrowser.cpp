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
 * @file   mrvCTLBrowser.cpp
 * @author gga
 * @date   Mon Jul  2 08:11:24 2007
 * 
 * @brief  
 * 
 * 
 */

#include <boost/filesystem.hpp>
#include <stdlib.h>
#include <ctype.h>

#include <mrvIO.h>

#include <algorithm>
#include <boost/tokenizer.hpp>
// #include <boost/filesystem/convenience.hpp>
// #include <boost/filesystem/operations.hpp>

#include <mrvCTLBrowser.h>

namespace fs = boost::filesystem;

typedef boost::tokenizer< boost::char_separator<char> > Tokenizer_t;

namespace 
{
  const char* kModule = "gui";
}


namespace mrv {

CTLBrowser::CTLBrowser(int x, int y, int w, int h, const char* l) :
  fltk::Browser( x, y, w, h, l )
{
}

CTLBrowser::~CTLBrowser()
{
}


void CTLBrowser::fill()
{
  this->clear();

  const char* pathEnv = getenv("CTL_MODULE_PATH");
  if (!pathEnv) {
    LOG_ERROR("Environment variable CTL_MODULE_PATH is undefined");
    return;
  }

  std::string str(pathEnv);

#if defined(WIN32) || defined(WIN64)
  boost::char_separator<char> sep(";");
#else
  boost::char_separator<char> sep(":");
#endif

  Tokenizer_t tokens(str, sep);

  boost::char_separator<char> sep2(",");
  Tokenizer_t prefixes( _prefix, sep2 );
 
  // size_t plen = _prefix.size();

  //
  // Iterate thru each path looking for .ctl files
  //
  for (Tokenizer_t::const_iterator it = tokens.begin(); 
       it != tokens.end(); ++it)
    {
      std::string path = *it;
      if ( ! fs::exists( path ) || ! fs::is_directory( path ) ) continue;

      LOG_INFO( path );

      fs::directory_iterator end_itr;
      for ( fs::directory_iterator itr( path ); itr != end_itr; ++itr )
	{
	  fs::path p = *itr;

	  if ( fs::is_directory( p ) ) continue;


	  std::string base = fs::basename( *itr );
	  std::string ext  = fs::extension( *itr );

	  // Make extension lowercase and compare it against "ctl"
	  std::transform( ext.begin(), ext.end(), ext.begin(), tolower );
	  if ( ext != ".ctl" ) continue;

	  // Skip those CTL files that don't match the prefix
          bool found = false;
          for (Tokenizer_t::const_iterator pt = prefixes.begin(); 
               pt != prefixes.end(); ++pt)
          {
              size_t num = base.size() - (*pt).size() + 1;
              for ( size_t i = 0; i < num; ++i )
              {
                  if ( !(*pt).empty() && base.substr(i, (*pt).size()) == *pt )
                  {
                      found = true; break;
                  }
              }
              if ( found ) break;
          }

          if ( !found ) continue;

	  // valid CTL, add it to the browser
	  this->add( base.c_str() );
	}
    }
}

int CTLBrowser::handle( int e )
{
  return fltk::Browser::handle( e );
}


} // namespace mrv
