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
 * @file   mainWindow.h
 * @author gga
 * @date   Fri Jul  6 14:31:50 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef mainWindow_h
#define mainWindow_h


#include <fltk/Window.h>


namespace mrv {

  class ViewerUI;

  class MainWindow : public fltk::Window
  {
  public:
    MainWindow( int W, int H, const char* title );
    ~MainWindow();
    
    void main( ViewerUI* m ) { uiMain = m; };

       virtual void layout();
    
    virtual int handle( int event );
    
    /** 
     * Make window appear always on top of others
     * 
     */
    void always_on_top();

    /** 
     * Change window's icon to mrViewer's icon
     * 
     */
    void set_icon();

    /** 
     * Iconize all windows
     * 
     */
    void iconize_all();

  protected:

    ViewerUI* uiMain;
  };


} // namespace mrv


#endif  // mainWindow_h




