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
