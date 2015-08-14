// $Id: Flu_File_Chooser.cpp,v 1.98 2004/11/02 00:33:31 jbryan Exp $

/***************************************************************
 *                FLU - FLTK Utility Widgets
 *  Copyright (C) 2002 Ohio Supercomputer Center, Ohio State University
 *
 * This file and its content is protected by a software license.
 * You should have received a copy of this license with this file.
 * If not, please contact the Ohio Supercomputer Center immediately:
 * Attn: Jason Bryan Re: FLU 1224 Kinnear Rd, Columbus, Ohio 43212
 *
 ***************************************************************/

#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

#include <iostream>
using namespace std;


#include <errno.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <limits>
#include <vector>
#include <string>
#include <algorithm>

#define FLU_USE_REGISTRY

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <lmcons.h>
#endif

#if defined WIN32 && !defined CYGWIN
#include <direct.h>
typedef __int64 int64_t;
#else
#include <unistd.h>
#endif

#include <fltk/draw.h>
#include <fltk/ask.h>
#include <fltk/events.h>
#include <fltk/Color.h>
#include <fltk/Cursor.h>
#include <fltk/Item.h>
#include <fltk/filename.h>
#include <fltk/xpmImage.h>
#include <fltk/Scrollbar.h>
#include <fltk/ScrollGroup.h>
#include <fltk/SharedImage.h>
#include "fltk/InvisibleBox.h"
#include "fltk/HighlightButton.h"
#include <fltk/LabelType.h>
#include <fltk/ItemGroup.h>
#include "fltk/utf.h" 
#include "fltk/events.h"  // for timeout methods
#include "fltk/run.h"  // for timeout methods

#include <boost/thread.hpp>

#include "flu_pixmaps.h"
#include "Flu_File_Chooser.h"
#include "flu_file_chooser_pixmaps.h"

#include "core/mrvHome.h"
#include "core/Sequence.h"
#include "core/mrvI8N.h"
#undef fprintf
#include "gui/mrvIO.h"

using namespace fltk;

static const char* kModule = "filereq";


// set default language strings
std::string Flu_File_Chooser::favoritesTxt = _("Favorites");
#ifdef WIN32
std::string Flu_File_Chooser::myComputerTxt = _("My Computer");
std::string Flu_File_Chooser::myDocumentsTxt = _("My Documents");
std::string Flu_File_Chooser::desktopTxt = _("Desktop");
#else
std::string Flu_File_Chooser::myComputerTxt = _("Home");
std::string Flu_File_Chooser::myDocumentsTxt = _("Temporary");
std::string Flu_File_Chooser::desktopTxt = _("Desktop");
#endif

std::string Flu_File_Chooser::detailTxt[] = { _("Name"), _("Size"), _("Date"), _("Type"), _("Frames") };
std::string Flu_File_Chooser::contextMenuTxt[3] = { _("New Folder"), _("Rename"), _("Delete") };
std::string Flu_File_Chooser::diskTypesTxt[6] = { _("Floppy Disk"), _("Removable Disk"),
						  _("Local Disk"), _("Compact Disk"),
						  _("Network Disk"), _("RAM Disk") };

std::string Flu_File_Chooser::filenameTxt = _("Filename");
std::string Flu_File_Chooser::okTxt = _("Ok");
std::string Flu_File_Chooser::cancelTxt = _("Cancel");
std::string Flu_File_Chooser::locationTxt = _("Location");
std::string Flu_File_Chooser::showHiddenTxt = _("Show Hidden Files");
std::string Flu_File_Chooser::fileTypesTxt = _("File Types");
std::string Flu_File_Chooser::directoryTxt = _("Directory");
std::string Flu_File_Chooser::allFilesTxt = _("All Files (*)");
std::string Flu_File_Chooser::defaultFolderNameTxt = _("New Folder");

std::string Flu_File_Chooser::backTTxt = _("Go back one directory in the history");
std::string Flu_File_Chooser::forwardTTxt = _("Go forward one directory in the history");
std::string Flu_File_Chooser::upTTxt = _("Go to the parent directory");
std::string Flu_File_Chooser::reloadTTxt = _("Refresh this directory");
std::string Flu_File_Chooser::trashTTxt = _("Delete file(s)");
std::string Flu_File_Chooser::newDirTTxt = _("Create new directory");
std::string Flu_File_Chooser::addFavoriteTTxt = _("Add this directory to my favorites");
std::string Flu_File_Chooser::previewTTxt = _("Preview files");
std::string Flu_File_Chooser::listTTxt = _("List mode");
std::string Flu_File_Chooser::wideListTTxt = _("Wide list mode");
std::string Flu_File_Chooser::detailTTxt = _("Detail mode");

std::string Flu_File_Chooser::createFolderErrTxt = _("Could not create directory '%s'. You may not have permission to perform this operation.");
std::string Flu_File_Chooser::deleteFileErrTxt = _("An error ocurred while trying to delete '%s'.");
std::string Flu_File_Chooser::fileExistsErrTxt = _("File '%s' already exists!");
std::string Flu_File_Chooser::renameErrTxt = _("Unable to rename '%s' to '%s'");
bool Flu_File_Chooser::singleButtonTravelDrawer = true;

const char *col_labels[] = {
  Flu_File_Chooser::detailTxt[0].c_str(),
  Flu_File_Chooser::detailTxt[3].c_str(),
  Flu_File_Chooser::detailTxt[1].c_str(),
  Flu_File_Chooser::detailTxt[2].c_str(),
  0
};

int col_widths[]   = {200, 70, 70, 141};


// just a string that no file could probably ever be called
#define FAVORITES_UNIQUE_STRING   "\t!@#$%^&*(Favorites)-=+"

#define DEFAULT_ENTRY_WIDTH 235

fltk::xpmImage up_folder_img( (char*const*)big_folder_up_xpm ),
  trash( (char*const*)trash_xpm ),
  new_folder( (char*const*)big_folder_new_xpm ),
  reload( (char*const*)reload_xpm ),
  preview_img( (char*const*)monalisa_xpm ),
  file_list_img( (char*const*)filelist_xpm ),
  file_listwide_img( (char*const*)filelistwide_xpm ),
  fileDetails( (char*const*)filedetails_xpm ),
  add_to_favorite_folder( (char*const*)folder_favorite_xpm ),
  home( (char*const*)bighome_xpm ),
  favorites( (char*const*)bigfavorites_xpm ),
  desktop( (char*const*)desktop_xpm ),
  folder_closed( (char*const*)folder_closed_xpm ),
  default_file( (char*const*)textdoc_xpm ),
  my_computer( (char*const*)my_computer_xpm ),
  computer( (char*const*)computer_xpm ),
  disk_drive( (char*const*)disk_drive_xpm ),
  cd_drive( (char*const*)cd_drive_xpm ),
  floppy_drive( (char*const*)floppy_drive_xpm ),
  removable_drive( (char*const*)removable_drive_xpm ),
  ram_drive( (char*const*)ram_drive_xpm ),
  network_drive( (char*const*)network_drive_xpm ),
  documents( (char*const*)filled_folder_xpm ),
  littlehome( (char*const*)home_xpm ),
  little_favorites( (char*const*)mini_folder_favorites_xpm ),
  little_desktop( (char*const*)mini_desktop_xpm ),
  bigdocuments( (char*const*)bigdocuments_xpm ),
  bigtemporary( (char*const*)bigtemporary_xpm ),
  reel( (char*const*) reel_xpm ),
picture( (char*const*) image_xpm ),
music( (char*const*) music_xpm );

#define streq(a,b) (strcmp(a,b)==0)

Flu_File_Chooser::FileTypeInfo* Flu_File_Chooser::types = NULL;
int Flu_File_Chooser::numTypes = 0;
int Flu_File_Chooser::typeArraySize = 0;
Flu_File_Chooser::ContextHandlerVector Flu_File_Chooser::contextHandlers;
Flu_File_Chooser::PreviewHandlerVector Flu_File_Chooser::previewHandlers;
Flu_File_Chooser::ImgTxtPreview* Flu_File_Chooser::imgTxtPreview = 0;
int (*Flu_File_Chooser::customSort)(const char*,const char*) = 0;
// std::string dArrow = "  @C0xa08080@-12DnArrow";
// std::string uArrow = "  @C0xa08080@-18UpArrow";
std::string dArrow = "  @C0xa08080@-12->";
std::string uArrow = "  @C0xa08080@-18->";

#ifdef WIN32
// Internationalized windows folder name access
// Fix suggested by Fabien Costantini
/*
  CSIDL_DESKTOPDIRECTORY -- desktop
  CSIDL_PERSONAL -- my documents
  CSIDL_PERSONAL and strip back to last "/" -> home
 */
static std::string flu_get_special_folder( int csidl )
{
  static char path[MAX_PATH+1];

#ifdef FLU_USE_REGISTRY
  HKEY key;
  DWORD size = MAX_PATH;
  const char *keyQuery = "";
  switch( csidl )
    {
    case CSIDL_DESKTOPDIRECTORY: keyQuery = "Desktop"; break;
    case CSIDL_PERSONAL: keyQuery = "Personal"; break;
    }

  if( RegOpenKeyEx( HKEY_CURRENT_USER,
		    "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
		    0, KEY_QUERY_VALUE, &key ) != ERROR_SUCCESS )
    return "";

  if( RegQueryValueEx( key, keyQuery, 0, 0, (LPBYTE)path, &size ) != ERROR_SUCCESS )
    return "";

  RegCloseKey( key );

  return path;

#else

  path[0] = '\0';
  if( SUCCEEDED( SHGetSpecialFolderPath( NULL, path, csidl, FALSE ) ) )
    //if( SUCCEEDED( SHGetFolderPath( NULL, csidl, NULL, 0, path ) ) )
    {
      int len = strlen(path);
      if( len > 0 && path[len-1] != '/' && path[len-1] != '\\' )
	strcat( path, "/" );
      return path;
    }
  return "";
#endif
}
#endif

// taken explicitly from fltk/src/filename_match.cxx
// and changed to support case-sensitive matching
static int flu_filename_match(const char *s, const char *p,
			      bool downcase = true)
{
  int matched;

  for (;;) {
    switch(*p++) {

    case '?' :	// match any single character
      if (!*s++) return 0;
      break;

    case '*' :	// match 0-n of any characters
      if (!*p) return 1; // do trailing * quickly
      while (!flu_filename_match(s, p, downcase)) if (!*s++) return 0;
      return 1;

    case '[': {	// match one character in set of form [abc-d] or [^a-b]
      if (!*s) return 0;
      int reverse = (*p=='^' || *p=='!'); if (reverse) p++;
      matched = 0;
      char last = 0;
      while (*p) {
	if (*p=='-' && last) {
	  if (*s <= *++p && *s >= last ) matched = 1;
	  last = 0;
	} else {
	  if (*s == *p) matched = 1;
	}
	last = *p++;
	if (*p==']') break;
      }
      if (matched == reverse) return 0;
      s++; p++;}
    break;

    case '{' : // {pattern1|pattern2|pattern3}
    NEXTCASE:
       if (flu_filename_match(s,p,downcase)) return 1;
       for (matched = 0;;) {
	  switch (*p++) {
	     case '\\': if (*p) p++; break;
	     case '{': matched++; break;
	     case '}': if (!matched--) return 0; break;
	     case '|': case ',': if (matched==0) goto NEXTCASE;
	     case 0: return 0;
	  }
       }
    case '|':	// skip rest of |pattern|pattern} when called recursively
    case ',':
      for (matched = 0; *p && matched >= 0;) {
	switch (*p++) {
	case '\\': if (*p) p++; break;
	case '{': matched++; break;
	case '}': matched--; break;
	}
      }
      break;
    case '}':
      break;

    case 0:	// end of pattern
      return !*s;

    case '\\':	// quote next character
      if (*p) p++;
    default:
#ifdef _WIN32
      if (tolower(*s) != tolower(*(p-1))) return 0;
#else
      if ( downcase )
	 if (tolower(*s) != tolower(*(p-1))) return 0;
      else
	 if( *s != *(p-1) ) return 0;
#endif
      s++;
      break;
    }
  }
}

void Flu_File_Chooser::add_context_handler( int type, const char *ext,
					      const char *name,
					      void (*cb)(const char*,int,void*), void *cbd )
{
  if( cb == NULL )
    return;
  ContextHandler h;
  h.ext = ext ? ext : "";
  int (*pf)(int) = tolower;
  std::transform( h.ext.begin(), h.ext.end(), h.ext.begin(), pf );
  h.type = type;
  h.name = name;
  h.callback = cb;
  h.callbackData = cbd;
  Flu_File_Chooser::contextHandlers.push_back( h );
}

void Flu_File_Chooser::add_preview_handler( PreviewWidgetBase *w )
{
  if( w == NULL )
    return;
  Flu_File_Chooser::previewHandlers.push_back( w );
}

// extensions == NULL implies directories
void Flu_File_Chooser::add_type( const char *extensions,
				 const char *short_description,
				 fltk::Image *icon )
{
  std::string ext;
  if( extensions )
    ext = extensions;
  else
    ext = "\t"; // indicates a directory

  int (*pf)(int) = toupper;
  std::transform( ext.begin(), ext.end(), ext.begin(), pf );

  // are we overwriting an existing type?
  for( int i = 0; i < numTypes; i++ )
    {
      if( types[i].extensions == ext )
	{
	  types[i].icon = icon;
	  types[i].type = short_description;
	  return;
	}
    }

  if( numTypes == typeArraySize )
    {
      int newSize = ( typeArraySize == 0 ) ? 1 : typeArraySize*2; // double the size of the old list (same behavior as STL vector)
      // allocate the new list
      FileTypeInfo* newTypes = new FileTypeInfo[ newSize ];
      // copy the old list to the new list
      for( int i = 0; i < numTypes; i++ )
	{
	  newTypes[i].icon = types[i].icon;
	  newTypes[i].extensions = types[i].extensions;
	  newTypes[i].type = types[i].type;
	}
      // delete the old list and replace it with the new list
      delete[] types;
      types = newTypes;
      typeArraySize = newSize;
    }

  types[numTypes].icon = icon;
  types[numTypes].extensions = ext;
  types[numTypes].type = short_description;

  numTypes++;
}

Flu_File_Chooser::FileTypeInfo*
Flu_File_Chooser::find_type( const char *extension )
{
  std::string ext;
  if( extension )
    ext = extension;
  else
    ext = "\t"; // indicates a directory

  int (*pf)(int) = toupper;
  std::transform( ext.begin(), ext.end(), ext.begin(), pf );

  // lookup the type based on the extension
  for( int i = 0; i < numTypes; i++ )
    {
      // check extension against every token
      std::string e = types[i].extensions;
      char *tok = strtok( (char*)e.c_str(), " ," );
      while( tok )
	{
	  if( ext == tok )
	    return &(types[i]);
	  tok = strtok( NULL, " ," );
	}
    }

  return NULL;
}

