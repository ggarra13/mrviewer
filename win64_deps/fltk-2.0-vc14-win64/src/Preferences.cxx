//
// "$Id: Preferences.cxx 4660 2005-11-27 14:45:48Z mike $"
//
// Preferences methods for the Fast Light Tool Kit (FLTK).
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
//

#include <config.h>
#include <fltk/Preferences.h>
#include <fltk/filename.h>
#include <fltk/string.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#  include <windows.h>
#  include <direct.h>
#  include <io.h>
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...
#  define access _access
#  define mkdir _mkdir
#elif defined (__APPLE__)
#  include <Carbon/Carbon.h>
#  include <unistd.h>
#else
#  include <unistd.h>
#endif

using namespace fltk;

struct Entry {
  char *name, *value;
};

class Preferences::Node // a node contains a list to all its entries
{          // and all means to manage the tree structure
  Node *child_, *next_, *parent_;
  char *path_;
  bool dirty_;
public:
  Node( const char *path );
  ~Node();
  // node methods
  int write( FILE *f );
  Node *find( const char *path );
  Node *search( const char *path, int offset=0 );
  Node *addChild( const char *path );
  void setParent( Node *parent );
  Node *parent() { return parent_; }
  bool remove();
  bool dirty();
  // entry methods
  int nChildren();
  const char *child( int ix );
  void set( const char *name, const char *value );
  void set( const char *line );
  void add( const char *line );
  const char *get( const char *name );
  int getEntry( const char *name );
  bool deleteEntry( const char *name );
  // public values
  Entry *entry;
  int nEntry, NEntry;
  static int lastEntrySet;
};

class Preferences::RootNode  // the root node manages file paths and basic reading and writing
{
  Preferences *prefs_;
  char *filename_;
  char *vendor_, *application_;
public:
  RootNode( Preferences *, Root root, const char *vendor, const char *application );
  RootNode( Preferences *, const char *path, const char *vendor, const char *application );
  ~RootNode();
  int read();
  int write();
  bool getPath( char *path, int pathlen );
};

/**
 * create the initial preferences base
 * - root: machine or user preferences
 * - vendor: unique identification of author or vendor of application
 *     Must be a valid directory name.
 * - application: vendor unique application name, i.e. "PreferencesTest"
 *     multiple preferences files can be created per application.
 *     Must be a valid file name.
 * example: Preferences base( Preferences::USER, "fltk.org", "test01");
 */
Preferences::Preferences( Root root, const char *vendor, const char *application )
{
  node = new Node( "." );
  rootNode = new RootNode( this, root, vendor, application );
}


/**
 * create the initial preferences base
 * - path: an application-supplied path
 * example: Preferences base( "/usr/foo" );
 */
Preferences::Preferences( const char *path, const char *vendor, const char *application )
{
  node = new Node( "." );
  rootNode = new RootNode( this, path, vendor, application );
}


/**
 * create a Preferences node in relation to a parent node for reading and writing
 * - parent: base name for group
 * - group: group name (can contain '/' seperated group names)
 * example: Preferences colors( base, "setup/colors" );
 */
Preferences::Preferences( Preferences &parent, const char *key )
{
  rootNode = parent.rootNode;
  node = parent.node->addChild( key );
}


/**
 * create a Preferences node in relation to a parent node for reading and writing
 * - parent: base name for group
 * - group: group name (can contain '/' seperated group names)
 * example: Preferences colors( base, "setup/colors" );
 */
Preferences::Preferences( Preferences *parent, const char *key )
{
  rootNode = parent->rootNode;
  node = parent->node->addChild( key );
}


/**
 * destroy individual keys
 * - destroying the base preferences will flush changes to the prefs file
 * - after destroying the base, none of the depending preferences must be read or written
 */
Preferences::~Preferences()
{
  if (!node->parent()) delete rootNode;
  // DO NOT delete nodes! The root node will do that after writing the preferences
}


/**
 * return the number of groups that are contained within a group
 * example: int n = base.groups();
 */
