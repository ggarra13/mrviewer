/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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
 */

//#define ALLOC_CONSOLE


#include <string.h>
#include <iostream>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef _WIN32
#include <winsock2.h>
#endif


#include <FL/fl_ask.H>
#include <FL/Fl.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/platform.H>  // for fl_open_callback (OSX)

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
#include "core/mrvCPU.h"

#include "gui/mrvLanguages.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvVersion.h"
#include "gui/mrvIO.h"
#include "gui/mrvMainWindow.h"

#include "standalone/mrvCommandLine.h"
#include "standalone/mrvRoot.h"

#include <libintl.h>


#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif


using namespace std;

namespace {
const char* const kModule = "main";
}

ViewerUI* ui = NULL;

void load_files( mrv::LoadList& files,
                 bool stereo = false,
                 std::string bgimage = "",
                 bool edl = false  )
{
    if ( files.empty() ) return;

   //
   // Window must be shown after images have been loaded.
   //
   mrv::ImageBrowser* image_list = ui->uiReelWindow->uiBrowser;
   image_list->load( files, stereo, bgimage, edl );
}

void load_new_files( void* s )
{

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
      double fps;

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
         g.get( "fps", fps, 24 );

         boost::int64_t first = strtoll( firstS, NULL, 10 );
         boost::int64_t last = strtoll( lastS, NULL, 10 );
         boost::int64_t start = strtoll( startS, NULL, 10 );
         boost::int64_t end = strtoll( endS, NULL, 10 );

         mrv::LoadInfo info( filename, first, last, start, end, fps, audio );
         files.push_back( info );
      }
   }

   if ( !files.empty() )
   {
       load_files( files );
       files.clear();

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

   Fl::repeat_timeout( 1.0, load_new_files );
}



stringArray OSXfiles;
void osx_open_cb(const char *fname)
{
    OSXfiles.push_back( fname );
}

