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
 * @file   mrvPostgreSQL.cpp
 * @author gga
 * @date   Wed Nov 15 19:33:29 2006
 * 
 * @brief  PostgreSQL database interface.
 * 
 * 
 */

#include <cstdio>     // for sprintf

using namespace std;

#include "mrvIO.h"
#include "mrvPostgreSQL.h"

namespace 
{
  const char* kModule = "psql";
}


namespace mrv
{

  PGconn*     PostgreSQL::_connection = NULL;
  PGresult*   PostgreSQL::_result = NULL;
  std::string PostgreSQL::_last_error;

  PostgreSQL::PostgreSQL( const char* server, const int port,
			  const char* user, const char* passwd,
			  const char* database ) :
    Database( server, port, user, passwd, database )
  {
    if ( !_connection )
      {
	if ( ! connect() )
	  {
	    LOG_ERROR( "Could not connect to postgreSQL database as:" );
	    log_variables();
	  }
      }
    else
      {
	_connected = true;
      }
  }

  bool PostgreSQL::connect()
  {
    char buf[4096];
    sprintf( buf, "host='%s' port='%d' dbname='%s' user='%s' password='%s'",
	     _host, _port, _database, _user, _passwd );

    _connection = PQconnectdb(buf);
    if ( PQstatus(_connection) != CONNECTION_OK )
      return false;

    _connected = true;
    LOG_INFO( _("Connected to database '") << _database << N_("'.") );

    return true;
  }

  PostgreSQL::~PostgreSQL()
  {
     clear_error();
     if ( _connection )
     	PQfinish(_connection);
     _connection = NULL;
  }


  bool PostgreSQL::sql( const char* cmd )
  {
    if ( !_connection ) return false;

    clear_error();
    _result = PQexecParams( _connection, cmd, 0,
			    NULL, NULL, NULL, NULL, 1 );

    ExecStatusType status = PQresultStatus( _result );

    if ( status == PGRES_COMMAND_OK ||
	 status == PGRES_COMMAND_OK ||
	 status == PGRES_TUPLES_OK )
      {
	return true;
      }
    else
      {
	_last_error = PQresStatus(status);
	_last_error += " ";
	_last_error += PQresultErrorMessage(_result);
	return false;
      }
  }

  void PostgreSQL::clear_error()
  {
    if ( _result ) {
      PQclear( _result ); _result = NULL;
    }
  }

  std::string PostgreSQL::error() const
  {
    if (!_result) return std::string();
    return _last_error;
  }

  std::string PostgreSQL::quote( std::string x )
  {
    std::string ret;
    char* buf = new char[ x.size() * 2 + 1 ];
    int err = 0;
    PQescapeStringConn( _connection, buf, x.c_str(), x.size(), &err );
    ret = buf;
    delete [] buf;
    return ret;
  }

  std::string PostgreSQL::binary( const unsigned char* x, const size_t len )
  {
    std::string ret;
    if ( !x ) return ret;
    size_t newlen;
    unsigned char* buf = PQescapeByteaConn( _connection, x, len, &newlen );
    ret = (char*)buf;
    PQfreemem( buf );
    return ret;
  }

  void PostgreSQL::result( std::string& res )
  {
    int rows = PQntuples( _result );
    int cols = PQnfields( _result );


    for ( int r = 0; r < rows; ++r )
      {
	for ( int c = 0; c < cols; ++c )
	  {
	    const char* text = PQgetvalue( _result, r, c);
	    res += text;
	    res += "\t";
	  }
	res += "\n";
      }
  }

  unsigned PostgreSQL::number_of_rows() const
  { 
    if ( !_result ) return 0;
    return PQntuples( _result );
  }

  void PostgreSQL::result( stringArray& res, const unsigned row )
  {
    int cols = PQnfields( _result );

    res.resize( cols );

    for ( int c = 0; c < cols; ++c )
      {
	res[c] = PQgetvalue( _result, row, c);
      }
  }


}
