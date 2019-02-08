/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later versioxn.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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


#include <cassert>
#include <cmath>


#include <inttypes.h>  // for PRId64


#if defined(_WIN32) || defined(_WIN64)
#  include <float.h>
#  define isnan(x) _isnan(x)
#endif

#include <iostream>
#include <sstream>
#include <limits>
#include <iomanip>
#include <sstream>
#include <set>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#define isfinite(x) _finite(x)
#endif

#include "video/mrvGLLut3d.h"
#include "core/CMedia.h"
#include "core/aviImage.h"

#include <GL/gl.h>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <fltk/utf.h>
#include <fltk/visual.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/draw.h>
#include <fltk/run.h>

#include <fltk/Color.h>
#include <fltk/Cursor.h>
#include <fltk/Font.h>
#include <fltk/Output.h>
#include <fltk/Choice.h>
#include <fltk/ValueOutput.h>
#include <fltk/Window.h>
#include <fltk/Menu.h>
#include <fltk/PopupMenu.h>
#include <fltk/Monitor.h>
#include <fltk/Button.h>
#include <fltk/Preferences.h>
#ifdef LINUX
#include <fltk/x11.h>
static Atom fl_NET_WM_STATE;
static Atom fl_NET_WM_STATE_FULLSCREEN;
#endif

#include <ImathMath.h> // for Math:: functions
#include <ImfRationalAttribute.h>
#include <ImfStringAttribute.h>

// CORE classes
#include "core/mrvClient.h"
#include "core/mrvColor.h"
#include "core/mrvColorProfile.h"
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
#include "core/mrvACES.h"
#include "core/ctlToLut.h"

// GUI classes
#include "gui/mrvColorInfo.h"
#include "gui/mrvFileRequester.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageInformation.h"
#include "gui/mrvIO.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvMainWindow.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvHotkey.h"
#include "mrvEDLWindowUI.h"
#include "mrvSOPNode.h"
#include "mrvAudioOffset.h"
#include "gui/mrvFontsWindowUI.h"
#include "gui/mrvVersion.h"
#include "gui/mrvLogDisplay.h"
#include "gui/mrvImageView.h"

#undef TRACE
#define TRACE(x)

#undef LOG
#define LOG(x) std::cerr << x << std::endl;

// Widgets
#include "mrViewer.h"
#include "mrvIccProfileUI.h"
#include "mrvColorAreaUI.h"

// Video
#include "video/mrvGLEngine.h"  // should be dynamically chosen from prefs

#undef DBG
#define DBG(x)
// Audio

// #define USE_TIMEOUT // USE TIMEOUTS INSTEAD OF IDLE CALLBACK

using namespace std;

namespace fs = boost::filesystem;

// FLTK2 currently has a problem on Linux with timout's fltk::event_x/y not
// taking into account other child widgets.  This works around it.
//#ifndef _WIN32
#  define FLTK_TIMEOUT_EVENT_BUG 1
//#endif



namespace
{
const char* kModule = "gui";
}


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

inline std::string remove_hash_number( std::string r )
{
    if ( r.empty() || r[0] != '#' ) return r;

    size_t pos = r.find( ' ' );
    if ( pos != std::string::npos )
    {
        r = r.substr( pos+1, r.size() );
    }
    return r;
}

inline std::string extract_root( std::string r )
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
    for ( unsigned int i = 0; i < sizeof(shortcuts)/sizeof(ChannelShortcuts); ++i )
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

#ifdef LINUX
void send_wm_event(XWindow wnd, Atom message,
                   unsigned long d0, unsigned long d1=0,
                   unsigned long d2=0, unsigned long d3=0,
                   unsigned long d4=0) {
    XEvent e;
    e.xany.type = ClientMessage;
    e.xany.window = wnd;
    e.xclient.message_type = message;
    e.xclient.format = 32;
    e.xclient.data.l[0] = d0;
    e.xclient.data.l[1] = d1;
    e.xclient.data.l[2] = d2;
    e.xclient.data.l[3] = d3;
    e.xclient.data.l[4] = d4;
    XSendEvent(fltk::xdisplay, RootWindow(fltk::xdisplay, fltk::xscreen),
               0, SubstructureNotifyMask | SubstructureRedirectMask,
               &e);
}

#define _NET_WM_STATE_REMOVE        0  /* remove/unset property */
#define _NET_WM_STATE_ADD           1  /* add/set property */
#define _NET_WM_STATE_TOGGLE        2  /* toggle property  */

void send_wm_state_event(XWindow wnd, int add, Atom prop) {
    send_wm_event(wnd, fl_NET_WM_STATE,
                  add ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE, prop);
}
#endif

}

extern void clone_all_cb( fltk::Widget* o, mrv::ImageBrowser* b );
extern void clone_image_cb( fltk::Widget* o, mrv::ImageBrowser* b );

void edit_audio_cb( fltk::Widget* o, mrv::ImageView* v )
{
    edit_audio_window( o, v );
}

void preload_image_cache_cb( fltk::Widget* o, mrv::ImageView* v )
{
    v->preload_caches();
}

void clear_image_cache_cb( fltk::Widget* o, mrv::ImageView* v )
{
    v->clear_caches();
}

void rotate_plus_90_cb( fltk::Widget* o, mrv::ImageView* v )
{
    mrv::media fg = v->foreground();
    if (!fg) return;

    mrv::CMedia* img = fg->image();
    img->rotate( -90.0 ); // this is reversed on purpose
    img->image_damage( mrv::CMedia::kDamageContents );
    v->fit_image();
    v->redraw();
}


void toggle_ics_cb( fltk::Widget* o, mrv::ViewerUI* main )
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
        main->uiICS->relayout();
        main->uiICS->show();
    }
}

void rotate_minus_90_cb( fltk::Widget* o, mrv::ImageView* v )
{
    mrv::media fg = v->foreground();
    if (!fg) return;

    mrv::CMedia* img = fg->image();
    img->rotate( 90.0 );  // this is reversed on purpose
    img->image_damage( mrv::CMedia::kDamageContents );
    v->fit_image();
    v->redraw();
}

void update_frame_cb( fltk::Widget* o, mrv::ImageView* v )
{
    mrv::media fg = v->foreground();
    if (!fg) return;

    mrv::CMedia* img = fg->image();
    img->has_changed();
    v->redraw();
}

void next_image_version_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    b->next_image_version();
}

void previous_image_version_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    b->previous_image_version();
}

void next_image_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    b->next_image();
}

void previous_image_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
    b->previous_image();
}

void next_channel_cb( fltk::Widget* o, mrv::ImageView* v )
{
    v->next_channel();
}

void previous_channel_cb( fltk::Widget* o, mrv::ImageView* v )
{
    v->previous_channel();
}

void switch_channels_cb( fltk::Widget* o, mrv::ImageView* v )
{
    v->switch_channels();
}

void set_as_background_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    view->bg_reel( view->fg_reel() );
    view->background( fg );
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
                  view->show_background() ? _("Shown") : _("Not Shown") );
    }
    else if ( fg )
    {
        snprintf( bufs, 255, _("mrViewer    FG: %s"),
                  fg->image()->name().c_str() );
    }
    else
    {
        snprintf( bufs, 32, _("mrViewer") );
    }

    view->main()->uiMain->copy_label( bufs );
}

void switch_fg_bg_cb( fltk::Widget* o, mrv::ImageView* view )
{
    const mrv::media& fg = view->foreground();
    if ( !fg ) {
        LOG_ERROR( _("No foreground image to switch to") );
        return;
    }
    size_t fg_reel = view->fg_reel();

    mrv::media bg = view->background();
    if ( !bg ) {
        LOG_ERROR( _("No background image to switch to") );
        return;
    }
    size_t bg_reel = view->bg_reel();

    if ( fg == bg )
    {
        LOG_INFO( _("Cannot switch images.  Foreground and background image are the same.") );
        return;
    }

    view->foreground( bg );
    view->fg_reel( bg_reel );

    view->background( fg );
    view->bg_reel( fg_reel );

    mrv::ViewerUI* m = view->main();
    mrv::Timeline* t = m->uiTimeline;
    mrv::CMedia* img = bg->image();
    // int64_t f = m->uiFrame->value();

    // std::cerr << "f " << f << std::endl;

    // if ( f > img->last_frame() ) f = img->last_frame();
    // else if ( f < img->first_frame() ) f = img->first_frame();
    m->uiStartFrame->value( img->first_frame() );
    m->uiEndFrame->value( img->last_frame() );
    t->minimum( img->first_frame() );
    t->maximum( img->last_frame() );
    // m->uiFrame->value( f );
    // t->value( f );

    update_title_bar( view );
    view->fit_image();
}

void toggle_background_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;
    view->toggle_background();
}

void open_dir_cb( fltk::Widget* o, mrv::ImageBrowser* uiReelWindow )
{
    uiReelWindow->open_directory();
}

void open_cb( fltk::Widget* o, mrv::ImageBrowser* uiReelWindow )
{
    uiReelWindow->open();
}

void open_single_cb( fltk::Widget* o, mrv::ImageBrowser* uiReelWindow )
{
    uiReelWindow->open_single();
}

void open_clip_xml_metadata_cb( fltk::Widget* o,
                                mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    mrv::CMedia* img = fg->image();
    read_clip_xml_metadata( img );
}

void open_stereo_cb( fltk::Widget* o,
                     mrv::ImageBrowser* uiReelWindow )
{
    uiReelWindow->open_stereo();
}

void save_clip_xml_metadata_cb( fltk::Widget* o,
                                mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    mrv::CMedia* img = fg->image();
    save_clip_xml_metadata( img );
}

void save_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    view->browser()->save();
}

void save_reel_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    view->browser()->save_reel();
}

void save_snap_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    view->stop();


    mrv::CMedia* img = fg->image();
    if ( !img ) return;

    mrv::save_sequence_file( view->main(), NULL, true );

    // Return all to normal
    view->redraw();

}

void save_sequence_cb( fltk::Widget* o, mrv::ImageView* view )
{
    view->stop();
    view->browser()->save_sequence();
}



void masking_cb( fltk::Widget* o, mrv::ViewerUI* uiMain )
{
    mrv::ImageView* view = uiMain->uiView;

    float mask = 1.0f;
    const char* fmt = o->label();

    mask = (float) atof( fmt );

    char buf[128];
    sprintf( buf, "Mask %g", mask );
    view->send_network( buf );

    view->masking( mask );


    view->redraw();
}


void change_video_cb( fltk::Widget* o, mrv::ImageView* view )
{
    fltk::Group* g = o->parent();
    if ( !g ) return;
    fltk::Menu* p = dynamic_cast< fltk::Menu* >( g );
    if ( !p ) return;

    if ( !view) return;

    mrv::media fg = view->foreground();
    if ( !fg ) return;

    int i = p->value();  // Video Track #
    fg->image()->video_stream(i);
}


void change_subtitle_cb( fltk::Widget* o, mrv::ImageView* view )
{
    fltk::Group* g = o->parent();
    if ( !g ) return;
    fltk::Menu* p = dynamic_cast< fltk::Menu* >( g );
    if ( !p ) return;

    if ( !view) return;

    mrv::media fg = view->foreground();
    if ( !fg ) return;

    int i = (int)p->value() - 2;  // Load Subtitle,  No subtitle
    fg->image()->subtitle_stream(i);

}

void hud_toggle_cb( fltk::Widget* o, mrv::ViewerUI* uiMain )
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

void hud_cb( fltk::Widget* o, mrv::ViewerUI* uiMain )
{
    mrv::ImageView* view = uiMain->uiView;

    int i;
    int num = uiMain->uiPrefs->uiPrefsHud->children();
    for ( i = 0; i < num; ++i )
    {
        const char* fmt = uiMain->uiPrefs->uiPrefsHud->child(i)->label();
        if ( strcmp( fmt, o->label() ) == 0 ) break;
    }

    unsigned int hud = view->hud();
    hud ^= ( 1 << i );
    view->hud( (mrv::ImageView::HudDisplay) hud );
    view->redraw();
}


void safe_areas_cb( fltk::Widget* o, mrv::ImageView* view )
{
    view->safe_areas( !view->safe_areas() );
    view->redraw();
}


void display_window_cb( fltk::Widget* o, mrv::ImageView* view )
{
    view->display_window( !view->display_window() );
    view->redraw();
}

void data_window_cb( fltk::Widget* o, mrv::ImageView* view )
{
    view->data_window( !view->data_window() );
    view->redraw();
}

static const float kMinZoom = 0.01f;  // Zoom 1/64
static const float kMaxZoom = 64.f;   // Zoom 64x



namespace mrv {

void window_cb( fltk::Widget* o, const mrv::ViewerUI* uiMain )
{
    int idx = -1;
    fltk::Group* g = o->parent();
    for ( int i = 0; i < g->children(); ++i )
    {
        if ( o == g->child(i) ) {
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

void ImageView::toggle_window( const ImageView::WindowList idx, const bool force )
{
    if ( idx == kReelWindow )
    {
        // Reel window
        if ( force || !uiMain->uiReelWindow->uiMain->visible() )
            uiMain->uiReelWindow->uiMain->show();
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
    else if ( idx == k3DStereoOptions )
    {
        if ( force || !uiMain->uiStereo->uiMain->visible() )
        {
            uiMain->uiStereo->uiMain->child_of( uiMain->uiMain );
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
            uiMain->uiEDLWindow->uiMain->child_of( uiMain->uiMain );
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
            uiMain->uiPaint->uiMain->child_of( uiMain->uiMain );
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
    else if ( idx == kICCProfiles )
    {
        if ( force || !uiMain->uiICCProfiles->uiMain->visible() )
        {
            uiMain->uiICCProfiles->uiMain->show();
            uiMain->uiICCProfiles->fill();
        }
        else
        {
            uiMain->uiICCProfiles->uiMain->hide();
        }
    }
    else if ( idx == kConnections )
    {
        if ( force || !uiMain->uiConnection->uiMain->visible() )
        {
            uiMain->uiConnection->uiMain->child_of( uiMain->uiMain );
            uiMain->uiConnection->uiMain->show();
        }
        else
        {
            uiMain->uiConnection->uiMain->hide();
        }
    }
    else if ( idx == kPreferences )
    {
        if ( force || !uiMain->uiPrefs->uiMain->visible() )
        {
            uiMain->uiPrefs->uiMain->child_of( uiMain->uiMain );
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
            uiMain->uiHotkey->uiMain->child_of( uiMain->uiMain );
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
            uiMain->uiLog->uiMain->child_of( uiMain->uiMain );
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


static void attach_color_profile_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    attach_icc_profile( fg->image() );
}


void load_subtitle_cb( fltk::Widget* o, mrv::ViewerUI* uiMain )
{
    const char* file = open_subtitle_file( NULL, uiMain );
    if ( !file ) return;

    mrv::ImageView* view = uiMain->uiView;

    mrv::media fg = view->foreground();
    if ( !fg ) return;


    aviImage* img = dynamic_cast< aviImage* >( fg->image() );
    if ( !img )
    {
        LOG_ERROR( _("Subtitles are only valid on video files") );
        return;
    }

    img->subtitle_file( file );
}

static void flip_x_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::ImageView::FlipDirection dir = view->flip();
    view->flip( (mrv::ImageView::FlipDirection)
                ( dir ^ mrv::ImageView::kFlipVertical ) );
    view->redraw();
}

static void flip_y_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::ImageView::FlipDirection dir = view->flip();
    view->flip( (mrv::ImageView::FlipDirection)
                ( dir ^ mrv::ImageView::kFlipHorizontal ) );
    view->redraw();
}

static void attach_ctl_script_cb( fltk::Widget* o, mrv::ImageView* view )
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

    CMedia* img = fg->image();
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

    mrv::ViewerUI* main = view->main();
    if ( main->uiSOPNode )
    {
        main->uiSOPNode->media( fg );
        main->uiSOPNode->uiMain->show();
    }
    else
    {
        main->uiSOPNode = new mrv::SopNode( view );
    }
    img->image_damage( img->image_damage() | CMedia::kDamageAll );
    view->use_lut( true );
    view->redraw();
}

void modify_sop_sat_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    modify_sop_sat( view );
}

static void attach_ctl_lmt_script_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    CMedia* img = fg->image();

    attach_ctl_lmt_script( img, img->number_of_lmts(), view->main() );
}

void ImageView::update_ICS() const
{
    mrv::media fg = foreground();
    if ( ! fg ) {
        return;
    }
    CMedia* img = fg->image();
    fltk::PopupMenu* o = uiMain->uiICS;
    int n = o->children();
    for ( int i = 0; i < n; ++i )
    {
        fltk::Widget* w = o->child(i);
        if ( img->ocio_input_color_space() == w->label() )
        {
            o->copy_label( w->label() );
            o->value(i);
            if (w->tooltip()) o->tooltip( w->tooltip() );
            o->redraw();
            return;
        }
    }
    o->copy_label( "scene_linear"  );
    o->redraw();
}

void attach_ocio_ics_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    attach_ocio_input_color_space( fg->image(), view );

    view->update_ICS();


}
static void attach_ocio_display_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    attach_ocio_display( fg->image(), view );
}
static void attach_ocio_view_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    attach_ocio_view( fg->image(), view );
}

static void attach_ctl_idt_script_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    attach_ctl_idt_script( fg->image(), view->main() );
}

