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
 * @file   mrvLMTBrowser.h
 * @author gga
 * @date   Mon Jul  2 08:08:45 2007
 * 
 * @brief  Opens a browser with a list of all .ctl scripts available in
 *         your LMT_MODULE_PATH.
 * 
 * 
 */
#ifndef mrvLMTBrowser_h
#define mrvLMTBrowser_h

#include <string>

#include <fltk/Browser.h>
#include "mrvMedia.h"

namespace mrv {

  class LMTBrowser : public fltk::Browser
  {
  public:
    LMTBrowser(int x, int y, int w, int h, const char* l = 0);
    ~LMTBrowser();

      
    void fill( const mrv::media& fg );
    virtual int handle( int event );

  protected:

  protected:
      mrv::media _fg;
  };

}


#endif // mrvLMTBrowser_h
