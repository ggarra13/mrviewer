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
