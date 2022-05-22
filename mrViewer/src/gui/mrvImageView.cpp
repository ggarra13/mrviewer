/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later versioxn.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file   mrvImageView.cpp
 * @author gga
 * @date   Wed Sep 20 10:24:52 2006
 *
 * @brief  Class to handle an image viewer
 *
 *
 */

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64



// #define DEBUG_KEYS

#define NET(x) if ( Preferences::debug > 0 ) LOG_INFO( "RECV. COMMAND: " << x )

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <cmath>
#endif


#if defined(_WIN32) || defined(_WIN64)
#  include <float.h>
#  define isnan(x) _isnan(x)
#define isfinite(x) _finite(x)
#endif

#include <math.h>







#include <iostream>
#include <sstream>
#include <limits>
#include <iomanip>
#include <sstream>
#include <set>

#include "video/mrvGLLut3d.h"
#include "core/CMedia.h"
#include "core/aviImage.h"
#include "core/mrvColorOps.h"

#ifdef OSX
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <FL/names.h>
#include <FL/fl_utf8.h>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>

#include <FL/Fl_Output.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu.H>
#include "mrvPopupMenu.h"
#include <FL/Fl_Button.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Preferences.H>


#include <ImathMath.h> // for Math:: functions
#include <ImfRationalAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfStringAttribute.h>
#include <ImfStandardAttributes.h>

// CORE classes
#include "core/mrvClient.h"
#include "core/mrvColor.h"
#include "core/mrvI8N.h"
#include "core/mrvLicensing.h"
#include "core/mrvMath.h"
#include "core/mrvPlayback.h"
#include "core/mrvString.h"
#include "core/Sequence.h"
#include "core/stubImage.h"
#include "core/mrvThread.h"
#include "core/mrvColorSpaces.h"
#include "core/mrvFrame.h"
#include "core/mrvHome.h"
#include "core/mrStackTrace.h"
#include "core/exrImage.h"

// GUI classes
#include "gui/mrvColorInfo.h"
#include "gui/mrvFileRequester.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageInformation.h"
#include "gui/mrvIO.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvColorOps.h"
#include "gui/mrvMainWindow.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvHotkey.h"
#include "gui/mrvEvents.h"
#include "gui/mrvMultilineInput.h"
#include "mrvHotkeyUI.h"
#include "mrvEDLWindowUI.h"
#include "mrvWaveformUI.h"
#include "mrvVectorscopeUI.h"
#include "mrvHistogramUI.h"
#include "mrvImageInfo.h"
#include "mrvGl3dView.h"
#include "mrvAudioOffset.h"
#include "AboutUI.h"
#include "mrvReelUI.h"
#include "mrvIccProfileUI.h"
#include "gui/mrvFontsWindowUI.h"
#include "gui/mrvVersion.h"
#include "gui/mrvLogDisplay.h"
#include "gui/mrvElement.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageView_shapes.h"


#undef LOG
#define LOG(x) std::cerr << x << std::endl;

// Widgets
#include "mrViewer.h"
#include "mrvColorAreaUI.h"
#include "mrvPreferencesUI.h"

// Video
#include "video/mrvGLEngine.h"  // should be dynamically chosen from prefs

// Audio

// #define USE_TIMEOUT // USE TIMEOUTS INSTEAD OF IDLE CALLBACK

using namespace std;

namespace fs = boost::filesystem;

// FLTK2 currently has a problem on Linux with timout's Fl::event_x/y not
// taking into account other child widgets.  This works around it.
//#ifndef _WIN32
#  define FLTK_TIMEOUT_EVENT_BUG 1
//#endif



namespace
{
const char* kModule = "gui";
}

const int kMASK_MENU_OFFSET = 43;
float kCrops[] = {
    0.00f, 1.00f, 1.19f, 1.37f, 1.50f, 1.56f, 1.66f, 1.77f, 1.85f, 2.00f,
    2.10f, 2.20f, 2.35f, 2.39f, 4.00f
};

struct ChannelShortcuts
{
    const char* channel;
    const char  key;
};


ChannelShortcuts shortcuts[] = {
    { _("Color"), 'c'         },
    { N_("left"),  'c'         },
    { N_("right"), 'c'         },
    { _("RGBA"),  'c'         },
    { _("RGB"),   'c'         },
    { _("Red"),   'r'         },
    { _("Green"), 'g'         },
    { _("Blue"),  'b'         },
    { _("Alpha"), 'a'         },
    { _("Alpha Overlay"), 'v' },
    { _("Lumma"), 'l' },
    { N_("N"), 'n' },
    { _("Normals"), 'n' },
    { N_("Z"), 'z' },
    { _("Z Depth"), 'z' },
};




namespace
{
    bool FullScreen = false;
    bool presentation = false;
  uint64_t cursor_counter = 0;
    bool has_tools_grp, has_menu_bar,
        has_top_bar, has_bottom_bar, has_pixel_bar;


    void exit_cb( Fl_Widget* o, ViewerUI* main )
    {
        delete main;
        exit(0);
        // // Close all windows
        // typedef std::vector< Fl_Window* > WindowList;
        // WindowList list;
        // Fl_Window* w = Fl::first_window();
        // for ( ; w ; w = Fl::next_window(w) )
        // {
        //     list.push_back(w);
        // }
        // WindowList::iterator i = list.begin();
        // WindowList::iterator e = list.end();
        // for ( ; i != e; ++i )
        // {
        //     (*i)->hide();
        // }
    }

inline std::string remove_hash_number( std::string& r )
{
    if ( r.empty() || r[0] != '#' ) return r;

    size_t pos = r.find( ' ' );
    if ( pos != std::string::npos )
    {
        r = r.substr( pos+1, r.size() );
    }
    return r;
}

inline std::string extract_root( std::string& r )
{
    if ( r.empty() ) return r;

    size_t pos = r.find( '.' );
    if ( pos != std::string::npos )
    {
        r = r.substr( 0, pos );
    }
    return r;
}


/**
 * Get a shortcut for a channel by looking at the channel mappings
 *
 * @param channel name of the channel (Color, Z Depth, etc)
 *
 * @return a short value representing the fltk shortcut.
 */
short get_shortcut( const char* channel )
{
    static std::string oldChannel;
    for ( unsigned int i = 0;
          i < sizeof(shortcuts)/sizeof(ChannelShortcuts); ++i )
    {
        if ( strcmp( _(shortcuts[i].channel), channel ) == 0 )
        {
            oldChannel = channel;
            return shortcuts[i].key;
        }
    }

    std::string channelName = channel;
    std::string root = remove_hash_number( channelName );
    root = extract_root( root );
    std::transform( root.begin(), root.end(), root.begin(),
                    (int(*)(int))toupper );

    //
    // Find last .
    //
    size_t pos  = channelName.rfind( '.' );

    if ( pos != std::string::npos && pos != channelName.size() )
    {
        size_t pos2  = oldChannel.rfind( '.' );

        std::string ext2;
        if ( pos2 != std::string::npos && pos2 != oldChannel.size() )
        {
            ext2 = oldChannel.substr( pos2+1, oldChannel.size() );
            std::transform( ext2.begin(), ext2.end(), ext2.begin(),
                            (int(*)(int)) toupper );
        }

        std::string ext = channelName.substr( pos+1, channelName.size() );
        std::transform( ext.begin(), ext.end(), ext.begin(),
                        (int(*)(int)) toupper );

        oldChannel = channelName;

        if ( ext == _("COLOR") || ext == N_("RGB") || ext == N_("RGBA"))
            return 'c';
        else if ( ext == N_("X") || ext == N_("U") || ext == N_("R") ||
                  ext == _("RED") ) return 'r';
        else if ( ext == N_("Y") || ext == N_("V") || ext == N_("G") ||
                  ext == _("GREEN") ) return 'g';
        else if ( ext == N_("B") || ext == _("BLUE") || ext == N_("W") ||
                  ( ext == N_("Z") &&
                    ( root.find("RGB") == std::string::npos ) ) )
            return 'b';
        else if ( ext == N_("A") || ext == _("ALPHA") ) return 'a';
        else if ( ext == N_("Z") || ext == _("Z DEPTH") ) return 'z';
        else {
            return 0;
        }
    }
    else
    {
        oldChannel = channelName;

        std::string ext = channel;
        std::transform( ext.begin(), ext.end(), ext.begin(),
                        (int(*)(int)) toupper );
        if ( ext.rfind( N_("LEFT") ) != std::string::npos ||
                ext.rfind( N_("RIGHT") ) != std::string::npos )
            return 'c';
        else if ( root == N_("N") ) return 'n';
        else if ( root == N_("Z") ) return 'z';
    }

    return 0;
}


}

extern void clone_all_cb( Fl_Widget* o, mrv::ImageBrowser* b );
extern void clone_image_cb( Fl_Widget* o, mrv::ImageBrowser* b );
extern void attach_ocio_ics_cb( Fl_Widget* o, mrv::ImageBrowser* v );

void edit_audio_cb( Fl_Widget* o, mrv::ImageView* v )
{
    edit_audio_window( o, v );
}

void preload_image_cache_cb( Fl_Widget* o, mrv::ImageView* v )
{
    v->preload_caches();
}

void clear_image_cache_cb( Fl_Widget* o, mrv::ImageView* v )
{
    v->clear_caches();
}

void rotate_plus_90_cb( Fl_Widget* o, mrv::ImageView* v )
{
    mrv::media fg = v->foreground();
    if (!fg) return;

    mrv::CMedia* img = fg->image();
    img->rotate( -90.0 ); // this is reversed on purpose
    img->image_damage( mrv::CMedia::kDamageContents );
    char buf[128];
    sprintf( buf, "Rotate %g", img->rot_z() );
    v->send_network( buf );
    v->fit_image();
    v->redraw();
}


void toggle_ics_cb( Fl_Widget* o, ViewerUI* main )
{
    if ( mrv::Preferences::use_ocio == false ) return;

    if ( main->uiICS->visible() )
    {
        main->uiICS->hide();
        main->uiFstopGroup->show();
        main->uiNormalize->show();
    }
    else
    {
        main->uiFstopGroup->hide();
        main->uiNormalize->hide();
        main->uiICS->show();
    }
}

void rotate_minus_90_cb( Fl_Widget* o, mrv::ImageView* v )
{
    mrv::media fg = v->foreground();
    if (!fg) return;

    mrv::CMedia* img = fg->image();
    img->rotate( 90.0 );  // this is reversed on purpose
    img->image_damage( mrv::CMedia::kDamageContents );
    char buf[128];
    sprintf( buf, "Rotate %g", img->rot_z() );
    v->send_network( buf );
    v->fit_image();
    v->redraw();
}

void update_frame_cb( Fl_Widget* o, mrv::ImageView* v )
{
    mrv::media fg = v->foreground();
    if (!fg) return;

    mrv::CMedia* img = fg->image();
    img->has_changed();
    v->redraw();
}

extern void first_image_version_cb( Fl_Widget* o, mrv::ImageBrowser* b );
extern void last_image_version_cb( Fl_Widget* o, mrv::ImageBrowser* b );

void next_image_version_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    b->next_image_version();
}

void previous_image_version_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    b->previous_image_version();
}

void next_image_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    b->next_image();
}

void previous_image_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    b->previous_image();
}

void next_image_limited_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    b->next_image_limited();
}

void previous_image_limited_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
    b->previous_image_limited();
}

void next_channel_cb( Fl_Widget* o, mrv::ImageView* v )
{
    v->next_channel();
}

void previous_channel_cb( Fl_Widget* o, mrv::ImageView* v )
{
    v->previous_channel();
}

void switch_channels_cb( Fl_Widget* o, mrv::ImageView* v )
{
    v->switch_channels();
}

void set_as_background_cb( Fl_Widget* o, mrv::ImageView* view )
{
    view->browser()->change_background();
}

static void update_title_bar( mrv::ImageView* view )
{
    // Print out image names in reverse
    if ( view == NULL ) return;

    mrv::media fg = view->foreground();
    mrv::media bg = view->background();


    char bufs[256];

    if ( fg && bg && fg != bg )
    {
        snprintf( bufs, 255, _("mrViewer    FG: %s [%d]   BG: %s [%d] (%s)"),
                  fg->image()->name().c_str(), view->fg_reel(),
                  bg->image()->name().c_str(), view->bg_reel(),
                  view->show_background() ? _("Comped") : _("Not Comped") );
    }
    else if ( fg )
    {
        snprintf( bufs, 255, _("mrViewer    FG: %s"),
                  fg->image()->name().c_str() );
    }
    else
    {
        snprintf( bufs, 32, "%s", N_("mrViewer") );
    }

    view->main()->uiMain->copy_label( bufs );
}

void switch_fg_bg_cb( Fl_Widget* o, mrv::ImageView* view )
{
    size_t fg_reel = view->fg_reel();
    size_t bg_reel = view->bg_reel();

    mrv::media fg = view->foreground();
    if ( !fg ) {
        LOG_ERROR( _("No foreground image to switch to") );
        return;
    }

    mrv::media bg = view->background();
    if ( !bg ) {
        LOG_ERROR( _("No background image to switch to") );
        return;
    }

    if ( fg_reel == bg_reel && fg == bg && fg_reel >= 0 )
    {
        mrv::Reel r = view->browser()->reel_at( fg_reel );
        size_t num = r->images.size();
        if ( num == 2 )
        {
            bg = r->images[0];
            if ( r->images[0] == fg ) bg = r->images[1];
        }
        else if ( num > 2 )
        {
            for ( size_t i = 0; i < num - 1; ++i )
            {
                if ( r->images[i] == fg ) {
                    bg = r->images[i+1]; break;
                }
            }
        }
    }

    if ( fg == bg )
    {
        LOG_INFO( _("Cannot switch images.  Foreground and background image are the same.") );
        return;
    }

    view->background( fg );
    view->bg_reel( fg_reel );
    Fl_Tree_Item* item = view->browser()->media_to_item( fg );
    if ( item )
    {
        mrv::Element* elem = (mrv::Element*) item->widget();
        elem->Label()->box( FL_PLASTIC_DOWN_BOX );
        elem->Label()->color( FL_YELLOW );
        elem->redraw();
    }

    view->foreground( bg );
    view->fg_reel( bg_reel );
    item = view->browser()->media_to_item( bg );
    view->browser()->Fl_Tree::deselect_all( NULL, 0 );
    if ( item )
    {
        mrv::Element* elem = (mrv::Element*) item->widget();
        elem->Label()->box( FL_NO_BOX );
        view->browser()->Fl_Tree::select( item, 0 );
        elem->redraw();
    }


    ViewerUI* m = view->main();
    mrv::Timeline* t = m->uiTimeline;
    mrv::CMedia* img = bg->image();

    int64_t f = img->frame();
#if 1
    if ( fg_reel >= 0 )
    {
        mrv::ImageBrowser* b = view->browser();
        mrv::Reel r = b->reel_at( fg_reel );
        if ( r )
        {
            if ( r->edl )
            {
                b->seek( fg->position() );
            }
            else
            {
                b->seek( f );
            }
        }
        b->redraw();
    }
#endif


    m->uiStartFrame->value( img->first_frame() );
    m->uiEndFrame->value( img->last_frame() );
    t->minimum( img->first_frame() );
    t->maximum( img->last_frame() );

    update_title_bar( view );
    view->fit_image();
}

void toggle_edl_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    mrv::ImageBrowser* b = view->browser();
    if ( !b ) return;

    b->toggle_edl();
}

void toggle_background_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;
    view->toggle_background();
}

void open_session_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow )
{
    uiReelWindow->open_session();
}

void open_dir_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow )
{
    uiReelWindow->open_directory();
}

void open_amf_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow )
{
    uiReelWindow->open_amf();
}

void open_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow )
{
    uiReelWindow->open();
}

void open_single_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow )
{
    uiReelWindow->open_single();
}

void open_clip_xml_metadata_cb( Fl_Widget* o,
                                mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    mrv::CMedia* img = fg->image();
    read_clip_xml_metadata( img );
}

void open_stereo_cb( Fl_Widget* o,
                     mrv::ImageBrowser* uiReelWindow )
{
    uiReelWindow->open_stereo();
}


void save_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    view->browser()->save();
}

void save_session_as_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    view->browser()->save_session();
}

void save_reel_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    view->browser()->save_reel();
}

void save_snap_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    view->stop();


    mrv::CMedia* img = fg->image();
    if ( !img ) return;

    if ( view->action_mode() & mrv::ImageView::kSelection )
    {
        // We will saving opengl, make sure selection is not drawn.
        view->scrub_mode();
        mrv::Rectd r( 0, 0, 0, 0 );
        view->selection( r );
        view->redraw();
    }

    mrv::save_sequence_file( view->main(), NULL, true );

    // Return all to normal
    view->redraw();

}

void save_sequence_cb( Fl_Widget* o, mrv::ImageView* view )
{
    view->stop();
    view->browser()->save_sequence();
}


void masking_cb( mrv::PopupMenu* menu, ViewerUI* uiMain )
{
    mrv::ImageView* view = uiMain->uiView;

    const char* tmp;
    int i;
    int num = uiMain->uiPrefs->uiPrefsCropArea->children();
    for ( i = 0; i < num; ++i )
    {
        tmp = uiMain->uiPrefs->uiPrefsCropArea->child(i)->label();
        if ( !tmp ) continue;
        if ( strcmp( tmp, menu->mvalue()->label() ) == 0 )
            break;
    }
    if ( i == num ) return;  // should never happen

    float mask = kCrops[i];

    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    char buf[128];
    sprintf( buf, "Mask %g", mask );
    view->send_network( buf );

    view->restore_locale( oldloc );

    view->masking( mask );


    view->redraw();
}


void change_video_cb( mrv::PopupMenu* menu, mrv::ImageView* view )
{
    const Fl_Menu_Item* p = menu->mvalue();
    if ( !p ) {
        return;
    }
    if ( !view) return;

    const char* lbl = p->label();
    char buf[128];
    int i;
    // Label format is:
    // Track #i - eng.
    int num = sscanf( lbl, "%s %c%d", buf, buf, &i);
    if ( num < 3 )
    {
        LOG_ERROR( "sscanf returned less than 3 elements" );
        return;
    }

    mrv::media fg = view->foreground();
    if ( !fg ) return;

    fg->image()->video_stream(i);
}


void change_subtitle_cb( mrv::PopupMenu* menu, mrv::ImageView* view )
{
    const Fl_Menu_Item* p = menu->mvalue();
    if ( !p ) {
        return;
    }

    if ( !view) return;

    mrv::media fg = view->foreground();
    if ( !fg ) return;

    const char* lbl = p->label();
    char buf[128];
    int i = -1;
    // Label format is:
    // No Subtitle
    // or
    // Track #i - eng.
    int num = sscanf( lbl, "%s %c%d", buf, buf, &i);
    if ( num < 3 )
    {
        i = -1;
    }

    mrv::CMedia* img = fg->image();
    mrv::CMedia::Playback play = img->playback();
    img->stop();
    img->subtitle_stream(i);
    if ( play != mrv::CMedia::kStopped )
        img->play( play, view->main(), true );
}

static Fl_Window* gridWindow = NULL;

void change_grid_size_cb( Fl_Spinner* o, mrv::ImageView* view )
{
    view->grid_size( o->value() );
}

void grid_size_cb( Fl_Widget* o, mrv::ImageView* view )
{
    if ( ! view->grid() ) return;
    if ( ! gridWindow || !gridWindow->visible() )
    {
        if ( gridWindow ) delete gridWindow;

        Fl_Group::current( view );
        gridWindow = new Fl_Window(220, 60, _("Grid Size") );
        Fl_Spinner* o = new Fl_Spinner( 50, 20, 100, 20,
                                        _("Grid Size") );
        o->align( FL_ALIGN_TOP );
        o->textcolor( FL_BLACK );
        o->value( view->grid_size() );
        o->step( 1.0 );
        o->minimum( 1.0 );
        o->maximum( 1920.0 );
        o->callback( (Fl_Callback*)change_grid_size_cb, view );
        gridWindow->end();
        gridWindow->show();
    }
}

void grid_toggle_cb( Fl_Widget* o, mrv::ImageView* view )
{
    view->grid( !view->grid() );
}

void hud_toggle_cb( Fl_Widget* o, ViewerUI* uiMain )
{
    mrv::ImageView* view = uiMain->uiView;

    static mrv::ImageView::HudDisplay hud_save = mrv::ImageView::kHudNone;
    mrv::ImageView::HudDisplay hud;

    if ( hud_save == mrv::ImageView::kHudNone )
    {
        hud_save = view->hud();
        hud = mrv::ImageView::kHudNone;
    }
    else
    {
        hud = hud_save;
        hud_save = mrv::ImageView::kHudNone;
    }

    view->hud( hud );
    view->redraw();
}

void hud_cb( mrv::PopupMenu* o, ViewerUI* uiMain )
{
    const Fl_Menu_Item* item = o->mvalue();
    mrv::ImageView* view = uiMain->uiView;

    if ( item->label() == NULL )
    {
        LOG_ERROR( "Label is null" );
        return;
    }

    int i;
    Fl_Group* menu = uiMain->uiPrefs->uiPrefsHud;
    int num = menu->children();
    for ( i = 0; i < num; ++i )
    {
        const char* fmt = menu->child(i)->label();
        if (!fmt) continue;
        if ( strcmp( fmt, item->label() ) == 0 ) break;
    }

    unsigned int hud = view->hud();
    hud ^= ( 1 << i );
    view->hud( (mrv::ImageView::HudDisplay) hud );
    view->redraw();
}


void texture_filtering_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::ImageView::TextureFiltering t = (mrv::ImageView::TextureFiltering)
        view->main()->uiPrefs->uiPrefsFiltering->value();
    if ( t == mrv::ImageView::kPresentationOnly )
    {
        bool presentation = view->in_presentation();
        if ( presentation )
        {
            if (view->texture_filtering() <= mrv::ImageView::kBilinearFiltering)
                view->texture_filtering( mrv::ImageView::kNearestNeighbor );
            else
                view->texture_filtering( mrv::ImageView::kBilinearFiltering );
            view->redraw();
            return;
        }
    }
    if ( view->texture_filtering() <= mrv::ImageView::kBilinearFiltering )
        view->texture_filtering( mrv::ImageView::kNearestNeighbor );
    else
        view->texture_filtering( mrv::ImageView::kBilinearFiltering );
    view->redraw();
}

void safe_areas_cb( Fl_Widget* o, mrv::ImageView* view )
{
    view->safe_areas( !view->safe_areas() );
    view->redraw();
}


void display_window_cb( Fl_Widget* o, mrv::ImageView* view )
{
    view->display_window( !view->display_window() );
    view->redraw();
}

void data_window_cb( Fl_Widget* o, mrv::ImageView* view )
{
    view->data_window( !view->data_window() );
    view->redraw();
}

static const float kMinZoom = 0.01f;  // Zoom 1/64
static const float kMaxZoom = 96.f;   // Zoom 96x



namespace mrv {


static void attach_ctl_script_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    CMedia* img = fg->image();
    const char* script = "";
    if ( img->rendering_transform() ) script = img->rendering_transform();

    attach_ctl_script( fg->image(), script, view->main() );
}

    static void modify_sop_sat( mrv::ImageView* view )
    {
        mrv::media fg = view->foreground();
        if ( ! fg ) return;

        mrv::CMedia* img = fg->image();
        size_t num = img->number_of_lmts();
        size_t num_graderefs = img->number_of_grade_refs();
        if ( num_graderefs == 0 )
        {
            attach_look_mod_transform( img, "ACEScsc.ACES_to_ACEScg", 0,
                                       view->main() );
            attach_look_mod_transform( img, "LMT.SOPNode", 1, view->main() );
            attach_look_mod_transform( img, "LMT.SatNode", 2, view->main() );
            attach_look_mod_transform( img, "ACEScsc.ACEScg_to_ACES", 3,
                                       view->main() );
        }

        if ( ! img->rendering_transform() )
        {
            attach_rt_script( img, "RRT", view->main() );
        }
        if ( mrv::Preferences::ODT_CTL_transform == "" )
        {
            mrv::Preferences::ODT_CTL_transform =
                "ODT.Academy.RGBmonitor_D60sim_100nits_dim";
        }

        ViewerUI* main = view->main();
        if ( main->uiSOPNode )
        {
            main->uiSOPNode->media( fg );
            main->uiSOPNode->uiMain->show();
        }
        else
        {
            main->uiSOPNode = new SopNode( view );
        }
        img->image_damage( img->image_damage() | mrv::CMedia::kDamageAll );
        view->use_lut( true );
        view->redraw();
    }

    void modify_sop_sat_cb( Fl_Widget* o, mrv::ImageView* view )
    {
        mrv::media fg = view->foreground();
        if ( ! fg ) return;

        modify_sop_sat( view );
    }

    void static_handle_dnd( mrv::ImageBrowser* b );

    void window_cb( mrv::PopupMenu* m, const ViewerUI* uiMain )
    {
        int idx = -1;
        const Fl_Menu_Item* o = m->child( m->value() );
        mrv::PopupMenu* g = uiMain->uiWindows;
        for ( unsigned i = 0; i < g->children(); ++i )
        {
            if (!o->label() || !g->child(i)->label() ) continue;
            if ( strcasecmp( o->label(),  g->child(i)->label() ) == 0 ) {
                idx = i;
                break;
            }
        }

        if ( idx == -1 )
        {
            const char* name = o->label();
            LOG_ERROR( _("Unknown Window \"") << name << "\"" );
            return;
        }


        uiMain->uiView->toggle_window( (ImageView::WindowList)idx, true );
}


void ImageView::clear_old()
{
    if ( _engine ) _engine->clear_old();
}


void ImageView::restore_locale( char* loc ) const
{
    setlocale( LC_NUMERIC, loc );
    av_free( loc );
}

    void toggle_action_tool_dock(Fl_Widget* w, ViewerUI* uiMain)
    {
        if ( uiMain->uiToolsGroup->visible() )
        {
            uiMain->uiToolsGroup->hide();
        }
        else
        {
            uiMain->uiToolsGroup->show();
            uiMain->uiToolsGroup->size( 45, 433 );
        }
        uiMain->uiViewGroup->init_sizes();
        uiMain->uiViewGroup->layout();
        uiMain->uiViewGroup->redraw();
    }

void ImageView::toggle_window( const ImageView::WindowList idx, const bool force )
{
    Fl_Group::current(0);
    if ( idx == kReelWindow )
    {
        // Reel window
        if ( force || !uiMain->uiReelWindow->uiMain->visible() )
        {
            uiMain->uiReelWindow->uiMain->show();
            mrv::media bg = background();
            if ( bg )
            {
                uiMain->uiReelWindow->uiBGButton->value(1);
            }
            else
            {
                uiMain->uiReelWindow->uiBGButton->value(0);
            }
        }
        else
            uiMain->uiReelWindow->uiMain->hide();
    }
    else if ( idx == kMediaInfo )
    {
        // Media Info
        if ( force || !uiMain->uiImageInfo->uiMain->visible() )
        {
            uiMain->uiImageInfo->uiMain->show();
            update_image_info();
            send_network( "MediaInfoWindow 1" );
        }
        else
        {
            uiMain->uiImageInfo->uiMain->hide();
            send_network( "MediaInfoWindow 0" );
        }
    }
    else if ( idx == kColorInfo )
    {
        if ( force || !uiMain->uiColorArea->uiMain->visible() )
        {
            // Color Area
            uiMain->uiColorArea->uiMain->show();
            update_color_info();
            send_network( "ColorInfoWindow 1" );
        }
        else
        {
            uiMain->uiColorArea->uiMain->hide();
            send_network( "ColorInfoWindow 0" );
        }
    }
    else if ( idx == kColorControls )
    {
        if ( force || !uiMain->uiColorControls->uiMain->visible() )
        {
            // Color Area
            uiMain->uiColorControls->uiMain->show();
            send_network( "ColorControlWindow 1" );
        }
        else
        {
            uiMain->uiColorControls->uiMain->hide();
            send_network( "ColorControlWindow 0" );
        }
    }
    else if ( idx == k3DStereoOptions )
    {
        if ( force || !uiMain->uiStereo->uiMain->visible() )
        {
            uiMain->uiStereo->uiMain->show();
            send_network( "StereoOptions 1" );
        }
        else
        {
            uiMain->uiStereo->uiMain->hide();
            send_network( "StereoOptions 0" );
        }
    }
    else if ( idx == kEDLEdit )
    {
        if ( force || !uiMain->uiEDLWindow->uiMain->visible() )
        {
            uiMain->uiReelWindow->uiBrowser->set_edl();
            uiMain->uiEDLWindow->uiMain->show();
        }
        else
        {
            uiMain->uiEDLWindow->uiMain->hide();
        }
    }
    else if ( idx == k3dView )
    {
        if ( force || !uiMain->uiGL3dView->uiMain->visible() )
        {
            uiMain->uiGL3dView->uiMain->show();
            send_network( "GL3dView 1" );
        }
        else
        {
            uiMain->uiGL3dView->uiMain->hide();
            send_network( "GL3dView 0" );
        }
    }
    else if ( idx == kActionTools )
    {
        if ( force || !uiMain->uiPaint->uiMain->visible() )
        {
            // Paint Tools
            uiMain->uiPaint->uiMain->show();
            send_network( "PaintTools 1" );
        }
        else
        {
            uiMain->uiPaint->uiMain->hide();
            send_network( "PaintTools 0" );
        }
    }
    else if ( idx == kHistogram )
    {
        if ( force || !uiMain->uiHistogram->uiMain->visible() )
        {
            uiMain->uiHistogram->uiMain->show();
            uiMain->uiView->send_network( "HistogramWindow 1" );
        }
        else
        {
            uiMain->uiHistogram->uiMain->hide();
            uiMain->uiView->send_network( "HistogramWindow 0" );
        }
    }
    else if ( idx == kVectorscope )
    {
        if ( force || !uiMain->uiVectorscope->uiMain->visible() )
        {
            uiMain->uiVectorscope->uiMain->show();
            uiMain->uiView->send_network( "VectorscopeWindow 1" );
        }
        else
        {
            uiMain->uiVectorscope->uiMain->hide();
            uiMain->uiView->send_network( "VectorscopeWindow 0" );
        }
    }
    else if ( idx == kWaveform )
    {
        if ( force || !uiMain->uiWaveform->uiMain->visible() )
        {
            uiMain->uiWaveform->uiMain->show();
            uiMain->uiView->send_network( "WaveformWindow 1" );
        }
        else
        {
            uiMain->uiWaveform->uiMain->hide();
            uiMain->uiView->send_network( "WaveformWindow 0" );
        }
    }
    else if ( idx == kConnections )
    {
        if ( force || !uiMain->uiConnection->uiMain->visible() )
        {
            uiMain->uiConnection->uiMain->show();
        }
        else
        {
            uiMain->uiConnection->uiMain->hide();
        }
    }
    else if ( idx == kICCProfiles )
    {
        if ( force || !uiMain->uiICCProfiles->uiMain->visible() )
        {
            uiMain->uiICCProfiles->uiMain->show();
        }
        else
        {
            uiMain->uiICCProfiles->uiMain->hide();
        }
    }
    else if ( idx == kPreferences )
    {
        if ( force || !uiMain->uiPrefs->uiMain->visible() )
        {
            uiMain->uiPrefs->uiMain->show();
        }
        else
        {
            uiMain->uiPrefs->uiMain->hide();
        }
    }
    else if ( idx == kHotkeys )
    {
        if ( force || !uiMain->uiHotkey->uiMain->visible() )
        {
            uiMain->uiHotkey->uiMain->show();
        }
        else
        {
            uiMain->uiHotkey->uiMain->hide();
        }
    }
    else if ( idx == kLogs )
    {
        if ( force || !uiMain->uiLog->uiMain->visible() )
        {
            uiMain->uiLog->uiMain->show();
        }
        else
        {
            uiMain->uiLog->uiMain->hide();
        }
    }
    else if ( idx == kAbout )
    {
        if ( force || !uiMain->uiAbout->uiMain->visible() )
        {
            uiMain->uiAbout->uiMain->show();
        }
        else
        {
            uiMain->uiAbout->uiMain->hide();
        }
    }
    else
    {
        LOG_ERROR( _("Unknown Window ") << idx );
    }
}


static void attach_color_profile_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    attach_icc_profile( fg->image() );
}




void load_subtitle_cb( Fl_Widget* o, ViewerUI* uiMain )
{
    std::string file = open_subtitle_file( NULL, uiMain );
    if ( file.empty() ) return;

    mrv::ImageView* view = uiMain->uiView;

    mrv::media fg = view->foreground();
    if ( !fg ) return;


    aviImage* img = dynamic_cast< aviImage* >( fg->image() );
    if ( !img )
    {
        LOG_ERROR( _("Subtitles are only valid on video files") );
        return;
    }

    img->subtitle_file( file.c_str() );
}

static void flip_x_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    CMedia* img = fg->image();
    img->flipX( !img->flipX() );
    view->update_color_info();
    view->redraw();
}

static void flip_y_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    CMedia* img = fg->image();
    img->flipY( !img->flipY() );
    view->update_color_info();
    view->redraw();
}

static void attach_ctl_lmt_script_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    CMedia* img = fg->image();

    attach_ctl_lmt_script( img, img->number_of_lmts(), view->main() );
}


void ImageView::update_ICS(mrv::media fg) const
{
    CMedia* img = fg->image();
    mrv::PopupMenu* o = uiMain->uiICS;
    int n = o->children();

    for ( int i = 0; i < n; ++i )
    {
        const Fl_Menu_Item* w = o->child(i);
        if ( w->label() && img->ocio_input_color_space() == w->label() )
        {
            o->copy_label( w->label() );
            o->value(i);
            //    if (w->tooltip()) o->tooltip( w->tooltip() );
            o->redraw();
            return;
        }
    }
}



static void attach_ocio_display_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    attach_ocio_display( fg->image(), view );
}

static void attach_ocio_view_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    attach_ocio_view( fg->image(), view );
}

static void attach_ctl_idt_script_cb2( const std::string& ret,
                                       mrv::ImageBrowser* v )
{
    Fl_Tree_Item* item = v->first_selected_item();

    mrv::ImageView* view = v->view();
    size_t i = 0;
    for ( ; item; item = v->next_selected_item(item), ++i )
    {
        mrv::Element* w = (mrv::Element*) item->widget();
        mrv::media m = w->media();
        mrv::CMedia* img = m->image();
        img->idt_transform( ret.c_str() );
    }

    view->redraw();
}

static void attach_ctl_rrt_script_cb2( const std::string& ret,
                                       mrv::ImageBrowser* v )
{
    Fl_Tree_Item* item = v->first_selected_item();

    mrv::ImageView* view = v->view();
    size_t i = 0;
    for ( ; item; item = v->next_selected_item(item), ++i )
    {
        mrv::Element* w = (mrv::Element*) item->widget();
        mrv::media m = w->media();
        mrv::CMedia* img = m->image();
        img->rendering_transform( ret.c_str() );
    }

    view->redraw();
}