int Preferences::groups()
{
  return node->nChildren();
}


/**
 * return the group name of the n'th group
 * - there is no guaranteed order of group names
 * - the index must be within the range given by groups()
 * example: printf( "Group(%d)='%s'\n", ix, base.group(ix) );
 */
const char *Preferences::group( int ix )
{
  return node->child( ix );
}


/**
 * return 1, if a group with this name exists
 * example: if ( base.groupExists( "setup/colors" ) ) ...
 */
bool Preferences::groupExists( const char *key )
{
    return node->search( key ) ? true : false ;
}


/**
 * delete a group
 * example: setup.deleteGroup( "colors/buttons" );
 */
bool Preferences::deleteGroup( const char *key )
{
  Node *nd = node->search( key );
  if ( nd ) return nd->remove();
  return false;
}


/**
 * return the number of entries (name/value) pairs for a group
 * example: int n = buttonColor.entries();
 */
int Preferences::entries()
{
  return node->nEntry;
}


/**
 * return the name of an entry
 * - there is no guaranteed order of entry names
 * - the index must be within the range given by entries()
 * example: printf( "Entry(%d)='%s'\n", ix, buttonColor.entry(ix) );
 */
const char *Preferences::entry( int ix )
{
  return node->entry[ix].name;
}


/**
 * return 1, if an entry with this name exists
 * example: if ( buttonColor.entryExists( "red" ) ) ...
 */
bool Preferences::entryExists( const char *key )
{
  return node->getEntry( key )>=0;
}


/**
 * remove a single entry (name/value pair)
 * example: buttonColor.deleteEntry( "red" );
 */
bool Preferences::deleteEntry( const char *key )
{
  return node->deleteEntry( key );
}


/**
 * read an entry from the group
 */
bool Preferences::get( const char *key, int &value, int defaultValue )
{
  const char *v = node->get( key );
  value = v ? atoi( v ) : defaultValue;
  return ( v != 0 );
}


/**
 * set an entry (name/value pair)
 */
bool Preferences::set( const char *key, int value )
{
  char nameBuffer[128];
  sprintf( nameBuffer, "%d", value );
  node->set( key, nameBuffer );
  return true;
}


/**
 * read an entry from the group
 */
bool Preferences::get( const char *key, float &value, float defaultValue )
{
  const char *v = node->get( key );
  value = v ? (float)atof( v ) : defaultValue;
  return ( v != 0 );
}


/**
 * set an entry (name/value pair)
 */
bool Preferences::set( const char *key, float value )
{
  char nameBuffer[128];
  sprintf( nameBuffer, "%g", value );
  node->set( key, nameBuffer );
  return true;
}


/**
 * read an entry from the group
 */
bool Preferences::get( const char *key, double &value, double defaultValue )
{
  const char *v = node->get( key );
  value = v ? atof( v ) : defaultValue;
  return ( v != 0 );
}


/**
 * set an entry (name/value pair)
 */
bool Preferences::set( const char *key, double value )
{
  char nameBuffer[128];
  sprintf( nameBuffer, "%g", value );
  node->set( key, nameBuffer );
  return true;
}


// remove control sequences from a string
static char *decodeText( const char *src )
{
  int len = 0;
  const char *s = src;
  for ( ; *s; s++, len++ )
  {
    if ( *s == '\\' ) {
      if ( isdigit( s[1] ) )
	s+=3;
      else
	s+=1;
    }
  }
  char *dst = new char[len+1];
  char *d = dst;
  for ( s = src; *s; s++ )
  {
    char c = *s;
    if ( c == '\\' )
    {
      if ( s[1] == '\\' ) { *d++ = c; s++; }
      else if ( s[1] == 'n' ) { *d++ = '\n'; s++; }
      else if ( s[1] == 'r' ) { *d++ = '\r'; s++; }
      else if ( isdigit( s[1] ) ) { *d++ = ((s[1]-'0')<<6) + ((s[2]-'0')<<3) + (s[3]-'0'); s+=3; }
      else s++; // error
    }
    else
      *d++ = c;
  }
  *d = 0;
  return dst;
}


