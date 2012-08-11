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

#include <fltk/ask.h>
#include <fltk/run.h>


#include "core/mrvI8N.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"
#include "mrViewer.h"
#include "gui/mrvMainWindow.h"
#include "mrvColorProfile.h"
#include "mrvException.h"
#include "mrvLicensing.h"
#include "standalone/mrvCommandLine.h"
#include "standalone/mrvRoot.h"
#include "core/mrvCPU.h"


using namespace std;

int main( const int argc, char** argv ) 
{
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


  mrv::LoadList files;
  mrv::parse_command_line( argc, argv, ui, files );

  // mrv::open_license( argv[0] );
  // mrv::checkout_license();

  mrv::LoadList::iterator i = files.begin();
  mrv::LoadList::iterator e = files.end();

  //
  // Window must be shown after images have been loaded.
  // 
  mrv::ImageBrowser* image_list = ui->uiReelWindow->uiBrowser;
  image_list->load( files );



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


  // mrv::checkin_license();
  // mrv::close_license();

  return ok;
}


#if defined(WIN32) || defined(WIN64)

#ifdef BORLAND5
# define __argc _argc
# define __argv _argv
#endif

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
