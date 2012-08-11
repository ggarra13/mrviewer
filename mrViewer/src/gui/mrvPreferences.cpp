/**
 * @file   mrvPreferences.cpp
 * @author gga
 * @date   Sun Jul  1 19:25:26 2007
 * 
 * @brief  
 * 
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <algorithm>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <fltk/Style.h>
#include <fltk/InputBrowser.h>
#include <fltk/Box.h>
#include <fltk/InvisibleBox.h>
#include <fltk/Browser.h>
#include <fltk/Button.h>
#include <fltk/Preferences.h>
#include <fltk/ProgressBar.h>
#include <fltk/Tooltip.h>
#include <fltk/run.h>

// CORE classes
#include "core/mrvAudioEngine.h"
#include "core/mrvException.h"
#include "core/mrvColorProfile.h"
#include "core/mrvOS.h"

// GUI  classes
#include "gui/mrvColorOps.h"
#include "gui/mrvImageView.h"
#include "gui/mrvMainWindow.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvFLTKHandler.h"
#include "gui/FLU/Flu_File_Chooser.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvIO.h"
#include "gui/mrvHotkey.h"

// Widgets
#include "mrvColorAreaUI.h"
#include "mrViewer.h"

extern fltk::NamedStyle* group_style;

namespace 
{
  const char* kModule = "prefs";



  /** 
   * This function allows the user to override a preference setting by
   * using an environment variable.
   * 
   * @param variable        environment variable to look for
   * @param defaultValue    default value to use if variable is not set
   * @param inPrefs         boolean specifying whether the value came from
   *                        saved preferences or not.  It is used to print
   *                        a warning if some setting is as of yet undefined.
   * 
   * @return a float corresponding to the value set in the environment or to
   *         to the default value.
   */
  int environmentSetting( const char* variable,
			  const int   defaultValue,
			  const bool  inPrefs )
  {
    int r = defaultValue;
    const char* env = getenv( variable );
    if ( !env )
      {
	if ( !inPrefs )
	  {
	    LOG_WARNING("Environment variable \"" << variable << "\" "
			"is not set; using default value "
			"(" << defaultValue << ").");
	  }
      }
    else
      {	
	int n = sscanf( env, " %d", &r );
	if (n != 1)
	  {
	    LOG_ERROR( "Cannot parse environment variable \"" << variable
		       << "\" as an integer value; using " 
		       << defaultValue << " instead." );
	  }
      }
    return r;
  }


  /** 
   * This function allows the user to override a preference setting by
   * using an environment variable.
   * 
   * @param variable        environment variable to look for
   * @param defaultValue    default value to use if variable is not set
   * @param inPrefs         boolean specifying whether the value came from
   *                        saved preferences or not.  It is used to print
   *                        a warning if some setting is as of yet undefined.
   * 
   * @return a float corresponding to the value set in the environment or to
   *         to the default value.
   */
  float environmentSetting( const char* variable,
			    const float defaultValue,
			    const bool inPrefs )
  {
    float r = defaultValue;
    const char* env = getenv( variable );
    if ( !env )
      {
	if ( !inPrefs )
	  {
	    LOG_WARNING("Environment variable \"" << variable << "\" "
			"is not set; using default value "
			"(" << defaultValue << ").");
	  }
      }
    else
      {	
	int n = sscanf( env, " %f", &r );
	if (n != 1)
	  {
	    LOG_ERROR( "Cannot parse environment variable \"" << variable
		       << "\" as a float value; using " << defaultValue
		       << " instead.");
	  }
      }
    return r;
  }

  /** 
   * This function allows the user to override a preference setting by
   * using an environment variable.
   * 
   * @param variable        environment variable to look for
   * @param defaultValue    default value to use if variable is not set
   * @param inPrefs         boolean specifying whether the value came from
   *                        saved preferences or not.  It is used to print
   *                        a warning if some setting is as of yet undefined.
   * 
   * @return a string corresponding to the value set in the environment or to
   *         to the default value.
   */
  const char* environmentSetting( const char* variable,
				  const char* defaultValue,
				  const bool inPrefs )
  {
    const char* env = getenv( variable );
    if ( !env )
      {
	env = defaultValue;
	if ( !inPrefs )
	  {
	    LOG_WARNING("Environment variable \"" << variable << "\" "
			"is not set; using default value "
			"(\"" << defaultValue << "\").");
	  }
      }
    return env;
  }


  Imf::Chromaticities
  environmentSetting( const char* variable,
		      const Imf::Chromaticities& defaultValue,
		      const bool inPrefs )
  {
    Imf::Chromaticities tmp = defaultValue;

    if (const char *val = getenv(variable))
    {
      int n = sscanf( val,
		      " red %f %f green %f %f blue %f %f white %f %f",
		      &tmp.red.x, &tmp.red.y,
		      &tmp.green.x, &tmp.green.y,
		      &tmp.blue.x, &tmp.blue.y,
		      &tmp.white.x, &tmp.white.y);

      if (n != 8)
	{
	  tmp = defaultValue;

	  LOG_ERROR("Cannot parse environment variable \"" <<
		    variable << "\"; Format: "
		    "\"red X X green X X blue X X white X X\"" );
	  LOG_ERROR("Using default value "
		    "(chromaticities according to Rec. ITU-R BT.709).");
	}
    }
    else
    {
      if ( !inPrefs )
	LOG_WARNING("Environment variable \"" << variable << "\" is "
		    "not set; using default value (chromaticities according "
		    "to Rec. ITU-R BT.709).");
    }
    return tmp;
  }

  Imf::Chromaticities chromaticities(
				     const char* variable,
				     const Imf::Chromaticities& defaultValue, 
				     fltk::Preferences& chroma
				     )
  {
    Imf::Chromaticities tmpC;
    bool ok  = chroma.get( "red_x", tmpC.red.x, defaultValue.red.x );
    ok &= chroma.get( "red_y",   tmpC.red.y,    defaultValue.red.y );
    ok &= chroma.get( "green_x", tmpC.green.x,  defaultValue.green.x );
    ok &= chroma.get( "green_y", tmpC.green.y,  defaultValue.green.y );
    ok &= chroma.get( "blue_x",  tmpC.blue.x,   defaultValue.blue.x );
    ok &= chroma.get( "blue_y",  tmpC.blue.y,   defaultValue.blue.y );
    ok &= chroma.get( "white_x", tmpC.white.x,  defaultValue.white.x );
    ok &= chroma.get( "white_y", tmpC.white.y,  defaultValue.white.y );
    return environmentSetting( variable, tmpC, ok );
  }
}


