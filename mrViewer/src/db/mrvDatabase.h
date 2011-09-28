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
