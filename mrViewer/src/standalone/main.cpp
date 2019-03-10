
/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂ±o

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
 * @file   main.cpp
 * @author gga
 * @date   Wed Jul  4 23:16:07 2007
 * 
 * @brief  Main entry point for mrViewer executable
 * 
 * 
 */

// #define ALLOC_CONSOLE

#include <string.h>
#include <locale.h>
#include <iostream>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <FL/fl_ask.H>
#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>

#include <MagickWand/MagickWand.h>

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "mrvPreferencesUI.h"
#include "mrvReelUI.h"
#include "mrViewer.h"
#include "core/mrvHome.h"
#include "core/mrvServer.h"
#include "core/mrvClient.h"
#include "core/mrvI8N.h"
#include "core/mrvException.h"
#include "core/mrvColorProfile.h"
#include "core/mrvCPU.h"

#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvVersion.h"
#include "gui/mrvIO.h"
#include "gui/mrvMainWindow.h"

#include "standalone/mrvCommandLine.h"
#include "standalone/mrvRoot.h"

#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif


using namespace std;

namespace {
const char* const kModule = "main"; 
}


void load_files( mrv::LoadList& files, 
                 ViewerUI* ui,
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
    ViewerUI* ui = (ViewerUI*) s;

   mrv::LoadList files;

   {
      Fl_Preferences lock( mrv::prefspath().c_str(), "filmaura",
                               "mrViewer.lock" );
      int pid = 1;
      lock.get( "pid", pid, 1 );
      

      
      char* filename;
      char* audio;
      char* firstS;
      char* lastS;
      char* startS;
      char* endS;
      
      int groups = lock.groups();
      
      for ( int i = 0; i < groups; ++i )
      {
	 const char* group = lock.group( i );
	 Fl_Preferences g( lock, group );
	 g.get( "filename", filename, "" );
	 g.get( "audio", audio, "" );
	 g.get( "first", firstS, "1" );
	 g.get( "last", lastS, "50" );
	 g.get( "start", startS, "1" );
	 g.get( "end", endS, "50" );

         boost::int64_t first = strtoll( firstS, NULL, 10 );
         boost::int64_t last = strtoll( lastS, NULL, 10 );
         boost::int64_t start = strtoll( startS, NULL, 10 );
         boost::int64_t end = strtoll( endS, NULL, 10 );
	 
	 mrv::LoadInfo info( filename, first, last, start, end, audio );
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

       Fl_Preferences base( mrv::prefspath().c_str(), "filmaura",
                            "mrViewer.lock" );
       base.set( "pid", 1 );
   }

   Fl::repeat_timeout( 1.0, load_new_files, ui ); 
}

int main( int argc, const char** argv ) 
{
  Fl::scheme(NULL);
  Fl::get_system_colors();
  
    // Avoid repetition in ffmpeg's logs
    av_log_set_flags(AV_LOG_SKIP_REPEATED);

    char* loc = _("unknown");

    const char* tmp = setlocale(LC_ALL, N_(""));

  
  // Create and install global locale
  std::locale::global(boost::locale::generator().generate( N_("") ));
  // Make boost.filesystem use it
  boost::filesystem::path::imbue(std::locale());
 
  if ( !tmp )  tmp = setlocale( LC_ALL, NULL );

  if ( tmp )
  {
      loc = strdup( tmp );
  }
  

  char buf[1024];
  sprintf( buf, "mrViewer%s", mrv::version() );

  std::string locale = argv[0];
  fs::path file = fs::path( locale );

  int ok = -1;

  file = fs::absolute( file );

  fs::path dir = file.parent_path().branch_path();
  std::string path = fs::canonical( dir ).string();

  path += "/share/locale";
  bindtextdomain(buf, path.c_str() );
  textdomain(buf);

  if ( loc )
  {
      LOG_INFO( _("Changed locale to ") << loc );
      free(loc);
  }

  // Try to set MRV_ROOT if not set already
  mrv::set_root_path( argc, argv );



  // Adjust ui based on preferences
  for (;;) {

      ViewerUI* ui = NULL;
      std::string lockfile;

      try {

          ui = new ViewerUI();

          mrv::Options opts;
          if ( argc > 0 )
              mrv::parse_command_line( argc, argv, ui, opts );
          argc = 0;


          lockfile = mrv::lockfile();

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

          if ( single_instance )
          {
              LOG_INFO( "lockfile " << lockfile << ". ");
              LOG_INFO( "(Remove if mrViewer does not start)" );
          }

          if ( fs::exists( lockfile ) && single_instance )
          {
              int idx;
              Fl_Preferences base( mrv::prefspath().c_str(), "filmaura",
                                   "mrViewer.lock" );
              mrv::LoadList::iterator i = opts.files.begin();
              mrv::LoadList::iterator e = opts.files.end();
              for ( idx = 0; i != e ; ++i, ++idx )
              {
                  Fl_Preferences base( mrv::prefspath().c_str(), "filmaura",
                                       "mrViewer.lock" );

                  mrv::LoadList::iterator i = opts.files.begin();
                  mrv::LoadList::iterator e = opts.files.end();
                  for ( idx = 0; i != e ; ++i, ++idx )
                  {
                      char buf[256];
                      sprintf( buf, "file%d", idx );
	
                      Fl_Preferences ui( base, buf );
                      ui.set( "filename", (*i).filename.c_str() );
                      ui.set( "audio", (*i).audio.c_str() );

                      sprintf( buf, "%" PRId64, (*i).first );
                      ui.set( "first", buf );

                      sprintf( buf, "%" PRId64, (*i).last );
                      ui.set( "last", buf );

                      sprintf( buf, "%" PRId64, (*i).start );
                      ui.set( "start", buf );

                      sprintf( buf, "%" PRId64, (*i).end );
                      ui.set( "end", buf );

                      ui.flush();
                  }
                  base.flush();
              }
              
          if ( idx == 0 )
          {
              mrvALERT( "Another instance of mrViewer is open.\n"
                        "Remove " << lockfile << " if mrViewer crashed\n"
                        "or modify Preferences->User Interface->Single Instance.");
              Fl::check();
          }

              exit(0);
          }

      {
          Fl_Preferences lock( mrv::prefspath().c_str(), "filmaura",
                                  "mrViewer.lock" );
          lock.set( "pid", 1 );
      }

          MagickWandGenesis();

      // mrv::open_license( argv[0] );
      // mrv::checkout_license();

          load_files( opts.files, ui );
          if ( opts.stereo.size() > 1 )
              load_files( opts.stereo, ui, true );

          if ( opts.edl )
          {
              ui->uiReelWindow->uiBrowser->current_reel()->edl = true;
              ui->uiTimeline->edl( true );
          }

          if (opts.fps > 0 )
          {
              ui->uiView->fps( opts.fps );
          }

      if ( single_instance )
          Fl::add_timeout( 1.0, load_new_files, ui );
      
      if (opts.host.empty() && opts.port != 0)
      {
          mrv::ServerData* data = new mrv::ServerData;
          data->ui = ui;
          data->port = opts.port;
          boost::thread( boost::bind( mrv::server_thread, 
                                      data ) );
      }
      else if ( ! opts.host.empty() && opts.port != 0 )
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

      if ( single_instance )
          Fl::add_timeout( 1.0, load_new_files, ui );
      
    
      ui->uiMain->show();   // so run() does something
      ok = Fl::run();
      }
      catch( const mrv::reinit_exception& e )
      {
          LOG_INFO( _(e.what()) );

          delete ui;
          continue;
      }
      catch( const std::exception& e )
      {
          LOG_ERROR( _( e.what() ) );
          ok = -1;
      }
      catch( const char* e )
      {
          LOG_ERROR( _(e) );
          ok = -1;
      }
      catch( ... )
      {
          LOG_ERROR( _("Unhandled exception") );
          ok = -1;
      }

      if( fs::exists(lockfile) )
      {
          try {
              if ( ! fs::remove( lockfile ) )
                  LOG_ERROR( "Could not remove lock file!" );
          }
          catch( const fs::filesystem_error& e )
          {
              LOG_ERROR( _("Filesystem error: ") << e.what() );
          }
          catch( const boost::exception& e )
          {
              LOG_ERROR( _("Boost exception: ") << boost::diagnostic_information(e) );
          }
          catch( ... )
          {
              LOG_ERROR( _("Unknown error") );
          }
      }
      break;
  }
  MagickWandTerminus();
  

  return ok;
}


#if defined(WIN32) || defined(WIN64)

#include <windows.h>


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
		     LPSTR lpCmdLine, int nCmdShow )
{
   
#ifdef ALLOC_CONSOLE
    AllocConsole();
    freopen("conin$", "r", stdin);
    freopen("conout$", "w", stdout);
    freopen("conout$", "w", stderr);
#endif

    int rc = main( __argc, __argv );
   
#ifdef ALLOC_CONSOLE
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
#endif

    return rc; 
}

#endif