void attach_ctl_idt_script_cb( Fl_Widget* o, mrv::ImageBrowser* v )
{
    Fl_Tree_Item* item = v->first_selected_item();

    mrv::Element* w = (mrv::Element*) item->widget();
    mrv::media m = w->media();
    mrv::CMedia* img = m->image();
    const char* transform = "";
    if ( img->idt_transform() ) transform = img->idt_transform();
    std::string ret = make_ctl_browser( transform,
                                        "ACEScsc,IDT" );
    attach_ctl_idt_script_cb2( ret, v );
}

void attach_ctl_rrt_script_cb( Fl_Widget* o, mrv::ImageBrowser* v )
{
    Fl_Tree_Item* item = v->first_selected_item();

    mrv::Element* w = (mrv::Element*) item->widget();
    mrv::media m = w->media();
    mrv::CMedia* img = m->image();
    const char* transform = "";
    if ( img->rendering_transform() ) transform = img->rendering_transform();
    std::string ret = make_ctl_browser( transform,
                                        "RRT,RT" );
    attach_ctl_rrt_script_cb2( ret, v );
}

static void monitor_icc_profile_cb( Fl_Widget* o, ViewerUI* uiMain )
{
    monitor_icc_profile(uiMain);
}

static void monitor_ctl_script_cb( Fl_Widget* o, ViewerUI* uiMain )
{
    monitor_ctl_script(uiMain);
}

static void copy_pixel_rgba_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    view->copy_pixel();
}

static void toggle_copy_frame_xy_cb( Fl_Widget* o, mrv::ImageView* view )
{
    view->toggle_copy_frame_xy();
}


static void attach_audio_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    std::string file = open_audio_file();
    if ( file.empty() ) return;

    CMedia* img = fg->image();
    if ( img == NULL ) return;

    DBGM3( "Attach audio file " << file << " first frame: " << img->first_frame() );
    img->audio_file( file.c_str() );
    DBGM3( "Attached audio file " << file << " first frame: " << img->first_frame() );
    view->refresh_audio_tracks();

}


static void detach_audio_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    view->stop();

    CMedia* img = fg->image();
    if ( img == NULL ) return;

    img->audio_file( NULL );
    view->refresh_audio_tracks();

}


void ImageView::scale_pic_mode()
{
    _mode = kScalePicture;
    uiMain->uiSelection->value(false);
    uiMain->uiErase->value(false);
    uiMain->uiCircle->value(false);
    uiMain->uiArrow->value(false);
    uiMain->uiDraw->value(false);
    uiMain->uiText->value(false);
    uiMain->uiScrub->value(false);

    uiMain->uiPaint->uiMovePic->value(true);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiCircle->value(false);
    uiMain->uiPaint->uiArrow->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    redraw();
}

void ImageView::move_pic_mode()
{
    if ( !foreground() ) return;

    _mode = kMovePicture;

    uiMain->uiStatus->copy_label( _("Move Pic.") );

    uiMain->uiSelection->value(false);
    uiMain->uiErase->value(false);
    uiMain->uiCircle->value(false);
    uiMain->uiArrow->value(false);
    uiMain->uiDraw->value(false);
    uiMain->uiText->value(false);
    uiMain->uiScrub->value(false);

    uiMain->uiPaint->uiMovePic->value(true);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiCircle->value(false);
    uiMain->uiPaint->uiArrow->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    redraw();
}


int ImageView::remove_children()
{
    int ret = 1;
    for ( int i = 0; i < children(); ++i )
    {
        MultilineInput* w = dynamic_cast< MultilineInput* >( child(i) );
        if (!w) continue;
        ret = w->accept();
    }

    redraw();
    return ret;
}

void ImageView::scrub_mode()
{
    _mode = kScrub;
    uiMain->uiStatus->copy_label( _("Scrub") );

    uiMain->uiSelection->value(false);
    uiMain->uiErase->value(false);
    uiMain->uiCircle->value(false);
    uiMain->uiArrow->value(false);
    uiMain->uiDraw->value(false);
    uiMain->uiText->value(false);
    uiMain->uiScrub->value(true);

    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiCircle->value(false);
    uiMain->uiPaint->uiArrow->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiRectangle->value(false);
    uiMain->uiPaint->uiCopyXY->value(false);
    uiMain->uiPaint->uiScrub->value(true);

    remove_children();
}

void ImageView::selection_mode( bool temporary )
{
    if ( !foreground() ) return;

    if ( temporary )
        _mode = kSelectionTemporary;
    else
        _mode = kSelection;

    uiMain->uiStatus->copy_label( _("Selection") );

    uiMain->uiSelection->value(true);
    uiMain->uiErase->value(false);
    uiMain->uiCircle->value(false);
    uiMain->uiArrow->value(false);
    uiMain->uiDraw->value(false);
    uiMain->uiText->value(false);
    uiMain->uiScrub->value(false);

    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(true);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiArrow->value(false);
    uiMain->uiPaint->uiCircle->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    uiMain->uiPaint->uiRectangle->value(false);
    uiMain->uiPaint->uiCopyXY->value(false);

    remove_children();
}

void ImageView::draw_mode( bool tmp )
{
    if ( !foreground() ) return;

    if ( tmp )
        _mode = kDrawTemporary;
    else
        _mode = kDraw;

    uiMain->uiStatus->copy_label( _("Draw") );

    uiMain->uiSelection->value(false);
    uiMain->uiErase->value(false);
    uiMain->uiCircle->value(false);
    uiMain->uiArrow->value(false);
    uiMain->uiDraw->value(true);
    uiMain->uiText->value(false);
    uiMain->uiScrub->value(false);

    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiCircle->value(false);
    uiMain->uiPaint->uiArrow->value(false);
    uiMain->uiPaint->uiDraw->value(true);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    uiMain->uiPaint->uiRectangle->value(false);
    uiMain->uiPaint->uiCopyXY->value(false);

    remove_children();
}

void ImageView::circle_mode()
{
    if ( !foreground() ) return;

    _mode = kCircle;

    uiMain->uiStatus->copy_label( _("Circle") );


    uiMain->uiSelection->value(false);
    uiMain->uiErase->value(false);
    uiMain->uiCircle->value(true);
    uiMain->uiArrow->value(false);
    uiMain->uiDraw->value(false);
    uiMain->uiText->value(false);
    uiMain->uiScrub->value(false);

    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiCircle->value(true);
    uiMain->uiPaint->uiArrow->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    uiMain->uiPaint->uiRectangle->value(false);
    uiMain->uiPaint->uiCopyXY->value(false);

    remove_children();
}

void ImageView::arrow_mode()
{
    if ( !foreground() ) return;

    _mode = kArrow;

    uiMain->uiStatus->copy_label( _("Arrow") );

    uiMain->uiSelection->value(false);
    uiMain->uiErase->value(false);
    uiMain->uiCircle->value(false);
    uiMain->uiArrow->value(true);
    uiMain->uiDraw->value(false);
    uiMain->uiText->value(false);
    uiMain->uiScrub->value(false);

    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiCircle->value(false);
    uiMain->uiPaint->uiArrow->value(true);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    uiMain->uiPaint->uiRectangle->value(false);
    uiMain->uiPaint->uiCopyXY->value(false);

    remove_children();
}

void ImageView::rectangle_mode()
{
    if ( !foreground() ) return;

    _mode = kRectangle;

    uiMain->uiStatus->copy_label( _("Rectangle") );

    uiMain->uiSelection->value(false);
    uiMain->uiErase->value(false);
    uiMain->uiCircle->value(false);
    uiMain->uiArrow->value(false);
    uiMain->uiDraw->value(false);
    uiMain->uiText->value(false);
    uiMain->uiScrub->value(false);

    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiCircle->value(false);
    uiMain->uiPaint->uiArrow->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    uiMain->uiPaint->uiRectangle->value(true);
    uiMain->uiPaint->uiCopyXY->value(false);

    remove_children();
}


void ImageView::erase_mode( bool tmp )
{
    if ( !foreground() ) return;

    if ( tmp )
        _mode = kEraseTemporary;
    else
        _mode = kErase;

    uiMain->uiStatus->copy_label( _("Erase") );

    uiMain->uiSelection->value(false);
    uiMain->uiErase->value(true);
    uiMain->uiCircle->value(false);
    uiMain->uiArrow->value(false);
    uiMain->uiDraw->value(false);
    uiMain->uiText->value(false);
    uiMain->uiScrub->value(false);

    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(true);
    uiMain->uiPaint->uiCircle->value(false);
    uiMain->uiPaint->uiArrow->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    uiMain->uiPaint->uiRectangle->value(false);
    uiMain->uiPaint->uiCopyXY->value(false);

    remove_children();
}

void ImageView::text_mode()
{
    if ( !foreground() ) return;

    remove_children();
    bool ok = mrv::make_window();
    if ( ok )
    {
        _mode = kText;
        uiMain->uiStatus->copy_label( _("Text") );
        uiMain->uiText->value(true);
        uiMain->uiScrub->value(false);
        uiMain->uiArrow->value(false);
        uiMain->uiErase->value(false);
        uiMain->uiCircle->value(false);
        uiMain->uiDraw->value(false);
        uiMain->uiSelection->value(false);

        uiMain->uiPaint->uiScrub->value(false);
        uiMain->uiPaint->uiText->value(true);
        uiMain->uiPaint->uiArrow->value(false);
        uiMain->uiPaint->uiMovePic->value(false);
        uiMain->uiPaint->uiErase->value(false);
        uiMain->uiPaint->uiCircle->value(false);
        uiMain->uiPaint->uiDraw->value(false);
        uiMain->uiPaint->uiSelection->value(false);
        uiMain->uiPaint->uiRectangle->value(false);
        uiMain->uiPaint->uiCopyXY->value(false);
    }
    else
    {
        scrub_mode();
    }

    redraw();
}

bool ImageView::in_presentation() const
{
    return presentation;
}

void ImageView::send_selection() const
{
    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    char buf[128];
    sprintf( buf, "Selection %g %g %g %g", _selection.x(),
             _selection.y(), _selection.w(), _selection.h() );

    restore_locale( oldloc );

    send_network( buf );
}

void ImageView::send_network( std::string m ) const
{
    if ( !_network_active) {
        return;
    }

    ParserList::const_iterator i = _clients.begin();
    ParserList::const_iterator e = _clients.end();

    if ( i != e )
    {
        (*i)->write( m, "" );  //<- this line writes all clients
    }
}

static void about_cb( Fl_Widget* o, mrv::ImageView* v )
{
  v->toggle_window( mrv::ImageView::kAbout );
}

static void static_timeout( mrv::ImageView* v )
{
    v->handle_timeout();
}

void ImageView::create_timeout( double t )
{
  if ( ! Fl::has_timeout( (Fl_Timeout_Handler) static_timeout, this ) )
  {
      Fl::add_timeout( t, (Fl_Timeout_Handler) static_timeout, this );
  }
}

void ImageView::delete_timeout()
{
  if ( Fl::has_timeout( (Fl_Timeout_Handler) static_timeout, this ) )
  {
    Fl::remove_timeout( (Fl_Timeout_Handler) static_timeout, this );
  }
}

ImageView::ImageView(int X, int Y, int W, int H, const char *l) :
Fl_Gl_Window( X, Y, W, H, l ),
_broadcast( false ),
uiMain( NULL ),
_engine( NULL ),
_update( true ),
_wait( false ),
_normalize( false ),
_safeAreas( false ),
_grid( false ),
_grid_size( 100 ),
_masking( 0.0f ),
_wipe_dir( kNoWipe ),
_wipe( 1.0 ),
_gamma( 1.0f ),
_gain( 1.0f ),
_zoom( 1 ),
_real_zoom( 1 ),
xoffset( 0 ),
yoffset( 0 ),
spinx( 0.0 ),
spiny( 0.0 ),
posX( X ),
posY( Y ),
flags( 0 ),
_ghost_previous( 5 ),
_ghost_next( 5 ),
_channel( 0 ),
_old_channel( 0 ),
_channelType( kRGB ),
_field( kFrameDisplay ),
_displayWindow( true ),
_dataWindow( true ),
_showBG( true ),
_showPixelRatio( false ),
_useLUT( false ),
_volume( 1.0f ),
_reel( 0 ),
_preframe( 1 ),
_old_fg_frame( 0 ),
_old_bg_frame( 0 ),
_idle_callback( false ),
_vr( kNoVR ),
menu( new Fl_Menu_Button( 0, 0, 0, 0 ) ),
_timeout( NULL ),
_old_fg( NULL ),
_fg_reel( 0 ),
_bg_reel( -1 ),
_mode( kNoAction ),
_selected_image( NULL ),
_selection( mrv::Rectd(0,0) ),
_playback( CMedia::kStopped ),
_orig_playback( CMedia::kForwards ),
_network_active( true ),
_interactive( true ),
_frame( 1 ),
_lastFrame( 0 )
{
    _timer.setDesiredSecondsPerFrame(0.05f);

    int stereo = 0;
    if ( can_do( FL_STEREO ) ) stereo = 0; // should be Fl_STEREO

    mode( FL_RGB | FL_DOUBLE | FL_ALPHA |
          FL_STENCIL | stereo );

    menu->type( Fl_Menu_Button::POPUP3 );

#ifdef OSX
    Fl_Sys_Menu_Bar::about( (Fl_Callback*)about_cb, this );
#endif

    float scale = Fl::screen_scale( window()->screen_num() );
    _real_zoom = _zoom / scale;

    create_timeout( 0.5 / 60.0 );
}


void ImageView::stop_playback()
{

    CMedia* img = NULL;

    mrv::media fg = foreground();
    if ( fg ) {
        img = fg->image();
        img->stop();
        if ( uiMain && !timeline()->edl() ) frame( img->frame() );
    }

    mrv::media bg = background();
    if ( bg ) bg->image()->stop(true);

}


ImageView::~ImageView()
{
    delete gridWindow; gridWindow = NULL;

    delete_timeout();
    if ( CMedia::preload_cache() )
        preload_cache_stop();


    if ( _server ) _server->remove( uiMain );

    // ParserList::iterator i = _clients.begin();
    // ParserList::iterator e = _clients.end();
    // for ( ; i != e; ++i )
    // {
    //     (*i)->connected = false;
    // }

    //_clients.clear();

    // make sure to stop any playback
    stop_playback();

    delete _engine;
    _engine = NULL;

    uiMain = NULL;
}

mrv::MainWindow* ImageView::fltk_main()
{
    assert( uiMain );
    assert( uiMain->uiMain );
    if ( !uiMain ) return NULL;
    return uiMain->uiMain;
}

const mrv::MainWindow* ImageView::fltk_main() const
{
    if ( !uiMain ) return NULL;
    assert( uiMain->uiMain );
    return uiMain->uiMain;
}


ImageBrowser* ImageView::browser() const {
    if ( !uiMain ) return NULL;
    if ( !uiMain->uiReelWindow ) return NULL;
    assert( uiMain->uiReelWindow );
    assert( uiMain->uiReelWindow->uiBrowser );
    return uiMain->uiReelWindow->uiBrowser;
}


Timeline*
ImageView::timeline() {
    if ( !uiMain ) return NULL;
    assert( uiMain->uiTimeline );
    return uiMain->uiTimeline;
}


void ImageView::switch_channels()
{
    mrv::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return; // Audio only - no channels

    static int old_channel;

    unsigned short idx = 0;
    for ( unsigned short i = 0; i < num; ++i, ++idx )
    {
        const Fl_Menu_Item* w = uiColorChannel->child(i);
        if ( w->flags & FL_SUBMENU )
        {
            for ( ; w->label(); w = w->next() )
                ++idx;
        }
    }

    if ( old_channel >= idx ) return; // too few channels

    unsigned int c = old_channel;
    old_channel = channel();
    channel( c );
}


void ImageView::ghost_previous( short x ) {
    _ghost_previous = x;
    char buf[64];
    sprintf( buf, N_("GhostPrevious %d"), x );
    send_network( buf );
    redraw();
}
void ImageView::ghost_next( short x ) {
    _ghost_next = x;
    char buf[64];
    sprintf( buf, N_("GhostNext %d"), x );
    send_network( buf );
    redraw();
}

bool ImageView::previous_channel()
{
    mrv::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return false; // Audio only - no channels

    int c = channel();

    bool is_group = false;
    short previous = -1;
    unsigned short total = 0;
    const Fl_Menu_Item* w;

    // Count (total) number of channels
    uiColorChannel->menu_end();
    total = uiColorChannel->children() - 1;
    --num;
    unsigned short idx = 0;
    for ( unsigned short i = 0; i < num; ++i, ++idx )
    {
        w = uiColorChannel->child(i);

        // This handles jump from first channel to last (Z or Color or
        // any channel done in loop later)
        if ( c == 0 && c == idx )
        {
            // Select last channel (group)
            const Fl_Menu_Item* last = uiColorChannel->child(total-1);

            // Jump to Z based on label
            if ( total > 8 && last && last->label() &&
                 strcmp( last->label(), N_("Z") ) == 0 )
            {
                previous = total-1;
                is_group = true;
                break;
            }
            // Jump to color based on label
            else if ( total >= 7 && last && last->label() &&
                      strcmp( last->label(), _("Alpha Overlay") ) == 0 )
            {
                previous = total - 1;
                is_group = true;
                break;
            }
            else if ( total >= 5 && last && last->label() &&
                      strcmp( last->label(), _("Lumma") ) == 0 )
            {
                previous = total - 1;
                is_group = true;
                break;
            }
        }

        // This handles jump from Z to Color
        if ( c == idx && c >= 4 && w && w->label() &&
             ( ( strcmp( w->label(), N_("Z") ) == 0 )  ) )
        {
            // Handle Z, Alpha Overlay and Lumma and RGBA
            if ( c >= 7 )  // Z
                previous = c - 7;
            else if ( c >= 6 )  // alpha overlay
                previous = c - 6;
            else if ( c >= 4 )  // lumma
                previous = c - 4;
            is_group = true;
        }
        if ( w && w->flags & FL_SUBMENU )
        {
            if ( c == idx && previous >= 0) {
                is_group = true;
            }

            if ( !is_group ) previous = idx;
        }
        else if ( c == idx && w && w->label() &&
                  previous >= 0 && strcmp( w->label(), _("Color") ) == 0 )
        {
            is_group = true;
        }
    }

    if ( (is_group && previous >= 0) || (!is_group && c > 0) )
    {
        if ( is_group )
        {
            channel( previous );
        }
        else
        {
            channel( c - 1 );
        }
    }
    else
    {
        if ( previous > 0 && previous < idx )
        {
            channel( previous );
        }
        else
        {
            channel( idx - 1 );
        }
    }

    return true;
}

bool ImageView::next_channel()
{
    mrv::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    // check if a channel shortcut
    uiColorChannel->menu_end();
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return false; // Audio only - no channels

    int c = channel();

    --num;

    bool is_group = false;
    unsigned short next = 0;
    unsigned short idx = 0;
    for ( unsigned short i = 0; i < num; ++i, ++idx )
    {
        const Fl_Menu_Item* w = uiColorChannel->child(i);
        if ( w->flags & FL_SUBMENU )
        {
            unsigned numc = 0;
            for ( ; w->label(); ++w )
                ++numc;

            if ( c == idx ) {
                is_group = true;
                next = idx + numc + 1;
            }
            continue;
        }
        if ( c == idx && w->label() && strcmp( w->label(), _("Color") ) == 0 )
        {
            if ( num > 7 )
            {
                is_group = true;
                next = idx + 7;
            }
        }
    }


    if ( (is_group && next < idx) || (!is_group && c < idx-1) )
    {
        if ( is_group )
        {
            channel( next );
        }
        else
        {
            channel( c + 1 );
        }
    }
    else
    {
        channel( (unsigned short)0 );
    }

    return true;
}

/**
 * Initialize OpenGL context, textures, and get opengl features for
 * this gfx card.
 *
 */
void ImageView::init_draw_engine()
{
    _engine = new mrv::GLEngine( this );
    DBGM3( __FUNCTION__ << " " << __LINE__ );
    if ( !_engine )
    {
        mrvALERT( _("Could not initialize draw engine") );
        return;
    }

    CMedia::supports_yuv( _engine->supports_yuv() );
    CMedia::supports_yuva( _engine->supports_yuva() );
}



void ImageView::fg_reel(int idx)
{
    _fg_reel = idx;

    char buf[128];
    sprintf( buf, N_("FGReel %d"), idx );
    send_network( buf );
}

void ImageView::bg_reel(int idx)
{
    _bg_reel = idx;

    char buf[128];
    sprintf( buf, N_("BGReel %d"), idx );
    send_network( buf );
}

void ImageView::toggle_copy_frame_xy()
{
    if ( !foreground() ) return;

    if ( _mode == kCopyFrameXY )
        scrub_mode();
    else
    {
        _mode = kCopyFrameXY;
        uiMain->uiStatus->copy_label( _("Copy X Y") );

        uiMain->uiSelection->value(false);
        uiMain->uiErase->value(false);
        uiMain->uiCircle->value(false);
        uiMain->uiArrow->value(false);
        uiMain->uiDraw->value(false);
        uiMain->uiText->value(false);
        uiMain->uiScrub->value(false);

        uiMain->uiPaint->uiMovePic->value(false);
        uiMain->uiPaint->uiSelection->value(false);
        uiMain->uiPaint->uiErase->value(false);
        uiMain->uiPaint->uiCircle->value(false);
        uiMain->uiPaint->uiArrow->value(false);
        uiMain->uiPaint->uiDraw->value(false);
        uiMain->uiPaint->uiText->value(false);
        uiMain->uiPaint->uiScrub->value(false);
        uiMain->uiPaint->uiRectangle->value(false);
        uiMain->uiPaint->uiCopyXY->value(true);

        remove_children();
    }
}

/**
 * Given window's x and y coordinates, return an image's
 * corresponding x and y coordinate
 *
 * @param img image to find coordinates for
 * @param x   window's x position
 * @param y   window's y position
 */
void ImageView::copy_frame_xy() const
{
    mrv::media fg = foreground();
    if ( !fg ) return;

    CMedia* img = fg->image();

    int x = lastX;
    int y = lastY;


    if ( x < 0 || y < 0 || x >= this->w() || y >= this->h() )
        return;

    mrv::image_type_ptr pic;
    bool outside = false;
    int off[2];
    int xp, yp, w, h;
    picture_coordinates( img, x, y, outside, pic, xp, yp, w, h, off );


    if ( outside || !pic ) return;

    yp = pic->height() - yp - 1;

    mrv::Recti daw[2];
    daw[0] = img->data_window();
    daw[1] = img->data_window2();

    CMedia::StereoOutput stereo_out = img->stereo_output();
    CMedia::StereoInput  stereo_in  = img->stereo_input();

    if ( stereo_out & CMedia::kStereoAnaglyph )
    {
        if ( stereo_in == CMedia::kTopBottomStereoInput )
        {
            yp += h;
        }
        else if ( stereo_in == CMedia::kLeftRightStereoInput )
        {
            xp += w;
        }
        else
        {
            if ( stereo_out & CMedia::kStereoRight )
            {
                pic = img->left();
                xp += daw[1].x();
                yp += daw[1].y();
                xp -= daw[0].x();
                yp -= daw[0].y();
            }
            else
            {
                pic = img->right();
                xp += daw[0].x();
                yp += daw[0].y();
                xp -= daw[1].x();
                yp -= daw[1].y();
            }
        }

    }

    const mrv::Reel& r = browser()->current_reel();
    unsigned shot_id = 0;
    for ( unsigned i = 0; i < r->images.size(); ++i )
    {
        if ( r->images[i] == fg )
        {
            shot_id = i; break;
        }
    }

    char buf[256];
    sprintf( buf,
             _("Reel %d (%s) | Shot %d (%s) | Frame %" PRId64
               " | X = %d | Y = %d\n"),
             _fg_reel, r->name.c_str(), shot_id, img->name().c_str(),
             _frame.load(), xp, yp );

    LOG_INFO( buf );

    // Copy text to both the clipboard and to X's XA_PRIMARY
    Fl::copy( buf, unsigned( strlen(buf) ), true );
    Fl::copy( buf, unsigned( strlen(buf) ), false );
}

/**
 * Given window's x and y coordinates, return an image's
 * corresponding x and y coordinate
 *
 * @param img image to find coordinates for
 * @param x   window's x position
 * @param y   window's y position
 */
void ImageView::copy_pixel() const
{
    mrv::media fg = foreground();

    CMedia* img = fg->image();

    int x = lastX;
    int y = lastY;


    if ( x < 0 || y < 0 || x >= this->w() || y >= this->h() )
        return;

    mrv::image_type_ptr pic;
    bool outside = false;
    int off[2];
    int xp, yp, w, h;
    picture_coordinates( img, x, y, outside, pic, xp, yp, w, h, off );


    if ( outside || !pic ) return;

    int ypr = pic->height() - yp - 1;

    CMedia::Pixel rgba = pic->pixel( xp, yp );
    pixel_processed( img, rgba );

    mrv::Recti daw[2];
    daw[0] = img->data_window();
    daw[1] = img->data_window2();

    CMedia::StereoOutput stereo_out = img->stereo_output();
    CMedia::StereoInput  stereo_in  = img->stereo_input();

    if ( stereo_out & CMedia::kStereoAnaglyph )
    {
        if ( stereo_in == CMedia::kTopBottomStereoInput )
        {
            yp += h;
        }
        else if ( stereo_in == CMedia::kLeftRightStereoInput )
        {
            xp += w;
        }
        else
        {
            if ( stereo_out & CMedia::kStereoRight )
            {
                pic = img->left();
                xp += daw[1].x();
                yp += daw[1].y();
                xp -= daw[0].x();
                yp -= daw[0].y();
            }
            else
            {
                pic = img->right();
                xp += daw[0].x();
                yp += daw[0].y();
                xp -= daw[1].x();
                yp -= daw[1].y();
            }
        }

        if ( pic && xp < (int)pic->width() && yp < (int)pic->height() )
        {
            float r = rgba.r;
            rgba = pic->pixel( xp, yp );
            pixel_processed( img, rgba );
            rgba.r = r;
        }
    }

    char buf[256];
    sprintf( buf, "%g %g %g %g", rgba.r, rgba.g, rgba.b, rgba.a );

    // Copy text to both the clipboard and to X's XA_PRIMARY
    Fl::copy( buf, unsigned( strlen(buf) ), true );
    Fl::copy( buf, unsigned( strlen(buf) ), false );
}


void ImageView::rot2vec( double& x, double& y, const double r )
{
    double radians = r * (M_PI / 180);

    double cs = cos( radians );
    double sn = sin( radians );

    double tmp = (double)x * cs - (double)y * sn;
    y = (double)x * sn + (double)y * cs;
    x = tmp;
}

void ImageView::data_window_coordinates( const CMedia* const img,
                                         double& x, double& y ) const
{
    image_coordinates( img, x, y );

    const mrv::Recti& dpw = img->display_window();
    unsigned W = dpw.w();
    unsigned H = dpw.h();


    x -= W/2.0;
    y -= H/2.0;



    {
        const mrv::Recti& daw = img->data_window();
        x -= daw.x();
        y -= daw.y();
    }

    //    rot2vec( x, y, img->rot_z() );

    //
    // If image is smaller than display window, we are dealing
    // with a RY or BY image.  We divide the window coordinates by 2.
    //
    mrv::image_type_ptr pic = img->left();
    std::string ch;
    if ( img->channel() ) ch = img->channel();
    if ( pic && ( ch == "RY" || ch == "BY" ) )
    {
        if ( pic->width() < W ) x /= 2;
        if ( pic->height() < H ) y /= 2;
    }
}

/**
 * Given window's x and y coordinates, return an image's
 * corresponding x and y coordinate
 *
 * @param img image to find coordinates for
 * @param x   window's x position
 * @param y   window's y position
 */
void ImageView::image_coordinates( const CMedia* const img,
                                   double& x, double& y ) const
{
    const mrv::Recti& dpw = img->display_window();

    double W = dpw.w();
    double H = dpw.h();

    if ( _showPixelRatio ) H /= pixel_ratio();




    double tw = (W / 2.0);
    double th = (H / 2.0);

    x -= (w() - W) / 2.0;
    y += (h() - H) / 2.0;

    y = (this->h() - y - 1);

    x -= tw;
    y -= th;

    // ImageView* self = const_cast< ImageView* >( this );
    // float scale = Fl::screen_scale( window()->screen_num() );
    // self->_real_zoom = _zoom / scale;

    x /= _real_zoom;
    y /= _real_zoom;
    x += tw;
    y += th;
    x -= xoffset;
    y -= yoffset;


    y = H - y;


    if ( _showPixelRatio ) y *= pixel_ratio();
}




void ImageView::center_image()
{
    mrv::media fg = foreground();
    if ( !fg ) return;

    Image_ptr img = fg->image();
    CMedia::StereoOutput stereo_out = img->stereo_output();
    mrv::Recti dpw;
    if ( display_window() )
    {
        dpw = img->display_window();

        if ( stereo_out & CMedia::kStereoAnaglyph )
        {
            mrv::Recti dpw2 = img->display_window2();
            dpw.merge( dpw2 );
        }
        else if ( stereo_out == CMedia::kStereoRight )
        {
            dpw = img->display_window2();
        }
        else if ( stereo_out & CMedia::kStereoSideBySide )
        {
            const mrv::Recti& dpw2 = img->display_window2();
            dpw.w( dpw.w() + dpw2.x() + dpw2.w() );
        }
        else if ( stereo_out & CMedia::kStereoTopBottom )
        {
            const mrv::Recti& dpw2 = img->display_window2();
            dpw.h( dpw.h() + dpw2.y() + dpw2.h() );
        }
    }
    else
    {
        dpw = img->data_window();
        if ( stereo_out & CMedia::kStereoAnaglyph )
        {
            mrv::Recti dpw2 = img->data_window2();
            dpw.merge( dpw2 );
        }
        else if ( stereo_out == CMedia::kStereoRight )
        {
            dpw = img->data_window2();
        }
        else if ( stereo_out & CMedia::kStereoSideBySide )
        {
            const mrv::Recti& dp = img->display_window();
            mrv::Recti daw = img->data_window2();
            daw.x( dp.w() + daw.x() );
            dpw.merge( daw );
        }
        else if ( stereo_out & CMedia::kStereoTopBottom )
        {
            const mrv::Recti& dp = img->display_window();
            mrv::Recti daw = img->data_window2();
            daw.y( dp.h() + daw.y() );
            dpw.merge( daw );
        }
    }


    dpw.x( dpw.x() + img->x() );  // here
    dpw.y( dpw.y() - img->y() );

    mrv::media bg = background();
    if ( bg )
    {
        CMedia* img2 = bg->image();
        mrv::Recti dpw2 = img2->display_window();
        dpw2.x( int(dpw2.x() + img2->x()) );
        dpw2.y( int(dpw2.y() - img2->y()) );
        if ( main()->uiPrefs->uiPrefsResizeBackground->value() == 0 )
        {
            dpw.merge( dpw2 );
        }
    }

    double pr = 1.0;
    if ( _showPixelRatio ) pr = pixel_ratio();

    mrv::image_type_ptr pic = img->left();
    int H = dpw.h();
    if ( H == 0 ) H = pic->height();
    int W = dpw.w();
    if ( W == 0 ) W = pic->width();


    // Handle image 90 degrees rotation
    double r = tan( ( 90 + img->rot_z() ) * (M_PI / 180) );
    if ( std::abs(r) <= 0.0001 )
    {
        int tmp = H;
        H = W;
        W = tmp;
        int x = dpw.x();
        dpw.x( dpw.y() );
        dpw.y( x );
    }

    yoffset = ( dpw.y() + H / 2.0) / pr;

    if ( stereo_out & CMedia::kStereoSideBySide )
    {
#ifdef OSX
        xoffset = -W/4.0 + 0.5;
#else
        xoffset = -W/2.0 + 0.5;
#endif
    }
    else if ( stereo_out & CMedia::kStereoTopBottom )
    {
        yoffset = (( -H/2.0 ) / pr + 0.5 );
    }
    else
    {
        xoffset = -dpw.x() - W / 2.0;
    }

    zrotation_to_offsets( xoffset, yoffset, img, W, H );


    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );


    char buf[128];
    sprintf( buf, N_("Offset %g %g"), xoffset, yoffset );
    send_network( buf );

    restore_locale( oldloc );

    redraw();
}


/**
 * Fit the current foreground image to the view window.
 *
 */
