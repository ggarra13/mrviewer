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

