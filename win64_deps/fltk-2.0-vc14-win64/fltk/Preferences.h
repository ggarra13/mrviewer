// "$Id: Preferences.H 4458 2005-07-26 07:59:01Z matt $"
//
// Copyright 2002-2005 by Matthias Melcher.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php

#ifndef fltk_Preferences_h
#define fltk_Preferences_h

#include "FL_API.h"

namespace fltk {

/**
 * Preferences are a data tree containing a root, branches and leafs
 */
class FL_API Preferences
{

public:

  enum Root { SYSTEM=0, USER };

  Preferences( Root root, const char *vendor, const char *application );
  Preferences( const char *path, const char *vendor, const char *application );
  Preferences( Preferences&, const char *group );
  Preferences( Preferences*, const char *group );
  ~Preferences();

  int groups();
  const char *group( int );
  bool groupExists( const char *group );
  bool deleteGroup( const char *group );

  int entries();
  const char *entry( int );
  bool entryExists( const char *entry );
  bool deleteEntry( const char *entry );

  bool set( const char *entry, int value );
  bool set( const char *entry, float value );
  bool set( const char *entry, double value );
  bool set( const char *entry, const char *value );
  bool set( const char *entry, const void *value, int size );

  bool get( const char *entry, int &value,    int defaultValue );
  bool get( const char *entry, float &value,  float defaultValue );
  bool get( const char *entry, double &value, double defaultValue );
  bool get( const char *entry, char *&value,  const char *defaultValue );
  bool get( const char *entry, char *value,   const char *defaultValue, int maxSize );
  bool get( const char *entry, void *&value,  const void *defaultValue, int defaultSize );
  bool get( const char *entry, void *value,   const void *defaultValue, int defaultSize, int maxSize );
  int size( const char *entry );

  bool getUserdataPath( char *path, int pathlen );

  void flush();

  // bool export( const char *filename, Type fileFormat );
  // bool import( const char *filename );

  class FL_API Name {
    char *data_;
  public:
    Name( int n );
    Name( const char *format, ... );
    operator const char *() { return data_; }
    ~Name() { delete[] data_; }
  };

private:

  // make the following functions unavailable 
  Preferences();
  Preferences(const Preferences&);
  Preferences &operator=(const Preferences&);

  class Node;
  friend class Node;
  Node *node;

  class RootNode;
  friend class RootNode;
  RootNode *rootNode;

};

}

#endif

// End of "$Id: Preferences.H 4458 2005-07-26 07:59:01Z matt $".
