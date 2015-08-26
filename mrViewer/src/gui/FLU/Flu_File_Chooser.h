// $Id: Flu_File_Chooser.h,v 1.63 2004/10/18 15:14:58 jbryan Exp $

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



#ifndef _FLU_FILE_CHOOSER_H
#define _FLU_FILE_CHOOSER_H

#include <vector>
#include <string>
#include <ctime>

#include <fltk/DoubleBufferWindow.h>
#include <fltk/Input.h>
#include <fltk/xpmImage.h>
#include <fltk/PopupMenu.h>
#include <fltk/TiledGroup.h>
#include <fltk/PackedGroup.h>
//#include <fltk/Scrollbar.h>
#include <fltk/ScrollGroup.h>
#include <fltk/CheckButton.h>
#include <fltk/ToggleButton.h>
#include <fltk/ReturnButton.h>
#include <fltk/HighlightButton.h>
#include <fltk/Browser.h>
#include <fltk/InputBrowser.h>

#include <boost/thread/recursive_mutex.hpp>

#include "FLU/flu_export.h"




#include "FLU/Flu_Combo_Tree.h"
#include "FLU/Flu_Wrap_Group.h"


typedef std::vector< std::string > FluStringVector;



extern std::string retname;

FLU_EXPORT const char* flu_file_chooser( const char *message, const char *pattern, const char *filename );
FLU_EXPORT int flu_multi_file_chooser( const char *message, const char *pattern, const char *filename, FluStringVector& filelist, const bool compact );
FLU_EXPORT const char* flu_save_chooser( const char *message, const char *pattern, const char *filename );
FLU_EXPORT const char* flu_dir_chooser( const char *message, const char *filename );
FLU_EXPORT const char* flu_dir_chooser( const char *message, const char *filename, bool showFiles );
FLU_EXPORT const char* flu_file_and_dir_chooser( const char *message, const char *filename );

//! A file and directory choosing widget that looks and acts similar to the stock Windows file chooser
class FLU_EXPORT Flu_File_Chooser : public fltk::DoubleBufferWindow
{

  friend class FileInput;
  class FileInput : public fltk::Input
    {
    public:
      FileInput( int x, int y, int w, int h, const char *l );
      ~FileInput();

      void set_chooser( Flu_File_Chooser* c ) { chooser = c; }

      int handle( int event );
    protected:
      Flu_File_Chooser *chooser;
    };

 public:

  //! strings to be set by a programmer to the correct phrase or 
  //! name for their language (they are in english by default)   */
  static std::string favoritesTxt;
  static std::string desktopTxt;
  static std::string myComputerTxt;
  static std::string myDocumentsTxt;

  static std::string filenameTxt;
  static std::string okTxt;
  static std::string cancelTxt;
  static std::string locationTxt;
  static std::string showHiddenTxt;
  static std::string fileTypesTxt;
  static std::string directoryTxt;
  static std::string allFilesTxt;
  static std::string defaultFolderNameTxt;

  static std::string backTTxt;
  static std::string forwardTTxt;
  static std::string upTTxt;
  static std::string reloadTTxt;
  static std::string trashTTxt;
  static std::string newDirTTxt;
  static std::string addFavoriteTTxt;
  static std::string previewTTxt;
  static std::string listTTxt;
  static std::string wideListTTxt;
  static std::string detailTTxt;

  static std::string detailTxt[];
  static std::string contextMenuTxt[3];
  static std::string diskTypesTxt[6];

  static std::string createFolderErrTxt;
  static std::string deleteFileErrTxt;
  static std::string fileExistsErrTxt;
  static std::string renameErrTxt;

  static bool singleButtonTravelDrawer;

  typedef boost::recursive_mutex Mutex;
  Mutex  mutex;

  //! File entry type
  enum { 
    ENTRY_NONE = 1,         /*!< An empty (or non-existant) entry */
    ENTRY_DIR = 2,          /*!< A directory entry */
    ENTRY_FILE = 4,         /*!< A file entry */
    ENTRY_FAVORITE = 8,     /*!< A favorite entry */
    ENTRY_DRIVE = 16,       /*!< An entry that refers to a disk drive */
    ENTRY_MYDOCUMENTS = 32, /*!< The entry referring to the current user's documents */
    ENTRY_MYCOMPUTER = 64,  /*!< The entry referring to "My Computer" in Windows */
    ENTRY_SEQUENCE = 128,   /*!< An image sequence */
  };

