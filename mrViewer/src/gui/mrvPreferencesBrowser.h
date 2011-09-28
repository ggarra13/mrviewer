/**
 * @file   mrvPreferencesBrowser.h
 * @author gga
 * @date   Tue Jan 29 11:44:23 2008
 * 
 * @brief  
 * 
 * 
 */


#ifndef mrvPreferencesBrowser_h
#define mrvPreferencesBrowser_h

#include "gui/mrvBrowser.h"


namespace mrv {

  class PreferencesUI;

  class PreferencesBrowser : public mrv::Browser
  {
  public:
    PreferencesBrowser( const int x, const int y, const int w, const int h, 
			const char* lbl = 0 );
    ~PreferencesBrowser();

    void update( mrv::PreferencesUI* prefs );

  protected:
    void update_ctl_tab( mrv::PreferencesUI* prefs );
  };

}


#endif // mrvPreferencesBrowser
