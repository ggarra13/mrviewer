/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuno

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
 * @file   mrvPreferences.cpp
 * @author gga
 * @date   Sun Jul  1 19:25:26 2007
 *
 * @brief
 *
 *
 */

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <algorithm>

#include <boost/locale.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <FL/filename.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl.H>

// OpenEXR threadcount
#include <OpenEXR/ImfThreading.h>

// OpenColorIO
#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

// CORE classes
#include "core/exrImage.h"
#include "core/mrvAudioEngine.h"
#include "core/mrvException.h"
#include "core/mrvColorProfile.h"
#include "core/mrvHome.h"
#include "core/mrvI8N.h"
#include "core/mrvOS.h"
#include "core/mrvMath.h"
#include "core/CMedia.h"

// GUI  classes
#include "gui/mrvColorOps.h"
#include "gui/mrvColorSchemes.h"
#include "gui/mrvImageView.h"
#include "gui/mrvMainWindow.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvLogDisplay.h"
#include "gui/mrvFLTKHandler.h"
#include "gui/FLU/Flu_File_Chooser.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvIO.h"
#include "gui/mrvHotkey.h"
#include "gui/mrvImageBrowser.h"
#include "video/mrvGLLut3d.h"
#include "video/mrvGLEngine.h"
#include "mrvEDLWindowUI.h"