static void monitor_icc_profile_cb( fltk::Widget* o, ViewerUI* uiMain )
{
    monitor_icc_profile(uiMain);
}


static void monitor_ctl_script_cb( fltk::Widget* o, ViewerUI* uiMain )
{
    monitor_ctl_script(uiMain);
}

static void copy_pixel_rgba_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( ! fg ) return;

    view->copy_pixel();
}


static void attach_audio_cb( fltk::Widget* o, mrv::ImageView* view )
{
    mrv::media fg = view->foreground();
    if ( !fg ) return;

    const char* file = open_audio_file();
    if ( file == NULL ) return;

    CMedia* img = fg->image();
    if ( img == NULL ) return;

    DBG( "Attach audio file " << file << " first frame: " << img->first_frame() );
    img->audio_file( file );
    DBG( "Attached audio file " << file << " first frame: " << img->first_frame() );
    view->refresh_audio_tracks();

}


static void detach_audio_cb( fltk::Widget* o, mrv::ImageView* view )
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
    uiMain->uiPaint->uiMovePic->value(true);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    redraw();
}

void ImageView::move_pic_mode()
{
    _mode = kMovePicture;
    uiMain->uiPaint->uiMovePic->value(true);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    redraw();
}


void ImageView::scrub_mode()
{
    _mode = kScrub;
    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(true);
    redraw();
}

void ImageView::selection_mode()
{
    _mode = kSelection;
    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(true);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    redraw();
}

void ImageView::draw_mode()
{
    _mode = kDraw;
    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiDraw->value(true);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    redraw();
}

void ImageView::erase_mode()
{
    _mode = kErase;
    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiSelection->value(false);
    uiMain->uiPaint->uiErase->value(true);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiText->value(false);
    uiMain->uiPaint->uiScrub->value(false);
    redraw();
}

void ImageView::text_mode()
{
    bool ok = mrv::make_window();
    if ( ok )
    {
        _mode = kText;
        uiMain->uiPaint->uiText->value(true);
        uiMain->uiPaint->uiScrub->value(false);
    }
    else
    {
        _mode = kScrub;
        uiMain->uiPaint->uiText->value(false);
        uiMain->uiPaint->uiScrub->value(true);
    }

    uiMain->uiPaint->uiMovePic->value(false);
    uiMain->uiPaint->uiErase->value(false);
    uiMain->uiPaint->uiDraw->value(false);
    uiMain->uiPaint->uiSelection->value(false);
    redraw();
}

bool ImageView::in_presentation() const
{
    return presentation;
}

void ImageView::send_network( std::string m ) const
{
    if ( !_network_active) return;

    ParserList::const_iterator i = _clients.begin();
    ParserList::const_iterator e = _clients.end();

    if ( i != e )
    {
        (*i)->write( m, "" );  //<- this line writes all clients
    }
}


void ImageView::create_timeout( double t )
{
    add_timeout( float(t) );
}

void ImageView::delete_timeout()
{
    remove_timeout();
}

ImageView::ImageView(int X, int Y, int W, int H, const char *l) :
    fltk::GlWindow( X, Y, W, H, l ),
    _broadcast( false ),
    uiMain( NULL ),
    _engine( NULL ),
    _update( true ),
    _wait( false ),
    _normalize( false ),
    _safeAreas( false ),
    _masking( 0.0f ),
    _wipe_dir( kNoWipe ),
    _wipe( 1.0 ),
    _gamma( 1.0f ),
    _gain( 1.0f ),
    _zoom( 1 ),
    xoffset( 0 ),
    yoffset( 0 ),
    spinx( 0.0 ),
    spiny( 0.0 ),
    posX( 4 ),
    posY( 22 ),
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
    _flip( kFlipNone ),
    _reel( 0 ),
    _preframe( 1 ),
    _old_fg_frame( 0 ),
    _old_bg_frame( 0 ),
    _idle_callback( false ),
    _vr( kNoVR ),
    _timeout( NULL ),
    _old_fg( NULL ),
    _fg_reel( 0 ),
    _bg_reel( -1 ),
    _mode( kNoAction ),
    _selected_image( NULL ),
    _selection( mrv::Rectd(0,0) ),
    _playback( CMedia::kStopped ),
    _network_active( true ),
    _lastFrame( 0 )
{
    _timer.setDesiredSecondsPerFrame(0.05f);


    int stereo = 0;
    if ( can_do( fltk::STEREO ) ) stereo = 0; // should be fltk::STEREO

    mode( fltk::RGB24_COLOR | fltk::DOUBLE_BUFFER | fltk::ALPHA_BUFFER |
          fltk::STENCIL_BUFFER | stereo );

    create_timeout( 0.02f );
}


void ImageView::stop_playback()
{
    
    mrv::media fg = foreground();
    if ( fg ) fg->image()->stop();

    mrv::media bg = background();
    if ( bg ) bg->image()->stop(true);

}


ImageView::~ImageView()
{
    delete_timeout();
    if ( CMedia::preload_cache() )
        preload_cache_stop();

    ParserList::iterator i = _clients.begin();
    ParserList::iterator e = _clients.end();
    for ( ; i != e; ++i )
    {
        (*i)->connected = false;
    }

    _clients.clear();

    // make sure to stop any playback
    stop_playback();

    foreground( mrv::media() );
    background( mrv::media() );

    delete _engine;
    _engine = NULL;

    delete uiMain->uiSOPNode;
    uiMain->uiSOPNode = NULL;

    uiMain = NULL;
}

fltk::Window* ImageView::fltk_main()
{
    assert( uiMain );
    assert( uiMain->uiMain );
    if ( !uiMain ) return NULL;
    return uiMain->uiMain;
}

const fltk::Window* ImageView::fltk_main() const
{
    if ( !uiMain ) return NULL;
    assert( uiMain->uiMain );
    return uiMain->uiMain;
}


ImageBrowser*
ImageView::browser() {
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
    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return; // Audio only - no channels

    static int old_channel;

    unsigned short idx = 0;
    fltk::Group* g = NULL;
    for ( unsigned short i = 0; i < num; ++i, ++idx )
    {
        fltk::Widget* w = uiColorChannel->child(i);
        if ( w->is_group() )
        {
            g = (fltk::Group*) w;
            unsigned numc = g->children();
            idx += numc;
        }
    }

    if ( old_channel >= idx ) return; // too few channels

    unsigned int c = old_channel;
    old_channel = channel();
    channel( c );
}


bool ImageView::previous_channel()
{
    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return false; // Audio only - no channels

    int c = channel();

    bool is_group = false;
    short previous = -1;
    unsigned short total = 0;
    fltk::Widget* w;
    fltk::Group* g;

    // Count (total) number of channels
    for ( unsigned j = 0; j < num; ++j, ++total )
    {
        w = uiColorChannel->child(j);
        if ( w->is_group() )
        {
            g = (fltk::Group*) w;
            total += g->children();
        }
    }

    unsigned short idx = 0;
    for ( unsigned short i = 0; i < num; ++i, ++idx )
    {
        w = uiColorChannel->child(i);

        // This handles jump from first channel to last (Z or Color or
        // any channel done in loop later)
        if ( c == 0 && c == idx )
        {
            // Select last channel (group)
            fltk::Widget* last = uiColorChannel->child(num-1);
            // Jump to Z based on label
            if ( total > 8 &&
                    strcmp( last->label(), N_("Z") ) == 0 )
            {
                previous = total-1;
                is_group = true;
                break;
            }
            // Jump to color based on label
            else if ( total > 7 &&
                      strcmp( last->label(), _("Alpha Overlay") ) == 0 )
            {
                previous = total-7;
                is_group = true;
                break;
            }
            else if ( total > 5 &&
                      strcmp( last->label(), _("Lumma") ) == 0 )
            {
                previous = total-5;
                is_group = true;
                break;
            }
        }

        // This handles jump from Z to Color
        if ( c == idx && c >= 4 &&
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
        if ( w->is_group() )
        {
            g = (fltk::Group*) w;

            unsigned numc = g->children();
            if ( c == idx && previous >= 0) {
                is_group = true;
            }

            if ( !is_group ) previous = idx;

            idx += numc;
        }
        else if ( c == idx &&
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
            channel( previous );
        else
            channel( idx - 1 );
    }

    return true;
}

bool ImageView::next_channel()
{
    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    // check if a channel shortcut
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return false; // Audio only - no channels

    int c = channel();

    bool is_group = false;
    unsigned short next = 0;
    unsigned short idx = 0;
    fltk::Group* g = NULL;
    for ( unsigned short i = 0; i < num; ++i, ++idx )
    {
        fltk::Widget* w = uiColorChannel->child(i);
        if ( w->is_group() )
        {
            g = (fltk::Group*) w;
            unsigned numc = g->children();
            if ( c == idx ) {
                is_group = true;
                next = idx + numc + 1;
            }
            idx += numc;
            continue;
        }
        if ( c == idx && strcmp( w->label(), _("Color") ) == 0 )
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
    DBG( __FUNCTION__ << " " << __LINE__ );
    if ( !_engine )
    {
        mrvALERT( _("Could not initialize draw engine") );
        return;
    }
#ifdef LINUX
    fl_NET_WM_STATE       = XInternAtom(fltk::xdisplay, "_NET_WM_STATE",       0);
    fl_NET_WM_STATE_FULLSCREEN = XInternAtom(fltk::xdisplay, "_NET_WM_STATE_FULLSCREEN", 0);
#endif

    CMedia::supports_yuv( _engine->supports_yuv() );
    CMedia::supports_yuva( _engine->supports_yuva() );
}



void ImageView::fg_reel(int idx)
{
    _fg_reel = idx;

    char buf[128];
    sprintf( buf, "FGReel %d", idx );
    send_network( buf );
}

void ImageView::bg_reel(int idx)
{
    _bg_reel = idx;

    char buf[128];
    sprintf( buf, "BGReel %d", idx );
    send_network( buf );
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
    if ( !fg ) return;

    CMedia* img = fg->image();

    int x = lastX;
    int y = lastY;

    if ( x < 0 || y < 0 || x >= this->w() || y >= this->h() )
        return;

    mrv::image_type_ptr pic;
    bool outside = false;
    int xp, yp, w, h;
    picture_coordinates( img, x, y, outside, pic, xp, yp, w, h );

    if ( outside || !pic ) return;

    int xpr = int((double)xp / img->width());
    int ypr = pic->height() - int((double)yp / img->height()) - 1;

    CMedia::Pixel rgba = pic->pixel( xpr, ypr );
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
            ypr = pic->height() - yp - 1;
            rgba = pic->pixel( xp, ypr );
            pixel_processed( img, rgba );
            rgba.r = r;
        }
    }

    char buf[256];
    sprintf( buf, "%g %g %g %g", rgba.r, rgba.g, rgba.b, rgba.a );

    // Copy text to both the clipboard and to X's XA_PRIMARY
    fltk::copy( buf, unsigned( strlen(buf) ), true );
    fltk::copy( buf, unsigned( strlen(buf) ), false );
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
        double& x, double& y,
        const bool flipon ) const
{
    image_coordinates( img, x, y );

    const mrv::Recti& dpw = img->display_window();
    unsigned W = dpw.w();
    unsigned H = dpw.h();


    x -= W/2.0;
    y -= H/2.0;

    if ( flipon )
    {
        if ( _flip & kFlipVertical )
        {
            x = W - x;
        }
        if ( _flip & kFlipHorizontal )
        {
            y = H - y;
        }
    }


    {
        const mrv::Recti& daw = img->data_window();
        x -= daw.x();
        y -= daw.y();
    }

    rot2vec( x, y, img->rot_z() );

    //
    // If image is smaller than display window, we are dealing
    // with a RY or BY image.  We divide the window coordinates by 2.
    //
    mrv::image_type_ptr pic = img->hires();
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
    x /= _zoom;
    y /= _zoom;
    x += tw;
    y += th;
    x -= xoffset;
    y -= yoffset;




    // double tn = tan( ( 90 + img->rot_z() ) * (M_PI / 180) );

    // if ( std::abs( tn ) < 0.0001 )
    // {
    //     std::cerr << "ORIG " <<  x << ", " << y << std::endl;
    //     double px = x;
    //     x = y;
    //     y = px;

    //     px = W;
    //     W = H;
    //     H = px;
    //     std::cerr << "NEW  " <<  x << ", " << y << std::endl;
    // };
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

    mrv::image_type_ptr pic = img->hires();
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
        xoffset = -W/2.0 + 0.5;
    }
    else if ( stereo_out & CMedia::kStereoTopBottom )
    {
        yoffset = (( -H/2.0 ) / pr + 0.5 );
    }
    else
    {
        xoffset = -dpw.x() - W / 2.0;
    }

    zrotation_to_offsets( xoffset, yoffset, img->rot_z(), _flip, W, H );


    char buf[128];
    sprintf( buf, N_("Offset %g %g"), xoffset, yoffset );
    send_network( buf );
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

    // if ( display_window() && stereo_out & CMedia::kStereoSideBySide )
    //     W *= 2;

    double w = (double) fltk_main()->w();
    double z = w / (double)W;

    double h = (double) fltk_main()->h();
    if ( uiMain->uiTopBar->visible() )
        h -= uiMain->uiTopBar->h();
    if ( uiMain->uiPixelBar->visible() )
        h -= uiMain->uiPixelBar->h();
    if ( uiMain->uiBottomBar->visible() )
        h -= uiMain->uiBottomBar->h();

    h /= H;

    double pr = 1.0;
    if ( _showPixelRatio ) pr = pixel_ratio();
    h *= pr;

    if ( h < z ) {
        z = h;
    }

    xoffset = -dpw.x() - W / 2.0;


    if ( (_flip & kFlipVertical) && stereo_out & CMedia::kStereoSideBySide  )
        xoffset = 0.0;

    yoffset = ( dpw.y() + H / 2.0) / pr;
    if ( (_flip & kFlipHorizontal) &&
            stereo_out & CMedia::kStereoTopBottom  )
        yoffset = 0.0;

    zrotation_to_offsets( xoffset, yoffset, img->rot_z(), _flip, W, H );

    char buf[128];
    sprintf( buf, "Offset %g %g", xoffset, yoffset );
    send_network( buf );
    zoom( float(z) );

    mouseMove( fltk::event_x(), fltk::event_y() );

    redraw();
}