/**
 * read a text entry from the group
 * the text will be moved into the given text buffer
 * text will be clipped to the buffer size
 */
bool Preferences::get( const char *key, char *text, const char *defaultValue, int maxSize )
{
  const char *v = node->get( key );
  if ( v && strchr( v, '\\' ) ) {
    char *w = decodeText( v );
    strlcpy(text, w, maxSize);
    delete[] w;
    return true;
  }
  if ( !v ) v = defaultValue;
  if ( v ) strlcpy(text, v, maxSize);
  else *text = 0;
  return ( v != defaultValue );
}


/**
 * read a text entry from the group
 * 'text' will be changed to point to a new text buffer
 * the text buffer must be freed with "delete[] text" by the user.
 */
bool Preferences::get( const char *key, char *&text, const char *defaultValue )
{
  const char *v = node->get( key );
  if ( v && strchr( v, '\\' ) )
  {
    text = decodeText( v );
    return true;
  }    
  if ( !v ) v = defaultValue;
  text = newstring(v);
  return ( v != defaultValue );
}


/**
 * set an entry (name/value pair)
 */
bool Preferences::set( const char *key, const char *text )
{
  const char *s = text;
  int n=0, ns=0;
  for ( ; *s; s++ ) { n++; if ( *s<32 || *s=='\\' || *s==0x7f ) ns+=4; }
  if ( ns )
  {
    char *buffer = new char[n+ns+1];
    char *d = buffer;
    for ( s=text; *s; ) 
    { 
      char c = *s;
      if ( c=='\\' ) { *d++ = '\\'; *d++ = '\\'; s++; }
      else if ( c=='\n' ) { *d++ = '\\'; *d++ = 'n'; s++; }
      else if ( c=='\r' ) { *d++ = '\\'; *d++ = 'r'; s++; }
      else if ( c<32 || c==0x7f ) 
	{ *d++ = '\\'; *d++ = '0'+((c>>6)&3); *d++ = '0'+((c>>3)&7); *d++ = '0'+(c&7);  s++; }
      else *d++ = *s++;
    }
    *d = 0;
    node->set( key, buffer );
    delete[] buffer;
  }
  else
    node->set( key, text );
  return true;
}


// convert a hex string to binary data
static char *decodeHex( const char *src, int &size )
{
  size = strlen( src )/2;
  char *data = new char[size];
  char *d = data;
  const char *s = src;
  int i;

  for ( i=size; i>0; i-- )
  {
    int v;
    char x = tolower(*s++);
    if ( x >= 'a' ) v = x-'a'+10; else v = x-'0';
    v = v<<4;
    x = tolower(*s++);
    if ( x >= 'a' ) v += x-'a'+10; else v += x-'0';
    *d++ = (char)v;
  }

  return data;
}


/**
 * read a binary entry from the group
 * the data will be moved into the given destination buffer
 * data will be clipped to the buffer size
 */
bool Preferences::get( const char *key, void *data, const void *defaultValue, int defaultSize, int maxSize )
{
  const char *v = node->get( key );
  if ( v )
  {
    int dsize;
    char *w = decodeHex( v, dsize );
    memcpy( data, w, dsize>maxSize?maxSize:dsize );
    delete[] w;
    return true;
  }    
  if ( defaultValue )
    memmove( data, defaultValue, defaultSize>maxSize?maxSize:defaultSize );
  return false;
}


/**
 * read a binary entry from the group
 * 'data' will be changed to point to a new data buffer
 * the data buffer must be deleted with "delete[] (char*)data" by the user.
 */
bool Preferences::get( const char *key, void *&data, const void *defaultValue, int defaultSize )
{
  const char *v = node->get( key );
  if ( v )
  {
    int dsize;
    data = (void*)decodeHex( v, dsize );
    return true;
  }    
  if ( defaultValue )
  {
    data = new char[defaultSize];
    memmove( data, defaultValue, defaultSize );
  }
  else
    data = 0;
  return false;
}


