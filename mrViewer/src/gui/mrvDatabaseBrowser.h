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