int main( int argc, const char** argv )
{


    for ( int i = 0; i < argc; ++i )
    {
        if ( strcmp( argv[i], "-d" ) == 0 ||
             strcmp( argv[i], "--debug") == 0 )
        {
            if ( i+1 < argc )
                mrv::Preferences::debug = atoi( argv[i+1] );
            else
                mrv::Preferences::debug = 1;
        }
    }

#ifdef _WIN32
    if ( mrv::Preferences::debug > 0 )
    {
        AttachConsole( -1 );
        DWORD err = GetLastError();
        if ( err == ERROR_INVALID_HANDLE ||
             err == ERROR_INVALID_PARAMETER )
        {
            AllocConsole();
        }
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
    }
#endif

#ifdef LINUX
  Fl_File_Icon::load_system_icons();
#endif
    // Avoid repetition in ffmpeg's logs
    av_log_set_flags(AV_LOG_SKIP_REPEATED);

#if defined __APPLE__ && defined __MACH__
    setenv( "LC_CTYPE",  "UTF-8", 1 );
#endif

    int lang = -1;
    const char* code = "C";
    {
        Fl_Preferences base( mrv::prefspath().c_str(), "filmaura",
                             "mrViewer" );

        // Load ui language preferences
        Fl_Preferences ui( base, "ui" );

        ui.get( "language", lang, -1 );
        if ( lang >= 0 )
        {
            for ( unsigned i = 0;
                  i < sizeof(kLanguages) / sizeof(LanguageTable); ++i)
            {
                if ( kLanguages[i].index == lang )
                {
                    code = kLanguages[i].code;
                    break;
                }
            }
#ifdef _WIN32
            setenv( "LC_CTYPE",  "UTF-8", 1 );
            if ( setenv( "LANGUAGE", code, 1 ) < 0 )
                LOG_ERROR( "Setting LANGUAGE failed" );
            setlocale( LC_ALL, "" );
            setlocale( LC_ALL, code );
            libintl_setlocale( LC_ALL, "" );
            libintl_setlocale( LC_ALL, code );
            libintl_setlocale( LC_MESSAGES, code );
#else
            setenv( "LANGUAGE", code, 1 );
            setlocale( LC_ALL, "" );
            setlocale(LC_ALL, code);
#ifdef OSX
            setenv( "LC_NUMERIC", code, 1 );
            setenv( "LC_MESSAGES", code, 1 );
#endif
#endif
        }
    }


    const char* tmp;
    if ( lang < 0 )
        tmp = setlocale(LC_ALL, "");
    else
    {
        tmp = setlocale(LC_ALL, NULL);
    }


#if defined __APPLE__ && defined __MACH__
    tmp = setlocale( LC_MESSAGES, NULL );
#endif

    const char* language = getenv( "LANGUAGE" );
    if ( !language || language[0] == '\0' ) language = getenv( "LC_ALL" );
    if ( !language || language[0] == '\0' ) language = getenv( "LC_NUMERIC" );
    if ( !language || language[0] == '\0' ) language = getenv( "LANG" );
    if ( language )
    {
        if (  strcmp( language, "C" ) == 0 ||
             strncmp( language, "ar", 2 ) == 0 ||
             strncmp( language, "en", 2 ) == 0 ||
             strncmp( language, "ja", 2 ) == 0 ||
             strncmp( language, "ko", 2 ) == 0 ||
             strncmp( language, "zh", 2 ) == 0 )
            tmp = "C";
    }

    setlocale( LC_NUMERIC, tmp );


    // Create and install global locale
    try {
        // std::locale::global( std::locale(language) );
        // Make boost.filesystem use it
        fs::path::imbue(std::locale());
    }
    catch( const std::runtime_error& e )
    {
        std::cerr << e.what() << std::endl;
    }




    DBG;
    char buf[1024];




    DBG;
    // Try to set MRV_ROOT if not set already
    mrv::set_root_path( argc, argv );

    std::string path = fl_getenv("MRV_ROOT");
    path += "/share/locale";

    sprintf( buf, "mrViewer%s", mrv::version() );
    bindtextdomain(buf, path.c_str() );
    bind_textdomain_codeset(buf, "UTF-8" );
    textdomain(buf);
    LOG_INFO( _("Translations: ") << path );

#ifdef OSX
    Fl_Mac_App_Menu::about = _("About mrViewer");
    Fl_Mac_App_Menu::print = "";
    Fl_Mac_App_Menu::hide = _("Hide mrViewer");
    Fl_Mac_App_Menu::hide_others = _("Hide Others");
    Fl_Mac_App_Menu::services = _("Services");
    Fl_Mac_App_Menu::quit = _("Quit mrViewer");
#endif

    int ok;

    DBG;
    Fl::scheme("gtk+");
    fl_open_display();
    Fl::option( Fl::OPTION_VISIBLE_FOCUS, false );

    // Adjust ui based on preferences
    for (;;) {
    DBG;

      std::string lockfile;

      // For macOS, to read command-line arguments
      fl_open_callback( osx_open_cb );


      try {
          mrv::Options opts;
          if ( argc > 0 )
            {
              DBG;
              mrv::parse_command_line( argc, argv, opts );
            }
          argc = 0;

          DBG;

          if ( !ui ) ui = new ViewerUI;
          DBG;

          // Make the main view window start with focus
          ui->uiView->take_focus();
          DBG;

          ui->uiMain->show();
          DBG;
          
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

          if ( !OSXfiles.empty() )
          {
              stringArray::iterator it = OSXfiles.begin();
              stringArray::iterator et = OSXfiles.end();

              for ( ; it != et; ++it )
              {
                  int64_t start = AV_NOPTS_VALUE, end = AV_NOPTS_VALUE;
                  std::string fileroot;

                  mrv::fileroot( fileroot, *it, true, false );
                  if ( ui->uiPrefs->uiPrefsLoadSequenceOnAssoc->value() &&
                       mrv::is_valid_sequence( fileroot.c_str() ) &&
                       !opts.single )
                  {
                      mrv::get_sequence_limits( start, end, fileroot );
                      opts.files.push_back( mrv::LoadInfo( fileroot, start, end,
                                                           start, end ) );
                  }
                  else
                  {
                      opts.files.push_back( mrv::LoadInfo( *it ) );
                  }
              }
              load_files( opts.files );
              opts.files.clear();
          }


          lockfile = mrv::lockfile();

            bool single_instance = false;

            if ( !opts.run )
                single_instance = ui->uiPrefs->uiPrefsSingleInstance->value();
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
                LOG_INFO( _("Lockfile ") << lockfile << ". ");
                LOG_INFO( _("(Remove if mrViewer does not start)") );
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


              }
              base.flush();

              if ( idx == 0 )
              {
                  mrvALERT( _("Another instance of mrViewer is open.\n"
                              "Remove ") << lockfile
                            << _(" if mrViewer crashed\n"
                                 "or modify Preferences->User Interface->Single Instance.") );
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
          load_files( opts.files, false, opts.bgfile, opts.edl );
          opts.files.clear();

          if ( !opts.stereo_input.empty() )
          {
              int idx = 0;
              if ( opts.stereo_input == _("Separate layers") )
                  idx = 0;
              else if ( opts.stereo_input == _("Top/bottom") )
                  idx = 1;
              else if ( opts.stereo_input == _("Left/right") )
                  idx = 2;
              else
              {
                  LOG_ERROR( "Stereo Input is invalid.  Valid values are: " );
                  LOG_ERROR( _("Separate layers") << ", " << _("Top/bottom")
                             << ", " << _("Left/right") );
                  exit(-1);
              }
              ui->uiStereo->uiStereoInput->value( idx );
              ui->uiStereo->uiStereoInput->do_callback();
          }

          int idx = 0;
          if ( !opts.stereo_output.empty() )
          {
              if ( opts.stereo_output == _("Left view") )
                  idx = 1;
              else if ( opts.stereo_output == _("Right view") )
                  idx = 2;
              else if ( opts.stereo_output == _("Stereo OpenGL") )
                  idx = 3;
              else if ( opts.stereo_output == _("Top/bottom") )
                  idx = 4;
              else if ( opts.stereo_output == _("Bottom/top") )
                  idx = 5;
              else if ( opts.stereo_output == _("Left/right") )
                  idx = 6;
              else if ( opts.stereo_output == _("Right/left") )
                  idx = 7;
              else if ( opts.stereo_output == _("Even/odd rows") )
                  idx = 8;
              else if ( opts.stereo_output == _("Even/odd columns") )
                  idx = 9;
              else if ( opts.stereo_output == _("Checkerboard pattern") )
                  idx = 10;
              else if ( opts.stereo_output == _("Red/cyan glasses") )
                  idx = 11;
              else if ( opts.stereo_output == _("Cyan/red glasses") )
                  idx = 12;
              else
              {
                  LOG_ERROR( _("Stereo Output is invalid.  Valid values are: " ) );
                  LOG_ERROR( _("Left view") << ", " << _("Right view") << ", " );
                  LOG_ERROR( _("Top/bottom") << ", " << _("Bottom/top") << ", " );
                  LOG_ERROR( _("Left/right") << ", " << _("Right/left") << ", " );
                  LOG_ERROR( _("Even/odd rows") << ", "
                             << _("Even/odd columns") << ", " );
                  LOG_ERROR( _( "Checkerboard pattern") << ", "
                             << _("Red/cyan glasses") << ", " );
                  LOG_ERROR( _("Cyan/red glasses") );
                  exit( -1 );
              }
          }


            if ( opts.stereo.size() > 1 )
            {
                load_files( opts.stereo, true );
                opts.stereo.clear();
            }

            if ( idx )
            {
                ui->uiStereo->uiStereoOutput->value( idx );
                ui->uiStereo->uiStereoOutput->do_callback();
                ui->uiView->fit_image();
            }


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
              Fl::add_timeout( 1.0, load_new_files );

          // If no image is found, resize the window to a suitable level
          mrv::media fg = ui->uiView->foreground();
          if ( !fg )
          {
              ui->uiView->resize_main_window();
          }

          // Start playback if command line forced us to do so
          if ( opts.play )
          {
              bool b = ui->uiView->network_active();
              ui->uiView->network_active(true);
              ui->uiView->play_forwards();
              ui->uiView->network_active(b);
          }

          ok = Fl::run();

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


    int rc = main( __argc, (const char**) __argv );

    fclose(stdout);
    fclose(stderr);

    return rc;
}

#endif