/**
 * set an entry (name/value pair)
 */
bool Preferences::set( const char *key, const void *data, int dsize )
{
  char *buffer = new char[dsize*2+1];
  char *d = buffer;
  unsigned char *s = (unsigned char*)data;
  for ( ; dsize>0; dsize-- )
  {
    static char lu[] = "0123456789abcdef";
    unsigned char v = *s++;
    *d++ = lu[v>>4];
    *d++ = lu[v&0xf];
  }
  *d = 0;
  node->set( key, buffer );
  delete[] buffer;
  return true;
}


/**
 * return the size of the value part of an entry
 */
int Preferences::size( const char *key )
{
  const char *v = node->get( key );
  return v ? strlen( v ) : 0 ;
}

/**
 * creates a path that is related to the preferences file
 * and that is usable for application data beyond what is covered 
 * by Preferences.
 * - 'getUserdataPath' actually creates the directory
 * - 'path' must be large enough to receive a complete file path
 * example:
 *   Preferences prefs( USER, "matthiasm.com", "test" );
 *   char path[PATH_MAX];
 *   prefs.getUserdataPath( path );
 * sample returns:
 *   Win32: c:/Documents and Settings/matt/Application Data/matthiasm.com/test/
 *   prefs: c:/Documents and Settings/matt/Application Data/matthiasm.com/test.prefs
 */
bool Preferences::getUserdataPath( char *path, int pathlen )
{
  if ( rootNode )
    return rootNode->getPath( path, pathlen );
  return false;
}

/**
 * write all preferences to disk
 * - this function works only with the base preference group
 * - this function is rarely used as deleting the base preferences flushes automatically
 */
void Preferences::flush()
{
  if ( rootNode && node->dirty() )
    rootNode->write();
}

//-----------------------------------------------------------------------------
/** \class Preferences::Name
  Helper class to create group and entry names on the fly. Use an instance
  as the first argument to the Preferences::set() command.
*/

/**
  Converts an integer into an entry name. Example:
\code
Preferences prev( appPrefs, "PreviousFiles" );
int n; prev.get( "n", n, 0 );
for ( int i=0; i<n; i++ )
  prev.get( Name(i), prevFile[i], "" );
\endcode
*/
Preferences::Name::Name( int n )
{
  char buf[20];
  snprintf(buf, 20, "%d", n);
  data_ = newstring(buf);
}

/**
  Uses printf output to create the entry name. Example:
\code
Preferences prefs( USER, "matthiasm.com", "test" );
int n; prev.get( "nFiles", n, 0 );
for ( int i=0; i<n; i++ )
  prev.get( Name( "File%d", i ), prevFile[i], "" );
\endcode
*/
Preferences::Name::Name( const char *format, ... )
{
  char buf[1024];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, 1024, format, args);
  va_end(args);
  data_ = newstring(buf);
}

//-----------------------------------------------------------------------------
// internal methods, do not modify or use as they will change without notice
//

int Preferences::Node::lastEntrySet = -1;

// recursively create a path in the file system
static char makePath( const char *path ) {
  if (access(path, 0)) {
    const char *s = strrchr( path, '/' );
    if ( !s ) return 0;
    int len = s-path;
    char *p = new char[len+1];
    memcpy( p, path, len );
    p[len] = 0;
    makePath( p );
    delete[] p;
#if defined(_WIN32) && !defined(__CYGWIN__)
    return ( mkdir( path ) == 0 );
#else
    return ( mkdir( path, 0777 ) == 0 );
#endif // _WIN32 && !__CYGWIN__
  }
  return 1;
}

// strip the filename and create a path
static void makePathForFile( const char *path )
{
  const char *s = strrchr( path, '/' );
  if ( !s ) return;
  int len = s-path;
  char *p = new char[len+1];
  memcpy( p, path, len );
  p[len] = 0;
  makePath( p );
  delete[] p;
}