void
ImageView::zrotation_to_offsets( double& X, double& Y,
                                 const double degrees,
                                 const FlipDirection flip,
                                 const int W,
                                 const int H )
{
    double r = degrees * (M_PI / 180);  // in radians, please
    double sn = sin(r);
    double cs = cos(r);
    if ( is_equal( sn, -1.0, 0.001 ) )
    {
        // This cascading if/then is correct
        if ( flip & kFlipVertical )
        {
            X -= W + H;
        }
        if ( flip & kFlipHorizontal )
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
        if ( flip & kFlipVertical )
        {
            X -= W * 2;
        }
        if ( flip & kFlipHorizontal )
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
        if ( flip & kFlipVertical )
        {
            X -= H - W;
        }
        if ( flip & kFlipHorizontal )
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

    mrv::ViewerUI* m = main();
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

    mrv::ViewerUI* m = main();
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
    bool update = _idle_callback;


    CMedia* img = NULL;
    if ( fg )
    {
        img = fg->image();

        if ( img->image_damage() & CMedia::kDamageLayers )
        {
            update_layers();
        }

        if ( img->image_damage() & CMedia::kDamageContents )
        {
            redraw();
            update = true;
        }

        if ( img->image_damage() & CMedia::kDamageThumbnail )
        {
            // Redraw browser to update thumbnail
            mrv::ImageBrowser* b = browser();
            if (b) b->redraw();
            img->image_damage( img->image_damage() & ~CMedia::kDamageThumbnail );
        }

        if ( img->image_damage() & CMedia::kDamageData )
        {
            update_image_info();
            if ( uiMain && uiMain->uiICCProfiles )
                uiMain->uiICCProfiles->fill();
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
            img->image_damage( img->image_damage() & ~CMedia::kDamageTimecode );
        }

        if ( img->image_damage() & CMedia::kDamageCache )
        {
            uiMain->uiTimeline->redraw();
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
            redraw();
            update = true;
        }

        if ( bimg->image_damage() & CMedia::kDamageThumbnail )
        {
            // Redraw browser to update thumbnail
            mrv::ImageBrowser* b = browser();
            if (b) b->redraw();
            bimg->image_damage( bimg->image_damage() & ~CMedia::kDamageThumbnail );
        }
    }

    if ( update && _playback != CMedia::kStopped ) {
#ifdef FLTK_TIMEOUT_EVENT_BUG
        int y = fltk::event_y();
#  ifdef LINUX
        if ( uiMain->uiTopBar->visible() ) y -= uiMain->uiTopBar->h();
#  else
        if ( uiMain->uiTopBar->visible() ) y -= uiMain->uiTopBar->h() *
                                           (!uiMain->uiMain->border());
#  endif
        mouseMove( fltk::event_x(), y );
#else
        mouseMove( fltk::event_x(), fltk::event_y() );
#endif
    }


    return update;
}

struct ThreadData
{
    mrv::ImageView* v;
    int64_t frame;
};

void static_preload( ThreadData* d )
{
    int64_t f = d->frame;
    mrv::ImageView* view = d->v;
    delete d;

    view->preload( f );
}



void thread_dispatcher( mrv::ImageView* v )
{
    mrv::media fg = v->foreground();
    if (!fg) return;

    if ( v->playback() != CMedia::kStopped )
        v->play( v->playback() );

    CMedia* img = fg->image();

    for ( int i = 0; i < 10; ++i )
    {
        if ( !v->idle_callback() || img->is_cache_full() ||
             v->playback() == CMedia::kStopped ) break;

        ThreadData* data = new ThreadData;
        data->v = v;
        data->frame = img->frame();
        data->frame++;

        boost::thread t( boost::bind( static_preload, data ) );
        t.detach();
    }

}



void ImageView::log() const
{
    //
    // First, handle log window showing up and scrolling
    //
    if (main() && main()->uiLog && main()->uiLog->uiMain )
    {
        mrv::LogUI* logUI = main()->uiLog;
        fltk::Window* logwindow = logUI->uiMain;
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
            static int lines = 0;
            if ( log->visible() && log->lines() != lines )
            {
                log->scroll( log->lines()-1, 0 );
                lines = log->lines();
            }
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

Timer t;

bool ImageView::ready_frame( std::atomic<int64_t>& f,
                             CMedia::Playback p,
                             CMedia* const img,
                             const int64_t& first,
                             const int64_t& last,
                             const bool seek )
{
    abort();
    if ( p == CMedia::kForwards )
    {
        f += p;
        if ( f > last )
        {
            switch( looping() )
            {
                case CMedia::kPingPong:
                    f = last;
                    playback( CMedia::kBackwards );
                    img->playback( CMedia::kBackwards );
                    break;
                case CMedia::kLoop:
                    f = first;
                    break;
            }
            if ( seek )
            {
                img->seek( f );
                img->do_seek();
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
                    playback( CMedia::kBackwards );
                    img->playback( CMedia::kBackwards );
                    break;
                case CMedia::kLoop:
                    f = last;
                    break;
            }
            if ( seek )
            {
                img->seek( f );
                img->do_seek();
            }
        }
        return true;
    }
    return false;
}

bool ImageView::preload( int64_t preframe )
{
    if ( !browser() || !timeline() ) return false;


    mrv::ImageBrowser* b = browser();

    mrv::Reel r = b->reel_at( _reel );
    if (!r) return false;

    _preframe = preframe;

    mrv::media fg;
    if ( r->edl )
    {
        fg = r->media_at( _preframe );
    }
    else
    {
        fg = foreground();
        if ( !fg )  return false;
    }



    CMedia* img = NULL;
    if ( fg ) img = fg->image();

    CMedia::Playback p = playback();


    // Exit early if we are dealing with a video instead of a sequence
    if ( !img || !img->is_sequence() || img->has_video() ) {
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
        f = img->frame();

        first = img->first_frame();
        last  = img->last_frame();
        // int64_t tfirst = timeline()->display_minimum();
        // int64_t tlast  = timeline()->display_maximum();
        // if ( tfirst > first && tfirst < last ) first = tfirst;
        // if ( tlast < last   && tlast > first )  last = tlast;

        if ( f < first ) f = first;
        else if ( f > last ) f = last;
    }

    bool found;
    {
        typedef CMedia::Mutex Mutex;
        mrv::PacketQueue& vp = img->video_packets();
        CMedia::Mutex& vpm = vp.mutex();
        SCOPED_LOCK( vpm );
        mrv::PacketQueue& ap = img->audio_packets();
        CMedia::Mutex& apm = ap.mutex();
        SCOPED_LOCK( apm );
        mrv::PacketQueue& sp = img->subtitle_packets();
        CMedia::Mutex& spm = sp.mutex();
        SCOPED_LOCK( spm );
        Mutex& mtx = img->video_mutex();
        SCOPED_LOCK( mtx );
        img->frame( f );
        found = img->find_image( f ); // this loads the frame if not present
    }

    if ( !found ) return false;

    std::atomic< int64_t > pf( f );
    ready_frame( pf, p, img, first, last, true );

    img->frame( pf );
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

    _network_active = false;
    Command c = commands.front();
    switch( c.type )
    {
    case kCreateReel:
    {
        std::string s = *(std::string*)c.data;
        mrv::Reel r = b->reel( s.c_str() );
        if ( !r )
        {
            b->new_reel( s.c_str() );
        }
        break;
    }
    case kLoadImage:
    {
        LoadInfo file = * (LoadInfo*) c.data;
        LoadList files;
        files.push_back( file );
        b->load( files, false, "", false, false );
        break;
    }
    case kCacheClear:
        clear_caches();
        break;
    case kChangeImage:
    {
        int* idx = (int*) c.data;
        b->change_image(*idx);
        break;
    }
    case kBGImage:
    {
        int idx = *( (int*) c.data );
        if ( idx < 0 ) background( mrv::media() );
        else
        {
            mrv::Reel r = b->current_reel();
            if ( idx < r->images.size() )
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
        int idx = *( (int*) c.data );
        fg_reel( idx );
        break;
    }
    case kBGReel:
    {
        int idx = *( (int*) c.data );
        bg_reel( idx );
        break;
    }
    case kStopVideo:
    {
        stop();
        break;
    }
    case kSeek:
    {
        int64_t f = * ((int64_t*) c.data);
        seek( f );
        break;
    }
    case kPlayForwards:
    {
        play_forwards();
        break;
    }
    case kPlayBackwards:
    {
        play_backwards();
        break;
    }
    case kRemoveImage:
    {
        int* idx = (int*) c.data;
        b->remove(*idx);
        break;
    }
    case kExchangeImage:
    {
        std::vector<int>* list = (std::vector<int>*) c.data;
        int oldsel = (*list)[0];
        int sel = (*list)[1];
        b->exchange(oldsel, sel);
        break;
    }
    case kICS:
    {
        std::string* s = (std::string*) c.data;
        mrv::media fg = foreground();
        if (fg)
        {
            CMedia* img = fg->image();
            img->ocio_input_color_space( *s );
            update_ICS();
        }
        break;
    }
    case kRT:
    {
        std::string* s = (std::string*) c.data;
        mrv::media fg = foreground();
        if (fg)
        {
            CMedia* img = fg->image();
            img->rendering_transform( s->c_str() );
            img->image_damage( img->image_damage() |
                               CMedia::kDamageLut );
        }
        break;
    }
    case kChangeChannel:
    {
        unsigned idx = * (unsigned*) c.data;
        if ( foreground() )
        {
            channel( idx );
        }
        else
            LOG_ERROR( "No image for channel selection" );
        break;
    }
    case kFULLSCREEN:
        toggle_fullscreen();
        break;
    case kPRESENTATION:
        toggle_presentation();
        break;
    case kMEDIA_INFO_WINDOW_SHOW:
        toggle_media_info(true);
        break;
    case kMEDIA_INFO_WINDOW_HIDE:
        toggle_media_info(false);
        break;
    case kCOLOR_AREA_WINDOW_SHOW:
        toggle_color_area(true);
        break;
    case kCOLOR_AREA_WINDOW_HIDE:
        toggle_color_area(false);
        break;
    case k3D_VIEW_WINDOW_SHOW:
        toggle_3d_view(true);
        break;
    case k3D_VIEW_WINDOW_HIDE:
        toggle_3d_view(false);
        break;
    case kHISTOGRAM_WINDOW_SHOW:
        toggle_histogram(true);
        break;
    case kHISTOGRAM_WINDOW_HIDE:
        toggle_histogram(false);
        break;
    case kVECTORSCOPE_WINDOW_SHOW:
        toggle_vectorscope(true);
        break;
    case kVECTORSCOPE_WINDOW_HIDE:
        toggle_vectorscope(false);
        break;
    case kWAVEFORM_WINDOW_SHOW:
        toggle_waveform(true);
        break;
    case kWAVEFORM_WINDOW_HIDE:
        toggle_waveform(false);
        break;
    case kSTEREO_OPTIONS_WINDOW_SHOW:
        toggle_stereo_options(true);
        break;
    case kSTEREO_OPTIONS_WINDOW_HIDE:
        toggle_stereo_options(false);
        break;
    case kPAINT_TOOLS_WINDOW_SHOW:
        toggle_paint_tools(true);
        break;
    case kPAINT_TOOLS_WINDOW_HIDE:
        toggle_paint_tools(false);
        break;
    case kLUT_CHANGE:
    {
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
    default:
    {
        LOG_ERROR( "Unknown mrv event " << commands.size() << " "
                   << c.type << " data " << c.data );
        break;
    }
    }  // switch

    _network_active = true;
    delete c.data;
    commands.pop_front();
    redraw();
}

void ImageView::timeout()
{
    mrv::Timeline* timeline = this->timeline();
    if  (!timeline) return;

    // Redraw browser to update thumbnail
    mrv::ImageBrowser* b = browser();
    if (!b) return;

    {
        SCOPED_LOCK( commands_mutex );
        while ( ! commands.empty()  )
        {
            handle_commands();
        }
    }

    TRACE( "" );
    //
    // If in EDL mode, we check timeline to see if frame points to
    // new image.
    //
    log();


    mrv::Reel reel = b->reel_at( _fg_reel );
    mrv::Reel bgreel = b->reel_at( _bg_reel );

    mrv::media fg = foreground();

    int64_t tframe = int64_t( timeline->value() );

    if ( reel && reel->edl )
    {
        TRACE("");
        fg = reel->media_at( tframe );

        if ( fg && fg != foreground() )
        {
            TRACE("");
            DBG( ">>>>>>>>>>>>>> CHANGE TO FG " << fg->image()->name() << " due to frame "
                 << tframe );
            foreground( fg );

            fit_image();
        }

    }

    mrv::media bg = background();

    if ( bgreel && bgreel->edl )
    {
        bg = bgreel->media_at( tframe );

        if ( bg && bg != background() )
        {
            // CMedia* img = bg->image();
            // if ( img->playback() == playback() )
            background( bg );
        }
    }

    bg = background();

    if ( bg && bg != fg )
    {
        TRACE("");
        CMedia* img = bg->image();
        // If not a video image check if image has changed on disk
        if ( ! img->has_video() &&
                uiMain->uiPrefs->uiPrefsAutoLoadImages->value() )
        {
            img->has_changed();
        }

    }


    double delay = 0.005;
    if ( fg )
    {
        TRACE("");
        CMedia* img = fg->image();
        delay = 0.25 / img->play_fps();

        // If not a video image check if image has changed on disk

        if ( ! img->has_video() &&
                uiMain->uiPrefs->uiPrefsAutoLoadImages->value() )
        {
            TRACE("");
            img->has_changed();
        }

    }

    if ( timeline && timeline->visible() )
    {

        TRACE("");
        if ( reel && !reel->edl && fg )
        {
            TRACE("");
            CMedia* img = fg->image();
            int64_t frame = img->frame();

            if ( this->frame() != frame )
            {
                TRACE("");
                this->frame( frame );
            }
        }
    }

    if ( fg && should_update( fg ) )
    {
        TRACE("");
        update_color_info( fg );
        TRACE("");
        if ( uiMain->uiEDLWindow )
            uiMain->uiEDLWindow->uiEDLGroup->redraw();
    }

    if ( vr() )
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


    repeat_timeout( delay );
}

void ImageView::selection( const mrv::Rectd& r )
{
    _selection = r;

    mrv::media fg = foreground();
    if ( fg )
        update_color_info( fg );
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
        undo_shapes.pop_back();

        send_network( "RedoDraw" );
        redraw();
    }
}

void ImageView::undo_draw()
{
    mrv::media fg = foreground();
    if (!fg) return;

    GLShapeList& shapes = fg->image()->shapes();
    GLShapeList& undo_shapes = fg->image()->undo_shapes();

    if ( ! shapes.empty() )
    {
        undo_shapes.push_back( shapes.back() );
        uiMain->uiPaint->uiRedoDraw->activate();
        shapes.pop_back();
        send_network( "UndoDraw" );
        redraw();
    }

}

void ImageView::draw_text( unsigned char r, unsigned char g, unsigned char b,
                           double x, double y, const char* t )
{
    char text[256];
    utf8toa( t, (unsigned) strlen(t), text, 255 );
    _engine->color( (uchar)0, (uchar)0, (uchar)0 );
    _engine->draw_text( int(x+1), int(y-1), text ); // draw shadow
    _engine->color( r, g, b );
    _engine->draw_text( int(x), int(y), text );  // draw text
}


void ImageView::vr( VRType t )
{
    _vr = t;
    valid(0);

    const mrv::media& fg = foreground();

    if ( fg )
    {
        CMedia* img = fg->image();
        img->image_damage( img->image_damage() | CMedia::kDamageContents );
    }

    const mrv::media& bg = foreground();

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

    DBG( "draw valid? " << (int)valid() );
    if ( !valid() )
    {
        if ( ! _engine )
        {
            DBG( __FUNCTION__ << " " << __LINE__ );
            init_draw_engine();
        }

        DBG( "GLengine " << _engine );
        if ( !_engine ) return;


        DBG( __FUNCTION__ << " " << __LINE__ );
        _engine->reset_view_matrix();


        valid(1);
    }



    mrv::PreferencesUI* uiPrefs = uiMain->uiPrefs;

    //
    // Clear canvas
    //
    {
        float r, g, b, a = 0.0f;
        if ( !presentation && !FullScreen )
        {
            uchar ur, ug, ub;
            fltk::split_color( uiPrefs->uiPrefsViewBG->color(), ur, ug, ub );
            r = ur / 255.0f;
            g = ur / 255.0f;
            b = ur / 255.0f;
        }
        else
        {
            r = g = b = a = 0.0f;
        }

        DBG( __FUNCTION__ << " " << __LINE__ );

        _engine->clear_canvas( r, g, b, a );

        if ( !_update ) return;

        DBG( __FUNCTION__ << " " << __LINE__ );
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
    if ( fg )
    {
        _engine->image( fg->image() );
    }
    mrv::media bg = background();
    TRACE("");

    ImageList images;
    images.reserve(2);

    if ( _showBG && bg && bg != fg  )
    {
        TRACE("");
        CMedia* img = bg->image();
        TRACE("");
        if ( img->has_picture() )
            images.push_back( img );
        TRACE("");
    }


    if ( fg )
    {
        TRACE("");
        CMedia* img = fg->image();
        if ( img->has_picture() )
        {
            images.push_back( img );
        }
        TRACE("");
    }


    DBG( __FUNCTION__ << " " << __LINE__ );
    if ( images.empty() ) return;
    TRACE("");


    DBG( __FUNCTION__ << " " << __LINE__ );
    _engine->draw_images( images );
    TRACE("");


    if ( _masking != 0.0f )
    {
        _engine->draw_mask( _masking );
    }

    CMedia* img = NULL;
    if ( fg )
    {
        img = fg->image();
        const char* label = img->label();
        if ( label )
        {
            uchar r, g, b;
            fltk::split_color( uiPrefs->uiPrefsViewTextOverlay->color(), r, g, b );


            int dx, dy;
            dx = int( (double) w() / (double)2 - (unsigned)strlen(label)*3 );
            dy = 24;

            draw_text( r, g, b, dx, dy, label );
        }
    }

    if ( _selection.w() > 0 || _selection.h() > 0 )
    {
        uchar r, g, b;
        fltk::split_color( uiPrefs->uiPrefsViewSelection->color(), r, g, b );
        _engine->color( r, g, b, 255 );
        _engine->draw_rectangle( _selection, flip(), img->rot_z() );
    }



    if ( fg && _safeAreas )
    {
        const CMedia* img = fg->image();
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

        double aspect = (double) W / (double) H;

        // Safe areas may change when pixel ratio is active
        double pr = 1.0;
        if ( uiMain->uiPixelRatio->value() )
        {
            pr = img->pixel_ratio();
            aspect *= pr;
        }

        if ( aspect < 1.66 || (aspect >= 1.77 && aspect <= 1.78) )
        {
            // Assume NTSC/PAL
            float f = float(H) * 1.33f;
            f = f / float(W);
            _engine->color( 1.0f, 0.0f, 0.0f );
            _engine->draw_safe_area( f * 0.9f, 0.9f, _("tv action") );
            _engine->draw_safe_area( f * 0.8f, 0.8f, _("tv title") );

            if ( aspect >= 1.77 )
            {
                // Draw hdtv too
                _engine->color( 1.0f, 0.0f, 1.0f );
                _engine->draw_safe_area( 1.0f, aspect/1.77f, _("hdtv") );
            }
        }
        else
        {
            if ( pr == 1.0f )
            {
                // Assume film, draw 2.35, 1.85 and 1.66 areas
                _engine->color( 0.0f, 1.0f, 0.0f );
                _engine->draw_safe_area( 1.0f, aspect/2.35f, "2.35" );
                _engine->draw_safe_area( 1.0f, aspect/1.85f, "1.85" );
                _engine->draw_safe_area( 1.0f, aspect/1.66f, "1.66" );

                // Draw hdtv too
                _engine->color( 1.0f, 0.0f, 1.0f );
                _engine->draw_safe_area( 1.0f, aspect/1.77f, _("hdtv") );
            }
            else
            {
                // Film fit for TV, Draw 4-3 safe areas
                float f = float(H) * 1.33f;
                f = f / float(W);
                _engine->color( 1.0f, 0.0f, 0.0f );
                _engine->draw_safe_area( f * 0.9f, 0.9f, _("tv action") );
                _engine->draw_safe_area( f * 0.8f, 0.8f, _("tv title") );
            }
        }

    }

    if ( !fg ) return;

    _engine->draw_annotation( img->shapes() );

    if ( !(flags & kMouseDown) && ( _mode == kDraw || _mode == kErase ) )
    {
        double xf = X;
        double yf = Y;

        data_window_coordinates( img, xf, yf );

        const mrv::Recti& dpw = img->display_window();

        unsigned int H = dpw.h();
        if ( H == 0 ) H = img->height();

        yf = -yf;

        const mrv::Recti& daw = img->data_window();
        xf += daw.x();
        yf -= daw.y();

        _engine->draw_cursor( xf, yf, _mode );
    }

    TRACE("");

    if ( _hud == kHudNone )
        return;


    std::ostringstream hud;
    hud.str().reserve( 512 );

    uchar r, g, b;
    fltk::split_color( uiPrefs->uiPrefsViewHud->color(), r, g, b );
    _engine->color( r, g, b );


    //
    // Draw HUD
    //

    if ( vr() )
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        ortho();

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }

    int y = h()-25;
    int yi = 25;
    static char buf[1024];

    if ( _hud & kHudDirectory )
    {
        hud << img->directory();
        draw_text( r, g, b, 5, y, hud.str().c_str() );
        y -= yi;
        hud.str("");
    }

    if ( _hud & kHudFilename )
    {
        sprintf( buf, img->name().c_str(), img->frame() );
        hud << buf;
    }

    if ( img->first_frame() != img->last_frame() && _hud & kHudFrameRange )
    {
        if ( !hud.str().empty() ) hud << " ";
        hud << img->first_frame() << " - " << img->last_frame();
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
            d = img->data_window( img->frame()-1 );
        }
        sprintf( buf, _("DAW: %d,%d %dx%d"), d.x(), d.y(), d.w(), d.h() );
        draw_text( r, g, b, 5, y, buf );
        y -= yi;
        {
            const mrv::Recti& d = img->display_window();
            sprintf( buf, _("DYW: %d,%d %dx%d"), d.x(), d.y(), d.w(), d.h() );
            draw_text( r, g, b, 5, y, buf );
            y -= yi;
        }
    }

    if ( _hud & kHudFrame )
    {
        sprintf( buf, "% 4" PRId64, img->frame() );
        hud << _("F: ") << buf;
    }

    if ( _hud & kHudTimecode )
    {
        mrv::Timecode::Display d = uiMain->uiFrame->display();
        if ( d != mrv::Timecode::kTimecodeDropFrame )
            d = mrv::Timecode::kTimecodeNonDrop;

        mrv::Timecode::format( buf, d, img->frame(), img->timecode(),
                               img->play_fps(), true );
        if ( !hud.str().empty() ) hud << " ";
        hud << _("T: ") << buf;
    }

    if ( (_hud & kHudAVDifference) && img->has_audio() )
    {
        double avdiff = img->avdiff();
        if ( !hud.str().empty() ) hud << " ";
        sprintf( buf, "% 4f", avdiff );
        hud << _("V-A: ") << buf;
    }


    //
    // Calculate and draw fps
    //
    if ( _hud & kHudFPS )
    {
        static int64_t unshown_frames = 0;
        int64_t frame = img->frame();

        CMedia::Playback p = playback();

        if ((p == CMedia::kForwards && _lastFrame < frame) ||
                (p == CMedia::kBackwards && _lastFrame > frame ) )
        {
            int64_t frame_diff = frame - _lastFrame;

            if ( p == CMedia::kForwards ) --frame_diff;
            else ++frame_diff;

            int64_t absdiff = std::abs(frame_diff);
            if ( absdiff > 0 && absdiff < 10 )
            {
                unshown_frames += absdiff;
            }

            _lastFrame = frame;
        }


        {
            sprintf( buf, _(" UF: %" PRId64 " "), unshown_frames );
            hud << buf;
            _timer.setDesiredSecondsPerFrame( 1.0 / img->play_fps() );
            _timer.waitUntilNextFrameIsDue();
            sprintf( buf, _("FPS: %.3f" ), _timer.actualFrameRate() );
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
            hud << "Wipe V";
        if ( _wipe_dir == kWipeHorizontal )
            hud << "Wipe H";
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

    TRACE("");

    if ( vr() )
    {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}


GLShapeList& ImageView::shapes()
{
    mrv::media fg = foreground();
    return fg->image()->shapes();
}

void ImageView::add_shape( mrv::shape_type_ptr s )
{
    mrv::media fg = foreground();
    if (!fg) {
        LOG_ERROR( "No image to add shape to" );
        return;
    }

    fg->image()->add_shape( s );
    uiMain->uiPaint->uiUndoDraw->activate();
    uiMain->uiPaint->uiRedoDraw->deactivate();
}

int sign (const Imath::V2i& p1,
          const Imath::V2i& p2,
          const Imath::V2i& p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

// bool PointInTriangle (const Imath::V2d& p,
//                       const Imath::V2d& p0,
//                       const Imath::V2d& p1,
//                       const Imath::V2d& p2)
// {
//     double dX = p.x-p2.x;
//     double dY = p.y-p2.y;
//     double dX21 = p2.x-p1.x;
//     double dY12 = p1.y-p2.y;
//     double D = dY12*(p0.x-p2.x) + dX21*(p0.y-p2.y);
//     double s = dY12*dX + dX21*dY;
//     double t = (p2.y-p0.y)*dX + (p0.x-p2.x)*dY;
//     if (D<0) return s<=0 && t<=0 && s+t>=D;
//     return s>=0 && t>=0 && s+t<=D;
// }
// bool PointInTriangle (const Imath::V2d& s,
//                       const Imath::V2d& a,
//                       const Imath::V2d& b,
//                       const Imath::V2d& c)
// {
//     int as_x = s.x-a.x;
//     int as_y = s.y-a.y;

//     bool s_ab = (b.x-a.x)*as_y-(b.y-a.y)*as_x > 0;

//     if((c.x-a.x)*as_y-(c.y-a.y)*as_x > 0 == s_ab) return false;

//     if((c.x-b.x)*(s.y-b.y)-(c.y-b.y)*(s.x-b.x) > 0 != s_ab) return false;

//     return true;
// }
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

/**
 * Handle a mouse press
 *
 * @param x fltk::event_x() coordinate
 * @param y fltk::event_y() coordinate
 */
int ImageView::leftMouseDown(int x, int y)
{
    lastX = x;
    lastY = y;



    //flags	= kMouseDown;
    flags	= 0;

    int button = fltk::event_button();
    if (button == 1)
    {
        flags  = kMouseDown;
        if (fltk::event_key_state(fltk::LeftAltKey) || vr() )
        {
            // Handle ALT+LMB moves
            flags |= kMouseMove;
            flags |= kMouseMiddle;
            return 1;
        }

        flags |= kMouseLeft;
        if ( fltk::event_key_state( fltk::LeftShiftKey ) ||
             fltk::event_key_state( fltk::RightShiftKey ) )
        {
            flags |= kLeftShift;
            // selection_mode();
        }
        else if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
                  fltk::event_key_state( fltk::RightCtrlKey ) )
        {
            flags |= kMouseLeft;
            flags |= kLeftCtrl;
            flags |= kGain;
        }

        if ( _mode == kSelection )
        {
            _selection = mrv::Rectd( 0, 0, 0, 0 );
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
                TRACE("");
                CMedia* img = fg->image();
                if ( img->has_picture() )
                {
                    images.push_back( img );
                }
                TRACE("");
            }

            if ( _showBG && bg && bg != fg )
            {
                TRACE("");
                CMedia* img = bg->image();
                TRACE("");
                if ( img->has_picture() )
                    images.push_back( img );
                TRACE("");
            }

            ImageList::const_iterator i = images.begin();
            ImageList::const_iterator e = images.end();


            _selected_image = NULL;

            for ( ; i != e; ++i )
            {
                CMedia* img = *i;

                mrv::image_type_ptr pic;
                bool outside = false;
                int xp, yp, w, h;
                picture_coordinates( img, x, y, outside, pic, xp, yp, w, h );

                // std::cerr << "x,y " << x << ", " << y << " outside " << outside
                //           << std::endl;
                CMedia::Pixel p;
                if ( xp >= 0 && ( xp < pic->width()*img->scale_x() ) &&
                        yp >= 0 && ( yp < pic->height()*img->scale_y() ) )
                {
                    Imath::V2i pt( xp, yp );
                    int W = pic->width() * img->scale_x();
                    int H = pic->height() * img->scale_y();
                    const int kSize = 30;
                    Imath::V2i v1( W, H );
                    Imath::V2i v2( W-kSize, H );
                    Imath::V2i v3( W, H-kSize );
                    if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
                            fltk::event_key_state( fltk::RightCtrlKey ) ||
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
        else if ( _mode == kDraw || _mode == kErase || _mode == kText )
        {

            _selection = mrv::Rectd( 0, 0, 0, 0 );

            mrv::media fg = foreground();
            if ( !fg ) return 0;

            CMedia* img = fg->image();
            if (!img) return 0;

            double xf = x;
            double yf = y;

            data_window_coordinates( img, xf, yf );

            const mrv::Recti& dpw = img->display_window();

            yf = -yf;

            const mrv::Recti& daw = img->data_window();
            xf += daw.x();
            yf -= daw.y();

            std::string str;
            GLPathShape* s;
            if ( _mode == kDraw )
            {
                s = new GLPathShape;
            }
            else if ( _mode == kErase )
            {
                s = new GLErasePathShape;
            }
            else if ( _mode == kText )
            {
                if ( mrv::font_text != "" )
                {
                    GLTextShape* t = new GLTextShape;

                    t->font( mrv::font_current );
                    t->size( mrv::font_size );
                    t->text( mrv::font_text );

                    mrv::font_text = "";
                    s = t;
                }
                else
                {
                    return 1;
                }

            }



            uchar r, g, b;
            fltk::split_color( uiMain->uiPaint->uiPenColor->color(), r, g, b );

            s->r = r / 255.0f;
            s->g = g / 255.0f;
            s->b = b / 255.0f;
            s->a = 1.0f;
            s->pen_size = (float) uiMain->uiPaint->uiPenSize->value();
            if ( _mode == kErase ) s->pen_size *= 2;

            if ( uiMain->uiPaint->uiAllFrames->value() )
            {
                s->frame = MRV_NOPTS_VALUE;
            }
            else
            {
                s->frame = frame();
            }

            mrv::Point p( xf, yf );
            s->pts.push_back( p );

            send_network( str );

            add_shape( mrv::shape_type_ptr(s) );
        }

        if ( _wipe_dir != kNoWipe )
        {
            _wipe_dir = (WipeDirection) (_wipe_dir | kWipeFrozen);
            window()->cursor(fltk::CURSOR_CROSS);
            fltk::check();
        }



        redraw();
        return 1;
    }

    else if ( button == 2 )
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



            fltk::Menu menu(0,0,0,0);
            menu.add( _("File/Open/Movie or Sequence"), kOpenImage.hotkey(),
                      (fltk::Callback*)open_cb, browser() );
            menu.add( _("File/Open/Single Image"), kOpenSingleImage.hotkey(),
                      (fltk::Callback*)open_single_cb, browser() );
            mrv::media fg = foreground();
            if ( !fg )
            {
                menu.add( _("File/Open/Directory"), kOpenDirectory.hotkey(),
                          (fltk::Callback*)open_dir_cb, browser() );
            }
            if ( fg )
            {
                menu.add( _("File/Open/Stereo Sequence or Movie"),
                          kOpenStereoImage.hotkey(),
                          (fltk::Callback*)open_stereo_cb, browser() );
                menu.add( _("File/Open/Clip XML Metadata"),
                          kOpenClipXMLMetadata.hotkey(),
                          (fltk::Callback*)open_clip_xml_metadata_cb, this );
                menu.add( _("File/Open/Directory"), kOpenDirectory.hotkey(),
                          (fltk::Callback*)open_dir_cb, browser() );
                menu.add( _("File/Save/Movie or Sequence As"),
                          kSaveSequence.hotkey(),
                          (fltk::Callback*)save_sequence_cb, this );
                menu.add( _("File/Save/Reel As"), kSaveReel.hotkey(),
                          (fltk::Callback*)save_reel_cb, this );
                menu.add( _("File/Save/Frame As"), kSaveImage.hotkey(),
                          (fltk::Callback*)save_cb, this );
                menu.add( _("File/Save/GL Snapshots As"), kSaveSnapshot.hotkey(),
                          (fltk::Callback*)save_snap_cb, this );
                menu.add( _("File/Save/Clip XML Metadata As"),
                          kSaveClipXMLMetadata.hotkey(),
                          (fltk::Callback*)save_clip_xml_metadata_cb, this );
            }

            char buf[256];
            const char* tmp;
            fltk::Widget* item;
            int num = uiMain->uiWindows->children();
            int i;
            for ( i = 0; i < num; ++i )
            {
                tmp = uiMain->uiWindows->child(i)->label();
                sprintf( buf, _("Windows/%s"), tmp );
                menu.add( buf, 0, (fltk::Callback*)window_cb, uiMain );
            }

            if ( fg && fg->image()->has_picture() )
            {

                menu.add( _("View/Safe Areas"), kSafeAreas.hotkey(),
                          (fltk::Callback*)safe_areas_cb, this );

                menu.add( _("View/Display Window"), kDisplayWindow.hotkey(),
                          (fltk::Callback*)display_window_cb, this );

                menu.add( _("View/Data Window"), kDataWindow.hotkey(),
                          (fltk::Callback*)data_window_cb, this );

                num = uiMain->uiPrefs->uiPrefsCropArea->children();
                for ( i = 0; i < num; ++i )
                {
                    tmp = uiMain->uiPrefs->uiPrefsCropArea->child(i)->label();
                    sprintf( buf, _("View/Mask/%s"), tmp );
                    item = menu.add( buf, 0, (fltk::Callback*)masking_cb, uiMain );
                    item->type( fltk::Item::TOGGLE );
                    float mask = -1.0f;
                    mask = (float) atof( tmp );
                    if ( mask == _masking ) item->set();
                }

                sprintf( buf, _("View/Hud/Toggle Selected") );
                item = menu.add( buf, kHudToggle.hotkey(),
                                 (fltk::Callback*)hud_toggle_cb, uiMain );

                num = uiMain->uiPrefs->uiPrefsHud->children();
                for ( i = 0; i < num; ++i )
                {
                    tmp = uiMain->uiPrefs->uiPrefsHud->child(i)->label();
                    sprintf( buf, _("View/Hud/%s"), tmp );
                    item = menu.add( buf, 0, (fltk::Callback*)hud_cb, uiMain );
                    item->type( fltk::Item::TOGGLE );
                    if ( hud() & (1 << i) ) item->set();
                }

                bool has_version = false;

                CMedia* img = fg->image();
                std::string file = img->fileroot();
                file = fs::path( file ).leaf().string();
                std::string dir = img->directory();
                file = dir + "/" + file;
                size_t pos = 0;
                mrv::PreferencesUI* prefs = main()->uiPrefs;
                std::string prefix = prefs->uiPrefsImageVersionPrefix->value();
                if ( (pos = file.find( prefix, pos) ) != std::string::npos )
                    has_version = true;

                if ( has_version )
                {
                    menu.add( _("Image/Next Version"), kNextVersionImage.hotkey(),
                              (fltk::Callback*)next_image_version_cb, browser());
                    menu.add( _("Image/Previous Version"),
                              kPreviousVersionImage.hotkey(),
                              (fltk::Callback*)previous_image_version_cb,
                              browser(), fltk::MENU_DIVIDER);
                }

                menu.add( _("Image/Next"), kNextImage.hotkey(),
                          (fltk::Callback*)next_image_cb, browser());
                menu.add( _("Image/Previous"), kPreviousImage.hotkey(),
                          (fltk::Callback*)previous_image_cb,
                          browser(), fltk::MENU_DIVIDER);


                const stubImage* simg = dynamic_cast< const stubImage* >( image() );
                if ( simg )
                {
                    menu.add( _("Image/Clone"), kCloneImage.hotkey(),
                              (fltk::Callback*)clone_image_cb, browser());
                    menu.add( _("Image/Clone All Channels"), 0,
                              (fltk::Callback*)clone_all_cb,
                              browser(), fltk::MENU_DIVIDER);
                }
                else
                {
                    menu.add( _("Image/Clone"), kCloneImage.hotkey(),
                              (fltk::Callback*)clone_image_cb, browser(),
                              fltk::MENU_DIVIDER );
                }

                item = menu.add( _("Image/Preload Caches"), kPreloadCache.hotkey(),
                                 (fltk::Callback*)preload_image_cache_cb, this );
                item->type( fltk::Item::TOGGLE );
                if ( CMedia::preload_cache() ) item->set();

                menu.add( _("Image/Clear Caches"), kClearCache.hotkey(),
                          (fltk::Callback*)clear_image_cache_cb, this );

                menu.add( _("Image/Update Single Frame in Cache"),
                          kClearSingleFrameCache.hotkey(),
                          (fltk::Callback*)update_frame_cb, this,
                          fltk::MENU_DIVIDER );

                menu.add( _("Image/Rotate +90"),
                          kRotatePlus90.hotkey(),
                          (fltk::Callback*)rotate_plus_90_cb, this );
                menu.add( _("Image/Rotate -90"),
                          kRotateMinus90.hotkey(),
                          (fltk::Callback*)rotate_minus_90_cb, this,
                          fltk::MENU_DIVIDER );

                if ( !Preferences::use_ocio )
                {
                    menu.add( _("Image/Attach CTL Input Device Transform"),
                              kIDTScript.hotkey(),
                              (fltk::Callback*)attach_ctl_idt_script_cb,
                              this);
                    menu.add( _("Image/Modify CTL ASC_CDL SOP Saturation"),
                              kSOPSatNodes.hotkey(),
                              (fltk::Callback*)modify_sop_sat_cb,
                              this);
                    menu.add( _("Image/Add CTL Look Mod Transform"),
                              kLookModScript.hotkey(),
                              (fltk::Callback*)attach_ctl_lmt_script_cb,
                              this);
                    menu.add( _("Image/Attach CTL Rendering Transform"),
                              kCTLScript.hotkey(),
                              (fltk::Callback*)attach_ctl_script_cb,
                              this, fltk::MENU_DIVIDER);
                    menu.add( _("Image/Attach ICC Color Profile"),
                              kIccProfile.hotkey(),
                              (fltk::Callback*)attach_color_profile_cb,
                              this, fltk::MENU_DIVIDER);
                }

                menu.add( _("Image/Mirror/Horizontal"),
                          kFlipX.hotkey(),
                          (fltk::Callback*)flip_x_cb,
                          this);
                menu.add( _("Image/Mirror/Vertical"),
                          kFlipY.hotkey(),
                          (fltk::Callback*)flip_y_cb,
                          this);
                menu.add( _("Image/Set as Background"), kSetAsBG.hotkey(),
                          (fltk::Callback*)set_as_background_cb,
                          (void*)this);
                menu.add( _("Image/Switch FG and BG"),
                          kSwitchFGBG.hotkey(),
                          (fltk::Callback*)switch_fg_bg_cb, (void*)this);
                menu.add( _("Image/Toggle Background"),
                          kToggleBG.hotkey(),
                          (fltk::Callback*)toggle_background_cb, (void*)this);

                Image_ptr image = fg->image();

                if ( Preferences::use_ocio )
                {
                    menu.add( _("OCIO/Input Color Space"),
                              kOCIOInputColorSpace.hotkey(),
                              (fltk::Callback*)attach_ocio_ics_cb, (void*)this);
                    menu.add( _("OCIO/Display"),
                              kOCIODisplay.hotkey(),
                              (fltk::Callback*)attach_ocio_display_cb, (void*)this);
                    menu.add( _("OCIO/View"),
                              kOCIOView.hotkey(),
                              (fltk::Callback*)attach_ocio_view_cb, (void*)this);
                }


                size_t num = image->number_of_video_streams();
                if ( num > 1 )
                {
                    for ( unsigned i = 0; i < num; ++i )
                    {
                        char buf[256];
                        sprintf( buf, _("Video/Track #%d - %s"), i,
                                 image->video_info(i).language.c_str() );

                        item = menu.add( buf, 0,
                                         (fltk::Callback*)change_video_cb, this );
                        item->type( fltk::Item::TOGGLE );
                        if ( image->video_stream() == i )
                            item->set();
                    }
                }

                num = image->number_of_subtitle_streams();

                if ( dynamic_cast< aviImage* >( image ) != NULL )
                {
                    item = menu.add( _("Subtitle/Load"), 0,
                                     (fltk::Callback*)load_subtitle_cb, uiMain );
                }

                if ( num > 0 )
                {
                    item = menu.add( _("Subtitle/No Subtitle"), 0,
                                     (fltk::Callback*)change_subtitle_cb, this );
                    item->type( fltk::Item::TOGGLE );
                    if ( image->subtitle_stream() == -1 )
                        item->set();
                    for ( unsigned i = 0; i < num; ++i )
                    {
                        char buf[256];
                        sprintf( buf, _("Subtitle/Track #%d - %s"), i,
                                 image->subtitle_info(i).language.c_str() );

                        item = menu.add( buf, 0,
                                         (fltk::Callback*)change_subtitle_cb, this );
                        item->type( fltk::Item::TOGGLE );
                        if ( image->subtitle_stream() == i )
                            item->set();
                    }
                }

                if ( 1 )
                {

                    menu.add( _("Audio/Attach Audio File"), kAttachAudio.hotkey(),
                              (fltk::Callback*)attach_audio_cb, this );
                    menu.add( _("Audio/Edit Audio Frame Offset"),
                              kEditAudio.hotkey(),
                              (fltk::Callback*)edit_audio_cb, this );
                    menu.add( _("Audio/Detach Audio File"), kDetachAudio.hotkey(),
                              (fltk::Callback*)detach_audio_cb, this );
                }

                menu.add( _("Pixel/Copy RGBA Values to Clipboard"),
                          kCopyRGBAValues.hotkey(),
                          (fltk::Callback*)copy_pixel_rgba_cb, (void*)this);
            }


            if ( !Preferences::use_ocio )
            {

                menu.add( _("Monitor/Attach CTL Display Transform"),
                          kMonitorCTLScript.hotkey(),
                          (fltk::Callback*)monitor_ctl_script_cb,
                          uiMain);
                menu.add( _("Monitor/Attach ICC Color Profile"),
                          kMonitorIccProfile.hotkey(),
                          (fltk::Callback*)monitor_icc_profile_cb,
                          uiMain, fltk::MENU_DIVIDER);
            }

            menu.popup( fltk::Rectangle( fltk::event_x(),
                                         fltk::event_y(), 80, 1) );
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
 * @param x fltk::event_x() coordinate
 * @param y fltk::event_y() coordinate
 */
void ImageView::leftMouseUp( int x, int y )
{

    flags &= ~kMouseDown;
    flags &= ~kMouseMove;
    flags &= ~kZoom;

    int button = fltk::event_button();
    if (button == 1)
        flags &= ~kMouseLeft;
    else if ( button == 2 )
        flags &= ~kMouseMiddle;
    else
        flags &= ~kMouseRight;

    if ( _mode == kSelection )
    {
        scrub_mode();
        return;
    }

    window()->cursor( fltk::CURSOR_CROSS );

    mrv::media fg = foreground();

    if ( !fg || fg->image()->shapes().empty() ) return;

    //
    // Send the shapes over the network
    //
    if ( _mode == kDraw )
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
        }
    }
    else if ( _mode == kErase )
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
        }
    }
    else if ( _mode == kText )
    {
        mrv::shape_type_ptr o = fg->image()->shapes().back();
        GLTextShape* s = dynamic_cast< GLTextShape* >( o.get() );
        if ( s == NULL )
        {
            LOG_ERROR( _("Not a GLTextShape pointer in mouseRelease") );
        }
        else
        {
            send_network( s->send() );
        }
    }
    // else if ( _mode == kScrub || _mode == kMovePicture ||
    //           _mode == kScalePicture )
    // {
    //     _mode = kNoAction;
    // }

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

    bool blend = img->hires()->format() != image_type::kLumma;

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

    //
    // To represent pixel properly, we need to do the lut
    //
    if ( use_lut() && ( p == kRGBA_Lut || p == kRGBA_Full ) )
    {
        Imath::V3f in( rgba.r, rgba.g, rgba.b );
        Imath::V3f out;
        _engine->evaluate( img, in, out );
        rgba.r = out[0];
        rgba.g = out[1];
        rgba.b = out[2];
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
                                     int& w, int& h ) const
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

    if ( _wait )
    {
        window()->cursor( fltk::CURSOR_WAIT );
    }
    else
    {
        window()->cursor(fltk::CURSOR_CROSS);
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

    xp -= daw[idx].x();
    yp -= daw[idx].y();


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
 * @param x fltk::event_x() coordinate
 * @param y fltk::event_y() coordinate
 */
void ImageView::mouseMove(int x, int y)
{
    if ( !uiMain || !uiMain->uiPixelBar->visible() || !_engine ) return;

    mrv::media fg = foreground();
    if ( !fg ) return;

    CMedia* img = fg->image();

    mrv::image_type_ptr pic;
    bool outside = false;
    int xp = x, yp = y, w, h;
    picture_coordinates( img, x, y, outside, pic, xp, yp, w, h );
    if ( !pic ) return;

    double xpct = 1.0 / img->scale_x();
    double ypct = 1.0 / img->scale_y();

    int ypr = pic->height() - yp - 1;
    char buf[40];
    sprintf( buf, "%5d, %5d", xp, ypr );
    uiMain->uiCoord->text(buf);

    CMedia::Pixel rgba;
    if ( outside || ypr < 0 )
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
        const mrv::image_type_ptr picb = bgr->hires();
        const mrv::Recti& dpw = img->display_window();
        const mrv::Recti& daw = img->data_window();
        const mrv::Recti& dpwb = bgr->display_window(picb->frame());
        const mrv::Recti& dawb = bgr->data_window(picb->frame());
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

    if ( vr() )
    {
        rgba.r = rgba.g = rgba.b = rgba.a =
                                       std::numeric_limits<float>::quiet_NaN();
    }

    switch( uiMain->uiAColorType->value() )
    {
    case kRGBA_Float:
        uiMain->uiPixelR->text( float_printf( rgba.r ).c_str() );
        uiMain->uiPixelG->text( float_printf( rgba.g ).c_str() );
        uiMain->uiPixelB->text( float_printf( rgba.b ).c_str() );
        uiMain->uiPixelA->text( float_printf( rgba.a ).c_str() );
        break;
    case kRGBA_Hex:
        uiMain->uiPixelR->text( hex_printf( rgba.r ).c_str() );
        uiMain->uiPixelG->text( hex_printf( rgba.g ).c_str() );
        uiMain->uiPixelB->text( hex_printf( rgba.b ).c_str() );
        uiMain->uiPixelA->text( hex_printf( rgba.a ).c_str() );
        break;
    case kRGBA_Decimal:
        uiMain->uiPixelR->text( dec_printf( rgba.r ).c_str() );
        uiMain->uiPixelG->text( dec_printf( rgba.g ).c_str() );
        uiMain->uiPixelB->text( dec_printf( rgba.b ).c_str() );
        uiMain->uiPixelA->text( dec_printf( rgba.a ).c_str() );
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

    fltk::Color c( fltk::color( col[0], col[1], col[2] ) );

    // bug in fltk color lookup? (0 != fltk::BLACK)
    if ( c == 0 )
    {
        uiMain->uiPixelView->color( fltk::BLACK );
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

    uiMain->uiPixelH->text( float_printf( hsv.r ).c_str() );
    uiMain->uiPixelS->text( float_printf( hsv.g ).c_str() );
    uiMain->uiPixelV->text( float_printf( hsv.b ).c_str() );

    mrv::BrightnessType brightness_type = (mrv::BrightnessType)
                                          uiMain->uiLType->value();
    hsv.a = calculate_brightness( rgba, brightness_type );
    uiMain->uiPixelL->text( float_printf( hsv.a ).c_str() );
}

float ImageView::vr_angle() const
{
    if (!_engine) return 45.0;
    return _engine->angle();
}

void ImageView::vr_angle( const float t )
{
    if ( t >= 90.0f || t <= 5.0f || !_engine ) return;

    char buf[32];
    sprintf( buf, "VRangle %g", t );
    send_network( buf );

    _engine->angle( t );
    valid(0);
    redraw();
}

/**
 * Handle a mouse drag
 *
 * @param x fltk::event_x() coordinate
 * @param y fltk::event_y() coordinate
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
            gain( _gain + float(dx) / 2000.0f );
            lastX = x;
            lastY = y;
        }
        else if ( flags & kMouseMove )
        {
            window()->cursor( fltk::CURSOR_MOVE );
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

                char buf[128];
                sprintf( buf, "Spin %g %g", spinx, spiny );
                send_network( buf );
            }
            else
            {
                xoffset += double(dx) / _zoom;
                yoffset -= double(dy) / _zoom;

                char buf[128];
                sprintf( buf, "Offset %g %g", xoffset, yoffset );
                send_network( buf );
            }

            lastX = x;
            lastY = y;


        }
        else if ( _mode == kScrub )
        {
            double s;
            s = uiMain->uiPrefs->uiPrefsScrubbingSensitivity->value();
            double dx = (fltk::event_x() - lastX) / s;
            if ( std::abs(dx) >= 1.0f )
            {
                scrub( dx );
                lastX = fltk::event_x();
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

                char buf[128];
                sprintf( buf, "ScalePicture %g %g", px, py );
                send_network( buf );
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

                char buf[128];
                sprintf( buf, "MovePicture %g %g", px, py );
                send_network( buf );
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
            data_window_coordinates( img, xf, yf, true );

            mrv::Recti daw[2], dpw[2];

            daw[0] = img->data_window();
            daw[1] = img->data_window2();
            dpw[0] = img->display_window();
            dpw[1] = img->display_window2();

            double xn = double(x);
            double yn = double(y);
            data_window_coordinates( img, xn, yn, true );


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



            if ( _mode == kSelection )
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

                char buf[128];
                sprintf( buf, "Selection %g %g %g %g", _selection.x(),
                         _selection.y(), _selection.w(), _selection.h() );


                send_network( buf );

            }


            if ( _mode == kDraw || _mode == kErase )
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

                    yn = H - yn;
                    yn -= H;

                    xn += daw[idx].x();
                    yn -= daw[idx].y();

                    mrv::Point p( xn, yn );
                    s->pts.push_back( p );
                }
            }
            else if ( _mode == kText )
            {
                GLShapeList& shapes = fg->image()->shapes();
                if ( shapes.empty() ) return;

                mrv::shape_type_ptr o = shapes.back();
                GLTextShape* s = dynamic_cast< GLTextShape* >( o.get() );
                if ( s == NULL )
                {
                    LOG_ERROR( _("Not a GLTextShape pointer in position") );
                }
                else
                {
                    yn = H - yn;
                    yn -= H;

                    xn += daw[idx].x();
                    yn -= daw[idx].y();

                    s->position( int(xn), int(yn) );
                }
            }

            assert( _selection.w() >= 0.0 );
            assert( _selection.h() >= 0.0 );

            update_color_info( fg );

            mouseMove( x, y );
        }

        redraw();
    }

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

    if ( _mode == kDraw || _mode == kErase )
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

    if ( kDrawMode.match( rawkey ) )
    {
        draw_mode();
        return 1;
    }
    else if ( kEraseMode.match( rawkey ) )
    {
        erase_mode();
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
    else if ( kOpenSingleImage.match( rawkey ) )
    {
        open_single_cb( this, browser() );
        return 1;
    }
    else if ( kOpenClipXMLMetadata.match( rawkey ) )
    {
        open_clip_xml_metadata_cb( this, this );
        return 1;
    }
    else if ( kCloneImage.match( rawkey ) )
    {
        clone_image_cb( NULL, browser() );
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
    else if ( kSaveClipXMLMetadata.match( rawkey ) )
    {
        save_clip_xml_metadata_cb( this, this );
        return 1;
    }
    else if ( kIDTScript.match( rawkey ) )
    {
        attach_ctl_idt_script_cb( NULL, this );
        return 1;
    }
    else if ( kIccProfile.match( rawkey ) )
    {
        attach_color_profile_cb( NULL, this );
        return 1;
    }
    else if ( kLookModScript.match( rawkey ) )
    {
        attach_ctl_lmt_script_cb( NULL, this );
        return 1;
    }
    else if ( kCTLScript.match( rawkey ) )
    {
        attach_ctl_script_cb( NULL, this );
        return 1;
    }
    else if ( kMonitorCTLScript.match( rawkey ) )
    {
        monitor_ctl_script_cb( NULL, uiMain );
        return 1;
    }
    else if ( kMonitorIccProfile.match( rawkey ) )
    {
        monitor_icc_profile_cb( NULL, uiMain );
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
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kPreviousChannel.match( rawkey ) )
    {
        previous_channel_cb(this, this);
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kNextChannel.match( rawkey ) )
    {
        next_channel_cb(this, this);
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kExposureMore.match( rawkey ) )
    {
        exposure_change( 0.5f );
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kExposureLess.match( rawkey ) )
    {
        exposure_change( -0.5f );
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kGammaMore.match( rawkey ) )
    {
        gamma( gamma() + 0.1f );
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kGammaLess.match( rawkey ) )
    {
        gamma( gamma() - 0.1f );
        mouseMove( fltk::event_x(), fltk::event_y() );
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
            if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
                    fltk::event_key_state( fltk::RightCtrlKey ) )
                z = 1.0f / z;
            zoom_under_mouse( z, fltk::event_x(), fltk::event_y() );
        }
        return 1;
    }
    else if ( kZoomIn.match( rawkey ) )
    {
        zoom_under_mouse( _zoom * 2.0f,
                          fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kZoomOut.match( rawkey ) )
    {
        zoom_under_mouse( _zoom * 0.5f,
                          fltk::event_x(), fltk::event_y() );
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
    else if ( kSafeAreas.match( rawkey ) )
    {
        safe_areas( safe_areas() ^ true );
        mouseMove( fltk::event_x(), fltk::event_y() );
        redraw();
        return 1;
    }
    else if ( kDataWindow.match( rawkey ) )
    {
        data_window( data_window() ^ true );
        mouseMove( fltk::event_x(), fltk::event_y() );
        redraw();
        return 1;
    }
    else if ( kDisplayWindow.match( rawkey ) )
    {
        display_window( display_window() ^ true );
        mouseMove( fltk::event_x(), fltk::event_y() );
        redraw();
        return 1;
    }
    else if ( kHudToggle.match( rawkey ) )
    {
        hud_toggle_cb( NULL, uiMain );
        mouseMove( fltk::event_x(), fltk::event_y() );
        redraw();
        return 1;
    }
    else if ( kWipe.match( rawkey ) )
    {
        if ( _wipe_dir == kNoWipe )  {
            _wipe_dir = kWipeVertical;
            _wipe = (float) fltk::event_x() / float( w() );

            char buf[128];
            sprintf( buf, "WipeVertical %g", _wipe );
            send_network( buf );

            window()->cursor(fltk::CURSOR_WE);
        }
        else if ( _wipe_dir & kWipeVertical )
        {
            _wipe_dir = kWipeHorizontal;
            _wipe = (float) (h() - fltk::event_y()) / float( h() );
            char buf[128];
            sprintf( buf, "WipeHorizontal %g", _wipe );
            send_network( buf );
            window()->cursor(fltk::CURSOR_NS);
        }
        else if ( _wipe_dir & kWipeHorizontal ) {
            _wipe_dir = kNoWipe;
            _wipe = 0.0f;
            char buf[128];
            sprintf( buf, "NoWipe" );
            send_network( buf );
            window()->cursor(fltk::CURSOR_CROSS);
        }

        mouseMove( fltk::event_x(), fltk::event_y() );
        redraw();
        return 1;
    }
    else if ( kFlipX.match( rawkey ) )
    {
        _flip = (FlipDirection)( (int) _flip ^ (int)kFlipVertical );
        fit_image();
        mouseMove( fltk::event_x(), fltk::event_y() );
        redraw();
        return 1;
    }
    else if ( kFlipY.match( rawkey ) )
    {
        _flip = (FlipDirection)( (int) _flip ^ (int)kFlipHorizontal );
        fit_image();
        mouseMove( fltk::event_x(), fltk::event_y() );
        redraw();
        return 1;
    }
    else if ( kAttachAudio.match(rawkey) )
    {
        attach_audio_cb( NULL, this );
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
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kFrameStepBack.match(rawkey) )
    {
        step_frame( -1 );
        mouseMove( fltk::event_x(), fltk::event_y() );
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
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kFrameStepFwd.match( rawkey ) )
    {
        step_frame( 1 );
        mouseMove( fltk::event_x(), fltk::event_y() );
        redraw();
        return 1;
    }
    else if ( kPlayBackTwiceSpeed.match( rawkey ) )
    {
        mrv::media fg = foreground();
        if ( ! fg ) return 1;

        const CMedia* img = fg->image();
        double FPS = 24;
        if ( img ) FPS = img->play_fps();
        fps( FPS * 2 );
        if ( playback() == CMedia::kBackwards )
            stop();
        else
            play_backwards();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kPlayBackHalfSpeed.match( rawkey ) )
    {
        mrv::media fg = foreground();
        if ( ! fg ) return 1;

        const CMedia* img = fg->image();
        double FPS = 24;
        if ( img ) FPS = img->play_fps();
        fps( FPS / 2 );

        if ( playback() == CMedia::kBackwards )
            stop();
        else
            play_backwards();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kPlayBack.match( rawkey ) )
    {
        if ( playback() == CMedia::kBackwards )
            stop();
        else
            play_backwards();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kPlayFwdTwiceSpeed.match( rawkey ) )
    {
        mrv::media fg = foreground();
        if ( ! fg ) return 1;

        const CMedia* img = fg->image();
        double FPS = 24;
        if ( img ) FPS = img->play_fps();

        fps( FPS * 2 );
        if ( playback() == CMedia::kForwards )
            stop();
        else
            play_forwards();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kPlayFwdHalfSpeed.match( rawkey ) )
    {
        mrv::media fg = foreground();
        if ( ! fg ) return 1;

        const CMedia* img = fg->image();
        double FPS = 24;
        if ( img ) FPS = img->play_fps();

        fps( FPS / 2 );
        if ( playback() == CMedia::kForwards )
            stop();
        else
            play_forwards();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kPlayFwd.match( rawkey ) )
    {
        if ( playback() == CMedia::kForwards )
            stop();
        else
            play_forwards();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kStop.match( rawkey ) )
    {
        stop();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kSwitchFGBG.match( rawkey ) )
    {
        switch_fg_bg_cb( this, this );
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kPreviousVersionImage.match( rawkey ) )
    {
        previous_image_version_cb(this, browser());
        update_title_bar( this );
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kNextVersionImage.match( rawkey ) )
    {
        next_image_version_cb(this, browser());
        update_title_bar( this );
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kPreviousImage.match( rawkey ) )
    {
        previous_image_cb(this, browser());
        update_title_bar( this );
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kNextImage.match( rawkey ) )
    {
        next_image_cb(this, browser());
        update_title_bar( this );
        mouseMove( fltk::event_x(), fltk::event_y() );
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
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kRotateMinus90.match( rawkey ) )
    {
        rotate_minus_90_cb( NULL, this );
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kToggleICS.match( rawkey ) )
    {
        toggle_ics_cb( NULL, main() );
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kFirstFrame.match( rawkey ) )
    {
        if ( fltk::event_key_state( fltk::LeftCtrlKey )  ||
                fltk::event_key_state( fltk::RightCtrlKey ) )
            first_frame_timeline();
        else
            first_frame();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kLastFrame.match( rawkey ) )
    {
        if ( fltk::event_key_state( fltk::LeftCtrlKey )  ||
                fltk::event_key_state( fltk::RightCtrlKey ) )
            last_frame_timeline();
        else
            last_frame();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kToggleBG.match( rawkey ) )
    {
        toggle_background();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kToggleTopBar.match( rawkey ) )
    {
        if ( uiMain->uiTopBar->visible() ) uiMain->uiTopBar->hide();
        else uiMain->uiTopBar->show();
        uiMain->uiRegion->relayout( fltk::LAYOUT_XYWH |
                                    fltk::LAYOUT_DAMAGE |
                                    fltk::LAYOUT_CHILD );
        uiMain->uiRegion->redraw();
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kTogglePixelBar.match( rawkey ) )
    {
        if ( uiMain->uiPixelBar->visible() ) uiMain->uiPixelBar->hide();
        else uiMain->uiPixelBar->show();
        uiMain->uiRegion->relayout( fltk::LAYOUT_XYWH |
                                    fltk::LAYOUT_DAMAGE |
                                    fltk::LAYOUT_CHILD );
        uiMain->uiRegion->redraw();
        return 1;
    }
    else if ( kToggleTimeline.match( rawkey ) )
    {
        if ( uiMain->uiBottomBar->visible() ) uiMain->uiBottomBar->hide();
        else uiMain->uiBottomBar->show();
        uiMain->uiRegion->relayout( fltk::LAYOUT_XYWH |
                                    fltk::LAYOUT_DAMAGE |
                                    fltk::LAYOUT_CHILD );
        uiMain->uiRegion->redraw();
        mouseMove( fltk::event_x(), fltk::event_y() );
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
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kToggleLut.match( rawkey ) )
    {
        toggle_lut();
        mouseMove( fltk::event_x(), fltk::event_y() );
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
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kSetInPoint.match( rawkey ) )
    {
        uiMain->uiStartButton->do_callback();
    }
    else if ( kSetOutPoint.match( rawkey ) )
    {
        uiMain->uiEndButton->do_callback();
    }
    else if ( ( _mode == kNoAction || _mode == kScrub ) &&
              kAreaMode.match( rawkey ) )
    {
        selection_mode();
        return 1;
    }
    else if ( rawkey == fltk::LeftAltKey )
    {
        flags |= kLeftAlt;
        return 1;
    }
    else
    {
        // Check if a menu shortcut
        fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;

        // check if a channel shortcut
        unsigned short num = uiColorChannel->children();
        unsigned short idx = 0;
        fltk::Group* g = NULL;
        for ( unsigned short c = 0; c < num; ++c, ++idx )
        {
            fltk::Widget* w = uiColorChannel->child(c);
            if ( rawkey == w->shortcut() )
            {
                channel( idx );
                return 1;
            }

            g = NULL;
            if ( w->is_group() )
            {
                g = (fltk::Group*) w;
                unsigned numc = g->children();
                for ( unsigned short i = 0; i < numc; ++i )
                {
                    ++idx;
                    if ( rawkey == g->child(i)->shortcut() )
                    {
                        channel( idx );
                        return 1;
                    }
                }
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
    if ( key == fltk::LeftAltKey )
    {
        flags &= ~kLeftAlt;
        flags &= ~kZoom;
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
        posX = fltk_main()->x();
        posY = fltk_main()->y();
        fltk_main()->fullscreen();
    }
    else
    {
        FullScreen = false;
        presentation = false;
        resize_main_window();
    }
    fltk_main()->relayout();

    fit_image();

    // These two take focus are needed
    fltk_main()->take_focus();

    take_focus();


    fltk::check();

    char buf[128];
    sprintf( buf, "FullScreen %d", FullScreen );
    send_network( buf );
}

void ImageView::toggle_presentation()
{
    fltk::Window* uiImageInfo = uiMain->uiImageInfo->uiMain;
    fltk::Window* uiColorArea = uiMain->uiColorArea->uiMain;
    fltk::Window* uiEDLWindow = uiMain->uiEDLWindow->uiMain;
    fltk::Window* uiReel  = uiMain->uiReelWindow->uiMain;
    fltk::Window* uiPrefs = uiMain->uiPrefs->uiMain;
    fltk::Window* uiAbout = uiMain->uiAbout->uiMain;
    fltk::Window* uiStereo = uiMain->uiStereo->uiMain;
    fltk::Window* uiPaint = uiMain->uiPaint->uiMain;
    fltk::Window* uiSOP = uiMain->uiSOPNode ? uiMain->uiSOPNode->uiMain : NULL;


    static bool has_image_info, has_color_area, has_reel, has_edl_edit,
           has_prefs, has_about, has_top_bar, has_bottom_bar, has_pixel_bar,
           has_stereo, has_paint, has_sop;

    if ( !presentation )
    {
        posX = fltk_main()->x();
        posY = fltk_main()->y();

        has_image_info = uiImageInfo ? uiImageInfo->visible() : false;
        has_color_area = uiColorArea ? uiColorArea->visible() : false;
        has_reel       = uiReel ? uiReel->visible() : false;
        has_edl_edit   = uiEDLWindow ? uiEDLWindow->visible() : false;
        has_prefs      = uiPrefs ? uiPrefs->visible() : false;
        has_about      = uiAbout ? uiAbout->visible() : false;
        has_top_bar    = uiMain->uiTopBar->visible();
        has_bottom_bar = uiMain->uiBottomBar->visible();
        has_pixel_bar  = uiMain->uiPixelBar->visible();
        has_paint      = uiPaint ? uiPaint->visible() : false;
        has_stereo     = uiStereo ? uiStereo->visible() : false;
        has_sop        = uiSOP ? uiSOP->visible() : false;

        if (uiSOP) uiSOP->hide();
        uiPaint->hide();
        uiStereo->hide();
        uiImageInfo->hide();
        uiColorArea->hide();
        uiReel->hide();
        uiEDLWindow->hide();
        uiPrefs->hide();
        uiAbout->hide();
        uiMain->uiTopBar->hide();
        uiMain->uiPixelBar->hide();
        uiMain->uiBottomBar->hide();


        presentation = true;

#ifdef WIN32
        fltk_main()->fullscreen();
        fltk_main()->resize(0, 0,
                            GetSystemMetrics(SM_CXSCREEN),
                            GetSystemMetrics(SM_CYSCREEN));
#else
        send_wm_state_event(fltk::xid( fltk_main() ), 1,
                            fl_NET_WM_STATE_FULLSCREEN);
#endif
    }
    else
    {
        if ( has_image_info ) uiImageInfo->show();
        if ( has_color_area ) uiColorArea->show();
        if ( has_reel  )      uiReel->show();
        if ( has_edl_edit )   uiEDLWindow->show();
        if ( has_prefs )      uiPrefs->show();
        if ( has_about )      uiAbout->show();
        if ( has_paint )      uiPaint->show();
        if ( has_stereo )     uiStereo->show();
        if ( has_sop )        uiSOP->show();

        if ( has_top_bar )    uiMain->uiTopBar->show();
        if ( has_bottom_bar)  uiMain->uiBottomBar->show();
        if ( has_pixel_bar )  uiMain->uiPixelBar->show();

#ifndef _WIN32
        send_wm_state_event(fltk::xid( fltk_main() ), 0,
                            fl_NET_WM_STATE_FULLSCREEN);
#endif
        presentation = false;
        FullScreen = false;
        resize_main_window();
    }

    fltk_main()->relayout();

    fit_image();

    // These two take focus are needed
    fltk_main()->take_focus();

    take_focus();


    fltk::check();


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

    update_color_info();
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

/**
 * Main fltk event handler
 *
 * @param event event to handle
 *
 * @return 1 if handled, 0 if not
 */
int ImageView::handle(int event)
{
    TRACE("");
    switch( event )
    {
    case fltk::TIMEOUT:
    {
        TRACE("");

        mrv::ImageBrowser* b = browser();
        if ( b && !_idle_callback && CMedia::cache_active()  &&
             ( CMedia::preload_cache() ||
               uiMain->uiPrefs->uiPrefsPlayAllFrames->value() ) )
        {
            unsigned _reel = b->number_of_reels();
            unsigned i = b->reel_index();
            for ( ; i < b->number_of_reels(); ++i )
            {
                mrv::Reel r = b->reel_at( i );
                if (!r) continue;

                mrv::media fg = r->media_at( frame() );
                if (!fg) continue;

                CMedia* img = fg->image();
                if ( !img->is_cache_full() && !img->has_video() &&
                     img->playback() == CMedia::kStopped )
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
            if ( _idle_callback && b->reel_index() >= b->number_of_reels() )
            {
                preload_cache_stop();
            }
        }
        timeout();
        TRACE("");
        return 1;
    }
    case fltk::FOCUS:
    {
        if ( _wait )
        {
            window()->cursor( fltk::CURSOR_WAIT );
        }
        else
        {
            window()->cursor(fltk::CURSOR_CROSS);
        }
        TRACE("");
        return fltk::GlWindow::handle( event );
    }
    case fltk::ENTER:
        TRACE("");
        focus(this);
        window()->cursor(fltk::CURSOR_CROSS);
        TRACE("");
        fltk::GlWindow::handle( event );
        TRACE("");
        return 1;
    case fltk::UNFOCUS:
    case fltk::LEAVE:
        TRACE("");
        window()->cursor(fltk::CURSOR_DEFAULT);
        TRACE("");
        fltk::GlWindow::handle( event );
        TRACE("");
        return 1;
    case fltk::PUSH:
        return leftMouseDown(fltk::event_x(), fltk::event_y());
        break;
    case fltk::RELEASE:
        leftMouseUp(fltk::event_x(), fltk::event_y());
        return 1;
        break;
    case fltk::MOVE:
        TRACE("");
        X = fltk::event_x();
        Y = fltk::event_y();

        if ( _wipe_dir != kNoWipe )
        {
            char buf[128];
            switch( _wipe_dir )
            {
            case kWipeVertical:
                _wipe = (float) fltk::event_x() / (float)w();
                sprintf( buf, "WipeVertical %g", _wipe );
                send_network( buf );
                window()->cursor(fltk::CURSOR_WE);
                break;
            case kWipeHorizontal:
                _wipe = (float) (h() - fltk::event_y()) / (float)h();
                sprintf( buf, "WipeHorizontal %g", _wipe );
                send_network( buf );
                window()->cursor(fltk::CURSOR_NS);
                break;
            default:
                break;
            }
            redraw();
            return 1;
        }

        mouseMove(fltk::event_x(), fltk::event_y());

        if ( _mode == kDraw || _mode == kErase )
            redraw();

        TRACE("");
        fltk::GlWindow::handle( event );
        TRACE("");
        return 1;
        break;
    case fltk::DRAG:
        X = fltk::event_x();
        Y = fltk::event_y();
        mouseDrag( int(X), int(Y) );
        return 1;
        break;
        //     case fltk::SHORTCUT:
    case fltk::KEY:
        // lastX = fltk::event_x();
        // lastY = fltk::event_y();
        return keyDown(fltk::event_key());
    case fltk::KEYUP:
        return keyUp(fltk::event_key());
    case fltk::MOUSEWHEEL:
    {
        if ( vr() )
        {
            float t = vr_angle();
            t += fltk::event_dy();
            vr_angle( t );
        }
        else
        {
            if ( fltk::event_dy() < 0.f )
            {
                zoom_under_mouse( _zoom * 2.0f,
                                  fltk::event_x(), fltk::event_y() );
            }
            else
            {
                zoom_under_mouse( _zoom * 0.5f,
                                  fltk::event_x(), fltk::event_y() );
            }
        }
        return 1;
        break;
    }
    case fltk::DND_ENTER:
    case fltk::DND_LEAVE:
    case fltk::DND_DRAG:
    case fltk::DND_RELEASE:
        return 1;
    case fltk::PASTE:
        browser()->handle_dnd();
        return 1;
    default:
        return fltk::GlWindow::handle( event );
    }

    return 0;
}



/**
 * Refresh the images in the view window
 *
 */
void ImageView::refresh()
{
    mrv::media fg = foreground();

    if ( fg )
    {
        CMedia* img = fg->image();
        img->refresh();
    }

    mrv::media bg = background();
    if ( bg )
    {
        CMedia* img = bg->image();
        img->refresh();
    }
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
        if ( idx == _fg_reel )
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
        _preframe = img->first_frame();
    }
}


void ImageView::preload_cache_start()
{
    if ( !foreground() ) return;

    if (!_idle_callback)
    {
        CMedia* img;
        mrv::Reel r = browser()->reel_at( fg_reel() );
        if ( r && r->edl )
        {
            timeline()->edl( true );
            _preframe = frame();
            CMedia* img = r->image_at( _preframe );
        }
        else
        {
            mrv::media fg = foreground();
            if ( fg )
            {
                img = fg->image();
                _preframe = img->first_frame();
            }
        }
        CMedia::preload_cache( true );
        _idle_callback = true;
        // boost::thread t( boost::bind( thread_dispatcher, this ) );
        // t.detach();
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
    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    char* lbl = NULL;
    unsigned short idx = 0;
    unsigned num = uiColorChannel->children();
    for ( unsigned i = 0; i < num; ++i, ++idx )
    {
        fltk::Widget* w = uiColorChannel->child(i);
        if ( idx == c )
        {
            lbl = strdup( w->label() );
            // if ( w->is_group() ||
            //      strcmp( lbl, _("Color") ) == 0 )
            //     _old_channel = idx;
            break;
        }

        if ( w->is_group() )
        {
            fltk::Group* g = (fltk::Group*) w;
            unsigned short numc = g->children();
            std::string layername;
            if ( w->label() )
                layername = w->label();
            for ( unsigned short j = 0; j < numc; ++j )
            {
                ++idx;
                if ( idx == c )
                {
                    if ( !layername.empty() ) layername += '.';
                    layername += g->child(j)->label();
                    lbl = strdup( layername.c_str() );
                    break;
                }

            }
        }

        if ( lbl ) break;
    }


    if ( !lbl )
    {
        LOG_WARNING( _("Color channel not found at index ") << c );
        return NULL;
    }

    return lbl;
}

void ImageView::channel( fltk::Widget* o )
{
    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    unsigned short num = uiColorChannel->children();
    unsigned short idx = 0;
    bool found = false;
    for ( unsigned short i = 0; i < num; ++i, ++idx )
    {
        fltk::Widget* w = uiColorChannel->child(i);
        if ( w == o ) {
            found = true;
            break;
        }

        if ( w->is_group() )
        {
            fltk::Group* g = (fltk::Group*) w;
            unsigned short numc = g->children();
            for ( unsigned short j = 0; j < numc; ++j )
            {
                ++idx;
                w = g->child(j);
                if ( w == o )
                {
                    found = true;
                    break;
                }
            }
        }
        if ( found ) break;
    }


    if ( !found )
    {
        LOG_ERROR( "Widget " << o->label() << " not found" );
        return;
    }

    channel( idx );
}

/**
 * Change image display to a new channel (from channel list)
 *
 * @param c index of channel
 */
void ImageView::channel( unsigned short c )
{
    boost::recursive_mutex::scoped_lock lk( _shortcut_mutex );

    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return; // Audio only - no channels


    unsigned short idx = 0;
    for ( unsigned short i = 0; i < num; ++i, ++idx )
    {
        fltk::Widget* w = uiColorChannel->child(i);
        if ( w->is_group() )
        {
            fltk::Group* g = (fltk::Group*) w;
            idx += g->children();
        }
    }

    if ( c >= idx )
    {
        c = 0;
        const char* lbl = uiColorChannel->label();
        if ( lbl && strcmp( lbl, _("(no image)") ) != 0 )
        {
            num = uiColorChannel->children();
            for ( unsigned short i = 0; i < num; ++i, ++c )
            {
                fltk::Widget* w = uiColorChannel->child(i);
                if ( strcmp( w->label(), lbl ) == 0 ) break;
                if ( w->is_group() )
                {
                    fltk::Group* g = (fltk::Group*) w;
                    unsigned n = g->children();
                    for ( unsigned j = 0; j < n; ++j, ++c )
                    {
                        w = g->child(j);
                        if ( strcmp( w->label(), lbl ) == 0 ) break;
                    }
                }
            }
        }

        if ( c >= idx )
        {
            LOG_ERROR( _("Invalid index ") << c << _(" for channel.  Maximum: " )
                       << idx );
            return;
        }
    }

    // If user selected the same channel again, toggle it with
    // other channel (diffuse.r goes to diffuse, for example)
    const mrv::media& fg = foreground();
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
              ext == N_("U") || ext == N_("S") )
    {
        _channelType = kRed;
    }
    else if ( channelName == _("Green") || ext == N_("G") || ext == N_("Y") ||
              ext == N_("V") || ext == N_("T") )
    {
        _channelType = kGreen;
    }
    else if ( channelName == _("Blue")  || ext == N_("B") || ext == N_("Z") ||
              ext == N_("W") )
    {
        _channelType = kBlue;
    }
    else if ( channelName == _("Alpha") || ext == N_("A") )
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
    free( lbl );

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
        _preframe = frame();
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

    char buf[256];
    sprintf( buf, "Gain %g", f );
    send_network( buf );

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
        DBG(  "gamma " << f );
        fg->image()->gamma( f );

        char buf[256];
        sprintf( buf, "Gamma %g", f );
        send_network( buf );

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

    static char tmp[128];
    if ( z >= 1.0f )
    {
        sprintf( tmp, "x%.2g", z );
    }
    else
    {
        sprintf( tmp, "1/%.3g", 1/z );
    }
    uiMain->uiZoom->label( tmp );
    uiMain->uiZoom->redraw();

    char buf[128];
    sprintf( buf, "Zoom %g", z );
    send_network( buf );

    _zoom = z;
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

    char buf[64];
    sprintf( buf, "Gain %g", _gain );
    uiMain->uiView->send_network( buf );
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
    if ( !vr() && (z == _zoom || z > kMaxZoom || z < kMinZoom) ) return;
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

    image_coordinates( img, xf, yf );


    zoom( z );

    int w2 = W / 2;
    int h2 = H / 2;

    xoffset = w2 - xf;
    yoffset = h2 - ( H - yf );
    xoffset -= (offx / _zoom);
    double ratio = 1.0f;
    if ( _showPixelRatio ) ratio = img->pixel_ratio();
    yoffset += (offy / _zoom * ratio);

    char buf[128];
    sprintf( buf, "Offset %g %g", xoffset, yoffset );
    send_network( buf );

    mouseMove( x, y );
}


double ImageView::pixel_ratio() const
{
    mrv::media fg = foreground();
    if ( !fg ) return 1.0f;
    return fg->image()->pixel_ratio();
}

int ImageView::update_shortcuts( const mrv::media& fg,
                                 const char* channelName )
{

    CMedia* img = fg->image();

    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    uiColorChannel->clear();

    const stringArray& layers = img->layers();

    stringArray::const_iterator i = layers.begin();
    stringArray::const_iterator e = layers.end();


    int v   = -1;
    int idx = 0;
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

    bool group = false;
    std::string x;
    fltk::Group* g = NULL;
    fltk::Widget* o = NULL;

    for ( ; i != e; ++i, ++idx )
    {

        const std::string& name = *i;

        std::string tmp = x + '.';
        if ( o && x != _("Alpha") && tmp != "." && name.find(tmp) == 0 )
        {
            if ( group )
            {
                // Copy shortcut to group and replace leaf with group
                unsigned s = 0;
                if ( uiColorChannel->children() >= 1 )
                {
                    unsigned last = uiColorChannel->children()-1;
                    fltk::Widget* w = uiColorChannel->child(last);
                    s = w->shortcut();
                    uiColorChannel->remove( w );
                }
                g = uiColorChannel->add_group( x.c_str(), NULL );
                g->shortcut( s );
                group = false;
            }

            // Now add current leaf, but without # prefix and period
            std::string y = name;

            if ( x.size() < name.size() )
                y = name.substr( x.size()+1, name.size() );

            o = uiColorChannel->add_leaf( y.c_str(), g );
        }
        else
        {
            // A new group, we add it here as empty group
            group = true;
            x = name;

            o = uiColorChannel->add( name.c_str(), NULL );
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
        if ( v >= 0 || name == _("Color") || chx == "N" || chx == "Z" )
        {
            // If we have a shortcut and it isn't in the list of shortcuts
            // yet, add it to interface and shortcut list.
            if ( shortcut && shortcuts.find( shortcut ) ==
                    shortcuts.end())
            {
                // std::cerr << "add shortcut " << (char) shortcut << " v: "
                //        << v << " name " << name << " ch " << ch << std::endl;
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

    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;

    mrv::media fg = foreground();
    if ( !fg )
    {
        _old_fg = NULL;
        uiColorChannel->clear();
        uiColorChannel->add( _("(no image)") );
        uiColorChannel->copy_label( _("(no image)") );
        uiColorChannel->value(0);
        uiColorChannel->redraw();
        return;
    }

    const char* lbl = uiColorChannel->label();
    if ( strcmp(lbl, _("(no image)")) == 0 ) lbl = _("Color");

    int v = update_shortcuts( fg, lbl );

    CMedia* img = fg->image();
    if ( v >= 0 && img != _old_fg )
    {
        channel( (unsigned short) v );
    }


    uiColorChannel->redraw();


    img->image_damage( img->image_damage() & ~CMedia::kDamageLayers );

}


/**
 * Change foreground image
 *
 * @param img new foreground image or NULL for no image.
 */
void ImageView::foreground( mrv::media fg )
{

    mrv::media old = foreground();
    if ( old == fg ) return;

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
                uiMain->uiVolume->value( engine->volume() );
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

            char buf[1024];
            std::string file = img->directory() + '/' + img->name();
            sprintf( buf, "CurrentImage \"%s\" %" PRId64 " %" PRId64,
                     file.c_str(), img->first_frame(), img->last_frame() );
            send_network( buf );

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
                if ( _zoom < 1.0f )
                {
                    fit_image();
                }
                else if ( old )
                {
                    CMedia* o = old->image();
                    if (o)
                    {
                        if ( o->display_window() != img->display_window() )
                            fit_image();
                    }
                }
            }


            img->image_damage( img->image_damage() | CMedia::kDamageContents |
                               CMedia::kDamageLut | CMedia::kDamage3DData );

            if ( dynamic_cast< stubImage* >( img )  )
            {
                create_timeout( 0.2 );
            }
        }
    }


    refresh_audio_tracks();

    // If this is the first image loaded, resize window to fit it
    if ( !old ) {

        posX = fltk_main()->x();
        posY = fltk_main()->y();

        fltk::RadioButton* r = (fltk::RadioButton*) uiMain->uiPrefs->uiPrefsOpenMode->child(0);
        if ( r->value() == 1 )
        {
            resize_main_window();
        }
    }

    update_layers();


    update_title_bar( this );
    update_image_info();
    update_color_info( fg );

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
        sprintf( buf, "Track #%02ld", i );
        uiMain->uiAudioTracks->add( buf );
    }
    uiMain->uiAudioTracks->add( "<no audio>" );
    uiMain->uiAudioTracks->value( img->audio_stream() );
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

        sprintf( buf, "CurrentBGImage \"%s\" %" PRId64 " %" PRId64,
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
        sprintf( buf, "CurrentBGImage \"\"" );
        send_network( buf );
    }

//   _BGpixelSize = 0;
    redraw();
}


/**
 * Resize the containing window to try to fit the image view.
 *
 */
void ImageView::resize_main_window()
{
    int w, h;
    mrv::media fg = foreground();
    if ( !fg )
    {
        w = 640;
        h = 480;
    }
    else
    {
        const CMedia* img = fg->image();

        w = img->width();
        h = img->height();
        h = (int) (h / img->pixel_ratio());
    }

    if ( uiMain->uiTopBar->visible() )
        h += uiMain->uiTopBar->h();

    if ( uiMain->uiPixelBar->visible() )
        h += uiMain->uiPixelBar->h();

    if ( uiMain->uiBottomBar->visible() )
        h += uiMain->uiBottomBar->h();

    PreferencesUI* uiPrefs = uiMain->uiPrefs;
    if ( uiPrefs && uiPrefs->uiWindowFixedPosition->value() )
    {
        posX = (int) uiPrefs->uiWindowXPosition->value();
        posY = (int) uiPrefs->uiWindowYPosition->value();
    }

    if ( uiPrefs && uiPrefs->uiWindowFixedSize->value() )
    {
        w = (int) uiPrefs->uiWindowXSize->value();
        h = (int) uiPrefs->uiWindowYSize->value();
    }

#ifdef _WIN32
    const unsigned kBorders = 8;
    const unsigned kMenus = 30;
#else
    const unsigned kBorders = 0;
    const unsigned kMenus = 30;
#endif

    fltk::Monitor monitor = fltk::Monitor::find( posX, posY );
    int minx = monitor.work.x() + kBorders;
    int miny = monitor.work.y() + kMenus;
    int maxh = monitor.work.h() - kMenus - 6;
#ifdef LINUX
    maxh -= kMenus * 2;
#endif
    int maxw = monitor.work.w() - kBorders * 2;
    int maxx = minx + maxw;
    int maxy = miny + maxh;

    bool fit = false;

    if ( w > maxw ) {
        fit = true;
        w = maxw;
    }
    if ( h > maxh ) {
        fit = true;
        h = maxh;
    }

    if ( posX + w > maxx ) {
        posX = ( w + posX - maxw ) / 2;
    }
    if ( posX + w > maxx ) {
        posX = minx;
        w = maxw;
    }
    if ( posX < minx )     posX = minx;

    if ( posY + h > maxy ) {
        posY = ( h + posY - maxh ) / 2;
    }
    if ( posY + h > maxy ) {
        posY = miny;
        h = maxh;
    }
    if ( posY < miny )     posY = miny;

    if ( w < 640 )  w = 640;
    if ( h < 535 )  h = 535;

    fltk_main()->fullscreen_off( posX, posY, w, h );
    fltk_main()->resize( posX, posY, w, h );
#ifdef LINUX
    fltk_main()->show();
#endif

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
    sprintf( buf, "DataWindow %d", (int)b );
    send_network( buf );
}

void ImageView::display_window( const bool b )
{
    _displayWindow = b;

    char buf[128];
    sprintf( buf, "DisplayWindow %d", (int)b );
    send_network( buf );
}

void ImageView::safe_areas( const bool b )
{
    _safeAreas = b;

    char buf[128];
    sprintf( buf, "SafeAreas %d", (int)b );
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
    uiMain->uiNormalize->state( normalize );

    char buf[128];
    sprintf( buf, "Normalize %d", (int) _normalize );
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
    sprintf( buf, "ShowPixelRatio %d", (int) b );
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
        sprintf( buf, "OCIOView \"%s\" \"%s\"", display.c_str(), view.c_str() );
        send_network( buf );
    }

    sprintf( buf, "UseLUT %d", (int)_useLUT );
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
    return uiMain->uiFrame->frame();
}

/**
 * Change view's frame number in frame indicator and timeline
 *
 * @param f new frame in timeline units
 */
void ImageView::frame( const int64_t f )
{
    // Redraw browser to update thumbnail
    mrv::ImageBrowser* b = browser();
    if (b) b->frame( f );
}



/**
 * Jump to a frame in timeline, creating new thumbnails if needed.
 *
 * @param f new frame in timeline units
 */
void ImageView::seek( const int64_t f )
{
    mrv::ImageBrowser* b = browser();

    // mrv::Reel r = b->current_reel();
    // if ( r && r->edl )
    // {
    //  _preframe = r->global_to_local( f );
    // }
    // else
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

    int64_t start = (int64_t) timeline()->minimum();
    int64_t end   = (int64_t) timeline()->maximum();

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

        uiMain->uiFrame->frame( f );
    }

    int64_t t = int64_t( timeline()->minimum() );
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

        uiMain->uiFrame->frame( f );

    }

    int64_t t = int64_t( timeline()->maximum() );
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
void ImageView::update_color_info( const mrv::media& fg ) const
{
    if ( uiMain->uiColorArea )
    {
        fltk::Window*  uiColorWindow = uiMain->uiColorArea->uiMain;
        if ( uiColorWindow->visible() )
            uiMain->uiColorArea->uiColorText->update();
    }

    if ( uiMain->uiSOPNode )
    {
        fltk::Window* uiSOPNode = uiMain->uiSOPNode->uiMain;
        if ( uiSOPNode->visible() ) uiMain->uiSOPNode->media( fg );
    }

    if ( uiMain->uiVectorscope )
    {
        fltk::Window*  uiVectorscope = uiMain->uiVectorscope->uiMain;
        if ( uiVectorscope->visible() ) uiVectorscope->redraw();
    }

    if ( uiMain->uiWaveform )
    {
        fltk::Window*  uiWaveform = uiMain->uiWaveform->uiMain;
        if ( uiWaveform->visible() ) uiWaveform->redraw();
    }

    if ( uiMain->uiHistogram )
    {
        fltk::Window*  uiHistogram   = uiMain->uiHistogram->uiMain;
        if ( uiHistogram->visible() ) uiHistogram->redraw();
    }

    if (!fg) return;
    CMedia* img = fg->image();
    if ( ! (img->image_damage() & CMedia::kDamageLut) )
        return;


    if ( uiMain->uiICS->visible() )
    {
        update_ICS();
    }

}

void ImageView::update_color_info() const
{
    mrv::media fg = foreground();
    if ( !fg ) return;

    update_color_info(fg);
}

/**
 * Update the image window information display
 *
 */

void ImageView::update_image_info() const
{
    if ( !uiMain->uiImageInfo ) return;

    mrv::media fg = foreground();
    if ( ! fg ) return;

    CMedia* img = NULL;
    if ( selected_image() )
        img = selected_image();
    else
        img = fg->image();
    uiMain->uiImageInfo->uiInfoText->set_image( img );
    img->image_damage( img->image_damage() & ~CMedia::kDamageData );
}

void ImageView::playback( const CMedia::Playback b )
{
    _playback = b;

    _lastFrame = frame();
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


    if ( dir == CMedia::kForwards )
    {
        send_network("playfwd");
    }
    else if ( dir == CMedia::kBackwards )
    {
        send_network("playback");
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

    delete_timeout();

    double fps = uiMain->uiFPS->value();

    create_timeout( 0.5/fps );

    // if ( !img->is_sequence() || img->is_cache_full() || (bg && fg != bg) ||
    //      !CMedia::cache_active() ||
    //      !( CMedia::preload_cache() ||
    //         uiMain->uiPrefs->uiPrefsPlayAllFrames->value() ) ||
    //   img->has_audio() )
    {
        // preload_cache_stop();
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
    stop();
    play( CMedia::kBackwards );
}


void ImageView::thumbnails()
{
    if ( playback() != CMedia::kStopped ) return;


    mrv::media fg = foreground();
    if ( fg ) fg->create_thumbnail();


    mrv::media bg = background();
    if ( bg ) bg->create_thumbnail();


    browser()->redraw();
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


    _playback = CMedia::kStopped;

    _last_fps = 0.0;
    _real_fps = 0.0;

    stop_playback();

    char buf[256];
    sprintf( buf, "stop %" PRId64, frame() );
    send_network( buf );

    if ( uiMain->uiPlayForwards )
        uiMain->uiPlayForwards->value(0);

    if ( uiMain->uiPlayBackwards )
        uiMain->uiPlayBackwards->value(0);


    frame( frame() );
    // seek( int64_t(timeline()->value()) );


    if ( CMedia::preload_cache() && ! _idle_callback )
    {
        preload_cache_start();
    }

    mouseMove( fltk::event_x(), fltk::event_y() );
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


    timeline()->fps( x );
    uiMain->uiFrame->fps( x );
    uiMain->uiStartFrame->fps( x );
    uiMain->uiEndFrame->fps( x );
    uiMain->uiFPS->value( x );

    char buf[128];
    sprintf( buf, "FPS %g", x );
    send_network( buf );
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


    char buf[128];
    sprintf( buf, "Volume %g", v );
    send_network( buf );

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

    if ( x != CMedia::kUnknownLoop )
    {
        uiMain->uiLoopMode->value( x );
        uiMain->uiLoopMode->label(uiMain->uiLoopMode->child(x)->label());
        uiMain->uiLoopMode->redraw();
    }

    char buf[64];
    sprintf( buf, "Looping %d", x );
    send_network( buf );

}

/**
 * Change the field to display
 *
 * @param p enum 0=Frame, 1=Top, 2=Bottom
 */
void ImageView::field( FieldDisplay p )
{
    _field = p;

    static const char* kFieldNames[] = {
        _("Frame"),
        _("Top"),
        _("Bottom")
    };

    std::string f = kFieldNames[_field];
    f = f.substr( 0, 1 );

    uiMain->uiField->copy_label( f.c_str() );

    char buf[128];
    sprintf( buf, "FieldDisplay %d", _field );
    send_network( buf );

    damage_contents();
}


} // namespace mrv
