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
 * @file   mrvDatabaseBrowser.cpp
 * @author root
 * @date   Fri Aug 31 10:52:55 2007
 * 
 * @brief  
 * 
 * 
 */

#include <iostream>

#include <fltk/Image.h>
#include <fltk/Item.h>


#include "core/mrvString.h"
#include "core/mrvI8N.h"

#include "db/mrvPostgreSQL.h"


#include "gui/mrvIO.h"
#include "gui/mrvDatabaseBrowser.h"


namespace 
{
  const char* kModule = "gui";
}


namespace mrv {



  DatabaseBrowser::DatabaseBrowser( const int x, const int y, 
				    const int w, const int h,
				    const char* l ) :
    fltk::Browser( x, y, w, h, l ),
    db( NULL )
  {
    update();
  }

  DatabaseBrowser::~DatabaseBrowser()
  {
    delete db;
  }
  
  void DatabaseBrowser::update()
  {
    clear();

    if ( !db )
      {
	db = mrv::Database::factory();
      }

    std::string fields;
    stringArray fieldArray;
    fieldArray.push_back( "thumbnail" );
    fieldArray.push_back( "directory" );
    fieldArray.push_back( "filename" );
    fieldArray.push_back( "frame_start" );
    fieldArray.push_back( "frame_end" );
    fieldArray.push_back( "width" );
    fieldArray.push_back( "height" );

    if ( fieldArray.empty() ) return;


    stringArray::const_iterator b = fieldArray.begin();
    stringArray::const_iterator i = b;
    stringArray::const_iterator e = fieldArray.end();
    for ( ; i != e; ++i )
      {
	if ( i != b ) fields += ", ";
	fields += *i;
      }


    std::string cmd = "SELECT "; cmd += fields + " FROM images;";

    if (! db->sql( cmd.c_str() ) )
      {
	LOG_ERROR( _("Could not get images in database: ") << std::endl 
		   << db->error() );
	return;
      }

    size_t cols = fieldArray.size();
    unsigned rows = db->number_of_rows();
    for ( unsigned i = 0; i < rows; ++i )
      {
	stringArray res;
	db->result( res, i );

	fltk::Item* elem = new fltk::Item( "" );

	std::string label;
	for ( unsigned c = 1; c < cols; ++c )
	  {
	    label += fieldArray[c];
	    label += ": ";
	    label += res[c];
	    label += "\n";
	  }


	elem->copy_label( label.c_str() );

	size_t icon_w = fieldArray[0].size() / (64*3);

	fltk::Image* img = new fltk::Image( fltk::RGB, unsigned(icon_w), 64, 
					    fieldArray[0].c_str() );
	elem->image( img );

	add( elem );
      }

    redraw();
  }

}