// create the root node
// - construct the name of the file that will hold our preferences
Preferences::RootNode::RootNode( Preferences *prefs, Root root, const char *vendor, const char *application )
{
  char filename[ PATH_MAX ]; filename[0] = 0;
#if defined(_WIN32) && !defined(__CYGWIN__)
#  define FLPREFS_RESOURCE	"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
  int appDataLen = strlen(vendor) + strlen(application) + 8;
  DWORD type, nn;
  LONG err;
  HKEY key;

  switch (root) {
    case SYSTEM:
      err = RegOpenKey( HKEY_LOCAL_MACHINE, FLPREFS_RESOURCE, &key );
      if (err == ERROR_SUCCESS) {
	nn = PATH_MAX - appDataLen;
	err = RegQueryValueEx( key, "Common AppData", 0L, &type, (BYTE*)filename, &nn );
	if ( ( err != ERROR_SUCCESS ) && ( type == REG_SZ ) )
	  filename[0] = 0;
        RegCloseKey(key);
      }
      break;
    case USER:
      err = RegOpenKey( HKEY_CURRENT_USER, FLPREFS_RESOURCE, &key );
      if (err == ERROR_SUCCESS) {
	nn = PATH_MAX - appDataLen;
	err = RegQueryValueEx( key, "AppData", 0L, &type, (BYTE*)filename, &nn );
	if ( ( err != ERROR_SUCCESS ) && ( type == REG_SZ ) )
	{
	  err = RegQueryValueEx( key, "Personal", 0L, &type, (BYTE*)filename, &nn );
	  if ( ( err != ERROR_SUCCESS ) && ( type == REG_SZ ) )
	    filename[0] = 0;
	}
        RegCloseKey(key);
      }
      break;
  }

  if (!filename[0]) {
    strcpy(filename, "C:\\FLTK");
  }

  snprintf(filename + strlen(filename), sizeof(filename) - strlen(filename),
           "/%s/%s.prefs", vendor, application);
  for (char *s = filename; *s; s++) if (*s == '\\') *s = '/';
#elif defined ( __APPLE__ )
  FSSpec spec = { 0 };
  FSRef ref;
  OSErr err = fnfErr;
  switch (root) {
    case SYSTEM:
      err = FindFolder( kLocalDomain, kPreferencesFolderType,
			1, &spec.vRefNum, &spec.parID );
      break;
    case USER:
      err = FindFolder( kUserDomain, kPreferencesFolderType, 
			1, &spec.vRefNum, &spec.parID );
      break;
  }
  FSpMakeFSRef( &spec, &ref );
  FSRefMakePath( &ref, (UInt8*)filename, PATH_MAX );
  snprintf(filename + strlen(filename), sizeof(filename) - strlen(filename),
           "/%s/%s.prefs", vendor, application );
#else
  const char *e;
  switch (root) {
    case USER:
      if ((e = getenv("HOME")) != NULL) {
	strlcpy(filename, e, sizeof(filename));

	if (filename[strlen(filename)-1] != '/') {
	  strlcat(filename, "/.fltk/", sizeof(filename));
	} else {
	  strlcat(filename, ".fltk/", sizeof(filename));
	}
	break;
      }

    case SYSTEM:
      strcpy(filename, "/etc/fltk/");
      break;
  }

  snprintf(filename + strlen(filename), sizeof(filename) - strlen(filename),
           "%s/%s.prefs", vendor, application);
#endif

  prefs_       = prefs;
  filename_    = newstring(filename);
  vendor_      = newstring(vendor);
  application_ = newstring(application);

  read();
}

// create the root node
// - construct the name of the file that will hold our preferences
Preferences::RootNode::RootNode( Preferences *prefs, const char *path, const char *vendor, const char *application )
{
  char filename[ PATH_MAX ]; filename[0] = 0;

  snprintf(filename, sizeof(filename), "%s/%s.prefs", path, application);

  prefs_       = prefs;
  filename_    = newstring(filename);
  vendor_      = newstring(vendor);
  application_ = newstring(application);

  read();
}

