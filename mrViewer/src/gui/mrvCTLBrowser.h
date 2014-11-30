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
 * @file   mrvCTLGroup.h
 * @author gga
 * @date   Mon Jul  2 08:08:45 2007
 * 
 * @brief  Opens a browser with a list of all .ctl scripts available in
 *         your CTL_MODULE_PATH.
 * 
 * 
 */
#ifndef mrvCTLBrowser_h
#define mrvCTLBrowser_h

#include <string>

#include <fltk/Browser.h>


namespace mrv {

  class CTLBrowser : public fltk::Browser
  {
  public:
    CTLBrowser(int x, int y, int w, int h, const char* l = 0);
    ~CTLBrowser();

    void set_prefix( const char* prefix ) { _prefix = prefix; fill(); }

    virtual int handle( int event );

  protected:
    void fill();

  protected:
    std::string _prefix;
  };

}


#endif // mrvCTLBrowser_h
