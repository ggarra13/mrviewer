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
 * @file   mrvPostgreSQL.h
 * @author gga
 * @date   Wed Nov 08 08:07:03 2006
 * 
 * @brief  Interact with postgress database.
 * 
 * 
 */

#ifndef mrvPostgreSQL_h
#define mrvPostgreSQL_h

#include "mrvDatabase.h"
#include "libpq-fe.h"


namespace mrv
{

  class PostgreSQL : public Database
  {
  public:
    PostgreSQL( const char* server = NULL, const int port = 5432,
		const char* user = NULL, const char* passwd = NULL,
		const char* database = NULL );
    virtual ~PostgreSQL();


    virtual bool connect();
    virtual bool sql( const char* cmd );
    virtual void clear_error();
    virtual std::string error() const;
    virtual std::string quote( std::string x );
    virtual std::string binary( const unsigned char* x, const size_t len );

    virtual unsigned number_of_rows() const;
    virtual void result( std::string& res );
    virtual void result( stringArray& res, const unsigned row );

  protected:
    static PGconn*   _connection;
    static PGresult* _result;
    static std::string _last_error;
  };

}


#endif // mrvPostgreSQL_h

