/**
 * @file   mrvDatabase.cpp
 * @author gga
 * @date   Wed Nov 08 09:21:57 2006
 * 
 * @brief  
 * 
 * 
 */

#include <cstring>

#include "core/mrvOS.h"
#include "mrvIO.h"

#include "mrvDatabase.h"

#include "mrvPostgreSQL.h"

namespace 
{
  const char* kModule = "db";
}

namespace mrv
{

  Database::Database( const char* host, const int port,
		      const char* user, const char* passwd,
		      const char* database )
  {
    if ( !host ) host = getenv( "MRV_HOST" );
    if ( !host ) host = "localhost";

    if ( !user ) user = getenv( "MRV_USER" );
    if ( !user ) user = "mrviewer";

    if ( !passwd ) passwd = getenv( "MRV_PASSWORD" );
    if ( !passwd ) passwd = "mrviewer";

    database = getenv( "MRV_DATABASE" );
#ifdef DEBUG
    if ( !database) database = "assets2d_development";
#else
    if ( !database) {
       const char* rails = getenv("RAILS_ENV");
       if (rails)
       {
	  if ( strcmp(rails, "production") == 0 )
	     database = "assets2d_production";
	  else if ( strcmp( rails, "test" ) == 0 )
	     database = "assets2d_test";
       }

       if ( !database)
       {
	  database = "assets2d_development";
       }
    }
#endif

    _host = strdup( host );
    _port = port;
    
    const char* portStr = getenv( "MRV_PORT" );
    if ( portStr ) _port = atoi( portStr );

    _user = strdup( user );
    _passwd = strdup( passwd );
    _database = strdup( database );
    _connected = false;
  }
  
  void Database::log_variables()
  {
    std::string driver;
    const char* var = getenv("MRV_DATABASE_DRIVER");
    if ( var ) driver = var;
    if ( driver == "" ) driver = "postgresql";

    LOG_ERROR( "MRV_DATABASE_DRIVER: " << driver );
    LOG_ERROR( "MRV_HOST: " << _host << " MRV_PORT: " << _port );
    LOG_ERROR( "MRV_USER: " << _user << " MRV_PASSWORD: " << _passwd );
    LOG_ERROR( "MRV_DATABASE: " << _database );
  }

  Database::~Database()
  {
    free( (void*) _host );
    free( (void*) _user );
    free( (void*) _passwd );
    free( (void*) _database );
  }

  Database* Database::factory()
  {
    std::string driver;

    const char* var = getenv("MRV_DATABASE_DRIVER");
    if ( var ) driver = var;

    if ( strcasecmp( driver.c_str(), "None" ) == 0 )
      return NULL;

    if ( strcasecmp( driver.c_str(), "PostgreSQL" ) == 0 )
      {
	 return new mrv::PostgreSQL();
      }

    LOG_ERROR( _("Invalid database driver '") << driver << N_("'") );
    return NULL;
  }

}