namespace mrv {

  AboutUI*          ViewerUI::uiAbout = NULL;
  LogUI*            ViewerUI::uiLog   = NULL;
  PreferencesUI*    ViewerUI::uiPrefs = NULL;
  ICCProfileListUI* ViewerUI::uiICCProfiles = NULL;
  HotkeyUI*         ViewerUI::uiHotkey = NULL;


  std::string         Preferences::ODT_CTL_transform;
  std::string         Preferences::ODT_ICC_profile;
  Imf::Chromaticities Preferences::ODT_CTL_chromaticities;
  float               Preferences::ODT_CTL_white_luminance = 120.0f;
  float               Preferences::ODT_CTL_surround_luminance = 12.0f;

  std::string         Preferences::CTL_8bits_save_transform;
  std::string         Preferences::CTL_16bits_save_transform;
  std::string         Preferences::CTL_32bits_save_transform;
  std::string         Preferences::CTL_float_save_transform;
  std::string         Preferences::root;
  std::string         Preferences::tempDir = "/usr/tmp/";
  Preferences::CacheType Preferences::cache_type = kCacheAsLoaded;

  int   Preferences::bgcolor;
  int   Preferences::textcolor;
  int   Preferences::selectioncolor;


  Preferences::Preferences( mrv::PreferencesUI* uiPrefs )
  {
    bool ok;
    int tmp;
    float tmpF;
    char  tmpS[2048];
    Imf::Chromaticities tmpC, c;

    root = getenv("MRV_ROOT");
    if ( root.empty() )
      {
	EXCEPTION("Environment variable MRV_ROOT not set.  Aborting");
      }

    fltk::Preferences base( fltk::Preferences::USER, "filmaura",
			    "mrViewer" );

    //
    // Get ui preferences
    //
    fltk::Preferences ui( base, "ui" );

    ui.get( "topbar", tmp, 1 );
    uiPrefs->uiPrefsTopbar->value(tmp);

    ui.get( "pixel_toolbar", tmp, 1 );
    uiPrefs->uiPrefsPixelToolbar->value(tmp);

    ui.get( "timeline_toolbar", tmp, 1 );
    uiPrefs->uiPrefsTimeline->value(tmp);

    ui.get( "reel_list", tmp, 0 );
    uiPrefs->uiPrefsReelList->value(tmp);

    ui.get( "image_info", tmp, 0 );
    uiPrefs->uiPrefsImageInfo->value(tmp);

    ui.get( "color_area", tmp, 0 );
    uiPrefs->uiPrefsColorArea->value(tmp);

    ui.get( "histogram", tmp, 0 );
    uiPrefs->uiPrefsHistogram->value(tmp);

    ui.get( "vectorscope", tmp, 0 );
    uiPrefs->uiPrefsVectorscope->value(tmp);

    ui.get( "timeline_display", tmp, 0 );
    uiPrefs->uiPrefsTimelineDisplay->value(tmp);


    //
    // ui/window preferences
    //
    {
       fltk::Preferences win( ui, "window" );
       
       win.get( "always_on_top", tmp, 0 );
       uiPrefs->uiPrefsAlwaysOnTop->value( tmp );
    
       win.get( "open_mode", tmp, 0 ); 
    
       {
	  fltk::RadioButton* r;
	  for ( int i = 0; i < uiPrefs->uiPrefsOpenMode->children(); ++i )
	  {
	     r = (fltk::RadioButton*) uiPrefs->uiPrefsOpenMode->child( i );
	     r->value(0);
	  }
	  r = (fltk::RadioButton*)uiPrefs->uiPrefsOpenMode->child( tmp );
	  r->value(1);
       }
    }



    //
    // ui/view
    //
    fltk::Preferences view( ui, "view" );

    view.get("gain", tmpF, 1.0f );
    uiPrefs->uiPrefsViewGain->value( tmpF );

    view.get("gamma", tmpF, 1.0f );
    uiPrefs->uiPrefsViewGamma->value( tmpF );

    view.get("compensate_pixel_ratio", tmp, 0 );
    uiPrefs->uiPrefsViewPixelRatio->value( tmp );

    view.get("lut", tmp, 0 );
    uiPrefs->uiPrefsViewLut->value( tmp );

    view.get("safe_areas", tmp, 0 );
    uiPrefs->uiPrefsSafeAreas->value( tmp );

    view.get("crop_area", tmp, 0 );
    uiPrefs->uiPrefsCropArea->value( tmp );

    //
    // ui/colors
    //
    fltk::Preferences colors( ui, "colors" );
    colors.get( "background_color", bgcolor, 0x43434300 );
    colors.get( "text_color", textcolor, 0xababab00 );
    colors.get( "selection_color", selectioncolor, 0x97a8a800 );

    //
    // ui/view/colors
    //
    {
      fltk::Preferences colors( view, "colors" );

      colors.get("background_color", tmp, 0x20202000 );
      uiPrefs->uiPrefsViewBG->color( tmp );

      colors.get("text_overlay_color", tmp, 0xFFFF0000 );
      uiPrefs->uiPrefsViewText->color( tmp );

      colors.get("selection_color", tmp, 0x0000FF00 );
      uiPrefs->uiPrefsViewSelection->color( tmp );

      colors.get("hud_color", tmp, 0x80808000 );
      uiPrefs->uiPrefsViewHud->color( tmp );
    }


    //
    // ui/view/hud
    //
    fltk::Preferences hud( view, "hud" );
    hud.get("filename", tmp, 0 );
    uiPrefs->uiPrefsHudFilename->value( tmp );
    hud.get("directory", tmp, 0 );
    uiPrefs->uiPrefsHudDirectory->value( tmp );
    hud.get("fps", tmp, 0 );
    uiPrefs->uiPrefsHudFPS->value( tmp );
    hud.get("av_difference", tmp, 0 );
    uiPrefs->uiPrefsHudAVDifference->value( tmp );
    hud.get("frame", tmp, 0 );
    uiPrefs->uiPrefsHudFrame->value( tmp );
    hud.get("timecode", tmp, 0 );
    uiPrefs->uiPrefsHudTimecode->value( tmp );
    hud.get("resolution", tmp, 0 );
    uiPrefs->uiPrefsHudResolution->value( tmp );
    hud.get("frame_range", tmp, 0 );
    uiPrefs->uiPrefsHudFrameRange->value( tmp );
    hud.get("iptc", tmp, 0 );
    uiPrefs->uiPrefsHudIPTC->value( tmp );

    fltk::Preferences win( view, "window" );
    win.get("fixed_position", tmp, 0 );
    uiPrefs->uiWindowFixedPosition->value( tmp );
    win.get("x_position", tmp, 0 );
    uiPrefs->uiWindowXPosition->value( tmp );
    win.get("y_position", tmp, 0 );
    uiPrefs->uiWindowYPosition->value( tmp );

    fltk::Preferences flu( ui, "file_requester" );
    flu.get("quick_folder_travel", tmp, 1 );
    uiPrefs->uiPrefsFileReqFolder->value( tmp );
    Flu_File_Chooser::singleButtonTravelDrawer = (bool) tmp;

    //
    // playback 
    //
    fltk::Preferences playback( base, "playback" );
    playback.get( "auto_playback", tmp, 0 );
    uiPrefs->uiPrefsAutoPlayback->value(tmp);

    playback.get( "fps", tmpF, 24.0 );
    uiPrefs->uiPrefsFPS->value(tmpF);

    playback.get( "loop_mode", tmp, 1 );
    uiPrefs->uiPrefsLoopMode->value(tmp);

    fltk::Preferences cache( base, "cache" );
    cache.get("pixel_type", tmp, 0 );
    uiPrefs->uiPrefsCachePixelType->value( tmp );

    //
    // audio
    //
    fltk::Preferences audio( base, "audio" );
    char device[256];
    audio.get( "device", device, "default", 255 );


    AudioEngine* engine = AudioEngine::factory();
    delete engine;

    const AudioEngine::DeviceList& devices = AudioEngine::devices();
    if ( devices.empty() )
      {
	LOG_ERROR("No audio device.");
      }
    else
      {
	AudioEngine::DeviceList::const_iterator i = devices.begin();
	AudioEngine::DeviceList::const_iterator e = devices.end();
	unsigned idx = 0;
	for ( ; i != e; ++i, ++idx )
	  {
	    if ( (*i).name == device ) 
	      {
		uiPrefs->uiPrefsAudioDevice->value(idx); 
		break;
	      }
	  }
	
	if ( idx >= devices.size() )
	  {
	    LOG_ERROR("Could not match audio device \"" << device << "\".");
	  }
      }



    //
    // Get environment preferences (LUTS)
    //
    const char* env = getenv( "CTL_MODULE_PATH");
    std::string ctlEnv = "CTL_MODULE_PATH=" + temporaryDirectory();
#if defined(WIN32) || defined(WIN64)
    ctlEnv += ";";
#else
    ctlEnv += ":";
#endif

    if ( !env )
      {
	ctlEnv += root;
	ctlEnv += N_("/ctl");
      }
    else
      {
	ctlEnv += env;
      }

    putenv( strdup( ctlEnv.c_str() ) );


    //
    // Get/set Image Magick preferences
    //
    env = getenv( "MAGICK_CODER_MODULE_PATH");
    if ( !env )
      {
	static const std::string magick_version = "6.6.8";

	std::string ctl = "MAGICK_CODER_MODULE_PATH=" + root;
	ctl += "/lib/ImageMagick-";
	ctl += magick_version;
	ctl += "/modules-Q32/coders";

	putenv( strdup( ctl.c_str() ) );
      }


    fltk::Preferences lut( base, "lut" );
    lut.get("quality", tmpS, "64x64x64", 2047 );
    uiPrefs->uiLUT_quality->value(2);
    int num = uiPrefs->uiLUT_quality->children();
    for ( int i = 0; i < num; ++i )
      {
	const char* label = uiPrefs->uiLUT_quality->child(i)->label();
	if ( strcmp( label, tmpS ) == 0 )
	  {
	    uiPrefs->uiLUT_quality->value(i); break;
	  }
      }

    {
      fltk::Preferences odt( lut, "ODT" );
      {
	odt.get( "algorithm", tmp, 0 );
	uiPrefs->ODT_algorithm->value(tmp);

	fltk::Preferences ctl( odt, "CTL" );
	{
	  ok = ctl.get( "transform", tmpS, "ODT_monitor", 2048 );
	  ODT_CTL_transform = environmentSetting( "MRV_ODT_CTL_TRANSFORM", 
						  tmpS, ok );

	  fltk::Preferences chroma( ctl, "Chromaticities" );
	  ODT_CTL_chromaticities = chromaticities( "MRV_ODT_CTL_CHROMATICITIES",
						   tmpC, chroma );

	  
	  ok = ctl.get( "white_luminance", tmpF, 120.0 );
	  ODT_CTL_white_luminance = environmentSetting( "MRV_ODT_CTL_WHITE_LUMINANCE",
							tmpF, ok );
	  ok = ctl.get( "surround_luminance", tmpF, tmpF * 0.1f );
	  ODT_CTL_white_luminance = environmentSetting( "MRV_ODT_CTL_SURROUND_LUMINANCE",
							tmpF, ok );
	}
	fltk::Preferences icc( odt, "ICC" );
	{
	  ok = icc.get( "profile", tmpS, "", 2048 );
	  ODT_ICC_profile = environmentSetting( "MRV_ODT_ICC_PROFILE", 
						tmpS, ok );
	  if ( !ODT_ICC_profile.empty() )
	    mrv::colorProfile::add( ODT_ICC_profile.c_str() );
	}
      }

      //
      // CTL
      //


      fltk::Preferences rt( lut, "RT" );
      {
	rt.get( "algorithm", tmp, 0 );
	uiPrefs->RT_algorithm->value(tmp);

	fltk::Preferences ctl( rt, "CTL" );
	{
#define RENDER_TRANSFORM(x, d)						\
	  ok = ctl.get( #x, tmpS, d, 2048 );				\
	  CMedia::rendering_transform_##x = environmentSetting( "MRV_CTL_RT_" #x, tmpS, ok )
      
	  RENDER_TRANSFORM( 8bits,  "" );
	  RENDER_TRANSFORM( 16bits, "" );
	  RENDER_TRANSFORM( 32bits, "" );
	  RENDER_TRANSFORM( float,  "transform_RRT" );
#undef RENDER_TRANSFORM
	}

	fltk::Preferences icc( rt, "ICC" );
	{
#define ICC_PROFILE(x, d)						\
	  ok = icc.get( #x, tmpS, d, 2048 );				\
	  CMedia::icc_profile_##x = environmentSetting( "MRV_ICC_RT_" #x, tmpS, ok )

	  ICC_PROFILE( 8bits,  "" );
	  ICC_PROFILE( 16bits, "" );
	  ICC_PROFILE( 32bits, "" );
	  ICC_PROFILE( float,  "" );
#undef ICC_PROFILE
	}
      }
    }

    //
    // ICC
    //



    fltk::Preferences loading( base, "loading" );
    loading.get( "auto_reload", tmp, 1 );
    uiPrefs->uiPrefsAutoReload->value(tmp);


    //
    // Database stuff
    //
    fltk::Preferences db( base, "db" );
    ok  = db.get( "driver", tmpS, "postgresql", 256 );
    env = environmentSetting( "MRV_DATABASE_DRIVER", tmpS, ok );
    if ( env )
      {
	int num = uiPrefs->DatabaseDriver->children();
	for ( int i = 0; i < num; ++i )
	  {
	    const char* label = uiPrefs->DatabaseDriver->child(i)->label();
	    if ( strcasecmp( env, label ) == 0 )
	      {
		char buf[1024];
		sprintf( buf, "MRV_DATABASE_DRIVER=%s", env );
		putenv( strdup(buf) );
		uiPrefs->DatabaseDriver->value(i); break;
	      }
	  }
      }

    //
    // Hotkeys
    //
    fltk::Preferences keys( base, "hotkeys" );
    for ( int i = 0; hotkeys[i].name != "END"; ++i )
    {
       keys.get( (hotkeys[i].name + " ctrl").c_str(), 
		 tmp, (int)hotkeys[i].hotkey.ctrl );
       if ( tmp ) hotkeys[i].hotkey.ctrl = true;
       else       hotkeys[i].hotkey.ctrl = false;
       keys.get( (hotkeys[i].name + " alt").c_str(), 
		 tmp, (int)hotkeys[i].hotkey.alt );
       if ( tmp ) hotkeys[i].hotkey.alt = true;
       else       hotkeys[i].hotkey.alt = false;

       keys.get( (hotkeys[i].name + " meta").c_str(), 
		 tmp, (int)hotkeys[i].hotkey.meta );
       if ( tmp ) hotkeys[i].hotkey.meta = true;
       else       hotkeys[i].hotkey.meta = false;


       keys.get( (hotkeys[i].name + " shift").c_str(), 
		 tmp, (int)hotkeys[i].hotkey.shift );
       if ( tmp ) hotkeys[i].hotkey.shift = true;
       else       hotkeys[i].hotkey.shift = false;

       keys.get( (hotkeys[i].name + " key").c_str(), 
		 tmp, (int)hotkeys[i].hotkey.key );
       hotkeys[i].hotkey.key = unsigned(tmp);
       
       keys.get( (hotkeys[i].name + " text").c_str(), 
		 tmpS, 
		 hotkeys[i].hotkey.text.c_str(), 16 );
       hotkeys[i].hotkey.text = tmpS;

    }

    // Add FLTK image handler (for file requester icons) 
    fltk::SharedImage::add_handler( mrv::fltk_handler );

    // Set the theme and colors for GUI
    fltk::theme( &Preferences::set_theme );
  }


  void Preferences::run( mrv::ViewerUI* main )
  {
    mrv::PreferencesUI* uiPrefs = main->uiPrefs;


    //
    // Windows
    //

    if ( uiPrefs->uiPrefsReelList->value() )
      main->uiReelWindow->uiMain->show();
    else
      main->uiReelWindow->uiMain->hide();

    if ( uiPrefs->uiPrefsImageInfo->value() )
      main->uiImageInfo->uiMain->show();
    else
      main->uiImageInfo->uiMain->hide();

    if ( uiPrefs->uiPrefsColorArea->value() )
      main->uiColorArea->uiMain->show();
    else
      main->uiColorArea->uiMain->hide();

    if ( uiPrefs->uiPrefsHistogram->value() )
      main->uiHistogram->uiMain->show();
    else
      main->uiHistogram->uiMain->hide();

    if ( uiPrefs->uiPrefsVectorscope->value() )
      main->uiVectorscope->uiMain->show();
    else
      main->uiVectorscope->uiMain->hide();

    //
    // Toolbars
    //
    if ( uiPrefs->uiPrefsTopbar->value() )
      main->uiTopBar->show();
    else
      main->uiTopBar->hide();

    if ( uiPrefs->uiPrefsPixelToolbar->value() )
      main->uiPixelBar->show();
    else
      main->uiPixelBar->hide();

    if ( uiPrefs->uiPrefsTimeline->value() )
      main->uiBottomBar->show();
    else
      main->uiBottomBar->hide();

    //
    // Widget/Viewer settings
    //
    main->uiFPS->value( uiPrefs->uiPrefsFPS->value() );
    main->uiFPS->do_callback();

    main->uiLoopMode->value( uiPrefs->uiPrefsLoopMode->value() );
    main->uiLoopMode->do_callback();

    main->uiGain->value( uiPrefs->uiPrefsViewGain->value() );
    main->uiGamma->value( uiPrefs->uiPrefsViewGamma->value() );

    main->uiPixelRatio->value( uiPrefs->uiPrefsViewPixelRatio->value() );
    if ( main->uiPixelRatio->value() )
       main->uiView->toggle_pixel_ratio();
    main->uiLUT->value( uiPrefs->uiPrefsViewLut->value() );
    
    mrv::ImageView* view = main->uiView;
    view->use_lut( uiPrefs->uiPrefsViewLut->value() );


    if ( uiPrefs->uiPrefsSafeAreas->value() )
      view->safe_areas(true);

    //
    // Handle file requester
    //
    Flu_File_Chooser::singleButtonTravelDrawer = (bool)
    uiPrefs->uiPrefsFileReqFolder->value();

    //
    // Handle crop area (masking)
    //
    int crop = uiPrefs->uiPrefsCropArea->value();
    if ( crop > 0 )
      {
	float mask = 1.0f;
	const char* fmt = uiPrefs->uiPrefsCropArea->child(crop)->label();
	sscanf( fmt, "%f", &mask );
	view->masking( mask ); 
      }

    //
    // Handle HUD
    //
    unsigned int hud = mrv::ImageView::kHudNone;
    if ( uiPrefs->uiPrefsHudFilename->value() )
      hud |= mrv::ImageView::kHudFilename;

    if ( uiPrefs->uiPrefsHudFPS->value() )
      hud |= mrv::ImageView::kHudFPS;

    if ( uiPrefs->uiPrefsHudAVDifference->value() )
      hud |= mrv::ImageView::kHudAVDifference;

    if ( uiPrefs->uiPrefsHudTimecode->value() )
      hud |= mrv::ImageView::kHudTimecode;

    if ( uiPrefs->uiPrefsHudFrame->value() )
      hud |= mrv::ImageView::kHudFrame;

    if ( uiPrefs->uiPrefsHudResolution->value() )
      hud |= mrv::ImageView::kHudResolution;

    if ( uiPrefs->uiPrefsHudFrameRange->value() )
      hud |= mrv::ImageView::kHudFrameRange;

    if ( uiPrefs->uiPrefsHudIPTC->value() )
      hud |= mrv::ImageView::kHudIPTC;

    view->hud( (mrv::ImageView::HudDisplay) hud );

   
    main->uiTimecodeSwitch->value( uiPrefs->uiPrefsTimelineDisplay->value() );
    change_timeline_display(main);


    //
    // Handle fullscreen and presentation mode
    //
    if ( uiPrefs->uiWindowFixedPosition->value() )
    {
       int x = uiPrefs->uiWindowXPosition->value();
       int y = uiPrefs->uiWindowYPosition->value();
       main->uiMain->position( x, y );
    }

    main->uiMain->show(0, NULL);
    main->uiMain->set_icon();
    fltk::check();


    fltk::RadioButton* r;
    r = (fltk::RadioButton*) uiPrefs->uiPrefsOpenMode->child(1);

    if ( r->value() == 1 )
      {
	// Fullscreen mode
	main->uiMain->fullscreen();
      }

    r = (fltk::RadioButton*) uiPrefs->uiPrefsOpenMode->child(2);

    if ( r->value() == 1 )
      {
	// Go to presentation mode - window must be shown first, thou.
	view->toggle_fullscreen();
      }


    if ( main->uiPrefs->uiPrefsAlwaysOnTop->value() )
      main->uiMain->always_on_top();
  }


  void Preferences::save()
  {
    int i;
    mrv::PreferencesUI* uiPrefs = mrv::ViewerUI::uiPrefs;

    fltk::Preferences base( fltk::Preferences::USER, "filmaura",
			    "mrViewer" );

    // Save ui preferences
    fltk::Preferences ui( base, "ui" );

    //
    // window options
    //
    {
       fltk::Preferences win( ui, "window" );
       win.set( "always_on_top", (int) uiPrefs->uiPrefsAlwaysOnTop->value() );
       int tmp = 0;
       for ( i = 0; i < uiPrefs->uiPrefsOpenMode->children(); ++i ) {
	  fltk::RadioButton* r = (fltk::RadioButton*) uiPrefs->uiPrefsOpenMode->child(i);
	  if ( r->value() ) {
	     tmp = i; break;
	  }
       }
       win.set( "open_mode", tmp );
    }

    //
    // ui options
    //
    ui.set( "topbar", (int) uiPrefs->uiPrefsTopbar->value() );
    ui.set( "pixel_toolbar", (int) uiPrefs->uiPrefsPixelToolbar->value() );
    ui.set( "timeline_toolbar", (int) uiPrefs->uiPrefsTimeline->value() );
    ui.set( "reel_list", (int) uiPrefs->uiPrefsReelList->value() );
    ui.set( "image_info", (int) uiPrefs->uiPrefsImageInfo->value() );
    ui.set( "color_area", (int) uiPrefs->uiPrefsColorArea->value() );
    ui.set( "histogram", (int) uiPrefs->uiPrefsHistogram->value() );
    ui.set( "vectorscope", (int) uiPrefs->uiPrefsVectorscope->value() );

    ui.set( "timeline_display", 
	    uiPrefs->uiPrefsTimelineDisplay->value() );

    //
    // ui/view prefs
    //
    fltk::Preferences view( ui, "view" );
    view.set("gain", uiPrefs->uiPrefsViewGain->value() );
    view.set("gamma", uiPrefs->uiPrefsViewGamma->value() );
    view.set("compensate_pixel_ratio", uiPrefs->uiPrefsViewPixelRatio->value() );
    view.set("lut", uiPrefs->uiPrefsViewLut->value() );
    view.set("safe_areas", uiPrefs->uiPrefsSafeAreas->value() );
    view.set("crop_area", uiPrefs->uiPrefsCropArea->value() );

    //
    // view/colors prefs
    //
    {
       fltk::Preferences colors( view, "colors" );
       int tmp = uiPrefs->uiPrefsViewBG->color();
       colors.set("background_color", tmp );
       tmp = uiPrefs->uiPrefsViewText->color();
       colors.set("text_overlay_color", tmp );
       tmp = uiPrefs->uiPrefsViewSelection->color();
       colors.set("selection_color", tmp );
       tmp = uiPrefs->uiPrefsViewHud->color();
       colors.set("hud_color", tmp );
    }


    //
    // view/hud prefs
    //
    fltk::Preferences hud( view, "hud" );
    hud.set("filename", uiPrefs->uiPrefsHudFilename->value() );
    hud.set("directory", uiPrefs->uiPrefsHudDirectory->value() );
    hud.set("fps", uiPrefs->uiPrefsHudFPS->value() );
    hud.set("av_difference", uiPrefs->uiPrefsHudAVDifference->value() );
    hud.set("non_drop_timecode", uiPrefs->uiPrefsHudTimecode->value() );
    hud.set("frame", uiPrefs->uiPrefsHudFrame->value() );
    hud.set("resolution", uiPrefs->uiPrefsHudResolution->value() );
    hud.set("frame_range", uiPrefs->uiPrefsHudFrameRange->value() );
    hud.set("iptc", uiPrefs->uiPrefsHudIPTC->value() );

    {
       fltk::Preferences win( view, "window" );
       win.set("fixed_position", uiPrefs->uiWindowFixedPosition->value() );
       win.set("x_position", uiPrefs->uiWindowXPosition->value() );
       win.set("y_position", uiPrefs->uiWindowYPosition->value() );
    }

    //
    // ui/colors prefs
    //
    fltk::Preferences colors( ui, "colors" );
    colors.set( "background_color", bgcolor );
    colors.set( "text_color", textcolor );
    colors.set( "selection_color", selectioncolor );

    fltk::Preferences flu( ui, "file_requester" );
    flu.set("quick_folder_travel", 
	    uiPrefs->uiPrefsFileReqFolder->value());
    
    Flu_File_Chooser::singleButtonTravelDrawer = 
    uiPrefs->uiPrefsFileReqFolder->value();

    //
    // playback prefs
    //
    fltk::Preferences playback( base, "playback" );
    playback.set( "auto_playback", (int) uiPrefs->uiPrefsAutoPlayback->value() );
    playback.set( "fps", uiPrefs->uiPrefsFPS->value() );
    playback.set( "loop_mode", uiPrefs->uiPrefsLoopMode->value() );


    //
    // Cache
    //
    fltk::Preferences cache( base, "cache" );
    cache.set("pixel_type", uiPrefs->uiPrefsCachePixelType->value() );


    fltk::Preferences loading( base, "loading" );
    loading.set( "auto_reload", (int) uiPrefs->uiPrefsAutoReload->value() );


    //
    // Audio prefs
    //
    fltk::Preferences audio( base, "audio" );
    unsigned int idx = uiPrefs->uiPrefsAudioDevice->value();

    const AudioEngine::DeviceList& devices = AudioEngine::devices();

    if ( idx >= devices.size() )
      {
	LOG_ERROR( "Invalid device selected" );
	audio.set( "device", "default" );
      }
    else
      {
	audio.set( "device", devices[idx].name.c_str() );
      }

    fltk::Preferences lut( base, "lut" );
    i = uiPrefs->uiLUT_quality->value();
    if ( i >= 0 && i < uiPrefs->uiLUT_quality->children() )
      {
	lut.set("quality", uiPrefs->uiLUT_quality->child(i)->label() );
      }
    {
      fltk::Preferences odt( lut, "ODT" );
      {
	odt.set( "algorithm", uiPrefs->ODT_algorithm->value() );
	fltk::Preferences ctl( odt, "CTL" );
	{
	  ctl.set( "transform", uiPrefs->uiODT_CTL_transform->text() );

	  fltk::Preferences chroma( ctl, "Chromaticities" );
	  chroma.set( "red_x", 
		      uiPrefs->uiODT_CTL_chromaticities_red_x->value() );
	  chroma.set( "red_y",  
		      uiPrefs->uiODT_CTL_chromaticities_red_y->value() );
	  chroma.set( "green_x",
		      uiPrefs->uiODT_CTL_chromaticities_green_x->value() );
	  chroma.set( "green_y",
		      uiPrefs->uiODT_CTL_chromaticities_green_y->value() );
	  chroma.set( "blue_x", 
		      uiPrefs->uiODT_CTL_chromaticities_blue_x->value()  );
	  chroma.set( "blue_y",  
		      uiPrefs->uiODT_CTL_chromaticities_blue_y->value()  );
	  chroma.set( "white_x", 
		      uiPrefs->uiODT_CTL_chromaticities_white_x->value() );
	  chroma.set( "white_y", 
		      uiPrefs->uiODT_CTL_chromaticities_white_y->value() );

	  ctl.set( "white_luminance", 
		   uiPrefs->uiODT_CTL_white_luminance->value() );
	  ctl.set( "surround_luminance", 
		   uiPrefs->uiODT_CTL_surround_luminance->value() );
	}
	fltk::Preferences icc( odt, "ICC" );
	{
	  icc.set( "profile",   uiPrefs->uiODT_ICC_profile->text() );
	}
      }

      fltk::Preferences  rt( lut, "RT" );
      {
	rt.set( "algorithm", uiPrefs->RT_algorithm->value() );
	
	fltk::Preferences ctl( rt, "CTL" );
	{
	  ctl.set( "8bits",  uiPrefs->uiCTL_8bits_load_transform->text() );
	  ctl.set( "16bits", uiPrefs->uiCTL_16bits_load_transform->text() );
	  ctl.set( "32bits", uiPrefs->uiCTL_32bits_load_transform->text() );
	  ctl.set( "float",  uiPrefs->uiCTL_float_load_transform->text() );
	}

	fltk::Preferences icc( rt, "ICC" );
	{
	  icc.set( "8bits",  uiPrefs->uiICC_8bits_profile->text() );
	  icc.set( "16bits", uiPrefs->uiICC_16bits_profile->text() );
	  icc.set( "32bits", uiPrefs->uiICC_32bits_profile->text() );
	  icc.set( "float",  uiPrefs->uiICC_float_profile->text() );
	}
      }
    }

    //
    // Database
    //
    fltk::Preferences db( base, "db" );
    i = uiPrefs->DatabaseDriver->value();
    const char* driver = NULL;
    if ( i >= 0 && i < uiPrefs->DatabaseDriver->children() )
      driver = uiPrefs->DatabaseDriver->child(i)->label();
    if ( driver ) {
       db.set( "driver", driver );
    }

    //
    // Hotkeys
    //
    fltk::Preferences keys( base, "hotkeys" );
    for ( int i = 0; hotkeys[i].name != "END"; ++i )
    {
       keys.set( (hotkeys[i].name + " ctrl").c_str(), 
		 hotkeys[i].hotkey.ctrl );
       keys.set( (hotkeys[i].name + " alt").c_str(), 
		 hotkeys[i].hotkey.alt );
       keys.set( (hotkeys[i].name + " meta").c_str(), 
		 hotkeys[i].hotkey.meta );
       keys.set( (hotkeys[i].name + " shift").c_str(), 
		 hotkeys[i].hotkey.shift );
       keys.set( (hotkeys[i].name + " key").c_str(), 
		 (int)hotkeys[i].hotkey.key );
       keys.set( (hotkeys[i].name + " text").c_str(), 
		 hotkeys[i].hotkey.text.c_str() );

    }
  }


  bool Preferences::set_theme()
  {
    // Default Style handling for changing the scheme of all widget at once
    fltk::reset_theme();  


    // this is ugly and fucks up all gray75 colors
    //   fltk::set_background( bgcolor );

    // this has default_style
    fltk::Style* style;

    style = fltk::Style::find( "Browser" );
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->labelsize( 10 );
	style->labelcolor( textcolor );
	style->selection_color( selectioncolor );
      }

    style = fltk::Style::find( "TextDisplay" );
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->labelsize( 10 );
	style->labelcolor( textcolor );
	style->selection_color( selectioncolor );
      }

    // this has default_style
    style = fltk::Style::find( "InputBrowser" );
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->labelsize( 10 );
	style->labelcolor( textcolor );
      }