  //! Chooser type
  enum { 
    SINGLE = 0,            /*!< Choose a single file or directory */
    MULTI = 1,             /*!< Choose multiple files or directories */
    DIRECTORY = 4,         /*!< Choose directories (choosing files is implicit if this bit is clear) */
    DEACTIVATE_FILES = 8,  /*!< When choosing directories, also show the files in a deactivated state */
    SAVING = 16,           /*!< When choosing files, whether to keep the current filename always in the input area */
    STDFILE = 32           /*!< Choose both files and directories at the same time */
  };

  //! Structure holding the info needed for custom file types
  struct FileTypeInfo
  {
    fltk::Image *icon;
    std::string extensions;
    std::string type, shortType;
  };

  //! Constructor opening a file chooser with title \b title visiting directory \b path with files filtered according to \b pattern. \b type is a logical OR of Flu_File_Chooser::SINGLE, Flu_File_Chooser::MULTI, and Flu_File_Chooser::DIRECTORY 
  Flu_File_Chooser( const char *path, const char *pattern, int type, 
		    const char *title, const bool compact );

  //! Destructor
  ~Flu_File_Chooser();

  //! Add a custom callback that is called when the user right-clicks on an entry
  /*! \param type is the type of entry to handle (i.e. a logical OR of \c ENTRY_NONE, \c ENTRY_DIR, \c ENTRY_FILE, \c ENTRY_FAVORITE, \c ENTRY_DRIVE, \c ENTRY_MYDOCUMENTS, \c ENTRY_MYCOMPUTER). To add a "nothing" handler (when the user right-clicks on nothing), use ENTRY_NONE
    \param ext is the extension of the file that will cause this handler to be added to the popup menu
    \param name is the name that will appear in the popup menu for this handler
   */
  static void add_context_handler( int type, const char *ext, const char *name,
				   void (*cb)(const char*,int,void*), void *cbd );

  //! Add descriptive information and an icon for a file type
  /*! \param extensions is a space- or comma-delimited list of file extensions, or \c NULL for directories. e.g. "zip,tgz,rar"
    \param short_description is a short description (!) of the file type. e.g. "Compressed Archive"
    \param icon is an optional custom icon to use for this file type
   */
  static void add_type( const char *extensions, const char *short_description, fltk::Image *icon = NULL );

  //! deprecated - do not use - right click to change filenames
  inline void allow_file_editing( bool b )
    { fileEditing = b; }

  //! deprecated - do not use - right click to change filenames
  inline bool allow_file_editing() const
    { return fileEditing; }

  inline void compact_files( const bool compact ) { _compact = compact; }

  inline bool compact_files() const { return _compact; }

  //! Set whether file sorting is case insensitive. Default value is case-insensitive for windows, case-sensitive for everything else
  inline void case_insensitive_sort( bool b )
    { caseSort = !b; }

  //! Get whether file sorting is case insensitive
  inline bool case_insensitive_sort() const
    { return !caseSort; }

  //! Change the current directory the chooser is browsing to \b path
  void cd( const char *path );

  //! Clear the history of which directories have been visited
  void clear_history();

  //! \return how many files are selected
  int count();

  //! Set the default icon to use for all files for which no other icon has been specified
  inline void default_file_icon( fltk::Image* i )
    { defaultFileIcon = i; }

  //! Alias for cd()
  inline void directory( const char *d )
    { cd( d ); }

  //! Alias for pattern()
  inline void filter( const char *p )
    { pattern( p ); }

  //! Alias for pattern()
  inline const char* filter() const
    { return pattern(); }

  //! \return a pointer to a FileTypeInfo structure for files with type \b extension
  static FileTypeInfo *find_type( const char *extension );

  //! \return the current directory that the browser is visiting
  inline const char* get_current_directory() const
    { return currentDir.c_str(); }

  //! Override of Fl_Double_Window::handle()
  int handle( int event );

  //! Change the file filter pattern to \b p
  void pattern( const char *p );

  //! Get the current file filter pattern
  inline const char* pattern() const
    { return rawPattern.c_str(); }

  //! Set the state of the preview button
  inline void preview( bool b )
    { previewBtn->value(b); previewBtn->do_callback(); }

  //! Get the state of the preview button
  inline int preview() const
    { return previewBtn->value(); }

  //! Refresh the current directory
  inline void rescan() { reloadCB(); }

  //! Override of Fl_Double_Window::resize()
  void resize( int x, int y, int w, int h );

  //! Select all entries (only valid for multiple-selections)
  void select_all();

  //! Set a custom sorting function for sorting entries based on filename
  inline void set_sort_function( int (*cb)(const char*,const char*) )
    { customSort = cb; rescan(); }

  //! Set the type of the chooser (see constructor)
  inline void type( int t )
    { selectionType = t; rescan(); }

  //! Get the type of the chooser
  inline int type( int t ) const
    { return selectionType; }

