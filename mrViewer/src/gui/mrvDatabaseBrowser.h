/**
 * @file   mrvDatabaseBrowser.h
 * @author gga
 * @date   Wed Aug 22 03:56:20 2007
 * 
 * @brief  
 * 
 * 
 */


#ifndef mrvDatabaseBrowser_h
#define mrvDatabaseBrowser_h

#include <fltk/Browser.h>

namespace mrv {

  class Database;

  class DatabaseBrowser : public fltk::Browser
  {
  public:
    DatabaseBrowser( int x, int y, int w, int h, const char* l = 0 );
    ~DatabaseBrowser();

    void update();

  protected:
    mrv::Database* db;
  };

} // namespace mrv

#endif // mrvDatabaseBrowser_h
