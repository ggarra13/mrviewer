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
 * @file   mrvDatabase.h
 * @author gga
 * @date   Wed Nov 08 08:08:33 2006
 * 
 * @brief  Abstract class used to interact with a database.
 * 
 * 
 */

#ifndef mrvDatabase_h
#define mrvDatabase_h

#include <string>
#include <vector>
#include "core/mrvString.h"

namespace mrv
{

  class Database
  {
  public:
    Database( const char* server, const int port,
	      const char* user, const char* passwd,
	      const char* database );
    
    virtual ~Database();

    virtual bool connect() = 0;

    virtual bool sql( const char* cmd ) = 0;
    virtual void result( std::string& res ) = 0;

    virtual unsigned number_of_rows() const = 0;
    virtual void result( stringArray& res, const unsigned row ) = 0;

    virtual void clear_error() = 0;
    virtual std::string error() const = 0;

    virtual std::string quote( std::string x ) = 0;
    virtual std::string binary( const unsigned char* x, 
				const size_t len ) = 0;

    const char* database() const { return _database; }

    bool connected() { return _connected; }

    static Database* factory();


  protected:
    void log_variables();

  protected:
    const char* _host;
    int   _port;
    bool  _connected;
    const char* _user;
    const char* _passwd;
    const char* _database;
  };

}


#endif // mrvDatabase_h