  //! Unselect all entries
  void unselect_all();

  //! Set the current file the chooser is selecting
  void value( const char *v );

  //! Get the current file the chooser is selecting
  const char *value();

  //! For MULTI file queries, get selected file \b n (base 1 - i.e. 1 returns the first file, 2 the second, etc)
  const char *value( int n );

  FileInput filename;

  bool _compact;

  fltk::ReturnButton ok;
  fltk::Button cancel;

  // apparently there is a bug in VC6 that prevents friend classes 
  // from accessing non-public members. stupid windows
  // several other compilers were reported to have a problem with this too, so 
  // i'm just making the whole class public to eliminate potential problems.
  // bad c++ - i know...
  //#ifndef WIN32
  //protected:
  //#endif

  class ContextHandler
    {
    public:
      std::string ext, name;
      int type;
      void (*callback)(const char*,int,void*);
      void *callbackData;
      inline ContextHandler& operator =( const ContextHandler &c )
      { ext = c.ext; name = c.name; type = c.type; callback = c.callback; callbackData = c.callbackData; return *this; }
    };

  typedef std::vector< ContextHandler >  ContextHandlerVector;
  static ContextHandlerVector contextHandlers;


  fltk::CheckButton* hiddenFiles;
  Flu_Combo_Tree*    location;

  static void timeout( void* c );

  inline static void _backCB( fltk::Widget *w, void *arg )
    { ((Flu_File_Chooser*)arg)->backCB(); }
  void backCB();

  inline static void _forwardCB( fltk::Widget *w, void *arg )
    { ((Flu_File_Chooser*)arg)->forwardCB(); }
  void forwardCB();

  inline static void _sortCB( fltk::Widget *w, void *arg )
    { ((Flu_File_Chooser*)arg)->sortCB( w ); }
  void sortCB( fltk::Widget *w );