// destroy the root node and all depending nodes
Preferences::RootNode::~RootNode()
{
  if ( prefs_->node->dirty() )
    write();
  delete[] filename_;
  delete[] vendor_;
  delete[] application_;
  delete prefs_->node;
}

// read a preferences file and construct the group tree and with all entry leafs
int Preferences::RootNode::read()
{
  char buf[1024];
  FILE *f = fopen( filename_, "rb" );
  if ( !f ) return 0;
  if(fgets( buf, 1024, f ));
  if(fgets( buf, 1024, f ));
  if(fgets( buf, 1024, f )); //ignore these return values from fgets()
  Node *nd = prefs_->node;
  for (;;)
  {
    if ( !fgets( buf, 1024, f ) ) break;	// EOF or Error
    if ( buf[0]=='[' ) // read a new group
    {
      int end = strcspn( buf+1, "]\n\r" );
      buf[ end+1 ] = 0;
      nd = prefs_->node->find( buf+1 );
    }
    else if ( buf[0]=='+' ) // 
    { // value of previous name/value pair spans multiple lines
      int end = strcspn( buf+1, "\n\r" );
      if ( end != 0 ) // if entry is not empty
      {
	buf[ end+1 ] = 0;
	nd->add( buf+1 );
      }
    }
    else // read a name/value pair
    {
      int end = strcspn( buf, "\n\r" );
      if ( end != 0 ) // if entry is not empty
      {
	buf[ end ] = 0;
	nd->set( buf );
      }
    }
  }
  fclose( f );
  return 0;
}

// write the group tree and all entry leafs
int Preferences::RootNode::write()
{
  makePathForFile(filename_);
  FILE *f = fopen( filename_, "wb" );
  if ( !f ) return 1;
  fprintf( f, "; FLTK preferences file format 1.0\n" );
  fprintf( f, "; vendor: %s\n", vendor_ );
  fprintf( f, "; application: %s\n", application_ );
  prefs_->node->write( f );
  fclose( f );
  return 0;
}

// get the path to the preferences directory
bool Preferences::RootNode::getPath( char *path, int pathlen )
{
  strlcpy( path, filename_, pathlen);

  char *s;
  for ( s = path; *s; s++ ) if ( *s == '\\' ) *s = '/';
  s = strrchr( path, '.' );
  if ( !s ) return 0;
  *s = 0;
  bool ret = makePath( path ) ? true : false;
  strcpy( s, "/" );
  return ret;
}

// create a node that represents a group
// - path must be a single word, prferable alnum(), dot and underscore only. Space is ok.
Preferences::Node::Node( const char *path )
{
  path_ = newstring( path );
  child_ = 0; next_ = 0; parent_ = 0;
  entry = 0;
  nEntry = NEntry = 0;
  dirty_ = 0;
}

// delete this and all depending nodes
Preferences::Node::~Node()
{
  Node *nx;
  for ( Node *nd = child_; nd; nd = nx )
  {
    nx = nd->next_;
    delete nd;
  }
  if ( entry )
  {
    for ( int i = 0; i < nEntry; i++ )
    {
      delete[] entry[i].name;
      delete[] entry[i].value;
    }
    delete[] entry;
  }
  delete[] path_;
}

// recursively check if any entry is dirty (was changed after loading a fresh prefs file)
bool Preferences::Node::dirty()
{
  if ( dirty_ ) return true;
  if ( next_ && next_->dirty() ) return true;
  if ( child_ && child_->dirty() ) return true;
  return false;
}

