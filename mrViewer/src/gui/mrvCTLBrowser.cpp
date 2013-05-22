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

  size_t plen = _prefix.size();

  //
  // Iterate thru each path looking for .ctl files
  //
  for (Tokenizer_t::const_iterator it = tokens.begin(); 
       it != tokens.end(); ++it)
    {
      std::string path = *it;
      if ( ! fs::exists( path ) ) continue;

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
	  if ( !_prefix.empty() && base.substr(0, plen) != _prefix )
	    continue;

	  // valid CTL, add it to the browser
	  std::cerr << "add " << base << std::endl;
	  this->add( base.c_str() );
	}
    }
}

int CTLBrowser::handle( int e )
{
  return fltk::Browser::handle( e );
}


} // namespace mrv