  inline static void _previewCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->previewCB(); }
  void previewCB();

  inline static void _listModeCB( fltk::Widget *w, void *arg )
    { ((Flu_File_Chooser*)arg)->listModeCB(w); }
  void listModeCB( fltk::Widget* w );

  inline static void _filenameCB( fltk::Widget *w, void *arg )
    { ((Flu_File_Chooser*)arg)->filenameCB(); }
  void filenameCB();

  inline static void _locationCB( fltk::Widget *w, void *arg )
  { 
    Flu_Combo_Tree* c = (Flu_Combo_Tree*)w;
    ((Flu_File_Chooser*)arg)->locationCB( c->text() ); 
  }
  void locationCB( const char *path );

  inline static void _locationQJCB( fltk::Widget *w, void *arg )
    { ((Flu_File_Chooser*)arg)->cd( ((fltk::Button*)w)->label() ); }

  inline static void delayedCdCB( void *arg )
    { ((Flu_File_Chooser*)arg)->cd( ((Flu_File_Chooser*)arg)->delayedCd.c_str() ); }

  inline static void selectCB( void *arg )
    { ((Flu_File_Chooser*)arg)->okCB(); }

  inline static void _cancelCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->cancelCB(); }
  void cancelCB();

  inline static void _okCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->okCB(); }
  void okCB();

  inline static void _trashCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->trashCB(); }
  void trashCB( bool recycle = true );

  inline static void _newFolderCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->newFolderCB(); }
  void newFolderCB();

  void saveFavorites();

  inline static void upDirCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->cd( "../" ); }

  inline static void reloadCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->reloadCB(); }
  void reloadCB();

  inline static void _homeCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->homeCB(); }
  void homeCB();

  inline static void _desktopCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->desktopCB(); }
  void desktopCB();

  inline static void _favoritesCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->favoritesCB(); }
  void favoritesCB();

  inline static void _myComputerCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->myComputerCB(); }
  void myComputerCB();

  inline static void _addToFavoritesCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->addToFavoritesCB(); }
  void addToFavoritesCB();

  inline static void _documentsCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->documentsCB(); }
  void documentsCB();

  inline static void _hideCB( fltk::Widget*, void *arg )
    { ((Flu_File_Chooser*)arg)->hideCB(); }
  void hideCB();
  void do_callback();

  enum {
    SORT_NAME = 1,
    SORT_TYPE = 2,
    SORT_SIZE = 4,
    SORT_DATE = 8,
    SORT_REVERSE = 16 
  };
  static void _qSort( int how, bool caseSort, fltk::Group* const g, 
		      int low, int high );


  friend class Entry;
  class Entry : public fltk::Input
    {
    public:
      Entry( const char* name, int t, bool d, Flu_File_Chooser *c );
      ~Entry();

      int handle( int event );

      void draw();

      static void loadRealIcon( Entry* entry);

      void updateSize();
      void updateIcon();

      // We should really use fltk's flags for this, but this
      // causes problems with fltk::Browser
      void set_selected()   { 
	 selected_ = true; 
	 color( fltk::DARK_BLUE );
	 textcolor( fltk::WHITE );
	 redraw();
      }
        void set_colors();
        bool selected()       { return selected_;  }
        void clear_selected();


      std::string filename, date, filesize, shortname, 
	description, shortDescription, toolTip, altname;
      std::string permissions;
      unsigned char pU, pG, pO; // 3-bit unix style permissions
      unsigned int type;
      time_t idate;
      unsigned long isize;
      int editMode;
      Flu_File_Chooser *chooser;
      fltk::Image *icon;

        int nameW, typeW, sizeW, dateW, permW;
        bool details;
        bool selected_;

      inline static void _inputCB( fltk::Widget *w, void *arg )
	{ ((Entry*)arg)->inputCB(); }
      void inputCB();

      inline static void _editCB( void *arg )
	{ ((Entry*)arg)->editCB(); }
      void editCB();
    };

  friend class FileList;

   class FileList : public Flu_Wrap_Group
    {
    public:
      FileList( int x, int y, int w, int h, Flu_File_Chooser *c );
      ~FileList();

      int handle( int event );
      void sort( int numDirs = -1 );


      int numDirs;
      Flu_File_Chooser* chooser;
    };

  friend class FileDetails;
  class FileDetails : public fltk::Browser
    {
    public:
      FileDetails( int x, int y, int w, int h, Flu_File_Chooser *c );
      ~FileDetails();

      int handle( int event );
      void sort( int numDirs = -1 );

      virtual void layout();

      void scroll_to( fltk::Widget *w );
      fltk::Widget* next( fltk::Widget* w );
      fltk::Widget* previous( fltk::Widget* w );

      int numDirs;
      Flu_File_Chooser *chooser;
    };



  friend class PreviewTile;
  class PreviewTile : public fltk::TiledGroup
    {
    public:
      PreviewTile( int x, int y, int w, int h, Flu_File_Chooser *c );
      Flu_File_Chooser *chooser;
    };



  fltk::Group *getEntryGroup();
  fltk::Group *getEntryContainer();

  void win2unix( std::string &s );

  void cleanupPath( std::string &s );

  bool correctPath( std::string &path );

  void updateEntrySizes();

  void buildLocationCombo();

  void updateLocationQJ();

  void addToHistory();

  std::string formatDate( const char *d );

  void recursiveScan( const char *dir, FluStringVector *files );

  bool stripPatterns( std::string s, FluStringVector* patterns );

  int popupContextMenu( Entry *entry );

  std::string commonStr();

  static int (*customSort)(const char*,const char*);

  PreviewTile *previewTile;
  fltk::Group *fileGroup, *locationQuickJump;
  fltk::Image *defaultFileIcon;
  Entry *lastSelected;
  FileList *filelist;
  fltk::Group *fileDetailsGroup;
  FileDetails *filedetails;
  fltk::Button *detailNameBtn, *detailTypeBtn, *detailSizeBtn, *detailDateBtn;
  std::string currentDir, delayedCd, rawPattern;
  std::string configFilename;
  std::string userHome, userDesktop, userDocs;
  std::string drives[26];
  fltk::xpmImage* driveIcons[26];
  fltk::ToggleButton *fileListBtn, *fileListWideBtn, *fileDetailsBtn;
  fltk::ToggleButton* previewBtn;
  fltk::HighlightButton* backBtn, *forwardBtn, *upDirBtn, *reloadBtn, *newDirBtn, *trashBtn, *addFavoriteBtn;
  fltk::InputBrowser *favoritesList;
  Flu_Combo_Tree *filePattern;
  int selectionType;
  bool filenameEnterCallback, filenameTabCallback, walkingHistory; 
  bool caseSort, fileEditing;
  int sortMethod;
  int dragX, dragY;

  FluStringVector patterns;

  static FileTypeInfo *types;
  static int numTypes;
  static int typeArraySize;

  enum ListModes
    {
      kNone,
      kWide,
      kDetails
    };

  ListModes listMode;

#ifdef WIN32
  unsigned int driveMask;
  unsigned int driveTypes[26];
  std::string volumeNames[26];
  bool refreshDrives;
#endif

  class History
  {
  public:
    History() { last = next = NULL; }
    std::string path;
    History *last, *next;
  };

  History *history, *currentHist;

  fltk::Callback *_callback;
  void *_userdata;

};

#endif