// write this node (recursively from the last neighbor back to this)
// write all entries
// write all children
int Preferences::Node::write( FILE *f )
{
  if ( next_ ) next_->write( f );
  fprintf( f, "\n[%s]\n\n", path_ );
  for ( int i = 0; i < nEntry; i++ )
  {
    char *src = entry[i].value;
    if ( src )
    { // hack it into smaller pieces if needed
      fprintf( f, "%s:", entry[i].name );
      int cnt;
      for ( cnt = 0; cnt < 60; cnt++ )
	if ( src[cnt]==0 ) break;
      fwrite( src, cnt, 1, f );
      fprintf( f, "\n" );
      src += cnt;
      for (;*src;)
      {
	for ( cnt = 0; cnt < 80; cnt++ )
	  if ( src[cnt]==0 ) break;
        fputc( '+', f );
	fwrite( src, cnt, 1, f );
        fputc( '\n', f );
	src += cnt;
      }
    }
    else
      fprintf( f, "%s\n", entry[i].name );
  }
  if ( child_ ) child_->write( f );
  dirty_ = 0;
  return 0;
}

// set the parent node and create the full path
void Preferences::Node::setParent( Node *pn )
{
  parent_ = pn;
  next_ = pn->child_;
  pn->child_ = this;
  char nameBuffer[128];
  sprintf( nameBuffer, "%s/%s", pn->path_, path_ );
  delete[] path_;
  path_ = newstring( nameBuffer );
}

// add a child to this node and set its path (try to find it first...)
Preferences::Node *Preferences::Node::addChild( const char *path )
{
  char nameBuffer[128];
  sprintf( nameBuffer, "%s/%s", path_, path );
  char *name = newstring( nameBuffer );
  Node *nd = find( name );
  delete[] name;
  dirty_ = 1;
  return nd;
}

// create and set, or change an entry within this node
void Preferences::Node::set( const char *name, const char *value )
{
  for ( int i=0; i<nEntry; i++ )
  {
    if ( strcmp( name, entry[i].name ) == 0 )
    {
      if ( !value ) return; // annotation
      if ( strcmp( value, entry[i].value ) != 0 )
      {
	delete[] entry[i].value;
	entry[i].value = newstring( value );
	dirty_ = 1;
      }
      lastEntrySet = i;
      return;
    }
  }
  if ( NEntry==nEntry )
  {
    NEntry = NEntry ? NEntry*2 : 10;
    Entry* newarray = new Entry[NEntry];
    if (entry) memcpy(newarray, entry, nEntry*sizeof(Entry));
    entry = newarray;
  }
  entry[ nEntry ].name = newstring( name );
  entry[ nEntry ].value = newstring( value );
  lastEntrySet = nEntry;
  nEntry++;
  dirty_ = 1;
}

// create or set a value (or annotation) from a single line in the file buffer
void Preferences::Node::set( const char *line )
{
  // hmm. If we assume that we always read this file in the beginning,
  // we can handle the dirty flag 'quick and dirty'
  bool dirt = dirty_;
  if ( line[0]==';' || line[0]==0 || line[0]=='#' )
  {
    set( line, 0 );
  }
  else
  {
    const char *c = strchr( line, ':' );
    if ( c ) {
      char nameBuffer[128];
      strlcpy( nameBuffer, line, c-line+1);
      set( nameBuffer, c+1 );
    }
    else
      set( line, "" );
  }
  dirty_ = dirt;
}

// add more data to an existing entry
void Preferences::Node::add( const char *line )
{
  if ( lastEntrySet<0 || lastEntrySet>=nEntry ) return;
  char *&dst = entry[ lastEntrySet ].value;
  int a = strlen( dst );
  int b = strlen( line );
  char* newdst = new char[a+b+1];
  memcpy( newdst, dst, a);
  strcpy( newdst+a, line );
  delete[] dst;
  dst = newdst;
  dirty_ = 1;
}

// get the value for a name, returns 0 if no such name
const char *Preferences::Node::get( const char *name )
{
  int i = getEntry( name );
  return i>=0 ? entry[i].value : 0 ;
}

// find the index of an entry, returns -1 if no such entry
int Preferences::Node::getEntry( const char *name )
{
  for ( int i=0; i<nEntry; i++ )
  {
    if ( strcmp( name, entry[i].name ) == 0 )
    {
      return i;
    }
  }
  return -1;
}