void ImageView::fit_image()
{
    const mrv::media fg = foreground();
    if ( !fg ) return;

    const CMedia* img = fg->image();

    mrv::image_type_ptr pic = img->left();
    if ( !pic ) return;

    CMedia::StereoOutput stereo_out = img->stereo_output();
    mrv::Recti dpw;
    if ( display_window() )
    {
        dpw = img->display_window();
        if ( stereo_out & CMedia::kStereoAnaglyph )
        {
            mrv::Recti dpw2 = img->display_window2();
            dpw.merge( dpw2 );
        }
        else if ( stereo_out == CMedia::kStereoRight )
        {
            dpw = img->display_window2();
        }
        else if ( stereo_out & CMedia::kStereoSideBySide )
        {
            const mrv::Recti& dpw2 = img->display_window2();
            dpw.w( dpw.w() + dpw2.x() + dpw2.w() );
        }
        else if ( stereo_out & CMedia::kStereoTopBottom )
        {
            const mrv::Recti& dpw2 = img->display_window2();
            dpw.h( dpw.h() + dpw2.y() + dpw2.h() );
        }
        else
        {
            if ( data_window() )
                dpw.merge( img->data_window() );
        }
    }
    else
    {
        dpw = img->data_window();
        if ( stereo_out & CMedia::kStereoAnaglyph )
        {
            mrv::Recti dpw2 = img->data_window2();
            dpw.merge( dpw2 );
        }
        else if ( stereo_out == CMedia::kStereoRight )
        {
            dpw = img->data_window2();
        }
        else if ( stereo_out & CMedia::kStereoSideBySide )
        {
            const mrv::Recti& dp = img->display_window();
            mrv::Recti daw = img->data_window2();
            daw.x( dp.w() + daw.x() );
            dpw.merge( daw );
        }
        else if ( stereo_out & CMedia::kStereoTopBottom )
        {
            const mrv::Recti& dp = img->display_window();
            mrv::Recti daw = img->data_window2();
            daw.y( dp.h() + daw.y() );
            dpw.merge( daw );
        }
    }

    mrv::media bg = background();
    dpw.x( int(dpw.x() + img->x()) );
    dpw.y( int(dpw.y() - img->y()) );
    if ( bg )
    {
        CMedia* img2 = bg->image();
        mrv::Recti dpw2 = img2->display_window();
        dpw2.x( int(dpw2.x() + img2->x()) );
        dpw2.y( int(dpw2.y() - img2->y()) );
        if ( main()->uiPrefs->uiPrefsResizeBackground->value() == 0 )
        {
            dpw.merge( dpw2 );
        }
    }

    int W = dpw.w();
    if ( W == 0 ) W = pic->width();
    int H = dpw.h();
    if ( H == 0 ) H = pic->height();

    // Handle image 90 degrees rotation
    double r = tan( ( 90 + img->rot_z() ) * (M_PI / 180) );
    if ( std::abs(r) <= 0.0001 )
    {
        unsigned tmp = H;
        H = W;
        W = tmp;
    }

#ifdef OSX
    if ( display_window() && stereo_out & CMedia::kStereoSideBySide )
        W *= 2;
#endif

    int     X = dpw.x();
    int     Y = dpw.y();

    double pct = _masking;
    if ( pct > 0.0 )
    {
        double aspectY = (double) W / (double) H;
        double aspectX = (double) H/ (double) W;

        double target_aspect = 1.0 / pct;
        double amountY = H * (0.5 - target_aspect * aspectY / 2);
        double amountX = W * (0.5 - pct * aspectX / 2);

        bool vertical = true;
        if ( amountY < amountX )
        {
            vertical = false;
        }
        if ( vertical )
        {
            H -= amountY * 2;
            Y += amountY;
        }
        else
        {
            W -= amountX * 2;
            X += amountX;
        }
    }

    double w = (double) this->pixel_w();
    double h = (double) this->pixel_h();
    DBGM1( "fit h=" << h );
    DBGM1( "fit H=" << H << " pic->h()= " << pic->height() );
    DBGM1( "fit h/H=" << h/H );


#ifdef OSX
    h /= 2;  // On OSX X and Y pixel coords are doubled
    w /= 2;
#endif

    DBGM1("fit w=" << w );
    DBGM1("fit W=" << W );
    float z = w / (double)W;
    DBGM1( "fit w/W=" << z );
    h /= H;

    double pr = 1.0;
    if ( _showPixelRatio ) pr = pixel_ratio();
    h *= pr;

    if ( h < z ) {
        z = h;
    }
    DBGM1( "fit z=" << z );

    double ox = xoffset;
    double oy = yoffset;

    yoffset = ( Y + H / 2.0) / pr;

    if ( stereo_out & CMedia::kStereoSideBySide )
    {
#ifdef OSX
        xoffset = -W/4.0 + 0.5;
#else
        xoffset = -W/2.0 + 0.5;
#endif
    }
    else
    {
        xoffset = -X - W / 2.0;
    }


    if ( img->flipY() && stereo_out & CMedia::kStereoSideBySide  )
        xoffset = 0.0;

    if ( img->flipX() && stereo_out & CMedia::kStereoTopBottom  )
        yoffset = 0.0;

    zrotation_to_offsets( xoffset, yoffset, img, W, H );

    char buf[128];
    sprintf( buf, "FitImage" );
    send_network( buf );

    bool old_network_active = _network_active;
    _network_active = false;

    zoom( z );

    int x = Fl::event_x();
    int y = Fl::event_y();
    if ( uiMain->uiToolsGroup->visible() )
        x -= uiMain->uiToolsGroup->w();
    mouseMove( x, y );

    _network_active = old_network_active;

    redraw();
}


void
ImageView::zrotation_to_offsets( double& X, double& Y,
                                 const CMedia* img,
                                 const int W,
                                 const int H )
{
    double degrees = img->rot_z();
    double r = degrees * (M_PI / 180);  // in radians, please
    double sn = sin(r);
    double cs = cos(r);
    if ( is_equal( sn, -1.0, 0.001 ) )
    {
        // This cascading if/then is correct
        if ( img->flipY() )
        {
            X -= W + H;
        }
        if ( img->flipX() )
        {
            X += W;
            Y -= H - W;
        }
        else {
            X += W;
        }
    }
    else if ( (is_equal( sn, 0.0, 0.001 ) && is_equal( cs, -1.0, 0.001 )) )
    {
        // This cascading if/then is correct
        if ( img->flipY() )
        {
            X -= W * 2;
        }
        if ( img->flipX() )
        {
            Y += H;
            X += W;
        }
        else
        {
            X += W;
            Y -= H;
        }
    }
    else if ( (is_equal( sn, 1.0, 0.001 ) && is_equal( cs, 0.0, 0.001 )) )
    {
        // This cascading if/then is correct
        if ( img->flipY() )
        {
            X -= H - W;
        }
        if ( img->flipX() )
            Y += W;
        else
            Y -= H;
    }
}

void ImageView::stereo_input( CMedia::StereoInput x )
{
    mrv::media fg = foreground();
    if (!fg) return;

    CMedia* img = fg->image();
    img->stereo_input(x);

    ViewerUI* m = main();
    if ( m )
    {
        m->uiStereo->refresh();
    }

    char buf[64];
    sprintf( buf, "StereoInput %d", x );
    send_network( buf );
}

void ImageView::stereo_output( CMedia::StereoOutput x )
{
    mrv::media fg = foreground();
    if (!fg) return;

    CMedia* img = fg->image();
    img->stereo_output(x);

    ViewerUI* m = main();
    if ( m )
    {
        m->uiStereo->refresh();
    }

    char buf[64];
    sprintf( buf, "StereoOutput %d", x );
    send_network( buf );
}

CMedia::StereoInput ImageView::stereo_input() const
{
    mrv::media fg = foreground();
    if (!fg) return CMedia::kNoStereoInput;

    CMedia* img = fg->image();
    return img->stereo_input();
}

CMedia::StereoOutput ImageView::stereo_output() const
{
    mrv::media fg = foreground();
    if (!fg) return CMedia::kNoStereo;

    CMedia* img = fg->image();
    return img->stereo_output();
}


/**
 * Check foreground and background images for updates.
 *
 * @return true if view needs redrawing, false if not.
 */
bool ImageView::should_update( mrv::media fg )
{
    bool update = _idle_callback || playback() != CMedia::kStopped;


    CMedia* img = NULL;
    if ( fg )
    {
        img = fg->image();

#if 0
        std::cerr << img->frame() << " " << img->image_damage()
                  << " update " << update
                  << " contents: " << (img->image_damage() & CMedia::kDamageContents)
                  << " layers: " << (img->image_damage() & CMedia::kDamageLayers)
                  << " ICS: " << (img->image_damage() & CMedia::kDamageICS)
                  << " data: " << (img->image_damage() & CMedia::kDamageData)
                  << std::endl;
#endif

        if ( img->image_damage() & CMedia::kDamageLayers )
        {
            update_layers();
        }

        if ( uiMain->uiICS->visible() &&
             ( img->image_damage() & CMedia::kDamageICS ) )
        {
            update_ICS(fg);
            img->image_damage( img->image_damage() & ~CMedia::kDamageICS );
        }

        if ( img->image_damage() & CMedia::kDamageContents )
        {
            update = true;
        }

        if ( _playback == CMedia::kStopped &&
             img->image_damage() & CMedia::kDamageThumbnail )
        {
            // Redraw browser to update thumbnail
            mrv::ImageBrowser* b = browser();
            if (b) b->redraw();
            img->image_damage( img->image_damage() & ~CMedia::kDamageThumbnail );
        }

        if ( img->image_damage() & CMedia::kDamageData )
        {
            update_image_info();
        }

        if ( img->image_damage() & CMedia::kDamageTimecode )
        {
            int64_t start = uiMain->uiStartFrame->frame();  // needed not sure why
            int64_t end   = uiMain->uiEndFrame->frame();  // needed not sure why
            int64_t   tc  = img->timecode();
            uiMain->uiFrame->timecode( tc );
            uiMain->uiStartFrame->timecode( tc );
            uiMain->uiStartFrame->frame( start );  // needed not sure why
            uiMain->uiEndFrame->timecode( tc );
            uiMain->uiEndFrame->frame( end );  // needed not sure why
            uiMain->uiTimeline->timecode( tc );

            mrv::Timeline* t = uiMain->uiEDLWindow->uiTimeline;
            if (t && t->visible() )
            {
                t->timecode( tc );
            }
            img->image_damage( img->image_damage() & ~CMedia::kDamageTimecode );
        }

        if ( img->image_damage() & CMedia::kDamageCache )
        {
            uiMain->uiTimeline->draw();
            img->image_damage( img->image_damage() & ~CMedia::kDamageCache );
        }


        if ( uiMain->uiGL3dView &&
             uiMain->uiGL3dView->uiMain->visible() &&
             uiMain->uiGL3dView->uiMain->shown() &&
             (img->image_damage() & CMedia::kDamage3DData) )
        {
            int zsize = 0;
            static Imf::Array< float* > zbuff;
            static Imf::Array< unsigned > sampleCount;

            size_t num = zbuff.size();
            for ( size_t i = 0; i < num; ++i )
            {
                delete [] zbuff[i];
            }


            zbuff.resizeErase( 0 );
            sampleCount.resizeErase( 0 );

            mrv::exrImage* exr = dynamic_cast< mrv::exrImage* >( img );
            if ( exr )
            {
                try
                {
                    float zmin, zmax;
                    float farPlane = 1000000.0f;
                    const mrv::Recti& daw = exr->data_window();
                    exr->loadDeepData( zsize, zbuff, sampleCount );
                    exr->findZBound( zmin, zmax, farPlane, zsize,
                                     zbuff, sampleCount );
                    uiMain->uiGL3dView->uiMain->load_data( zsize,
                                                           zbuff,
                                                           sampleCount,
                                                           daw.w(), daw.h(),
                                                           zmin, zmax,
                                                           farPlane );
                    uiMain->uiGL3dView->uiMain->redraw();
                }
                catch( const std::exception& e )
                {
                    LOG_ERROR( e.what() );
                }
            }
            img->image_damage( img->image_damage() & ~CMedia::kDamage3DData );
        }
    }

    mrv::media bg = background();
    if ( bg )
    {
        CMedia* bimg = bg->image();

        if ( bimg->image_damage() & CMedia::kDamageContents )
        {
            // resize_background();
            update = true;
        }

        if ( _playback == CMedia::kStopped &&
             bimg->image_damage() & CMedia::kDamageThumbnail )
        {
            // Redraw browser to update thumbnail
            mrv::ImageBrowser* b = browser();
            if (b) b->redraw();
            bimg->image_damage( bimg->image_damage() & ~CMedia::kDamageThumbnail );
        }
    }

    if ( update && _playback != CMedia::kStopped ) {
        int x = Fl::event_x();
        int y = Fl::event_y();
#ifdef FLTK_TIMEOUT_EVENT_BUG
        if ( uiMain->uiMenuGroup->visible() )
            y -= uiMain->uiMenuGroup->h();
        if ( uiMain->uiTopBar->visible() )
            y -= uiMain->uiTopBar->h();
#endif
        if ( uiMain->uiToolsGroup->visible() )
            x -= uiMain->uiToolsGroup->w();
        mouseMove( x, y );
    }


    return update;
}



void static_preload( mrv::ImageView* v )
{
    v->preload();

    mrv::media m = v->foreground();
    if ( m )
    {
        CMedia* img = m->image();
        double delay = 1.0 / img->fps();
        if ( ! img->is_stereo() )
        {
            if ( ! Fl::has_timeout( (Fl_Timeout_Handler) static_preload, v ) )
                Fl::repeat_timeout( delay, (Fl_Timeout_Handler) static_preload,
                                    v );
        }
        else
            if ( Fl::has_timeout( (Fl_Timeout_Handler) static_preload, v ) )
                Fl::remove_timeout( (Fl_Timeout_Handler) static_preload, v );
    }
}






void ImageView::log() const
{
    //
    // First, handle log window showing up and scrolling
    //
    LogUI* logUI = main()->uiLog;
    Fl_Window* logwindow = logUI->uiMain;
    if ( logwindow )
    {
        mrv::LogDisplay* log = logUI->uiLogText;

        if ( mrv::LogDisplay::show == true )
        {
            mrv::LogDisplay::show = false;
            if (main() && logUI && logwindow )
            {
                logwindow->show();
            }
        }
        static unsigned  lines = 0;
        if ( log->visible() && log->lines() != lines )
        {
            log->scroll( log->lines()-1, 0 );
            lines = log->lines();
        }
    }
}


int timeval_substract(struct timeval *result, struct timeval *x,
                      struct timeval *y)
{
    result->tv_sec = x->tv_sec - y->tv_sec;

    if ((result->tv_usec = x->tv_usec - y->tv_usec) < 0)
    {
        result->tv_usec += 1000000;
        result->tv_sec--; // borrow
    }

    return result->tv_sec < 0;
}

bool ImageView::ready_preframe( std::atomic<int64_t>& f,
                                CMedia::Playback p,
                                CMedia* const img,
                                const int64_t& first,
                                const int64_t& last )
{

    if ( p == CMedia::kForwards || p == CMedia::kStopped )
    {
        ++f;
        if ( f > last )
        {
            switch( looping() )
            {
            case CMedia::kPingPong:
                f = last;
                // playback( CMedia::kBackwards );
                // img->playback( CMedia::kBackwards );
                break;
            case CMedia::kLoop:
                f = first;
                break;
            default:
                break;
            }
        }
        return true;
    }
    else if ( p == CMedia::kBackwards )
    {
        f += p;
        if ( f < first )
        {
            switch( looping() )
            {
            case CMedia::kPingPong:
                f = first;
                // playback( CMedia::kForwards );
                // img->playback( CMedia::kForwards );
                break;
            case CMedia::kLoop:
                f = last;
                break;
            default:
                break;
            }
        }
        return true;
    }
    return false;
}

bool ImageView::preload()
{
    if ( !browser() || !timeline() ) return false;


    mrv::ImageBrowser* b = browser();

    mrv::Reel r = b->reel_at( _reel );
    if (!r) return false;


    mrv::media fg;
    if ( r->edl )
    {
        fg = r->media_at( _preframe );
    }
    else
    {
        fg = foreground();
    }

    if ( !fg )  return false;



    CMedia* img = fg->image();

    CMedia::Playback p = playback();


    // Exit early if we are dealing with a video instead of a sequence
    if ( !img->is_sequence() || img->has_video() ) {
        img = r->image_at( _preframe );
        if (!img) {
            // if no image, go to next reel
            if ( _reel + 1 < b->number_of_reels() )
                _reel++;
            else
                _reel = 0;
            _preframe = timeline()->display_minimum();
        }
        else
        {
            _preframe = fg->position() + img->duration(); // go to next image
            img->refresh();
        }

        return true;
    }

    int64_t f, first, last;
    if ( r->edl )
    {
        f = r->global_to_local( _preframe );
        // first  = img->first_frame();
        // last   = img->last_frame();
        first = timeline()->display_minimum();
        last  = timeline()->display_maximum();
    }
    else
    {
        f = _preframe;

        first = img->first_frame();
        last  = img->last_frame();
        // int64_t tfirst = timeline()->display_minimum();
        // int64_t tlast  = timeline()->display_maximum();
        // if ( tfirst > first && tfirst < last ) first = tfirst;
        // if ( tlast < last   && tlast > first )  last = tlast;

        if ( f < first ) f = first;
        else if ( f > last ) f = last;
    }



    if ( img->stopped() )
    {
        typedef CMedia::Mutex Mutex;
        Mutex& vpm = img->video_packets().mutex();
        SCOPED_LOCK( vpm );
        Mutex& apm = img->audio_packets().mutex();
        SCOPED_LOCK( apm );
        Mutex& spm = img->subtitle_packets().mutex();
        SCOPED_LOCK( spm );
        Mutex& mtx = img->video_mutex();
        SCOPED_LOCK( mtx );

        // Add a frame to image queue
        while ( ! img->frame( f ) )
        {
            if ( img->stopped() || playback() == CMedia::kStopped ) break;
            sleep_ms( 20 );
        }
    }
    else
    {
        // Needed as video thread will probably not refresh on time.
        // This assures image is loaded and ready.
        int64_t f = img->frame();
        img->find_image( f );
        //img->find_image( _preframe );
    }


    // Ready preframe for next iteration
    ready_preframe( _preframe, p, img, first, last );

    // Redraw timeline (cache line)
    timeline()->redraw();

    // Redraw view window
    redraw();

    return true;
}


void ImageView::rot_x( double x ) {
    _engine->rot_x(x);
    valid(0);
}
void ImageView::rot_y( double x ) {
    _engine->rot_y(x);
    valid(0);
}

double ImageView::rot_x() const {
    return _engine->rot_x();
}
double ImageView::rot_y() const {
    return _engine->rot_y();
}

void ImageView::handle_commands()
{
    mrv::ImageBrowser* b = browser();
    if (!b) return;
    if ( commands.empty() ) return;

    bool old_network_active = _network_active;
    bool old_interactive = _interactive;


    _interactive = false;

    Command& c = commands.front();

    if ( !_server )
        _network_active = false;

    switch( c.type )
    {
    case kCreateReel:
    {
        Imf::StringAttribute* attr = dynamic_cast< Imf::StringAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( "Create Reel failed" );
            break;
        }
        const std::string& s = attr->value();
        NET( "change to reel " << s );
        mrv::Reel r = b->reel( s.c_str() );
        if ( !r )
        {
            b->new_reel( s.c_str() );
        }
        break;
    }
    case kTimelineMinDisplay:
    {
        int64_t x = c.frame;
        NET("TimelineMinDisplay " << x );
        uiMain->uiTimeline->display_minimum( x );
        uiMain->uiTimeline->redraw();
        uiMain->uiStartFrame->value( x );
        uiMain->uiStartFrame->redraw();
        break;
    }
    case kTimelineMaxDisplay:
    {
        int64_t x = c.frame;
        NET("TimelineMaxDisplay " << x );
        uiMain->uiTimeline->display_maximum( x );
        uiMain->uiTimeline->redraw();
        uiMain->uiEndFrame->value( x );
        uiMain->uiEndFrame->redraw();
        break;
    }
    case kTimelineMin:
    {
        int64_t x = c.frame;
        NET("TimelineMin " << x );
        uiMain->uiTimeline->minimum( x );
        uiMain->uiTimeline->redraw();
        uiMain->uiStartFrame->value( x );
        uiMain->uiStartFrame->redraw();
        break;
    }
    case kTimelineMax:
    {
        int64_t x = c.frame;
        NET( "TimelineMax " << x );
        uiMain->uiTimeline->maximum( double(x) );
        uiMain->uiTimeline->redraw();
        uiMain->uiEndFrame->value( x );
        uiMain->uiEndFrame->redraw();
        break;
    }
    case kCacheClear:
        NET( "clear caches ");
        clear_caches();
        break;
    case kChangeImage:
    {
        Imf::IntAttribute* attr = dynamic_cast< Imf::IntAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( _("Change Image failed") );
            break;
        }
        int idx = attr->value();
        mrv::Reel r = b->current_reel();
        bool found = false;
        mrv::MediaList::iterator j = r->images.begin();
        mrv::MediaList::iterator e = r->images.end();
        int i = 0;
        std::string imgname;
        if ( c.linfo )
        {
            imgname = c.linfo->filename;
            NET( "change image #" << idx << " " << imgname );
            for ( ; j != e; ++j, ++i )
            {
                if ( !(*j) ) {
                    --i;
                    continue;
                }

                CMedia* img = (*j)->image();
                std::string file = img->fileroot();
                if ( i == idx && file == imgname )
                {
                    found = true;
                    break;
                }
                else if ( i == idx && file != imgname )
                {
                    LOG_ERROR( _("Image at position ") << idx << " '" << file
                               << _("' does not match '")
                               << imgname << "'." );
                    LOG_ERROR( _("Don't know which one should go first.") );
                    delete c.linfo; c.linfo = NULL;
                }
            }
        }
        else
        {
            NET( "change image to #" << idx << " < " << r->images.size() );
            found = ((size_t)idx < r->images.size() );
        }
        if ( found ) {
            NET( "change image found.  Set to #" << idx );
            b->change_image(idx);
        }
        else
        {
            if ( c.linfo ) {
                NET( "Load Imag for idx #" << idx << " is " << imgname );
                const LoadInfo* file = c.linfo;
                LoadList files;
                files.push_back( *file );
                b->load( files, false, "", false, false );
            }
        }
        break;
    }
    case kInsertImage:
    {
        Imf::IntAttribute* attr = dynamic_cast< Imf::IntAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( "Insert Image failed" );
            break;
        }
        int idx = attr->value();

        NET( "insert image #" << idx );

        mrv::Reel r = b->reel_at( bg_reel() );
        if ( (size_t)idx < r->images.size() )
        {
            assert0( c.linfo != NULL );
            const LoadInfo* file = c.linfo;

            CMedia* img = CMedia::guess_image( file->filename.c_str(), NULL, 0,
                                               false );
            if (!img) goto final;

            mrv::media m( new mrv::gui::media( img ) );
            b->insert( idx, m );
        }
        break;
    }
    case kBGImage:
    {
        Imf::IntAttribute* attr = dynamic_cast< Imf::IntAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( "BGImage failed" );
            break;
        }
        int idx = attr->value();

        NET( "change bg image #" << idx );

        if ( idx < 0 ) background( mrv::media() );
        else
        {
            mrv::Reel r = b->reel_at( bg_reel() );
            if ( (unsigned)idx < r->images.size() )
            {
                background( r->images[idx] );
            }
            else
            {
                background( mrv::media() );
            }
        }
        break;
    }
    case kFGReel:
    {
        Imf::IntAttribute* attr = dynamic_cast< Imf::IntAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( "FGReel failed" );
            break;
        }
        int idx = attr->value();
        NET( "change fg reel #" << idx );
        fg_reel( idx );
        break;
    }
    case kBGReel:
    {
        Imf::IntAttribute* attr = dynamic_cast< Imf::IntAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( "BGReel failed" );
            break;
        }
        int idx = attr->value();
        NET( "change bg reel #" << idx );
        bg_reel( idx );
        break;
    }
    case kStopVideo:
    {
        NET( "stop at " << c.frame );
        stop();
        seek( c.frame );
        break;
    }
    case kSeek:
    {
        NET( "seek " << c.frame );
        seek( c.frame );
        break;
    }
    case kPlayForwards:
    {
        NET( "playfwd" );
        play( CMedia::kForwards );
        break;
    }
    case kPlayBackwards:
    {
        NET( "playbwd" );
        play( CMedia::kBackwards );
        break;
    }
    case kRemoveImage:
    {
        Imf::IntAttribute* attr = dynamic_cast< Imf::IntAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( "Remove Image failed" );
            break;
        }
        int idx = attr->value();
        NET("remove image " << idx );
        b->remove(idx);
        break;
    }
    case kExchangeImage:
    {
        Imf::V2iAttribute* attr = dynamic_cast< Imf::V2iAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( _("Exchange Image failed") );
            break;
        }
        const Imath::V2i& list = attr->value();
        int oldsel = list[0];
        int sel = list[1];
        NET( "exchange " << oldsel << " with " << sel );
        b->exchange(oldsel, sel);
        break;
    }
    case kICS:
    {
        Imf::StringAttribute* attr = dynamic_cast< Imf::StringAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( "ICS for image failed" );
            break;
        }
        const std::string& s = attr->value();
        NET("ICS " << s );
        mrv::media fg = foreground();
        if (fg)
        {
            CMedia* img = fg->image();
            img->ocio_input_color_space( s );
        }
        break;
    }
    case kChangeChannel:
    {
        Imf::IntAttribute* attr = dynamic_cast< Imf::IntAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( "Change chanel for image failed" );
            break;
        }
        unsigned idx = attr->value();
        NET( "Change Channel " << idx );
        if ( foreground() )
        {
            channel( idx );
        }
        break;
    }
    case kFULLSCREEN:
    {
        toggle_fullscreen();
        break;
    }
    case kPRESENTATION:
        toggle_presentation();
        break;
    case kMEDIA_INFO_WINDOW_SHOW:
        toggle_media_info(true);
        break;
    case kMEDIA_INFO_WINDOW_HIDE:
        toggle_media_info(false);
#ifdef OSX
        if ( old_interactive ) Fl::check();
#endif
        break;
    case kCOLOR_AREA_WINDOW_SHOW:
        toggle_color_area(true);
        break;
    case kCOLOR_AREA_WINDOW_HIDE:
        toggle_color_area(false);
#ifdef OSX
        if ( old_interactive ) Fl::check();
#endif
        break;
    case kCOLOR_CONTROL_WINDOW_SHOW:
        toggle_color_control(true);
        break;
    case kCOLOR_CONTROL_WINDOW_HIDE:
        toggle_color_control(false);
#ifdef OSX
        if ( old_interactive ) Fl::check();
#endif
        break;
    case k3D_VIEW_WINDOW_SHOW:
        toggle_3d_view(true);
#ifdef OSX
        if ( old_interactive ) Fl::check();
#endif
        break;
    case k3D_VIEW_WINDOW_HIDE:
        toggle_3d_view(false);
#ifdef OSX
        if ( old_interactive ) Fl::check();
#endif
        break;
    case kHISTOGRAM_WINDOW_SHOW:
        toggle_histogram(true);
        break;
    case kHISTOGRAM_WINDOW_HIDE:
        toggle_histogram(false);
#ifdef OSX
        if ( old_interactive ) Fl::check();
#endif
        break;
    case kVECTORSCOPE_WINDOW_SHOW:
        toggle_vectorscope(true);
        break;
    case kVECTORSCOPE_WINDOW_HIDE:
        toggle_vectorscope(false);
#ifdef OSX
        if ( old_interactive ) Fl::check();
#endif
        break;
    case kWAVEFORM_WINDOW_SHOW:
        toggle_waveform(true);
        break;
    case kWAVEFORM_WINDOW_HIDE:
        toggle_waveform(false);
#ifdef OSX
        if ( old_interactive ) Fl::check();
#endif
        break;
    case kSTEREO_OPTIONS_WINDOW_SHOW:
        toggle_stereo_options(true);
        break;
    case kSTEREO_OPTIONS_WINDOW_HIDE:
        toggle_stereo_options(false);
#ifdef OSX
        if ( old_interactive ) Fl::check();
#endif
        break;
    case kPAINT_TOOLS_WINDOW_SHOW:
        toggle_paint_tools(true);
        break;
    case kPAINT_TOOLS_WINDOW_HIDE:
        toggle_paint_tools(false);
#ifdef OSX
        if ( old_interactive ) Fl::check();
#endif
        break;
    case kFitImage:
    {
        fit_image();
        break;
    }
    case kRotateImage:
    {
        Imf::FloatAttribute* f = dynamic_cast< Imf::FloatAttribute* >( c.data );
        if ( !f )
        {
            LOG_ERROR( _("Rotate for image failed") );
            break;
        }

        mrv::media fg = foreground();
        if (fg)
        {
            CMedia* img = fg->image();
            NET("img->rot_z " << f->value() );
            img->rot_z( f->value() );
            img->image_damage( CMedia::kDamageContents | CMedia::kDamageData );
            redraw();
        }
        break;
    }
    case kZoomChange:
    {
        Imf::FloatAttribute* f = dynamic_cast< Imf::FloatAttribute* >( c.data );
        if ( !f )
        {
            LOG_ERROR( _("Zoom for image failed") );
            break;
        }
        NET("zoom " << f->value() );
        zoom( f->value() );
        break;
    }
    case kLUT_CHANGE:
    {
        NET( "LUT change");
        mrv::media fg = foreground();
        if ( fg )
        {
            CMedia* img = fg->image();
            img->image_damage( img->image_damage() |
                               CMedia::kDamageLut );
        }
        use_lut( true );
        break;
    }
    case kGAIN:
    {
        Imf::FloatAttribute* f = dynamic_cast< Imf::FloatAttribute* >( c.data );
        if ( !f )
        {
            LOG_ERROR( _("Gain for image failed") );
            break;
        }
        NET("gain " << f->value() );
        gain( f->value() );
        break;
    }
    case kGAMMA:
    {
        Imf::FloatAttribute* f = dynamic_cast< Imf::FloatAttribute* >( c.data );
        if ( !f )
        {
            LOG_ERROR( _("Gamma for image failed") );
            break;
        }
        NET("gamma " << f->value() );
        gamma( f->value() );
        break;
    }
    case kOCIOViewChange:
    {
        Imf::StringVectorAttribute* attr =
            dynamic_cast< Imf::StringVectorAttribute* >( c.data );
        if ( !attr )
        {
            LOG_ERROR( _("OCIO View change failed") );
            break;
        }
        const Imf::StringVector& s = attr->value();
        if ( ! s[0].empty() ) mrv::Preferences::OCIO_Display = s[0];
        if ( ! s[1].empty() )
        {
            mrv::Preferences::OCIO_View = s[1];
            uiMain->gammaDefaults->copy_label( s[1].c_str() );
            uiMain->gammaDefaults->redraw();
        }
        gamma( 1.0f );
        use_lut(true);
        uiMain->uiGammaInput->value( 1.0f );
        uiMain->uiLUT->value(true);
        break;
    }
    default:
    {
        LOG_ERROR( "Unknown mrv event size " << commands.size() << " type "
                   << c.type << " data " << c.data << " c.frame " << c.frame );
        goto final;
        break;
    }
    }  // switch


final:
    delete c.data;  c.data = NULL;
    delete c.linfo; c.linfo = NULL;

    if( !commands.empty() )  // needed
        commands.pop_front();

    _network_active = old_network_active;
    _interactive = old_interactive;
    redraw();

}



void ImageView::timeout()
{

    mrv::Timeline* timeline = this->timeline();

    // Redraw browser to update thumbnail
    mrv::ImageBrowser* b = browser();
    if ( !b || !timeline ) return;

    {
        SCOPED_LOCK( commands_mutex );
        while ( ! commands.empty()  )
        {
            handle_commands();
        }
    }

    //
    if (main() && main()->uiLog && main()->uiLog->uiMain )
        log();



    mrv::Reel reel = b->reel_at( _fg_reel );
    mrv::Reel bgreel = b->reel_at( _bg_reel );


    //
    // If in EDL mode, we check timeline to see if frame points to
    // new image.
    //
    if ( timeline->visible() )
    {
        timeline->value( double(_frame) );
        uiMain->uiFrame->value( _frame );  // so it is displayed properly
    }

    if ( uiMain->uiEDLWindow )
    {
        mrv::Timeline* t = uiMain->uiEDLWindow->uiTimeline;
        if (t && t->visible() )
        {
            t->value( double(_frame) );
        }
    }


    const mrv::media fg = foreground();
    mrv::media bg = background();

    if ( bgreel && bgreel->edl )
    {
        bg = bgreel->media_at( _frame );

        if ( bg && bg != background() )
        {
            background( bg );
        }
    }


    if ( bg && bg != fg )
    {
        CMedia* img = bg->image();
        // If not a video image check if image has changed on disk
        if ( ! img->has_video() &&
                uiMain->uiPrefs->uiPrefsAutoLoadImages->value() )
        {
            img->has_changed();
        }

    }


    double delay = 0.025;
    CMedia* img = NULL;
    if ( fg )
    {
        img = fg->image();
        delay = 0.5 / img->play_fps();

        // If not a video image check if image has changed on disk

        if ( ! img->has_video() &&
             uiMain->uiPrefs->uiPrefsAutoLoadImages->value() )
        {
            img->has_changed();
        }

    }


    // if ( timeline->visible() )
    // {

    //     if ( reel && !reel->edl && img )
    //     {
    //         int64_t frame = img->frame();

    //         if ( this->frame() != frame && playback() != CMedia::kStopped )
    //         {
    //             this->frame( frame );
    //         }
    //     }
    // }


    if ( should_update( fg ) )
    {
        redraw();  // Clear the damage to redraw it
        update_color_info();
        if ( uiMain->uiEDLWindow && uiMain->uiEDLWindow->uiEDLGroup->visible() )
            uiMain->uiEDLWindow->uiEDLGroup->redraw();
    }


    if ( vr() )
    {
        handle_vr( delay );
    }
    Fl::repeat_timeout( delay, (Fl_Timeout_Handler)static_timeout, this );

}

void ImageView::handle_vr( double& delay )
{
#define sumX 0.005
#define sumY 0.005
    bool changed = false;
    if (spinx >= sumX ) {
        spinx -= sumX;
        changed = true;
    }
    else if ( spinx <= -sumX ) {
        spinx += sumX;
        changed = true;
    }
    else spinx = 0.0;

    if (spiny >= sumY ) {
        spiny -= sumY;
        changed = true;
    }
    else if ( spiny <= -sumY ) {
        spiny += sumY;
        changed = true;
    }
    else spiny = 0.0;

    if ( changed ) {
        if ( delay > 0.03 ) delay = 0.03;
        redraw();
    }
}


void ImageView::selection( const mrv::Rectd& r )
{
    _selection = r;

    update_color_info();
}

void ImageView::redo_draw()
{
    mrv::media fg = foreground();
    if (!fg) return;

    GLShapeList& shapes = fg->image()->shapes();
    GLShapeList& undo_shapes = fg->image()->undo_shapes();
    if ( !undo_shapes.empty() )
    {
        shapes.push_back( undo_shapes.back() );
        uiMain->uiPaint->uiUndoDraw->activate();
        uiMain->uiUndoDraw->activate();
        uiMain->uiPaint->uiUndoDraw->redraw();
        uiMain->uiUndoDraw->redraw();
        undo_shapes.pop_back();
        if ( undo_shapes.empty() )
        {
            uiMain->uiPaint->uiRedoDraw->deactivate();
            uiMain->uiRedoDraw->deactivate();
        }

        send_network( "RedoDraw" );
        redraw();
        timeline()->redraw();
    }
}

void ImageView::undo_draw()
{
    GLShapeList& shapes = this->shapes();
    GLShapeList& undo_shapes = this->undo_shapes();

    if ( ! shapes.empty() )
    {
        if ( ! remove_children() ) return;
        undo_shapes.push_back( shapes.back() );
        uiMain->uiPaint->uiRedoDraw->activate();
        uiMain->uiRedoDraw->activate();
        shapes.pop_back();
        if ( shapes.empty() )
        {
            uiMain->uiPaint->uiUndoDraw->deactivate();
            uiMain->uiUndoDraw->deactivate();
        }
        send_network( "UndoDraw" );
        redraw();
        timeline()->redraw();
    }

}

void ImageView::draw_text( unsigned char r, unsigned char g, unsigned char b,
                           double x, double y, const char* t )
{
    char text[256];
    fl_utf8toa( t, (unsigned) strlen(t), text, 255 );
    _engine->color( (uchar)0, (uchar)0, (uchar)0 );
    _engine->draw_text( int(x+1), int(y-1), text ); // draw shadow
    _engine->color( r, g, b );
    _engine->draw_text( int(x), int(y), text );  // draw text
}


void ImageView::vr( VRType t )
{
    _vr = t;
    valid(0);

    const mrv::media fg = foreground();

    if ( fg )
    {
        CMedia* img = fg->image();
        img->image_damage( img->image_damage() | CMedia::kDamageContents );
    }

    const mrv::media bg = background();

    if ( bg )
    {
        CMedia* img = bg->image();
        img->image_damage( img->image_damage() | CMedia::kDamageContents );
    }

    redraw();

    char buf[32];
    if ( t == kVRSphericalMap )
        sprintf( buf, "VRSpherical 1" );
    else if ( t == kVRCubeMap )
        sprintf( buf, "VRCubic 1" );
    else
    {
        sprintf( buf, "VRSpherical 0" );
        send_network( buf );
        sprintf( buf, "VRCubic 0" );
    }
    send_network( buf );
}

