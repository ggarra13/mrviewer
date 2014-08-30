/**
 * @file   main.cpp
 * @author gga
 * @date   Wed Jul  4 23:16:07 2007
 * 
 * @brief  Main entry point for mrViewer executable
 * 
 * 
 */



#include <iostream>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef LINUX
#include <X11/Xlib.h>
#endif

#include <fltk/ask.h>
#include <fltk/run.h>
#include <fltk/Preferences.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "core/mrvHome.h"
#include "core/mrvServer.h"
#include "core/mrvClient.h"
#include "core/mrvI8N.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvIO.h"
#include "mrViewer.h"
#include "gui/mrvMainWindow.h"
#include "mrvColorProfile.h"
#include "mrvException.h"
#include "mrvLicensing.h"
#include "standalone/mrvCommandLine.h"
#include "standalone/mrvRoot.h"
#include "core/mrvCPU.h"


using namespace std;

namespace {
const char* const kModule = "main"; 
}


void load_files( mrv::LoadList& files, 
                 mrv::ViewerUI* ui,
                 bool stereo = false )
{
   //
   // Window must be shown after images have been loaded.
   // 
   mrv::ImageBrowser* image_list = ui->uiReelWindow->uiBrowser;
   image_list->load( files, stereo );
}

void load_new_files( void* s )
{
   mrv::ViewerUI* ui = (mrv::ViewerUI*) s;

   mrv::LoadList files;

   {
       fltk::Preferences lock( mrv::prefspath().c_str(), "filmaura",
                               "mrViewer.lock" );
      int pid = 1;
      lock.get( "pid", pid, 1 );
      

      
      char* filename;
      char* audio;
      int start = 1, end = 50;
      
      
      int groups = lock.groups();
      
      for ( int i = 0; i < groups; ++i )
      {
	 const char* group = lock.group( i );
	 fltk::Preferences g( lock, group );
	 g.get( "filename", filename, "" );
	 g.get( "audio", audio, "" );
	 g.get( "start", start, 1 );
	 g.get( "end", end, 50 );
	 
	 mrv::LoadInfo info( filename, start, end, audio );
	 files.push_back( info );
      }
   }

   if ( !files.empty() )
   {
       load_files( files, ui );

       std::string lockfile = mrv::lockfile();
       
       if(fs::exists(lockfile))
       {
           if ( ! fs::remove( lockfile ) )
               LOG_ERROR( "Could not remove lock file" );
       }

       fltk::Preferences base( mrv::prefspath().c_str(), "filmaura",
                               "mrViewer.lock" );
       base.set( "pid", 1 );
   }

   fltk::repeat_timeout( 0.5, load_new_files, ui ); 
}


int main( const int argc, char** argv ) 
{
#ifdef LINUX
  XInitThreads();
#endif
  fltk::lock();   // Initialize X11 thread system

  // Try to set MRV_ROOT if not set already
  mrv::set_root_path( argc, argv );

#ifdef USE_GETTEXT

  setlocale (LC_MESSAGES, "");

  std::string tmp = getenv("MRV_ROOT");
  tmp += "/locale";

  bindtextdomain("mrViewer", tmp.c_str());
  textdomain("mrViewer");

#endif

  // Adjust ui based on preferences
  mrv::ViewerUI* ui = new mrv::ViewerUI();



  mrv::Options opts;

  mrv::parse_command_line( argc, argv, ui, opts );


  std::string lockfile = mrv::lockfile();

  bool single_instance = ui->uiPrefs->uiPrefsSingleInstance->value();
  if ( opts.port != 0 ) {
     ui->uiPrefs->uiPrefsSingleInstance->value(0);
     single_instance = false;
     if ( fs::exists( lockfile ) )
     {
	if ( ! fs::remove( lockfile ) )
	  LOG_ERROR("Could not remove lock file");
     }
  }

  LOG_INFO( "lockfile " << lockfile );


  if ( fs::exists( lockfile ) && single_instance )
  {
     {
         fltk::Preferences base( mrv::prefspath().c_str(), "filmaura",
				"mrViewer.lock" );
	
	mrv::LoadList::iterator i = opts.files.begin();
	mrv::LoadList::iterator e = opts.files.end();
	for ( int idx = 0; i != e ; ++i, ++idx )
	{
	   char buf[256];
	   sprintf( buf, "file%d", idx );
	
	   fltk::Preferences ui( base, buf );
	   ui.set( "filename", (*i).filename.c_str() );
	   ui.set( "audio", (*i).audio.c_str() );
	   ui.set( "start", (int)(*i).start );
	   ui.set( "end", (int)(*i).end );
	   ui.flush();
	}
	base.flush();
     }
     
     // mrvALERT( "Another instance of mrViewer open.\n"
     //           "Remove " << lockfile << "\n"
     //           "or modify Preferences->User Interface->Single Instance.");
     // fltk::check();

     exit(0);
  }

  {
      fltk::Preferences lock( mrv::prefspath().c_str(), "filmaura",
                              "mrViewer.lock" );
     lock.set( "pid", 1 );
     
  }

  // mrv::open_license( argv[0] );
  // mrv::checkout_license();

  load_files( opts.files, ui );

  if ( opts.edl )
  {
     ui->uiReelWindow->uiBrowser->current_reel()->edl = true;
     ui->uiTimeline->edl( true );
  }

  if (opts.fps > 0 )
  {
     ui->uiView->fps( opts.fps );
  }

  if (opts.host == "" && opts.port != 0)
  {

     mrv::ServerData* data = new mrv::ServerData;
     data->ui = ui;
     data->port = opts.port;
     boost::thread( boost::bind( mrv::server_thread, 
     				 data ) );
  }
  else if ( opts.host != "" && opts.port != 0 )
  {
     mrv::ServerData* data = new mrv::ServerData;
     data->ui = ui;
     data->host = opts.host;
     data->port = opts.port;
     char buf[128];
     sprintf( buf, "%d", opts.port );
     data->group = buf;

     boost::thread( boost::bind( mrv::client_thread, 
				 data ) );
  }

  fltk::add_timeout( 0.5, load_new_files, ui );


  int ok;

  try {
    ok = fltk::run();
  }
  catch( const std::exception& e )
    {
      std::cerr << e.what() << std::endl;
      ok = -1;
    }
  catch( const char* e )
    {
      std::cerr << e << std::endl;
      ok = -1;
    }
  catch( ... )
    {
      std::cerr << "Unhandled exception" << std::endl;
      ok = -1;
    }

  if(fs::exists(lockfile))
  {
      try {
	 if ( ! fs::remove( lockfile ) )
	    LOG_ERROR( "Could not remove lock file!" );
      }
      catch( fs::filesystem_error& e )
      {
	 LOG_ERROR( "Filesystem error: " << e.what() );
      }
  }

  // mrv::checkin_license();
  // mrv::close_license();

  return ok;
}


#if defined(WIN32) || defined(WIN64)

#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
		     LPSTR lpCmdLine, int nCmdShow )
{
   
   // AllocConsole();
   // freopen("conin$", "r", stdin);
   // freopen("conout$", "w", stdout);
   // freopen("conout$", "w", stderr);
   
   int rc = main( __argc, __argv );
   
   // fclose(stdin);
   // fclose(stdout);
   // fclose(stderr);

  return rc; 
}
#endif