    // this has default_style
    style = fltk::Style::find( "Item" );
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->labelsize( 10 );
	style->labelcolor( textcolor );
      }

    // this has default_style
    // style = fltk::Style::find( "Group" );
    style = group_style;
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->textsize( 10 );
	style->labelsize( 10 );
	style->labelcolor( textcolor );
      }


    style = fltk::Style::find( "Choice" );
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->textsize( 10 );
	style->labelcolor( textcolor );
	style->selection_color( selectioncolor );
      }

    style = fltk::Style::find( "Message" );
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->textsize( 10 );
	style->labelsize( 10 );
	style->labelcolor( textcolor );
	style->selection_color( selectioncolor );
      }

    // this has default_style
    // style = fltk::Style::find( "InvisibleBox" );
    style = fltk::InvisibleBox::default_style;
    if ( style )
      {
	style->labelsize( 10 );
	style->labelcolor( textcolor );
      }

    // this has default_styl
    style = fltk::Style::find( "Button" );
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->textsize( 12 );
	style->labelsize( 12 );
	style->labelcolor( textcolor );
	style->selection_color( selectioncolor );
      }


    // this has default_style
    style = fltk::Style::find("Tooltip");
    if ( style )
      {
	//       style->color( fltk::YELLOW );
	style->textcolor( fltk::BLACK );
	//       style->buttoncolor( bgcolor );
	//       style->textsize( 10 );
	//       style->labelsize( 10 );
	//       style->labelcolor( textcolor );
      }


    //
    // This changes up/down buttons to draw a tad darker
    //
    fltk::FrameBox* box;
    box = (fltk::FrameBox*) fltk::Symbol::find( "down_" );
    if ( box ) box->data(  "2HHOOAA" );

    box = (fltk::FrameBox*) fltk::Symbol::find( "up" );
    if ( box ) box->data(  "AAOOHH" );

    box = (fltk::FrameBox*) fltk::Symbol::find( "engraved" );
    if ( box ) box->data(  "2HHOOOOOOHH" );

    // This is for slider lines
    box = (fltk::FrameBox*) fltk::Symbol::find( "thin_down" );
    if ( box ) box->data(  "OOLL" );


    // this has default_style
    style = fltk::Style::find( "PopupMenu" );
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
 	style->buttoncolor( bgcolor );
	style->textsize( 12 );
	style->labelsize( 12 );
	style->labelcolor( textcolor );
	style->selection_color( selectioncolor );
	style->highlight_color( selectioncolor );
      }

    // this has default_style (not used)
    style = fltk::Style::find( "Input" );
    if ( style )
      {
// 	style->color( mrv::lighter( bgcolor, 0x20 ) );
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->labelcolor( textcolor );
	style->textsize( 10 );
	style->labelsize( 10 );
      }

    // this has default_style
    style = fltk::Style::find( "Window" );
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->labelsize( 10 );
	style->labelcolor( textcolor );
      }

    // this has default_style
    style = fltk::Style::find( "Scrollbar" );
    if ( style )
      {
	style->color( mrv::darker( bgcolor, 0x20 ) );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->labelsize( 10 );
	style->labelcolor( textcolor );
      }

    style = fltk::Style::find( "ProgressBar" );
    if ( style )
      {
	style->selection_color( bgcolor );
	style->color( bgcolor  );
	style->textcolor( textcolor );
	style->labelcolor( textcolor );
	style->buttoncolor( bgcolor );

	style->labelsize( 10 );
      }

    // this has default_style
    style = fltk::Style::find( "Slider" );
    if ( style )
      {
	style->color( mrv::lighter( bgcolor ) );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->textsize( 8 );
	style->labelsize( 10 );
	style->labelcolor( textcolor );


 	style->highlight_textcolor( 0xFFFF0000 );
      }

    // this has default_style
    style = fltk::Widget::default_style;
    if ( style )
      {
	style->color( bgcolor );
	style->textcolor( textcolor );
	style->buttoncolor( bgcolor );
	style->textsize( 14 );
	style->labelsize( 14 );
	style->labelcolor( textcolor );
	style->selection_color( selectioncolor );
      }
    else
      {
	LOG_ERROR( "fltk's widget style not found" );
      }


    // Set ui window settings
    mrv::PreferencesUI* uiPrefs = mrv::ViewerUI::uiPrefs;
    uiPrefs->uiODT_CTL_transform->value( ODT_CTL_transform.c_str() );
    uiPrefs->uiODT_CTL_chromaticities_red_x->value( ODT_CTL_chromaticities.red.x );
    uiPrefs->uiODT_CTL_chromaticities_red_y->value( ODT_CTL_chromaticities.red.y );
    uiPrefs->uiODT_CTL_chromaticities_green_x->value( ODT_CTL_chromaticities.green.x );
    uiPrefs->uiODT_CTL_chromaticities_green_y->value( ODT_CTL_chromaticities.green.y );
    uiPrefs->uiODT_CTL_chromaticities_blue_x->value( ODT_CTL_chromaticities.blue.x );
    uiPrefs->uiODT_CTL_chromaticities_blue_y->value( ODT_CTL_chromaticities.blue.y );
    uiPrefs->uiODT_CTL_chromaticities_white_x->value( ODT_CTL_chromaticities.white.x );
    uiPrefs->uiODT_CTL_chromaticities_white_y->value( ODT_CTL_chromaticities.white.y );

    uiPrefs->uiCTL_8bits_load_transform->value( CMedia::rendering_transform_8bits.c_str() );
    uiPrefs->uiCTL_16bits_load_transform->value( CMedia::rendering_transform_16bits.c_str() );
    uiPrefs->uiCTL_32bits_load_transform->value( CMedia::rendering_transform_32bits.c_str() );
    uiPrefs->uiCTL_float_load_transform->value( CMedia::rendering_transform_float.c_str() );

    uiPrefs->uiODT_ICC_profile->value( ODT_ICC_profile.c_str() );
    uiPrefs->uiICC_8bits_profile->value( CMedia::icc_profile_8bits.c_str() );
    uiPrefs->uiICC_16bits_profile->value( CMedia::icc_profile_16bits.c_str() );
    uiPrefs->uiICC_32bits_profile->value( CMedia::icc_profile_32bits.c_str() );
    uiPrefs->uiICC_float_profile->value( CMedia::icc_profile_float.c_str() );

    return true;
  }


  Preferences::~Preferences()
  {
  }



} // namespace mrv