// remove one entry form this group
bool Preferences::Node::deleteEntry( const char *name )
{
  int ix = getEntry( name );
  if ( ix == -1 ) return false;
  memmove( entry+ix, entry+ix+1, (nEntry-ix-1) * sizeof(Entry) );
  nEntry--;
  dirty_ = 1;
  return true;
}

// find a group somewhere in the tree starting here
// - this method will always return a valid node (except for memory allocation problems)
// - if the node was not found, 'find' will create the required branch
Preferences::Node *Preferences::Node::find( const char *path )
{
  int len = strlen( path_ );
  if ( strncmp( path, path_, len ) == 0 )
  {
    if ( path[ len ] == 0 ) 
      return this;
    if ( path[ len ] == '/' )
    {
      Node *nd;
      for ( nd = child_; nd; nd = nd->next_ )
      {
	Node *nn = nd->find( path );
	if ( nn ) return nn;
      }
      const char *s = path+len+1;
      const char *e = strchr( s, '/' );
      char nameBuffer[128];
      if (e) strlcpy( nameBuffer, s, e-s+1 );
      else strlcpy( nameBuffer, s, sizeof(nameBuffer));
      nd = new Node( nameBuffer );
      nd->setParent( this );
      return nd->find( path );
    }
  }
  return 0;
}

// find a group somewhere in the tree starting here
// caller must not set 'offset' argument
// - if the node does not exist, 'search' returns NULL
// - if the pathname is "." (current node) return this node
// - if the pathname is "./" (root node) return the topmost node
// - if the pathname starts with "./", start the search at the root node instead
Preferences::Node *Preferences::Node::search( const char *path, int offset )
{

  if ( offset == 0 )
  {
    if ( path[0] == '.' )
    {
      if ( path[1] == 0 )
      {
	return this; // user was searching for current node
      }
      else if ( path[1] == '/' )
      {
	Node *nn = this;
	while ( nn->parent_ ) nn = nn->parent_;
	if ( path[2]==0 )
	{ // user is searching for root ( "./" )
	  return nn;
	}
	return nn->search( path+2, 2 ); // do a relative search on the root node
      }
    }
    offset = strlen( path_ ) + 1;
  }
  
  int len = strlen( path_ );
  if ( len < offset-1 ) return 0;
  len -= offset;
  if ( ( len <= 0 ) || ( strncmp( path, path_+offset, len ) == 0 ) )
  {
    if ( len > 0 && path[ len ] == 0 ) 
      return this;
    if ( len <= 0 || path[ len ] == '/' )
    {
      for ( Node *nd = child_; nd; nd = nd->next_ )
      {
	Node *nn = nd->search( path, offset );
	if ( nn ) return nn;
      }
      return 0;
    }
  }
  return 0;
}

// return the number of child nodes (groups)
int Preferences::Node::nChildren()
{
  int cnt = 0;
  for ( Node *nd = child_; nd; nd = nd->next_ )
    cnt++;
  return cnt;
}

// return the n'th child node
const char *Preferences::Node::child( int ix )
{
  Node *nd;
  for ( nd = child_; nd; nd = nd->next_ )
  {
    if ( !ix-- ) break;
  }
  if ( nd && nd->path_ )
  {
    char *r = strrchr( nd->path_, '/' );
    return r ? r+1 : nd->path_ ;
  }
  return 0L ;
}

// remove myself from the list and delete me (and all children)
bool Preferences::Node::remove()
{
  Node *nd = 0, *np;
  if ( parent_ )
  {
    nd = parent_->child_; np = 0L;
    for ( ; nd; np = nd, nd = nd->next_ )
    {
      if ( nd == this )
      {
	if ( np ) 
	  np->next_ = nd->next_; 
	else 
	  parent_->child_ = nd->next_;
	break;
      }
    }
    parent_->dirty_ = 1;
  }
  delete this;
  return ( nd != 0 );
}


//
// End of "$Id: Preferences.cxx 4660 2005-11-27 14:45:48Z mike $".
//
