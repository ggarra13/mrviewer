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




