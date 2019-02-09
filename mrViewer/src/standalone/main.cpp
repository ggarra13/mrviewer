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
 * @file   main.cpp
 * @author gga
 * @date   Wed Jul  4 23:16:07 2007
 *
 * @brief  Main entry point for mrViewer executable
 *
 *
 */

#ifdef _WIN32
// #define ALLOC_CONSOLE  // ALLOC a Console for debugging stderr/stdout
#endif

#include <string.h>
#include <locale.h>
#include <iostream>

#include <inttypes.h>

#ifdef LINUX
#include <X11/Xlib.h>
#endif

#include <fltk/ask.h>
#include <fltk/run.h>
#include <fltk/Preferences.h>

#include <MagickWand/MagickWand.h>
#include <OpenEXR/ImfHeader.h>

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

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

#ifdef _WIN32

#include <windows.h>

void release_console()
{
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
    Sleep(100000);
}

#endif


using namespace std;

namespace {
const char* const kModule = "main";
}


void load_files( mrv::LoadList& files,
                 mrv::ViewerUI* ui,
                 bool stereo = false,
                 std::string bgimage = "",
		 bool edl = false )
{
    //
    // Window must be shown after images have been loaded.
    //
    mrv::ImageBrowser* image_list = ui->uiReelWindow->uiBrowser;
    image_list->load( files, stereo, bgimage, edl );
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
        char* firstS;
        char* lastS;
        char* startS;
        char* endS;

        int groups = lock.groups();

        for ( int i = 0; i < groups; ++i )
        {
            const char* group = lock.group( i );
            fltk::Preferences g( lock, group );
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

        fltk::Preferences base( mrv::prefspath().c_str(), "filmaura",
                                "mrViewer.lock" );
        base.set( "pid", 1 );
    }

    fltk::repeat_timeout( 1.0, load_new_files, ui );
}

int main( int argc, char** argv )
{
#ifdef LINUX
    XInitThreads();
#endif
    fltk::lock();  // This calls XInitThreads on Linux


    setbuf( stderr, NULL );
    
    // Avoid repetition in ffmpeg's logs
    av_log_set_flags(AV_LOG_SKIP_REPEATED);

    const char* tmp = setlocale(LC_ALL, "");
    // Create and install global locale
    try {

        //std::locale::global(boost::locale::generator().generate(""));
        const char* env = getenv("LC_ALL");
        if ( !env )
            std::locale::global( std::locale("") );
        else
            std::locale::global( std::locale(env) );
        // Make boost.filesystem use it
        fs::path::imbue(std::locale());
    }
    catch( const std::runtime_error& e )
    {
        std::cerr << e.what() << std::endl;
    }


    if ( !tmp )  tmp = setlocale( LC_ALL, NULL );
// #undef setlocale
//   if ( tmp )
//   {
//       loc = strdup( tmp );
//       setlocale( LC_ALL, loc );
//   }


    char buf[1024];
    sprintf( buf, "mrViewer%s", mrv::version() );

    std::string locale = argv[0];
    fs::path file = fs::path( locale );

    int ok = -1;

    file = fs::absolute( file );

    fs::path dir = file.parent_path().branch_path();
    std::string path = fs::canonical( dir ).string();

    path += "/share/locale";

    DBG( "bindtextdomain" );
    const char* bind = bindtextdomain(buf, path.c_str() );
    DBG( "textdomain" );
    const char* domain = textdomain(buf);


// Try to set MRV_ROOT if not set already
    DBG( "set MRV_ROOT" );
    mrv::set_root_path( argc, argv );



    // Adjust ui based on preferences
    for (;;) {

        mrv::ViewerUI* ui = NULL;
        std::string lockfile;


        try {

            DBG("instantiate mrv::ViewerUI" );
            ui = new mrv::ViewerUI();
            DBG("mrv::ViewerUI is now " << ui );

            mrv::Options opts;
            bool ok = true;
            if ( argc > 0 )
                ok = mrv::parse_command_line( argc, argv, ui, opts );


            if (!ok) throw std::runtime_error("Could not parse commandline");


            argc = 0;


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
                LOG_INFO( "lockfile " << lockfile << ". ");
                LOG_INFO( "(Remove if mrViewer does not start)" );
            }

            if ( fs::exists( lockfile ) && single_instance )
            {
                int idx;
                {
                    fltk::Preferences base( mrv::prefspath().c_str(), "filmaura",
                                            "mrViewer.lock" );

                    mrv::LoadList::iterator i = opts.files.begin();
                    mrv::LoadList::iterator e = opts.files.end();
                    for ( idx = 0; i != e ; ++i, ++idx )
                    {
                        char buf[256];
                        sprintf( buf, "file%d", idx );

                        fltk::Preferences ui( base, buf );
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
                    fltk::check();
                }

                exit(0);
            }

            {
                fltk::Preferences lock( mrv::prefspath().c_str(), "filmaura",
                                        "mrViewer.lock" );
                lock.set( "pid", 1 );

            }

            // Imf::staticInitialize();
            MagickWandGenesis();

            // mrv::open_license( argv[0] );
            // mrv::checkout_license();

	    
            load_files( opts.files, ui, false, opts.bgfile, opts.edl );
	    
            if ( opts.edl )
            {
		mrv::Reel r = ui->uiReelWindow->uiBrowser->current_reel();
		r->edl = true;
                ui->uiTimeline->edl( true );
            }

            if ( opts.stereo_input != "" )
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
            if ( opts.stereo_output != "" )
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
                load_files( opts.stereo, ui, true );
            }

	    if ( idx )
	    {
                ui->uiStereo->uiStereoOutput->value( idx );
                ui->uiStereo->uiStereoOutput->do_callback();
                ui->uiView->fit_image();
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

            if ( single_instance )
                fltk::add_timeout( 1.0, load_new_files, ui );

            DBG( "show uiMain" );
            ui->uiMain->show();   // so run() does something
            DBG( "shown uiMain" );

            // Start playback if command line forced us to do so
            if ( opts.play )
            {
                ui->uiView->play_forwards();
            }
            DBG( "fltk::run" );
            ok = fltk::run();
            DBG( "fltk::run returned" );

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
    // Imf::staticUninitialize();

#ifdef _WIN32
    mrv::io::logbuffer* log =
        static_cast<mrv::io::logbuffer*>( mrv::io::info.rdbuf() );
    if ( log->_debug )
        release_console();
#endif

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
    Sleep(100000);
#endif

    return rc;
}

#endif