Flu_File_Chooser::Flu_File_Chooser( const char *pathname,
				    const char *pat, int type,
				    const char *title,
				    const bool compact )
  : fltk::DoubleBufferWindow( 600, 480, title ),
    filename( 70, 0, w()-70-85-10, 25, "" ),
    ok( w()-90, 0, 85, 25 ),
    cancel( w()-90, 0, 85, 25 ),
    _compact( compact )
{
  int normal_size = 12;

  filename.set_chooser( this );

  defaultFileIcon = NULL;
  _callback = 0;
  _userdata = 0;
  fltk::DoubleBufferWindow::callback( _hideCB, this );

  fltk::DoubleBufferWindow::size_range( 600, 400 );

  fltk::Group *g;

  filename.label( filenameTxt.c_str() );
  ok.label( okTxt.c_str() );
  ok.labelsize( (float)normal_size );
  cancel.label( cancelTxt.c_str() );
  cancel.labelsize( (float)normal_size );

  add_type( NULL, directoryTxt.c_str(), &folder_closed );
  add_type( N_("mov"),   _( "Quicktime Movie"), &reel );
  add_type( N_("qt"),    _( "Quicktime Movie"), &reel );
  add_type( N_("avi"),   _( "AVI Movie"), &reel );
  add_type( N_("divx"),  _( "DIVX Movie"), &reel );
  add_type( N_("mpg"),   _( "MPEG Movie"), &reel );
  add_type( N_("mpeg"),  _( "MPEG Movie"), &reel );
  add_type( N_("wmv"),   _( "WMV Movie"), &reel );
  add_type( N_("vob"),   _( "VOB Movie"), &reel );
  add_type( N_("mp4"),   _( "MP4 Movie"), &reel );
  add_type( N_("flv"),   _( "Flash Movie"), &reel );
  add_type( N_("bmp"),   _( "Bitmap Picture"), &picture );
  add_type( N_("bit"),   _( "mental ray Bit Picture"), &picture );
  add_type( N_("cin"),   _( "Cineon Picture"), &picture );
  add_type( N_("ct"),    _( "mental ray Contour Picture"), &picture );
  add_type( N_("dpx"),   _( "DPX Picture"), &picture );
  add_type( N_("exr"),   _( "EXR Picture"), &picture );
  add_type( N_("tif"),   _( "TIFF Picture"), &picture );
  add_type( N_("iff"),   _( "IFF Picture"), &picture );
  add_type( N_("jpg"),   _( "JPEG Picture"), &picture );
  add_type( N_("jpeg"),  _( "JPEG Picture"), &picture );
  add_type( N_("map"),   _( "Map Picture"), &picture );
  add_type( N_("gif"),   _( "GIF Picture"), &picture );
  add_type( N_("nt"),    _( "mental ray Normal Picture"), &picture );
  add_type( N_("mt"),    _( "mental ray Motion Picture"), &picture );
  add_type( N_("pic"),   _( "Softimage Picture"), &picture );
  add_type( N_("png"),   _( "PNG Picture"), &picture );
  add_type( N_("psd"),   _( "Photoshop Picture"), &picture );
  add_type( N_("rgb"),   _( "RGB Picture"), &picture );
  add_type( N_("rpf"),   _( "Rich Picture Format"), &picture );
  add_type( N_("shmap"), _( "mental ray Shadow Map"), &picture );
  add_type( N_("sgi"),   _( "SGI Picture"), &picture );
  add_type( N_("st"),    _( "mental ray Scalar Picture"), &picture );
  add_type( N_("sxr"),   _( "Stereo EXR Picture"), &picture );
  add_type( N_("tga"),   _( "Targa Picture"), &picture );
  add_type( N_("tif"),   _( "TIFF Picture"), &picture );
  add_type( N_("tiff"),  _( "TIFF Picture"), &picture );
  add_type( N_("zt"),    _( "mental ray Z Depth Picture"), &picture );
  add_type( N_("mp3"),   _( "MP3 music"), &music );
  add_type( N_("ogg"),   _( "OGG music"), &music );
  add_type( N_("wav"),   _( "Wave music"), &music );

  history = currentHist = NULL;
  walkingHistory = false;
  fileEditing = false;
#ifdef WIN32
  refreshDrives = true;
  caseSort = false;
#else
  caseSort = true;
#endif

  userHome = mrv::homepath();

  // determine the system paths for the user's home area, desktop, documents, app data, etc
#ifdef _WIN32
  userDesktop = flu_get_special_folder( CSIDL_DESKTOPDIRECTORY );
  userDocs = flu_get_special_folder( CSIDL_PERSONAL );

  // construct the user desktop path
  //userDesktop = userHome + "/" + desktopTxt;

  win2unix( userDesktop );
  win2unix( userDocs );

  // make sure they don't end in '/'
  if( userDesktop[userDesktop.size()-1] == '/' )
    userDesktop[userDesktop.size()-1] = '\0';
  if( userDocs[userDocs.size()-1] == '/' )
    userDocs[userDocs.size()-1] = '\0';

  // get the actual name of the "My Documents" folder by pulling off the
  // last name in the field.
  // We do this because the actual name may vary from country to country
  {
    size_t slash = userDesktop.rfind( '/' );
    if( slash != -1 )
      desktopTxt = userDesktop.c_str() + slash + 1;
    slash = userDocs.rfind( '/' );
    if( slash != -1 )
      myDocumentsTxt = userDocs.c_str() + slash + 1;
  }

  // make sure they end in '/'
  userHome += "/";
  userDesktop += "/";
  userDocs += "/";

#else
  {
    userDesktop = userHome + "/" + desktopTxt + "/";
    userDocs = "/tmp/";
  }
#endif
  configFilename = userHome + "/.fltk/";

#if ( defined _WIN32 || defined MINGW ) && !defined CYGWIN
  mkdir( configFilename.c_str() );
#else
  mkdir( configFilename.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif

  configFilename += "filmaura/";

#if ( defined _WIN32 || defined MINGW ) && !defined CYGWIN
  mkdir( configFilename.c_str() );
#else
  mkdir( configFilename.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif

  configFilename += "mrViewer.favorites";

  selectionType = type;
  filenameEnterCallback = filenameTabCallback = false;
  sortMethod = SORT_NAME;

  lastSelected = NULL;
  filename.labelsize( 12 );
  filename.when( fltk::WHEN_ENTER_KEY_ALWAYS );
  filename.callback( _filenameCB, this );
  filename.value( "" );

  begin();

  fltk::Group *quickIcons = new fltk::Group( 5, 5, 100, h()-10-60 );
  quickIcons->box( fltk::DOWN_BOX );

  uchar cr,cg,cb;
  fltk::split_color( color(), cr, cg, cb);
  Color bg = fltk::color( uchar(cr/2), uchar(cg/2), uchar(cb/2));
  Color hi = fltk::color( uchar(cr*2), uchar(cg*2), uchar(cb*2));

  quickIcons->color( bg );

  quickIcons->begin();

  fltk::HighlightButton *desktopBtn = new fltk::HighlightButton( 30, 18, 50, 48 );
  desktopBtn->box( fltk::UP_BOX );
  desktopBtn->image( desktop );
  desktopBtn->highlight_color( hi );
  desktopBtn->callback( _desktopCB, this );
  {
      fltk::InvisibleBox *l = new fltk::InvisibleBox( 5, 62, 100, 20, _(desktopTxt.c_str()) );
    l->labelcolor( fltk::WHITE );
    l->align( fltk::ALIGN_CENTER );
  }

  fltk::HighlightButton *homeBtn = new fltk::HighlightButton( 30, 98, 50, 48 );
  homeBtn->box( fltk::UP_BOX );
  homeBtn->highlight_color( hi );
  homeBtn->callback( _homeCB, this );
  {
#ifdef WIN32
    fltk::InvisibleBox *l = new fltk::InvisibleBox( 5, 142, 100, 20,
						    _(myComputerTxt.c_str()) );
    homeBtn->image( my_computer );
#else
    fltk::InvisibleBox *l = new fltk::InvisibleBox( 5, 142, 100, 20,
						    _(myComputerTxt.c_str()) );
    homeBtn->image( home );
#endif
    l->labelcolor( fltk::WHITE );
    l->align( fltk::ALIGN_CENTER );
  }

  fltk::HighlightButton *documentsBtn = new fltk::HighlightButton( 30, 178, 50, 48 );
  documentsBtn->box( fltk::UP_BOX );
  documentsBtn->labelcolor( fltk::WHITE );
  documentsBtn->highlight_color( hi );
  documentsBtn->callback( _documentsCB, this );
  {
#ifdef _WIN32
    fltk::InvisibleBox *l = new fltk::InvisibleBox( 5, 222, 100, 20,
						    _(myDocumentsTxt.c_str()) );
    documentsBtn->image( &bigdocuments );
#else
    fltk::InvisibleBox *l = new fltk::InvisibleBox( 5, 222, 100, 20,
						    _(myDocumentsTxt.c_str()) );
    documentsBtn->image( &bigtemporary );
#endif
    l->labelcolor( fltk::WHITE );
    l->align( fltk::ALIGN_CENTER );
  }

  fltk::HighlightButton *favoritesBtn = new fltk::HighlightButton( 30, 258, 50, 48 );
  favoritesBtn->box( fltk::UP_BOX );
  favoritesBtn->image( favorites );
  favoritesBtn->highlight_color( hi );
  favoritesBtn->callback( _favoritesCB, this );
  {
      fltk::InvisibleBox *l = new fltk::InvisibleBox( 5, 302, 100, 20, _(favoritesTxt.c_str()) );
    l->labelcolor( fltk::WHITE );
    l->align( fltk::ALIGN_CENTER );
  }

  {
    fltk::Group* dummy = new fltk::Group( 5, h()-10-61, 100, 1 );
    quickIcons->resizable( dummy );
  }
  quickIcons->end();

  favoritesList = new fltk::InputBrowser( 0, 0, 0, 0 );
  favoritesList->hide();

  fltk::Group *dummy = new fltk::Group( 110, 0, w()-110, 70 );
  dummy->begin();

  locationQuickJump = new fltk::Group( 56, 5, w()-171, 8 );
  locationQuickJump->begin();
  locationQuickJump->box( fltk::NO_BOX );
  locationQuickJump->end();

  location = new Flu_Combo_Tree( 56, 15, w()-171, 22, locationTxt.c_str() );
  location->minh( 200 );
  location->maxh( 500 );
  // location->pop_height( 200 );
  // location->tree.all_branches_always_open( true );
#ifdef _WIN32
  // location->tree.show_root( false );
#endif
  // location->tree.show_connectors( false );
  // location->tree.horizontal_gap( -10 );
  // location->tree.show_leaves( false );
  location->callback( _locationCB, this );

  ////////////////////////////////////////////////////////////////

  g = new fltk::Group( 0, 40, w()-110, 30 ); // group enclosing all the buttons at top
  g->begin();

  hiddenFiles = new fltk::CheckButton( 0, 3, 130, 25, showHiddenTxt.c_str() );
  hiddenFiles->callback( reloadCB, this );
#ifdef _WIN32
  hiddenFiles->hide();
#endif

  backBtn = new fltk::HighlightButton( 175, 3, 25, 25, "@<-" );
  backBtn->labelcolor( fltk::color( 80, 180, 200 ) );
  backBtn->highlight_color( hi );
  backBtn->labelsize( 16 );
//   backBtn->box( fltk::FLAT_BOX );
  backBtn->callback( _backCB, this );
  backBtn->tooltip( backTTxt.c_str() );

  forwardBtn = new fltk::HighlightButton( 200, 3, 25, 25, "@->" );
  forwardBtn->labelcolor( fltk::color( 80, 180, 200 ) );
  forwardBtn->highlight_color( hi );
  forwardBtn->labelsize( 16 );
//   forwardBtn->box( fltk::FLAT_BOX );
  forwardBtn->callback( _forwardCB, this );
  forwardBtn->tooltip( forwardTTxt.c_str() );

  upDirBtn = new fltk::HighlightButton( 225, 3, 25, 25 );
  upDirBtn->image( up_folder_img );
//   upDirBtn->box( fltk::FLAT_BOX );
  upDirBtn->highlight_color( hi );
  upDirBtn->callback( upDirCB, this );
  upDirBtn->tooltip( upTTxt.c_str() );

  reloadBtn = new fltk::HighlightButton( 250, 3, 25, 25 );
  reloadBtn->image( reload );
//   reloadBtn->box( fltk::FLAT_BOX );
  reloadBtn->highlight_color( hi );
  reloadBtn->callback( reloadCB, this );
  reloadBtn->tooltip( reloadTTxt.c_str() );

  //@todo
//   {
//     Flu_Separator *sep = new Flu_Separator( 275, 2, 10, 28 );
//     sep->type( Flu_Separator::VERTICAL );
//     sep->box( fltk::ENGRAVED_BOX );
//   }

  trashBtn = new fltk::HighlightButton( 285, 3, 25, 25 );
  trashBtn->image( trash );
//   trashBtn->box( fltk::FLAT_BOX );
  trashBtn->highlight_color( hi );
  trashBtn->callback( _trashCB, this );
  trashBtn->tooltip( trashTTxt.c_str() );

  newDirBtn = new fltk::HighlightButton( 310, 3, 25, 25 );
  newDirBtn->image( new_folder );
//   newDirBtn->box( fltk::FLAT_BOX );
  newDirBtn->highlight_color( hi );
  newDirBtn->callback( _newFolderCB, this );
  newDirBtn->tooltip( newDirTTxt.c_str() );

  addFavoriteBtn = new fltk::HighlightButton( 335, 3, 25, 25 );
  addFavoriteBtn->image( add_to_favorite_folder );
//   addFavoriteBtn->box( fltk::FLAT_BOX );
  addFavoriteBtn->highlight_color( hi );
  addFavoriteBtn->callback( _addToFavoritesCB, this );
  addFavoriteBtn->tooltip( addFavoriteTTxt.c_str() );

  //@todo
//   {
//     Flu_Separator *sep = new Flu_Separator( 360, 2, 10, 28 );
//     sep->type( Flu_Separator::VERTICAL );
//     sep->box( fltk::ENGRAVED_BOX );
//   }

  previewBtn = new fltk::ToggleButton( 372, 3, 23, 25 );
  previewBtn->image( preview_img );
  previewBtn->callback( _previewCB, this );
  previewBtn->tooltip( previewTTxt.c_str() );

  {
    fltk::Group *g2 = new fltk::Group( 401, 3, 81, 25 );
    g2->begin();
    listMode = kDetails;
    fileListBtn = new fltk::ToggleButton( 0, 0, 25, 25 );
    fileListBtn->callback( _listModeCB, this );
    fileListBtn->image( file_list_img );
    fileListBtn->tooltip( listTTxt.c_str() );
    fileListWideBtn = new fltk::ToggleButton( 29, 0, 25, 25 );
    fileListWideBtn->callback( _listModeCB, this );
    fileListWideBtn->image( file_listwide_img );
    fileListWideBtn->tooltip( wideListTTxt.c_str() );
    fileDetailsBtn = new fltk::ToggleButton( 58, 0, 25, 25 );
    fileDetailsBtn->image( fileDetails );
    fileDetailsBtn->callback( _listModeCB, this );
    fileDetailsBtn->value(1);
    fileDetailsBtn->tooltip( detailTTxt.c_str() );
    g2->end();
  }

  g->resizable( hiddenFiles );
  g->end();

  dummy->resizable( location );
  dummy->end();

  ////////////////////////////////////////////////////////////////

  previewTile = new PreviewTile( 110, 70, w()-110-5, h()-80-40-15, this );
  previewTile->begin();
  fileGroup = new fltk::Group( 0, 0, w()-115, h()-80-40-15 );
  {
    fileGroup->box( fltk::DOWN_BOX );
    fileGroup->begin();
    filelist = new FileList( 2, 2, fileGroup->w()-4, fileGroup->h()-4, this );
    filelist->box( fltk::FLAT_BOX );
    filelist->color( fltk::GRAY85 );

    filelist->type( fltk::ScrollGroup::HORIZONTAL );
    filelist->spacing( 4, 1 );

    // filelist->begin();
    // filelist->end();

    fileDetailsGroup = new fltk::Group( 2, 2, fileGroup->w()-4, 
					fileGroup->h()-4 );
    fileDetailsGroup->begin();
    {
      filedetails = new FileDetails( 0, 0, fileGroup->w()-4, 
				     fileGroup->h()-4, this );
      filedetails->box( fltk::FLAT_BOX );
      filedetails->column_labels( col_labels );
      filedetails->column_widths( col_widths );
      filedetails->end();
      filedetails->color( fltk::GRAY85 );
      filedetails->callback( _sortCB, this );
    }
    fileDetailsGroup->end();

    fileGroup->resizable( filelist );
  }
  fileGroup->end();

  previewGroup = new PreviewGroup( fileGroup->w(), 0,
				   previewTile->w()-fileGroup->w(),
				   fileGroup->h(), this );
  previewGroup->begin();
  previewGroup->end();

  previewTile->end();

  resizable( previewTile );


  ok.callback( _okCB, this );
  cancel.callback( _cancelCB, this );

  {
    g = new fltk::Group( 0, h()-60, w(), 30 );
    g->begin();
    g->add( filename );
    g->add( ok );
    g->resizable( filename );
    g->end();
    g = new fltk::Group( 0, h()-30, w(), 30 );
    g->begin();

    filePattern = new Flu_Combo_Tree( 70, 0, w()-70-85-10, 25, fileTypesTxt.c_str() );
    filePattern->type( InputBrowser::NONEDITABLE );
    filePattern->callback( reloadCB, this );
    filePattern->minh( 200 );
    filePattern->maxh( 200 );

    g->add( cancel );
    g->resizable( filePattern );
    g->end();
  }

  end();

  char buf[1024];

  // try to load the favorites
  {
    FILE *f = fltk::fltk_fopen( configFilename.c_str(), "r" );
    if( f )
      {
	buf[0] = '\0';
	while( !feof(f) )
	  {
	     char* d = fgets( buf, 1024, f );
	     if (d == NULL) continue;
	     char *newline = strrchr( buf, '\n' );
	     if( newline )
		*newline = '\0';
	     if( strlen( buf ) > 0 )
	     {
		// eliminate duplicates
		bool duplicate = false;
		for( int i = 0; i < favoritesList->size(); ++i )
		  {
		    if( streq( buf, favoritesList->child(i)->label() ) )
		      {
			duplicate = true;
			break;
		      }
		  }
		if( !duplicate )
		  {
		    favoritesList->add( buf );
		  }
	      }
	  }
	fclose( f );
      }
  }

  if( !imgTxtPreview )
    {
      imgTxtPreview = new ImgTxtPreview();
      // make the text previewer the first one
      Flu_File_Chooser::previewHandlers.insert( previewHandlers.begin(),
                                                imgTxtPreview );
    }

  pattern( pat );
  default_file_icon( &default_file );
  // cd( NULL ); // prime with the current directory
  clear_history();
  cd( pathname );

  fileDetailsBtn->redraw();
  fileDetailsBtn->do_callback();

  // if pathname does not start with "/" or "~", set the filename to it
  if( pathname &&
      ( strlen(pathname) > 1 && pathname[0] != '/' && pathname[0] != '~')
      && ( strlen(pathname) > 2 && pathname[1] != ':' ) )
     filename.value( pathname );
}

Flu_File_Chooser::~Flu_File_Chooser()
{
  //fltk::remove_timeout( Entry::_editCB );
  fltk::remove_timeout( Flu_File_Chooser::delayedCdCB );
  fltk::remove_timeout( Flu_File_Chooser::selectCB );

  for( int i = 0; i < locationQuickJump->children(); i++ )
    free( (void*)locationQuickJump->child(i)->label() );

  filelist->clear();
  filedetails->clear();

  clear_history();
  unselect_all();
}

void Flu_File_Chooser::hideCB()
{
  // the user hid the browser by pushing the "X"
  // this is the same as cancel
  cancelCB();
}

void Flu_File_Chooser::cancelCB()
{
  filename.value("");
  filename.position( filename.size(), filename.size() );
  unselect_all();
  do_callback();
  hide();
  lastSelected = NULL;
}

void Flu_File_Chooser::do_callback()
{
  if( _callback )
    _callback( this, _userdata );
}

void Flu_File_Chooser::pattern( const char *p )
{
  // just like in fltk::File_Chooser, we accept tab, |, and ; delimited strings like this:
  // "Description (patterns)" or just "patterns" where patterns is
  // of the form *.xxx or *.{xxx,yyy,zzz}}

  rawPattern = p;

  // clear out the old
  filePattern->clear();
  filePattern->text( "" );
  patterns.clear();

  if( p == 0 )
    p = "*";
  else if( p[0] == '\0' )
    p = "*";

  std::string pat = p, pattern;

  bool addedAll = false;
  const char *next = strtok( (char*)pat.c_str(), "\t|;" );
  const char *start;
  while( next )
    {
      if( next[0] == '\0' )
	break;

      // eat whitespace
      while( isspace( *next ) )
	next++;

      // degenerate check
      if( strcmp( next, "*" ) == 0 )
	{
	  addedAll = true;
	  filePattern->add( allFilesTxt.c_str() );
	  patterns.push_back( "*" );
	  next = strtok( NULL, "\t|;" );
	  continue;
	}

      // extract the patterns from the substring
      if( next[0] != '*' ) // starts with description
	{
	  // the pattern starts after the first '('
	  start = strchr( next, '(' );
	  if( !start ) // error: couldn't find the '('
	    {
	      next = strtok( NULL, "\t|;" );
	      continue;
	    }
	  start++; // skip the '('
	}
      else
	start = next;

      if( start[0] != '*' )
	{
	  next = strtok( NULL, "\t|;" );
	  continue;
	}
      start++; // skip the '*'

      if( start[0] != '.' )
	{
	  next = strtok( NULL, "\t|;" );
	  continue;
	}
      start++; // skip the '.'

      if( start[0] == '{' )
	{
	  // the pattern is between '{' and '}'
	  pattern = start+1;
	}
      else
	pattern = start;

      // remove the last '}'
      size_t brace = pattern.find( '}' );
      if( brace != std::string::npos )
	pattern[brace] = '\0';

      // remove the last ')'
      size_t paren = pattern.find( ')' );
      if( paren != std::string::npos )
	pattern[paren] = '\0';

      if( pattern.size() )
	{
	  // add the whole string to the list
	  filePattern->add( next );
	  patterns.push_back( pattern );
	}

      // advance to the pattern token
      next = strtok( NULL, "\t|;" );
   }

  // add all files
  if( !addedAll )
    {
      filePattern->add( allFilesTxt.c_str() );
      patterns.push_back( "*" );
    }

  // choose the first added item
  filePattern->value(0);
  filePattern->text( filePattern->child(0)->label() );
}

int Flu_File_Chooser::handle( int event )
{
  if( fltk::DoubleBufferWindow::callback() != _hideCB )
    {
      _callback = fltk::DoubleBufferWindow::callback();
      _userdata = fltk::DoubleBufferWindow::user_data();
      fltk::DoubleBufferWindow::callback( _hideCB, this );
    }

  if ( event == fltk::PUSH )
    {
      dragX = fltk::event_x();
      dragY = fltk::event_y();

    }

  if( fltk::DoubleBufferWindow::handle( event ) )
    {
      return 1;
    }
  else if( event == fltk::KEY && fltk::event_key() == fltk::EscapeKey )
    {
      cancel.do_callback();
      return 1;
    }
  else if( event == fltk::KEY && fltk::event_key() == 'a' &&
	   fltk::event_state(fltk::CTRL) )
    {
      select_all();
      return 1;
    }
  switch( event )
  {
     case fltk::DRAG:
	{
	   
	   if( selectionType & MULTI )
	   {
	      fltk::Group* g = getEntryGroup();
	      // toggle all items from the last selected item to this one
	      if( lastSelected != NULL )
	      {
		 
	      // get the index of the last selected item and this item
		 int lastindex = -1, thisindex = -1;
		 
		 int i;
	      for( i = 0; i < g->children(); i++ )
	      {
		 if( g->child(i) == lastSelected )
		    lastindex = i;
		 if( g->child(i) == this )
		    thisindex = i;
		 if( lastindex >= 0 && thisindex >= 0 )
		    break;
	      }
	      if( lastindex >= 0 && thisindex >= 0 )
		{
		   // loop from this item to the last item, 
		  // toggling each item except the last
		   int inc;
		   if( thisindex > lastindex )
		      inc = -1;
		   else
		      inc = 1;
		   Entry *e;
		   for( i = thisindex; i != lastindex; i += inc )
		   {
		      e = (Entry*)g->child(i);
		      e->selected()? e->clear_selected() : e->set_selected();
		      e->redraw();
		   }
		   // 		  lastSelected = this;
		   // 		  if( type == ENTRY_FILE || type == ENTRY_SEQUENCE )
		   // 		    previewGroup->file = currentDir + filename;
		   redraw();
		}
	      redraw();
	      if( selected() )
		 trashBtn->activate();
	      return 1;
	      }
	   }
	}
 
  }
  return 0;
}

void Flu_File_Chooser::newFolderCB()
{
  // start with the name "New Folder". while the name exists, keep appending a number (1..2..etc)
  std::string newName = defaultFolderNameTxt.c_str(), path = currentDir + newName;
  int count = 1;
  int i;
  for(;;)
    {
      bool found = false;
      // see if any entry already has that name
      fltk::Group *g = getEntryGroup();
      for( i = 0; i < g->children(); i++ )
	{
	  if( ((Entry*)g->child(i))->filename == newName )
	    {
	      found = true;
	      break;
	    }
	}

      // since an entry already exists, change the name and try again
      if( found )
	{
	  char buf[16];
	  sprintf( buf, "%d", count++ );
	  newName = defaultFolderNameTxt.c_str() + std::string(buf);
	  path = currentDir + newName;
	}
      else
	break;
    }

  // try to create the folder
#if ( defined WIN32 || defined MINGW ) && !defined CYGWIN
  if( mkdir( path.c_str() ) != 0 )
#else
  if( mkdir( path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) != 0 )
#endif
    {
      fltk::alert( createFolderErrTxt.c_str(), newName.c_str() );
      return;
    }

  // create a new entry with the name of the new folder. add to either the list or the details
  Entry *entry = new Entry( newName.c_str(), ENTRY_DIR, fileDetailsBtn->value(), this );
  if( !fileDetailsBtn->value() )
    filelist->add( *entry );
  else
    filedetails->add( *entry );

  // switch that entry to input mode and scroll the browser to it
  entry->editCB();
  /*
  entry->editMode = 2;
  entry->value( entry->filename.c_str() );
  entry->take_focus();
  entry->position( 0, entry->filename.size() );
  entry->redraw();
  */

  // @todo: verify scrollTo
//   if( !fileDetailsBtn->value() )
//     filelist->goto_index( filelist->children()-1 );
//     filelist->scrollTo( entry->x(), entry->y() );
//   else
//     filedetails->goto_index( filelist->children()-1 );
//    filedetails->scrollTo( entry->x(), entry->y() );
}

void Flu_File_Chooser::recursiveScan( const char *dir, FluStringVector *files )
{
  dirent **e;
  char *name;
  std::string fullpath;

  int num = fltk::filename_list( dir, &e );
  for( int i = 0; i < num; i++ )
    {
      name = e[i]->d_name;

      // if 'name' ends in '/' or '\', remove it
      if( name[strlen(name)-1] == '/' || name[strlen(name)-1] == '\\' )
	name[strlen(name)-1] = '\0';

      // ignore the "." and ".." names
      if( strcmp( name, "." ) == 0 || strcmp( name, ".." ) == 0 )
	continue;

      // file or directory?
      fullpath = dir;
      fullpath += "/";
      fullpath += name;
      if( fltk::filename_isdir( fullpath.c_str() ) != 0 )
	recursiveScan( fullpath.c_str(), files );

      files->push_back( fullpath );
    }
  files->push_back( dir );
}

void Flu_File_Chooser::trashCB( bool recycle )
{
  // linux doesn't have a recycle bin
#ifndef WIN32
  recycle = false;
#endif

  bool inFavorites = ( currentDir == FAVORITES_UNIQUE_STRING );
  if( inFavorites )
  {
    recycle = false;
  }

  // see how many files are selected
  std::string name;
  int selected = 0;
  int i;
  const char *first = "";
  fltk::Group *g = getEntryGroup();
  for( i = 0; i < g->children(); i++ )
    {
      if( ((Entry*)g->child(i))->selected() )
	{
	  if( selected == 0 )
	    first = ((Entry*)g->child(i))->filename.c_str();
	  selected++;
	}
    }

   if( selected )
     {
       if( selected == 1 )
	 {
	   if( recycle )
	     {
		if( !fltk::ask( _("Really send '%s' to the Recycle Bin?"), 
				first ) )
		 return;
	     }
	   else
	     {
		if( !fltk::ask( _("Really delete '%s'?"), first ) )
		 return;
	     }
	 }
       else
	 {
	   if( recycle )
	     {
		if( !fltk::ask( _("Really send these %d files to the Recycle Bin?"), selected ) )
		 return;
	     }
	   else
	     {
		if( !fltk::ask( _("Really delete these %d files?"), selected ) )
		 return;
	     }
	 }

       if( inFavorites )
	 {
	   for( i = 0; i < g->children(); )
	     {
	       Entry *e = ((Entry*)g->child(i));
	       if( e->selected() )
		 {
		   favoritesList->remove(i);
		   g->remove( *e );
		   delete e;
		 }
	       else
		 i++;
	     }
	   
	   // save the favorites
	   FILE *f = fltk::fltk_fopen( configFilename.c_str(), "w" );
	   if( f )
	     {
	       for( i = 0; i < favoritesList->children(); ++i )
                   if ( favoritesList->child(i)->label() == NULL ) continue;
		 fprintf( f, "%s\n", favoritesList->child(i)->label() );
	       fclose( f );
	     }
	   cd( FAVORITES_UNIQUE_STRING );
	   return;
	 }

#ifdef WIN32
       SHFILEOPSTRUCT fileop;
       memset( &fileop, 0, sizeof(SHFILEOPSTRUCT) );
       fileop.fFlags = FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION;
       if( recycle )
	 fileop.fFlags |= FOF_ALLOWUNDO;
       fileop.wFunc = FO_DELETE;
       fileop.pTo = NULL;
#endif

       for( i = 0; i < g->children(); i++ )
	 {
	   if( ((Entry*)g->child(i))->selected() )
	     {
	       int result = 0;

	       name = currentDir + ((Entry*)g->child(i))->filename;

	       // if directory, recursively remove
	       if( ((Entry*)g->child(i))->type == ENTRY_DIR )
		 {
		   // if we are recycling in windows, then the recursive part happens automatically
#ifdef _WIN32
		   if( !recycle )
#endif
		     {
		       fltk::Group::current(0);
		       fltk::Window *win = new fltk::Window( 200, 100, "Notice" );
		       win->begin();
		       fltk::InvisibleBox *label = new fltk::InvisibleBox( 30, 30, 150, 30, "Preparing to delete..." );
		       win->end();
		       win->show();
		       fltk::check();
		       // recursively build a list of all files that will be deleted
		       FluStringVector files;
		       recursiveScan( name.c_str(), &files );
		       // delete all the files
		       label->label( "Deleting files..." );
		       for( unsigned int i = 0; i < files.size(); i++ )
			 {
			   if( ::remove( files[i].c_str() ) != 0 )
			     {
			       win->hide();
			       delete win;
			       cd( "./" );
			       return;
			     }
			 }
		       win->hide();
		       delete win;
		       fltk::check();
		       continue;
		     }
		 }

#ifdef _WIN32
	       // this moves files to the recycle bin, depending on the value of 'recycle'
	       {
		 size_t len = name.size();
		 char *buf = (char*)malloc( len+2 );
		 strcpy( buf, name.c_str() );
		 buf[len+1] = '\0'; // have to have 2 '\0' at the end
		 fileop.pFrom = buf;
		 result = SHFileOperation( &fileop );
		 free( buf );
	       }
#else
	       result = ::remove( name.c_str() );
#endif

	       // if remove fails, report an error
	       if( result != 0 )
		 {
		   fltk::alert( deleteFileErrTxt.c_str(), name.c_str() );
		   cd( "./" );
		   return;
		 }
	     }
	 }

       // refresh this directory
       cd( "./" );
    }
}

void Flu_File_Chooser::updateLocationQJ()
{
  const char *path = location->text();
  for( int i = 0; i < locationQuickJump->children(); i++ )
    free( (void*)locationQuickJump->child(i)->label() );
  locationQuickJump->clear();

  fltk::setfont( location->textfont(), location->textsize() );

  const char *next = path;
  const char *slash = strchr( next, '/' );
  char *blank = strdup( path );
  int offset = 0;
  while( slash )
    {
      memset( blank, 0, strlen(path) );
      slash++;
      memcpy( blank, next, slash-next );
      int w = 0, h = 0;
      fltk::measure( blank, w, h );
      if( blank[0] == '/' )
	w += location->box()->dx();
      memset( blank, 0, strlen(path) );
      memcpy( blank, path, slash-path );
      fltk::Button *b = new fltk::Button( offset, 0, w, locationQuickJump->h(), strdup(blank) );
      b->labeltype( fltk::NO_LABEL );
      b->callback( _locationQJCB, this );
      offset += w;
      locationQuickJump->add( b );
      next = slash;
      slash = strchr( next, '/' );
    }
  fltk::Button *b = new fltk::Button( offset, 0, 1, locationQuickJump->h(), strdup("") );
  b->box( fltk::NO_BOX );
  b->labeltype( fltk::NO_LABEL );
  locationQuickJump->add( b );
  locationQuickJump->resizable( b );
  free( blank );
}

void Flu_File_Chooser::favoritesCB()
{
  cd( FAVORITES_UNIQUE_STRING );
}

void Flu_File_Chooser::myComputerCB()
{
  cd( "/" );
}

void Flu_File_Chooser::documentsCB()
{
  cd( userDocs.c_str() );
}

Flu_File_Chooser::FileInput::FileInput( int x, int y, int w, int h,
					const char *l ) :
  fltk::Input( x, y, w, h, l )
{
  color( fltk::GRAY75 );
  textcolor( fltk::BLACK );
}

Flu_File_Chooser::FileInput::~FileInput()
{
}

int Flu_File_Chooser::FileInput::handle( int event )
{
  if( event == fltk::KEY )
    {
      if( fltk::event_key() == fltk::TabKey )
	{
	  chooser->filenameTabCallback = true;
	  std::string v(value());
#ifdef _WIN32
	  // turn "C:" into "C:/"
	  if( v.size() >= 3 )
	    if( v.size() == 2 && v[1] == ':' )
	      {
		v += "/";
		value( v.c_str() );
		position( size(), size() );
	      }
#endif
	  chooser->delayedCd = v + "*";
	  fltk::add_timeout( 0.0f, Flu_File_Chooser::delayedCdCB, chooser );
	  return 1;
	}
      else if( fltk::event_key() == fltk::LeftKey )
	{
	  if( fltk::Input::position() == 0 )
	    return 1;
	  else
	    return fltk::Input::handle( event );
	}
      else if( fltk::event_key() == fltk::RightKey )
	{
	  if( fltk::Input::position() == (int)strlen(fltk::Input::value()) )
	    return 1;
	  else
	    return fltk::Input::handle( event );
	}
      else if( fltk::event_key() == fltk::UpKey ||
	       fltk::event_key() == fltk::DownKey )
	{
	  if( !chooser->lastSelected )
	    {
	      if( chooser->getEntryGroup()->children() )
		{
		  Flu_File_Chooser::Entry* e;
		  e = (Flu_File_Chooser::Entry*)chooser->getEntryGroup()->child(0);
		  e->set_selected();
		  chooser->lastSelected = e;
		  e->redraw();
		}
	    }
	  return chooser->getEntryContainer()->handle( event );
	}
    }

  return fltk::Input::handle( event );
}

Flu_File_Chooser::PreviewTile::PreviewTile( int x, int y, int w, int h, Flu_File_Chooser *c )
   : fltk::TiledGroup( x, y, w, h )
{
  chooser = c;
}

// int Flu_File_Chooser::PreviewTile::handle( int event )
// {
//   // if we're not in preview mode, then the user isn't allowed to resize the tile
//   if( !chooser->previewBtn->value() )
//     return fltk::Group::handle( event );
//   if( event == fltk::DRAG )
//     {
//       // the user is probably dragging to resize the columns
//       // update the sizes for each entry
//       chooser->updateEntrySizes();
//       chooser->redraw();
//     }
//   return fltk::TiledGroup::handle(event);
// }

Flu_File_Chooser::PreviewWidgetBase::PreviewWidgetBase()
  : fltk::Group( 0, 0, 0, 0 )
{
}

Flu_File_Chooser::PreviewWidgetBase::~PreviewWidgetBase()
{
}

Flu_File_Chooser::PreviewGroup::PreviewGroup( int x, int y, int w, int h,
					      Flu_File_Chooser *c )
  : fltk::Group( x, y, w, h )
{
  box( fltk::DOWN_BOX );
  align( fltk::ALIGN_CENTER | fltk::ALIGN_CLIP );
  labelfont( fltk::HELVETICA );
  chooser = c;
  handled = 0;
  lastFile = "";
}

void Flu_File_Chooser::PreviewGroup::draw()
{
  if( !chooser->previewBtn->value() )
    return;

  labelsize( 20 );
  if( file.size() == 0 )
    {
      label( "Preview" );
      fltk::Group::draw();
      return;
    }

  struct stat statbuf;

  std::string root;
  boost::int64_t frameStart=1, frameEnd=50;

  if ( chooser->compact_files() )
  {
     bool ok = mrv::get_sequence_limits( frameStart,
					 frameEnd, 
					 file );
     if (ok)
     {
	std::string root;
	ok = mrv::fileroot( root, file );
	char buf[1024];
	sprintf( buf, root.c_str(), frameStart );
	file = buf;
     }
     else 
     {
#undef stat
	int ok = fltk::fltk_stat( file.c_str(), &statbuf );
	if( ok )
	{
	   switch( errno )
	   {
#if !defined(WIN32) && !defined(WIN64)
	      case ELOOP:
		 label( _("Too many symlinks") ); break;
#endif
	      case ENAMETOOLONG:
	      case ENOTDIR:
	      case ENOENT:
		 label( _("Bad\npath") ); break;
	      case ENOMEM:
		 label( _("Not\nMemory") ); break;
	      case EACCES:
	      default:
		 label( _("Not\nReadable") ); break;
	   }
	   fltk::Group::draw();
	   return;
	}
     }
  }
  
  if( lastFile != file )
     {
	lastFile = file;
	
	handled = NULL;
	PreviewWidgetBase *next;
	for( int i = (int)chooser->previewHandlers.size()-1; i >= 0; i-- )
	{
	   next = chooser->previewHandlers[i];
           next->hide();
           if( !handled )
           {
               fltk::Group *p = next->parent();
 
               //fltk::Group::add( next );
               if( next->preview( file.c_str() ) != 0 )
               {
		   handled = next;
               }
               //fltk::Group::remove( next );

               //if ( p ) p->add( handled );

           }
	}
    }

  if( !handled )
    {
      labelsize( 60 );
      label( "?" );
      fltk::Group::draw();
    }
  else
    {
      label( "" );

      handled->resize( box()->dx(), box()->dy(),
 		       w() - box()->dw(), h()- box()->dh() );
      handled->set_visible();

      fltk::Group::add( handled );
      fltk::Group::draw();
      fltk::Group::remove( handled );

      handled->clear_visible();

    }
  
}

void Flu_File_Chooser::ImgTxtPreview::draw()
{
  fltk::Group::draw();
}

// adapted from fltk::File_Chooser2.cxx : update_preview()
int Flu_File_Chooser::ImgTxtPreview::preview( const char *filename )
{
  int	        w = 0, h = 0;		// Width and height of preview image


  fltk::SharedImage* img = fltk::SharedImage::get( filename );


  if( img )
    {
      img->measure(w, h);
    }

  if( !img )
    {
      image( NULL );

      // Try reading the first 1k of data for a label...
      FILE *f = fltk::fltk_fopen( filename, "rb" );
      if( f )
	{
	  size_t bytes = fread( previewTxt, 1, sizeof(previewTxt) - 1, f );
	  previewTxt[bytes] = '\0';
	  fclose( f );
	}
      else
	return 0;

      // Scan the buffer for printable chars...
      unsigned char *ptr;
      for( ptr = previewTxt; *ptr && (isprint(*ptr) || isspace(*ptr));
	   ++ptr ) 
	{
	}

      if( *ptr || ptr == previewTxt )
	{
	  // Non-printable file - can't handle
	  return 0;
	}
      else
	{
	  // Show the first 1k of text...
	  label( (const char*)previewTxt );
	  align( fltk::ALIGN_CLIP | fltk::ALIGN_INSIDE |
		 fltk::ALIGN_LEFT | fltk::ALIGN_TOP );
	  labelsize( 12 );
	  labelfont( fltk::COURIER );
	}
    }
  else if( w > 0 && h > 0 )
    {
      align( fltk::ALIGN_CLIP );
      image( img );
      const char* c = filename + strlen(filename) - 1;
      for ( ; c > filename; --c )
	{
	  if ( *c == '/' || *c == '\\' || *c == ':' )
	    {
	      ++c; break;
	    }
	}

      copy_label( c );
    }

  return 1;
}

void Flu_File_Chooser::previewCB()
{
  if( previewBtn->value() )
    {
      fileGroup->resize( fileGroup->x(), fileGroup->y(),
			 previewTile->w()-200, fileGroup->h() );
      previewGroup->resize( fileGroup->x()+previewTile->w()-200, 
			    previewGroup->y(),
			    previewTile->w()-fileGroup->w(), 
			    previewGroup->h() );
      previewGroup->show();
    }
  else
    {
      fileGroup->resize( fileGroup->x(), fileGroup->y(), 
			 previewTile->w(), fileGroup->h() );
      previewGroup->hide();
    }
  previewTile->relayout();

#if 0
  previewTile->init_sizes();
  fileDetailsGroup->parent()->init_sizes();
#endif

  updateEntrySizes();
}

void Flu_File_Chooser::sortCB( fltk::Widget *w )
{
  fltk::Browser* b = (fltk::Browser*) w;

  int col = b->selected_column();
  if (col == Browser::NO_COLUMN_SELECTED )
    return;


  int sortType = ( 1 << col );
  if( sortMethod & sortType )
    sortMethod ^= SORT_REVERSE;
  else
    sortMethod = sortType;

  bool reverse = ( (sortMethod & SORT_REVERSE) != 0 );

  b->column_labels( col_labels );
  fltk::Widget* c = b->header(col);
  if (!c) return;

  std::string title( c->label() );
  if ( sortMethod & (1 << col) )
    {
      if ( reverse )
	{
	  title += uArrow.c_str();
	}
      else
	{
	  title += dArrow.c_str();
	}
    }
  c->copy_label( title.c_str() );
  filelist->sort();
  filedetails->sort();

}


void Flu_File_Chooser::filenameCB()
{
  filenameEnterCallback = true;
  //cd( filename.value() );
  okCB();
}

inline bool _isProbablyAPattern( const char *s )
{
    // return( strpbrk( s, "*;|[]?" ) != NULL );
  return( strpbrk( s, "*;|?" ) != NULL );
}

void Flu_File_Chooser::okCB()
{
  // if exactly one directory is selected and we are not choosing directories,
  // cd to that directory.
  if( !( selectionType & DIRECTORY ) && !( selectionType & STDFILE ) )
    {
      fltk::Group *g = getEntryGroup();
      std::string dir;
      int count = 0;
      for( int i = 0; i < g->children(); i++ )
	{
	  if( ((Flu_File_Chooser::Entry*)g->child(i))->selected() )
	    {
	      count++;
	      dir = ((Flu_File_Chooser::Entry*)g->child(i))->filename;
	    }
	}
      if( count == 1 )
	{
	  std::string path = currentDir + dir;
	  if( fltk::filename_isdir( path.c_str() ) )
	    {
	      cd( dir.c_str() );
	      return;
	    }
	}
    }

  // only hide if the filename is not blank or the user is choosing directories,
  // in which case use the current directory

  if( selectionType & DIRECTORY ||
      ( (selectionType & STDFILE) && fltk::filename_isdir( (currentDir+filename.value()).c_str() ) )
      )
    {
#ifdef WIN32
      if( myComputerTxt == filename.value() )
	{
	  myComputerCB();
	  return;
	}
#endif
      if( !(selectionType & MULTI ) )
	{
	  if( strlen( filename.value() ) != 0 )
	    cd( filename.value() );
	  filename.value( currentDir.c_str() );
	  filename.position( filename.size(), filename.size() );
	}
      do_callback();
      hide();
    }
  else
    {
      if( strlen( filename.value() ) != 0 )
	{
#ifdef WIN32
	  if( filename.value()[1] == ':' )
#else
	  if( filename.value()[0] == '/' )
#endif
	    if( fltk::filename_isdir( filename.value() ) )
	      {
		filename.value( "" );
		return;
	      }

	  // prepend the path
	  std::string path = currentDir + filename.value();
	  filename.value( path.c_str() );
	  filename.position( filename.size(), filename.size() );
	  do_callback();
	  hide();
	}
    }

  lastSelected = NULL;
}

void Flu_File_Chooser::homeCB()
{
#ifdef WIN32
  cd( "/" );
#else
  cd( userHome.c_str() );
#endif
}

void Flu_File_Chooser::desktopCB()
{
  cd( userDesktop.c_str() );
}


int casecompare(const std::string & s1, const std::string& s2)
{
  std::string::const_iterator it1=s1.begin();
  std::string::const_iterator it2=s2.begin();

  //stop when either string's end has been reached
  while ( (it1!=s1.end()) && (it2!=s2.end()) )
  {
    if(::toupper(*it1) != ::toupper(*it2)) //letters differ?
     // return -1 to indicate smaller than, 1 otherwise
      return (::toupper(*it1)  < ::toupper(*it2)) ? -1 : 1;
    //proceed to the next character in each string
    ++it1;
    ++it2;
  }
  size_t size1=s1.size(), size2=s2.size();// cache lengths
   //return -1,0 or 1 according to strings' lengths
    if (size1==size2)
      return 0;
    return (size1<size2) ? -1 : 1;
}


#define QSCANL( field ) \
      if ( a->field > b->field ) swap = true;

#define RQSCANL( field ) \
      if ( a->field < b->field ) swap = true;


#define CASE_QSCANL( field ) \
      if ( casecompare( a->field, b->field ) > 0 ) swap = true;

#define CASE_RQSCANL( field ) \
      if ( casecompare( a->field, b->field ) < 0 ) swap = true;


#define CUSTOM_QSCANL( field ) \
      if ( customSort( a->field, b->field ) > 0 ) swap = true;

#define CUSTOM_RQSCANL( field ) \
      if ( customSort( a->field, b->field ) < 0 ) swap = true;

void Flu_File_Chooser::_qSort( int how, bool caseSort, fltk::Group* const g,
			       int low, int high )
{
  bool reverse = ( (how & SORT_REVERSE) != 0 );

  if( high <= low ) return;

  for ( int t = low; t <= high; ++t )
    {
      for ( int r = t+1; r <= high; ++r )
	{
	  bool swap = false;
	  Entry* a = (Entry*)g->child(t);
	  Entry* b = (Entry*)g->child(r);

	  switch( how & ~SORT_REVERSE )
	    {
	    case SORT_NAME:
	      if( reverse )
		{
		  if( customSort )
		    {
		      CUSTOM_RQSCANL( filename.c_str() );
		    }
		  else if( !caseSort )
		    {
		      CASE_RQSCANL( filename );
		    }
		  else
		    {
		      RQSCANL( filename );
		    }
		}
	      else
		{
		  if( customSort )
		    {
		      CUSTOM_QSCANL( filename.c_str() );
		    }
		  else if( !caseSort )
		    {
		      CASE_QSCANL( filename );
		    }
		  else
		    {
		      QSCANL( filename );
		    }
		}
	      break;
	    case SORT_SIZE:
	      if( reverse )
		{
		  RQSCANL( isize );
		}
	      else
		{
		  QSCANL( isize );
		}
	      break;
	    case SORT_DATE:
	      if( reverse )
		{
		  RQSCANL( idate );
		}
	      else
		{
		  QSCANL( idate );
		}
	      break;
	    case SORT_TYPE:
	      if( reverse )
		{
		  RQSCANL( description );
		}
	      else
		{
		  QSCANL( description );
		}
	    }

	  if ( swap )
	    {
	      g->swap(t,r);
	    }
	}
    }
}

Flu_File_Chooser::FileList::FileList( int x, int y, int w, int h,
				      Flu_File_Chooser *c )
  : Flu_Wrap_Group( x, y, w, h )
{
  chooser = c;
  numDirs = 0;
}

Flu_File_Chooser::FileList::~FileList()
{
}

void Flu_File_Chooser::FileList::sort( int n )
{
  if( n != -1 )
    numDirs = n;

  if( children() < 2 )
    return;

  // the directories are already first.
  // sort the directories then the names lexigraphically
  Flu_File_Chooser::_qSort( chooser->sortMethod, chooser->caseSort,
			    this, 0, numDirs-1 );
  Flu_File_Chooser::_qSort( chooser->sortMethod, chooser->caseSort,
			    this, numDirs, children()-1 );
  chooser->redraw();
}

int Flu_File_Chooser::FileList::handle( int event )
{
  if( event == fltk::FOCUS || event == fltk::UNFOCUS )
    return 1;


  if( Flu_Wrap_Group::handle( event ) )
    return 1;

  // if push on no file, unselect all files and turn off editing mode
  if( event == fltk::PUSH &&
      !(fltk::event_key() & fltk::SHIFT) &&
      !(fltk::event_key() & fltk::CTRL) )
    {
      chooser->unselect_all();
      chooser->filename.value( "" );
      chooser->filename.position( chooser->filename.size(), 
				  chooser->filename.size() );

      if( fltk::event_button() == fltk::RightButton )
	return chooser->popupContextMenu( NULL );

      return 1;
    }
  else if( event == fltk::KEY )
    {
      if( fltk::event_key() == fltk::DeleteKey )
	{
	  // recycle by default, unless the shift key is held down
	  chooser->trashCB( !fltk::event_state( fltk::SHIFT ) );
	  return 1;
	}

      Flu_File_Chooser::Entry *e = chooser->lastSelected;
      if( !e )
	{
	  for( int i = 0; i < children(); ++i )
	    if( ((Flu_File_Chooser::Entry*)child(i))->selected() )
	      {
		e = (Flu_File_Chooser::Entry*)child(i);
		break;
	      }
	}
      if( e )
	{
	  switch( fltk::event_key() )
	    {
	    case fltk::UpKey:
	      e = (Flu_File_Chooser::Entry*)previous( e );
	      if( !e && children() ) e = (Flu_File_Chooser::Entry*)child(0);
	      break;
	    case fltk::DownKey:
	      e = (Flu_File_Chooser::Entry*)next( e );
	      if( !e && children() ) e = (Flu_File_Chooser::Entry*)child(children()-1);
	      break;
	    case fltk::LeftKey:  e = (Flu_File_Chooser::Entry*)left( e ); break;
	    case fltk::RightKey: e = (Flu_File_Chooser::Entry*)right( e ); break;
	    case fltk::HomeKey: if( children() ) e = (Flu_File_Chooser::Entry*)child(0); break;
	    case fltk::EndKey: if( children() ) e = (Flu_File_Chooser::Entry*)child(children()-1); break;
	    case fltk::ReturnKey:
	      chooser->filenameEnterCallback = true;
	      //chooser->cd( e->filename.c_str() );
	      chooser->okCB();
	      return 1;
	    case ' ':
 	      chooser->cd( e->filename.c_str() );
	      return 1;
	    default: e = 0; break;
	    }
	  if( e )
	    {
	      chooser->unselect_all();
	      e->set_selected();
	      chooser->lastSelected = e;
	      chooser->filename.value( e->filename.c_str() );
	      chooser->filename.position( chooser->filename.size(),
					  chooser->filename.size() );
	      chooser->redraw();
	      if( e->type == ENTRY_FILE || e->type == ENTRY_SEQUENCE )
		{
		  chooser->previewGroup->file = chooser->currentDir + e->filename;
		}

	      scroll_to( e );
	      return 1;
	    }
	}
    }


  return 0;
}

Flu_File_Chooser::FileDetails::FileDetails( int x, int y, int w, int h,
					    Flu_File_Chooser *c )
  : fltk::Browser( x, y, w, h )
{
  uchar r,g,b;
  fltk::split_color( color(), r, g, b );
  scrollbar.color( fltk::color(uchar(r*2),uchar(g*2),uchar(b*2)) );

  chooser = c;
  numDirs = 0;
}

Flu_File_Chooser::FileDetails::~FileDetails()
{
}

void Flu_File_Chooser::FileDetails::scroll_to( fltk::Widget *w )
{
  for( int i = 0; i < children(); ++i )
    {
      if( child(i) == w )
	{
	  display(i);
	  return;
	}
    }
}

void Flu_File_Chooser::FileDetails::sort( int n )
{
  if( n != -1 )
    numDirs = n;
  if( children() < 2 )
    return;
  // the directories are already first. sort the directories then the names lexigraphically
  Flu_File_Chooser::_qSort( chooser->sortMethod, chooser->caseSort, this, 0, numDirs-1 );
  Flu_File_Chooser::_qSort( chooser->sortMethod, chooser->caseSort, this, numDirs, children()-1 );
  chooser->redraw();
}

fltk::Widget* Flu_File_Chooser::FileDetails::next( fltk::Widget* w )
{
  for( int i = 0; i < children()-1; i++ )
    {
      if( w == child(i) )
	return child(i+1);
    }
  return NULL;
}

fltk::Widget* Flu_File_Chooser::FileDetails::previous( fltk::Widget* w )
{
  for( int i = 1; i < children(); i++ )
    {
      if( w == child(i) )
	return child(i-1);
    }
  return NULL;
}


void Flu_File_Chooser::FileDetails::layout()
{
  fltk::Browser::layout();
  chooser->updateEntrySizes();
}

int Flu_File_Chooser::FileDetails::handle( int event )
{
  if( fltk::Browser::handle( event ) )
    return 1;
  else if( event == fltk::KEY )
    {
      if( fltk::event_key() == fltk::DeleteKey )
	{
	  // recycle by default, unless the shift key is held down
	  chooser->trashCB( !fltk::event_state( fltk::SHIFT ) );
	  return 1;
	}

      Flu_File_Chooser::Entry *e = chooser->lastSelected;
      if( !e )
	{
	  for( int i = 0; i < children(); i++ )
	    if( ((Flu_File_Chooser::Entry*)child(i))->selected() )
	      {
		e = (Flu_File_Chooser::Entry*)child(i);
		break;
	      }
	}
      if( e )
	{
	  switch( fltk::event_key() )
	    {
	    case fltk::UpKey: e = (Flu_File_Chooser::Entry*)previous( e );
	      if( !e && children() ) e = (Flu_File_Chooser::Entry*)child(0); break;
	    case fltk::DownKey: e = (Flu_File_Chooser::Entry*)next( e );
	      if( !e && children() ) e = (Flu_File_Chooser::Entry*)child(children()-1); break;
	    case fltk::HomeKey: if( children() ) e = (Flu_File_Chooser::Entry*)child(0); break;
	    case fltk::EndKey: if( children() ) e = (Flu_File_Chooser::Entry*)child(children()-1); break;
	    case fltk::ReturnKey:
	      chooser->filenameEnterCallback = true;
	      //chooser->cd( e->filename.c_str() );
	      chooser->okCB();
	      return 1;
	    case ' ':
	      chooser->cd( e->filename.c_str() );
	      return 1;
	    default: e = 0; break;
	    }
	  if( e )
	    {
	      chooser->unselect_all();
	      e->set_selected();
	      chooser->lastSelected = e;
	      chooser->filename.value( e->filename.c_str() );
	      chooser->filename.position( chooser->filename.size(),
					  chooser->filename.size() );
	      chooser->redraw();
	      scroll_to( e );
	      return 1;
	    }
	}
    }

  return 0;
}

void loadRealIcon(void* entry)
{
    Flu_File_Chooser::Entry* e = (Flu_File_Chooser::Entry*) entry;
    if  ( !e || !e->chooser || !e->chooser->get_current_directory() ) return;


    char fmt[1024];
    char buf[1024];
    sprintf( fmt, "%s%s", e->chooser->get_current_directory(),
             e->filename.c_str() );
    sprintf( buf, fmt, 1 );  // should be first frame

    std::cerr << buf << " entry: " << e 
              << " chooser: " << e->chooser << std::endl;

    fltk::SharedImage* img = fltk::SharedImage::get( buf );

    std::cerr << "img: " << img << " " << img->w() << "x"
              << img->h() << std::endl;

    if ( img )
    {
        int w,h = img->h();
        e->icon = img;
        e->resize(e->w(), h+4 );
        e->redraw();
        e->chooser->relayout();
        e->chooser->redraw();
    }

    // fltk::remove_timeout( loadRealIcon );
    
}


Flu_File_Chooser::Entry::Entry( const char* name, int t, bool d, 
				Flu_File_Chooser *c )
: fltk::Input( 0, 0, 0, 0 ),
  filename( name ),
  type( t ),
  details( d ),
  chooser( c )
{
  resize( 0, 0, DEFAULT_ENTRY_WIDTH, 20 );
  textsize( 12 );

  color( fltk::GRAY85 );
  textcolor( fltk::BLACK );
  clear_selected();

  box( fltk::FLAT_BOX );
  when( fltk::WHEN_RELEASE_ALWAYS | fltk::WHEN_ENTER_KEY_ALWAYS );
  callback( _inputCB, this );
  icon = NULL;
  editMode = 0;
  description = "";

  if( type == ENTRY_FILE && (c->selectionType & DEACTIVATE_FILES) )
    {
      textcolor( fltk::GRAY75 );
      deactivate();
    }

  updateSize();
  updateIcon();

}

void Flu_File_Chooser::Entry::updateIcon()
{
  icon = NULL;
  Flu_File_Chooser::FileTypeInfo *tt = NULL;
  if( type==ENTRY_MYCOMPUTER )
    {
      icon = &computer;
      description = myComputerTxt;
    }
  else if( type==ENTRY_MYDOCUMENTS )
    {
      icon = &documents;
      description = myDocumentsTxt;
    }
  else if( type==ENTRY_DRIVE )
    {
      //icon = &disk_drive;
      //description = "";
    }
  else if( type==ENTRY_DIR || type==ENTRY_FAVORITE )
    tt = Flu_File_Chooser::find_type( NULL );
  else
    {
      const char *dot = strrchr( filename.c_str(), '.' );
      if( dot )
	{
	  tt = Flu_File_Chooser::find_type( dot+1 );
	  if( !tt )
	    description = dot+1;
	}
    }
  if( tt )
    {
      icon = tt->icon;
      description = tt->type;

    }
  // if there is no icon, assign a default one
  if( !icon && type==ENTRY_FILE && 
      !(chooser->selectionType & DEACTIVATE_FILES) )
    icon = chooser->defaultFileIcon;
  if( type==ENTRY_FAVORITE )
    icon = &little_favorites;

  toolTip = detailTxt[0] + ": " + filename;
  if( type == ENTRY_FILE )
    toolTip += "\n" + detailTxt[1] +": " + filesize;
  if( type == ENTRY_SEQUENCE )
    toolTip += "\n" + detailTxt[4] +": " + filesize;
  toolTip += "\n" + detailTxt[3] + ": " + description;
  tooltip( toolTip.c_str() );

  // Here's the icon setup

  if ( icon )
    {
      int w = 0, h = 0;
      icon->measure(w,h);
    }

  redraw();
}

void Flu_File_Chooser::resize( int x, int y, int w, int h )
{
  fltk::DoubleBufferWindow::resize( x, y, w, h );
  for( int i = 0; i < filedetails->children(); ++i )
    ((Entry*)filedetails->child(i))->updateSize();
  filedetails->relayout();
}

void Flu_File_Chooser::listModeCB( fltk::Widget* o )
{
  fltk::Button* b = (fltk::Button*)o;
  fileListBtn->value(0);
  fileListWideBtn->value(0);
  fileDetailsBtn->value(0);
  b->value(1);

  bool listMode = !fileDetailsBtn->value() || 
                  ( currentDir == FAVORITES_UNIQUE_STRING );
  if( listMode )
    {
      if ( fileListWideBtn->value() )
	filelist->set_horizontal();
      else
	filelist->set_vertical();
      filelist->scroll_to_beginning();

      while( filedetails->children() )
	filelist->add( filedetails->child(0) );

      fileDetailsGroup->hide();
      filelist->show();
      filelist->relayout();
      filelist->parent()->resizable( filelist );
    }
  else
    {
      while( filelist->children() )
	filedetails->add( filelist->child(0) );

      filelist->hide();
      filedetails->relayout();
      fileDetailsGroup->show();
      fileDetailsGroup->parent()->resizable( fileDetailsGroup );

    }

  updateEntrySizes();
  if ( lastSelected ) {
    filedetails->scroll_to( lastSelected );
    filelist->scroll_to( lastSelected );
  }
}

void Flu_File_Chooser::Entry::updateSize()
{
  if( type==ENTRY_FAVORITE || chooser->fileListWideBtn->value() )
    {
      resize( x(), y(), chooser->filelist->w()-4, 20 );
    }
  else
    resize( x(), y(), DEFAULT_ENTRY_WIDTH, 20 );

  details = chooser->fileDetailsBtn->value() && ( type != ENTRY_FAVORITE );

  if( details )
    {
      int cw[4];
      for ( int i = 0; i < 4; ++i )
	cw[i] = chooser->filedetails->header(i)->w();
      nameW = cw[0];
      typeW = cw[1];
      sizeW = cw[2];
      dateW = cw[3];
      resize( x(), y(), chooser->filedetails->w(), 20 );
    }
  else
    nameW = w();

  // how big is the icon?
  int iW = 0, iH = 0;
  if( icon )
    {
      iW = icon->w()+2;
      iH = icon->h();
    }

  fltk::setfont( textfont(), textsize() );

  // measure the name and see if we need a truncated version
  int W = 0, H = 0;
  fltk::measure( filename.c_str(), W, H );
  if( W > nameW-iW )
    {
      // progressively strip characters off the end of the name until
      // it fits with "..." at the end
      if( altname.size() > 0 )
	shortname = altname;
      else
	shortname = filename;
      size_t len = shortname.size();
      while( W > (nameW-iW) && len > 3 )
	{
	  shortname[len-4] = '.';
	  shortname[len-3] = '.';
	  shortname[len-2] = '.';
	  shortname[len-1] = '\0';
	  len--;
	  W = 0;
	  fltk::measure( shortname.c_str(), W, H );
	}
    }
  else
    shortname = "";

  // measure the description and see if we need a truncated version
  shortDescription = "";
  if( details )
    {
      W = 0; H = 0;
      fltk::measure( description.c_str(), W, H );
      if( W > typeW-4 )
	{
	  // progressively strip characters off the end of the description until
	  // it fits with "..." at the end
	  shortDescription = description;
	  size_t len = shortDescription.size();
	  while( W > typeW-4 && len > 3 )
	    {
	      shortDescription[len-4] = '.';
	      shortDescription[len-3] = '.';
	      shortDescription[len-2] = '.';
	      shortDescription[len-1] = '\0';
	      len--;
	      W = 0;
	      fltk::measure( shortDescription.c_str(), W, H );
	    }
	}
    }

  redraw();
}

Flu_File_Chooser::Entry::~Entry()
{
    std::cerr << "delete entry " << this << std::endl;
}

void Flu_File_Chooser::Entry::inputCB()
{
  redraw();

  // if the user tried to change the string to nothing, restore the original name and turn off edit mode
  if( strlen( value() ) == 0 )
    {
      editMode = 0;
      return;
    }

  // if input text is different from filename, try to change the filename
  if( strcmp( value(), filename.c_str() ) != 0 )
    {
      // build the total old filename and new filename
      std::string oldName = chooser->currentDir + filename,
	newName = chooser->currentDir + value();
      // see if new name already exists
      struct stat s;
      int result = ::stat( newName.c_str(), &s );
      if( result == 0 )
	{
	  fltk::alert( fileExistsErrTxt.c_str(), newName.c_str() );
	  return;  // leave editing on
	}

      if( rename( oldName.c_str(), newName.c_str() ) == -1 )
	{
	  fltk::alert( renameErrTxt.c_str(), oldName.c_str(), newName.c_str() );
	  //return;  // leave editing on
	}
      else
	{
	  filename = value();
	  updateSize();
	  updateIcon();
	}
      // QUESTION: should we set the chooser filename to the modified name?
      chooser->filename.value( filename.c_str() );
    }

  // only turn off editing if we have a successful name change
  editMode = 0;
}

fltk::Group* Flu_File_Chooser::getEntryGroup()
{
  if ( !fileDetailsBtn->value() || currentDir == FAVORITES_UNIQUE_STRING )
    return filelist;
  else
    return filedetails;
}

fltk::Group* Flu_File_Chooser::getEntryContainer()
{
  if ( !fileDetailsBtn->value() || currentDir == FAVORITES_UNIQUE_STRING )
    return filelist;
  else
    return filedetails;
}

int Flu_File_Chooser::Entry::handle( int event )
{
  if( editMode )
    {
      // if user hits 'Escape' while in edit mode,
      // restore the original name and turn off edit mode
      if( event == fltk::KEY && fltk::event_key() == fltk::EscapeKey )
	{
	  editMode = 0;
	  redraw();
	  if( selected() )
	    chooser->trashBtn->activate();
	  return 1;
	}
      return fltk::Input::handle( event );
    }

  if( event == fltk::FOCUS || event == fltk::UNFOCUS )
  {
    return 1;
  }

  if( event == fltk::ENTER )
  {
     // if user enters an entry cell, color it cyan
     if (!selected()) {
	color( fltk::CYAN );
	redraw();
     }
     return 1;
  }
  if( event == fltk::LEAVE )
  {
     // if user leaves an entry cell, color it gray or blue
     if (selected()) {
	color( fltk::DARK_BLUE );
	redraw();
     }
     else
     {
	color( fltk::GRAY85 );
	redraw();
     }
     return 1;
  }

  if ( event == fltk::MOVE )
  {
     return 1;
  }

  fltk::Group *g = chooser->getEntryGroup();
  if( event == fltk::PUSH )
  {
      if ( Flu_File_Chooser::singleButtonTravelDrawer && 
           fltk::event_button() == fltk::LeftButton )
     {
	// double-clicking a favorite cd's to it
	if( type == ENTRY_FAVORITE )
	{
	   if ( fltk::event_clicks() > 1 )
	   {
	      fltk::event_clicks(0);
	      chooser->delayedCd = filename;
	      fltk::add_timeout( 0.0f, Flu_File_Chooser::delayedCdCB, chooser );
	   }
	}
	else if( type != ENTRY_FILE && type != ENTRY_SEQUENCE )
	{
#ifdef WIN32
	   if( filename[1] == ':' )
	      chooser->delayedCd = filename;
	   else
#endif
	      chooser->delayedCd = chooser->currentDir + filename + "/";
	   fltk::add_timeout( 0.0f, Flu_File_Chooser::delayedCdCB, chooser );
	}
     }
     
     if( fltk::event_clicks() > 0 )
	{
	   if( type == ENTRY_FAVORITE )
	   {
	      fltk::event_clicks(0);
	      chooser->delayedCd = filename;
	      fltk::add_timeout( 0.0f, Flu_File_Chooser::delayedCdCB, chooser );
	   }
	   else if( type != ENTRY_FILE && type != ENTRY_SEQUENCE )
	   {
	      fltk::event_clicks(0);
#ifdef WIN32
	      if( filename[1] == ':' )
		 chooser->delayedCd = filename;
	      else
#endif
		 chooser->delayedCd = chooser->currentDir + filename + "/";
	      fltk::add_timeout( 0.0f, Flu_File_Chooser::delayedCdCB, chooser );
	   }
	   // double-clicking a file chooses it if we are in file selection mode
	   else if( !(chooser->selectionType & DIRECTORY) ||
		    (chooser->selectionType & STDFILE) )
	   {
	      fltk::event_clicks(0);
	      fltk::add_timeout( 0.0f, Flu_File_Chooser::selectCB, chooser );
	   }
	   if( selected() )
	      chooser->trashBtn->activate();
	   return 1;
	}
     
     

     /*
      if( selected && !fltk::event_button3() && !fltk::event_state(fltk::CTRL) && !fltk::event_state(fltk::SHIFT) )
	{
	  // only allow editing of certain files and directories
	  if( chooser->fileEditing && ( type == ENTRY_FILE || type == ENTRY_DIR ) )
	    {
	      // if already selected, switch to input mode
	      fltk::add_timeout( 1.0, _editCB, this );
	      return 1;
	    }
	}

	else*/
      if( chooser->selectionType & MULTI )
	{
	  if( fltk::event_state(fltk::CTRL) )
	    {
	      selected() ? clear_selected() : set_selected();
	      chooser->lastSelected = this;
	      if( type == ENTRY_FILE || type == ENTRY_SEQUENCE )
		{
		  chooser->previewGroup->file = chooser->currentDir + filename;
		}

	      chooser->redraw();
	    }
	  else if( fltk::event_state(fltk::SHIFT) )
	    {
	      // toggle all items from the last selected item to this one
	      if( chooser->lastSelected == NULL )
		{
		  set_selected();
		  chooser->lastSelected = this;
		  if( type == ENTRY_FILE || type == ENTRY_SEQUENCE )
		    {
		    chooser->previewGroup->file = chooser->currentDir + filename;
		    }

		  chooser->redraw();
		}
	      else
		{
		  // get the index of the last selected item and this item
		  int lastindex = -1, thisindex = -1;
		  int i;
		  for( i = 0; i < g->children(); i++ )
		    {
		      if( g->child(i) == chooser->lastSelected )
			lastindex = i;
		      if( g->child(i) == this )
			thisindex = i;
		      if( lastindex >= 0 && thisindex >= 0 )
			break;
		    }
		  if( lastindex >= 0 && thisindex >= 0 )
		    {
		      // loop from this item to the last item, toggling each item except the last
		      int inc;
		      if( thisindex > lastindex )
			inc = -1;
		      else
			inc = 1;
		      Entry *e;
		      for( i = thisindex; i != lastindex; i += inc )
			{
			  e = (Entry*)g->child(i);
			  e->selected() ? e->clear_selected() : e->set_selected();
			  e->redraw();
			}
		      chooser->lastSelected = this;
		      if( type == ENTRY_FILE || type == ENTRY_SEQUENCE )
			{
			chooser->previewGroup->file = chooser->currentDir + filename;
			}
		      chooser->redraw();
		    }
		}
	    }
	  else
	    {
	      chooser->unselect_all();
	      set_selected();
	      chooser->lastSelected = this;
	      if( type == ENTRY_FILE || type == ENTRY_SEQUENCE )
		{
		chooser->previewGroup->file = chooser->currentDir + filename;
		}
	      chooser->redraw();
	    }


	  if( !((chooser->selectionType & Flu_File_Chooser::DIRECTORY) ||
		(chooser->selectionType & Flu_File_Chooser::STDFILE)) &&
	      ( fltk::event_state(fltk::CTRL) || fltk::event_state(fltk::SHIFT) ) )
	    {
	      // if we are only choosing multiple files, don't allow a directory
	      // to be selected
	      fltk::Group *g = chooser->getEntryGroup();
	      for( int i = 0; i < g->children(); i++ )
		{
		  Entry *e = (Entry*)g->child(i);
		  if( e->type == ENTRY_DIR )
		    e->clear_selected();
		}
	    }
	}
      else
	{
	  chooser->unselect_all();
	  set_selected();
	  chooser->lastSelected = this;
	  if( type == ENTRY_FILE || type == ENTRY_SEQUENCE )
	    {
	      chooser->previewGroup->file = chooser->currentDir + filename;
	    }
	}

      redraw();
      if( selected() )
	chooser->trashBtn->activate();

      if( fltk::event_button() == fltk::RightButton )
	return chooser->popupContextMenu( this );

      // don't put the filename into the box if we are a directory but we are not choosing directories
      // or if we are in SAVING mode
      if( (chooser->selectionType & Flu_File_Chooser::DIRECTORY) ||
	  (chooser->selectionType & Flu_File_Chooser::STDFILE) ||
	  type==ENTRY_FILE || type == ENTRY_SEQUENCE )
      {
	chooser->filename.value( filename.c_str() );
      }
      else if( !(chooser->selectionType & Flu_File_Chooser::SAVING ) )
      {
	chooser->filename.value( "" );
      }

      chooser->filename.position( chooser->filename.size(), 
				  chooser->filename.size() );

      return 1;
    }

  return fltk::Widget::handle(event);
}

void Flu_File_Chooser::Entry::editCB()
{
  // if already selected, switch to input mode
  editMode = 2;
  value( filename.c_str() );
  take_focus();
  // select the text up to but not including the extension
  const char *dot = strrchr( filename.c_str(), '.' );
  if( dot )
    position( 0, (int)(dot-filename.c_str()) );
  else
    position( 0, (int)filename.size() );
  chooser->trashBtn->deactivate();
  redraw();
}

int Flu_File_Chooser::popupContextMenu( Entry *entry )
{
  int type = entry ? entry->type : ENTRY_NONE;
  const char *filename = entry ? entry->filename.c_str() : NULL;
  char *ext = NULL;

  if( filename )
     ext = (char*) strrchr( filename, '.' );
  if( ext )
    {
      ext = strdup( ext+1 ); // skip the '.'
      for( unsigned int i = 0; i < strlen(ext); i++ )
          ext[i] = (char)tolower( ext[i] );
    }

  enum { ACTION_NEW_FOLDER = -1, ACTION_RENAME = -2, ACTION_DELETE = -3 };

  fltk::PopupMenu   entryPopup(0,0,0,0);
  entryPopup.clear();
  switch( type )
    {
    case ENTRY_NONE: // right click on nothing
      entryPopup.add( contextMenuTxt[0].c_str(), (void*)ACTION_NEW_FOLDER );
      break;

    case ENTRY_DIR:
      entryPopup.add( contextMenuTxt[1].c_str(), (void*)ACTION_RENAME );
      entryPopup.add( contextMenuTxt[2].c_str(), (void*)ACTION_DELETE );
      break;

    case ENTRY_FILE:
      entryPopup.add( contextMenuTxt[1].c_str(), (void*)ACTION_RENAME );
      entryPopup.add( contextMenuTxt[2].c_str(), (void*)ACTION_DELETE );
      break;

   case ENTRY_FAVORITE:
     entryPopup.add( contextMenuTxt[2].c_str(), (void*)ACTION_DELETE );
      break;

    case ENTRY_DRIVE:
      break;

    case ENTRY_MYDOCUMENTS:
      break;

    case ENTRY_MYCOMPUTER:
      break;

    case ENTRY_SEQUENCE:
      break;
    }

  // add the programmable context handlers
  for( unsigned int i = 0; i < contextHandlers.size(); i++ )
    {
      if( !(contextHandlers[i].type & type) )
	continue;
      if( type == ENTRY_FILE || type == ENTRY_SEQUENCE )
	if( contextHandlers[i].ext.size() && contextHandlers[i].ext != ext )
	  continue;
      entryPopup.add( contextHandlers[i].name.c_str(), (void*) (intptr_t)i );
    }
  if( ext )
    free( ext );

  entryPopup.popup();
  const fltk::Widget* selection = entryPopup.get_item();
  if( selection )
    {
      size_t handler = (size_t) selection->user_data();
      switch( handler )
	{
	case ACTION_NEW_FOLDER:
	  newFolderCB();
	  break;
	case ACTION_RENAME:
	  entry->editCB();
	  /*
	  entry->editMode = 2;
	  entry->value( entry->filename.c_str() );
	  entry->take_focus();
	  entry->position( 0, entry->filename.size() );
	  trashBtn->deactivate();
	  */
	  break;
	case ACTION_DELETE:
	  // recycle by default, unless the shift key is held down
	  trashCB( !fltk::event_state( fltk::SHIFT ) );
	  break;
	default:
	  contextHandlers[handler].callback( filename, type, 
					     contextHandlers[handler].callbackData );
	  break;
	}
    }
  else
    return handle( fltk::PUSH );
  return 1;
}

void Flu_File_Chooser::Entry::draw()
{
  if( editMode )
    {
      if( editMode == 2 )
	{
	  editMode--;
	  draw_box();
	  redraw();
	}
      fltk::Input::draw();
      return;
    }

  draw_box();

  int X = 4;


  if( icon )
    {
      icon->draw( X, h()/2-icon->h()/2 );
      X += icon->w()+2;
    }
  
  if( shortname.size() > 0 )
    labeltype()->draw( shortname.c_str(), fltk::Rectangle( X, 0, nameW, h() ),
		       fltk::ALIGN_LEFT );
  else if( altname.size() > 0 )
    labeltype()->draw( altname.c_str(), fltk::Rectangle( X, 0, nameW, h() ),
		       fltk::ALIGN_LEFT );
  else
    labeltype()->draw( filename.c_str(), fltk::Rectangle( X, 0, nameW, h() ),
		       fltk::ALIGN_LEFT );

  X = 4 + nameW;

  if( details )
    {
        if( shortDescription.size() )
            labeltype()->draw( shortDescription.c_str(),
                               fltk::Rectangle( X, 0, typeW-4, h() ),
                               fltk::ALIGN_LEFT | fltk::ALIGN_CLIP );
        else
            labeltype()->draw( description.c_str(),
                               fltk::Rectangle( X, 0, typeW-4, h() ),
                               fltk::ALIGN_LEFT | fltk::ALIGN_CLIP );

        X += typeW;

        labeltype()->draw( filesize.c_str(),
                           fltk::Rectangle( X, 0, sizeW-4, h() ),
                           fltk::ALIGN_LEFT | fltk::ALIGN_CLIP );

        X += sizeW+4;

        labeltype()->draw( date.c_str(),
                           fltk::Rectangle( X, 0, dateW-4, h() ),
                           fltk::ALIGN_LEFT | fltk::ALIGN_CLIP );
    }
}


void Flu_File_Chooser::unselect_all()
{
  fltk::Group *g = getEntryGroup();
  Entry *e;
  for( int i = 0; i < g->children(); ++i )
    {
      e = ((Entry*)g->child(i));
      e->clear_selected();
      e->color( fltk::GRAY85 );
      e->editMode = 0;
    }
  lastSelected = 0;
  previewGroup->file = "";
  previewGroup->redraw();
  trashBtn->deactivate();
  redraw();
}

void Flu_File_Chooser::select_all()
{
  if( !( selectionType & MULTI ) )
    return;
  fltk::Group *g = getEntryGroup();
  Entry *e;
  previewGroup->file = "";
  for( int i = 0; i < g->children(); i++ )
    {
      e = ((Entry*)g->child(i));
      e->set_selected();
      e->editMode = 0;
      previewGroup->file = e->filename;
      filename.value( e->filename.c_str() );
    }
  lastSelected = 0;
  previewGroup->redraw();
  trashBtn->deactivate();
  redraw();
}

void Flu_File_Chooser::updateEntrySizes()
{
  int i;
  for( i = 0; i < filedetails->children(); i++ )
    ((Entry*)filedetails->child(i))->updateSize();
  for( i = 0; i < filelist->children(); i++ )
    ((Entry*)filelist->child(i))->updateSize();
}

const char* Flu_File_Chooser::value()
{
  if( filename.size() == 0 )
    return NULL;
  else
    {
#ifdef WIN32
      // on windows, be sure the drive letter is lowercase for
      // compatibility with fl_filename_relative()
      if( filename.size() > 1 && filename.value()[1] == ':' )
	((char*)(filename.value()))[0] = tolower( filename.value()[0] );
#endif
      return filename.value();
    }
}

int Flu_File_Chooser::count()
{
  if( selectionType & MULTI )
    {
      int n = 0;
      fltk::Group *g = getEntryGroup();
      for( int i = 0; i < g->children(); i++ )
	{
#ifdef WIN32
	  if( ((Entry*)g->child(i))->filename == myComputerTxt )
	    continue;
#endif
	  if( ((Entry*)g->child(i))->selected() )
	    n++;
	}
      return n;
    }
  else
    return (strlen(filename.value())==0)? 0 : 1;
}

void Flu_File_Chooser::value( const char *v )
{
  cd( v );
  if( !v )
    return;
  // try to find the file and select it
  const char *slash = strrchr( v, '/' );
  if( slash )
    slash++;
  else
    {
      slash = strrchr( v, '\\' );
      if( slash )
	slash++;
      else
	slash = v;
    }
  filename.value( slash );
  filename.position( filename.size(), filename.size() );
  fltk::Group *g = getEntryGroup();
  for( int i = 0; i < g->children(); i++ )
    {
      Entry* e = (Entry*) g->child(i);
      if( e->filename == slash )
	{
	  e->set_selected();
	  filelist->scroll_to( e );
	  filedetails->scroll_to( e );
	  redraw();
	  return;
	}
    }
}

const char* Flu_File_Chooser::value( int n )
{
  fltk::Group *g = getEntryGroup();
  for( int i = 0; i < g->children(); i++ )
    {
#ifdef WIN32
      if( ((Entry*)g->child(i))->filename == myComputerTxt )
	continue;
#endif
      if( ((Entry*)g->child(i))->selected() )
	{
	  n--;
	  if( n == 0 )
	    {
	      std::string s = currentDir + ((Entry*)g->child(i))->filename;
	      filename.value( s.c_str() );
	      filename.position( filename.size(), filename.size() );
	      return value();
	    }
	}
    }
  return "";
}

void Flu_File_Chooser::reloadCB()
{
#ifdef _WIN32
  refreshDrives = true;
#endif
  cd( currentDir.c_str() );
}

void Flu_File_Chooser::addToFavoritesCB()
{
  // eliminate duplicates
  bool duplicate = false;
  for( int i = 0; i < favoritesList->children(); ++i )
    {
      if ( favoritesList->child(i)->label() == NULL ) continue;
      if( streq( currentDir.c_str(), favoritesList->child(i)->label() ) )
	{
	  duplicate = true;
	  break;
	}
    }
  if( !duplicate )
    {
      favoritesList->add( currentDir.c_str() );
      favoritesList->relayout();
      buildLocationCombo();
    }

  // save the favorites
  FILE *f = fltk::fltk_fopen( configFilename.c_str(), N_("w") );
  if( !f ) return;


  for( int i = 0; i < favoritesList->children(); ++i )
    {
        if ( favoritesList->child(i)->label() == NULL ) continue;
        fprintf( f, N_("%s\n"), favoritesList->child(i)->label() );
    }
  fclose( f );
}

std::string Flu_File_Chooser::formatDate( const char *d )
{
  if( d == 0 )
    {
      std::string s;
      return s;
    }

  // convert style "Wed Mar 19 07:23:11 2003" to "MM/DD/YY HH:MM AM|PM"

  int month, day, year, hour, minute, second;
  bool pm;
  char MM[16], dummy[64];

  sscanf( d, "%s %s %d %d:%d:%d %d", dummy, MM, &day, &hour, &minute, &second, &year );

  pm = ( hour >= 12 );
  if( hour == 0 )
    hour = 12;
  if( hour >= 13 )
    hour -= 12;

  if( strcmp(MM,"Jan")==0 ) month = 1;
  else if( strcmp(MM,"Feb")==0 ) month = 2;
  else if( strcmp(MM,"Mar")==0 ) month = 3;
  else if( strcmp(MM,"Apr")==0 ) month = 4;
  else if( strcmp(MM,"May")==0 ) month = 5;
  else if( strcmp(MM,"Jun")==0 ) month = 6;
  else if( strcmp(MM,"Jul")==0 ) month = 7;
  else if( strcmp(MM,"Aug")==0 ) month = 8;
  else if( strcmp(MM,"Sep")==0 ) month = 9;
  else if( strcmp(MM,"Oct")==0 ) month = 10;
  else if( strcmp(MM,"Nov")==0 ) month = 11;
  else month = 12;

  sprintf( dummy, "%d/%d/%02d %d:%02d %s", month, day, year, hour, minute, pm?"PM":"AM" );

  std::string formatted = dummy;

  return formatted;
}

void Flu_File_Chooser::win2unix( std::string &s )
{
  size_t len = s.size();
  for( size_t i = 0; i < len; i++ )
    if( s[i] == '\\' )
      s[i] = '/';
}

void Flu_File_Chooser::cleanupPath( std::string &s )
{
  // convert all '\' to '/'
  win2unix( s );

  std::string newS(s.size()+1, '\0');

  size_t oldPos, newPos;
  for( oldPos = 0, newPos = 0; oldPos < s.size(); oldPos++ )
    {
      // remove "./"
      if( s[oldPos] == '.' && oldPos+1 < s.size() && s[oldPos+1] == '/' )
	oldPos += 2;

      // convert "//" to "/"
      else if( s[oldPos] == '/' && oldPos+1 < s.size() && s[oldPos+1] == '/' )
	oldPos++;

#ifdef WIN32
      // downcase "c:" to "C:"
      else if( oldPos+1 < s.size() && s[oldPos+1] == ':' )
	s[oldPos] = toupper( s[oldPos] );
#endif

      // remove "../" by removing everything back to the last "/"
      if( oldPos+2 < s.size() ) // bounds check
	{
	  if( s[oldPos] == '.' && s[oldPos+1] == '.' && s[oldPos+2] == '/' && newS != "/" )
	    {
	      // erase the last character, which should be a '/'
	      newPos--;
	      newS[newPos] = '\0';
	      // look for the previous '/'
	      char *lastSlash = (char*)strrchr( newS.c_str(), '/' );
	      // make the new string position after the slash
	      newPos = (lastSlash-newS.c_str())+1;
	      oldPos += 3;
	    }
	}

      newS[newPos] = s[oldPos];
      newPos++;
    }

  s = newS.c_str();
}

void Flu_File_Chooser::backCB()
{
  if( !currentHist ) return;
  if( currentHist->last )
    {
      currentHist = currentHist->last;
      walkingHistory = true;
      delayedCd = currentHist->path;
      fltk::add_timeout( 0.0f, Flu_File_Chooser::delayedCdCB, this );
    }
}

void Flu_File_Chooser::forwardCB()
{
  if( !currentHist ) return;
  if( currentHist->next )
    {
      currentHist = currentHist->next;
      walkingHistory = true;
      delayedCd = currentHist->path;
      fltk::add_timeout( 0.0f, Flu_File_Chooser::delayedCdCB, this );
    }
}

bool Flu_File_Chooser::correctPath( std::string &path )
{
  // the path may or may not be an alias, needing corrected
#ifdef WIN32
  // point to the correct desktop
  if( path == "/"+desktopTxt+"/" )
    {
      path = userDesktop;
      return true;
    }
  else if( path == userDesktop )
    return true;
  else if( path == "/"+desktopTxt+"/"+myComputerTxt+"/" ||
	   path == userDesktop+myComputerTxt+"/" )
    path = "/";
  else if( path == "/"+desktopTxt+"/"+myDocumentsTxt+"/" ||
	   path == userDesktop+myDocumentsTxt+"/" )
    path = userDocs;
#endif
  return false;
}

void Flu_File_Chooser::locationCB( const char *path )
{
#ifdef WIN32
  std::string p = path;
  if( p == favoritesTxt+"/" )
    favoritesCB();
  else if( p == desktopTxt+"/"+myComputerTxt+"/" )
    myComputerCB();
  else if( p == desktopTxt+"/"+myDocumentsTxt+"/" )
    documentsCB();
  else if( p == desktopTxt+"/" )
    desktopCB();
  // if the path leads off with "/Desktop/My Computer", then strip that part off and cd
  // to the remaining
  else
    {
      // seach for '(' and if present, extract the drive name and cd to it
      char *paren = strdup( strrchr( path, '(' ) );
      if( paren )
	{
	  char drive[] = "A:/";
	  drive[0] = toupper(paren[1]);
	  cd( drive );
	}
      else
	{
	  cd( path );
	}
      free( paren );
    }
#else
  cd( path );
#endif
  updateLocationQJ();
}

void Flu_File_Chooser::buildLocationCombo()
{
  // add all filesystems
  location->clear();

  fltk::Widget* n;
  std::string s;
  s = favoritesTxt+"/";
  fltk::ItemGroup* favoritesGrp = (fltk::ItemGroup*)location->add( s.c_str(), 0, NULL  );
  favoritesGrp->image( little_favorites );
  favoritesGrp->set_flag( fltk::OPENED );

#ifdef WIN32
  char volumeName[1024];
  s = desktopTxt+"/";
  n = location->add( s.c_str(), 0, NULL );
  n->image( little_desktop );
  n->set_flag( fltk::OPENED );
  s = desktopTxt+"/"+myDocumentsTxt+"/";
  n = location->add( s.c_str(), 0, NULL );
  n->image( documents );
  n->set_flag( fltk::OPENED );
  s = desktopTxt+"/"+myComputerTxt+"/";
  n = location->add( s.c_str(), 0, NULL );
  n->image( computer );
  n->set_flag( fltk::OPENED );
  // get the location and add them
  {
    if( refreshDrives )
      driveMask = GetLogicalDrives();
    DWORD mask = driveMask;

    for( int i = 0; i < 26; i++ )
      {
	drives[i] = "";
	driveIcons[i] = &disk_drive;
	if( mask & 1 )
	  {
	    s = desktopTxt+"/"+myComputerTxt+"/";
	    char drive[] = "A:";
	    char windrive[] = "A:\\";
	    windrive[0] = drive[0] = 'A' + i;
	    DWORD type;
	    if( refreshDrives )
	      {
		volumeName[0] = '\0';
		type = driveTypes[i] = GetDriveType( windrive );
		if( type != DRIVE_REMOVABLE && type != DRIVE_REMOTE )
		  GetVolumeInformation( windrive, volumeName, 1024, NULL, NULL, NULL, NULL, 0 );
		volumeNames[i] = volumeName;
	      }
	    else
	      {
		strncpy( volumeName, volumeNames[i].c_str(), 1024 );
		type = driveTypes[i];
	      }

	    //s += volume
	    const char *disk = "Disk";
	    switch( type )
	      {
	      case DRIVE_REMOVABLE:
		disk = strlen(volumeName)?volumeName: ( 1 < 2 ? diskTypesTxt[0].c_str() : diskTypesTxt[1].c_str() );
		driveIcons[i] = &floppy_drive;
		break;
	      case DRIVE_FIXED:
		disk = strlen(volumeName)?volumeName:diskTypesTxt[2].c_str();
		//driveIcons[i] = &disk_drive;
		break;
	      case DRIVE_CDROM:
		disk = strlen(volumeName)?volumeName:diskTypesTxt[3].c_str();
		driveIcons[i] = &cd_drive;
		break;
	      case DRIVE_REMOTE:
		disk = strlen(volumeName)?volumeName:diskTypesTxt[4].c_str();
		driveIcons[i] = &network_drive;
		break;
	      case DRIVE_RAMDISK:
		disk = strlen(volumeName)?volumeName:diskTypesTxt[5].c_str();
		driveIcons[i] = &ram_drive;
		break;
	      }
	    drives[i] = std::string(disk) + " (" + std::string(drive) + ")";
	    s += drives[i];
	    n = location->add( s.c_str(), 0, NULL );
	    n->image( driveIcons[i] );
	  }
	mask >>= 1;
      }
  }

  refreshDrives = false;

#elif defined __APPLE__

//   location->label( "/" );

  // get all volume mount points and add to the location combobox
  dirent **e;
  char *name;
  int num = fltk::filename_list( "/Volumes/", &e );
  if( num > 0 )
    {
      int i;
      for( i = 0; i < num; i++ )
	{
	  name = e[i]->d_name;

	  // ignore the "." and ".." names
	  if( strcmp( name, "." ) == 0 || strcmp( name, ".." ) == 0 ||
	      strcmp( name, "./" ) == 0 || strcmp( name, "../" ) == 0 ||
	      strcmp( name, ".\\" ) == 0 || strcmp( name, "..\\" ) == 0 )
	    continue;

	  // if 'name' ends in '/', remove it
	  if( name[strlen(name)-1] == '/' )
	    name[strlen(name)-1] = '\0';

	  std::string fullpath = "/Volumes/";
	  fullpath += name;
	  fullpath += "/";
	  location->add( fullpath.c_str(), 0, NULL );
	}
    }

#else

//   location->label( "/" );

  // get all mount points and add to the location combobox
  FILE	*fstab;		// /etc/mtab or /etc/mnttab file
  char	dummy[256], mountPoint[256], line[1024];	// Input line
  std::string mount;

  fstab = fltk::fltk_fopen( "/etc/fstab", "r" );	// Otherwise fallback to full list
  if( fstab )
    {
      while( fgets( line, 1024, fstab ) )
	{
	  if( line[0] == '#' || line[0] == '\n' )
	    continue;

	  // in fstab, mount point is second full string
	  sscanf( line, "%s %s", dummy, mountPoint );
	  mount = mountPoint;

	  // cull some stuff
	  if( mount[0] != '/' ) continue;
	  if( mount == "/" ) continue;
	  if( mount == "/boot" ) continue;
	  if( mount == "/proc" ) continue;

	  // now add the mount point
	  mount += "/";
	  location->add( mount.c_str(), 0, NULL );
	}

      fclose( fstab );
    }

#endif

  // Add current dir as first favorite
  xpmImage* icon = NULL;
#ifdef WIN32
  if( currentDir == (userHome+desktopTxt+"/") )
    icon = &little_desktop;
  if( currentDir == (userHome+myDocumentsTxt+"/") )
    icon = &documents;
  n = favoritesGrp->add( currentDir.c_str() );
#else
  n = favoritesGrp->add( currentDir.c_str() );
#endif
  n->image( icon );

  // Add favorites list
  int j;
  for ( int i = 0; i < favoritesList->children(); ++i )
    {
      const char* dir = favoritesList->child(i)->label();
      for ( j = 0; j < location->children(); ++j )
	{
	  if ( strcmp( location->child(j)->label(), dir ) == 0 )
	    break;
	}
      if ( j >= location->children() )
	{
	  favoritesGrp->add( dir );
	}
    }

  location->relayout();
  location->redraw();
}

void Flu_File_Chooser::clear_history()
{
  currentHist = history;
  while( currentHist )
    {
      History *next = currentHist->next;
      delete currentHist;
      currentHist = next;
    }
  currentHist = history = NULL;
  backBtn->deactivate();
  forwardBtn->deactivate();
}

void Flu_File_Chooser::addToHistory()
{
  // remember history
  // only store this path in the history if it is not the current directory
  if( currentDir.size() && !walkingHistory )
    {
      if( history == NULL )
	{
	  history = new History;
	  currentHist = history;
	  currentHist->path = currentDir;
	}
      else if( currentHist->path != currentDir )
	{
	  // since we are adding a new path, delete everything after this path
	  History *h = currentHist->next;
	  while( h )
	    {
	      History *next = h->next;
	      delete h;
	      h = next;
	    }
	  currentHist->next = new History;
	  currentHist->next->last = currentHist;
	  currentHist = currentHist->next;
	  currentHist->path = currentDir;
	}
      History * h = history;
      while( h )
	h = h->next;
    }
  walkingHistory = false;

  if( currentHist )
    {
      if( currentHist->last )
	backBtn->activate();
      else
	backBtn->deactivate();
      if( currentHist->next )
	forwardBtn->activate();
      else
	forwardBtn->deactivate();
    }
}

// treating the string as a '|' or ';' delimited sequence of patterns, strip them out and place in patterns
// return whether it is likely that "s" represents a regexp file-matching pattern
bool Flu_File_Chooser::stripPatterns( std::string s, FluStringVector* patterns )
{
  if( s.size() == 0 )
    return false;

  char *tok = strtok( (char*)s.c_str(), "|;" );
  int tokens = 0;
  while( tok )
    {
      tokens++;
      if( tok[0] == ' ' )
	tok++; // skip whitespace
      patterns->push_back( tok );
      tok = strtok( NULL, "|;" );
    }

  // if there is just a single token and it looks like it's not a pattern,
  // then it is probably JUST a filename, in which case it should not be
  // treated as a pattern
  if( _isProbablyAPattern( s.c_str() ) )
    return true;
  else if( tokens == 1 )
    {
      patterns->clear();
      return false;
    }
  else
    return true;
}





void Flu_File_Chooser::cd( const char *path )
{
  Entry *entry;
  char cwd[1024];


  if( !path || path[0] == '\0' )
    {
      path = getcwd( cwd, 1024 );
      if( !path )
	path = "./";
    }

  if( path[0] == '~' )
    {
      if( path[1] == '/' || path[1] == '\\' )
	sprintf( cwd, "%s%s", userHome.c_str(), path+2 );
      else
	sprintf( cwd, "%s%s", userHome.c_str(), path+1 );
      path = cwd;
    }

  lastSelected = 0;
  previewGroup->file = "";
  previewGroup->redraw();

  // filelist->scroll_to_beginning();

  bool listMode = !fileDetailsBtn->value() || streq( path, FAVORITES_UNIQUE_STRING );

#ifdef WIN32
  // refresh the drives if viewing "My Computer"
  if( strcmp( path, "/" ) == 0 )
    refreshDrives = true;
#endif

  filename.take_focus();

  trashBtn->deactivate();
  reloadBtn->activate();
  newDirBtn->activate();
  previewBtn->activate();
  previewBtn->value(true);
  previewBtn->do_callback();

  hiddenFiles->activate();
  addFavoriteBtn->activate();

  resize( x(), y(), w(), h() );
  if( listMode )
    {
      //filecolumns->hide();
      //filescroll->hide();
      fileDetailsGroup->hide();
      filelist->show();
      filelist->parent()->resizable( filelist );
    }
  else
    {
      filelist->hide();
      //filecolumns->show();
      //filescroll->show();
      //filescroll->parent()->resizable( filescroll );
      fileDetailsGroup->show();

      fileDetailsGroup->parent()->resizable( fileDetailsGroup );


      //updateEntrySizes();
    }

  std::string currentFile = filename.value();

  //fltk::focus( &filename );
  upDirBtn->activate();
  ok.activate();

  // check for favorites
  if( streq( path, FAVORITES_UNIQUE_STRING ) )
    {
      currentDir = FAVORITES_UNIQUE_STRING;
      addToHistory();

      newDirBtn->deactivate();
      previewBtn->deactivate();
      reloadBtn->deactivate();
      addFavoriteBtn->deactivate();
      hiddenFiles->deactivate();
      location->text( favoritesTxt.c_str() );
      updateLocationQJ();

      filelist->clear();
      filedetails->clear();

      for( int i = 0; i < favoritesList->children(); ++i )
	{
	  entry = new Entry( favoritesList->child(i)->label(), ENTRY_FAVORITE,
			     false/*fileDetailsBtn->value()*/, this );
	  entry->updateSize();
	  entry->updateIcon();
	  if( listMode )
	    {
	      filelist->add(entry);
	    }
	  else
	    {
	      filedetails->add(entry);
	    }
	}
      if( listMode )
	{
	  filelist->relayout();
	}
      else
	{
	  filedetails->relayout();
	}

      relayout();
      redraw();
      ok.deactivate();
      return;
    }
  // check for the current directory
  else if( streq( path, "." ) || streq( path, "./" ) || streq( path, ".\\" ) )
    {
      // do nothing. just rescan this directory
    }
  // check for parent directory
  else if( streq( path, ".." ) || streq( path, "../" ) || streq( path, "..\\" ) )
    {
      // if we are viewing the favorites and want to go back a directory, go to the previous directory
      if( currentDir == FAVORITES_UNIQUE_STRING )
	{
	  backCB();
	  return;
	}
#ifdef _WIN32
      // if we are at the desktop already, then we cannot go back any further
      //if( currentDir == "/Desktop/" )
      //{
	  // do nothing
      //}
      //else if( currentDir == userHome+"Desktop/" )
      //currentDir = userHome;
      // if we are viewing "My Computer" and want to go back a directory, go to the desktop
      if( currentDir == "/" )
	{
	  //currentDir = userDesktop;//userHome + "Desktop";
	  // do nothing
	}
      // if we are at a top level drive, go to "My Computer" (i.e. "/")
      else if( currentDir[1] == ':' && currentDir[3] == '\0' )
	currentDir = "/";
      else
#else
	// if the current directory is already as far back as we can go, ignore
	if( currentDir != "/" )
#endif
	  {
	    // strip everything off the end to the next "/"
	    size_t end = currentDir.size()-1;
	    currentDir[end] = '\0';
	    while( currentDir[end] != '/' )
	      {
		currentDir[end] = '\0';
		end--;
	      }
	  }
    }
  // check for absolute path
#ifdef _WIN32
  else if( path[1] == ':' || path[0] == '/' )
#else
  else if( path[0] == '/' )
#endif
    {
      currentDir = path;
    }
  // else relative path
  else
    {
      // concatenate currentDir with path to make an absolute path
       currentDir += path;
    }


  int numDirs = 0, numFiles = 0;
  filelist->clear();
  filedetails->clear();

  cleanupPath( currentDir );

#ifdef _WIN32
  bool isTopDesktop = ( currentDir == (desktopTxt+"/") );
  bool isDesktop = correctPath( currentDir );
  if( isTopDesktop )
    upDirBtn->deactivate();
#else
  if( currentDir == "/" )
    upDirBtn->deactivate();
#endif

#ifdef _WIN32
  bool root = false;
  // check for my computer
  if( currentDir == "/" )
    {
      ok.deactivate();
      root = true;
      for( int i = 0; i < 26; i++ )
	{
	  if( drives[i][0] != '\0' )
	    {
	      char drive[] = "A:/";
	      drive[0] = 'A' + i;
	      entry = new Entry( drive, ENTRY_DRIVE, fileDetailsBtn->value(), this );
	      switch( driveTypes[i] )
		{
		case DRIVE_REMOVABLE: entry->description = diskTypesTxt[0].c_str(); break;
		case DRIVE_FIXED: entry->description = diskTypesTxt[2].c_str(); break;
		case DRIVE_CDROM: entry->description = diskTypesTxt[3].c_str(); break;
		case DRIVE_REMOTE: entry->description = diskTypesTxt[4].c_str(); break;
		case DRIVE_RAMDISK: entry->description = diskTypesTxt[5].c_str(); break;
		}
	      entry->icon = driveIcons[i];
	      entry->altname = drives[i];
	      entry->updateSize();
	      entry->updateIcon();
	      if( listMode )
		{
		  filelist->add(entry);
		}
	      else
		{
		  filedetails->add(entry);
		}
	    }
	}
      if( listMode )
	{
	  filelist->relayout();
	} 
      else
	{
	  filedetails->relayout();
	}

      relayout();
      redraw();
    }
  // check for desktop. if so, add My Computer and My Documents
  else if( isDesktop )
    {
      entry = new Entry( myDocumentsTxt.c_str(), ENTRY_MYDOCUMENTS, fileDetailsBtn->value(), this );
      entry->updateSize();
      entry->updateIcon();
      if( listMode )
	{
	  filelist->add(entry);
	}
      else
	{
	  filedetails->add(entry);
	}
      entry = new Entry( myComputerTxt.c_str(), ENTRY_MYCOMPUTER, fileDetailsBtn->value(), this );
      entry->updateSize();
      entry->updateIcon();
      if( listMode )
	{
	  filelist->add(entry);
	  filelist->relayout();
	}
      else
	{
	  filedetails->add(entry);
	  filedetails->relayout();
	}
      numDirs += 2;
    }
#endif

  // see if currentDir is in fact a directory
  // if so, make sure there is a trailing "/" and we're done
  if( fltk::filename_isdir( currentDir.c_str() ) || currentDir=="/" )
    {
      if( currentDir[strlen(currentDir.c_str())-1] != '/' )
	currentDir += "/";
#ifdef WIN32
      if( filename.value()[1] != ':' )
#else
      if( filename.value()[0] != '/' )
#endif
	{
	  if( !(selectionType & SAVING ) )
	    filename.value( "" );
	}
      if( !(selectionType & SAVING ) )
	currentFile = "";
    }

  // now we have the current directory and possibly a file at the end
  // try to split into path and file
  if( currentDir[currentDir.size()-1] != '/' )
    {
       char *lastSlash = (char*)strrchr( currentDir.c_str(), '/' );
      if( lastSlash )
	{
	  currentFile = lastSlash+1;
	  currentDir  = currentDir.substr(0, lastSlash-currentDir.c_str());
	}
    }
  // make sure currentDir ends in '/'
  if( currentDir[currentDir.size()-1] != '/' )
    currentDir += "/";

#ifdef _WIN32
  {
    std::string tmp = currentDir;
    if( isTopDesktop )
      currentDir = desktopTxt+"/";
    addToHistory();
    if( isTopDesktop )
      currentDir = tmp;
  }
#else
  addToHistory();
#endif

  delayedCd = "./";

#ifdef _WIN32
  // set the location input value
  // check for drives
  if( currentDir.size() > 1 && currentDir[1] == ':' && 
      currentDir.size() == 2 )
    {
      location->text( currentDir.c_str() );
    }
  else if( currentDir == "/" )
    location->text( myComputerTxt.c_str() );
  else
#endif
    {
      location->text( currentDir.c_str() );
    }

  buildLocationCombo();
  updateLocationQJ();

#ifdef _WIN32
  if( root )
    return;
#endif

  std::string pathbase, fullpath;
  bool isDir = false, isCurrentFile = false;
  const char *lastAddedFile = NULL, *lastAddedDir = NULL;

  pathbase = currentDir;

  // take the current pattern and make a list of filter pattern strings
  FluStringVector currentPatterns;
  {
    std::string pat = patterns[filePattern->value()];
    while( pat.size() )
      {
	size_t p = pat.find( ',' );
	if( p == std::string::npos )
	  {
	    if( pat != "*" )
	      pat = "*." + pat;
	    currentPatterns.push_back( pat );
	    break;
	  }
	else
	  {
	    std::string s = pat.c_str() + p + 1;
	    pat[p] = '\0';
	    if( pat != "*" )
	      pat = "*." + pat;
	    currentPatterns.push_back( pat );
	    pat = s;
	  }
      }
  }

  // add any user-defined patterns
  FluStringVector userPatterns;
  // if the user just hit <Tab> but the filename input area is empty,
  // then use the current patterns
  if( !filenameTabCallback || currentFile != "*" )
  {
    stripPatterns( currentFile, &userPatterns );
  }

  typedef std::vector< std::string > Directories;
  Directories dirs;
  typedef std::vector< std::string > Files;
  Files files;

  mrv::Sequences tmpseqs;

  // read the directory
  dirent **e;
  char *name;


  int num = fltk::filename_list( pathbase.c_str(), &e );
  if( num > 0 )
    {
      int i;
      for( i = 0; i < num; i++ )
	{
	  name = e[i]->d_name;

	  // ignore the "." and ".." names
	  if( strcmp( name, "." ) == 0 || strcmp( name, ".." ) == 0 ||
	      strcmp( name, "./" ) == 0 || strcmp( name, "../" ) == 0 ||
	      strcmp( name, ".\\" ) == 0 || strcmp( name, "..\\" ) == 0 )
	    continue;

	  // if 'name' ends in '/', remove it
	  if( name[strlen(name)-1] == '/' )
	    name[strlen(name)-1] = '\0';

	  // file or directory?
	  fullpath = pathbase + name;
	  isDir = ( fltk::filename_isdir( fullpath.c_str() ) != 0 );

	  // was this file specified explicitly?
	  isCurrentFile = ( currentFile == name );

#ifndef _WIN32
	  // filter hidden files
	  if( !isCurrentFile && !hiddenFiles->value() && ( name[0] == '.' ) )
	    continue;
#endif

	  // only directories?
	  if( (selectionType & DIRECTORY) &&
	      !isDir &&
	      !(selectionType & STDFILE) &&
	      !(selectionType & DEACTIVATE_FILES) )
	    continue;

	  //if( !isDir /*!isCurrentFile*/ )
	    {
	      // filter according to the user pattern in the filename input
	      if( userPatterns.size() )
		{
		  bool cull = true;
		  for( unsigned int i = 0; i < userPatterns.size(); i++ )
		    {
		      if( flu_filename_match( name,
					      userPatterns[i].c_str(),
					      true ) != 0 )
			{
			  cull = false;
			  break;
			}
		    }
		  if( cull )
		    {
		      // only filter directories if someone just hit <TAB>
		      if( !isDir || ( isDir && filenameTabCallback ) )
                      {
			continue;
                      }
		    }
		}
	      // filter files according to the current pattern
	      else
		{
		  bool cull = true;
		  for( unsigned int i = 0; i < currentPatterns.size(); i++ )
		    {
		      if( flu_filename_match( name,
					      currentPatterns[i].c_str(),
					      true ) != 0 )
			{
			  cull = false;
			  break;
			}
		    }
		  if( cull )
		    {
		      // only filter directories if someone just hit <TAB>
		      if( !isDir || ( isDir && filenameTabCallback ) )
                      {
                          continue;
                      }
		    }
		}
	    }

	    if ( isDir )
	      {
		dirs.push_back( name );
	      }
	    else
	      {

		 bool is_sequence = false;

		 std::string root, frame, view, ext;
		 bool ok = mrv::split_sequence( root, frame, view, ext, name );

		 if ( compact_files() )
		 {
		    if ( root != "" && frame != "" )
		       is_sequence = true;
		    
		    std::string tmp = ext;
		    std::transform( tmp.begin(), tmp.end(), tmp.begin(),
				    (int(*)(int)) tolower);
		    if ( tmp == N_(".avi")  || tmp == N_(".mov")  || 
			 tmp == N_(".divx") || tmp == N_(".mp3")  ||
			 tmp == N_(".wmv")  || tmp == N_(".mpeg") ||
			 tmp == N_(".mpg")  || tmp == N_(".mp4")  ||
			 tmp == N_(".qt")   || tmp == N_(".wav")  ||
			 tmp == N_(".vob")  || tmp == N_(".icc")  ||
			 tmp == N_(".wav")  || tmp == N_(".icm")  ||
                         tmp == N_(".vp9")  || tmp == N_(".ctl") || 
                         tmp == N_(".xml")  )
		       is_sequence = false;
		 }
		 else
		 {
		    is_sequence = false;
		 }


		 if ( is_sequence )
		  {
                     mrv::Sequence seq;
                     seq.ext = ext;
                     seq.view = view;
                     seq.number = frame;
                     seq.root = root;
                     tmpseqs.push_back( seq );
		  }
		else
		  {
		    files.push_back( name );
		  }
	      }
	}

      //
      // Add all directories first
      //
      {
	Directories::const_iterator i = dirs.begin();
	Directories::const_iterator e = dirs.end();
	for ( ; i != e; ++i )
	  {
	    entry = new Entry( (*i).c_str(), ENTRY_DIR,
			       fileDetailsBtn->value(), this );
	    if (!entry) continue;

	    if( listMode )
	      filelist->insert( *entry, 0 );
	    else
	      filedetails->insert( *entry, 0 );
	    ++numDirs;
	    lastAddedDir = entry->filename.c_str();
	  }
      }

      //
      // Then, sort sequences and collapse them into a single file entry
      //
      {
         std::sort( tmpseqs.begin(), tmpseqs.end(), mrv::SequenceSort() );


	std::string root;
	std::string first;
	std::string number;
        std::string view;
	std::string ext;
	int zeros = -1;

	std::string seqname;
        mrv::Sequences seqs;

	{
           mrv::Sequences::iterator i = tmpseqs.begin();
           mrv::Sequences::iterator e = tmpseqs.end();
           for ( ; i != e; ++i )
	    {
	      const char* s = (*i).number.c_str();
	      int z = 0;
	      for ( ; *s == '0'; ++s )
		++z;

	      if ( (*i).root != root || (*i).view != view ||
                   (*i).ext != ext || ( zeros != z && z != zeros-1 ) )
              {
		  // New sequence
		  if ( seqname != "" )
		    {
                       mrv::Sequence seq;
                       seq.root = seqname;
                       seq.number = seq.ext = first;
                       if ( first != number )
                       {
			  seq.ext = number;
                       }
                       seq.view = (*i).view;
                       seqs.push_back( seq );
		    }

		  root   = (*i).root;
		  zeros  = z;
		  number = first = (*i).number;
                  view   = (*i).view;
		  ext    = (*i).ext;

		  seqname  = root;
		  if ( z == 0 )
		    seqname += "%d";
		  else
		    {
		      seqname += "%0";
		      char buf[19]; buf[18] = 0;
#ifdef WIN32
		      seqname += itoa( int((*i).number.size()), buf, 10 );
#else
		      sprintf( buf, "%ld", (*i).number.size() );
		      seqname += buf;
#endif
		      seqname += "d";
		    }
                  seqname += view;
		  seqname += ext;
		}
	      else
		{
		  zeros  = z;
		  number = (*i).number;
		}
	    }
	}

	if ( root != "" )
	  {
             mrv::Sequence seq;
             seq.root = seqname;
             seq.number = seq.ext = first;
             seq.view = view;
             if ( first != number )
             {
		seq.ext = number;
             }
             seqs.push_back( seq );
	  }
        
        mrv::Sequences::const_iterator i = seqs.begin();
        mrv::Sequences::const_iterator e = seqs.end();
	for ( ; i != e; ++i )
	{
	  entry = new Entry( (*i).root.c_str(), ENTRY_SEQUENCE,
			     fileDetailsBtn->value(), this );
	  entry->isize = 1 + ( atoi( (*i).ext.c_str() ) -
			       atoi( (*i).number.c_str() ) );
	  entry->altname = (*i).root.c_str();
          if ( listMode )
          {
              entry->altname += " ";
              entry->altname += (*i).number;
              entry->filesize = (*i).number;
              if ( entry->isize > 1 )
              {
                  entry->filesize += "-";
                  entry->filesize += (*i).view;
                  entry->filesize += (*i).ext;
                  entry->altname += (*i).view;
                  entry->altname += "-";
                  entry->altname += (*i).ext;
              }
          }

	  entry->updateIcon();

	  ++numFiles;
	  if( listMode )
	    filelist->add( entry );
	  else
	    filedetails->add( entry );
	}

      }

      {
	Files::const_iterator i = files.begin();
	Files::const_iterator e = files.end();
	for ( ; i != e; ++i )
	{
            entry = new Entry( (*i).c_str(), ENTRY_FILE,
                               fileDetailsBtn->value(), this );

            if( listMode )
            {
                filelist->add( entry );
            }
            else
            {
                filedetails->add( entry );
            }
            numFiles++;
            lastAddedFile = entry->filename.c_str();


            // get some information about the file
            fullpath = pathbase + *i;
            struct stat s;
            ::stat( fullpath.c_str(), &s );

            // store size as human readable and sortable integer
            entry->isize = s.st_size;
            if( isDir && entry->isize == 0 )
            {
                entry->filesize = "";
            }
            else
            {
                char buf[32];
	      
                if( (entry->isize >> 30) > 0 ) // gigabytes
                {
                    double GB = double(entry->isize)/double(1<<30);
                    sprintf( buf, "%.1f GB", GB );
                }
                else if( (entry->isize >> 20) > 0 ) // megabytes
                {
                    double MB = double(entry->isize)/double(1<<20);
                    sprintf( buf, "%.1f MB", MB );
                }
                else if( (entry->isize >> 10) > 0 ) // kilabytes
                {
                    double KB = double(entry->isize)/double(1<<10);
                    sprintf( buf, "%.1f KB", KB );
                }
                else // bytes
                {
                    sprintf( buf, "%d bytes", (int)entry->isize );
                }
                entry->filesize = buf;
                entry->updateIcon();
            }

            // store date as human readable and sortable integer
            entry->date = formatDate( ctime( &s.st_mtime ) );//ctime( &s.st_mtime );
            entry->idate = s.st_mtime;

            // convert the permissions into UNIX style rwx-rwx-rwx (user-group-others)
            /*
              unsigned int p = s.st_mode;
	    entry->pU = bool(p&S_IRUSR)<<2 | bool(p&S_IWUSR)<<1 | bool(p&S_IXUSR);
	    entry->pG = bool(p&S_IRGRP)<<2 | bool(p&S_IWGRP)<<1 | bool(p&S_IXGRP);
	    entry->pO = bool(p&S_IROTH)<<2 | bool(p&S_IWOTH)<<1 | bool(p&S_IXOTH);
	    char* perms[8] = { "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx" };
	    entry->permissions = perms[entry->pU];
	    entry->permissions += perms[entry->pG];
	    entry->permissions += perms[entry->pO];
	  */


	  entry->updateSize();
	  entry->updateIcon();

	  // was this file specified explicitly?
	  isCurrentFile = ( currentFile == entry->filename );
	  if( isCurrentFile )
	    {
	      filename.value( currentFile.c_str() );
	      entry->set_selected();
	      lastSelected = entry;
	      if( entry->type == ENTRY_FILE )
		{
		  previewGroup->file = currentDir + currentFile.c_str();
		}
	      previewGroup->redraw();

	      filelist->scroll_to( entry );
	      filedetails->scroll_to( entry );

	      //break;
	    }
	} // i != e

      }

      for( i = 0; i < num; i++ )
	free((void*)(e[i]));
      free((void*)e);
    }  // num > 0

  // sort the files: directories first, then files
  if( listMode ) {
    filelist->sort( numDirs );
  }
  else {
    filedetails->sort( numDirs );
  }


  // see if the user pushed <Tab> in the filename input field
  if( filenameTabCallback )
    {
      filenameTabCallback = false;

      std::string prefix = commonStr();

      if( numDirs == 1 &&
	  currentFile == (std::string(lastAddedDir)+"*") )
	{
	  delayedCd = lastAddedDir;
	  fltk::add_timeout( 0.0f, Flu_File_Chooser::delayedCdCB, this );
	}

      if( numDirs == 1 && numFiles == 0 )
	{
#ifdef WIN32
	  if( filename.value()[1] == ':' )
#else
	  if( filename.value()[0] == '/' )
#endif
	    {
	      std::string s = currentDir + lastAddedDir + "/";
	      filename.value( s.c_str() );
	    }
	  else
	    filename.value( lastAddedDir );
	}
      else if( numFiles == 1 && numDirs == 0 )
	{
#ifdef WIN32
	  if( filename.value()[1] == ':' )
#else
	  if( filename.value()[0] == '/' )
#endif
	    {
	      std::string s = currentDir + lastAddedFile;
	      filename.value( s.c_str() );
	    }
	  else
	    filename.value( lastAddedFile );
	}
      else if( prefix.size() >= currentFile.size() )
	{
#ifdef WIN32
	  if( filename.value()[1] == ':' )
#else
	  if( filename.value()[0] == '/' )
#endif
	    {
	      std::string s = currentDir + prefix;
	      filename.value( s.c_str() );
	    }
	  else
	    filename.value( prefix.c_str() );
	}

      if( currentFile == "*" &&
#ifdef WIN32
	  filename.value()[1] != ':' )
#else
	  filename.value()[0] != '/' )
#endif
        {
	  filename.value( "" );
	}
    }

  // see if the user pushed <Enter> in the filename input field
  if( filenameEnterCallback )
    {
      filenameEnterCallback = false;

#ifdef WIN32
      if( filename.value()[1] == ':' )
#else
      if( filename.value()[0] == '/' )
#endif
	filename.value( "" );

      //if( isCurrentFile && numFiles == 1 )
      if( !_isProbablyAPattern( filename.value() ) )
	okCB();
    }

  if( _isProbablyAPattern( filename.value() ) )
    filename.position( 0, filename.size() );
  else
    filename.position( filename.size(), filename.size() );
  filename.take_focus();

  // Handle loading of icons
  fltk::Group *g = getEntryGroup();
  int c = g->children();
  for ( int i = 0; i < c; ++i )
  {
      Entry* e = (Entry*) g->child(i);
      if ( e->type == ENTRY_SEQUENCE || e->type == ENTRY_FILE )
      {
          std::cerr << "load real icon for " << e->filename << std::endl;
          loadRealIcon( e );
      }
  }

  if( listMode )
  {
    filelist->relayout();
    filelist->redraw();
  }
  else
  {
    filedetails->relayout();
    filedetails->redraw();
  }
  relayout();
  redraw();
}

// find the prefix string that is common to all entries in the list
std::string Flu_File_Chooser::commonStr()
{
  std::string common;
  unsigned index = 0;
  const char* name;
  size_t len;
  int i;
  fltk::Group *g = getEntryGroup();
  for(;;)
    {
      bool allSkipped = true;
      for( i = 0; i < g->children(); i++ )
	{
	  name = ((Entry*)g->child(i))->filename.c_str();
	  len = strlen( name );
	  if( index >= len )
	    continue;
	  allSkipped = false;
	  if( i == 0 )
	    common.push_back( name[index] );
	  else if( toupper(common[index]) != toupper(name[index]) )
	    {
	      common[index] = '\0';
	      return common;
	    }
	}
      if( allSkipped )
	break;
      index++;
    }
  return common;
}

std::string retname;

static const char* _flu_file_chooser( const char *message, const char *pattern,
				      const char *filename, int type,
				      int *count = 0, 
				      FluStringVector *filelist = 0,
				      const bool compact_files = true )
{
   static Flu_File_Chooser *fc = NULL;

  if( !fc )
    {
       fc = new Flu_File_Chooser( filename, pattern, type, message, 
				  compact_files );
      if (fc && retname != "")
      {
	 fc->value( retname.c_str() );
      }
    }
  else
    {
      fc->type( type );
      fc->clear_history();
      fc->label( message );
      fc->compact_files( compact_files );
      if( !filename || filename[0] == '\0' )
	{
	  if( (!pattern || !fc->filter() || strcmp(pattern,fc->filter())) && fc->value() )
	    {
	      // if pattern is different, remove name but leave old directory:
	      retname = fc->value();
	      char *p = (char*)strrchr( retname.c_str(), '/' );
	      if( p )
		{
		  // If the filename is "/foo", then the directory will be "/", not ""
		  if( p == retname.c_str() )
		    retname[1] = '\0';
		  else
		    p[1] = '\0';
		}
	    }
	  fc->filter( pattern );
	  fc->value( retname.c_str() );
	}
      else
	{
	  fc->filter( pattern );
	  fc->value( filename );
	}
    }

  fc->exec();

  Group::current(0);

  if( fc->value() )
    {
      if( count && filelist )
	{
	  *count = fc->count();
	  for( int i = 1; i <= *count; ++i )
	    filelist->push_back( std::string(fc->value(i)) );
	}
      retname = fc->value();
      return retname.c_str();
    }
  else
    return 0;
}

int flu_multi_file_chooser( const char *message, const char *pattern, const char *filename, FluStringVector& filelist, const bool compact_files )
{
  int count = 0;
  _flu_file_chooser( message, pattern, filename, Flu_File_Chooser::MULTI,
		     &count, &filelist, compact_files );
  return count;
}

const char* flu_file_chooser( const char *message, const char *pattern, const char *filename )
{
  return _flu_file_chooser( message, pattern, filename, Flu_File_Chooser::SINGLE );
}

const char* flu_save_chooser( const char *message, const char *pattern, const char *filename )
{
  return _flu_file_chooser( message, pattern, filename, Flu_File_Chooser::SINGLE | Flu_File_Chooser::SAVING );
}

const char* flu_dir_chooser( const char *message, const char *filename )
{
  return _flu_file_chooser( message, "*", filename, Flu_File_Chooser::DIRECTORY );
}

const char* flu_dir_chooser( const char *message, const char *filename, bool showFiles )
{
  if( showFiles )
    return _flu_file_chooser( message, "*", filename,
			      Flu_File_Chooser::DIRECTORY | Flu_File_Chooser::DEACTIVATE_FILES );
  else
    return( flu_dir_chooser( message, filename ) );
}

const char* flu_file_and_dir_chooser( const char *message, const char *filename )
{
  return _flu_file_chooser( message, "*", filename, Flu_File_Chooser::STDFILE );
}