// Widgets
#include "mrvPreferencesUI.h"
#include "mrvColorAreaUI.h"
#include "mrvReelUI.h"
#include "mrViewer.h"


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
    if ( !env || strlen(env) == 0 )
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
    Fl_Preferences& chroma
)
{
    Imf::Chromaticities tmpC;
    int ok  = chroma.get( "red_x", tmpC.red.x, defaultValue.red.x );
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


AboutUI*          ViewerUI::uiAbout = NULL;
LogUI*            ViewerUI::uiLog   = NULL;
PreferencesUI*    ViewerUI::uiPrefs = NULL;
ICCProfileListUI* ViewerUI::uiICCProfiles = NULL;
HotkeyUI*         ViewerUI::uiHotkey = NULL;
ConnectionUI*     ViewerUI::uiConnection = NULL;

namespace mrv {

ColorSchemes        Preferences::schemes;
bool                Preferences::use_ocio = false;
ViewerUI*           Preferences::uiMain = NULL;
bool                Preferences::native_file_chooser;
std::string         Preferences::OCIO_Display;
std::string         Preferences::OCIO_View;
std::string         Preferences::ODT_CTL_transform;
std::string         Preferences::ODT_ICC_profile;
Imf::Chromaticities Preferences::ODT_CTL_chromaticities;
float               Preferences::ODT_CTL_white_luminance = 120.0f;
float               Preferences::ODT_CTL_surround_luminance = 12.0f;

mrv::Preferences::MissingFrameType      Preferences::missing_frame;

std::string         Preferences::video_threads;

std::string         Preferences::CTL_8bits_save_transform;
std::string         Preferences::CTL_16bits_save_transform;
std::string         Preferences::CTL_32bits_save_transform;
std::string         Preferences::CTL_float_save_transform;
std::string         Preferences::root;
int                 Preferences::debug = 0;
std::string         Preferences::tempDir = "/usr/tmp/";



int   Preferences::bgcolor;
int   Preferences::textcolor;
int   Preferences::selectioncolor;
int   Preferences::selectiontextcolor;

int64_t Preferences::max_memory = 1000000000;

static std::string expandVariables( const std::string &s,
                                    const char* START_VARIABLE,
                                    const char END_VARIABLE)
{


    size_t p = s.find( START_VARIABLE );

    if( p == std::string::npos ) return s;

    std::string pre  = s.substr( 0, p );
    std::string post = s.substr( p + strlen(START_VARIABLE) );

    size_t e = post.find( END_VARIABLE );

    if( e == std::string::npos ) return s;

    std::string variable = post.substr( 0, e );
    std::string value    = "";

    post = post.substr( e + 1 );

    const char *v = getenv( variable.c_str() );
    if( v != NULL ) value = std::string( v );

    return expandVariables( pre + value + post, START_VARIABLE,
                            END_VARIABLE );
}

Preferences::Preferences( PreferencesUI* uiPrefs )
{
    bool ok;
    int version;
    int tmp;
    float tmpF;
    char  tmpS[2048];
    Imf::Chromaticities tmpC, c;

    DBG;
    const char* r = getenv( "MRV_ROOT" );
    if ( r )
    {
        DBG;
        root = r;
        DBG;
        if ( root.empty() )
        {
            DBG;
            EXCEPTION("Environment variable MRV_ROOT not set.  Aborting");
        }
    }


    DBG;
    Fl_Preferences base( prefspath().c_str(), "filmaura",
                         "mrViewer" );

    DBG;
    base.get( "version", version, 3 );

    //
    // Get ui preferences
    //
    DBG;
    Fl_Preferences ui( base, "ui" );

    ui.get( "single_instance", tmp, 0 );
    uiPrefs->uiPrefsSingleInstance->value( (bool) tmp );

    DBG;
    ui.get( "topbar", tmp, 1 );
    uiPrefs->uiPrefsTopbar->value( (bool) tmp );

    ui.get( "pixel_toolbar", tmp, 1 );
    uiPrefs->uiPrefsPixelToolbar->value( (bool) tmp );

    DBG;
    ui.get( "timeline_toolbar", tmp, 1 );
    uiPrefs->uiPrefsTimeline->value( (bool) tmp );

    ui.get( "reel_list", tmp, 0 );
    uiPrefs->uiPrefsReelList->value( (bool) tmp );

    DBG;
    ui.get( "edl_edit", tmp, 0 );
    uiPrefs->uiPrefsEDLEdit->value(tmp);

    ui.get( "stereo3d_options", tmp, 0 );
    uiPrefs->uiPrefsStereoOptions->value(tmp);

    DBG;
    ui.get( "action_tools", tmp, 0 );
    uiPrefs->uiPrefsPaintTools->value(tmp);

    ui.get( "image_info", tmp, 0 );
    uiPrefs->uiPrefsImageInfo->value(tmp);

    ui.get( "color_area", tmp, 0 );
    uiPrefs->uiPrefsColorArea->value(tmp);

    DBG;
    ui.get( "histogram", tmp, 0 );
    uiPrefs->uiPrefsHistogram->value(tmp);

    ui.get( "vectorscope", tmp, 0 );
    uiPrefs->uiPrefsVectorscope->value(tmp);

    DBG;
    ui.get( "waveform", tmp, 0 );
    uiPrefs->uiPrefsWaveform->value(tmp);

    ui.get( "timeline_display", tmp, 0 );
    uiPrefs->uiPrefsTimelineDisplay->value(tmp);


    DBG;
    //
    // ui/window preferences
    //
    {
        Fl_Preferences win( ui, "window" );

        win.get( "auto_fit_image", tmp, 1 );
        uiPrefs->uiPrefsAutoFitImage->value( tmp );

    DBG;
        win.get( "always_on_top", tmp, 0 );
        uiPrefs->uiPrefsAlwaysOnTop->value( tmp );

    DBG;
        win.get( "open_mode", tmp, 0 );

        {
            Fl_Round_Button* r;
            for ( int i = 0; i < uiPrefs->uiPrefsOpenMode->children(); ++i )
            {
    DBG;
                r = (Fl_Round_Button*) uiPrefs->uiPrefsOpenMode->child( i );
                r->value(0);
            }
    DBG;
            r = (Fl_Round_Button*)uiPrefs->uiPrefsOpenMode->child( tmp );
            r->value(1);
        }

    }



    //
    // ui/view
    //
    DBG;
    Fl_Preferences view( ui, "view" );

    view.get("gain", tmpF, 1.0f );
    uiPrefs->uiPrefsViewGain->value( tmpF );

    view.get("gamma", tmpF, 1.0f );
    uiPrefs->uiPrefsViewGamma->value( tmpF );

    DBG;
    view.get("compensate_pixel_ratio", tmp, 0 );
    uiPrefs->uiPrefsViewPixelRatio->value( (bool) tmp );

    view.get("lut", tmp, 1 );

    uiPrefs->uiPrefsViewLut->value( (bool) tmp );

    DBG;
    view.get("safe_areas", tmp, 0 );
    uiPrefs->uiPrefsSafeAreas->value( (bool) tmp );

    view.get("crop_area", tmp, 0 );
    uiPrefs->uiPrefsCropArea->value( tmp );

    DBG;
    view.get("display_window", tmp, 1 );
    uiPrefs->uiPrefsViewDisplayWindow->value( (bool)tmp );

    view.get("data_window", tmp, 1 );
    uiPrefs->uiPrefsViewDataWindow->value( (bool)tmp );

    //
    // ui/colors
    //
    DBG;
    Fl_Preferences colors( ui, "colors" );
    DBG;
    colors.get( "scheme", tmpS, "plastic", 2048 );
    DBG;
    const Fl_Menu_Item* item = uiPrefs->uiScheme->find_item( tmpS );
    if ( item )
    {
    DBG;
        uiPrefs->uiScheme->picked( item );
    }
    DBG;
    colors.get( "background_color", bgcolor, 0x43434300 );
    DBG;
    colors.get( "text_color", textcolor, 0xababab00 );
    DBG;
    colors.get( "selection_color", selectioncolor, 0x97a8a800 );
    DBG;
    colors.get( "selection_text_color", selectiontextcolor, 0x00000000 );

    bool loaded = false;
    DBG;
    std::string colorname = prefspath() + "mrViewer.colors";
    if ( ! (loaded = schemes.read_themes( colorname.c_str() )) )
    {
    DBG;
        colorname = root + "/colors/mrViewer.colors";
        if ( ! (loaded = schemes.read_themes( colorname.c_str() )) )
        {
    DBG;
            LOG_ERROR( _("Could not open \"") << colorname << "\"" );
        }
    }

    if ( loaded )
    {
    DBG;
        LOG_INFO( _("Loaded color themes from ") << colorname << "." );
    }

    DBG;
    for ( auto& s: schemes.themes )
    {
    DBG;
        uiPrefs->uiColorTheme->add( s.name.c_str() );
    }

    colors.get( "theme", tmpS, "Shake", 2048 );
    DBG;
    item = uiPrefs->uiColorTheme->find_item( tmpS );
    if ( item )
    {
    DBG;
        uiPrefs->uiColorTheme->picked( item );
    }

    //
    // ui/view/colors
    //
    {
    DBG;
        Fl_Preferences colors( view, "colors" );

        colors.get("background_color", tmp, 0x20202000 );
        uiPrefs->uiPrefsViewBG->color( tmp );

        colors.get("text_overlay_color", tmp, 0xFFFF0000 );
        uiPrefs->uiPrefsViewTextOverlay->color( tmp );

        colors.get("selection_color", tmp, 0x0000FF00 );
        uiPrefs->uiPrefsViewSelection->color( tmp );

    DBG;
        colors.get("hud_color", tmp, 0xF0F08000 );
        uiPrefs->uiPrefsViewHud->color( tmp );
    }

    DBG;
    Fl_Preferences ocio( view, "ocio" );
    if ( version < 3 )
    {
        ocio.get( "use_ocio", tmp, 0 );
        const char* var = getenv( "OCIO" );

    DBG;
        if ( var && strlen(var) > 0 )
            tmp = true;
    }
    else
    {
    DBG;
        ocio.get( "use_ocio", tmp, 1 );
    }
    DBG;
    uiPrefs->uiPrefsUseOcio->value( tmp );
    use_ocio = (bool)tmp;


    DBG;
    ocio.get( "save_config", tmp, 0 );
    uiPrefs->uiPrefsSaveOcio->value( tmp );

    DBG;
    ocio.get( "config", tmpS, "", 2048 );
    uiPrefs->uiPrefsOCIOConfig->value( tmpS );

    DBG;
    Fl_Preferences ics( ocio, "ICS" );
    {
#define OCIO_ICS(x, d)							\
        ok = ics.get( #x, tmpS, d, 2048 );                              \
        CMedia::ocio_##x##_ics = environmentSetting( "MRV_OCIO_" #x "_ICS" , \
                                                     tmpS, ok );	\
        uiPrefs->uiOCIO_##x##_ics->value( tmpS );

        OCIO_ICS( 8bits,  "" );
    DBG;
        OCIO_ICS( 16bits, "" );
    DBG;
        OCIO_ICS( 32bits, "" );
    DBG;
        OCIO_ICS( float,  "" );
    DBG;

    }

    //
    // ui/view/hud
    //
    Fl_Preferences hud( view, "hud" );
    DBG;
    hud.get("filename", tmp, 0 );
    DBG;
    uiPrefs->uiPrefsHudFilename->value( (bool) tmp );
    hud.get("directory", tmp, 0 );
    uiPrefs->uiPrefsHudDirectory->value( (bool) tmp );
    hud.get("fps", tmp, 0 );
    uiPrefs->uiPrefsHudFPS->value( (bool) tmp );
    hud.get("av_difference", tmp, 0 );
    DBG;
    uiPrefs->uiPrefsHudAVDifference->value( (bool) tmp );
    hud.get("frame", tmp, 0 );
    uiPrefs->uiPrefsHudFrame->value( (bool) tmp );
    hud.get("timecode", tmp, 0 );
    uiPrefs->uiPrefsHudTimecode->value( (bool) tmp );
    hud.get("resolution", tmp, 0 );
    DBG;
    uiPrefs->uiPrefsHudResolution->value( (bool) tmp );
    hud.get("frame_range", tmp, 0 );
    uiPrefs->uiPrefsHudFrameRange->value( (bool) tmp );
    hud.get("memory", tmp, 0 );
    DBG;
    uiPrefs->uiPrefsHudMemory->value( (bool) tmp );
    hud.get("attributes", tmp, 0 );
    DBG;
    uiPrefs->uiPrefsHudAttributes->value( (bool) tmp );

    Fl_Preferences win( view, "window" );
    win.get("fixed_position", tmp, 0 );
    uiPrefs->uiWindowFixedPosition->value( (bool) tmp );
    win.get("x_position", tmp, 0 );
    DBG;
    uiPrefs->uiWindowXPosition->value( tmp );
    win.get("y_position", tmp, 0 );
    uiPrefs->uiWindowYPosition->value( tmp );
    win.get("fixed_size", tmp, 0 );
    DBG;
    uiPrefs->uiWindowFixedSize->value( (bool) tmp );
    win.get("x_size", tmp, 640 );
    uiPrefs->uiWindowXSize->value( tmp );
    win.get("y_size", tmp, 530 );
    DBG;
    uiPrefs->uiWindowYSize->value( tmp );

    Fl_Preferences flu( ui, "file_requester" );
    //

    DBG;
    flu.get("quick_folder_travel", tmp, 1 );
    uiPrefs->uiPrefsFileReqFolder->value( (bool) tmp );
    Flu_File_Chooser::singleButtonTravelDrawer = (bool) tmp;
    DBG;
    flu.get("thumbnails", tmp, 1 );
    uiPrefs->uiPrefsFileReqThumbnails->value( (bool) tmp );
    Flu_File_Chooser::thumbnailsFileReq = (bool) tmp;

    //
    // playback
    //
    Fl_Preferences playback( base, "playback" );
    DBG;
    playback.get( "auto_playback", tmp, 1 );
    uiPrefs->uiPrefsAutoPlayback->value(tmp);

    DBG;
    playback.get( "play_all_frames", tmp, 1 );
    uiPrefs->uiPrefsPlayAllFrames->value(tmp);

    DBG;
    playback.get( "override_fps", tmp, 0 );
    uiPrefs->uiPrefsOverrideFPS->value(tmp);

    DBG;
    playback.get( "fps", tmpF, 24.0 );
    uiPrefs->uiPrefsFPS->value(tmpF);
    CMedia::default_fps = tmpF;

    DBG;
    playback.get( "loop_mode", tmp, 1 );
    uiPrefs->uiPrefsLoopMode->value(tmp);

    DBG;
    playback.get( "scrubbing_sensitivity", tmpF, 5.0f );
    uiPrefs->uiPrefsScrubbingSensitivity->value(tmpF);

    DBG;
    playback.get( "selection_display_mode", tmp, 0 );
    uiPrefs->uiPrefsTimelineSelectionDisplay->value(tmp);

    Fl_Preferences pixel_toolbar( base, "pixel_toolbar" );
    DBG;
    pixel_toolbar.get( "RGBA_pixel", tmp, 0 );
    uiPrefs->uiPrefsPixelRGBA->value( tmp );

    DBG;
    pixel_toolbar.get( "pixel_values", tmp, 0 );
    uiPrefs->uiPrefsPixelValues->value( tmp );

    DBG;
    pixel_toolbar.get( "HSV_pixel", tmp, 0 );
    uiPrefs->uiPrefsPixelHSV->value( tmp );

    DBG;
    pixel_toolbar.get( "Lumma_pixel", tmp, 0 );
    uiPrefs->uiPrefsPixelLumma->value( tmp );


    Fl_Preferences action( base, "action" );
    DBG;
    action.get( "scrubbing", tmp, 1 );
    uiPrefs->uiScrub->value( (bool) tmp );
    action.get( "move_picture", tmp, 0 );
    uiPrefs->uiMovePicture->value( (bool) tmp );
    action.get( "color_area", tmp, 0 );
    DBG;
    uiPrefs->uiSelection->value( (bool) tmp );
    action.get( "pencil", tmp, 0 );
    uiPrefs->uiDraw->value( (bool) tmp );
    action.get( "text", tmp, 0 );
    DBG;
    uiPrefs->uiText->value( (bool) tmp );
    action.get( "eraser", tmp, 0 );
    uiPrefs->uiErase->value( (bool) tmp );
    DBG;

    Fl_Preferences caches( base, "caches" );

    DBG;
    caches.get( "active", tmp, 1 );
    uiPrefs->uiPrefsCacheActive->value( (bool) tmp );
    CMedia::cache_active( (bool) tmp );

    DBG;
    caches.get( "preload", tmp, 1 );
    uiPrefs->uiPrefsPreloadCache->value( (bool) tmp );
    CMedia::preload_cache( (bool) tmp );

    DBG;
    caches.get( "scale", tmp, 0 );
    uiPrefs->uiPrefsCacheScale->value( tmp );
    CMedia::cache_scale( tmp );


    DBG;
    caches.get( "8bit_caches", tmp, 0 );
    uiPrefs->uiPrefs8BitCaches->value( (bool) tmp );
    CMedia::eight_bit_caches( (bool) tmp );

    DBG;

    caches.get( "fps", tmp, 1 );
    uiPrefs->uiPrefsCacheFPS->value( (bool) tmp );
    if ( !tmp )
    {
    DBG;
        caches.get( "size", tmp, 20 );
        uiPrefs->uiPrefsCacheSize->activate();
    DBG;
        uiPrefs->uiPrefsCacheSize->value( tmp );
        CMedia::video_cache_size( tmp );
        CMedia::audio_cache_size( tmp );
    }
    else
    {
    DBG;
        uiPrefs->uiPrefsCacheSize->deactivate();
        CMedia::video_cache_size( 0 );
        CMedia::audio_cache_size( 0 );
    }

    DBG;
    caches.get( "cache_memory", tmpF, 4.0 );
#if defined( _WIN64 ) || defined( LINUX )
    if ( tmpF == 1.5f )
    {
    DBG;
        tmpF = 4.0;
    }
#endif
    DBG;
    uiPrefs->uiPrefsCacheMemory->value( tmpF );

    //
    // audio
    //
    Fl_Preferences audio( base, "audio" );
    char device[256];
    audio.get( "device", device, "default", 255 );

    DBG;
    AudioEngine* engine = AudioEngine::factory();
    DBG;
    delete engine;

    DBG;
    const AudioEngine::DeviceList& devices = AudioEngine::devices();
    DBG;
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
    DBG;
            if ( (*i).name == device )
            {
    DBG;
                uiPrefs->uiPrefsAudioDevice->value(idx);
                break;
            }
        }

    DBG;
        if ( idx >= devices.size() )
        {
    DBG;
            LOG_ERROR("Could not match audio device \"" << device << "\".");
        }
    }

    audio.get( "override_audio", tmp, 0 );
    uiPrefs->uiPrefsOverrideAudio->value( tmp );

    DBG;
    audio.get( "volume", tmpF, 1.0f );
    uiPrefs->uiPrefsAudioVolume->value( tmpF );

    DBG;
    audio.get( "volume_mute", tmp, 0 );
    uiPrefs->uiPrefsAudioMute->value( tmp );

    // Images
    DBG;
    Fl_Preferences images( base, "images" );
    images.get( "all_layers", tmp, 0 );
    uiPrefs->uiPrefsAllLayers->value( tmp );

    DBG;
    images.get( "aces_metadata", tmp, 0 );
    uiPrefs->uiPrefsACESClipMetadata->value( tmp );

    // OpenEXR
    DBG;
    Fl_Preferences openexr( base, "openexr" );
    openexr.get( "thread_count", tmp, 4 );
    uiPrefs->uiPrefsOpenEXRThreadCount->value( tmp );

    DBG;
    openexr.get( "gamma", tmpF, 2.2f );
    if ( !use_ocio ) {
    DBG;
        exrImage::_default_gamma = tmpF;
        uiPrefs->uiPrefsOpenEXRGamma->value( tmpF );
    }
    else
    {
    DBG;
        exrImage::_default_gamma = 1.0f;
        uiPrefs->uiPrefsOpenEXRGamma->value( 1.0f );
    }

    DBG;
    openexr.get( "compression", tmp, 4 );   // PIZ default
    exrImage::_default_compression = (Imf::Compression) tmp;
    uiPrefs->uiPrefsOpenEXRCompression->value( tmp );

    DBG;
    openexr.get( "dwa_compression", tmpF, 45.0f );
    exrImage::_default_dwa_compression = tmpF;
    uiPrefs->uiPrefsOpenEXRDWACompression->value( tmpF );



    //
    // Get environment preferences (LUTS)
    //
    const char* env = getenv( "CTL_MODULE_PATH");
    std::string ctlEnv = temporaryDirectory();
#if defined(WIN32) || defined(WIN64)
    char sep = ';';
#else
    char sep = ':';
#endif

    DBG;
    ctlEnv += sep;

    if ( !env )
    {
    DBG;
        ctlEnv += root;
        ctlEnv += N_("/ctl");
    }
    else
    {
    DBG;
        ctlEnv += env;
    }

    DBG;
    std::string var = "CTL_MODULE_PATH=" + ctlEnv;
    putenv( strdup( var.c_str() ) );


    size_t found = 0;
    DBG;
    while( (found = ctlEnv.find(sep)) != std::string::npos )
    {
        std::string part2;
        if ( found+1 < ctlEnv.size() )
            part2 = ctlEnv.substr( found + 1, ctlEnv.size() );
    DBG;
        ctlEnv = ctlEnv.substr(0, found);
    DBG;
        uiPrefs->uiPrefsCTLModulePath->add( ctlEnv.c_str() );
        ctlEnv = part2;
    }

    for ( int j = 1; j <= uiPrefs->uiPrefsCTLModulePath->size(); ++j )
    {
    DBG;
        char* name;
        dirent** e;
        const char* dir = uiPrefs->uiPrefsCTLModulePath->text(j);
        int num = fl_filename_list( dir, &e );
        for( int i = 0; i < num; i++ )
        {
            name = e[i]->d_name;

            // if 'name' ends in '/' or '\', remove it
            if( name[strlen(name)-1] == '/' || name[strlen(name)-1] == '\\' )
                name[strlen(name)-1] = '\0';

            // ignore the "." and ".." names
            if( strcmp( name, "." ) == 0 || strcmp( name, ".." ) == 0 )
                continue;

            std::string fullpath = dir;
            fullpath += "/";
            fullpath += name;

            if ( fullpath.substr( fullpath.size() - 4, fullpath.size() ) !=
                 ".ctl" ) continue;

            if( fl_filename_isdir( fullpath.c_str() ) )
                continue;

            uiPrefs->uiPrefsCTLScripts->add( name );
        }

        fl_filename_free_list( &e, num );
    }




    DBG;
    Fl_Preferences lut( base, "lut" );
    lut.get("quality", tmpS, "128x128x128", 2047 );
    uiPrefs->uiLUT_quality->value(3);
    DBG;
    int num = uiPrefs->uiLUT_quality->children();
    for ( int i = 0; i < num; ++i )
    {
        const char* label = uiPrefs->uiLUT_quality->child(i)->label();
        if ( strcmp( label, tmpS ) == 0 )
        {
    DBG;
            uiPrefs->uiLUT_quality->value(i);
            break;
        }
    }

    DBG;
    lut.get("number_stops", tmp, 10 );
    uiPrefs->uiPrefsNumStops->value( tmp );

    {
        Fl_Preferences odt( lut, "ODT" );
        {
            odt.get( "algorithm", tmp, 0 );
            uiPrefs->ODT_algorithm->value(tmp);

    DBG;
            Fl_Preferences ctl( odt, "CTL" );
            {
                ok = ctl.get( "transform", tmpS, "ODT.Academy.RGBmonitor_D60sim_100nits_dim", 2048 );
                ODT_CTL_transform = environmentSetting( "MRV_ODT_CTL_DISPLAY_TRANSFORM",
                                                        tmpS, ok );

    DBG;
                Fl_Preferences chroma( ctl, "Chromaticities" );
                ODT_CTL_chromaticities = chromaticities( "MRV_ODT_CTL_DISPLAY_CHROMATICITIES",
                                         tmpC, chroma );


    DBG;
                ok = ctl.get( "white_luminance", tmpF, 120.0 );
                ODT_CTL_white_luminance = environmentSetting( "MRV_ODT_CTL_DISPLAY_WHITE_LUMINANCE",
                                          tmpF, ok );
                ok = ctl.get( "surround_luminance", tmpF, tmpF * 0.1f );
                ODT_CTL_white_luminance = environmentSetting( "MRV_ODT_CTL_DISPLAY_SURROUND_LUMINANCE",
                                          tmpF, ok );
    DBG;
            }
            Fl_Preferences icc( odt, "ICC" );
            {
                ok = icc.get( "profile", tmpS, "", 2048 );
                ODT_ICC_profile = environmentSetting( "MRV_ODT_ICC_PROFILE",
                                                      tmpS, ok );
    DBG;
                if ( !ODT_ICC_profile.empty() )
                    mrv::colorProfile::add( ODT_ICC_profile.c_str() );
            }
        }

        //
        // CTL
        //


#if 0
        Fl_Preferences idt( lut, "IDT" );
        {
            idt.get( "MRV_CTL_IDT_TRANSFORM", tmpS, "" );
            uiPrefs->IDT_transform->value(tmpS);
        }
#endif


        Fl_Preferences rt( lut, "RT" );
        {
    DBG;
            rt.get( "algorithm", tmp, 0 );
            uiPrefs->RT_algorithm->value(tmp);

    DBG;
            Fl_Preferences ctl( rt, "CTL" );
            {
#define RENDER_TRANSFORM(x, d)						\
          ok = ctl.get( #x, tmpS, d, 2048 );				\
          CMedia::rendering_transform_##x = environmentSetting( "MRV_CTL_RT_" #x, tmpS, ok )

                RENDER_TRANSFORM( 8bits,  "" );
    DBG;
                RENDER_TRANSFORM( 16bits, "" );
    DBG;
                RENDER_TRANSFORM( 32bits, "" );
    DBG;
                RENDER_TRANSFORM( float,  "RRT" );
    DBG;
#undef RENDER_TRANSFORM
            }

            //
            // ICC
            //

            Fl_Preferences icc( rt, "ICC" );
            {
#define ICC_PROFILE(x, d)						\
          ok = icc.get( #x, tmpS, d, 2048 );				\
          CMedia::icc_profile_##x = environmentSetting( "MRV_ICC_RT_" #x, tmpS, ok ); \
          uiPrefs->uiICC_## x ## _profile->value( tmpS ); \
          if ( strlen( tmpS ) > 0 ) mrv::colorProfile::add( tmpS );
    DBG;
                ICC_PROFILE( 8bits,  "" );
    DBG;
                ICC_PROFILE( 16bits, "" );
    DBG;
                ICC_PROFILE( 32bits, "" );
    DBG;
                ICC_PROFILE( float,  "" );
#undef ICC_PROFILE
            }
        }
    }


    Fl_Preferences loading( base, "loading" );

    loading.get( "load_library", tmp, 1 );
    uiPrefs->uiPrefsLoadLibrary->value( tmp );

    DBG;
    loading.get( "missing_frames", tmp, 0 );
    uiPrefs->uiPrefsMissingFrames->value( tmp );

    loading.get( "drag_load_seq", tmp, 1 );
    uiPrefs->uiPrefsLoadSequence->value( (bool) tmp );
    DBG;

    loading.get( "file_assoc_load_seq", tmp, 1 );
    uiPrefs->uiPrefsLoadSequenceOnAssoc->value( (bool) tmp );

    DBG;
    loading.get( "autoload_images", tmp, 0 );
    uiPrefs->uiPrefsAutoLoadImages->value( (bool) tmp );

#ifdef _WIN32
    loading.get( "native_file_chooser", tmp, 1 );
#else
    loading.get( "native_file_chooser", tmp, 0 );
#endif
    uiPrefs->uiPrefsNativeFileChooser->value( (bool) tmp );


    loading.get( "thumbnail_percent", tmpF, 0.0 );
    CMedia::thumbnail_percent = tmpF;
    uiPrefs->uiPrefsThumbnailPercent->value( tmpF );

    DBG;
    loading.get( "uses_16bits", tmp, 0 );
    uiPrefs->uiPrefsUses16Bits->value( (bool) tmp );
    CMedia::uses_16bits( (bool) tmp );

    loading.get( "image_version_prefix", tmpS, "_v", 10 );
    uiPrefs->uiPrefsImageVersionPrefix->value( tmpS );

    loading.get( "max_images_apart", tmp, 10 );
    uiPrefs->uiPrefsMaxImagesApart->value( tmp );

    DBG;
    Fl_Preferences saving( base, "saving" );
    saving.get( "use_relative_paths", tmp, 1 );
    uiPrefs->uiPrefsRelativePaths->value( tmp );

    saving.get( "use_image_path", tmp, 1 );
    uiPrefs->uiPrefsImagePathReelPath->value( tmp );

    DBG;
    Fl_Preferences video( base, "video" );
    video.get( "filtering", tmp, 0 );
    uiPrefs->uiPrefsFiltering->value( tmp );
    video.get( "video_codec", tmp, 0 );
    uiPrefs->uiPrefsVideoCodec->value(tmp);
    video.get( "yuv_hint", tmp, 2 );
    uiPrefs->uiPrefsYUVConversion->value(tmp);
    CMedia::colorspace_override = tmp;
    video.get( "thread_count", tmp, 0 );
    DBG;
    uiPrefs->uiPrefsVideoThreadCount->value( tmp );

    Fl_Preferences comp( base, "compositing" );
    comp.get( "blend_mode", tmp, 0 );
    DBG;
    uiPrefs->uiPrefsBlendMode->value(tmp);
    comp.get( "resize_bg", tmp, 1 );
    DBG;
    uiPrefs->uiPrefsResizeBackground->value(tmp);

    Fl_Preferences subtitles( base, "subtitles" );
    DBG;
    subtitles.get( "font", tmpS, "Arial", 2048 );
    uiPrefs->uiPrefsSubtitleFont->value(0);  // in case no font is found
    for (int i = 0; i < uiPrefs->uiPrefsSubtitleFont->children(); ++i )
    {
        DBG;
        const char* label = uiPrefs->uiPrefsSubtitleFont->child(i)->label();
        if ( tmpS && label && strcmp( label, tmpS ) == 0 )
        {
            DBG;
            uiPrefs->uiPrefsSubtitleFont->value(i);
            break;
        }
    }
    DBG;
    subtitles.get( "encoding", tmpS, "ISO-8859-1", 2048 );
    uiPrefs->uiPrefsSubtitleEncoding->value( tmpS );
    DBG;

    Fl_Preferences errors( base, "errors" );
    DBG;
    errors.get( "raise_log_window_on_error", tmp, 0 );
    uiPrefs->uiPrefsRaiseLogWindowOnError->value(tmp);

    //
    // Hotkeys
    //
    Fl_Preferences keys( base, "hotkeys" );
    DBG;
    for ( int i = 0; hotkeys[i].name != "END"; ++i )
    {
        // If version >= 1 of preferences, do not set scrub
        if ( version >= 1 && hotkeys[i].name == "Scrub" )
            continue;

    DBG;
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

    DBG;

        keys.get( (hotkeys[i].name + " shift").c_str(),
                  tmp, (int)hotkeys[i].hotkey.shift );
        if ( tmp ) hotkeys[i].hotkey.shift = true;
        else       hotkeys[i].hotkey.shift = false;

        keys.get( (hotkeys[i].name + " key").c_str(),
                  tmp, (int)hotkeys[i].hotkey.key );
        hotkeys[i].hotkey.key = unsigned(tmp);

        keys.get( (hotkeys[i].name + " key2").c_str(),
                  tmp, (int)hotkeys[i].hotkey.key2 );
        hotkeys[i].hotkey.key2 = unsigned(tmp);

    DBG;
        keys.get( (hotkeys[i].name + " text").c_str(),
                  tmpS,
                  hotkeys[i].hotkey.text.c_str(), 16 );
        if ( tmpS ) hotkeys[i].hotkey.text = tmpS;
        else hotkeys[i].hotkey.text.clear();
    }


    fill_ui_hotkeys( uiMain->uiHotkey->uiFunction );

    // Set the CTL/ICC transforms in GUI
    if ( ! set_transforms() )
    {
    DBG;
        LOG_ERROR( _("Could not set transforms in GUI") );
    }
}

#ifdef _WIN32
static const char* kCLocale = "English";
#else
static const char* kCLocale = "C";
#endif

//#undef DBG
//#define DBG(c) std::cerr << c << " " << __FUNCTION__ << " "	\
//    << __LINE__ << std::endl;

void Preferences::run( ViewerUI* main )
{
    uiMain = main;
    PreferencesUI* uiPrefs = main->uiPrefs;

    DBG;

    main->uiMain->show();

    // Fl_Widget* w = new Fl_Widget( 0, 48, 639, 40, "Eye1" );
    // main->uiBottomBar->add( w );
    // w = new Fl_Widget( 0, 88, 639, 40, "Eye2" );
    // main->uiBottomBar->add( w );

    DBG;
    Fl::check();

    //
    // Windows
    //



    DBG;
    if ( uiPrefs->uiPrefsEDLEdit->value() )
    {
        main->uiEDLWindow->uiMain->show();
    }
    else
        main->uiEDLWindow->uiMain->hide();

    PaintUI* uiPaint = main->uiPaint;
    DBG;
    if ( uiPrefs->uiPrefsPaintTools->value() )
    {
        uiPaint->uiMain->show();
    }
    else
        uiPaint->uiMain->hide();


    DBG;
    if ( uiPrefs->uiPrefsStereoOptions->value() )
    {
        main->uiStereo->uiMain->show();
    }
    else
        main->uiStereo->uiMain->hide();

    DBG;
    if ( uiPrefs->uiPrefsReelList->value() )
    {
        main->uiReelWindow->uiMain->show();
    }
    else
        main->uiReelWindow->uiMain->hide();

    DBG;
    mrv::ImageView* v = uiMain->uiView;
    if ( uiPrefs->uiPrefsImageInfo->value() )
        v->toggle_window( ImageView::kMediaInfo,
                          uiPrefs->uiPrefsImageInfo->value() );

    DBG;
    if ( uiPrefs->uiPrefsColorArea->value() )
        v->toggle_window( ImageView::kColorInfo,
                          uiPrefs->uiPrefsColorArea->value() );

    DBG;
    if ( uiPrefs->uiPrefsHistogram->value() )
        v->toggle_window( ImageView::kHistogram,
                          uiPrefs->uiPrefsHistogram->value() );

    DBG;
    if ( uiPrefs->uiPrefsVectorscope->value() )
        v->toggle_window( ImageView::kVectorscope,
                          uiPrefs->uiPrefsVectorscope->value() );

    DBG;
    if ( uiPrefs->uiPrefsWaveform->value() )
        v->toggle_window( ImageView::kWaveform,
                          uiPrefs->uiPrefsWaveform->value() );

    //
    // Toolbars
    //
    DBG;
    if ( uiPrefs->uiPrefsTopbar->value() )
    {
        main->uiTopBar->show();
    }
    else
    {
        main->uiTopBar->hide();
    }

    DBG;
    if ( uiPrefs->uiPrefsPixelToolbar->value() )
    {
        main->uiPixelBar->show();
    }
    else
    {
        main->uiPixelBar->hide();
    }

    DBG;
    if ( uiPrefs->uiPrefsTimeline->value() )
    {
        main->uiBottomBar->show();
    }
    else
    {
        main->uiBottomBar->hide();
    }

    main->uiView->resize_main_window();

    //
    // Widget/Viewer settings
    //
    mrv::ImageView* view = main->uiView;

    DBG;
    main->uiLoopMode->value( uiPrefs->uiPrefsLoopMode->value() );
    main->uiLoopMode->do_callback();

    DBG;
    main->uiGain->value( uiPrefs->uiPrefsViewGain->value() );
    main->uiGamma->value( uiPrefs->uiPrefsViewGamma->value() );


    DBG;
    main->uiPixelRatio->value( uiPrefs->uiPrefsViewPixelRatio->value() );
    if ( main->uiPixelRatio->value() )
        view->toggle_pixel_ratio();

    view->texture_filtering( ImageView::kNearestNeighbor );
    if ( main->uiPrefs->uiPrefsFiltering->value() ==
         ImageView::kBilinearFiltering )
        view->texture_filtering( ImageView::kBilinearFiltering );

    view->display_window( uiPrefs->uiPrefsViewDisplayWindow->value() );
    view->data_window( uiPrefs->uiPrefsViewDataWindow->value() );

    DBG;
    if ( uiPrefs->uiScrub->value() )
        view->scrub_mode();
    else if ( uiPrefs->uiMovePicture->value() )
        view->move_pic_mode();
    else if ( uiPrefs->uiSelection->value() )
        view->selection_mode();
    else if ( uiPrefs->uiDraw->value() )
        view->draw_mode();
    else if ( uiPrefs->uiText->value() )
        view->text_mode();
    else if ( uiPrefs->uiErase->value() )
        view->erase_mode();

    DBG;
    if ( !view->use_lut() )
    {
        bool use = uiPrefs->uiPrefsViewLut->value();
        if ( ! GLEngine::floatTextures() ) use = false;
        DBG;
        main->uiLUT->value( use );
        view->use_lut( use );
        DBG;
    }


        DBG;
    if ( uiPrefs->uiPrefsSafeAreas->value() )
        view->safe_areas(true);

        DBG;
    missing_frame = (MissingFrameType)uiPrefs->uiPrefsMissingFrames->value();

    //////////////////////////////////////////////////////
    // OCIO
    /////////////////////////////////////////////////////

        DBG;
    use_ocio = (bool) uiPrefs->uiPrefsUseOcio->value();

    const char* var = environmentSetting( "OCIO",
                                          uiPrefs->uiPrefsOCIOConfig->value(),
                                          true );

        DBG;
    std::string tmp = root + "/ocio/nuke-default/config.ocio";

    if (  ( !var || strlen(var) == 0 || tmp == var ) && use_ocio )
    {
        DBG;
        mrvLOG_INFO( "ocio",
                     _("Setting OCIO environment variable to nuke-default." )
                     << std::endl );
        var = strdup( tmp.c_str() );
    }
        DBG;
    if ( var && use_ocio && strlen(var) > 0 )
    {
        static std::string old_ocio;

        DBG;
        if ( old_ocio != var )
        {
        DBG;
            mrvLOG_INFO( "ocio", _("Setting OCIO environment variable to:")
                         << std::endl );
            old_ocio = var;
            mrvLOG_INFO( "ocio", old_ocio << std::endl );
        }

        char buf[2048];

        DBG;
        std::string parsed = expandVariables( var, "%", '%' );
        parsed = expandVariables( parsed, "${", '}' );
        if ( old_ocio != parsed )
        {
        DBG;
            mrvLOG_INFO( "ocio", _("Expanded OCIO environment variable to:")
                         << std::endl );
            mrvLOG_INFO( "ocio", parsed << std::endl );

        }

        DBG;
        sprintf( buf, "OCIO=%s", parsed.c_str() );
        putenv( strdup(buf) );
        DBG;
        uiPrefs->uiPrefsOCIOConfig->value( var );

// #ifdef __linux__
//         char tmpS[256];
//         sprintf( tmpS, "sRGB:rec709:Film:Log:Raw:None" );
//         const char* var = environmentSetting( "OCIO_ACTIVE_VIEWS", tmpS, true);
//         mrvLOG_INFO( "ocio", _("Setting OCIO's view environment variable to:")
//                      << std::endl );
//         sprintf( buf, "OCIO_ACTIVE_VIEWS=%s", var );
//         mrvLOG_INFO( "ocio", buf << std::endl );
//         putenv( strdup(buf) );
// #endif

        DBG;
        std::locale::global( std::locale("C") );
        setlocale( LC_NUMERIC, "C" );

        DBG;

        try
        {
            DBG;
            OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();

            uiPrefs->uiPrefsOCIOConfig->tooltip( config->getDescription() );

            DBG;
            OCIO_Display = config->getDefaultDisplay();

            OCIO_View = config->getDefaultView( OCIO_Display.c_str() );

            DBG;
            // First, remove all additional defaults if any from pulldown menu
            if ( use_ocio && !OCIO_View.empty() && !OCIO_Display.empty() )
            {
                main->gammaDefaults->clear();
            }
            DBG;


            int numDisplays = config->getNumDisplays();
            DBG;

            stringArray active_displays;
            const char* displaylist = config->getActiveDisplays();
            if ( displaylist )
            {
                mrv::split( active_displays, displaylist, ',' );

                // Eliminate forward spaces in names
                for ( int i = 0; i < active_displays.size(); ++i )
                {
                    while ( active_displays[i][0] == ' ' )
                        active_displays[i] =
                        active_displays[i].substr( 1, active_displays[i].size() );
                }
            }
            else
            {
                int num = config->getNumDisplays();
                for ( int i = 0; i < num; ++i )
                {
                    active_displays.push_back( config->getDisplay( i ) );
                }
            }

            stringArray active_views;
            const char* viewlist = config->getActiveViews();
            if ( viewlist )
            {
                mrv::split( active_views, viewlist, ',' );

                // Eliminate forward spaces in names
                for ( int i = 0; i < active_views.size(); ++i )
                {
                    while ( active_views[i][0] == ' ' )
                        active_views[i] =
                        active_views[i].substr( 1, active_views[i].size() );
                }
            }

            unsigned num_active_displays = active_displays.size();
            unsigned num_active_views = active_views.size();

            for ( int j = 0; j < num_active_displays; ++j )
            {
                std::string display = active_displays[j];
                DBG;

                int numViews = config->getNumViews(display.c_str());
                DBG;
                // Collect all views

                if ( num_active_views )
                {
                    for ( int h = 0; h < num_active_views; ++h )
                    {
                        std::string view;
                        bool add = false;

                        for (int i = 0; i < numViews; ++i)
                        {
                            view = config->getView(display.c_str(), i);
                            if ( active_views[h] == view )
                            {
                                add = true; break;
                            }
                        }

                        if ( add )
                        {
                            std::string name = display;
                            name += "/";
                            name += view;
                            main->gammaDefaults->add( name.c_str() );

                            if ( view == OCIO_View && !OCIO_View.empty() )
                            {
                                main->gammaDefaults->copy_label( view.c_str() );
                                main->uiGamma->value( 1.0f );
                                main->uiGammaInput->value( 1.0f );
                                main->uiView->gamma( 1.0f );
                            }
                        }
                    }
                }
                else
                {
                    for(int i = 0; i < numViews; i++)
                    {
                        std::string view = config->getView(display.c_str(), i);

                        std::string name = display;
                        name += "/";
                        name += view;
                        main->gammaDefaults->add( name.c_str() );

                        if ( view == OCIO_View && !OCIO_View.empty() )
                        {
                            main->gammaDefaults->copy_label( view.c_str() );
                            main->uiGamma->value( 1.0f );
                            main->uiGammaInput->value( 1.0f );
                            main->uiView->gamma( 1.0f );
                        }
                    }
                }
            }


            DBG;

            main->gammaDefaults->redraw();
        }
        catch( const OCIO::Exception& e )
        {
        DBG;
            LOG_ERROR( e.what() );
            use_ocio = false;
        }
        catch( const std::exception& e )
        {
        DBG;
            LOG_ERROR( e.what() );
            use_ocio = false;
        }

        DBG;
        std::locale::global( std::locale("") );
        setlocale(LC_NUMERIC, "" );
        DBG;
    }
    else
    {
        DBG;
        if ( !var || strlen(var) == 0 )
            LOG_INFO( _("OCIO environment variable is not set.  "
                        "Defaulting to CTL. ") );
        DBG;
        main->gammaDefaults->copy_label( _("Gamma") );
        DBG;
        use_ocio = false;
    }

    if ( use_ocio )
    {
        DBG3( "use_OCIO" );
        DBG;
        main->uiFstopGroup->hide();
        main->uiNormalize->hide();
        DBG;
        try
        {
        DBG;
            OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();
            std::vector< std::string > spaces;
            for(int i = 0; i < config->getNumColorSpaces(); ++i)
            {
        DBG;
                std::string csname = config->getColorSpaceNameByIndex(i);
                spaces.push_back( csname );
            }

        DBG;
            if ( std::find( spaces.begin(), spaces.end(),
                            OCIO::ROLE_SCENE_LINEAR ) == spaces.end() )
            {
                spaces.push_back( OCIO::ROLE_SCENE_LINEAR );
        DBG;
            }

            CMedia* img = NULL;
            mrv::media fg = main->uiView->foreground();
            if ( fg )
            {
        DBG;
                img = fg->image();
            }

        DBG;
            mrv::PopupMenu* w = main->uiICS;
            w->clear();
            std::sort( spaces.begin(), spaces.end() );
            for ( size_t i = 0; i < spaces.size(); ++i )
            {
                const char* space = spaces[i].c_str();
                OCIO::ConstColorSpaceRcPtr cs = config->getColorSpace( space );
                w->add( space );
        DBG;
                //w->child(i)->tooltip( strdup( cs->getDescription() ) );
                if ( img && img->ocio_input_color_space() == space )
                {
        DBG;
                    w->copy_label( space );
                    w->value( i );
                }
            }
            w->do_callback();
        DBG;
            w->redraw();
        }
        catch( const std::exception& e )
        {
            LOG_ERROR( e.what() );
        }
        DBG;
        main->uiICS->show();
    }
    else
    {
        DBG;
        main->uiICS->hide();
        DBG;
        main->uiFstopGroup->show();
        main->uiNormalize->show();
    }

    // Handle file loading
    CMedia::load_library = (CMedia::LoadLib)
                           uiPrefs->uiPrefsLoadLibrary->value();

        DBG;
    char buf[64];
    sprintf( buf, "%d", (int) uiPrefs->uiPrefsVideoThreadCount->value() );
    video_threads = buf;

    //
    // Handle file requester
    //
        DBG;
    Flu_File_Chooser::thumbnailsFileReq = (bool)
                                          uiPrefs->uiPrefsFileReqThumbnails->value();

        DBG;
    Flu_File_Chooser::singleButtonTravelDrawer = (bool)
            uiPrefs->uiPrefsFileReqFolder->value();

        DBG;
    native_file_chooser = uiPrefs->uiPrefsNativeFileChooser->value();

    // Handle caches
        DBG;
    CMedia::cache_active( (bool)uiPrefs->uiPrefsCacheActive->value() );
    CMedia::preload_cache( (bool)uiPrefs->uiPrefsPreloadCache->value() );

    int scale = CMedia::cache_scale();
    CMedia::cache_scale( uiPrefs->uiPrefsCacheScale->value() );

        DBG;
    if ( uiPrefs->uiPrefsCacheFPS->value() == 0 )
    {
        uiPrefs->uiPrefsCacheSize->activate();
        CMedia::audio_cache_size(uiPrefs->uiPrefsCacheSize->value());
        CMedia::video_cache_size(uiPrefs->uiPrefsCacheSize->value());
    }
    else
    {
        uiPrefs->uiPrefsCacheSize->deactivate();
        CMedia::audio_cache_size( 0 );
        CMedia::video_cache_size( 0 );
    }

    Preferences::max_memory = ( uiPrefs->uiPrefsCacheMemory->value() *
                                1000000000 );

        DBG;
    bool old = CMedia::eight_bit_caches();
    CMedia::eight_bit_caches( (bool) uiPrefs->uiPrefs8BitCaches->value() );
    if ( !CMedia::cache_active() || CMedia::eight_bit_caches() != old ||
            CMedia::cache_scale() != scale )
    {
        view->clear_caches();
    }



    //
    // Handle pixel values
    //
        DBG;
    main->uiAColorType->value( uiPrefs->uiPrefsPixelRGBA->value() );
    main->uiAColorType->redraw();
        DBG;
    main->uiAColorType->do_callback();
        DBG;
    main->uiPixelValue->value( uiPrefs->uiPrefsPixelValues->value() );
    main->uiPixelValue->redraw();
        DBG;
    main->uiPixelValue->do_callback();
        DBG;
    main->uiBColorType->value( uiPrefs->uiPrefsPixelHSV->value() );
    main->uiBColorType->redraw();
        DBG;
    main->uiBColorType->do_callback();
        DBG;
    main->uiLType->value( uiPrefs->uiPrefsPixelLumma->value() );
    main->uiLType->redraw();
        DBG;
    main->uiLType->do_callback();
        DBG;

    //
    // Handle crop area (masking)
    //
        DBG;
    int crop = uiPrefs->uiPrefsCropArea->value();
    if ( crop > 0 )
    {
        float mask = 1.0f;
        const char* fmt = uiPrefs->uiPrefsCropArea->child(crop)->label();
        sscanf( fmt, "%f", &mask );
        view->masking( mask );
    }

        DBG;
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

    if ( uiPrefs->uiPrefsHudMemory->value() )
        hud |= mrv::ImageView::kHudMemoryUse;

    if ( uiPrefs->uiPrefsHudAttributes->value() )
        hud |= mrv::ImageView::kHudAttributes;

        DBG;
    view->hud( (mrv::ImageView::HudDisplay) hud );


        DBG;
    main->uiTimecodeSwitch->value( uiPrefs->uiPrefsTimelineDisplay->value() );
        DBG;
    change_timeline_display(main);

    double mn = 1, mx = 50,
    dmn = main->uiTimeline->display_minimum(),
    dmx = main->uiTimeline->display_maximum();

    if ( !main->uiTimeline->edl() )
    {
        DBG;
        mrv::media fg = main->uiView->foreground();
        if ( fg )
        {
            Image_ptr img = fg->image();
            mn = img->first_frame();
            mx = img->last_frame();
        }
    }
    else
    {
        DBG;
       // edl
        mrv::Reel reel = main->uiReelWindow->uiBrowser->current_reel();
        if ( !reel || reel->images.size() == 0 ) return;
        DBG;

        mrv::media fg = reel->images[0];
        mrv::media last = reel->images[ reel->images.size()-1 ];
        if ( fg ) {
            mn = fg->position();
            mx = last->position() + last->duration() - 1;
        }
    }
    if ( uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
    {
        DBG;
       main->uiTimeline->minimum( dmn );
        main->uiTimeline->maximum( dmx );
    }
    else
    {
        DBG;
        main->uiTimeline->minimum( mn );
        main->uiTimeline->display_minimum( dmn );
        main->uiTimeline->maximum( mx );
        main->uiTimeline->display_maximum( dmx );
    }

        DBG;
    unsigned idx = uiPrefs->uiPrefsAudioDevice->value();
    mrv::AudioEngine::device( idx );
        DBG;

    if ( uiPrefs->uiPrefsOverrideAudio->value() )
    {
        double x = uiPrefs->uiPrefsAudioVolume->value();
        if ( uiPrefs->uiPrefsAudioMute->value() )
            x = 0.0;
        view->volume( float(x) );
    }
    DBG;

    //
    // Handle fullscreen and presentation mode
    //
    if ( uiPrefs->uiWindowFixedPosition->value() )
    {
        int x = int(uiPrefs->uiWindowXPosition->value());
        int y = int(uiPrefs->uiWindowYPosition->value());
        main->uiMain->position( x, y );
    }
    DBG;
    if ( uiPrefs->uiWindowFixedSize->value() )
    {
        int w = int(uiPrefs->uiWindowXSize->value());
        int h = int(uiPrefs->uiWindowYSize->value());
        main->uiMain->resize( main->uiMain->x(),
                              main->uiMain->y(),
                              w, h );
    }

    //
    // Handle FPS
    //
    CMedia::default_fps = uiPrefs->uiPrefsFPS->value();

    DBG;

#if defined(_WIN32) || defined(_WIN64)
    main->uiMain->resize(  main->uiMain->x(), main->uiMain->y(),
                           main->uiMain->w(), main->uiMain->h()-20 );
#endif

    DBG;
    Fl_Round_Button* r;
    r = (Fl_Round_Button*) uiPrefs->uiPrefsOpenMode->child(1);

    if ( r->value() == 1 )
    {
        DBG;
        // Fullscreen mode
        view->toggle_fullscreen();
    }

    DBG;
    r = (Fl_Round_Button*) uiPrefs->uiPrefsOpenMode->child(2);

    if ( r->value() == 1 )
    {
        // Go to presentation mode - window must be shown first, thou.
        DBG;
         view->toggle_presentation();
    }

    DBG;
    GLLut3d::NUM_STOPS = (unsigned) uiPrefs->uiPrefsNumStops->value();

    DBG;
    int num = (int)main->uiPrefs->uiPrefsOpenEXRThreadCount->value();
    Imf::setGlobalThreadCount( num );

    DBG;
    float tmpF = (float)main->uiPrefs->uiPrefsOpenEXRGamma->value();
    exrImage::_default_gamma = tmpF;

    DBG;
    num = main->uiPrefs->uiPrefsOpenEXRCompression->value();
    exrImage::_default_compression = (Imf::Compression) num;

    DBG;
    tmpF = (float) main->uiPrefs->uiPrefsOpenEXRDWACompression->value();
    exrImage::_default_dwa_compression = tmpF;

    DBG;
    bool b = (bool)main->uiPrefs->uiPrefsAllLayers->value();
    CMedia::all_layers( b );

    DBG;

    b = (bool)main->uiPrefs->uiPrefsACESClipMetadata->value();
    CMedia::aces_metadata( b );

    DBG;
    idx = main->uiPrefs->uiPrefsSubtitleFont->value();
    num = main->uiPrefs->uiPrefsSubtitleFont->children();
    if ( (int)idx < num )
    {    DBG;

        const char* font = main->uiPrefs->uiPrefsSubtitleFont->child(idx)->label();
        if ( font )
            CMedia::default_subtitle_font( font );
    }
    const char* enc = main->uiPrefs->uiPrefsSubtitleEncoding->value();
    DBG;
    if ( enc )
        CMedia::default_subtitle_encoding( enc );

    LogDisplay::prefs = (LogDisplay::ShowPreferences)
                        main->uiPrefs->uiPrefsRaiseLogWindowOnError->value();
    LogDisplay::shown = false;

    DBG;
    if ( main->uiPrefs->uiPrefsAlwaysOnTop->value() )
        main->uiMain->always_on_top();
    DBG;
}


void Preferences::save()
{
    int i;
    PreferencesUI* uiPrefs = ViewerUI::uiPrefs;


    Fl_Preferences base( prefspath().c_str(), "filmaura",
                            "mrViewer" );
    base.set( "version", 3 );

    // Save ui preferences
    Fl_Preferences ui( base, "ui" );

    //
    // window options
    //
    {
        Fl_Preferences win( ui, "window" );
        win.set( "auto_fit_image", (int) uiPrefs->uiPrefsAutoFitImage->value() );
        win.set( "always_on_top", (int) uiPrefs->uiPrefsAlwaysOnTop->value() );
        int tmp = 0;
        for ( i = 0; i < uiPrefs->uiPrefsOpenMode->children(); ++i ) {
            Fl_Round_Button* r = (Fl_Round_Button*) uiPrefs->uiPrefsOpenMode->child(i);
            if ( r->value() ) {
                tmp = i;
                break;
            }
        }
        win.set( "open_mode", tmp );
    }

    //
    // ui options
    //
    ui.set( "topbar", (int) uiPrefs->uiPrefsTopbar->value() );
    ui.set( "single_instance", (int) uiPrefs->uiPrefsSingleInstance->value() );
    ui.set( "pixel_toolbar", (int) uiPrefs->uiPrefsPixelToolbar->value() );
    ui.set( "timeline_toolbar", (int) uiPrefs->uiPrefsTimeline->value() );
    ui.set( "reel_list", (int) uiPrefs->uiPrefsReelList->value() );
    ui.set( "edl_edit", (int) uiPrefs->uiPrefsEDLEdit->value() );
    ui.set( "stereo3d_options", (int) uiPrefs->uiPrefsStereoOptions->value() );
    ui.set( "action_tools", (int) uiPrefs->uiPrefsPaintTools->value() );
    ui.set( "image_info", (int) uiPrefs->uiPrefsImageInfo->value() );
    ui.set( "color_area", (int) uiPrefs->uiPrefsColorArea->value() );
    ui.set( "histogram", (int) uiPrefs->uiPrefsHistogram->value() );
    ui.set( "vectorscope", (int) uiPrefs->uiPrefsVectorscope->value() );
    ui.set( "waveform", (int) uiPrefs->uiPrefsWaveform->value() );


    ui.set( "timeline_display",
            uiPrefs->uiPrefsTimelineDisplay->value() );

    //
    // ui/view prefs
    //
    Fl_Preferences view( ui, "view" );
    view.set("gain", uiPrefs->uiPrefsViewGain->value() );
    view.set("gamma", uiPrefs->uiPrefsViewGamma->value() );
    view.set("compensate_pixel_ratio", uiPrefs->uiPrefsViewPixelRatio->value() );
    view.set("display_window", uiPrefs->uiPrefsViewDisplayWindow->value() );
    view.set("data_window", uiPrefs->uiPrefsViewDataWindow->value() );

    view.set("lut", uiPrefs->uiPrefsViewLut->value() );
    view.set("safe_areas", uiPrefs->uiPrefsSafeAreas->value() );
    view.set("crop_area", uiPrefs->uiPrefsCropArea->value() );

    //
    // view/colors prefs
    //
    {
        Fl_Preferences colors( view, "colors" );
        int tmp = uiPrefs->uiPrefsViewBG->color();
        colors.set("background_color", tmp );
        tmp = uiPrefs->uiPrefsViewTextOverlay->color();
        colors.set("text_overlay_color", tmp );
        tmp = uiPrefs->uiPrefsViewSelection->color();
        colors.set("selection_color", tmp );
        tmp = uiPrefs->uiPrefsViewHud->color();
        colors.set("hud_color", tmp );
    }

    {
        Fl_Preferences ocio( view, "ocio" );
        int tmp = uiPrefs->uiPrefsUseOcio->value();
        ocio.set( "use_ocio", tmp );


        if ( uiPrefs->uiPrefsSaveOcio->value() )
        {
            ocio.set( "save_config", 1 );
            ocio.set( "config", uiPrefs->uiPrefsOCIOConfig->value() );
        }
        else
        {
            ocio.set( "save_config", 1 );
            ocio.set( "config", "" );
        }

        Fl_Preferences ics( ocio, "ICS" );
        {
            ics.set( "8bits",  uiPrefs->uiOCIO_8bits_ics->value() );
            ics.set( "16bits", uiPrefs->uiOCIO_16bits_ics->value() );
            ics.set( "32bits", uiPrefs->uiOCIO_32bits_ics->value() );
            ics.set( "float",  uiPrefs->uiOCIO_float_ics->value() );
        }

    }

    //
    // view/hud prefs
    //
    Fl_Preferences hud( view, "hud" );
    hud.set("filename", uiPrefs->uiPrefsHudFilename->value() );
    hud.set("directory", uiPrefs->uiPrefsHudDirectory->value() );
    hud.set("fps", uiPrefs->uiPrefsHudFPS->value() );
    hud.set("av_difference", uiPrefs->uiPrefsHudAVDifference->value() );
    hud.set("non_drop_timecode", uiPrefs->uiPrefsHudTimecode->value() );
    hud.set("frame", uiPrefs->uiPrefsHudFrame->value() );
    hud.set("resolution", uiPrefs->uiPrefsHudResolution->value() );
    hud.set("frame_range", uiPrefs->uiPrefsHudFrameRange->value() );
    hud.set("memory", uiPrefs->uiPrefsHudMemory->value() );
    hud.set("attributes", uiPrefs->uiPrefsHudAttributes->value() );

    {
        Fl_Preferences win( view, "window" );
        win.set("fixed_position", uiPrefs->uiWindowFixedPosition->value() );
        win.set("x_position", uiPrefs->uiWindowXPosition->value() );
        win.set("y_position", uiPrefs->uiWindowYPosition->value() );
        win.set("fixed_size", uiPrefs->uiWindowFixedSize->value() );
        win.set("x_size", uiPrefs->uiWindowXSize->value() );
        win.set("y_size", uiPrefs->uiWindowYSize->value() );
    }

    //
    // ui/colors prefs
    //
    Fl_Preferences colors( ui, "colors" );
    colors.set( "scheme", uiPrefs->uiScheme->text() );
    colors.set( "background_color", bgcolor );
    colors.set( "text_color", textcolor );
    colors.set( "selection_color", selectioncolor );
    colors.set( "selection_text_color", selectiontextcolor );
    colors.set( "theme", uiPrefs->uiColorTheme->text() );

    Fl_Preferences flu( ui, "file_requester" );
    flu.set("quick_folder_travel", uiPrefs->uiPrefsFileReqFolder->value());
    flu.set("thumbnails", uiPrefs->uiPrefsFileReqThumbnails->value());

    // @TODO: fltk1.4
    Flu_File_Chooser::singleButtonTravelDrawer =
        uiPrefs->uiPrefsFileReqFolder->value();
    Flu_File_Chooser::thumbnailsFileReq =
        uiPrefs->uiPrefsFileReqThumbnails->value();

    //
    // playback prefs
    //
    Fl_Preferences playback( base, "playback" );
    playback.set( "auto_playback", (int) uiPrefs->uiPrefsAutoPlayback->value() );
    playback.set( "play_all_frames",
                  (int) uiPrefs->uiPrefsPlayAllFrames->value() );
    playback.set( "override_fps", uiPrefs->uiPrefsOverrideFPS->value() );
    playback.set( "fps", uiPrefs->uiPrefsFPS->value() );
    playback.set( "loop_mode", uiPrefs->uiPrefsLoopMode->value() );
    playback.set( "scrubbing_sensitivity",
                  uiPrefs->uiPrefsScrubbingSensitivity->value() );
    playback.set( "selection_display_mode",
                  uiPrefs->uiPrefsTimelineSelectionDisplay->value() );

    Fl_Preferences pixel_toolbar( base, "pixel_toolbar" );
    pixel_toolbar.set( "RGBA_pixel", uiPrefs->uiPrefsPixelRGBA->value() );
    pixel_toolbar.set( "pixel_values", uiPrefs->uiPrefsPixelValues->value() );
    pixel_toolbar.set( "HSV_pixel", uiPrefs->uiPrefsPixelHSV->value() );
    pixel_toolbar.set( "Lumma_pixel", uiPrefs->uiPrefsPixelLumma->value() );


    Fl_Preferences action( base, "action" );

    action.set( "scrubbing", (int)uiPrefs->uiScrub->value() );
    action.set( "move_picture", (int)uiPrefs->uiMovePicture->value() );
    action.set( "color_area", (int)uiPrefs->uiSelection->value() );
    action.set( "pencil", (int)uiPrefs->uiDraw->value() );
    action.set( "text", (int) uiPrefs->uiText->value() );
    action.set( "eraser", (int)  uiPrefs->uiErase->value() );

    Fl_Preferences caches( base, "caches" );
    caches.set( "active", (int) uiPrefs->uiPrefsCacheActive->value() );
    caches.set( "preload", (int) uiPrefs->uiPrefsPreloadCache->value() );
    caches.set( "scale", (int) uiPrefs->uiPrefsCacheScale->value() );
    caches.set( "8bit_caches", (int) uiPrefs->uiPrefs8BitCaches->value() );
    caches.set( "fps", (int) uiPrefs->uiPrefsCacheFPS->value() );
    caches.set( "size", (int) uiPrefs->uiPrefsCacheSize->value() );

    caches.set( "cache_memory", (float)uiPrefs->uiPrefsCacheMemory->value() );

    Fl_Preferences loading( base, "loading" );
    loading.set( "load_library", uiPrefs->uiPrefsLoadLibrary->value() );
    loading.set( "missing_frames", uiPrefs->uiPrefsMissingFrames->value());
    loading.set( "drag_load_seq", (int) uiPrefs->uiPrefsLoadSequence->value() );
    loading.set( "file_assoc_load_seq",
                 (int) uiPrefs->uiPrefsLoadSequenceOnAssoc->value() );
    loading.set( "autoload_images",
                 (int) uiPrefs->uiPrefsAutoLoadImages->value() );
    loading.set( "native_file_chooser", (int) uiPrefs->uiPrefsNativeFileChooser->value() );
    loading.set( "thumbnail_percent",
                 uiPrefs->uiPrefsThumbnailPercent->value() );

    loading.set( "uses_16bits", (int) uiPrefs->uiPrefsUses16Bits->value() );
    loading.set( "image_version_prefix",
                 uiPrefs->uiPrefsImageVersionPrefix->value() );
    loading.set( "max_images_apart", uiPrefs->uiPrefsMaxImagesApart->value() );

    Fl_Preferences saving( base, "saving" );
    saving.set( "use_relative_paths", (int)
                uiPrefs->uiPrefsRelativePaths->value() );

    saving.set( "use_image_path", (int)
                uiPrefs->uiPrefsImagePathReelPath->value() );

    Fl_Preferences video( base, "video" );
    video.set( "filtering", (int) uiPrefs->uiPrefsFiltering->value() );
    video.set( "video_codec", (int) uiPrefs->uiPrefsVideoCodec->value() );
    video.set( "yuv_hint", (int) uiPrefs->uiPrefsYUVConversion->value() );
    video.set( "thread_count", (int) uiPrefs->uiPrefsVideoThreadCount->value());

    Fl_Preferences comp( base, "compositing" );
    comp.set( "blend_mode", (int) uiPrefs->uiPrefsBlendMode->value() );
    comp.set( "resize_bg", (int) uiPrefs->uiPrefsResizeBackground->value() );

    //
    // Audio prefs
    //
    Fl_Preferences audio( base, "audio" );
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


    audio.set( "override_audio", uiPrefs->uiPrefsOverrideAudio->value() );
    audio.set( "volume", uiPrefs->uiPrefsAudioVolume->value() );

    audio.set( "volume_mute", uiPrefs->uiPrefsAudioMute->value() );



    Fl_Preferences lut( base, "lut" );
    i = uiPrefs->uiLUT_quality->value();
    if ( i >= 0 && i < uiPrefs->uiLUT_quality->children() )
    {
        lut.set("quality", uiPrefs->uiLUT_quality->child(i)->label() );
    }

    lut.set( "number_stops", uiPrefs->uiPrefsNumStops->value() );

    {
        Fl_Preferences odt( lut, "ODT" );
        {
            odt.set( "algorithm", uiPrefs->ODT_algorithm->value() );
            Fl_Preferences ctl( odt, "CTL" );
            {
                ctl.set( "transform", uiPrefs->uiODT_CTL_transform->value() );

                Fl_Preferences chroma( ctl, "Chromaticities" );
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
            Fl_Preferences icc( odt, "ICC" );
            {
                icc.set( "profile",   uiPrefs->uiODT_ICC_profile->value() );
            }
        }

        Fl_Preferences  rt( lut, "RT" );
        {
            rt.set( "algorithm", uiPrefs->RT_algorithm->value() );

            Fl_Preferences ctl( rt, "CTL" );
            {
                ctl.set( "8bits",  uiPrefs->uiCTL_8bits_load_transform->value() );
                ctl.set( "16bits", uiPrefs->uiCTL_16bits_load_transform->value() );
                ctl.set( "32bits", uiPrefs->uiCTL_32bits_load_transform->value() );
                ctl.set( "float",  uiPrefs->uiCTL_float_load_transform->value() );
            }

            Fl_Preferences icc( rt, "ICC" );
            {
                icc.set( "8bits",  uiPrefs->uiICC_8bits_profile->value() );
                icc.set( "16bits", uiPrefs->uiICC_16bits_profile->value() );
                icc.set( "32bits", uiPrefs->uiICC_32bits_profile->value() );
                icc.set( "float",  uiPrefs->uiICC_float_profile->value() );
            }
        }

    }

    {
        Fl_Preferences subtitles( base, "subtitles" );
        int idx = uiPrefs->uiPrefsSubtitleFont->value();
        if ( idx >= 0 )
        {
            subtitles.set( "font",
                           uiPrefs->uiPrefsSubtitleFont->child(idx)->label() );
        }
        subtitles.set( "encoding",
                       uiPrefs->uiPrefsSubtitleEncoding->value() );
    }

    Fl_Preferences errors( base, "errors" );
    errors.set( "raise_log_window_on_error",
                uiPrefs->uiPrefsRaiseLogWindowOnError->value() );

    // Images
    Fl_Preferences images( base, "images" );
    images.set( "all_layers", (int) uiPrefs->uiPrefsAllLayers->value() );
    images.set( "aces_metadata",
                (int) uiPrefs->uiPrefsACESClipMetadata->value());

    // OpenEXR
    Fl_Preferences openexr( base, "openexr" );
    openexr.set( "thread_count", (int) uiPrefs->uiPrefsOpenEXRThreadCount->value() );
    openexr.set( "gamma", uiPrefs->uiPrefsOpenEXRGamma->value() );
    openexr.set( "compression",
                 (int) uiPrefs->uiPrefsOpenEXRCompression->value() );
    openexr.set( "dwa_compression",
                 uiPrefs->uiPrefsOpenEXRDWACompression->value() );

    //
    // Hotkeys
    //
    Fl_Preferences keys( base, "hotkeys" );
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
        keys.set( (hotkeys[i].name + " key2").c_str(),
                  (int)hotkeys[i].hotkey.key2 );
        keys.set( (hotkeys[i].name + " text").c_str(),
                  hotkeys[i].hotkey.text.c_str() );

    }

}


bool Preferences::set_transforms()
{

    // Set ui window settings
    PreferencesUI* uiPrefs = ViewerUI::uiPrefs;
    if (!uiPrefs) return true;

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
