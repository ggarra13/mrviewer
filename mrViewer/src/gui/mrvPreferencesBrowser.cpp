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
 * @file   mrvPreferencesBrowser.cpp
 * @author gga
 * @date   Tue Jan 29 11:51:40 2008
 * 
 * @brief  
 * 
 * 
 */

#include <iostream>
#include <string>



#include <boost/tokenizer.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

#include <FL/Fl_Wizard.H>
#include <FL/Fl_XPM_Image.H>

#include "mrViewer.h"
#include "core/mrvI8N.h"
#include "gui/mrvPreferencesBrowser.h"
#include "gui/FLU/flu_pixmaps.h"

namespace fs = boost::filesystem;

typedef boost::tokenizer< boost::char_separator<char> > Tokenizer_t;

namespace {
  Fl_Pixmap book_closed(book_xpm);
  Fl_Pixmap book_open(question_book_xpm);
  Fl_Pixmap folder_filled(filled_folder_xpm);
  Fl_Pixmap folder_closed(folder_closed_xpm);

/* @todo: fltk1.3 */
  // fltk::MultiImage folder( folder_closed, 
  //       		   fltk::OPENED, folder_filled );
  // fltk::MultiImage book( book_closed, 
  //       		 fltk::SELECTED, book_open ); 
  Fl_Pixmap folder( folder_closed_xpm );
  Fl_Pixmap book( book_xpm );
}

namespace mrv {

  PreferencesBrowser::PreferencesBrowser( const int x, const int y, 
					  const int w, const int h, 
					  const char* lbl ) :
    mrv::Browser( x, y, w, h, lbl )
  {
      // @todo: fltk1.3
    // leaf_symbol( &book );
    // group_symbol( &folder );
  }

  PreferencesBrowser::~PreferencesBrowser()
  {
  }

  //
  // Routine to update the main CTL group (paths and ctl scripts)
  //
  void PreferencesBrowser::update_ctl_tab( PreferencesUI* prefs )
  {

     if ( !prefs ) return;

    Fl_Browser* ctlpaths = prefs->uiPrefsCTLModulePath;
    Fl_Browser* scripts  = prefs->uiPrefsCTLScripts;
    ctlpaths->clear();
    scripts->clear();

    const char* pathEnv = getenv("CTL_MODULE_PATH");
    if ( !pathEnv ) return;

    std::string str(pathEnv);

#if defined(WIN32) || defined(WIN64)
    boost::char_separator<char> sep(";");
#else
    boost::char_separator<char> sep(":");
#endif
    Tokenizer_t tokens(str, sep);

    //
    // Iterate thru each path looking for .ctl files
    //
    for (Tokenizer_t::const_iterator it = tokens.begin(); 
	 it != tokens.end(); ++it)
      {
	std::string path = *it;
	    
	ctlpaths->add( path.c_str() );

	if ( ! fs::exists( path ) ) continue;

	fs::directory_iterator end_itr;
	for ( fs::directory_iterator itr( path ); itr != end_itr; ++itr )
	  {
	    fs::path p = *itr;

	    if ( fs::is_directory( p ) ) continue;


	    std::string base = fs::basename( *itr );
	    std::string ext  = fs::extension( *itr );

	    // Make extension lowercase and compare it against "ctl"
	    std::transform( ext.begin(), ext.end(), ext.begin(), tolower );
	    if ( ext != ".ctl" ) continue;

	    // valid CTL, add it to the browser
	    scripts->add( base.c_str() );
	  }
      }

  }

  void PreferencesBrowser::update( PreferencesUI* prefs )
  {
     if ( prefs == NULL ) return;

     Fl_Wizard* uiWizard = prefs->uiWizard;
     if (uiWizard == NULL ) return;

     int wizard_index = value();  //@todo: fltk1.3 was absolute_item_index

    if ( wizard_index < 0 || wizard_index >= uiWizard->children() )
       return;

    Fl_Widget* child = uiWizard->child( wizard_index );
    if ( !child ) return;

    std::string name = child->label();

    if ( name == _("CTL Paths") )
      {
	update_ctl_tab( prefs );
      }

    // Display proper wizard's child (group)
    uiWizard->value( child );
  }

}