/**
 * Main fltk drawing routine
 *
 */
void ImageView::draw()
{

    if ( !valid() )
    {
        if ( ! _engine )
        {
            DBGM3( __FUNCTION__ << " " << __LINE__ );
            init_draw_engine();
        }

        DBGM1( "***** NOT VALID OPENGL CONTEXT - REINIT" );

        DBGM3( "GLengine " << _engine );
        if ( !_engine ) return;


        DBGM3( __FUNCTION__ << " " << __LINE__ );
        _engine->reset_view_matrix();


        valid(1);
    }


    PreferencesUI* uiPrefs = uiMain->uiPrefs;

    //
    // Clear canvas
    //
    {
        float r, g, b, a = 0.0f;
        if ( !presentation )
        {
            uchar ur, ug, ub;
            Fl::get_color( uiPrefs->uiPrefsViewBG->color(), ur, ug, ub );
            r = ur / 255.0f;
            g = ur / 255.0f;
            b = ur / 255.0f;
        }
        else
        {
            r = g = b = a = 0.0f;
        }

        DBGM3( __FUNCTION__ << " " << __LINE__ );

        _engine->clear_canvas( r, g, b, a );

        // if ( !_update ) return;

        DBGM3( __FUNCTION__ << " " << __LINE__ );
        switch( uiPrefs->uiPrefsBlendMode->value() )
        {
        case kBlendTraditional:
        case kBlendTraditionalNonGamma:
            _engine->set_blend_function( GL_SRC_ALPHA,
                                         GL_ONE_MINUS_SRC_ALPHA );
            break;
        case kBlendPremultNonGamma:
        case kBlendPremult:
            _engine->set_blend_function( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
            break;
        }
    }


    const mrv::media& fg = foreground();
    const mrv::media& bg = background();

    ImageList images;
    images.reserve(2);

    if ( bg && bg != fg /* && ( _wipe > 0.0f || _showBG ) */ )
    {
        CMedia* img = bg->image();
        if ( img->has_picture() )
            images.push_back( img );
    }

    if ( fg )
    {
        CMedia* img = fg->image();
        if ( img->has_picture() )
        {
            _engine->image( fg->image() );
            images.push_back( img );
        }
    }

    DBGM3( __FUNCTION__ << " " << __LINE__ );
    if ( images.empty() ) return;

    CMedia* img = NULL;
    if ( fg )
    {
        img = fg->image();
        Mutex& mtx = img->video_mutex();
        SCOPED_LOCK( mtx );

        DBGM3( __FUNCTION__ << " " << __LINE__ );
        _engine->draw_images( images );
    }


    if ( ! mrv::is_equal( _masking, 0.0f ) )
    {
        _engine->draw_mask( _masking );
    }

    if ( img )
    {
        const char* label = img->label();
        if ( label )
        {
            uchar r, g, b;
            Fl::get_color( uiPrefs->uiPrefsViewTextOverlay->color(), r, g, b );


            int dx, dy;
            dx = int( (double) w() / (double)2 - (unsigned)strlen(label)*3 );
            dy = 24;

            draw_text( r, g, b, dx, dy, label );
        }
    }


    if ( !fg ) return;


    _engine->draw_annotation( img->shapes(), img );
    _engine->line_width(1.0);

    if ( _zoom_grid )
      {
        _engine->draw_grid( img, 1.0 );
      }

    if ( _grid )
    {
        _engine->draw_grid( img, _grid_size );
    }

    if ( _safeAreas )
    {
        const mrv::Recti& dpw = img->display_window();
        unsigned W = dpw.w();
        unsigned H = dpw.h();
        if ( W == 0 )
        {
            mrv::image_type_ptr pic = img->left();
            if (!pic)
            {
                W = img->width();
                H = img->height();
            }
            else
            {
                W = pic->width();
                H = pic->height();
            }
        }


        // Safe areas may change when pixel ratio is active
        double pr = 1.0;
        if ( uiMain->uiPixelRatio->value() )
        {
            pr = img->pixel_ratio();
            H *= pr;
        }

        double aspectY = (double) W / (double) H;

        if ( aspectY < 1.66 || (aspectY >= 1.77 && aspectY <= 1.78) )
        {
            // Assume NTSC/PAL
            float f = float(H) * 1.33f;
            f = f / float(W);
            _engine->color( 1.0f, 0.0f, 0.0f );
            _engine->draw_safe_area( f * 0.9f, 0.9f, N_("tv action") );
            _engine->draw_safe_area( f * 0.8f, 0.8f, N_("tv title") );

            if ( aspectY >= 1.77 )
            {
                // Draw hdtv too
                _engine->color( 1.0f, 0.0f, 1.0f );
                _engine->draw_safe_area( 1.0f, aspectY/1.77f, N_("hdtv") );
            }
        }
        else
        {
            if ( mrv::is_equal( pr, 1.0 ) )
            {
                double aspectX = (double) H / (double) W;
                // Assume film, draw 2.35, 1.85, 1.66 and hdtv areas
                _engine->color( 0.0f, 1.0f, 0.0f );
                _engine->draw_safe_area( 2.35*aspectX,
                                         1.0, _("2.35") );
                _engine->draw_safe_area( 1.89*aspectX,
                                         1.0, _("1.85") );
                _engine->draw_safe_area( 1.66*aspectX,
                                         1.0, _("1.66") );
                // Draw hdtv too
                _engine->color( 1.0f, 0.0f, 1.0f );
                _engine->draw_safe_area( 1.77*aspectX, 1.0, N_("hdtv") );
            }
            else
            {
                // Film fit for TV, Draw 4-3 safe areas
                float f = float(H) * 1.33f;
                f = f / float(W);
                _engine->color( 1.0f, 0.0f, 0.0f );
                _engine->draw_safe_area( f * 0.9f, 0.9f, N_("tv action") );
                _engine->draw_safe_area( f * 0.8f, 0.8f, N_("tv title") );
            }
        }

    }

    if ( _selection.w() > 0 || _selection.h() > 0 )
    {
        uchar r, g, b;
        Fl::get_color( uiPrefs->uiPrefsViewSelection->color(), r, g, b );
        _engine->color( r, g, b, 255 );
        if ( _zoom >= 32 ) _engine->line_width(4.0);
        _engine->draw_rectangle( _selection, img );
        if ( _zoom >= 32 ) _engine->line_width(1.0);
    }

    if ( !(flags & kMouseDown) && Fl::belowmouse() == this )
      {
          // std::cerr << "flags " << flags << " "
          //           << !(flags & kMouseDown ) <
          //     < std::endl;
          if ( (_mode & kDraw) || (_mode & kErase) ||
               (_mode & kCircle) || (_mode & kArrow ) ||
               (_mode & kRectangle) )
            {
                double xf = X;
                double yf = Y;


                data_window_coordinates( img, xf, yf );

                yf = -yf;

                const mrv::Recti& daw = img->data_window();
                xf += daw.x();
                yf -= daw.y();


                window()->cursor( FL_CURSOR_NONE );
                _engine->draw_cursor( xf, yf, _mode );
            }
          else
            {
                if ( presentation )
                  {
                    ++cursor_counter;
                    // Wait 5 seconds and hide cursor in presentation mode
                    if ( cursor_counter >= 24 * 5 )
                      {
                        cursor_counter = 0;
                        window()->cursor( FL_CURSOR_NONE );
                      }
                  }
                else
                  {
                    cursor_counter = 0;
                    window()->cursor(FL_CURSOR_CROSS);
                  }
            }
      }


    if ( _hud == kHudNone )
    {
        Fl_Gl_Window::draw();
        return;
    }

    //
    // Draw HUD
    //
    if ( vr() )
    {
        _engine->reset_vr_matrix();

    }



    std::ostringstream hud;
    hud.str().reserve( 512 );

    uchar r, g, b;
    Fl::get_color( uiPrefs->uiPrefsViewHud->color(), r, g, b );
    _engine->color( r, g, b );


#if defined( LINUX )
    int y = h() - 25;
#else
    int y = h() - 25 - 25 * presentation;
#endif
    int yi = 25;
    static char buf[1024];

    if ( _hud & kHudDirectory )
    {
        draw_text( r, g, b, 5, y, img->directory().c_str() );
        y -= yi;
    }

    int64_t frame = img->frame();

    if ( _hud & kHudFilename )
    {
        sprintf( buf, img->name().c_str(), frame );
        hud << buf;
    }

    int64_t last = img->last_frame();
    if ( uiMain->uiEndFrame->frame() < last )
        last = uiMain->uiEndFrame->frame();
    int64_t first = img->first_frame();
    if ( uiMain->uiStartFrame->frame() > first )
        first = uiMain->uiStartFrame->frame();

    if ( first != last && _hud & kHudFrameRange )
    {
        if ( !hud.str().empty() ) hud << " ";
        hud << first << " - " << last;
    }

    if ( _hud & kHudFrameCount )
    {
        hud << N_(" FC: ") << ( last - first + 1 );
    }

    if ( !hud.str().empty() )
    {
        draw_text( r, g, b, 5, y, hud.str().c_str() );
        y -= yi;
        hud.str("");
    }


    if ( _hud & kHudResolution )
    {
        mrv::Recti d = img->data_window();
        if ( d.w() == 0 )
        {
            // Work around for data window not set yet (use previous frame)
            d = img->data_window( frame-1 );
        }
        sprintf( buf, N_("DAW: %d,%d %dx%d"), d.x(), d.y(), d.w(), d.h() );
        draw_text( r, g, b, 5, y, buf );
        y -= yi;
        {
            const mrv::Recti& d = img->display_window();
            sprintf( buf, N_("DYW: %d,%d %dx%d"), d.x(), d.y(), d.w(), d.h() );
            draw_text( r, g, b, 5, y, buf );
            y -= yi;
        }
    }

    if ( _hud & kHudFrame )
    {
        sprintf( buf, "% 4" PRId64, frame );
        hud << N_("F: ") << buf;
    }

    if ( _hud & kHudTimecode )
    {
        mrv::Timecode::Display d = uiMain->uiFrame->display();
        if ( d != mrv::Timecode::kTimecodeDropFrame )
            d = mrv::Timecode::kTimecodeNonDrop;

        mrv::Timecode::format( buf, d, frame, img->timecode(),
                               img->play_fps(), true );
        if ( !hud.str().empty() ) hud << " ";
        hud << N_("T: ") << buf;
    }

    if ( (_hud & kHudAVDifference) && img->has_audio() )
    {
        double avdiff = img->avdiff();
        if ( !hud.str().empty() ) hud << " ";
        sprintf( buf, "% 4f", avdiff );
        hud << N_("V-A: ") << buf;
    }


    //
    // Calculate and draw fps
    //
    if ( _hud & kHudFPS )
    {
        static uint64_t unshown_frames = 0;
        static CMedia* oldimg = NULL;

        CMedia::Playback p = playback();

        if ( img == oldimg && img->start_frame() != img->end_frame() )
        {
            if ((p == CMedia::kForwards  && _lastFrame < frame ) ||
                (p == CMedia::kBackwards && _lastFrame > frame ))
            {
                float ifps = img->fps();
                float fps = img->play_fps() / ifps;
                int64_t frame_diff = ( frame - _lastFrame ) / fps;
                int64_t absdiff = std::abs(frame_diff);
                if ( absdiff > 1 && absdiff < 11 )
                {
                    unshown_frames += absdiff - 1;
                }

                _lastFrame = frame;
            }
        }
        oldimg = img;


        {
            sprintf( buf, N_(" UF: %" PRId64 " "), unshown_frames );
            hud << buf;
            sprintf( buf, N_("FPS: %.3f" ), img->actual_frame_rate() );
            hud << buf;
        }


        if ( !hud.str().empty() )
        {
            draw_text( r, g, b, 5, y, hud.str().c_str() );
            hud.str("");
            y -= yi;
        }

    }

    if ( _hud & kHudWipe )
    {
        if ( _wipe_dir == kWipeVertical )
            hud << N_("Wipe V");
        if ( _wipe_dir == kWipeHorizontal )
            hud << N_("Wipe H");
    }

    if ( !hud.str().empty() )
    {
        draw_text( r, g, b, 5, y, hud.str().c_str() );
        hud.str("");
        y -= yi;
    }

    if ( _hud & kHudMemoryUse )
    {
        char buf[128];
        uint64_t totalVirtualMem = 0;
        uint64_t virtualMemUsed = 0;
        uint64_t virtualMemUsedByMe = 0;
        uint64_t totalPhysMem = 0;
        uint64_t physMemUsed = 0;
        uint64_t physMemUsedByMe = 0;
        memory_information( totalVirtualMem, virtualMemUsed, virtualMemUsedByMe,
                            totalPhysMem, physMemUsed, physMemUsedByMe );

        sprintf( buf, _("PMem: %" PRIu64 "/%" PRIu64
                        " MB  VMem: %" PRIu64 "/%" PRIu64 " MB"),
                 physMemUsedByMe, totalPhysMem,
                 virtualMemUsedByMe, totalVirtualMem );

        draw_text( r, g, b, 5, y, buf );
        y -= yi;
    }

    if ( _hud & kHudAttributes )
    {
        const CMedia::Attributes& attributes = img->attributes();
        CMedia::Attributes::const_iterator i = attributes.begin();
        CMedia::Attributes::const_iterator e = attributes.end();
        for ( ; i != e; ++i )
        {
            std::string val = CMedia::attr2str( i->second );
            std::string text = i->first + ": " + val;
            draw_text( r, g, b, 5, y, text.c_str() );
            y -= yi;
        }
    }


    if ( vr() )
    {
        _engine->restore_vr_matrix();

    }

//#define DEBUG_GL_CROSS
#ifdef DEBUG_GL_CROSS
    int W = w();
    int H = h();
    glColor4f( 1, 0, 0, 1 );
    glBegin(GL_LINE_STRIP); glVertex2f(0,0); glVertex2f(W,H); glEnd();
    glBegin(GL_LINE_STRIP); glVertex2f(0,H); glVertex2f(W,0); glEnd();
#endif

    Fl_Gl_Window::draw();

}


GLShapeList& ImageView::shapes()
{
    static GLShapeList empty;
    mrv::media fg = foreground();
    if (!fg) return empty;
    return fg->image()->shapes();
}

GLShapeList& ImageView::undo_shapes()
{
    static GLShapeList empty;
    mrv::media fg = foreground();
    if (!fg) return empty;
    return fg->image()->undo_shapes();
}

void ImageView::add_shape( mrv::shape_type_ptr s )
{
    mrv::media fg = foreground();
    if (!fg) {
        LOG_ERROR( _("No image to add shape to") );
        return;
    }

    fg->image()->add_shape( s );
    uiMain->uiPaint->uiUndoDraw->activate();
    uiMain->uiPaint->uiRedoDraw->deactivate();
    uiMain->uiUndoDraw->activate();
    uiMain->uiRedoDraw->deactivate();
}

int sign (const Imath::V2i& p1,
          const Imath::V2i& p2,
          const Imath::V2i& p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle (const Imath::V2i& pt,
                      const Imath::V2i& v1,
                      const Imath::V2i& v2,
                      const Imath::V2i& v3)
{
    bool b1, b2, b3;

    b1 = sign(pt, v1, v2) < 0.0f;
    b2 = sign(pt, v2, v3) < 0.0f;
    b3 = sign(pt, v3, v1) < 0.0f;

    return ((b1 == b2) && (b2 == b3));
}


 void ImageView::fill_menu( Fl_Menu_* menu )
 {
     SCOPED_LOCK( _shortcut_mutex );

     menu->clear();
     int idx = 1;

     menu->add( _("File/Open/Movie or Sequence"),
                      kOpenImage.hotkey(),
                      (Fl_Callback*)open_cb, browser() );

     menu->add( _("File/Open/Single Image"), kOpenSingleImage.hotkey(),
                (Fl_Callback*)open_single_cb, browser() );

     mrv::media fg = foreground();
     if ( !fg )
     {
         menu->add( _("File/Open/Directory"), kOpenDirectory.hotkey(),
                          (Fl_Callback*)open_dir_cb, browser() );
         menu->add( _("File/Open/Session"),
                    kOpenSession.hotkey(),
                    (Fl_Callback*)open_session_cb, browser() );
     }
     if ( fg )
     {
         menu->add( _("File/Open/Stereo Sequence or Movie"),
                    kOpenStereoImage.hotkey(),
                    (Fl_Callback*)open_stereo_cb, browser() );
         menu->add( _("File/Open/Directory"), kOpenDirectory.hotkey(),
                    (Fl_Callback*)open_dir_cb, browser() );
         menu->add( _("File/Open/AMF file"), kOpenAMF.hotkey(),
                    (Fl_Callback*)open_amf_cb, browser() );
         idx = menu->add( _("File/Open/Session"),
                    kOpenSession.hotkey(),
                    (Fl_Callback*)open_session_cb, browser() );
         menu->add( _("File/Save/Movie or Sequence As"),
                    kSaveSequence.hotkey(),
                    (Fl_Callback*)save_sequence_cb, this );
         menu->add( _("File/Save/Reel As"), kSaveReel.hotkey(),
                    (Fl_Callback*)save_reel_cb, this );
         menu->add( _("File/Save/Frame As"), kSaveImage.hotkey(),
                    (Fl_Callback*)save_cb, this );
         menu->add( _("File/Save/GL Snapshots As"), kSaveSnapshot.hotkey(),
                    (Fl_Callback*)save_snap_cb, this );
         menu->add( _("File/Save/Session As"),
                    kSaveSession.hotkey(),
                    (Fl_Callback*)save_session_as_cb, this );
         idx += 2;
     }

     Fl_Menu_Item* item = (Fl_Menu_Item*) &menu->menu()[idx];

     if ( dynamic_cast< Fl_Menu_Bar* >( menu ) )
     {
         item->flags |= FL_MENU_DIVIDER;
         menu->add( _("File/Quit"), kQuitProgram.hotkey(),
                    (Fl_Callback*)exit_cb, uiMain );
     }


     char buf[256];
     int num = uiMain->uiWindows->children() - 1;
     int i;
     for ( i = 0; i < num; ++i )
     {
         std::string tmp = uiMain->uiWindows->child(i)->label();

         // Quote any / to avoid submenu ( A/B Options for example ).
         size_t pos = tmp.find( '/' );
         if ( pos != std::string::npos )
         {
             tmp = tmp.substr( 0, pos ) + '\\' +
                   tmp.substr( pos, tmp.size() );
         }
         unsigned hotkey = 0;
         if ( tmp == _("Reels") ) hotkey = kToggleReel.hotkey();
         else if ( tmp == _("Media Info") ) hotkey = kToggleMediaInfo.hotkey();
         else if ( tmp == _("Color Info") ) hotkey = kToggleColorInfo.hotkey();
         else if ( tmp == _("Color Controls") )
             hotkey = kToggleColorControls.hotkey();
         else if ( tmp == _("Action Tools") ) hotkey = kToggleAction.hotkey();
         else if ( tmp == _("A-B, Stereo 3D Options") )
             hotkey = kToggleStereoOptions.hotkey();
         else if ( tmp == _("Preferences") )
             hotkey = kTogglePreferences.hotkey();
         else if ( tmp == _("EDL Edit") )
             hotkey = kToggleEDLEdit.hotkey();
         else if ( tmp == _("3dView") )
             hotkey = kToggle3dView.hotkey();
         else if ( tmp == _("Histogram") )
             hotkey = kToggleHistogram.hotkey();
         else if ( tmp == _("Vectorscope") )
             hotkey = kToggleVectorscope.hotkey();
         else if ( tmp == _("Waveform") )
             hotkey = kToggleWaveform.hotkey();
         else if ( tmp == _("ICC Profiles") )
             hotkey = kToggleICCProfiles.hotkey();
         else if ( tmp == _("Connections") )
             hotkey = kToggleConnections.hotkey();
         else if ( tmp == _("Hotkeys") )
             hotkey = kToggleHotkeys.hotkey();
         else if ( tmp == _("Logs") )
             hotkey = kToggleLogs.hotkey();
         else if ( tmp == _("About") )
             hotkey = kToggleAbout.hotkey();
         tmp = _("Windows/") + tmp;
         menu->add( tmp.c_str(), hotkey, (Fl_Callback*)window_cb, uiMain );
     }


     if ( fg && fg->image()->has_picture() )
     {


         menu->add( _("View/Safe Areas"), kSafeAreas.hotkey(),
                    (Fl_Callback*)safe_areas_cb, this );

         idx = menu->add( _("View/Display Window"), kDisplayWindow.hotkey(),
                          (Fl_Callback*)display_window_cb, this,
                          FL_MENU_TOGGLE );
         item = (Fl_Menu_Item*) &(menu->menu()[idx]);
         if ( display_window() ) item->set();

         idx = menu->add( _("View/Data Window"), kDataWindow.hotkey(),
                          (Fl_Callback*)data_window_cb, this, FL_MENU_TOGGLE );
         item = (Fl_Menu_Item*) &(menu->menu()[idx]);
         if ( data_window() ) item->set();

         idx = menu->add( _("View/Texture Filtering  "),
                          kTextureFiltering.hotkey(),
                          (Fl_Callback*)texture_filtering_cb, this,
                          FL_MENU_TOGGLE );
         item = (Fl_Menu_Item*) &(menu->menu()[idx]);
         if ( texture_filtering() == ImageView::kBilinearFiltering )
             item->set();


         sprintf( buf, "%s", _("View/Toggle Action Dock") );
         idx = menu->add( buf, kToggleToolBar.hotkey(),
                          (Fl_Callback*)toggle_action_tool_dock, uiMain,
                          FL_MENU_TOGGLE );
         item = (Fl_Menu_Item*) &(menu->menu()[idx]);
         if ( uiMain->uiToolsGroup->visible() )
             item->set();

         const char* tmp;
         num = uiMain->uiPrefs->uiPrefsCropArea->children();
         for ( i = 0; i < num; ++i )
         {
             tmp = uiMain->uiPrefs->uiPrefsCropArea->child(i)->label();
             if ( !tmp ) continue;
             sprintf( buf, _("View/Mask/%s"), tmp );
             idx = menu->add( buf, 0, (Fl_Callback*)masking_cb, uiMain,
                              FL_MENU_RADIO );
             item = (Fl_Menu_Item*) &(menu->menu()[idx]);
             float mask = kCrops[i];
             if ( mrv::is_equal( mask, _masking ) ) item->set();
         }


         sprintf( buf, "%s", _("View/Grid/Toggle Selected") );
         menu->add( buf, kGridToggle.hotkey(),
                    (Fl_Callback*)grid_toggle_cb, this );

         sprintf( buf, "%s", _("View/Grid/Size") );
         menu->add( buf, kGridSize.hotkey(),
                    (Fl_Callback*)grid_size_cb, this );

         sprintf( buf, "%s", _("View/Hud/Toggle Selected") );
         menu->add( buf, kHudToggle.hotkey(),
                    (Fl_Callback*)hud_toggle_cb, uiMain );

         num = uiMain->uiPrefs->uiPrefsHud->children();
         for ( i = 0; i < num; ++i )
         {
             tmp = uiMain->uiPrefs->uiPrefsHud->child(i)->label();
             sprintf( buf, _("View/Hud/%s"), tmp );
             idx = menu->add( buf, 0, (Fl_Callback*)hud_cb, uiMain,
                              FL_MENU_TOGGLE );
             item = (Fl_Menu_Item*) &(menu->menu()[idx]);
             if ( hud() & (1 << i) ) item->set();
         }


         bool has_version = false;

         CMedia* img = fg->image();
         std::string file = img->fileroot();
         file = fs::path( file ).leaf().string();
         std::string dir = img->directory();
         file = dir + "/" + file;

         size_t pos = 0;

         PreferencesUI* prefs = main()->uiPrefs;
         std::string prefix = prefs->uiPrefsImageVersionPrefix->value();
         if ( (pos = file.find( prefix, pos) ) != std::string::npos )
             has_version = true;

         if ( has_version )
         {
             menu->add( _("Version/First"), kFirstVersionImage.hotkey(),
                        (Fl_Callback*)first_image_version_cb, browser(),
                        FL_MENU_DIVIDER);
             menu->add( _("Version/Next"), kNextVersionImage.hotkey(),
                        (Fl_Callback*)next_image_version_cb, browser());
             menu->add( _("Version/Previous"),
                        kPreviousVersionImage.hotkey(),
                        (Fl_Callback*)previous_image_version_cb,
                        browser(), FL_MENU_DIVIDER);
             menu->add( _("Version/Last"),
                        kLastVersionImage.hotkey(),
                        (Fl_Callback*)last_image_version_cb,
                        browser(), FL_MENU_DIVIDER);
         }


         menu->add( _("Image/Next"), kNextImage.hotkey(),
                    (Fl_Callback*)next_image_cb, browser());
         menu->add( _("Image/Previous"), kPreviousImage.hotkey(),
                    (Fl_Callback*)previous_image_cb,
                    browser(), FL_MENU_DIVIDER);


         const stubImage* simg = dynamic_cast< const stubImage* >( image() );

         if ( simg )
         {
             menu->add( _("Image/Clone"), kCloneImage.hotkey(),
                        (Fl_Callback*)clone_image_cb, browser());
             menu->add( _("Image/Clone All Channels"), 0,
                        (Fl_Callback*)clone_all_cb,
                        browser(), FL_MENU_DIVIDER);
         }
         else
         {
             menu->add( _("Image/Clone"), kCloneImage.hotkey(),
                        (Fl_Callback*)clone_image_cb, browser(),
                        FL_MENU_DIVIDER );
         }



         idx = menu->add( _("Image/Preload Caches"),
                          kPreloadCache.hotkey(),
                          (Fl_Callback*)preload_image_cache_cb, this,
                          FL_MENU_TOGGLE );
         item = (Fl_Menu_Item*) &(menu->menu()[idx]);
         if ( CMedia::preload_cache() ) item->set();



         menu->add( _("Image/Clear Caches"), kClearCache.hotkey(),
                    (Fl_Callback*)clear_image_cache_cb, this );

         menu->add( _("Image/Update Single Frame in Cache"),
                    kClearSingleFrameCache.hotkey(),
                    (Fl_Callback*)update_frame_cb, this,
                    FL_MENU_DIVIDER );



         menu->add( _("Image/Rotate +90"),
                    kRotatePlus90.hotkey(),
                    (Fl_Callback*)rotate_plus_90_cb, this );
         menu->add( _("Image/Rotate -90"),
                    kRotateMinus90.hotkey(),
                    (Fl_Callback*)rotate_minus_90_cb, this,
                    FL_MENU_DIVIDER );

         if ( !Preferences::use_ocio )
         {


             menu->add( _("Image/Attach CTL Input Device Transform"),
                        kIDTScript.hotkey(),
                        (Fl_Callback*)attach_ctl_idt_script_cb,
                        browser() );
             menu->add( _("Image/Modify CTL ASC_CDL SOP Saturation"),
                        kSOPSatNodes.hotkey(),
                        (Fl_Callback*)modify_sop_sat_cb,
                        this);
             menu->add( _("Image/Add CTL Look Mod Transform"),
                        kLookModScript.hotkey(),
                        (Fl_Callback*)attach_ctl_lmt_script_cb,
                        this);
             menu->add( _("Image/Attach CTL Rendering Transform"),
                        kCTLScript.hotkey(),
                        (Fl_Callback*)attach_ctl_script_cb,
                        this, FL_MENU_DIVIDER);
             menu->add( _("Image/Attach ICC Color Profile"),
                        kIccProfile.hotkey(),
                        (Fl_Callback*)attach_color_profile_cb,
                        this, FL_MENU_DIVIDER);
         }


         menu->add( _("Image/Mirror/Horizontal"),
                    kFlipX.hotkey(),
                    (Fl_Callback*)flip_x_cb,
                    this);
         menu->add( _("Image/Mirror/Vertical"),
                    kFlipY.hotkey(),
                    (Fl_Callback*)flip_y_cb,
                    this);
         menu->add( _("Image/Set as Background"), kSetAsBG.hotkey(),
                    (Fl_Callback*)set_as_background_cb,
                    (void*)this);


         menu->add( _("Image/Switch FG and BG"),
                    kSwitchFGBG.hotkey(),
                    (Fl_Callback*)switch_fg_bg_cb, (void*)this);
         menu->add( _("Image/Toggle Background"),
                    kToggleBG.hotkey(),
                    (Fl_Callback*)toggle_background_cb, (void*)this);
         mrv::ImageBrowser* b = browser();
         mrv::Reel reel = b->current_reel();
         if ( reel->images.size() > 1 )
         {
             menu->add( _("Image/Toggle EDL"),
                        kToggleEDL.hotkey(),
                        (Fl_Callback*)toggle_edl_cb, (void*)this);
         }

         Image_ptr image = fg->image();


         if ( Preferences::use_ocio )
         {


             menu->add( _("OCIO/Input Color Space"),
                        kOCIOInputColorSpace.hotkey(),
                        (Fl_Callback*)attach_ocio_ics_cb, (void*)browser());

             menu->add( _("OCIO/Display"),
                        kOCIODisplay.hotkey(),
                        (Fl_Callback*)attach_ocio_display_cb, (void*)this);
             menu->add( _("OCIO/View"),
                        kOCIOView.hotkey(),
                        (Fl_Callback*)attach_ocio_view_cb, (void*)this);
         }




         size_t num = image->number_of_video_streams();
         if ( num > 1 )
         {


             for ( unsigned i = 0; i < num; ++i )
             {
                 char buf[256];
                 sprintf( buf, _("Video/Track #%d - %s"), i,
                          image->video_info(i).language.c_str() );

                 idx = menu->add( buf, 0,
                                  (Fl_Callback*)change_video_cb, this,
                                  FL_MENU_RADIO );
                 item = (Fl_Menu_Item*) &(menu->menu()[idx]);
                 if ( image->video_stream() == (int) i )
                     item->set();
             }
         }

         num = image->number_of_subtitle_streams();

         if ( dynamic_cast< aviImage* >( image ) != NULL )
         {
             menu->add( _("Subtitle/Load"), 0,
                        (Fl_Callback*)load_subtitle_cb, uiMain );
         }



         if ( num > 0 )
         {
             idx = menu->add( _("Subtitle/No Subtitle"), 0,
                              (Fl_Callback*)change_subtitle_cb, this,
                              FL_MENU_TOGGLE  );
             Fl_Menu_Item* item = (Fl_Menu_Item*) &(menu->menu()[idx]);
             if ( image->subtitle_stream() == -1 )
                 item->set();
             for ( unsigned i = 0; i < num; ++i )
             {
                 char buf[256];
                 sprintf( buf, _("Subtitle/Track #%d - %s"), i,
                          image->subtitle_info(i).language.c_str() );

                 idx = menu->add( buf, 0,
                                  (Fl_Callback*)change_subtitle_cb, this, FL_MENU_RADIO );
                 item = (Fl_Menu_Item*) &(menu->menu()[idx]);
                 if ( image->subtitle_stream() == (int)i )
                     item->set();
             }
         }



         if ( 1 )
         {

             menu->add( _("Audio/Attach Audio File"), kAttachAudio.hotkey(),
                        (Fl_Callback*)attach_audio_cb, this );
             menu->add( _("Audio/Edit Audio Frame Offset"),
                        kEditAudio.hotkey(),
                        (Fl_Callback*)edit_audio_cb, this );
             menu->add( _("Audio/Detach Audio File"), kDetachAudio.hotkey(),
                        (Fl_Callback*)detach_audio_cb, this );
         }



         if ( dynamic_cast< Fl_Menu_Button* >( menu ) )
         {
             menu->add( _("Pixel/Copy RGBA Values to Clipboard"),
                        kCopyRGBAValues.hotkey(),
                        (Fl_Callback*)copy_pixel_rgba_cb, (void*)this);
         }

         if ( !Preferences::use_ocio )
         {



             menu->add( _("Monitor/Attach CTL Display Transform"),
                        kMonitorCTLScript.hotkey(),
                        (Fl_Callback*)monitor_ctl_script_cb,
                        uiMain);
             menu->add( _("Monitor/Attach ICC Color Profile"),
                        kMonitorIccProfile.hotkey(),
                        (Fl_Callback*)monitor_icc_profile_cb,
                        uiMain, FL_MENU_DIVIDER);
         }
     }


     menu->menu_end();
     menu->redraw();
 }

unsigned ImageView::font_height() {
    return mrv::font_size;
}

/**
 * Handle a mouse press
 *
 * @param x Fl::event_x() coordinate
 * @param y Fl::event_y() coordinate
 */
int ImageView::leftMouseDown(int x, int y)
{
    lastX = x;
    lastY = y;



    flags	|= kMouseDown;

#ifdef DEBUG_KEYS
    std::cerr << "MOUSEDOWN flags " << flags << std::endl;
    std::cerr << "\tMouseDown: " << (flags & kMouseDown) << std::endl;
    std::cerr << "\tMouseLeft: " << (flags & kMouseLeft) << std::endl;
    std::cerr << "\tMouseMiddle: " << (flags & kMouseMiddle) << std::endl;
    std::cerr << "\tMouseRight: " << (flags & kMouseRight) << std::endl;
    std::cerr << "\tMouseMove: " << (flags & kMouseMove) << std::endl;
    std::cerr << "\tLeftAlt: " << (flags & kLeftAlt) << std::endl;
    std::cerr << "\tLeftShift: " << (flags & kLeftShift) << std::endl;
    std::cerr << "\tLeftCtrl: " << (flags & kLeftCtrl) << std::endl
              << std::endl;
    std::cerr << "\tGain: " << (flags & kGain) << std::endl;
    std::cerr << "\tGamma: " << (flags & kGamma) << std::endl;
#endif

    int button = Fl::event_button();
#ifdef __APPLE__
    int ev_state = Fl::event_state();
    // On apple, ctrl+left click as right click
    // if ( (ev_state & (FL_CTRL | FL_BUTTON1)) == (FL_CTRL | FL_BUTTON1) )
    //     button = FL_RIGHT_MOUSE;
#endif
    if ( button == FL_LEFT_MOUSE )
    {
        flags	|= kMouseLeft;
        if (Fl::event_key(FL_Alt_L) || vr() )
        {
            // Handle ALT+LMB moves
          flags |= kLeftAlt;
          flags |= kMouseMove;
          flags |= kMouseMiddle;
          flags &= ~kMouseLeft;
          return 1;
        }

        if ( Fl::event_state( FL_SHIFT ) )
        {
            flags |= kLeftShift;
            flags &= ~kGain;
            if ( Fl::event_state( FL_CTRL ) )
            {
                flags |= kLeftCtrl;
                flags |= kGamma;
            }
            else
            {
              flags &= ~kLeftCtrl;
              flags &= ~kGamma;
              selection_mode( true );
            }
        }
        else if ( Fl::event_state( FL_CTRL ) )
        {
          flags &= ~kLeftShift;
          flags |= kLeftCtrl;
          flags |= kGain;


          if ( Fl::event_clicks() > 1 )
          {
              gain(1.0);
              Fl::event_clicks(0);
          }
        }

        if ( _mode & kSelection )
        {
            _selection = mrv::Rectd( 0, 0, 0, 0 );
            update_color_info();
            send_selection();
            return 1;
        }
        else if ( _mode == kCopyFrameXY )
        {
            copy_frame_xy();
            return 1;
        }
        else if ( _mode == kMovePicture || _mode == kScalePicture )
        {
            ImageList images;
            images.reserve(2);

            mrv::media fg = foreground();
            mrv::media bg = background();

            if ( fg )
            {

                CMedia* img = fg->image();
                if ( img->has_picture() )
                {
                    images.push_back( img );
                }

            }

            if ( bg && bg != fg )
            {

                CMedia* img = bg->image();

                if ( img->has_picture() )
                    images.push_back( img );

            }

            ImageList::const_iterator i = images.begin();
            ImageList::const_iterator e = images.end();


            _selected_image = NULL;

            for ( ; i != e; ++i )
            {
                CMedia* img = *i;

                mrv::image_type_ptr pic;
                bool outside = false;
                int off[2];
                int xp, yp, w, h;
                picture_coordinates( img, x, y, outside, pic, xp, yp, w, h,
                                     off );

                // std::cerr << "x,y " << x << ", " << y << " outside " << outside
                //           << std::endl;
                CMedia::Pixel p;
                if ( xp >= 0 && ( xp < pic->width()*img->scale_x() ) &&
                        yp >= 0 && ( yp < pic->height()*img->scale_y() ) )
                {
                    Imath::V2i pt( xp, yp );
                    int W = int( pic->width() * img->scale_x() );
                    int H = int( pic->height() * img->scale_y() );
                    const int kSize = 30;
                    Imath::V2i v1( W, H );
                    Imath::V2i v2( W-kSize, H );
                    Imath::V2i v3( W, H-kSize );
                    if ( Fl::event_state( FL_CTRL ) ||
                         PointInTriangle( pt, v1, v2, v3 ) )
                    {
                        scale_pic_mode();
                        _selected_image = img;
                        return 1;
                    }
                    p = pic->pixel( xp, yp );
                }
                if ( outside || !pic || (img->has_alpha() && p.a < 0.0001f) ) {
                    // draw image without borders (in case this was selected
                    // before).
                    img->image_damage( CMedia::kDamageContents );
                    continue;
                }

                move_pic_mode();
                _selected_image = img;
                break;
            }

            return 1;
        }
        else if ( (_mode & kDraw) || (_mode & kErase) || (_mode == kCircle) ||
                  (_mode & kArrow) || (_mode & kRectangle) || _mode == kText )
        {

            _selection = mrv::Rectd( 0, 0, 0, 0 );
            update_color_info();
            send_selection();

            mrv::media fg = foreground();
            if ( !fg ) return 0;

            CMedia* img = fg->image();
            if (!img) return 0;

            double xf = x;
            double yf = y;

            data_window_coordinates( img, xf, yf );

            yf = -yf;

            const mrv::Recti& daw = img->data_window();
            xf += daw.x();
            yf -= daw.y();


            std::string str;
            GLShape* s;
            if ( _mode & kDraw )
            {
                s = new GLPathShape;
            }
            else if ( _mode & kErase )
            {
                s = new GLErasePathShape;
            }
            else if ( _mode & kCircle )
            {
                 s = new GLCircleShape;
            }
            else if ( _mode & kArrow )
            {
                 s = new GLArrowShape;
            }
            else if ( _mode & kRectangle )
            {
                 s = new GLRectangleShape;
            }
            else if ( _mode == kText )
            {
                bool found = false;
                MultilineInput* w;
                GLTextShape* t;
                for ( int i = 0; i < children(); ++i )
                {
                    w = dynamic_cast< MultilineInput* >( child(i) );
                    if ( !w ) continue;
                    found = true; break;
                }
                if ( ! found )
                {
                    w = new MultilineInput( x, y, 20, ( mrv::font_size + 24 ) *
                                            zoom() );
                    w->take_focus();
                    uchar r, g, b;
                    Fl::get_color( uiMain->uiPaint->uiPenColor->color(),
                                   r, g, b );
                    w->textfont( mrv::font_current );
                    w->textsize( mrv::font_size * zoom() );
                    w->font_size( mrv::font_size * zoom() );
                    w->textcolor( fl_rgb_color( r, g, b ) );

                    this->add( w );

                    t = new GLTextShape;
                    t->font( mrv::font_current );
                    t->size( mrv::font_size );
                    s = t;
                }
                else
                {
                    const GLShapeList& shapes = this->shapes();
                    if ( shapes.empty() ) return 0;
                    t = dynamic_cast< GLTextShape* >( shapes.back().get() );
                    if ( !t ) return 0;
                    t->pts[0] = mrv::Point( xf, yf );
                    w->Fl_Widget::position( x, y );
                    redraw();
                    return 1;
                }
            }
            else
            {
                return 1;
            }



            uchar r, g, b;
            Fl::get_color( uiMain->uiPaint->uiPenColor->color(), r, g, b );

            s->r = r / 255.0f;
            s->g = g / 255.0f;
            s->b = b / 255.0f;
            s->a = 1.0f;
            s->pen_size = (float) uiMain->uiPaint->uiPenSize->value();
            if ( _mode & kErase ) s->pen_size *= 2;

            if ( uiMain->uiPaint->uiAllFrames->value() )
            {
                s->frame = MRV_NOPTS_VALUE;
            }
            else
            {
                s->frame = frame();
            }

            if ( dynamic_cast< GLCircleShape* >( s ) )
                {
                    GLCircleShape* c = static_cast< GLCircleShape* >( s );
                    c->center = mrv::Point( xf, yf );
                }
            else
                {
                    GLPathShape* path = nullptr;
                    if ( ( path = dynamic_cast< GLArrowShape* >( s ) ) )
                    {
                        mrv::Point p( xf, yf );
                        path->pts.push_back( p );
                        path->pts.push_back( p );
                        path->pts.push_back( p );
                        path->pts.push_back( p );
                        path->pts.push_back( p );
                    }
                    else if ( ( path = dynamic_cast< GLRectangleShape* >( s )) )
                    {
                        mrv::Point p( xf, yf );
                        path->pts.push_back( p );
                        path->pts.push_back( p );
                    }
                    else if ( ( path = dynamic_cast< GLPathShape* >( s ) ) )
                    {
                        mrv::Point p( xf, yf );
                        path->pts.push_back( p );
                    }
                }

            send_network( str );

            add_shape( mrv::shape_type_ptr(s) );
        }

        if ( _wipe_dir != kNoWipe )
        {
            _wipe_dir = (WipeDirection) (_wipe_dir | kWipeFrozen);
            window()->cursor(FL_CURSOR_CROSS);
        }



        redraw();
        return 1;
    }
    else if ( button == FL_MIDDLE_MOUSE )
    {
        // handle MMB moves
        flags |= kMouseMove;
        flags |= kMouseMiddle;
        return 1;
    }
    else
    {
        if ( (flags & kLeftAlt) == 0 )
        {
            fill_menu( menu );
            menu->popup();
            return 1;
        }
        else
        {
            flags |= kMouseRight;
            flags |= kZoom;
        }
        return 1;
    }

    if ( (flags & kLeftAlt) && (flags & kMouseLeft) && (flags & kMouseMiddle) )
    {
        flags |= kZoom;
        return 1;
    }


    return 0;
}


/**
 * Handle a mouse release
 *
 * @param x Fl::event_x() coordinate
 * @param y Fl::event_y() coordinate
 */
void ImageView::leftMouseUp( int x, int y )
{

    flags &= ~kMouseDown;
    flags &= ~kMouseMove;
    flags &= ~kZoom;
    flags &= ~kGain;
    flags &= ~kGamma;

#ifdef DEBUG_KEYS
    std::cerr << "MOUSEUP flags " << flags << std::endl;
    std::cerr << "\tMouseDown: " << (flags & kMouseDown) << std::endl;
    std::cerr << "\tMouseLeft: " << (flags & kMouseLeft) << std::endl;
    std::cerr << "\tMouseMiddle: " << (flags & kMouseMiddle) << std::endl;
    std::cerr << "\tMouseRight: " << (flags & kMouseRight) << std::endl;
    std::cerr << "\tMouseMove: " << (flags & kMouseMove) << std::endl;
    std::cerr << "\tLeftAlt: " << (flags & kLeftAlt) << std::endl;
    std::cerr << "\tLeftShift: " << (flags & kLeftShift) << std::endl;
    std::cerr << "\tLeftCtrl: " << (flags & kLeftCtrl) << std::endl
              << std::endl;
    std::cerr << "\tGain: " << (flags & kGain) << std::endl;
    std::cerr << "\tGamma: " << (flags & kGamma) << std::endl;
#endif

    int button = Fl::event_button();
#ifdef __APPLE__
    int ev_state = Fl::event_state();
    // On apple, ctrl+left click as right click
    if ( (ev_state & (FL_CTRL | FL_BUTTON1)) == (FL_CTRL | FL_BUTTON1) )
        button = FL_RIGHT_MOUSE;
#endif
    if ( button == FL_LEFT_MOUSE )
        flags &= ~kMouseLeft;
    else if ( button == FL_MIDDLE_MOUSE )
        flags &= ~kMouseMiddle;
    else
        flags &= ~kMouseRight;

    if ( _mode & kSelection )
    {
        send_selection();
    }

    if ( _mode & kTemporary )
    {
        scrub_mode();
        return;
    }

    if ( !presentation )
        window()->cursor( FL_CURSOR_CROSS );

    mrv::media fg = foreground();

    if ( !fg || fg->image()->shapes().empty() ) return;

    //
    // Send the shapes over the network
    //
    if ( _mode & kDraw )
    {
        mrv::shape_type_ptr o = fg->image()->shapes().back();
        GLPathShape* s = dynamic_cast< GLPathShape* >( o.get() );
        if ( s == NULL )
        {
            LOG_ERROR( _("Not a GLPathShape pointer") );
        }
        else
        {
            send_network( s->send() );
            timeline()->redraw();
        }
    }
    else if ( _mode & kErase )
    {
        mrv::shape_type_ptr o = fg->image()->shapes().back();
        GLPathShape* s = dynamic_cast< GLErasePathShape* >( o.get() );
        if ( s == NULL )
        {
            LOG_ERROR( _("Not a GLErasePathShape pointer") );
        }
        else
        {
            send_network( s->send() );
            timeline()->redraw();
        }
    }
    else if ( _mode & kRectangle )
    {
        mrv::shape_type_ptr o = fg->image()->shapes().back();
        GLRectangleShape* s = dynamic_cast< GLRectangleShape* >( o.get() );
        if ( s == NULL )
        {
            LOG_ERROR( _("Not a GLRectangleShape pointer") );
        }
        else
        {
            send_network( s->send() );
            timeline()->redraw();
        }
    }
    else if ( _mode & kArrow )
    {
        mrv::shape_type_ptr o = fg->image()->shapes().back();
        GLArrowShape* s = dynamic_cast< GLArrowShape* >( o.get() );
        if ( s == NULL )
        {
            LOG_ERROR( _("Not a GLArrowShape pointer") );
        }
        else
        {
            send_network( s->send() );
            timeline()->redraw();
        }
    }
    else if ( _mode == kText )
    {
        const GLShapeList& shapes = this->shapes();
        if ( shapes.empty() ) return;
        mrv::shape_type_ptr o = shapes.back();
        GLTextShape* s = dynamic_cast< GLTextShape* >( o.get() );
        if ( s == NULL )
        {
            LOG_ERROR( _("Not a GLTextShape pointer in mouseRelease") );
        }
        else
        {
            send_network( s->send() );
            timeline()->redraw();
        }
    }
    else if ( _mode == kCircle )
    {
        const GLShapeList& shapes = this->shapes();
        if ( shapes.empty() ) return;
        mrv::shape_type_ptr o = shapes.back();
        GLCircleShape* s = dynamic_cast< GLCircleShape* >( o.get() );
        if ( s == NULL )
        {
            LOG_ERROR( _("Not a GLCircleShape pointer in mouseRelease") );
        }
        else
        {
            send_network( s->send() );
            timeline()->redraw();
        }
    }

}

bool ImageView::has_redo() const
{
    mrv::media fg = foreground();
    if (!fg) return false;

    return ( fg->image()->undo_shapes().size() > 0 );
}

bool ImageView::has_undo() const
{
    mrv::media fg = foreground();
    if (!fg) return false;

    return ( fg->image()->shapes().size() > 0 );
}

/**
 * Utility function to print a float value with 8 digits
 *
 * @param x float number
 *
 * @return a new 9 character buffer
 */
std::string float_printf( float x )
{
    if ( isnan(x) )
    {
        static std::string empty( _("   NAN  ") );
        return empty;
    }
    else if ( !isfinite(x) )
    {
        static std::string inf( _("  INF.  ") );
        return inf;
    }
    else
    {
        char buf[ 64 ];
        sprintf( buf, " %7.4f", x );
        return buf + strlen(buf) - 8;
    }
}

/**
 * Utility function to print a float value with 8 digits
 *
 * @param x float number
 *
 * @return a new 9 character buffer
 */
std::string hex_printf( float x )
{
    if ( isnan(x) )
    {
        static std::string empty( "        " );
        return empty;
    }
    else
    {
        char buf[ 64 ];
        unsigned h = 0;
        if ( x > 0.0f ) h = unsigned(x*255.0f);
        sprintf( buf, " %7x", h );
        return buf + strlen(buf) - 8;
    }
}


/**
 * Utility function to print a float value with 8 digits
 *
 * @param x float number
 *
 * @return a new 9 character buffer
 */
std::string dec_printf( float x )
{
    if ( isnan(x) )
    {
        static std::string empty( "        " );
        return empty;
    }
    else
    {
        char buf[ 64 ];
        unsigned h = 0;
        if ( x > 0.0f ) h = unsigned(x*255.0f);
        sprintf( buf, " %7d", h );
        return buf + strlen(buf) - 8;
    }
}


void ImageView::pixel_processed( const CMedia* img,
                                 CMedia::Pixel& rgba ) const
{
    PixelValue p = (PixelValue) uiMain->uiPixelValue->value();
    if ( p == kRGBA_Original ) return;

    mrv::image_type_ptr pic = img->left();
    if (!pic) return;

    bool blend = pic->format() != image_type::kLumma;

    BlendMode mode = (BlendMode)uiMain->uiPrefs->uiPrefsBlendMode->value();
    switch( mode )
    {
    case kBlendTraditional:
    case kBlendTraditionalNonGamma:
        if ( img->has_alpha() && rgba.a > 0.00001f && blend)
        {
            rgba.r /= rgba.a;
            rgba.g /= rgba.a;
            rgba.b /= rgba.a;
        }
        break;
    case kBlendPremult:
    case kBlendPremultNonGamma:
        break;
    }

    //
    // To represent pixel properly, we need to do gain
    //
    rgba.r *= _gain;
    rgba.g *= _gain;
    rgba.b *= _gain;

    ColorControlsUI* cc = uiMain->uiColorControls;
    if ( cc->uiActive->value() )
    {
        const Imath::M44f& m = colorMatrix(cc);
        Imath::V3f* iop = (Imath::V3f*)&rgba;
        *iop *= m;
    }

    //
    // To represent pixel properly, we need to do the lut
    //
    if ( use_lut() && ( p == kRGBA_Lut || p == kRGBA_Full ) )
    {
        Imath::V3f* in = (Imath::V3f*) &rgba;
        Imath::V3f out;
        _engine->evaluate( img, *in, out );
        *in = out;
    }


    //
    // And we do gamma last
    //
    if ( mode < kBlendTraditionalNonGamma && p == kRGBA_Full )
    {
        float one_gamma = 1.0f / img->gamma();
        // the code below is equivalent to rgba.g = powf(rgba.g, one_gamma);
        // but faster.
        if ( rgba.r >= 0.00001f && isfinite(rgba.r) )
            rgba.r = expf( logf(rgba.r) * one_gamma );
        if ( rgba.g >= 0.00001f && isfinite(rgba.g) )
            rgba.g = expf( logf(rgba.g) * one_gamma );
        if ( rgba.b >= 0.00001f && isfinite(rgba.b) )
            rgba.b = expf( logf(rgba.b) * one_gamma );
    }

    //
    // Finally, do a mult for those blending modes that need it
    //
    switch( mode )
    {
    case kBlendTraditional:
    case kBlendTraditionalNonGamma:
        if ( img->has_alpha() && blend )
        {
            rgba.r *= rgba.a;
            rgba.g *= rgba.a;
            rgba.b *= rgba.a;
        }
        break;
    case kBlendPremult:
    case kBlendPremultNonGamma:
        break;
    }

}

void ImageView::separate_layers( const CMedia* const img,
                                 mrv::image_type_ptr& pic, int& xp, int& yp,
                                 short& idx, bool& outside, int w, int h,
                                 const mrv::Recti& dpw ) const
{
    CMedia::StereoOutput output = img->stereo_output();
    if ( output == CMedia::kStereoRight )
    {
        idx = 1;
    }
    else if ( output & CMedia::kStereoSideBySide )
    {
        if ( xp >= dpw.x()+dpw.w())
        {
            if ( output & CMedia::kStereoRight ) {
                pic = img->left();
                idx = 0;
            }
            else {
                pic = img->right();
                idx = 1;
            }
            if (!pic) return;

            xp -= w;
        }
        else
        {
            if ( output & CMedia::kStereoRight ) {
                idx = 1;
            }
            else
            {
                idx = 0;
            }
        }
    }
    else if ( output & CMedia::kStereoTopBottom )
    {
        if ( yp >= dpw.y()+dpw.h() )
        {
            if ( output & CMedia::kStereoRight ) {
                pic = img->left();
                idx = 0;
            }
            else {
                pic = img->right();
                idx = 1;
            }
            if (!pic) return;

            yp -= h;
        }
        else
        {
            if ( output & CMedia::kStereoRight ) {
                idx = 1;
            }
            else
            {
                idx = 0;
            }
        }
    }
    else if ( output & CMedia::kStereoAnaglyph )
    {
        if ( output & CMedia::kStereoRight )
        {
            idx = 1;
            pic = img->right();
        }
        else
        {
            pic = img->left();
        }
        if (!pic) return;
    }

    if ( xp < dpw.x() || yp < dpw.y() ||
            xp >= dpw.w()*img->scale_x() || yp >= dpw.h()*img->scale_y() ) {
        outside = true;
    }
}

void ImageView::top_bottom( const CMedia* const img,
                            mrv::image_type_ptr& pic, int& xp, int& yp,
                            short& idx, bool& outside, int w, int h ) const
{
    CMedia::StereoOutput output = img->stereo_output();
    idx = 0;
    if ( output == CMedia::kStereoLeft )
    {
        idx = 1;
        if ( yp >= h ) outside = true;
    }
    else if ( output == CMedia::kNoStereo ||
              output == CMedia::kStereoRight )
    {
        idx = 0;
        yp += h;
        if ( yp < h ) outside = true;
    }
    else if ( output & CMedia::kStereoSideBySide )
    {
        if ( xp >= w )
        {
            if ( output & CMedia::kStereoRight ) {
                if ( yp >= h ) outside = true;
                idx = 0;
            }
            else {
                idx = 1;
                yp += h;
                if ( yp <= h ) outside = true;
            }
            xp -= w;
        }
        else
        {
            if ( output & CMedia::kStereoRight )
            {
                idx = 0;
                yp += h;
                if ( yp <= h ) outside = true;
            }
            else
            {
                idx = 1;
                if ( yp >= h ) outside = true;
            }
        }
    }
    else if ( output & CMedia::kStereoTopBottom )
    {
        if ( yp >= h )
        {
            if ( output & CMedia::kStereoRight ) {
                yp -= h;
                idx = 0;
            }
            else
            {
                idx = 1;
            }
        }
        else
        {
            if ( output & CMedia::kStereoRight )
            {
                idx = 0;
                yp += h;
                if ( yp < h ) outside = true;
            }
            else
            {
                idx = 1;
            }
        }
    }
}


void ImageView::left_right( const CMedia* const img,
                            mrv::image_type_ptr& pic, int& xp, int& yp,
                            short& idx, bool& outside, int w, int h ) const
{
    CMedia::StereoOutput output = img->stereo_output();
    if ( output == CMedia::kStereoLeft )
    {
        idx = 1;
        if ( xp >= w ) outside = true;
    }
    else if ( output == CMedia::kNoStereo ||
              output == CMedia::kStereoRight )
    {
        idx = 0;
        xp += w;
        if ( xp < w ) outside = true;
    }
    else if ( output & CMedia::kStereoSideBySide )
    {
        if ( xp >= w )
        {
            if ( output & CMedia::kStereoRight ) {
                xp -= w;
                idx = 0;
            }
            else
            {
                idx = 1;
            }
        }
        else
        {
            if ( output & CMedia::kStereoRight )
            {
                xp += w;
                idx = 0;
            }
            else
            {
                idx = 1;
            }
        }
    }
    else if ( output & CMedia::kStereoTopBottom )
    {
        if ( yp >= h )
        {
            if ( output & CMedia::kStereoRight ) {
                idx = 0;
                if ( xp >= w ) outside = true;
            }
            else {
                idx = 1;
                xp += w;
                if ( xp <= w ) outside = true;
            }
            yp -= h;
        }
        else
        {
            if ( output & CMedia::kStereoRight )
            {
                xp += w;
                idx = 0;
                if ( xp <= w ) outside = true;
            }
            else
            {
                idx = 1;
                if ( xp >= w ) outside = true;
            }
        }
    }
}

void ImageView::picture_coordinates( const CMedia* const img, const int x,
                                     const int y, bool& outside,
                                     mrv::image_type_ptr& pic,
                                     int& xp, int& yp,
                                     int& w, int& h, int off[2] ) const
{
    assert( img != NULL );
    outside = false;
    double xf = (double) x;
    double yf = (double) y;

    data_window_coordinates( img, xf, yf );


    CMedia::StereoOutput output = img->stereo_output();
    CMedia::StereoInput  input  = img->stereo_input();

    short idx = 0;
    pic = img->left();
    if ( output & CMedia::kStereoRight )
    {
        pic = img->right();
        idx = 0;
    }

    if ( !pic ) return;

    mrv::Recti daw[2], dpw[2];

    daw[0] = img->data_window();
    daw[1] = img->data_window2();
    dpw[0] = img->display_window();
    dpw[1] = img->display_window2();

    xp = (int)floor(xf);
    yp = (int)floor(yf);

    xp += daw[idx].x();
    yp += daw[idx].y();

    xp -= (int)img->x();
    yp += (int)img->y();


    mrv::Recti dpm = dpw[idx];
    w = dpm.w();
    h = dpm.h();

    if ( ! display_window() )
        dpm.merge( daw[idx] );

    if ( w == 0 ) {
        w = pic->width();
        dpm.x( 0 );
        dpm.w( w );
    }
    if ( h == 0 ) {
        h = pic->height();
        dpm.y( 0 );
        dpm.h( h );
    }


    if ( input == CMedia::kNoStereoInput ||
            input == CMedia::kSeparateLayersInput )
    {
        separate_layers( img, pic, xp, yp, idx, outside, w, h, dpm );
    }
    else if ( input == CMedia::kTopBottomStereoInput )
    {
        top_bottom( img, pic, xp, yp, idx, outside, w, h );
    }
    else if ( input == CMedia::kLeftRightStereoInput )
    {
        left_right( img, pic, xp, yp, idx, outside, w, h );
    }

    xp -= off[0] = daw[idx].x();
    yp -= off[1] = daw[idx].y();

    if ( output == CMedia::kStereoInterlaced )
    {
        if ( yp % 2 == 1 )
        {
            if ( input == CMedia::kTopBottomStereoInput )
            {
                yp += h;
            }
            else if ( input == CMedia::kLeftRightStereoInput )
            {
                xp += w;
            }
            else
            {
                pic = img->right();
                xp += daw[0].x();
                yp += daw[0].y();
                xp -= daw[1].x();
                yp -= daw[1].y();
            }
        }
        if ( !pic ) return;
    }
    else if ( output == CMedia::kStereoInterlacedColumns )
    {
        if ( xp % 2 == 1 )
        {
            if ( input == CMedia::kTopBottomStereoInput )
            {
                yp += h;
            }
            else if ( input == CMedia::kLeftRightStereoInput )
            {
                xp += w;
            }
            else
            {
                pic = img->right();
                xp += daw[0].x();
                yp += daw[0].y();
                xp -= daw[1].x();
                yp -= daw[1].y();
            }
        }
        if ( !pic ) return;
    }
    else if ( output == CMedia::kStereoCheckerboard )
    {
        if ( (xp + yp) % 2 == 0 )
        {
            if ( input == CMedia::kTopBottomStereoInput )
            {
                yp += h;
            }
            else if ( input == CMedia::kLeftRightStereoInput )
            {
                xp += w;
            }
            else
            {
                pic = img->right();
                xp += daw[0].x();
                yp += daw[0].y();
                xp -= daw[1].x();
                yp -= daw[1].y();
            }
        }
        if ( !pic ) return;
    }

    dpm = dpw[idx];
    if ( ! display_window() )
        dpm.merge( daw[idx] );
    w = dpm.w();
    h = dpm.h();

    if (!pic) return;


    if ( xp < 0 || xp >= (int)((double)pic->width()*img->scale_x()) ||
         yp < 0 || yp >= (int)((double)pic->height()*img->scale_y()) )
    {
        outside = true;
    }
    else if ( input == CMedia::kSeparateLayersInput &&
              output != CMedia::kNoStereo &&
              ( xp > w-daw[idx].x() || yp > h-daw[idx].y() ) ) {
        outside = true;
    }
    else if ( vr() )
    {
        xp = yp = 0;
        outside = true;
    }

}

void ImageView::normalize( CMedia::Pixel& rgba, unsigned short idx ) const
{
    if ( !_engine ) return;

    float pMin = _engine->norm_min();
    float pMax = _engine->norm_max();
    if ( pMin != 0.0 || pMax != 1.0 )
    {
        float span = pMax - pMin;
        rgba.r = (rgba.r - pMin) / span;
        rgba.g = (rgba.g - pMin) / span;
        rgba.b = (rgba.b - pMin) / span;
    }
}

/**
 * Handle a mouse release
 *
 * @param x Fl::event_x() coordinate
 * @param y Fl::event_y() coordinate
 */
void ImageView::mouseMove(int x, int y)
{
    if ( !uiMain || !uiMain->uiPixelBar->visible() || !_engine ) return;

    mrv::media fg = foreground();
    if ( !fg ) return;

    CMedia* img = fg->image();

    mrv::image_type_ptr pic;
    bool outside = false;
    int off[2];
    int xp = x, yp = y, w, h;
    picture_coordinates( img, x, y, outside, pic, xp, yp, w, h, off );
    if ( !pic ) return;

    double xpct = 1.0 / img->scale_x();
    double ypct = 1.0 / img->scale_y();

    int ypr = pic->height() - yp - 1;


    off[0] += xp;
    off[1] += ypr;

    char buf[40];
    sprintf( buf, "%5d, %5d", off[0], off[1] );
    uiMain->uiCoord->value(buf);



    CMedia::Pixel rgba;
    if ( outside || ypr < 0 || vr() )
    {
        rgba.r = rgba.g = rgba.b = rgba.a = std::numeric_limits< float >::quiet_NaN();
    }
    else
    {

        mrv::Recti daw[2];
        daw[0] = img->data_window();
        daw[1] = img->data_window2();


        xp = int( (double)xp * xpct );
        yp = int( (double)yp * ypct );
        rgba = pic->pixel( xp, yp );

        pixel_processed( img, rgba );

        if ( normalize() )
        {
            normalize( rgba );
        }



        CMedia::StereoOutput stereo_out = img->stereo_output();
        CMedia::StereoInput  stereo_in  = img->stereo_input();


        if ( stereo_out & CMedia::kStereoAnaglyph )
        {

            if ( stereo_in == CMedia::kTopBottomStereoInput )
            {
                if ( stereo_out & CMedia::kStereoRight )
                    yp += h;
                else
                    yp -= h;
            }
            else if ( stereo_in == CMedia::kLeftRightStereoInput )
            {
                if ( stereo_out & CMedia::kStereoRight )
                    xp -= w;
                else
                    xp += w;
            }
            else
            {
                if ( stereo_out & CMedia::kStereoRight )
                {

                    pic = img->left();

                    xp += daw[1].x();
                    yp += daw[1].y();
                    xp -= daw[0].x();
                    yp -= daw[0].y();
                }
                else
                {

                    pic = img->right();

                    xp += daw[0].x();
                    yp += daw[0].y();
                    xp -= daw[1].x();
                    yp -= daw[1].y();
                }
            }
            if ( pic )
            {
                outside = false;
                if ( xp < 0 || xp >= (int)pic->width() ||
                     yp < 0 || yp >= (int)pic->height() )
                    outside = true;

                if (!outside)
                {

                    float r = rgba.r;
                    rgba = pic->pixel( xp, yp );

                    pixel_processed( img, rgba );

                    if ( normalize() )
                    {
                        normalize( rgba );
                    }

                    rgba.r = r;
                }
                else
                {
                    rgba.g = rgba.b = 0.0f;
                }
            }

        }
        // double yp = yf;
        // if ( _showPixelRatio ) yp /= img->pixel_ratio();
    }


    CMedia* bgr = _engine->background();


    if ( _showBG && bgr && ( outside || rgba.a < 1.0f ) )
    {
        double xf, yf;
        const mrv::image_type_ptr picb = bgr->left();

        const mrv::Recti& dpw = img->display_window();

        const mrv::Recti& daw = img->data_window();

        const mrv::Recti& dpwb = bgr->display_window();

        const mrv::Recti& dawb = bgr->data_window();

        if ( picb )
        {
            int w = dawb.w();
            int h = dawb.h();
            if ( w == 0 ) w = picb->width()-1;
            if ( h == 0 ) h = picb->height()-1;


            if ( dpw == dpwb && dpwb.h() != 0 )
            {
                xf = double(x);
                yf = double(y);
                data_window_coordinates( bgr, xf, yf );
                xp = (int)floor( xf );
                yp = (int)floor( yf );
            }
            else
            {
                double px = 1.0, py = 1.0;

                if ( uiMain->uiPrefs->uiPrefsResizeBackground->value() )
                {
                    if ( dpw.w() > 0 )
                    {
                        px = (double) picb->width() / (double) dpw.w();
                        py = (double) picb->height() / (double) dpw.h();
                    }
                    else
                    {
                        px = (double) picb->width() / (double) pic->width();
                        py = (double) picb->height() / (double) pic->height();
                    }
                }

                xp += daw.x();
                yp += daw.y();
                xp = (int)floor( xp * px );
                yp = (int)floor( yp * py );
            }



            CMedia::Pixel bg;
            bool outside = false;
            if ( xp < 0 || yp < 0 || xp >= (int)w || yp >= (int)h )
            {
                outside = true;
            }
            else
            {

                bg = picb->pixel( xp, yp );
                pixel_processed( bgr, bg );
            }

            if ( outside )
            {
                rgba = bg;
            }
            else
            {

                float t = 1.0f - rgba.a;
                rgba.r += bg.r * t;
                rgba.g += bg.g * t;
                rgba.b += bg.b * t;

            }
        }
    }

    switch( uiMain->uiAColorType->value() )
    {
    case kRGBA_Float:

        uiMain->uiPixelR->value( float_printf( rgba.r ).c_str() );
        uiMain->uiPixelG->value( float_printf( rgba.g ).c_str() );
        uiMain->uiPixelB->value( float_printf( rgba.b ).c_str() );
        uiMain->uiPixelA->value( float_printf( rgba.a ).c_str() );
        break;
    case kRGBA_Hex:

        uiMain->uiPixelR->value( hex_printf( rgba.r ).c_str() );
        uiMain->uiPixelG->value( hex_printf( rgba.g ).c_str() );
        uiMain->uiPixelB->value( hex_printf( rgba.b ).c_str() );
        uiMain->uiPixelA->value( hex_printf( rgba.a ).c_str() );
        break;
    case kRGBA_Decimal:

        uiMain->uiPixelR->value( dec_printf( rgba.r ).c_str() );
        uiMain->uiPixelG->value( dec_printf( rgba.g ).c_str() );
        uiMain->uiPixelB->value( dec_printf( rgba.b ).c_str() );
        uiMain->uiPixelA->value( dec_printf( rgba.a ).c_str() );
        break;
    }

    //
    // Show this pixel as 8-bit fltk color box
    //

    if ( rgba.r > 1.0f ) rgba.r = 1.0f;
    else if ( rgba.r < 0.0f ) rgba.r = 0.0f;
    if ( rgba.g > 1.0f ) rgba.g = 1.0f;
    else if ( rgba.g < 0.0f ) rgba.g = 0.0f;
    if ( rgba.b > 1.0f ) rgba.b = 1.0f;
    else if ( rgba.b < 0.0f ) rgba.b = 0.0f;

    uchar col[3];

    col[0] = uchar(rgba.r * 255.f);
    col[1] = uchar(rgba.g * 255.f);
    col[2] = uchar(rgba.b * 255.f);

    Fl_Color c( fl_rgb_color( col[0], col[1], col[2] ) );

    // bug in fltk color lookup? (0 != Fl_BLACK)
    if ( c == 0 )
    {

        uiMain->uiPixelView->color( FL_BLACK );
    }
    else
    {
        uiMain->uiPixelView->color( c );
    }

    uiMain->uiPixelView->redraw();



    CMedia::Pixel hsv;

    int cspace = uiMain->uiBColorType->value() + 1;

    switch( cspace )
    {
    case color::kRGB:
        hsv = rgba;
        break;
    case color::kHSV:
        hsv = color::rgb::to_hsv( rgba );
        break;
    case color::kHSL:
        hsv = color::rgb::to_hsl( rgba );
        break;
    case color::kCIE_XYZ:
        hsv = color::rgb::to_xyz( rgba );
        break;
    case color::kCIE_xyY:
        hsv = color::rgb::to_xyY( rgba );
        break;
    case color::kCIE_Lab:
        hsv = color::rgb::to_lab( rgba );
        break;
    case color::kCIE_Luv:
        hsv = color::rgb::to_luv( rgba );
        break;
    case color::kYUV:
        hsv = color::rgb::to_yuv( rgba );
        break;
    case color::kYDbDr:
        hsv = color::rgb::to_YDbDr( rgba );
        break;
    case color::kYIQ:
        hsv = color::rgb::to_yiq( rgba );
        break;
    case color::kITU_601:
        hsv = color::rgb::to_ITU601( rgba );
        break;
    case color::kITU_709:
        hsv = color::rgb::to_ITU709( rgba );
        break;
    default:
        LOG_ERROR("Unknown color type");
    }


    uiMain->uiPixelH->value( float_printf( hsv.r ).c_str() );
    uiMain->uiPixelS->value( float_printf( hsv.g ).c_str() );
    uiMain->uiPixelV->value( float_printf( hsv.b ).c_str() );
    mrv::BrightnessType brightness_type = (mrv::BrightnessType)
                                          uiMain->uiLType->value();
    hsv.a = calculate_brightness( rgba, brightness_type );
    uiMain->uiPixelL->value( float_printf( hsv.a ).c_str() );

}

float ImageView::vr_angle() const
{
    if (!_engine) return 45.0;
    return _engine->angle();
}

void ImageView::vr_angle( const float t )
{
    if ( t >= 90.0f || t <= 5.0f || !_engine ) return;

    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    char buf[32];
    sprintf( buf, "VRangle %g", t );
    send_network( buf );

    restore_locale( oldloc );

    _engine->angle( t );
    valid(0);
    redraw();
}

/**
 * Handle a mouse drag
 *
 * @param x Fl::event_x() coordinate
 * @param y Fl::event_y() coordinate
 */
void ImageView::mouseDrag(int x,int y)
{
    if (flags & kMouseDown)
    {
        int dx = x - lastX;
        int dy = y - lastY;

        if ( flags & kZoom )
        {
            if ( vr() )
            {
                float angle = vr_angle();
                angle -= float(dx) / 10.0f;
                vr_angle( angle );
            }
            else
            {
                zoom( _zoom + float(dx)*_zoom / 500.0f );
            }

            lastX = x;
            lastY = y;
        }
        else if ( flags & kGain )
        {
            exposure_change( float(dx) / 150.0f );
            lastX = x;
            lastY = y;
        }
        else if ( flags & kGamma )
        {
            gamma( _gamma + float(dx) / 250.0f );
            lastX = x;
            lastY = y;
        }
        else if ( flags & kMouseMove )
        {
            window()->cursor( FL_CURSOR_MOVE );
            if ( vr() )
            {
#define SPINY_MIN 0.005
#define SPINX_MIN 0.005
#define SPINY_MAX 1.0
#define SPINX_MAX 0.5
                spiny += double(dx) / 360.0;
                if ( spiny > SPINY_MAX ) spiny = SPINY_MAX;
                else if ( spiny < -SPINY_MAX ) spiny = -SPINY_MAX;
                else if ( std::abs(spiny) <= SPINY_MIN ) spiny = 0.0;
                spinx += double(dy) / 90.0;
                if ( spinx > SPINX_MAX ) spinx = SPINX_MAX;
                else if ( spinx < -SPINX_MAX ) spinx = -SPINX_MAX;
                else if ( std::abs(spinx) <= SPINX_MIN ) spinx = 0.0;

                char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
                setlocale( LC_NUMERIC, "C" );

                char buf[128];
                sprintf( buf, "Spin %g %g", spinx, spiny );
                send_network( buf );

                restore_locale( oldloc );
            }
            else
            {
                double ox = xoffset;
                double oy = yoffset;
                xoffset += double(dx) / _zoom;
                yoffset -= double(dy) / _zoom;

                if ( !mrv::is_equal( ox, xoffset ) ||
                     !mrv::is_equal( oy, yoffset ) )
                {
                    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
                    setlocale( LC_NUMERIC, "C" );

                    char buf[128];
                    sprintf( buf, "Offset %g %g", xoffset, yoffset );
                    send_network( buf );

                    restore_locale( oldloc );
                }
            }

            lastX = x;
            lastY = y;


        }
        else if ( _mode == kScrub )
        {
            double s;
            s = uiMain->uiPrefs->uiPrefsScrubbingSensitivity->value();
            double dx = (Fl::event_x() - lastX) / s;
            if ( std::abs(dx) >= 1.0f )
            {
                scrub( dx );
                lastX = Fl::event_x();
            }
        }
        else if ( _mode == kMovePicture || _mode == kScalePicture )
        {
            CMedia* img = selected_image();
            if ( ! img ) return;

            if ( _mode == kScalePicture )
            {
                double px = img->scale_x();
                double py = img->scale_y();

                px += double(dx) / w() / _zoom;
                py += double(dy) / h() / _zoom;

                img->scale_x( px );
                img->scale_y( py );

                update_image_info();

                char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
                setlocale( LC_NUMERIC, "C" );

                char buf[128];
                sprintf( buf, "ScalePicture %g %g", px, py );
                send_network( buf );

                restore_locale( oldloc );
            }
            else
            {
                double px = img->x();
                double py = img->y();

                px += double(dx) / _zoom;
                py -= double(dy) / _zoom;

                img->x( px );
                img->y( py );

                update_image_info();

                char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
                setlocale( LC_NUMERIC, "C" );

                char buf[128];
                sprintf( buf, "MovePicture %g %g", px, py );
                send_network( buf );

                restore_locale( oldloc );
            }
            lastX = x;
            lastY = y;

        }
        else
        {


            mrv::media fg = foreground();
            if ( ! fg ) return;

            CMedia* img = fg->image();

            double xf = double(lastX);
            double yf = double(lastY);
            data_window_coordinates( img, xf, yf );

            mrv::Recti daw[2], dpw[2];

            daw[0] = img->data_window();
            daw[1] = img->data_window2();
            dpw[0] = img->display_window();
            dpw[1] = img->display_window2();

            double xn = double(x);
            double yn = double(y);
            data_window_coordinates( img, xn, yn );

            short idx = 0;

            unsigned W = dpw[0].w();
            unsigned H = dpw[0].h();

            bool right = false;
            bool bottom = false;
            int diffX = 0;
            int diffY = 0;
            CMedia::StereoOutput stereo_out = img->stereo_output();
            if ( stereo_out == CMedia::kStereoRight )
            {
                idx = 1;
                diffX = daw[1].x() - daw[0].x();
                diffY = daw[1].y() - daw[0].y();
            }
            else if ( stereo_out & CMedia::kStereoSideBySide )
            {
                // if ( xf >= W - daw[0].x() )
                if ( xf >= W )
                {
                    right = true;
                    xf -= W;
                    xn -= W;

                    if ( stereo_out & CMedia::kStereoRight )
                    {
                        idx = 0;
                    }
                    else
                    {
                        diffX = daw[1].x() - daw[0].x();
                        diffY = daw[1].y() - daw[0].y();
                        idx = 1;
                    }
                }
                else if ( stereo_out & CMedia::kStereoRight )
                {
                    idx = 1;
                    diffX = daw[1].x() - daw[0].x();
                    diffY = daw[1].y() - daw[0].y();
                }
            }
            else if ( stereo_out & CMedia::kStereoTopBottom )
            {
                // if ( yf >= H - daw[0].y() )
                if ( yf >= H )
                {
                    bottom = true;
                    yf -= H;
                    yn -= H;

                    if ( stereo_out & CMedia::kStereoRight )
                    {
                        idx = 0;
                    }
                    else
                    {
                        diffX = daw[1].x() - daw[0].x();
                        diffY = daw[1].y() - daw[0].y();
                        idx = 1;
                    }
                }
                else if ( stereo_out & CMedia::kStereoRight )
                {
                    idx = 1;
                    diffX = daw[1].x() - daw[0].x();
                    diffY = daw[1].y() - daw[0].y();
                }
            }

            W = dpw[idx].w();
            H = dpw[idx].h();

            xf = floor(xf);
            yf = floor(yf);

            xn = floor(xn+0.5f);
            yn = floor(yn+0.5f);


            if ( _mode & kSelection )
            {

                if ( xn < xf )
                {
                    double tmp = xf;
                    xf = xn;
                    xn = tmp;
                }
                if ( yn < yf )
                {
                    double tmp = yf;
                    yf = yn;
                    yn = tmp;
                }


                double X, XM, Y, YM;
                if ( display_window() )
                {
                    // X = dpw[idx].l()-daw[idx].x();
                    // XM = dpw[idx].r()-daw[idx].x();
                    // Y = dpw[idx].t()-daw[idx].y();
                    // YM = dpw[idx].b()-daw[idx].y();
                    X = dpw[idx].l()-daw[0].x();
                    XM = dpw[idx].r()-daw[0].x();
                    Y = dpw[idx].t()-daw[0].y();
                    YM = dpw[idx].b()-daw[0].y();
                }
                else
                {
                    X = diffX;
                    XM = daw[idx].w() + diffX;

                    Y = diffY;
                    YM = daw[idx].h() + diffY;
                }


                if ( xf < X ) xf = X;
                else if ( xf > XM )  xf = XM;
                if ( yf < Y ) yf = Y;
                else if ( yf > YM ) yf = YM;

                if ( xn < X ) xn = X;
                else if ( xn > XM ) xn = XM;
                if ( yn < Y ) yn = Y;
                else if ( yn > YM ) yn = YM;


                double dx = (double) std::abs( xn - xf );
                double dy = (double) std::abs( yn - yf );
                if ( dx == 0.0 || dy == 0.0 )
                {
                    dx = 0.0;
                    dy = 0.0;
                }

                double xt = xf + daw[0].x() + dpw[0].w() * right;
                double yt = yf + daw[0].y() + dpw[0].h() * bottom;

                _selection = mrv::Rectd( xt, yt, dx, dy );


            }


            float scale = Fl::screen_scale( window()->screen_num() );

            if ( (_mode & kDraw) || (_mode & kErase) || (_mode & kArrow) ||
                 (_mode & kRectangle) )
            {
                GLShapeList& shapes = fg->image()->shapes();
                if ( shapes.empty() ) return;

                mrv::shape_type_ptr o = shapes.back();
                GLPathShape* s = dynamic_cast< GLPathShape* >( o.get() );
                if ( s == NULL )
                {
                    LOG_ERROR( _("Not a GLPathShape pointer") );
                }
                else
                {

                    yn = -yn;

                    xn += daw[idx].x();
                    yn -= daw[idx].y();



                    mrv::Point p2( xn, yn );
                    if ( _mode & kArrow )
                    {
                        Imath::V2d p1 = s->pts[0];
                        Imath::V2d lineVector = p2 - p1;
                        double lineLength = lineVector.length();


                        const float theta = 45 * M_PI / 180;
                        const int nWidth = 35;

                        double tPointOnLine = nWidth /
                                              (2 * (tanf(theta) / 2) *
                                               lineLength);
                        Imath::V2d pointOnLine = p2 +
                                                 -tPointOnLine * lineVector;

                        Imath::V2d normalVector( -lineVector.y,
                                                 lineVector.x );

                        double tNormal = nWidth / (2 * lineLength );
                        Imath::V2d tmp = pointOnLine +
                                         tNormal * normalVector;
                        s->pts[1] = p2;
                        s->pts[2] = tmp;
                        s->pts[3] = p2;
                        tmp = pointOnLine + -tNormal * normalVector;
                        s->pts[4] = tmp;
                    }
                    else if ( _mode & kRectangle )
                    {
                        s->pts[1] = p2;
                    }
                    else
                    {
                        s->pts.push_back( p2 );
                    }
                }
            }
            else if ( _mode == kCircle )
            {
                GLShapeList& shapes = fg->image()->shapes();
                if ( shapes.empty() ) return;

                mrv::shape_type_ptr o = shapes.back();
                GLCircleShape* s = dynamic_cast< GLCircleShape* >( o.get() );
                if ( s == NULL )
                {
                    LOG_ERROR( _("Not a GLCircleShape pointer") );
                }

                yn = -yn;

                xn += daw[idx].x();
                yn -= daw[idx].y();


                mrv::Point p( xn, yn );
                double A = p.x - s->center.x;
                double B = p.y - s->center.y;
                s->radius = sqrt( A*A+B*B ) / scale;
            }
            else if ( _mode & kText )
            {
                bool found = false;
                MultilineInput* w;
                GLTextShape* t;
                for ( int i = 0; i < children(); ++i )
                {
                    w = dynamic_cast< MultilineInput* >( child(i) );
                    if ( !w ) continue;
                    found = true; break;
                }
                if ( found )
                {
                    const GLShapeList& shapes = this->shapes();
                    if ( shapes.empty() ) return;
                    t = dynamic_cast< GLTextShape* >( shapes.back().get() );
                    if ( !t ) return;
                    t->pts[0] = mrv::Point( xf, yf );
                    w->Fl_Widget::position( x, y );
                    redraw();
                }
            }

            assert( _selection.w() >= 0.0 );
            assert( _selection.h() >= 0.0 );

            update_color_info();

            mouseMove( x, y );
        }

        redraw();
    }

}

void ImageView::texture_filtering( const TextureFiltering p )
{
    _texture_filtering = p;

    char buf[64];
    sprintf( buf, N_("TextureFiltering %d"), (int)p );
    send_network( buf );

    redraw();
}


/**
 * Handle a keypress
 *
 * @param key fltk code for key pressed
 *
 * @return 1 if handled, 0 if not
 */
int ImageView::keyDown(unsigned int rawkey)
{
    if ( _mode & kDraw || _mode & kErase || _mode & kArrow ||
         _mode & kRectangle )
    {
        double pen = uiMain->uiPaint->uiPenSize->value();
        // Use exposure hotkey ( default [ and ] )
        if ( kPenSizeMore.match( rawkey ) )
        {
            uiMain->uiPaint->uiPenSize->value( pen + 1.0 );
            redraw();
            return 1;
        }
        else if ( kPenSizeLess.match( rawkey ) )
        {
            uiMain->uiPaint->uiPenSize->value( pen - 1.0 );
            redraw();
            return 1;
        }
    }

    if ( kResetChanges.match( rawkey ) )
    {
        gamma( 1.0 );
        gain( 1.0 );
        update_image_info();
        redraw();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kOpenSingleImage.match( rawkey ) )
    {
        open_single_cb( this, browser() );
        return 1;
    }
    else if ( kShapeFrameStepFwd.match(rawkey) )
    {
        next_shape_frame( this );
        return 1;
    }
    else if ( kShapeFrameStepBack.match(rawkey) )
    {
        previous_shape_frame( this );
        return 1;
    }
    else if ( kDrawTemporaryMode.match( rawkey ) )
    {
        draw_mode( true );
        return 1;
    }
    else if ( kDrawMode.match( rawkey ) )
    {
        draw_mode();
        return 1;
    }
    else if ( kEraseTemporaryMode.match( rawkey ) )
    {
        erase_mode( true );
        return 1;
    }
    else if ( kEraseMode.match( rawkey ) )
    {
        erase_mode();
        return 1;
    }
    else if ( kArrowMode.match( rawkey ) )
    {
        arrow_mode();
        return 1;
    }
    else if ( kRectangleMode.match( rawkey ) )
    {
        rectangle_mode();
        return 1;
    }
    else if ( kCircleMode.match( rawkey ) )
    {
        circle_mode();
        return 1;
    }
    else if ( kTextMode.match( rawkey ) )
    {
        text_mode();
        return 1;
    }
    else if ( kScrubMode.match( rawkey ) )
    {
        scrub_mode();
        return 1;
    }
    else if ( kMoveSizeMode.match( rawkey ) )
    {
        move_pic_mode();
        return 1;
    }
    else if ( kOpenImage.match( rawkey ) )
    {
        open_cb( this, browser() );
        return 1;
    }
    else if ( kOpenDirectory.match( rawkey ) )
    {
        open_dir_cb( this, browser() );
        return 1;
    }
    else if ( kOpenStereoImage.match( rawkey ) )
    {
        open_stereo_cb( this, browser() );
        return 1;
    }
    else if ( kCloneImage.match( rawkey ) )
    {
        clone_image_cb( NULL, browser() );
        return 1;
    }
    else if ( kOCIOInputColorSpace.match( rawkey ) )
    {
        attach_ocio_ics_cb( NULL, browser() );
        return 1;
    }
    else if ( kOCIODisplay.match( rawkey ) )
    {
        attach_ocio_display_cb( NULL, this );
        return 1;
    }
    else if ( kOCIOView.match( rawkey ) )
    {
        attach_ocio_view_cb( NULL, this );
        return 1;
    }
    else if ( kSaveSequence.match( rawkey ) )
    {
        save_sequence_cb( this, this );
        return 1;
    }
    else if ( kSaveImage.match( rawkey ) )
    {
        browser()->save();
        return 1;
    }
    else if ( kSaveReel.match( rawkey ) )
    {
        save_reel_cb( this, this );
        return 1;
    }
    else if ( kSaveSnapshot.match( rawkey ) )
    {
        save_snap_cb( this, this );
        return 1;
    }
    else if ( kSaveSession.match( rawkey ) )
    {
        save_session_as_cb( this, this );
        return 1;
    }
    else if ( kSetAsBG.match( rawkey ) )
    {
        set_as_background_cb( NULL, this );
        return 1;
    }
    else if ( kSwitchChannels.match( rawkey ) )
    {
        switch_channels_cb(this, this);
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kPreviousChannel.match( rawkey ) )
    {
        previous_channel_cb(this, this);
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kNextChannel.match( rawkey ) )
    {
        next_channel_cb(this, this);
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kExposureMore.match( rawkey ) )
    {
        exposure_change( 0.5f );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kExposureLess.match( rawkey ) )
    {
        exposure_change( -0.5f );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kGammaMore.match( rawkey ) )
    {
        gamma( gamma() + 0.1f );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kGammaLess.match( rawkey ) )
    {
        gamma( gamma() - 0.1f );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( rawkey >= kZoomMin.key && rawkey <= kZoomMax.key )
    {
        if ( rawkey == kZoomMin.key )
        {
            zoom( 1.0f );
        }
        else
        {
            float z = (float) (rawkey - kZoomMin.key);
            if ( Fl::event_state( FL_CTRL ) )
                z = 1.0f / z;
            zoom_under_mouse( z, Fl::event_x(), Fl::event_y() );
        }
        return 1;
    }
    else if ( kZoomIn.match( rawkey ) )
    {
        zoom_under_mouse( _zoom * 2.0f,
                          Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kZoomOut.match( rawkey ) )
    {
        zoom_under_mouse( _zoom * 0.5f,
                          Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kFullScreen.match( rawkey ) )
    {
        toggle_fullscreen();
        return 1;
    }
    else if ( kCenterImage.match(rawkey) )
    {
        if ( vr() )
        {
            spinx = 2000.0; // >= 1000.0 resets rotation in mrvGLSphere/Cube.cpp
            redraw();
            return 1;
        }
        center_image();
        return 1;
    }
    else if ( kTextureFiltering.match( rawkey ) )
    {
        texture_filtering_cb( NULL, this );
        return 1;
    }
    else if ( kUndoDraw.match( rawkey ) )
    {
        undo_draw();
        return 1;
    }
    else if ( kRedoDraw.match( rawkey ) )
    {
        redo_draw();
        return 1;
    }
    else if ( kFitScreen.match( rawkey ) )
    {
        if ( vr() )
        {
            vr_angle( 45.0f );
            return 1;
        }
        fit_image();
        return 1;
    }
    else if ( kResizeMainWindow.match( rawkey ) )
    {
         resize_main_window();
         return 1;
    }
    else if ( kSafeAreas.match( rawkey ) )
    {
        safe_areas( safe_areas() ^ true );
        mouseMove( Fl::event_x(), Fl::event_y() );
        redraw();
        return 1;
    }
    else if ( kDataWindow.match( rawkey ) )
    {
        data_window( data_window() ^ true );
        mouseMove( Fl::event_x(), Fl::event_y() );
        redraw();
        return 1;
    }
    else if ( kDisplayWindow.match( rawkey ) )
    {
        display_window( display_window() ^ true );
        mouseMove( Fl::event_x(), Fl::event_y() );
        redraw();
        return 1;
    }
    else if ( kHudToggle.match( rawkey ) )
    {
        hud_toggle_cb( NULL, uiMain );
        mouseMove( Fl::event_x(), Fl::event_y() );
        redraw();
        return 1;
    }
    else if ( kGridToggle.match( rawkey ) )
    {
        grid_toggle_cb( NULL, this );
        redraw();
        return 1;
    }
    else if ( kGridSize.match( rawkey ) )
    {
        grid_size_cb( NULL, this );
        redraw();
        return 1;
    }
    else if ( kWipe.match( rawkey ) )
    {
        if ( _wipe_dir == kNoWipe )  {
            _wipe_dir = kWipeVertical;
            _wipe = (float) Fl::event_x() / float( w() );

            char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
            setlocale( LC_NUMERIC, "C" );

            char buf[128];
            sprintf( buf, "WipeVertical %g", _wipe );
            send_network( buf );

            restore_locale( oldloc );

            window()->cursor(FL_CURSOR_WE);
        }
        else if ( _wipe_dir & kWipeVertical )
        {
            _wipe_dir = kWipeHorizontal;
            _wipe = (float) (h() - Fl::event_y()) / float( h() );

            char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
            setlocale( LC_NUMERIC, "C" );

            char buf[128];
            sprintf( buf, "WipeHorizontal %g", _wipe );
            send_network( buf );
            window()->cursor(FL_CURSOR_NS);

            restore_locale( oldloc );
        }
        else if ( _wipe_dir & kWipeHorizontal ) {
            _wipe_dir = kNoWipe;
            _wipe = 0.0f;
            char buf[128];
            sprintf( buf, "NoWipe" );
            send_network( buf );
            window()->cursor(FL_CURSOR_CROSS);
        }

        _wipe = std::abs( _wipe );

        mouseMove( Fl::event_x(), Fl::event_y() );
        redraw();
        return 1;
    }
    else if ( kFlipX.match( rawkey ) )
    {
        flip_x_cb( NULL, this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        redraw();
        return 1;
    }
    else if ( kFlipY.match( rawkey ) )
    {
        flip_y_cb( NULL, this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        redraw();
        return 1;
    }
    else if ( kAttachAudio.match(rawkey) )
    {
        attach_audio_cb( NULL, this );
        return 1;
    }
    else if ( kEditAudio.match(rawkey) )
    {
        edit_audio_cb( NULL, this );
        return 1;
    }
    else if ( kDetachAudio.match(rawkey) )
    {
        detach_audio_cb( NULL, this );
        return 1;
    }
    else if ( kFrameStepFPSBack.match(rawkey) )
    {
        mrv::media fg = foreground();
        if ( ! fg ) return 1;

        const CMedia* img = fg->image();

        double fps = 24;
        if ( img ) fps = img->play_fps();


        step_frame( int64_t(-fps) );
        mouseMove( Fl::event_x(), Fl::event_y() );
        redraw();
        return 1;
    }
    else if ( kFrameStepBack.match(rawkey) )
    {
        step_frame( -1 );
        mouseMove( Fl::event_x(), Fl::event_y() );
        redraw();
        return 1;
    }
    else if ( kFrameStepFPSFwd.match(rawkey) )
    {
        mrv::media fg = foreground();
        if ( ! fg ) return 1;

        const CMedia* img = fg->image();

        double fps = 24;
        if ( img ) fps = img->play_fps();

        step_frame( int64_t(fps) );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kFrameStepFwd.match( rawkey ) )
    {
        step_frame( 1 );
        mouseMove( Fl::event_x(), Fl::event_y() );
        redraw();
        return 1;
    }
    else if ( kPlayBackHalfSpeed.match( rawkey ) )
    {
        mrv::media fg = foreground();
        if ( ! fg ) return 1;

        const CMedia* img = fg->image();
        double FPS = 24.0f;
        if ( img ) {
            const double kFPS_INCR = img->orig_fps() / 2.0;
            FPS = img->play_fps();
            if ( img->playback() == CMedia::kForwards )
              {
                  if ( FPS > img->orig_fps() )
                    {
                        fps( FPS - kFPS_INCR );
                        play_forwards();
                    }
                  else
                    {
                        fps( img->orig_fps() );
                        play_backwards();
                    }
              }
            else if ( img->playback() == CMedia::kBackwards )
              {
                  fps( FPS + kFPS_INCR );
                  play_backwards();
              }
            else
              {
                  play_backwards();
              }
        }

        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kPlayBack.match( rawkey ) )
    {
        if ( playback() == CMedia::kBackwards )
            stop();
        else
            play_backwards();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kPlayFwdTwiceSpeed.match( rawkey ) )
    {
        mrv::media fg = foreground();
        if ( ! fg ) return 1;

        const CMedia* img = fg->image();
        double FPS = 24.0f;
        if ( img ) {
            const double kFPS_INCR = img->orig_fps() / 2.0;
            FPS = img->play_fps();
            if ( img->playback() == CMedia::kBackwards )
              {
                  if ( FPS > img->orig_fps() )
                    {
                        fps( FPS - kFPS_INCR );
                        play_backwards();
                    }
                  else
                    {
                        fps( img->orig_fps() );
                        play_forwards();
                    }
              }
            else if ( img->playback() == CMedia::kForwards )
              {
                    fps( FPS + kFPS_INCR );
                    play_forwards();
              }
            else
              {
                  play_forwards();
              }
        }
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kPlayFwd.match( rawkey ) )
    {
        if ( playback() == CMedia::kForwards )
            stop();
        else
            play_forwards();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kPlayDirection.match( rawkey ) )
    {
        if ( playback() != CMedia::kStopped )
        {
            stop();
        }
        else
        {
            if ( _orig_playback == CMedia::kBackwards )
                play_backwards();
            else
                play_forwards();
        }
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kStop.match( rawkey ) )
    {
        stop();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kSwitchFGBG.match( rawkey ) )
    {
        switch_fg_bg_cb( this, this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kPreviousVersionImage.match( rawkey ) )
    {
        previous_image_version_cb(this, browser());
        update_title_bar( this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kNextVersionImage.match( rawkey ) )
    {
        next_image_version_cb(this, browser());
        update_title_bar( this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kPreviousImageLimited.match( rawkey ) )
    {
        previous_image_limited_cb(this, browser());
        update_title_bar( this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kNextImageLimited.match( rawkey ) )
    {
        next_image_limited_cb(this, browser());
        update_title_bar( this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kPreviousImage.match( rawkey ) )
    {
        previous_image_cb(this, browser());
        update_title_bar( this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kNextImage.match( rawkey ) )
    {
        next_image_cb(this, browser());
        update_title_bar( this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kPreloadCache.match( rawkey ) )
    {
        preload_image_cache_cb( NULL, this );
        return 1;
    }
    else if ( kClearCache.match( rawkey ) )
    {
        clear_image_cache_cb( NULL, this );
        return 1;
    }
    else if ( kClearSingleFrameCache.match( rawkey ) )
    {
        update_frame_cb( NULL, this );
        return 1;
    }
    else if ( kRotatePlus90.match( rawkey ) )
    {
        rotate_plus_90_cb( NULL, this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kRotateMinus90.match( rawkey ) )
    {
        rotate_minus_90_cb( NULL, this );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kCopyFrameXYValues.match( rawkey ) )
    {
        toggle_copy_frame_xy_cb( NULL, this);
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kCopyRGBAValues.match( rawkey ) )
    {
        copy_pixel_rgba_cb( NULL, this);
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kToggleICS.match( rawkey ) )
    {
        toggle_ics_cb( NULL, main() );
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kFirstFrame.match( rawkey ) )
    {
        if ( Fl::event_state( FL_CTRL ) )
            first_frame_timeline();
        else
            first_frame();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kLastFrame.match( rawkey ) )
    {
        if ( Fl::event_state( FL_CTRL ) )
            last_frame_timeline();
        else
            last_frame();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kToggleEDL.match( rawkey ) )
    {
        toggle_edl_cb(this, this);
        return 1;
    }
    else if ( kToggleBG.match( rawkey ) )
    {
        toggle_background();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kToggleMenuBar.match( rawkey ) )
    {
        int H = uiMain->uiRegion->h();
        int w = uiMain->uiMenuGroup->w();
        // MenuBar MUST be 25 pixels-- for some reason it changes size
        uiMain->uiMenuGroup->size( w, int(25) );
        if ( uiMain->uiMenuGroup->visible() ) {
            has_menu_bar = false;
            uiMain->uiMenuGroup->hide();
            H += uiMain->uiMenuGroup->h();
        }
        else {
            has_menu_bar = true;
            fill_menu( uiMain->uiMenuBar );
            uiMain->uiMenuGroup->show();
            H -= uiMain->uiMenuGroup->h();
        }
        int X = uiMain->uiRegion->x();
        int Y = uiMain->uiRegion->y();
        int W = uiMain->uiRegion->w();
        uiMain->uiRegion->resize( X, Y, W, H );
        uiMain->uiRegion->init_sizes();
        uiMain->uiRegion->layout();
        uiMain->uiRegion->redraw();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kToggleTopBar.match( rawkey ) )
    {
        int H = uiMain->uiRegion->h();
        int w = uiMain->uiTopBar->w();
        // Topbar MUST be 28 pixels-- for some reason it changes size
        uiMain->uiTopBar->size( w, int(28) );
        if ( uiMain->uiTopBar->visible() ) {
            has_top_bar = false;
            uiMain->uiTopBar->hide();
            H += uiMain->uiTopBar->h();
        }
        else {
            has_top_bar = true;
            uiMain->uiTopBar->show();
            H -= uiMain->uiTopBar->h();
        }
        int X = uiMain->uiRegion->x();
        int Y = uiMain->uiRegion->y();
        int W = uiMain->uiRegion->w();
        uiMain->uiRegion->resize( X, Y, W, H );
        uiMain->uiRegion->init_sizes();
        uiMain->uiRegion->layout();
        uiMain->uiRegion->redraw();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kTogglePixelBar.match( rawkey ) )
    {
        int H = uiMain->uiRegion->h();
        if ( uiMain->uiPixelBar->visible() ) {
            has_pixel_bar = false;
            uiMain->uiPixelBar->hide();
            H += uiMain->uiPixelBar->h();
        }
        else {
            has_pixel_bar = true;
            uiMain->uiPixelBar->show();
            H -= uiMain->uiPixelBar->h();
        }
        int X = uiMain->uiRegion->x();
        int Y = uiMain->uiRegion->y();
        int W = uiMain->uiRegion->w();
        uiMain->uiRegion->resize( X, Y, W, H );
        uiMain->uiRegion->init_sizes();
        uiMain->uiRegion->layout();
        uiMain->uiRegion->redraw();
        return 1;
    }
    else if ( kToggleTimeline.match( rawkey ) )
    {
        int H = uiMain->uiRegion->h();
        if ( uiMain->uiBottomBar->visible() ) {
            has_bottom_bar = false;
            uiMain->uiBottomBar->hide();
            H += uiMain->uiBottomBar->h();
        }
        else {
            has_bottom_bar = true;
            uiMain->uiBottomBar->show();
            H -= uiMain->uiBottomBar->h();
        }
        int X = uiMain->uiRegion->x();
        int Y = uiMain->uiRegion->y();
        int W = uiMain->uiRegion->w();
        uiMain->uiRegion->resize( X, Y, W, H );
        uiMain->uiRegion->init_sizes();
        uiMain->uiRegion->layout();
        uiMain->uiRegion->redraw();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kToggleToolBar.match( rawkey ) )
    {
        toggle_action_tool_dock(NULL, uiMain);
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kToggleReel.match( rawkey ) )
    {
        // f4
        toggle_window( kReelWindow );
        return 1;
    }
    else if ( kToggleMediaInfo.match( rawkey ) )
    {
        toggle_window( kMediaInfo );
        return 1;
    }
    else if ( kToggleColorInfo.match( rawkey ) )
    {
        toggle_window( kColorInfo );
        return 1;
    }
    else if ( kToggleAction.match( rawkey ) )
    {
        toggle_window( kActionTools );
        return 1;
    }
    else if ( kToggleStereoOptions.match( rawkey ) )
    {
        toggle_window( k3DStereoOptions );
        return 1;
    }
    else if ( kToggleEDLEdit.match( rawkey ) )
    {
        toggle_window( kEDLEdit );
        return 1;
    }
    else if ( kTogglePreferences.match( rawkey ) )
    {
        toggle_window( kPreferences );
        return 1;
    }
    else if ( kTogglePixelRatio.match( rawkey ) )
    {
        toggle_pixel_ratio();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kToggleLut.match( rawkey ) )
    {
        toggle_lut();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kToggle3dView.match( rawkey ) )
    {
        toggle_window( k3dView );
        return 1;
    }
    else if ( kToggleHistogram.match( rawkey ) )
    {
        toggle_window( kHistogram );
        return 1;
    }
    else if ( kToggleVectorscope.match( rawkey ) )
    {
        toggle_window( kVectorscope );
        return 1;
    }
    else if ( kToggleWaveform.match( rawkey ) )
    {
        toggle_window( kWaveform );
        return 1;
    }
    else if ( kToggleICCProfiles.match( rawkey ) )
    {
        toggle_window( kICCProfiles );
        return 1;
    }
    else if ( kToggleConnections.match( rawkey ) )
    {
        toggle_window( kConnections );
        return 1;
    }
    else if ( kToggleHotkeys.match( rawkey ) )
    {
        toggle_window( kHotkeys );
        return 1;
    }
    else if ( kToggleLogs.match( rawkey ) )
    {
        toggle_window( kLogs );
        return 1;
    }
    else if ( kToggleAbout.match( rawkey ) )
    {
        toggle_window( kAbout );
        return 1;
    }
    else if ( kTogglePresentation.match( rawkey ) )
    {
        toggle_presentation();
        mouseMove( Fl::event_x(), Fl::event_y() );
        return 1;
    }
    else if ( kSetInPoint.match( rawkey ) )
    {
        uiMain->uiStartButton->do_callback();
        return 1;
    }
    else if ( kSetOutPoint.match( rawkey ) )
    {
        uiMain->uiEndButton->do_callback();
        return 1;
    }
    else if ( rawkey == FL_Alt_L )
    {
        flags |= kLeftAlt;
        return 1;
    }
    else if ( ( _mode == kNoAction || _mode == kScrub ) &&
              kAreaMode.match( rawkey ) )
    {
        selection_mode();
        return 1;
    }
    else
    {
        // Check if a menu shortcut
        mrv::PopupMenu* uiColorChannel = uiMain->uiColorChannel;

        // check if a channel shortcut
        uiColorChannel->menu_end();
        unsigned short num = uiColorChannel->children();
        unsigned short idx = 0;
        for ( unsigned short c = 0; c < num; ++c, ++idx )
        {
            const Fl_Menu_Item* w = uiColorChannel->child(c);
            if ( (int)rawkey == w->shortcut() )
            {
                channel( idx );
                return 1;
            }
        }
    }
    return 0;
}


/**
 * Handle a key release
 *
 * @param key fltk code for key pressed
 *
 * @return 1 if handled, 0 if not
 */
int ImageView::keyUp(unsigned int key)
{

#ifdef DEBUG_KEYS
  std::cerr << "KEYUP key: " << key << " flags " << flags << std::endl;
  std::cerr << "\tMouseDown: " << (flags & kMouseDown) << std::endl;
  std::cerr << "\tMouseLeft: " << (flags & kMouseLeft) << std::endl;
  std::cerr << "\tMouseMiddle: " << (flags & kMouseMiddle) << std::endl;
  std::cerr << "\tMouseRight: " << (flags & kMouseRight) << std::endl;
  std::cerr << "\tMouseMove: " << (flags & kMouseMove) << std::endl;
  std::cerr << "\tLeftAlt: " << (flags & kLeftAlt) << std::endl;
  std::cerr << "\tLeftShift: " << (flags & kLeftShift) << std::endl;
  std::cerr << "\tLeftCtrl: " << (flags & kLeftCtrl) << std::endl
              << std::endl;
  std::cerr << "\tGain: " << (flags & kGain) << std::endl;
  std::cerr << "\tGamma: " << (flags & kGamma) << std::endl;
#endif

  if ( ( key & FL_Alt_L ) == FL_Alt_L )
    {
        flags &= ~kLeftAlt;
        flags &= ~kZoom;
        return 1;
    }

#ifdef __APPLE__
  if ( ( key & FL_Control_L ) == FL_Control_L )
    {
      flags &= ~kGamma;
      flags &= ~kZoom;
      return 1;
    }
#endif


    if ( _mode & kTemporary && !Fl::get_key(key))
      {
          scrub_mode();
          flags &= ~kGain;
          flags &= ~kGamma;
          return 1;
      }

    if ( ( _mode & kSelection ) && (((key & FL_Shift_L) == FL_Shift_L) ||
                                    ((key & FL_Shift_R) == FL_Shift_R)))
    {
        send_selection();
        scrub_mode();
        flags &= ~kGain;
        flags &= ~kGamma;
        return 1;
    }
    else if ( flags & kLeftShift &&
              !Fl::get_key(FL_Shift_L) && !Fl::get_key( FL_Shift_R ) )
    {
      flags &= ~kGain;
      flags &= ~kGamma;
      return 1;
    }
    else if ( flags & kLeftCtrl &&
              !Fl::get_key(FL_Control_L) && !Fl::get_key( FL_Control_R ) )
    {
      flags &= ~kGamma;
      return 1;
    }

    return 0;
}

void ImageView::show_background( const bool b )
{
    _showBG = b;

    char buf[128];
    sprintf( buf, "ShowBG %d", (int) b );
    send_network( buf );

    damage_contents();
}


    void ImageView::show_bars( ViewerUI* uiMain, bool showtop )
    {
        if ( has_tools_grp ) {
            uiMain->uiToolsGroup->size( 45, 433 );
            uiMain->uiToolsGroup->show();
        }

        int W = uiMain->uiRegion->w();
        int H = uiMain->uiRegion->h();

        if ( has_menu_bar )    {
            // Menubar MUST be 25 pixels-- for some reason it changes size
            uiMain->uiMenuGroup->size( W, int(25) );
            fill_menu( uiMain->uiMenuBar );
            uiMain->uiMenuGroup->show();
            H -= uiMain->uiMenuGroup->h();
        }


        if ( has_top_bar )    {
            int w = uiMain->uiTopBar->w();
            // Topbar MUST be 28 pixels-- for some reason it changes size
            uiMain->uiTopBar->size( w, int(28) );
            uiMain->uiTopBar->show();
        }

        if ( has_bottom_bar)  {
            uiMain->uiBottomBar->size( W, int(49) );
            uiMain->uiBottomBar->show();
        }
        if ( has_pixel_bar )  {
            uiMain->uiPixelBar->size( W, int(30) );
            uiMain->uiPixelBar->show();
        }
        uiMain->uiViewGroup->layout();
        uiMain->uiViewGroup->init_sizes();
        uiMain->uiViewGroup->redraw();
        uiMain->uiRegion->layout();
        uiMain->uiRegion->init_sizes();
        uiMain->uiRegion->redraw();
        uiMain->uiView->redraw();
    }

/**
 * Toggle between a fullscreen view and a normal view with window borders.
 *
 */
void ImageView::toggle_fullscreen()
{

    // full screen...
    if ( !FullScreen && !presentation )
    {
        FullScreen = true;
        presentation = false;
        if ( fltk_main()->fullscreen_active() ) fltk_main()->fullscreen_off();
        posX = fltk_main()->x();
        posY = fltk_main()->y();
        sizeX = fltk_main()->w();
        sizeY = fltk_main()->h();
        fltk_main()->fullscreen();
        has_tools_grp  = uiMain->uiToolsGroup ?
                         uiMain->uiToolsGroup->visible() : false;
        has_menu_bar = uiMain->uiMenuGroup ?
                       uiMain->uiMenuGroup->visible() : false;
        has_top_bar = uiMain->uiTopBar->visible();
        has_bottom_bar = uiMain->uiBottomBar->visible();
        has_pixel_bar = uiMain->uiPixelBar->visible();
        uiMain->uiRegion->layout();
        uiMain->uiRegion->init_sizes();
    }
    else
    {
        if ( !presentation ) FullScreen = false;
        else FullScreen = true;
        presentation = false;
        show_bars( uiMain );
        fltk_main()->fullscreen_off( posX, posY, sizeX, sizeY );
        Fl::check();
        if ( FullScreen ) fltk_main()->fullscreen();
        else fltk_main()->resize( posX, posY, sizeX, sizeY );
    }


    fit_image();

    take_focus();


    char buf[128];
    sprintf( buf, "FullScreen %d", FullScreen );
    send_network( buf );
}

void ImageView::toggle_presentation()
{
    Fl_Window* uiImageInfo = uiMain->uiImageInfo->uiMain;
    Fl_Window* uiColorArea = uiMain->uiColorArea->uiMain;
    Fl_Window* uiColorControls = uiMain->uiColorControls->uiMain;
    Fl_Window* uiEDLWindow = uiMain->uiEDLWindow->uiMain;
    Fl_Window* uiICCProfiles  = uiMain->uiICCProfiles->uiMain;
    Fl_Window* uiReel  = uiMain->uiReelWindow->uiMain;
    Fl_Window* uiPrefs = uiMain->uiPrefs->uiMain;
    Fl_Window* uiAbout = uiMain->uiAbout->uiMain;
    Fl_Window* uiStereo = uiMain->uiStereo->uiMain;
    Fl_Window* uiPaint = uiMain->uiPaint->uiMain;
    Fl_Window* uiLog = uiMain->uiLog->uiMain;

    static bool has_image_info, has_color_area, has_reel, has_edl_edit,
        has_icc, has_prefs, has_about, has_stereo, has_paint, has_log,
        has_color_ctrl;
    static TextureFiltering filter;

    if ( !presentation )
    {
        if ( !FullScreen )
        {
            posX = fltk_main()->x();
            posY = fltk_main()->y();
        }

        has_image_info = uiImageInfo ? uiImageInfo->visible() : false;
        has_color_area = uiColorArea ? uiColorArea->visible() : false;
        has_color_ctrl = uiColorControls ? uiColorControls->visible() : false;
        has_reel       = uiReel ? uiReel->visible() : false;
        has_icc        = uiICCProfiles->visible();
        has_edl_edit   = uiEDLWindow ? uiEDLWindow->visible() : false;
        has_prefs      = uiPrefs ? uiPrefs->visible() : false;
        has_about      = uiAbout ? uiAbout->visible() : false;
        has_menu_bar   = uiMain->uiMenuGroup->visible();
        has_top_bar    = uiMain->uiTopBar->visible();
        has_bottom_bar = uiMain->uiBottomBar->visible();
        has_pixel_bar  = uiMain->uiPixelBar->visible();
        has_paint      = uiPaint ? uiPaint->visible() : false;
        has_stereo     = uiStereo ? uiStereo->visible() : false;
        has_log        = uiLog ? uiLog->visible() : false;
        has_tools_grp  = uiMain->uiToolsGroup ?
                         uiMain->uiToolsGroup->visible() : false;
        filter         = texture_filtering();

        uiPaint->hide();
        uiStereo->hide();
        uiImageInfo->hide();
        uiColorArea->hide();
        uiColorControls->hide();
        uiReel->hide();
        uiICCProfiles->hide();
        uiEDLWindow->hide();
        uiPrefs->hide();
        uiAbout->hide();
        uiLog->hide();
        uiMain->uiToolsGroup->hide();
        uiMain->uiBottomBar->hide();
        uiMain->uiPixelBar->hide();
        uiMain->uiTopBar->hide();
        uiMain->uiMenuGroup->hide();

        if ( fltk_main()->fullscreen_active() ) fltk_main()->fullscreen_off();

        Fl::check();

        uiMain->uiRegion->init_sizes();
        uiMain->uiRegion->layout();

        uiMain->uiViewGroup->init_sizes();
        uiMain->uiViewGroup->layout();


        presentation = true;
        if ( (TextureFiltering) main()->uiPrefs->uiPrefsFiltering->value() ==
             kPresentationOnly )
        {
            texture_filtering( kBilinearFiltering );
        }

        if ( !FullScreen )
        {
            sizeX = fltk_main()->w();
            sizeY = fltk_main()->h();
        }

        fltk_main()->fullscreen();

    }
    else
    {
        if ( has_image_info ) uiImageInfo->show();
        if ( has_icc )        uiICCProfiles->show();
        if ( has_color_area ) uiColorArea->show();
        if ( has_color_ctrl ) uiColorControls->show();
        if ( has_reel  )      uiReel->show();
        if ( has_edl_edit )   uiEDLWindow->show();
        if ( has_prefs )      uiPrefs->show();
        if ( has_about )      uiAbout->show();
        if ( has_paint )      uiPaint->show();
        if ( has_stereo )     uiStereo->show();
        if ( has_log )        uiLog->show();

        texture_filtering( filter );
        presentation = FullScreen = false;

        show_bars( uiMain, false );

        fltk_main()->fullscreen_off();
        Fl::check();

        show_bars( uiMain, true );


        fltk_main()->resize( posX, posY, sizeX, sizeY );
    }

    fit_image();

    take_focus();

    char buf[128];
    sprintf( buf, "PresentationMode %d", presentation );
    send_network( buf );

}

/**
 * Scrub the sequence
 *
 * @param dx > 0 scrub forwards, < 0 scrub backwards
 */
void ImageView::scrub( double dx )
{
    stop();

    uiMain->uiPlayForwards->value(0);
    uiMain->uiPlayBackwards->value(0);

    step_frame( int64_t(dx) );
}

void ImageView::toggle_color_control( bool show )
{
    if ( !show )
    {
        uiMain->uiColorControls->uiMain->hide();
    }
    else
    {
        uiMain->uiColorControls->uiMain->show();
    }
}

void ImageView::toggle_color_area( bool show )
{
    if ( !show )
    {
        uiMain->uiColorArea->uiMain->hide();
    }
    else
    {
        uiMain->uiColorArea->uiMain->show();
        update_color_info();
    }
}

void ImageView::toggle_histogram( bool show )
{
    if ( !show )
    {
        uiMain->uiHistogram->uiMain->hide();
    }
    else
    {
        uiMain->uiHistogram->uiMain->show();
    }
}

void ImageView::toggle_vectorscope( bool show )
{
    if ( !show )
    {
        uiMain->uiVectorscope->uiMain->hide();
    }
    else
    {
        uiMain->uiVectorscope->uiMain->show();
    }
}


void ImageView::toggle_waveform( bool show )
{
    if ( !show )
    {
        uiMain->uiWaveform->uiMain->hide();
    }
    else
    {
        uiMain->uiWaveform->uiMain->show();
    }
}

void ImageView::toggle_stereo_options( bool show )
{
    if ( !show )
    {
        uiMain->uiStereo->uiMain->hide();
    }
    else
    {
        uiMain->uiStereo->uiMain->show();
    }
}

void ImageView::toggle_paint_tools( bool show )
{
    if ( !show )
    {
        uiMain->uiPaint->uiMain->hide();
    }
    else
    {
        uiMain->uiPaint->uiMain->show();
    }
}

void ImageView::toggle_3d_view( bool show )
{
    if ( !show )
    {
        uiMain->uiGL3dView->uiMain->hide();
    }
    else
    {
        uiMain->uiGL3dView->uiMain->show();
    }
}

void ImageView::toggle_media_info( bool show )
{
    if ( !show )
    {
        uiMain->uiImageInfo->uiMain->hide();
    }
    else
    {
        uiMain->uiImageInfo->uiMain->show();
        update_image_info();
    }
}


void ImageView::handle_timeout()
{

    mrv::ImageBrowser* b = browser();
    if ( b && !_idle_callback && CMedia::cache_active()  &&
         ( CMedia::preload_cache() ||
           uiMain->uiPrefs->uiPrefsPlayAllFrames->value() ) )
    {
        size_t _reel = b->number_of_reels();
        size_t i = b->reel_index();
        for ( ; i < b->number_of_reels(); ++i )
        {
            mrv::Reel r = b->reel_at( unsigned(i) );
            if (!r) continue;

            mrv::media fg = r->media_at( frame() );
            if (!fg) continue;

            CMedia* img = fg->image();
            if ( !img->is_cache_full() && !img->has_video() &&
                 img->stopped() )
            {
                _reel = i;
                break;
            }
        }

        if ( _reel < b->number_of_reels() )
        {
            preload_cache_start();
        }
    }
    else
    {
        if ( _idle_callback )
        {
            if ( b->reel_index() >= b->number_of_reels() )
            {
                preload_cache_stop();
            }
            else
            {
                mrv::Reel r = b->current_reel();
                if ( r && !r->edl )
                {
                    mrv::media fg = foreground();
                    if ( fg )
                    {
                        CMedia* img = fg->image();
                        if ( img->is_cache_full() )
                        {
                            preload_cache_stop();
                        }
                    }
                }
            }
        }
    }

    timeout();

}

/**
 * Main fltk event handler
 *
 * @param event event to handle
 *
 * @return 1 if handled, 0 if not
 */
int ImageView::handle(int event)
{
    int ret = Fl_Gl_Window::handle( event );
    if ( ret && event == FL_PUSH ) {
        redraw();
        return ret;
    }

    switch( event )
    {
    case FL_FOCUS:
    {
        if ( _wait )
        {
            window()->cursor( FL_CURSOR_WAIT );
        }
        else
        {
            if ( !presentation && !( _mode & kScrub || _mode & kSelection ||
                                     _mode & kArrow || _mode & kCircle ||
                                     _mode & kRectangle ) )
                window()->cursor( FL_CURSOR_CROSS );
        }
        return 1;
    }
    case FL_ENTER:
        if ( !presentation && !( _mode & kScrub || _mode & kSelection ||
                                 _mode & kArrow || _mode & kCircle ||
                                 _mode & kRectangle ) )
            window()->cursor( FL_CURSOR_CROSS );
        if ( _mode & kDraw || _mode & kErase || _mode & kArrow ||
             _mode & kCircle ||  _mode & kRectangle )
            window()->cursor( FL_CURSOR_NONE );
        redraw();
        return 1;
    case FL_UNFOCUS:
        if ( !presentation )
            window()->cursor( FL_CURSOR_DEFAULT );
        return 1;
    case FL_LEAVE:
        if ( !presentation )
            window()->cursor( FL_CURSOR_DEFAULT );
        redraw();
        return 1;
    case FL_PUSH:
        if (!children()) take_focus();
        return leftMouseDown(Fl::event_x(), Fl::event_y());
        break;
    case FL_RELEASE:
        leftMouseUp(Fl::event_x(), Fl::event_y());
        return 1;
        break;
    case FL_MOVE:
        // Store X, Y for MOUSEWHEEL support
        X = Fl::event_x();
        Y = Fl::event_y();

        cursor_counter = 0;
        if ( presentation )
            window()->cursor( FL_CURSOR_CROSS );

        if ( _wipe_dir != kNoWipe )
        {
            char buf[128];
            switch( _wipe_dir )
            {
            case kWipeVertical:
                _wipe = (float) Fl::event_x() / (float)w();
                sprintf( buf, "WipeVertical %g", _wipe );
                send_network( buf );
                window()->cursor(FL_CURSOR_WE);
                break;
            case kWipeHorizontal:
                _wipe = (float) (h() - Fl::event_y()) / (float)h();
                sprintf( buf, "WipeHorizontal %g", _wipe );
                send_network( buf );
                window()->cursor(FL_CURSOR_NS);
                break;
            default:
                break;
            }


            if ( playback() == CMedia::kStopped )
                redraw();

            return 1;
        }

        if ( playback() == CMedia::kStopped )
            mouseMove(int(X), int(Y));

        lastX = X;
        lastY = Y;
        redraw();

        return 1;
        break;
    case FL_DRAG:
        X = Fl::event_x();
        Y = Fl::event_y();
        mouseDrag( int(X), int(Y) );
        mouseMove( int(X), int(Y) );
        return 1;
        break;
    case FL_SHORTCUT:
    case FL_KEYBOARD:
        // DON'T DO THIS!
        // lastX = Fl::event_x();
        // lastY = Fl::event_y();
        if ( !keyDown( Fl::event_key() ) )
        {
            return 0; //Fl_Gl_Window::handle( event );
        }
        return 1;
    case FL_KEYUP:
        if ( ! keyUp(Fl::event_key() ) )
        {
            return 0; //Fl_Gl_Window::handle( event );
        }
        return 1;
    case FL_MOUSEWHEEL:
    {
        if ( vr() )
        {
            float t = vr_angle();
            t += Fl::event_dy();
            vr_angle( t );
        }
        else
        {
            float dy = Fl::event_dy();
            if ( Fl::event_state( FL_ALT ) )
            {
                scrub( -dy );
            }
            else if ( Fl::event_state( FL_SHIFT ) )
            {
                volume( volume() - dy / 50 );
            }
            else
            {
                int idx = uiMain->uiPrefs->uiPrefsZoomSpeed->value();
                const float speedValues[] = { 0.1f, 0.25f, 0.5f };
                float speed = speedValues[idx];
                float change;
                if ( dy > 0 )
                {
                    change = 1.0f + dy * speed;
                    change = 1.0f / change;
                }
                else
                {
                    change = 1.0f - dy * speed;
                }
                zoom_under_mouse( _zoom * change, X, Y );
            }
        }
        return 1;
        break;
    }
    case FL_DND_ENTER:
    case FL_DND_LEAVE:
    case FL_DND_DRAG:
    case FL_DND_RELEASE:
    case FL_SHOW:
    case FL_HIDE:
    {
        return 1;
    }
    case FL_PASTE:
        {
            std::string text;
            if ( Fl::event_text() ) text = Fl::event_text();
            browser()->dnd_text( text );
            LOG_INFO( "DND: " << text );
            Fl::add_idle( (Fl_Idle_Handler)static_handle_dnd, browser() );
            return 1;
        }
    default:
        break;
    }

    return ret;
}



/**
 * Refresh the images in the view window
 *
 */
void ImageView::refresh()
{
    mrv::media fg = foreground();

    DBG3;
    if ( fg )
    {
        CMedia* img = fg->image();
        img->refresh();
    }

    DBG3;
    mrv::media bg = background();
    if ( bg )
    {
        CMedia* img = bg->image();
        img->refresh();
    }
    DBG3;
}

void ImageView::clear_reel_cache( size_t idx )
{
    mrv::ImageBrowser* b = browser();
    if (!b) return;

    mrv::Reel r = b->reel_at( unsigned(idx) );
    if ( !r ) return;

    if ( r->edl )
    {
        timeline()->edl( true );
        for ( size_t i = 0; i < r->images.size(); ++i )
        {
            mrv::media fg = r->images[i];
            if ( fg )
            {
                CMedia* img = fg->image();
                if ( img->first_frame() != img->last_frame() )
                {
                    img->clear_cache();
                }
            }
        }
    }
    else
    {
        mrv::media fg;
        if ( idx == (size_t)_fg_reel )
            fg = foreground();
        else
            fg = background();

        if ( fg )
        {
            CMedia* img = fg->image();
            if ( img->first_frame() != img->last_frame() )
            {
                img->clear_cache();
            }
        }
    }

    _preframe = frame();

}

void ImageView::flush_image( mrv::media fg )
{
    if ( fg )
    {
        CMedia* img = fg->image();
        if ( img->is_sequence() &&
                img->first_frame() != img->last_frame() &&
                ( _engine->shader_type() == DrawEngine::kNone )  )
        {
            img->clear_cache();
            image_type_ptr canvas;
            if ( img->fetch(canvas, frame() ) )
            {
                img->cache( canvas );
            }
        }
    }
}


void ImageView::reset_caches()
{
    mrv::Reel r = browser()->reel_at( fg_reel() );
    if ( r && r->edl )
    {
        timeline()->edl( true );
        _preframe = frame();
    }
    else
    {
        mrv::media fg = foreground();
        if (!fg) return;
        CMedia* img = fg->image();
        _preframe = img->frame();
    }
}


void ImageView::preload_cache_start()
{
    if ( !foreground() ) return;

    if (!_idle_callback)
    {
        CMedia* img = NULL;
        mrv::Reel r = browser()->reel_at( fg_reel() );
        if ( r && r->edl )
        {
            timeline()->edl( true );
            _preframe = frame();
        }
        mrv::media m = foreground();
        if ( m ) {
            img = m->image();
            if ( !img->is_stereo() )
            {
                CMedia::preload_cache( true );
                _idle_callback = true;
                if ( !Fl::has_timeout( (Fl_Timeout_Handler) static_preload,
                                       this ) )
                    Fl::add_timeout( 1.0/img->fps(),
                                     (Fl_Timeout_Handler) static_preload,
                                     this );
            }
        }
        //Fl::add_idle( (Fl_Timeout_Handler) static_preload, this );
    }
}

/**
 * Toggle Preload sequence in background
 *
 */
void ImageView::preload_cache_stop()
{
    if ( !foreground() ) return;

    if ( _idle_callback )
    {
        _idle_callback = false;
        if ( Fl::has_timeout( (Fl_Timeout_Handler) static_preload, this ) )
            Fl::remove_timeout( (Fl_Timeout_Handler) static_preload, this );
        //Fl::remove_idle( (Fl_Timeout_Handler) static_preload, this );
    }
}

/**
 * Toggle Preload sequence in background
 *
 */
void ImageView::preload_caches()
{
    if ( !foreground() ) return;


    CMedia::preload_cache( !CMedia::preload_cache() );
    if ( !CMedia::preload_cache() )
    {
        preload_cache_stop();
    }
    else
    {
        preload_cache_start();
    }
}

/**
 * Clear and refresh sequence
 *
 */
void ImageView::clear_caches()
{
    clear_reel_cache( _fg_reel );
    clear_reel_cache( _bg_reel );

    mrv::Timeline* t = timeline();
    if (t) t->redraw();
}


/**
 * Clear and refresh sequence
 *
 */
void ImageView::flush_caches()
{
    flush_image( foreground() );
    flush_image( background() );

    mrv::Timeline* t = timeline();
    if (t) t->redraw();
}

/**
 * Clear and refresh sequence
 *
 */
void ImageView::smart_refresh()
{
    if ( !_engine ) return;

    if ( _engine->shader_type() == DrawEngine::kNone )
    {
        refresh();
    }

    redraw();
}

char* ImageView::get_layer_label( unsigned short c )
{
    mrv::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    char* lbl = NULL;
    unsigned short idx = 0;
    uiColorChannel->menu_end();
    unsigned num = uiColorChannel->children();
    std::string layername;
    const Fl_Menu_Item* o = NULL;
    const Fl_Menu_Item* w; //uiColorChannel->child(0);
    for ( unsigned i = 0; i < num; ++i, ++idx )
    {
        w = uiColorChannel->child(i);
        if ( w->flags & FL_SUBMENU )
            o = w;
        else
            if ( !w->label() ) o = NULL;
        if ( idx == c )
        {
            if ( w != o && o && o->label() ) layername = o->label();
            if ( !layername.empty() ) layername += '.';
            if ( w->label() ) layername += w->label();
            lbl = av_strdup( layername.c_str() );
            break;
        }
    }


    if ( !lbl )
    {
        LOG_WARNING( _("Color channel not found at index ") << c );
        return NULL;
    }

    return lbl;
}

void ImageView::channel( Fl_Menu_Item* o )
{
    mrv::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return;
    uiColorChannel->menu_end();
    unsigned int i = 0;
    for ( ; i < num; ++i )
    {
        const Fl_Menu_Item* w = uiColorChannel->child(i);
        if ( w == o )
            break;
    }

    if ( i >= num )
    {
        LOG_ERROR( "Channel " << i << " not found in image" );
        return;
    }

    channel( i );
}

/**
 * Change image display to a new channel (from channel list)
 *
 * @param c index of channel
 */
void ImageView::channel( unsigned short c )
{
    boost::recursive_mutex::scoped_lock lk( _shortcut_mutex );

    mrv::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return; // Audio only - no channels

    uiColorChannel->menu_end();
    const Fl_Menu_Item* o = uiColorChannel->child(c);

    unsigned short idx = num;
    /*
    const Fl_Menu_Item* w ;
    for ( unsigned short i = 0; i < num; ++i, ++idx )
    {
        w = uiColorChannel->child(i);
        if ( w->flags & FL_SUBMENU )
        {
            for ( ++w; w->label(); ++w )
                ++idx;
        }
    }
    */

    if ( c >= idx )
    {
        c = 0;
        const Fl_Menu_Item* w = NULL;
        const char* lbl = uiColorChannel->label();
        if ( lbl && strcmp( lbl, _("(no image)") ) != 0 )
        {
            uiColorChannel->menu_end();
            num = uiColorChannel->children();
            for ( unsigned short i = 0; i < num; ++i, ++c )
            {
                w = uiColorChannel->child(i);
                if ( w->label() && strcmp( w->label(), lbl ) == 0 )  break;
                // if ( w->flags & FL_SUBMENU )
                // {
                //     for ( ; w->label(); w = w->next(), ++idx )
                //     {
                //      if ( strcmp( w->label(), lbl ) == 0 )
                //      {
                //          found = true;
                //          break;
                //      }
                //     }
                //     if ( found ) break;
                // }
            }
        }

        if ( c >= idx || (w && ! w->label() ))
        {
            LOG_ERROR( _("Invalid index ") << c
                       << _(" for channel.  Maximum: " )
                       << idx << _(". Widget label is ")
                       << ((w && w->label()) ? w->label() : "empty" ) );
            return;
        }
    }

    // If user selected the same channel again, toggle it with
    // other channel (diffuse.r goes to diffuse, for example)
    const mrv::media fg = foreground();
    if ( c == _channel && fg && _old_fg == fg->image() && network_active() ) {
        c = _old_channel;
    }
    _old_channel = _channel;

    if ( fg )
        _old_fg = fg->image();
    else
        _old_fg = NULL;


    char* lbl = get_layer_label( c );
    if ( !lbl ) return;

    _channel = c;


    char buf[128];
    sprintf( buf, "Channel %d %s", c, lbl );
    send_network( buf );

    std::string channelName( lbl );

    static std::string oldChannel;

    std::string ext = channelName;
    std::string oext = oldChannel;



    size_t pos = ext.rfind('.');
    size_t pos2 = oext.rfind('.');

    // Get ext from channel
    if ( pos != std::string::npos && pos != ext.size() )
    {
        ext = ext.substr( pos+1, ext.size() );
    }

    // Get old ext from channel
    if ( pos2 != std::string::npos && pos2 != oext.size() )
    {
        oext = oext.substr( pos2+1, oext.size() );
    }

    // Get suffix from label
    std::string x = lbl;
    size_t loc = x.find( '.' );
    if ( x[0] == '#' && loc != std::string::npos && loc != x.size() )
    {
        x = x.substr( loc+1, x.size() );
    }

    uiColorChannel->menu_end();
    uiColorChannel->value( c );
    uiColorChannel->copy_label( x.c_str() );
    uiColorChannel->redraw();

    std::transform( ext.begin(), ext.end(), ext.begin(),
                    (int(*)(int))toupper );


    _channelType = kRGB;
    if ( channelName == _("Alpha Overlay") )
    {
        _channelType = kAlphaOverlay;
    }
    else if ( channelName == _("Red") || ext == N_("R") || ext == N_("X") ||
              ext == N_("U") || ext == N_("S") || ext == N_("RED") )
    {
        _channelType = kRed;
    }
    else if ( channelName == _("Green") || ext == N_("G") || ext == N_("Y") ||
              ext == N_("V") || ext == N_("T") || ext == N_("GREEN") )
    {
        _channelType = kGreen;
    }
    else if ( channelName == _("Blue")  || ext == N_("B") || ext == N_("Z") ||
              ext == N_("W") || ext == N_("BLUE") )
    {
        _channelType = kBlue;
    }
    else if ( channelName == _("Alpha") || ext == N_("A") ||
              ext == N_("ALPHA") )
    {
        _channelType = kAlpha;
    }
    else if ( channelName == _("Lumma") )
    {
        _channelType = kLumma;
    }


    mrv::media bg = background();

    {
        if ( fg ) fg->image()->channel( lbl );
        if ( bg ) bg->image()->channel( lbl );
    }

    update_image_info();

    // We must send the full channel name
    update_shortcuts( fg, channelName.c_str() );

    oldChannel = channelName;
    av_free( lbl );

    // When changing channels, the cache may get thrown away.  Reflect that
    // in the timeline.
    timeline()->redraw();
    if ( browser()->reel_index() >= browser()->number_of_reels() )
        browser()->reel( (unsigned)0 );

    mrv::Reel r = browser()->current_reel();
    if ( r && r->edl )
    {
        timeline()->edl( true );
        _preframe = frame();
    }
    else
    {
        CMedia* img = fg->image();
        _preframe = img->frame();
    }



    smart_refresh();
}

void ImageView::refresh_fstop() const
{
    float exposure = calculate_exposure();
    float fstop = calculate_fstop( exposure );
    set_fstop_display( exposure, fstop );
}

/**
 * Change images' gain
 *
 * @param f new gain value
 */
void ImageView::gain( const float f )
{
    if ( _gain == f ) return;

    _gain = f;

    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    char buf[256];
    sprintf( buf, "Gain %g", f );
    send_network( buf );

    restore_locale( oldloc );

    uiMain->uiGain->value( f );
    uiMain->uiGainInput->value( f );

    refresh_fstop();
    flush_caches();
    smart_refresh();
    update_color_info();
}


/**
 * Change images' gamma
 *
 * @param f new gamma value
 */
void ImageView::gamma( const float f )
{
    if ( _gamma == f ) return;

    _gamma = f;

    mrv::media fg = foreground();
    if ( fg )
    {
        DBGM3(  "gamma " << f );
        fg->image()->gamma( f );

        char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
        setlocale( LC_NUMERIC, "C" );

        char buf[256];
        sprintf( buf, "Gamma %g", f );
        send_network( buf );

        restore_locale( oldloc );

        uiMain->uiGamma->value( f );
        uiMain->uiGammaInput->value( f );

        flush_caches();
        smart_refresh();
        update_color_info();
    }
}


/**
 * Change view's zoom factor
 *
 * @param z new zoom factor
 */
void ImageView::zoom( float z )
{

    if ( z > kMaxZoom || z < kMinZoom ) return;

    char tmp[128];
    if ( z >= 1.0f )
    {
        sprintf( tmp, N_("x%.2g"), z );
    }
    else
    {
        sprintf( tmp, N_("1/%.3g"), 1.0f/z );
    }
    uiMain->uiZoom->copy_label( tmp );

    _zoom = z;
    if ( z >= 32.0f ) { _zoom_grid = true; }
    else _zoom_grid = false;

    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    char buf[128];
    sprintf( buf, N_("Zoom %g"), z );
    send_network( buf );

    restore_locale( oldloc );

    float scale = Fl::screen_scale( window()->screen_num() );
    _real_zoom = z / scale;

    redraw();
}


/**
 * Calculate an fstop value given an exposure
 *
 * @param exposure exposure value
 *
 * @return fstop number
 */
float ImageView::calculate_fstop( float exposure ) const
{
    float base = 3.0f; // for exposure 0 = f/8

    float seq1, seq2;

    // Chack if image has an F Stop or Aperture ATTRIBUTES attribute
    mrv::media fg = foreground();
    if ( fg )
    {
        CMedia* img = fg->image();

        CMedia::Attributes& attrs = img->attributes();
        CMedia::Attributes::const_iterator i = attrs.find( N_("F Number") );
        if ( i == attrs.end() )
        {
            i = attrs.find( N_("Aperture Value") );
        }

        if ( i != attrs.end() )
        {
            float exp = 1.0f;
            {
                Imf::RationalAttribute* attr =
                    dynamic_cast< Imf::RationalAttribute* >( i->second );
                if ( attr )
                {
                    Imf::Rational& r = attr->value();

                    exp = (float) r.n / (float) r.d;
                }
            }
            {
                Imf::StringAttribute* attr =
                    dynamic_cast< Imf::StringAttribute* >( i->second );
                if ( attr )
                {
                    int n = 8;
                    unsigned d = 1;
                    int num = sscanf( attr->value().c_str(), "%d / %d", &n, &d );
                    if ( num == 2 ) exp = (float) n / (float) d;
                }
            }
            seq1 = seq2 = 0.0f;
            base = 0.0f;

            for ( ; seq1 < exp && seq2 < exp; base += 1.0f )
            {
                seq1 = Imath::Math<float>::pow( 2.0f, base+1);
                seq2 = 1.4f * Imath::Math<float>::pow( 2.0f, base);
            }

            float t = seq1 - exp;
            if ( fabs(t) < fabs(seq2 - exp) )
            {
                exposure += t / fabs(seq2 - seq1);
            }
            else
            {
                --base;
                seq1 = Imath::Math<float>::pow( 2.0f, base);

                t = fabs(seq2 - exp);
                if ( t >= fabs(seq1 - exp) )
                {
                    t = fabs(seq1-exp) / fabs(seq2 - seq1);
                }
                else
                {
                    t = 1.0f - t / fabs(seq2 - seq1);
                }
                exposure -= t;
            }
        }
    }


    // F-stop progression = 1, 1.4, 2, 2.8, 4, 5.6, 8, 11, 16, 22, 32, 44, 64
    //
    // can be described as a lerp between two sequences:
    //
    //     v:   0    1    2   3   4   5   6
    //  seq1:   1,   2,   4,  8, 16, 32, 64
    //  seq2: 1.4, 2.8, 5.6, 11, 22, 44, 88   -- .5 bases

    float e = exposure * 0.5f;
    float v = (float) base + (float) int( -e );

    float f = Imath::Math<float>::fmod( fabs(exposure), 2.0f );
    if ( exposure >= 0 )
    {
        seq1 = 1.0f * Imath::Math<float>::pow( 2.0f, v);    // 8
        seq2 = 1.4f * Imath::Math<float>::pow( 2.0f, v-1);  // 5.6
    }
    else
    {
        seq1 = 1.0f * Imath::Math<float>::pow( 2.0f, v);  // 8
        seq2 = 1.4f * Imath::Math<float>::pow( 2.0f, v);  // 11
    }


    float fstop = seq1 * (1-f) + f * seq2;
    return fstop;
}


/**
 * Calculate exposure value from gain.
 *
 *
 * @return exposure value
 */
float ImageView::calculate_exposure() const
{
    float exposure = ( Imath::Math<float>::log(_gain) /
                       Imath::Math<float>::log(2.0f) );
    return exposure;
}


/**
 * Set the f-stop button display
 *
 * @param exposure exposure value
 * @param fstop    fstop value
 */
void ImageView::set_fstop_display( float exposure, float fstop ) const
{
    char m[64];
    sprintf( m, "%1.1f  f/%1.1f", exposure, fstop );
    uiMain->uiFstop->copy_label( m );
    uiMain->uiFstop->redraw();
}


/**
 * Change exposure by some unit
 *
 * @param d unit to change exposure by
 */
void ImageView::exposure_change( float d )
{
    float exposure = calculate_exposure() + d;
    gain( Imath::Math<float>::pow( 2.0f, exposure ) );
    uiMain->uiGain->value( _gain );
    uiMain->uiGainInput->value( _gain );

    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    char buf[64];
    sprintf( buf, N_("Gain %g"), _gain );
    uiMain->uiView->send_network( buf );

    restore_locale( oldloc );
}


/**
 * Perform a zoom keeping the x and y coordinates relatively
 * consistant.
 *
 * @param z zoom factor
 * @param x window's x coordinate
 * @param y window's y coordinate
 */
void ImageView::zoom_under_mouse( float z, int x, int y )
{
    if ( !vr() && (z > kMaxZoom || z < kMinZoom ||
                   (z == 1.0f && _zoom == z) ) ) return;
    double mw = (double) w() / 2;
    double mh = (double) h() / 2;
    double offx = mw - x;
    double offy = mh - y;
    double xf = (double) x;
    double yf = (double) y;

    mrv::media fg = foreground();
    if ( ! fg ) {
        zoom( z );
        return;
    }

    CMedia* img = fg->image();

    const mrv::Recti& dpw = img->display_window();

    int W = dpw.w();
    int H = dpw.h();

    double pr = 1.0f;
    if ( _showPixelRatio ) pr = img->pixel_ratio();

    H /= pr;

    float scale = Fl::screen_scale( window()->screen_num() );

    image_coordinates( img, xf, yf );

    zoom( z );

    int w2 = W / 2;
    int h2 = H / 2;

    yf /= pr;

    xoffset = w2 - xf;
    yoffset = h2 - ( H - yf );
    xoffset -= (offx / _real_zoom);
    yoffset += (offy / _real_zoom);

    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    char buf[128];
    sprintf( buf, N_("Offset %g %g"), xoffset, yoffset );
    send_network( buf );

    restore_locale( oldloc );

    mouseMove( x, y );
}


double ImageView::pixel_ratio() const
{
    mrv::media fg = foreground();
    if ( !fg ) return 1.0f;
    return fg->image()->pixel_ratio();
}

int ImageView::update_shortcuts( const mrv::media fg,
                                 const char* channelName )
{
    if ( !fg ) return -1;

    CMedia* img = fg->image();

    mrv::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    uiColorChannel->menu_end();
    if ( uiColorChannel->popped() ) {
        return -1;
    }

    uiColorChannel->clear();


    const stringArray& layers = img->layers();


    stringArray::const_iterator i = layers.begin();
    stringArray::const_iterator e = layers.end();


    int v   = -1;
    std::set< unsigned short > shortcuts;

    std::string root;

    size_t pos = 0;

    if (!channelName) v = 0;
    else
    {
        root = channelName;


        // Root name if channel name minus the last .extension.
        // Like sub.AO.R, root is sub.AO
        pos = root.find('.');

        if ( pos != std::string::npos )
        {
            root = root.substr( 0, pos );
        }
        else
        {
            // if ( root == _("Red") || root == _("Green") ||
            //      root == _("Blue") || root == _("Alpha") ||
            //      root == _("Alpha Overlay") ||
            //      root == _("Lumma") )
            //     root = _("Color");
        }
    }

    int idx = -1;
    bool group = false;
    std::string x;
    Fl_Menu_Item* g = NULL;
    Fl_Menu_Item* o = NULL;

    for ( ; i != e; ++i, ++idx )
    {

        const std::string& name = *i;


        std::string tmp = x + '.';
        if ( o && x != _("Alpha") && tmp != "." && name.find(tmp) == 0 )
        {
            if ( group )
            {
                // Copy shortcut to group and replace leaf with group
                // unsigned s = 0;
                if ( uiColorChannel->children() >= 2 )
                {
                    uiColorChannel->menu_end();
                    unsigned last = uiColorChannel->children()-2;
                    Fl_Menu_Item* w = (Fl_Menu_Item*)uiColorChannel->child(last);
                    // s = w->shortcut();
                    if ( w->flags & FL_SUBMENU )
                        uiColorChannel->clear_submenu( last );
                    uiColorChannel->remove( last );
                    uiColorChannel->menu_end();
                }

                // int idx = uiColorChannel->add( x.c_str(), s, NULL, 0 );
                // g  = (Fl_Menu_Item*) uiColorChannel->child(idx);
                group = false;
            }

            // Now add current leaf, but without # prefix and period
            std::string y = name;

            if ( x.size() < name.size() )
                y = x + '/' + name.substr( x.size()+1, name.size() );

            idx = uiColorChannel->add( y.c_str() );
            uiColorChannel->menu_end();
            o = (Fl_Menu_Item*) uiColorChannel->child(idx);
        }
        else
        {
            // A new group, we add it here as empty group
            group = true;
            x = name;

            idx = uiColorChannel->add( name.c_str(), 0, NULL, 0 );
            uiColorChannel->menu_end();
            o = (Fl_Menu_Item*) uiColorChannel->child(idx);
        }

        // If name matches root name or name matches full channel name,
        // store the index to the channel.
        std::string chx = remove_hash_number( x );
        std::string chroot = remove_hash_number( root );
        if ( v == -1 && ( chx == chroot ||
                          (channelName && name == channelName) ) )
        {
            v = idx;
        }

        // Get a shortcut to this layer
        short shortcut = get_shortcut( name.c_str() );


        // N, Z and Color are special in that they don't change, except
        // when in Stereo, but then they are not called that.
        if ( v >= 0 || ( name == _("Color") || name == _("Red") ||
                         name == _("Green") || name == _("Blue") ||
                         name == _("Alpha") || name == _("Lumma") ||
                         name == _("Alpha Overlay") || name == _("Z")
                         /* || chx == "N" || chx == "Z" */ ) )
        {
            // If we have a shortcut and it isn't in the list of shortcuts
            // yet, add it to interface and shortcut list.
           if ( shortcut && shortcuts.find( shortcut ) ==
                 shortcuts.end())
            {
                o->shortcut( shortcut );
                shortcuts.insert( shortcut );
            }
        }

    }

    // If no channel was selected, select first channel
    if ( v == -1 ) v = 0;

    return v;
}

/**
 * Update image channel/layers display
 *
 */
void ImageView::update_layers()
{
    boost::recursive_mutex::scoped_lock lk( _shortcut_mutex );

    mrv::PopupMenu* uiColorChannel = uiMain->uiColorChannel;

    mrv::media fg = foreground();
    if ( !fg )
    {
        _old_fg = NULL;
        uiColorChannel->clear();
        uiColorChannel->add( _("(no image)") );
        uiColorChannel->copy_label( _("(no image)") );
        uiColorChannel->menu_end();
        uiColorChannel->value(0);
        uiColorChannel->redraw();
        return;
    }

    const char* lbl = uiColorChannel->label();
    if ( strcmp(lbl, _("(no image)")) == 0 ) lbl = _("Color");

    int v = update_shortcuts( fg, lbl );

    CMedia* img = fg->image();
    if ( v >= 0 )
    {
        if ( img != _old_fg )
        {
            channel( (unsigned short) v );
        }
        img->image_damage( img->image_damage() & ~CMedia::kDamageLayers );
    }

    uiColorChannel->redraw();
}


/**
 * Change foreground image
 *
 * @param img new foreground image or NULL for no image.
 */
void ImageView::foreground( mrv::media fg )
{
    mrv::media old = foreground();
    if ( old == fg ) {
        fill_menu( uiMain->uiMenuBar );
        return;
    }

    CMedia::StereoInput  stereo_in = stereo_input();
    CMedia::StereoOutput stereo_out = stereo_output();

    _fg_reel = browser()->reel_index();

    CMedia* img = NULL;
    if ( fg )
    {
        img = fg->image();

        double fps = img->fps();
        if ( img->is_sequence() &&
                uiMain->uiPrefs->uiPrefsOverrideFPS->value() )
        {
            fps = uiMain->uiPrefs->uiPrefsFPS->value();
            img->fps( fps );
            img->play_fps( fps );
        }
        const int64_t& tc = img->timecode();
        timeline()->fps( fps );
        timeline()->timecode( tc );
        uiMain->uiFrame->timecode( tc );
        uiMain->uiFrame->fps( fps );
        uiMain->uiStartFrame->fps( fps );
        uiMain->uiStartFrame->timecode( tc );
        uiMain->uiEndFrame->timecode( tc );
        uiMain->uiEndFrame->fps( fps );
        uiMain->uiFPS->value( img->play_fps() );

        if ( uiMain->uiPrefs->uiPrefsOverrideAudio->value() )
        {
            img->volume( _volume );

            CMedia* right = img->right_eye();
            if ( right ) right->volume( _volume );
        }
        else
        {
            mrv::AudioEngine* engine = img->audio_engine();
            if ( engine )
            {
                _volume = engine->volume();
                uiMain->uiVolume->value( _volume );
            }
        }
    }

    _fg = fg;


    if ( fg )
    {
        CMedia* img = fg->image();

        if ( img )
        {
            if ( ! img->is_cache_full() && !_idle_callback &&
                 CMedia::cache_active() && CMedia::preload_cache() )
                preload_cache_start();


            if ( img->looping() == CMedia::kUnknownLoop )
            {
                img->looping( looping() );
            }
            else
            {
                looping( img->looping() );
            }

            // Per session gamma: requested by Willa
            if ( img->gamma() > 0.0f ) gamma( img->gamma() );

            img->stereo_input( stereo_in );
            img->stereo_output( stereo_out );

            refresh_fstop();

            if ( img->width() > 160 && (FullScreen || presentation) ) {
                fit_image();
            }

            if ( uiMain->uiPrefs->uiPrefsAutoFitImage->value() )
            {
                if ( _zoom <= 1.0f )
                {
                    fit_image();
                }
                else if ( old )
                {
                    CMedia* o = old->image();
                    if (o)
                    {
                       if ( o->display_window() != img->display_window() )
                       {
                           fit_image();
                       }
                    }
                }
            }


            img->image_damage( img->image_damage() | CMedia::kDamageContents |
                               CMedia::kDamageLut | CMedia::kDamageLayers |
                               CMedia::kDamage3DData );

            if ( dynamic_cast< stubImage* >( img )  )
            {
                create_timeout( 0.2 );
            }

            update_ICS( fg );
        }
    }


    refresh_audio_tracks();

    // If this is the first image loaded, resize window to fit it
    if ( !old ) {
        Fl_Round_Button* r = (Fl_Round_Button*) uiMain->uiPrefs->uiPrefsOpenMode->child(0);
        if ( r->value() == 1 )
        {
            resize_main_window();
        }
    }

    update_layers();

    fill_menu( uiMain->uiMenuBar );

    update_title_bar( this );
    update_image_info();
    update_color_info();

    mrv::media bg = background();
    if ( fg == bg )
    {
        uiMain->uiBButton->copy_label( "B" );
        uiMain->uiBButton->down_box( FL_PLASTIC_DOWN_BOX );
        uiMain->uiBButton->selection_color( FL_YELLOW );
        uiMain->uiBButton->redraw();
    }
    else
    {
        uiMain->uiBButton->copy_label( "A/B" );
        uiMain->uiBButton->selection_color( FL_BACKGROUND_COLOR );
        uiMain->uiBButton->down_box( FL_PLASTIC_DOWN_BOX );
        uiMain->uiBButton->redraw();
    }

    change_foreground();

    //if (_engine && valid() ) _engine->refresh_shaders();

    redraw();
}


/**
 * Update the pull-down menu of audio tracks
 *
 */
void ImageView::refresh_audio_tracks() const
{
    mrv::media fg = foreground();
    if ( ! fg ) return;

    CMedia* img = fg->image();

    uiMain->uiAudioTracks->clear();
    size_t numTracks = img->number_of_audio_streams();

    for ( size_t i = 0; i < numTracks; ++i )
    {
        char buf[80];
        sprintf( buf, "Track #%02zd", i );
        uiMain->uiAudioTracks->add( buf );
    }
    uiMain->uiAudioTracks->add( "<no audio>" );
    uiMain->uiAudioTracks->value( img->audio_stream() );
    uiMain->uiAudioTracks->menu_end();
    uiMain->uiAudioTracks->redraw();
}

/// Change audio stream
void ImageView::audio_stream( unsigned int idx )
{

    CMedia::Playback p = playback();
    stop();

    mrv::Reel reel = browser()->current_reel();
    if ( !reel ) return;

    if ( reel->edl )
    {
        mrv::MediaList::iterator i = reel->images.begin();
        mrv::MediaList::iterator e = reel->images.end();
        for ( ; i != e; ++i )
        {
            if (! *i) continue;
            CMedia* img = (*i)->image();
            if ( idx >= img->number_of_audio_streams() )
            {
                img->audio_stream( -1 );
            }
            else
            {
                img->audio_stream( idx );
            }
        }
    }
    else
    {
        mrv::media fg = foreground();
        if ( ! fg ) return;
        CMedia* img = fg->image();

        unsigned int numAudioTracks = uiMain->uiAudioTracks->children();
        if ( idx >= numAudioTracks - 1 )
            img->audio_stream( -1 );
        else
            img->audio_stream( idx );
    }

    if ( p != CMedia::kStopped ) play( p );

    char buf[64];
    sprintf( buf, N_("AudioStream %d"), idx );
    send_network( buf );
}


/**
 * Change current background image
 *
 * @param img new background image or NULL
 */
void ImageView::background( mrv::media bg )
{
    mrv::media old = background();
    if ( old == bg ) return;


    char buf[1024];

    _bg = bg;

    update_title_bar( this );


    if ( bg )
    {
        CMedia* img = bg->image();
        if (!img) return;

        std::string file = img->directory() + '/' + img->name();

        sprintf( buf, N_("CurrentBGImage \"%s\" %" PRId64 " %" PRId64),
                 file.c_str(), img->first_frame(), img->last_frame() );
        send_network( buf );


        img->refresh();

        if ( img->is_sequence() )
            img->play_fps( fps() );

        img->image_damage( img->image_damage() | CMedia::kDamageLut );


        if ( dynamic_cast< stubImage* >( img ) )
        {
            create_timeout( 0.2 );
        }
    }
    else
    {
        sprintf( buf, N_("CurrentBGImage \"\"") );
        send_network( buf );
    }

//   _BGpixelSize = 0;
    redraw();
}

void ImageView::resize( int X, int Y, int W, int H )
{
    Fl_Gl_Window::resize( X, Y, W, H );

    if ( uiMain->uiPrefs->uiPrefsAutoFitImage->value() )
    {
        if ( _zoom <= 1.0f )
        {
            fit_image();
        }
    }
}

/**
 * Resize the containing window to try to fit the image view.
 *
 */
void ImageView::resize_main_window()
{
    if ( fltk_main()->fullscreen_active() )
    {
        return;
    }

    int w, h;

    mrv::media fg = foreground();
    if ( !fg )
    {
        w = 1260;
        h = 800;
    }
    else
    {
        const CMedia* img = fg->image();

        w = img->width();
        h = img->height();
        h = (int) (h / img->pixel_ratio());
    }

    int minx, miny, maxw, maxh;

    int screen = window()->screen_num();
    float scale = Fl::screen_scale( screen );
    Fl::screen_work_area( minx, miny, maxw, maxh, screen );

    PreferencesUI* uiPrefs = uiMain->uiPrefs;
    if ( uiPrefs && uiPrefs->uiWindowFixedPosition->value() )
    {
        posX = (int) uiPrefs->uiWindowXPosition->value();
        posY = (int) uiPrefs->uiWindowYPosition->value();
    }
    else
    {
        posX = minx;
        posY = miny;
    }



    int decw = fltk_main()->decorated_w();
    int dech = fltk_main()->decorated_h();

    int dw = decw - fltk_main()->w();
    int dh = dech - fltk_main()->h();


    maxw -= dw;
    maxh -= dh;
    posX += dw / 2;
#ifdef _WIN32
    posY += dh - dw / 2;
#else
    posY += dh;
#endif

    int maxx = posX + maxw;
    int maxy = posY + maxh;


    bool fit = false;

    if ( w > maxw ) {
        fit = true;
        w = maxw;
    }
    if ( h > maxh ) {
        fit = true;
        h = maxh;
    }


    if ( uiMain->uiMenuGroup->visible() )
    {
      uiMain->uiMenuGroup->size( uiMain->uiMenuGroup->w(),
                               int(25) );
      h += uiMain->uiMenuGroup->h();
    }

    if ( uiMain->uiTopBar->visible() )
    {
      uiMain->uiTopBar->size( uiMain->uiTopBar->w(),
                              int(28) );
      h += uiMain->uiTopBar->h();
    }

    if ( uiMain->uiPixelBar->visible() )
    {
      uiMain->uiPixelBar->size( uiMain->uiPixelBar->w(),
                                int(30) );
      h += uiMain->uiPixelBar->h();
    }

    if ( uiMain->uiBottomBar->visible() )
    {
      uiMain->uiBottomBar->size( uiMain->uiBottomBar->w(),
                                 int(49) );
      h += uiMain->uiBottomBar->h();
    }

    if ( uiPrefs && uiPrefs->uiWindowFixedSize->value() )
    {
        w = (int) uiPrefs->uiWindowXSize->value();
        h = (int) uiPrefs->uiWindowYSize->value();
    }

    maxw = (int) (maxw / scale);
    if ( w < 660 )  w = 660;
    else if ( w > maxw )
    {
        w = maxw;
    }

    maxh = (int) (maxh / scale);
    if ( h < 535 )  h = 535;
    else if ( h > maxh )
    {
        h = maxh;
    }


    fltk_main()->resize( posX, posY, w, h );

    uiMain->uiMenuGroup->size( uiMain->uiMenuGroup->w(),
                               int(25) );

    uiMain->uiTopBar->size( uiMain->uiTopBar->w(),
                            int(28) );

    uiMain->uiPixelBar->size( uiMain->uiPixelBar->w(),
                              int(30) );

    uiMain->uiBottomBar->size( uiMain->uiBottomBar->w(),
                               int(49) );

    uiMain->uiRegion->init_sizes();
    uiMain->uiRegion->layout();


    uiMain->uiRegion->redraw();

    //valid(0);
    redraw();

    if ( fit ) fit_image();

}


/**
 * Toggle background image.
 *
 */
void ImageView::toggle_background()
{
    show_background( !show_background() );
    update_title_bar( this );
    redraw();
}

void ImageView::data_window( const bool b )
{
    _dataWindow = b;

    char buf[128];
    sprintf( buf, N_("DataWindow %d"), (int)b );
    send_network( buf );
}

void ImageView::display_window( const bool b )
{
    _displayWindow = b;

    char buf[128];
    sprintf( buf, N_("DisplayWindow %d"), (int)b );
    send_network( buf );
}

void ImageView::safe_areas( const bool b )
{
    _safeAreas = b;

    char buf[128];
    sprintf( buf, N_("SafeAreas %d"), (int)b );
    send_network( buf );
}

/**
 * Returns normalize state
 *
 */
bool ImageView::normalize() const
{
    return (bool) uiMain->uiNormalize->value();
}

void ImageView::normalize( const bool normalize)
{
    _normalize = normalize;
    uiMain->uiNormalize->value( normalize );

    char buf[128];
    sprintf( buf, N_("Normalize %d"), (int) _normalize );
    send_network( buf );

    damage_contents();
}

void ImageView::damage_contents()
{
    mrv::media fg = foreground();

    if (fg)
    {
        CMedia* img = fg->image();
        img->image_damage( img->image_damage() | CMedia::kDamageContents |
                           CMedia::kDamageLut );
        redraw();
    }

    mrv::media bg = background();
    if (bg)
    {
        CMedia* img = bg->image();
        img->image_damage( img->image_damage() | CMedia::kDamageContents |
                           CMedia::kDamageLut );
        redraw();
    }
}

/**
 * Toggle normalizing of foreground image channels
 *
 */
void ImageView::toggle_normalize()
{
    normalize( !_normalize );
}

/**
 * Toggle pixel ratio stretching
 *
 */
void ImageView::toggle_pixel_ratio()
{
    show_pixel_ratio( ! show_pixel_ratio() );
}

bool ImageView::show_pixel_ratio() const
{
    return _showPixelRatio;
}

void ImageView::show_pixel_ratio( const bool b )
{
    _showPixelRatio = b;

    char buf[64];
    sprintf( buf, N_("ShowPixelRatio %d"), (int) b );
    send_network( buf );

    uiMain->uiPixelRatio->value( b );


    damage_contents();
}


/**
 * Toggle 3D LUT
 *
 */
void ImageView::toggle_lut()
{
    _useLUT = !_useLUT;

    std::string display = mrv::Preferences::OCIO_Display;
    std::string view = mrv::Preferences::OCIO_View;

    char buf[1024];
    if ( _useLUT )
    {
        sprintf( buf, N_("OCIOView \"%s\" \"%s\""), display.c_str(),
                 view.c_str() );
        send_network( buf );
    }

    sprintf( buf, N_("UseLUT %d"), (int)_useLUT );
    send_network( buf );

    flush_caches();
    if ( _useLUT ) {
        if ( _engine )
        {
            _engine->refresh_luts();
        }
    }


    uiMain->uiLUT->value( _useLUT );
    uiMain->gammaDefaults->copy_label( view.c_str() );

    smart_refresh();

    update_color_info();
}


/**
 * Resize background image to fit foreground image.
 *
 */
void ImageView::resize_background()
{
}

/// Returns current frame number in view (uiFrame)
int64_t ImageView::frame() const
{
    return _frame;
}

/**
 * Change view's frame number in frame indicator and timeline
 *
 * @param f new frame in timeline units
 */
void ImageView::frame( const int64_t f )
{
    // Redraw browser to update thumbnail
    _frame = f;

    if  ( playback() == CMedia::kStopped )
    {
        mrv::ImageBrowser* b = browser();
        if ( b ) b->redraw();
    }

    redraw();
}




/**
 * Jump to a frame in timeline, creating new thumbnails if needed.
 *
 * @param f new frame in timeline units
 */
void ImageView::seek( const int64_t f )
{
    mrv::ImageBrowser* b = browser();

    {
        _preframe = f;
    }

    if ( b ) b->seek( f );

    thumbnails();
    update_color_info();
    mouseMove( lastX, lastY );


    _lastFrame = f;

}



/**
 * Change view's frame number
 *
 * @param f new frame
 */
void ImageView::step_frame( int64_t n )
{
    assert( n != 0 );
    if ( n == 0 ) return;

    stop_playback();

    int64_t start = (int64_t) timeline()->display_minimum();
    int64_t end   = (int64_t) timeline()->display_maximum();

    int64_t f = frame();

    CMedia::Looping loop = looping();

    if ( n > 0 )
    {
        if ( f + n > end )
        {
            if ( loop == CMedia::kLoop )
            {
                f = start + ( f + n - end) - 1;
            }
            else
            {
                f = end;
            }
        }
        else
        {
            f += n;
        }
    }
    else
    {
        if ( f + n < start )
        {
            if ( loop == CMedia::kLoop )
            {
                f = end - (start - f + n) - 1;
            }
            else
            {
                f = start;
            }
        }
        else
        {
            f += n;
        }
    }


    seek( f );
}


/// Change frame number to first frame of image or previous EDL image if
/// already on first frame
void ImageView::first_frame()
{
    mrv::media fg = foreground();
    if ( ! fg ) return;

    CMedia* img = fg->image();

    if (!img) return;

    int64_t f = img->first_frame();

    if ( timeline()->edl() )
    {
        f = fg->position();

        if ( int64_t( frame() ) == f )
        {
            browser()->previous_image();
            fit_image();
            return;
        }

        frame( f );
    }

    int64_t t = int64_t( timeline()->display_minimum() );
    if ( t > f ) f = t;

    seek( f );
}

/// Change frame number to last frame of image or next image in EDL
/// if on last frame
void ImageView::last_frame()
{

    mrv::media fg = foreground();
    if ( ! fg ) return;

    CMedia* img = fg->image();

    int64_t f = img->last_frame();

    if ( timeline()->edl() )
    {
        f -= img->first_frame();
        f += fg->position();
        if ( int64_t( frame() ) == f )
        {
            browser()->next_image();
            fit_image();
            return;
        }

        frame( f );

    }

    int64_t t = int64_t( timeline()->display_maximum() );
    if ( t < f ) f = t;

    seek( f );
}

/// Change frame number to start of timeline
void ImageView::first_frame_timeline()
{
    int64_t f = (int64_t) timeline()->minimum();
    seek( f );
}

/// Change frame number to end of timeline
void ImageView::last_frame_timeline()
{
    int64_t f = (int64_t) timeline()->maximum();
    seek( f );
}


/**
 * Update color information
 *
 */
void ImageView::update_color_info() const
{
    if ( uiMain->uiColorArea )
    {
        Fl_Window*  uiColorWindow = uiMain->uiColorArea->uiMain;
        if ( uiColorWindow->visible() )
        {
            uiMain->uiColorArea->uiColorText->update();
            uiMain->uiColorArea->uiColorText->redraw();
        }
    }

    if ( uiMain->uiVectorscope )
    {
        Fl_Window*  uiVectorscope = uiMain->uiVectorscope->uiMain;
        if ( uiVectorscope->visible() )
            uiMain->uiVectorscope->uiVectorscope->redraw();
    }

    if ( uiMain->uiWaveform )
    {
        Fl_Window*  uiWaveform = uiMain->uiWaveform->uiMain;
        if ( uiWaveform->visible() ) uiWaveform->redraw();
    }

    if ( uiMain->uiHistogram )
    {
        Fl_Double_Window*  uiHistogram   = uiMain->uiHistogram->uiMain;
        if ( uiHistogram->visible() )
            uiMain->uiHistogram->uiHistogram->redraw();
    }


}


/**
 * Update the image window information display
 *
 */

void ImageView::update_image_info() const
{
    if ( !uiMain->uiImageInfo ||
         !uiMain->uiImageInfo->uiInfoText->visible()) return;

    mrv::media fg = foreground();
    if ( ! fg ) return;

    CMedia* img = NULL;
    if ( selected_image() )
        img = selected_image();
    else
        img = fg->image();
    uiMain->uiImageInfo->uiInfoText->set_image( img );
}

void ImageView::playback( const CMedia::Playback b )
{
    if ( !uiMain ) return;

    _playback = _orig_playback = b;

    _lastFrame = this->frame();

    _last_fps = 0.0;

    if ( b == CMedia::kForwards )
    {
        uiMain->uiPlayForwards->value(1);
        uiMain->uiPlayBackwards->value(0);
    }
    else if ( b == CMedia::kBackwards )
    {
        uiMain->uiPlayForwards->value(0);
        uiMain->uiPlayBackwards->value(1);
    }
    else
    {
        uiMain->uiPlayForwards->value(0);
        uiMain->uiPlayBackwards->value(0);
    }

    uiMain->uiPlayForwards->redraw();
    uiMain->uiPlayBackwards->redraw();

    // take_focus();
}


/**
 * Play image sequence forwards.
 *
 */
void ImageView::play_forwards()
{
    stop();
    play( CMedia::kForwards );
}

/**
 * Play image sequence forwards or backwards.
 *
 */
void ImageView::play( const CMedia::Playback dir )
{

    if ( dir == playback() )
        return;

    if ( dir == CMedia::kForwards )
    {
        send_network(N_("playfwd"));
    }
    else if ( dir == CMedia::kBackwards )
    {
        send_network(N_("playback"));
    }
    else
    {
        LOG_ERROR( "Not a valid playback mode" );
        return;
    }


    mrv::media fg = foreground();
    if (!fg) return;

    mrv::media bg = background();

    CMedia* img = fg->image();
    if ( img->saving() ) return;

    if ( CMedia::preload_cache() && _idle_callback &&
         img->is_cache_full() )
    {
        preload_cache_stop();
    }
    else
    {
        _preframe = frame();

    }


    playback( dir );


    double fps = uiMain->uiFPS->value();

    create_timeout( 0.25 / fps );

    // if ( !img->is_sequence() || img->is_cache_full() || (bg && fg != bg) ||
    //      !CMedia::cache_active() ||
    //      !( CMedia::preload_cache() ||
    //         uiMain->uiPrefs->uiPrefsPlayAllFrames->value() ) ||
    //   img->has_audio() )
    {
        // preload_cache_stop();
        if ( img->start_frame() != img->end_frame() )
            img->play( dir, uiMain, true );
    }



    if ( bg && bg != fg )
    {
        CMedia* img = bg->image();
        img->play( dir, uiMain, false);
        typedef boost::recursive_mutex Mutex;
        Mutex& bgm = img->video_mutex();
        SCOPED_LOCK( bgm );
        CMedia::Barrier* barrier = img->fg_bg_barrier();
        if ( !barrier ) return;
        img = fg->image();
        Mutex& fgm = img->video_mutex();
        SCOPED_LOCK( fgm );
        barrier->threshold( barrier->threshold() + img->has_audio() +
                            img->has_picture() );
        img->fg_bg_barrier( barrier );
    }

}

/**
 * Play image sequence backwards.
 *
 */
void ImageView::play_backwards()
{
    play( CMedia::kBackwards );
}


void ImageView::thumbnails()
{
    if ( playback() != CMedia::kStopped ) return;


    mrv::media fg = foreground();
    if ( fg ) fg->create_thumbnail();


    mrv::media bg = background();
    if ( bg ) bg->create_thumbnail();

    mrv::ImageBrowser* b = browser();
    if ( b ) b->redraw();
}


bool ImageView::preload_cache_full( CMedia* img )
{
    if ( img->is_cache_full() ) return true;
    if ( Preferences::max_memory <= CMedia::memory_used )
        return true;
    return false;
}

/**
 * Stop image sequence.
 *
 */
void ImageView::stop()
{

    if ( playback() == CMedia::kStopped ) {
        return;
    }



    playback( CMedia::kStopped );

    _last_fps = 0.0;
    _real_fps = 0.0;

    stop_playback();



    frame( frame() );
    // seek( int64_t(timeline()->value()) );


    if ( CMedia::preload_cache() )
    {
        mrv::media fg = foreground();
        if (fg)
        {
            _preframe = fg->image()->first_cache_empty_frame();
            if ( _idle_callback )
            {
                preload_cache_stop();
            }
            if ( ! preload_cache_full( fg->image() ) )
                preload_cache_start();
        }
    }

    char buf[256];
    sprintf( buf, N_("stop %" PRId64), frame() );
    send_network( buf );


    mouseMove( Fl::event_x(), Fl::event_y() );
    redraw();
}



double ImageView::fps() const
{
    mrv::media fg = foreground();
    if ( fg ) return fg->image()->play_fps();

    mrv::media bg = background();
    if ( bg ) return bg->image()->play_fps();

    return 24;
}

/**
 * Change the frame rate of the video playback
 *
 * @param x new frame rate (in frames per second)
 */
void ImageView::fps( double x )
{
    if ( x <= 0 ) return;

    mrv::media fg = foreground();
    if ( fg ) fg->image()->play_fps( x );

    mrv::media bg = background();
    if ( bg ) bg->image()->play_fps( x );

    int64_t start = uiMain->uiStartFrame->frame();  // needed not sure why
    int64_t end   = uiMain->uiEndFrame->frame();  // needed not sure why
    timeline()->fps( x );
    uiMain->uiFrame->fps( x );
    uiMain->uiStartFrame->fps( x );
    uiMain->uiEndFrame->fps( x );
    uiMain->uiFPS->value( x );

    uiMain->uiStartFrame->frame( start );  // needed not sure why
    uiMain->uiEndFrame->frame( end );  // needed not sure why

    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    char buf[128];
    sprintf( buf, N_("FPS %g"), x );
    send_network( buf );

    restore_locale( oldloc );
}

/**
 * Change the audio volume
 *
 * @param v new volume value [ 0.0...1.0 ]
 */
void ImageView::volume( float v )
{

    _volume = v;

    mrv::media fg = foreground();
    if ( fg )
    {
        CMedia* img = fg->image();
        img->volume( v );
    }

    mrv::media bg = background();
    if ( bg )
    {
        CMedia* img = bg->image();
        img->volume( v );
    }

    uiMain->uiVolume->value( v );
    uiMain->uiVolume->redraw();


    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    char buf[128];
    sprintf( buf, N_("Volume %g"), v );
    send_network( buf );

    restore_locale( oldloc );

}

CMedia::Looping ImageView::looping() const
{
    if ( !uiMain || !uiMain->uiLoopMode )
        return CMedia::kUnknownLoop;
    return (CMedia::Looping) uiMain->uiLoopMode->value();
}

/// Set Playback looping mode
void  ImageView::looping( CMedia::Looping x )
{

    media fg = foreground();
    if (fg)
    {
        CMedia* img = fg->image();
        img->looping( x );
    }

    media bg = foreground();
    if (bg)
    {
        CMedia* img = bg->image();
        img->looping( x );
    }

    if ( x != CMedia::kUnknownLoop )
    {
        uiMain->uiLoopMode->value( x );
        uiMain->uiLoopMode->label(uiMain->uiLoopMode->child(x)->label());
        uiMain->uiLoopMode->redraw();
    }

    char buf[64];
    sprintf( buf, N_("Looping %d"), x );
    send_network( buf );

}

/**
 * Change the field to display
 *
 * @param p enum 0=Frame, 1=Top, 2=Bottom
 */
void ImageView::field( FieldDisplay f )
{
    _field = f;

    static const char* kFieldNames[] = {
        _("Frame"),
        _("Top"),
        _("Bottom")
    };

    const char* p = kFieldNames[_field];
    uiMain->uiField->copy_label( select_character( p ) );

    char buf[64];
    sprintf( buf, "FieldDisplay %d", _field );
    send_network( buf );

    damage_contents();
}


/// Change stereo main image and attach B image
    void ImageView::change_foreground()
    {
        uiMain->uiBButton->color( FL_BACKGROUND_COLOR );

        mrv::media fg = foreground();
        if (!fg) return;


        CMedia* img = fg->image();
        img->is_stereo( false );
        img->is_left_eye( true );
        if ( !img->owns_right_eye() )
            img->right_eye( NULL );

        mrv::media bg = background();
        if ( !bg || fg == bg )
            return;

        CMedia* bimg = bg->image();

        img->is_stereo( true );
        img->is_left_eye( true );
        if ( !img->owns_right_eye() || img->right_eye() == NULL )
        {
            img->right_eye( bimg );
            img->owns_right_eye( false );
        }

        bimg->is_stereo( true );
        bimg->is_left_eye( false );
        bimg->right_eye( NULL );

    }

} // namespace mrv
