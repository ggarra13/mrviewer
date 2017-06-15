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

#include "ImathMath.h" // for Math:: functions


// CORE classes
#include "core/mrvClient.h"
#include "core/mrvColor.h"
#include "core/mrvColorProfile.h"
#include "core/mrvI8N.h"
#include "core/mrvLicensing.h"
#include "core/mrvMath.h"
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
#include "gui/mrvEvents.h"
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
#include "gui/mrvFontsWindowUI.h"
#include "gui/mrvAudioOffset.h"
#include "gui/mrvVersion.h"
#include "gui/mrvLogDisplay.h"
#include "gui/mrvImageView.h"

#undef TRACE
#define TRACE(x) 


// Widgets
#include "mrViewer.h"
#include "mrvIccProfileUI.h"
#include "mrvColorAreaUI.h"

// Video
#include "video/mrvGLEngine.h"  // should be dynamically chosen from prefs

// Audio


using namespace std;

namespace fs = boost::filesystem;

// FLTK2 currently has a problem on Linux with timout's fltk::event_x/y not 
// taking into account other child widgets.  This works around it.
#ifndef _WIN32
#  define FLTK_TIMEOUT_EVENT_BUG
#endif




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
                 ((ext == N_("Z") && ext2 == "Y")) ) 
           return 'b';
       else if ( ext == N_("A") || ext == _("ALPHA") ) return 'a';
       else if ( ext == N_("Z") || ext == N_("Z DEPTH") ) return 'z';
       else return 0;
    }
    else
    {
        oldChannel = channelName;

        std::string ext = channel;
        std::transform( ext.begin(), ext.end(), ext.begin(),
                        (int(*)(int)) toupper );
        if ( ext == N_("LEFT") || ext == N_("RIGHT") )
            return 'c';
        if ( ext.find("RGB") != std::string::npos )
            return 'c';
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

void preload_image_cache_cb( fltk::Widget* o, mrv::ImageView* v )
{
    v->preload_caches();
}

void clear_image_cache_cb( fltk::Widget* o, mrv::ImageView* v )
{
    v->clear_caches();
}

void update_frame_cb( fltk::Widget* o, mrv::ImageView* v )
{
    mrv::media fg = v->foreground();
    if (!fg) return;

    mrv::CMedia* img = fg->image();
    img->update_frame( img->frame() );
    img->cache( img->hires() );
    v->redraw();
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


void set_as_background_cb( fltk::Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( !fg ) return;

  view->background( fg );
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

enum WindowList
{
kReelWindow = 0,
kMediaInfo = 1,
kColorInfo = 2,
k3DStereoOptions = 3,
kEDLEdit = 4,
kPaintTools = 5,
k3dView = 6,
kHistogram = 7,
kVectorscope = 8,
kICCProfiles = 9,
kConnections = 10,
kPreferences = 11,
kHotkeys = 12,
kLogs = 13,
kAbout = 14,
kLastWindow
};

void window_cb( fltk::Widget* o, const mrv::ViewerUI* uiMain )
{
   int idx = -1;
   fltk::Group* g = o->parent();
   for ( int i = 0; i < g->children(); ++i )
   {
      if ( o == g->child(i) ) {
	 idx = i; break;
      }
   }

  if ( idx == kReelWindow )
    {
       // Reel window
      uiMain->uiReelWindow->uiMain->show();
    }
  else if ( idx == kMediaInfo )
  {
       // Media Info
      uiMain->uiImageInfo->uiMain->show();
      uiMain->uiView->update_image_info();
      uiMain->uiView->send_network( "MediaInfoWindow 1" );
  }
  else if ( idx == kColorInfo )
    {
       // Color Area
      uiMain->uiColorArea->uiMain->show();
      uiMain->uiView->update_color_info();
      uiMain->uiView->send_network( "ColorInfoWindow 1" );
    }
  else if ( idx == k3DStereoOptions )
  {
      uiMain->uiStereo->uiMain->child_of( uiMain->uiMain );
      uiMain->uiStereo->uiMain->show();
      uiMain->uiView->send_network( "StereoOptions 1" );
  }
  else if ( idx == kEDLEdit )
  {
     uiMain->uiReelWindow->uiBrowser->set_edl();
     uiMain->uiEDLWindow->uiMain->child_of( uiMain->uiMain );
     uiMain->uiEDLWindow->uiMain->show();
  }
  else if ( idx == k3dView )
  {
      uiMain->uiGL3dView->uiMain->show();
      uiMain->uiView->send_network( "GL3dView 1" );
  }
  else if ( idx == kPaintTools )
    {
       // Paint Tools
      uiMain->uiPaint->uiMain->child_of( uiMain->uiMain );
      uiMain->uiView->send_network( "PaintTools 1" );
      uiMain->uiPaint->uiMain->show();
    }
  else if ( idx == kHistogram )
    {
      uiMain->uiHistogram->uiMain->show();
      uiMain->uiView->send_network( "HistogramWindow 1" );
    }
  else if ( idx == kVectorscope )
    {
      uiMain->uiVectorscope->uiMain->show();
      uiMain->uiView->send_network( "VectorscopeWindow 1" );
    }
  else if ( idx == kICCProfiles )
    {
      uiMain->uiICCProfiles->uiMain->show();
      uiMain->uiICCProfiles->fill();
    }
  else if ( idx == kConnections )
    {
      uiMain->uiConnection->uiMain->child_of( uiMain->uiMain );
      uiMain->uiConnection->uiMain->show();
    }
  else if ( idx == kPreferences )
    {
      uiMain->uiPrefs->uiMain->child_of( uiMain->uiMain );
      uiMain->uiPrefs->uiMain->show();
    }
  else if ( idx == kHotkeys )
    {
      uiMain->uiHotkey->uiMain->child_of( uiMain->uiMain );
      uiMain->uiHotkey->uiMain->show();
    }
  else if ( idx == kLogs )
    {
      uiMain->uiLog->uiMain->child_of( uiMain->uiMain );
      uiMain->uiLog->uiMain->show();
    }
  else if ( idx == kAbout )
    {
      uiMain->uiAbout->uiMain->show();
    }
  else
    {
       const char* name = o->label();
       LOG_ERROR( _("Unknown Window \"") << name << "\"" );
    }
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
static const float kMaxZoom = 32.f;   // Zoom 32x



namespace mrv {

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

  attach_ctl_script( fg->image() );
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
        img->append_look_mod_transform( "ACEScsc.ACES_to_ACEScg" );
        img->append_look_mod_transform( "LMT.SOPNode" );
        img->append_look_mod_transform( "LMT.SatNode" );
        img->append_look_mod_transform( "ACEScsc.ACEScg_to_ACES" );
    }

    if ( ! img->rendering_transform() )
    {
        img->rendering_transform( "RRT" );
    }
    if ( mrv::Preferences::ODT_CTL_transform == "" )
    {
        mrv::Preferences::ODT_CTL_transform =
        "ODT.Academy.RGBmonitor_D60sim_100nits_dim";
    }

    mrv::ViewerUI* main = view->main();
    if ( main->uiSOPNode ) main->uiSOPNode->uiMain->show();
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

  attach_ctl_lmt_script( img, img->number_of_lmts() );
}


static void attach_ctl_idt_script_cb( fltk::Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( ! fg ) return;

  attach_ctl_idt_script( fg->image() );
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

void ImageView::text_mode()
{
   bool ok = mrv::make_window();
   if ( ok )
   {
      _mode = kText;
      uiMain->uiPaint->uiErase->value(false);
      uiMain->uiPaint->uiDraw->value(false);
      uiMain->uiPaint->uiText->value(true);
      uiMain->uiPaint->uiSelection->value(false);
   }
   else
   {
      _mode = kSelection;
      uiMain->uiPaint->uiErase->value(false);
      uiMain->uiPaint->uiDraw->value(false);
      uiMain->uiPaint->uiText->value(false);
      uiMain->uiPaint->uiSelection->value(true);
   }
}

bool ImageView::in_presentation() const
{
    return presentation;
}

void ImageView::send_network( std::string m )
{

    ParserList::iterator i = _clients.begin();
    ParserList::iterator e = _clients.end();
   
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
rotx( 0 ),
roty( 0 ),
posX( 4 ),
posY( 22 ),
flags( 0 ),
_ghost_previous( true ),
_ghost_next( true ),
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
_preframe( 1),
_old_fg_frame( 0 ),
_old_bg_frame( 0 ),
_reel( 0 ),
_idle_callback( false ),
_vr( false ),
_event( 0 ),
_timeout( NULL ),
_old_fg( NULL ),
_fg_reel( -1 ),
_bg_reel( -1 ),
_mode( kSelection ),
_selection( mrv::Rectd(0,0) ),
_playback( CMedia::kStopped ),
_lastFrame( 0 )
{
  _timer.setDesiredSecondsPerFrame(0.0f);

  
  int stereo = 0;
  if ( can_do( fltk::STEREO ) ) stereo = 0; // should be fltk::STEREO

  mode( fltk::RGB24_COLOR | fltk::DOUBLE_BUFFER | fltk::ALPHA_BUFFER |
	fltk::STENCIL_BUFFER | stereo );

  create_timeout( 0.2f );

}


void ImageView::stop_playback()
{

  mrv::media fg = foreground();
  if ( fg ) fg->image()->stop();

  mrv::media bg = background();
  if ( bg ) bg->image()->stop();

}


ImageView::~ImageView()
{
   delete_timeout();

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

   delete _engine; _engine = NULL;

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




bool ImageView::previous_channel()
{
    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return false; // Audio only - no channels

    int c = channel();
    if ( c > 0 )
    {
        channel( c - 1 );
        return true;
    }

    return false;
}

bool ImageView::next_channel()
{
    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    // check if a channel shortcut
    unsigned short num = uiColorChannel->children();
    if ( num == 0 ) return false; // Audio only - no channels

    unsigned short idx = 0;
    fltk::Group* g = NULL;
    for ( unsigned short c = 0; c < num; ++c, ++idx )
        {
            fltk::Widget* w = uiColorChannel->child(c);
            if ( w->is_group() )
            {
                g = (fltk::Group*) w;
                unsigned numc = g->children();
                idx += numc;
            }
        }

    int c = channel();
    if ( c < idx-1 )
    {
        channel( c + 1 );
        return true;
    }

    return false;
}

/** 
 * Initialize OpenGL context, textures, and get opengl features for
 * this gfx card.
 * 
 */
void ImageView::init_draw_engine()
{
  _engine = new mrv::GLEngine( this );
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
  bool outside;
  int xp, yp, w, h;
  picture_coordinates( img, x, y, outside, pic, xp, yp, w, h );

  if ( outside || !pic ) return;

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
  fltk::copy( buf, unsigned( strlen(buf) ), true );
  fltk::copy( buf, unsigned( strlen(buf) ), false );
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

    if ( _showPixelRatio ) H = (int) (H / pixel_ratio());

    double tw = (W / 2.0);
    double th = (H / 2.0);

    x -= (w() - W) / 2.0;
    y += (h() - H) / 2.0;

    y = (this->h() - y - 1);

    x -= tw; y -= th;
    x /= _zoom; y /= _zoom;
    x += tw; y += th;
    x -= xoffset; y -= yoffset;


    y = H - y;
    if ( _showPixelRatio ) y *= pixel_ratio();
}




void ImageView::center_image()
{
    mrv::media fg = foreground();
    if ( !fg ) return;

    Image_ptr img = fg->image();
    mrv::Recti dpw = img->display_window();
    if ( dpw.w() == 0 || !display_window() )
    {
        dpw = img->data_window();

        if ( dpw.w() == 0 )
        {
            dpw.w( img->width() );
            dpw.h( img->height() );
        }
    }

    double pr = 1.0;
    if ( _showPixelRatio ) pr = pixel_ratio();

    yoffset = ( dpw.y() + dpw.h() / 2.0 ) / pr;

    CMedia::StereoOutput stereo_out = img->stereo_output();
    
    if ( img && stereo_out & CMedia::kStereoSideBySide )
    {
        int w = dpw.w();
        xoffset = -w/2.0 + 0.5;
    }
    else if ( img && stereo_out & CMedia::kStereoTopBottom )
    {
        int h = dpw.h();
        yoffset = (( -h/2.0 ) / pr + 0.5 );
    }
    else
    {
        xoffset = -dpw.x() - dpw.w() / 2.0;
    }



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
      if ( stereo_out == CMedia::kStereoRight )
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
      if ( stereo_out == CMedia::kStereoRight )
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

  double W = dpw.w();
  if ( W == 0 ) W = pic->width();
  double H = dpw.h();
  if ( H == 0 ) H = pic->height();

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

  if ( h < z ) { z = h; }

  xoffset = -dpw.x() - W / 2.0;
  if ( (_flip & kFlipVertical) && stereo_out & CMedia::kStereoSideBySide  )
      xoffset = 0.0;

  yoffset = (dpw.y()+H / 2.0) / pr;
  if ( (_flip & kFlipHorizontal) &&
       stereo_out & CMedia::kStereoTopBottom  )
      yoffset = 0.0;

  char buf[128];
  sprintf( buf, "Offset %g %g", xoffset, yoffset );
  send_network( buf );
  zoom( float(z) );

  mouseMove( fltk::event_x(), fltk::event_y() );

  redraw();
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
  bool update = false;


  if ( fg )
  {

      CMedia* img = fg->image();


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

      if ( uiMain->uiGL3dView->uiMain->visible() && 
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
      CMedia* img = bg->image();


      if ( img->image_damage() & CMedia::kDamageContents )
      {
          // resize_background();
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
  }

  if ( update && _playback != CMedia::kStopped ) {
#ifdef FLTK_TIMEOUT_EVENT_BUG
    int y = fltk::event_y();
    if ( uiMain->uiTopBar->visible() ) y -= uiMain->uiTopBar->h();
    mouseMove( fltk::event_x(), y );
#else
    mouseMove( fltk::event_x(), fltk::event_y() );
#endif
  }


  return update;
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

    if ( !fg ) return false;

    CMedia* img = fg->image();
    if (!img) return false;


    if ( !img->is_sequence() || img->has_video() ) {
        _preframe = fg->position() + img->duration();
        img = r->image_at( _preframe );
        if (!img) {
            _reel++;
            _preframe = 1;
        }
        if ( _reel >= b->number_of_reels() )
            return false;
        return true;
    }
        
    int64_t f = r->global_to_local( _preframe );
    int64_t first = img->first_frame();
    int64_t last  = img->last_frame();
    int64_t i = f;
    bool found = false;
        
    // Find a frame to cache from timeline point on
    for ( ; i <= last; ++i )
    {
        if ( !img->is_cache_filled(i) )
        {
            found = true;
            break;
        }
    }
        
    // None found, check backwards
    if ( !found )
    {
        int64_t j = first;
        for ( ; j < f; ++j )
        {
            if ( !img->is_cache_filled(j) )
            {
                i = j; found = true;
                break;
            }
        }
    }

    if ( found )
    {
        boost::recursive_mutex::scoped_lock lk( img->video_mutex() );
        mrv::image_type_ptr pic = img->hires();
        if (!pic) return false;
        img->find_image( i );  // this loads the frame if not present
        img->hires( pic );
        timeline()->redraw();
    }
    else
    {
        if ( CMedia::eight_bit_caches() && gamma() > 1.0 )
            gamma( 1.0 );

        _preframe = fg->position() + img->duration();

        img = r->image_at( _preframe );
        if (!img) {
            _reel++;
            _preframe = 1;
        }
        if ( _reel >= b->number_of_reels() )
            return false;
    }
    

    return true;

}

void ImageView::timeout()
{
    TRACE( "" );
  //
  // If in EDL mode, we check timeline to see if frame points to
  // new image.
  //
    log();

   mrv::Timeline* timeline = this->timeline();
   if  (!timeline) return;

   // Redraw browser to update thumbnail
   mrv::ImageBrowser* b = browser();
   if (!b) return;

   mrv::Reel reel = b->reel_at( _fg_reel );
   mrv::Reel bgreel = b->reel_at( _bg_reel );

   mrv::media fg = foreground();

   int64_t tframe = boost::int64_t( timeline->value() );

   if ( reel && reel->edl )
   {
       TRACE("");
      fg = reel->media_at( tframe );

      if ( fg && fg != foreground() ) 
      {
       TRACE("");
         DBG( "CHANGE TO FG " << fg->image()->name() << " due to frame "
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

#if 0
      if ( img->stopped() && playback() != CMedia::kStopped )
       {
           if ( tframe >= img->first_frame() && tframe <= img->last_frame() )
           {
               img->seek( tframe );
               img->play( playback(), uiMain, false );
           }
       }
#endif
   }

   
   double delay = 0.005;
   if ( fg )
   {
       TRACE("");
      CMedia* img = fg->image();
      delay = 0.5 / img->play_fps();

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
      uiMain->uiEDLWindow->uiEDLGroup->redraw();
  }

  if ( vr() )
  {
#define sumX 0.005
#define sumY 0.005
      if (rotx >= sumX ) rotx -= sumX;
      else if ( rotx <= -sumX ) rotx += sumX;
      else rotx = 0.0;
      
      if (roty >= sumY ) roty -= sumY;
      else if ( roty <= -sumY ) roty += sumY;
      else roty = 0.0;
      
      redraw();
  }
  
  repeat_timeout( float(delay) );
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

void static_preload( mrv::ImageView* v )
{
    v->preload();
}

void ImageView::vr( bool t )
{
    _vr = t;
    valid(0);
    
    const mrv::media& fg = foreground();

    if ( fg )
    {
        CMedia* img = fg->image();
        img->image_damage( img->image_damage() | CMedia::kDamageContents );
    }
    
    redraw();
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
            init_draw_engine();
	}

        if ( !_engine ) return;

        
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

    
    _engine->clear_canvas( r, g, b, a );


    switch( uiPrefs->uiPrefsBlendMode->value() )
    {
        case kBlendTraditional:
        case kBlendTraditionalNonGamma:
            _engine->set_blend_function( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            break;
        case kBlendPremultNonGamma:
        case kBlendPremult:
            _engine->set_blend_function( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
            break;
    }
  }


    const mrv::media& fg = foreground();
  mrv::media bg = background();
  TRACE("");

  ImageList images;
  images.reserve(2);

  if ( _showBG && bg && bg != fg && bg->image()  )
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


  if ( images.empty() ) return;
  TRACE("");
  

  _engine->draw_images( images );
  TRACE("");


  if ( _masking != 0.0f )
    {
      _engine->draw_mask( _masking );
    }

  if ( fg )
  {
      const char* label = fg->image()->label();
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
        _engine->draw_rectangle( _selection, flip() );
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


  CMedia* img = fg->image();

  _engine->draw_annotation( img->shapes() );


  if ( !(flags & kMouseDown) && ( _mode == kDraw || _mode == kErase ) )
  {


     double xf = X;
     double yf = Y;

     data_window_coordinates( img, xf, yf );

     const mrv::Recti& dpw = img->display_window();

     unsigned int H = dpw.h();
     if ( H == 0 ) H = img->height();

     yf = H - yf;
     yf -= H;

     const mrv::Recti& daw = img->data_window();
     xf += daw.x();
     yf -= daw.y();

     _engine->draw_cursor( xf, yf );
  }

  TRACE("");

  if ( _hud == kHudNone )
    return;


  std::ostringstream hud;
  hud.str().reserve( 512 );

  uchar r, g,  b;
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
       y -= yi; hud.str("");
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
       y -= yi; hud.str("");
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

       if ( (p == CMedia::kForwards && _lastFrame < frame) ||
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
  
      if ( img->real_fps() > 0.0 )
      {
         sprintf( buf, _(" UF: %" PRId64 " "), unshown_frames );
         hud << buf;

         sprintf( buf, _("FPS: %.3f" ), img->real_fps() );
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
      uint64_t totalVirtualMem;
      uint64_t virtualMemUsed;
      uint64_t virtualMemUsedByMe;
      uint64_t totalPhysMem;
      uint64_t physMemUsed;
      uint64_t physMemUsedByMe;
      memory_information( totalVirtualMem, virtualMemUsed, virtualMemUsedByMe, 
                          totalPhysMem, physMemUsed, physMemUsedByMe );

      sprintf( buf, _("PMem: %" PRId64 "/%" PRId64 
                      " MB  VMem: %" PRId64 "/%" PRId64 " MB"),
               physMemUsedByMe, totalPhysMem,
               virtualMemUsedByMe, totalVirtualMem );

      draw_text( r, g, b, 5, y, buf );
      y -= yi;
  }

  if ( _hud & kHudIPTC )
    {
      CMedia::Attributes::const_iterator i = img->iptc().begin();
      CMedia::Attributes::const_iterator e = img->iptc().end();
      for ( ; i != e; ++i )
	{
	  std::string text = i->first + ": " + i->second;
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

   

  flags		|= kMouseDown;
	

  int button = fltk::event_button();
  if (button == 1) 
    {
      if (fltk::event_key_state(fltk::LeftAltKey) ||
          vr() )
      {
	 // Handle ALT+LMB moves
	 flags  = kMouseDown;
	 flags |= kMouseMove;
	 flags |= kMouseMiddle;
	 return 1;
      }

      flags |= kMouseLeft;

      if ( _mode == kSelection )
      {
	 _selection = mrv::Rectd( 0, 0, 0, 0 );
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

         unsigned int H = dpw.h();
         if ( H == 0 ) H = img->height();

         yf = H - yf;
         yf -= H;

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
	    
      	    menu.add( _("Image/Next"), kNextImage.hotkey(), 
		      (fltk::Callback*)next_image_cb, browser());
	    menu.add( _("Image/Previous"), kPreviousImage.hotkey(), 
		      (fltk::Callback*)previous_image_cb, 
		      browser(), fltk::MENU_DIVIDER);

	    const stubImage* img = dynamic_cast< const stubImage* >( image() );
	    if ( img )
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
	    menu.add( _("Image/Toggle Background"),
		      kToggleBG.hotkey(),
		      (fltk::Callback*)toggle_background_cb, (void*)this);

	    Image_ptr image = fg->image();

   
	    size_t num = image->number_of_subtitle_streams();

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



	  menu.add( _("Monitor/Attach CTL Display Transform"),
		    kMonitorCTLScript.hotkey(),
		   (fltk::Callback*)monitor_ctl_script_cb,
		   uiMain);
	  menu.add( _("Monitor/Attach ICC Color Profile"),
		    kMonitorIccProfile.hotkey(),
		    (fltk::Callback*)monitor_icc_profile_cb,
		    uiMain, fltk::MENU_DIVIDER);

   
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


    //
    // To represent pixel properly, we need to do the lut
    //
    if ( use_lut() && p == kRGBA_Full )
    {
        Imath::V3f in( rgba.r, rgba.g, rgba.b );
        Imath::V3f out;
        _engine->evaluate( img, in, out );
        rgba.r = out[0];
        rgba.g = out[1];
        rgba.b = out[2];
    }

    //
    // To represent pixel properly, we need to do lut/gain/gamma 
    //
    rgba.r *= _gain;
    rgba.g *= _gain;
    rgba.b *= _gain;

    
    float one_gamma = 1.0f / _gamma;
    // the code below is equivalent to rgba.g = powf(rgba.g, one_gamma);
    // but faster.
    if ( rgba.r > 0.0f && isfinite(rgba.r) )
        rgba.r = expf( logf(rgba.r) * one_gamma );
    if ( rgba.g > 0.0f && isfinite(rgba.g) )
        rgba.g = expf( logf(rgba.g) * one_gamma );
    if ( rgba.b > 0.0f && isfinite(rgba.b) )
        rgba.b = expf( logf(rgba.b) * one_gamma );

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

    if ( xp < dpw.x() || yp < dpw.y() || xp > dpw.w() || yp > dpw.h() ) {
        outside = true;
    }
}

void ImageView::top_bottom( const CMedia* const img,
                            mrv::image_type_ptr& pic, int& xp, int& yp,
                            short& idx, bool& outside, int w, int h ) const
{
    CMedia::StereoOutput output = img->stereo_output();
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
    assert( outside == false );
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
  
  if ( xp < 0 || xp >= (int)pic->width() || yp < 0 || 
       yp >= (int)pic->height() )
  {
      outside = true;
  }
  else if ( input == CMedia::kSeparateLayersInput &&
            ( xp > w-daw[idx].x() || yp > h-daw[idx].y() ) ) {
      outside = true;
  }
  else if ( vr() )
  {
      xp = yp = 0; outside = true;
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
  int xp, yp, w, h;
  picture_coordinates( img, x, y, outside, pic, xp, yp, w, h );
  if ( !pic ) return;
  
  char buf[40];
  sprintf( buf, "%5d, %5d", xp, yp );
  uiMain->uiCoord->text(buf);
  
  CMedia::Pixel rgba;
  if ( outside )
  {
      rgba.r = rgba.g = rgba.b = rgba.a = std::numeric_limits< float >::quiet_NaN();
  }
  else
  {

      rgba = pic->pixel( xp, yp );

      pixel_processed( img, rgba );

      if ( normalize() )
      {
          normalize( rgba );
      }

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
          if ( pic )
          {
              outside = false;
              if ( xp < 0 || xp >= (int)pic->width() || yp < 0 ||
                   yp >= (int)pic->height() )
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
              xf = float(x);
              yf = float(y);
              data_window_coordinates( bgr, xf, yf );
              xp = (int)floor( xf );
              yp = (int)floor( yf );
          }
          else
          {
              double px, py;
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

              xp += daw.x();
              yp += daw.y();
              xp = (int)floor( xp * px );
              yp = (int)floor( yp * py );
          }

          bool outside2 = false;
          if ( xp < 0 || yp < 0 || xp >= (int)w || yp >= (int)h )
          {
              outside2 = true;
          }
          else
          {

              float t = 1.0f - rgba.a;
              CMedia::Pixel bg = picb->pixel( xp, yp );

              pixel_processed( bgr, bg );

              if ( outside )
              {
                  rgba = bg;
              }
              else
              {
                  rgba.r += bg.r * t;
                  rgba.g += bg.g * t;
                  rgba.b += bg.b * t;
              }
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
      hsv = rgba; break;
    case color::kHSV:
      hsv = color::rgb::to_hsv( rgba ); break;
    case color::kHSL:
      hsv = color::rgb::to_hsl( rgba ); break;
    case color::kCIE_XYZ:
      hsv = color::rgb::to_xyz( rgba ); break;
    case color::kCIE_xyY:
      hsv = color::rgb::to_xyY( rgba ); break;
    case color::kCIE_Lab:
      hsv = color::rgb::to_lab( rgba ); break;
    case color::kCIE_Luv:
      hsv = color::rgb::to_luv( rgba ); break;
    case color::kYUV:
      hsv = color::rgb::to_yuv( rgba );   break;
    case color::kYDbDr:
      hsv = color::rgb::to_YDbDr( rgba ); break;
    case color::kYIQ:
      hsv = color::rgb::to_yiq( rgba );   break;
    case color::kITU_601:
      hsv = color::rgb::to_ITU601( rgba );   break;
    case color::kITU_709:
      hsv = color::rgb::to_ITU709( rgba );   break;
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
                float vr_angle = _engine->angle();
                vr_angle -= float(dx) / 10.0f;
                if ( vr_angle >= 90.0f || vr_angle <= 5.0f ) return;

                _engine->angle( vr_angle );
                valid(0);
            }
            else
            {
                zoom( _zoom + float(dx)*_zoom / 500.0f );
            }

            lastX = x;
            lastY = y;
	}
      else if ( flags & kMouseMove )
	{
	   window()->cursor( fltk::CURSOR_MOVE );
           if ( vr() )
           {
#define ROTY_MIN 0.005
#define ROTX_MIN 0.005
#define ROTY_MAX 1.0
#define ROTX_MAX 0.5
               roty += double(dx) / 360.0;
               if ( roty > ROTY_MAX ) roty = ROTY_MAX;
               else if ( roty < -ROTY_MAX ) roty = -ROTY_MAX;
               else if ( std::abs(roty) <= ROTY_MIN ) roty = 0.0;
               rotx += double(dy) / 90.0;
               if ( rotx > ROTX_MAX ) rotx = ROTX_MAX;
               else if ( rotx < -ROTX_MAX ) rotx = -ROTX_MAX;
               else if ( std::abs(rotx) <= ROTX_MIN ) rotx = 0.0;
               
               char buf[128];
               sprintf( buf, "Spin %g %g", rotx, roty );
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
               if ( xf >= W - daw[0].x() )
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
               if ( yf >= H - daw[0].y() )
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

    if ( kOpenImage.match( rawkey ) )
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
    else if ( kToggleLUT.match( rawkey ) )
    {
        toggle_lut();
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
            rotx = 2000.0; // >= 1000.0 resets rotation in mrvGLSphere.cpp
        }
        center_image();
        return 1;
    }
    else if ( kFitScreen.match( rawkey ) ) 
    {
        if ( vr() )
        {
            _engine->angle( 45.0f );
            valid(0);
        }
        fit_image();
        return 1;
    }
    else if ( kSafeAreas.match( rawkey ) )
    {
        safe_areas( safe_areas() ^ true );
        redraw();
        return 1;
    }
    else if ( kDataWindow.match( rawkey ) )
    {
        data_window( data_window() ^ true );
        redraw();
        return 1;
    }
    else if ( kDisplayWindow.match( rawkey ) )
    {
        display_window( display_window() ^ true );
        redraw();
        return 1;
    }
    else if ( kHudToggle.match( rawkey ) )
    {
        hud_toggle_cb( NULL, uiMain );
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

        redraw();
        return 1;
    }
    else if ( kFlipX.match( rawkey ) )
    {
        _flip = (FlipDirection)( (int) _flip ^ (int)kFlipVertical );
        mouseMove( fltk::event_x(), fltk::event_y() );
        redraw();
        return 1;
    }
    else if ( kFlipY.match( rawkey ) )
    {
        _flip = (FlipDirection)( (int) _flip ^ (int)kFlipHorizontal );
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
        return 1;
    }
    else if ( kFrameStepBack.match(rawkey) ) 
    {
        step_frame( -1 );
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
        return 1;
    }
    else if ( kFrameStepFwd.match( rawkey ) ) 
    {
        step_frame( 1 );
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
        return 1;
    }
    else if ( kPlayBack.match( rawkey ) ) 
    {
        if ( playback() == CMedia::kBackwards )
            stop();
        else
            play_backwards();
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
        if ( playback() != CMedia::kStopped )
            stop();
        else
            play_forwards();
        return 1;
    }
    else if ( kPlayFwd.match( rawkey ) ) 
    {
        if ( playback() != CMedia::kStopped )
            stop();
        else
            play_forwards();
        return 1;
    }
    else if ( kStop.match( rawkey ) ) 
    {
        stop();
        return 1;
    }
    else if ( kPreviousImage.match( rawkey ) ) 
    {
        previous_image_cb(this, browser());
        mouseMove( fltk::event_x(), fltk::event_y() );
        return 1;
    }
    else if ( kNextImage.match( rawkey ) ) 
    {
        next_image_cb(this, browser());
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
        return 1;
    }
    else if ( kTogglePresentation.match( rawkey ) )
    {
        toggle_presentation();
        return 1;
    }
    else if ( kSetInPoint.match( rawkey ) )
    {
        int64_t x = uiMain->uiFrame->value();
        uiMain->uiStartButton->value( 1 );
        uiMain->uiStartFrame->value( x );
        uiMain->uiTimeline->minimum( (double)x );
        uiMain->uiTimeline->redraw();
    }
    else if ( kSetOutPoint.match( rawkey ) )
    {
        int64_t x = uiMain->uiFrame->value();
        uiMain->uiEndButton->value( 1 );
        uiMain->uiEndFrame->value( x );
        uiMain->uiTimeline->maximum( (double)x );
        uiMain->uiTimeline->redraw();
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
        if ( _playback == CMedia::kScrubbing ) 
	{
            _playback = CMedia::kStopped;
	}

      flags &= ~kLeftAlt;
      flags &= ~kZoom;
      return 1;
    }
  return 0;
}

void ImageView::show_background( const bool b )
{
   _showBG = b;

   damage_contents();

   char buf[128];
   sprintf( buf, "ShowBG %d", (int) b );
   send_network( buf );
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


  static bool has_image_info, has_color_area, has_reel, has_edl_edit,
  has_prefs, has_about, has_top_bar, has_bottom_bar, has_pixel_bar,
  has_stereo, has_paint;

  if ( !presentation )
    {
      posX = fltk_main()->x();
      posY = fltk_main()->y();

      has_image_info = uiImageInfo->visible();
      has_color_area = uiColorArea->visible();
      has_reel       = uiReel->visible();
      has_edl_edit   = uiEDLWindow->visible();
      has_prefs      = uiPrefs->visible();
      has_about      = uiAbout->visible();
      has_top_bar    = uiMain->uiTopBar->visible();
      has_bottom_bar = uiMain->uiBottomBar->visible();
      has_pixel_bar  = uiMain->uiPixelBar->visible();
      has_paint      = uiPaint->visible();
      has_stereo     = uiStereo->visible();

      uiPaint->hide();
      uiStereo->hide();
      uiImageInfo->hide();
      uiColorArea->hide();
      uiReel->hide();
      uiEDLWindow->hide();
      uiPrefs->hide();
      uiAbout->hide();
      uiMain->uiTopBar->hide();
      uiMain->uiBottomBar->hide();
      uiMain->uiPixelBar->hide();


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

  //  _playback = CMedia::kStopped; // CMedia::kScrubbing;
  uiMain->uiPlayForwards->value(0);
  uiMain->uiPlayBackwards->value(0);

  step_frame( boost::int64_t(dx) );

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
        case mrv::kFULLSCREEN:
        case mrv::kPRESENTATION:
        case mrv::kMEDIA_INFO_WINDOW_SHOW:
        case mrv::kMEDIA_INFO_WINDOW_HIDE:
        case mrv::kCOLOR_AREA_WINDOW_SHOW:
        case mrv::kCOLOR_AREA_WINDOW_HIDE:
        case mrv::k3D_VIEW_WINDOW_SHOW:
        case mrv::k3D_VIEW_WINDOW_HIDE:
        case mrv::kHISTOGRAM_WINDOW_SHOW:
        case mrv::kHISTOGRAM_WINDOW_HIDE:
        case mrv::kVECTORSCOPE_WINDOW_SHOW:
        case mrv::kVECTORSCOPE_WINDOW_HIDE:
        case mrv::kSTEREO_OPTIONS_WINDOW_SHOW:
        case mrv::kSTEREO_OPTIONS_WINDOW_HIDE:
        case mrv::kPAINT_TOOLS_WINDOW_SHOW:
        case mrv::kPAINT_TOOLS_WINDOW_HIDE:
            {
                _event = event;
                return 1;
            }
        case fltk::TIMEOUT:
            {
                unsigned e = _event;
                ParserList c = _clients;
                _clients.clear();
                bool ok = false;
                switch( e )
                {
                    case 0:
                        break;
                    case mrv::kFULLSCREEN:
                        toggle_fullscreen();
                        ok = true; break;
                    case mrv::kPRESENTATION:
                        toggle_presentation();
                        ok = true; break;
                    case mrv::kMEDIA_INFO_WINDOW_SHOW:
                        toggle_media_info(true);
                        ok = true; break;
                    case mrv::kMEDIA_INFO_WINDOW_HIDE:
                        toggle_media_info(false);
                        ok = true; break;
                    case mrv::kCOLOR_AREA_WINDOW_SHOW:
                        toggle_color_area(true);
                        ok = true; break;
                    case mrv::kCOLOR_AREA_WINDOW_HIDE:
                        toggle_color_area(false);
                        ok = true; break;
                    case mrv::k3D_VIEW_WINDOW_SHOW:
                        toggle_3d_view(true);
                        ok = true; break;
                    case mrv::k3D_VIEW_WINDOW_HIDE:
                        toggle_3d_view(false);
                        ok = true; break;
                    case mrv::kHISTOGRAM_WINDOW_SHOW:
                        toggle_histogram(true);
                        ok = true; break;
                    case mrv::kHISTOGRAM_WINDOW_HIDE:
                        toggle_histogram(false);
                        ok = true; break;
                    case mrv::kVECTORSCOPE_WINDOW_SHOW:
                        toggle_vectorscope(true); 
                        ok = true; break;
                    case mrv::kVECTORSCOPE_WINDOW_HIDE:
                        toggle_vectorscope(false);
                        ok = true; break;
                    case mrv::kSTEREO_OPTIONS_WINDOW_SHOW:
                        toggle_stereo_options(true); 
                        ok = true; break;
                    case mrv::kSTEREO_OPTIONS_WINDOW_HIDE:
                        toggle_stereo_options(false);
                        ok = true; break;
                    case mrv::kPAINT_TOOLS_WINDOW_SHOW:
                        toggle_paint_tools(true); 
                        ok = true; break;
                    case mrv::kPAINT_TOOLS_WINDOW_HIDE:
                        toggle_paint_tools(false);
                        ok = true; break;
                    default:
                        LOG_ERROR( "Unknown mrv event" );
                }

                if ( ok ) { _event = 0; }

                TRACE("");
                _clients = c;

                mrv::ImageBrowser* b = browser();
                if ( b && !_idle_callback && CMedia::cache_active() &&
                     CMedia::preload_cache() &&
                     ( _reel < b->number_of_reels() ) )
                {
                    add_idle( (fltk::TimeoutHandler)static_preload, this );
                    _idle_callback = true;
                }
                else
                {
                    if ( _idle_callback && _reel >= b->number_of_reels() )
                    {
                        fltk::remove_idle( (fltk::TimeoutHandler)static_preload, this );
                        _idle_callback = false;
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
            fltk::GlWindow::handle( event );
                TRACE("");
            return 1;
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

            if ( kScrub.match( fltk::event_key() ) )
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
            else
            {
                mouseMove(fltk::event_x(), fltk::event_y());
            }

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
            lastX = fltk::event_x();
            lastY = fltk::event_y();
            return keyDown(fltk::event_key());
        case fltk::KEYUP:
            return keyUp(fltk::event_key());
        case fltk::MOUSEWHEEL:
            {
                if ( vr() )
                {
                    float vr_angle = _engine->angle();
                    vr_angle += fltk::event_dy();
                    if ( vr_angle >= 90.0f || vr_angle <= 5.0f ) return 1;
                    _engine->angle( vr_angle );
                    valid(0);  // to reset matrix
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

    reset_caches();
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
            img->fetch(frame());
        }
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
        fltk::remove_idle( (fltk::TimeoutHandler) static_preload, this );
        _idle_callback = false;
    }
    else
    {
        if (!_idle_callback) 
        {
            fltk::add_idle( (fltk::TimeoutHandler) static_preload, this );
            _idle_callback = true;
        }
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
            if ( w->is_group() ||
                 strcmp( lbl, _("Color") ) == 0 )
                _old_channel = idx;
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
        LOG_ERROR( _("Label not found for index ") << c );
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
      LOG_ERROR( _("Invalid index ") << c << _(" for channel.  Maximum: " )
                 << idx );
      return;
  }

  // If user selected the same channel again, toggle it with
  // other channel (diffuse.r goes to diffuse, for example)
  const mrv::media& fg = foreground();
  if ( c == _channel && fg && _old_fg == fg->image() ) {
      c = _old_channel;
      _old_channel = _channel;
  }
  else
  {
      if ( fg && _old_fg != fg->image() )
      {
          _old_channel = c;
      }
  }

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

  if ( pos != std::string::npos && pos != ext.size() )
  {
     ext = ext.substr( pos+1, ext.size() );
  }

  if ( pos2 != std::string::npos && pos2 != oext.size() )
  {
     oext = oext.substr( pos2+1, oext.size() );
  }

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


  if ( pos != pos2 && channelName.size() > oldChannel.size() )
  {
     pos2 = channelName.find( oldChannel );
     if ( pos2 == std::string::npos ) {
        ext = "";
     } 
  }
  else
  {
     if ( pos != pos2 )
     {
	pos2 = oldChannel.find( channelName );
	if ( pos2 == std::string::npos ) {
           oext = "";
        }
     }
     else
     {
        std::string temp1 = channelName.substr( 0, pos );
        std::string temp2 = oldChannel.substr( 0, pos2 );
        if ( temp1 != temp2 || ( oext != "R" && oext != "G" &&
                                 oext != "B" && oext != "A") ) {
           ext = "";
        }
     }
  }

  mrv::media bg = background();

  {
      if ( fg ) fg->image()->channel( lbl );
      if ( bg ) bg->image()->channel( lbl );
  }

  update_image_info();
  update_shortcuts( fg, channelName.c_str() );

  oldChannel = channelName;
  free( lbl );

  // When changing channels, the cache may get thrown away.  Reflect that
  // in the timeline.
  timeline()->redraw();

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
     fg->image()->gamma( f );
  }

  char buf[256];
  sprintf( buf, "Gamma %g", f );
  send_network( buf );

  uiMain->uiGamma->value( f );
  uiMain->uiGammaInput->value( f );

  flush_caches();
  smart_refresh();
  update_color_info();
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
      sprintf( tmp, "1/%.2g", 1/z );
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

  // Chack if image has an F Stop or Aperture EXIF attribute
  mrv::media fg = foreground();
  if ( fg )
    {
      CMedia* img = fg->image();

      const char* fnumber = img->exif( "F Number" );
      if ( fnumber == NULL )
	fnumber = img->exif( "Aperture Value" );

      if ( fnumber )
	{
	  stringArray tokens;
	  mrv::split_string( tokens, fnumber, "/" );
	  if ( tokens.size() == 2 )
	    {
	      int num = atoi( tokens[0].c_str() );
	      int den = atoi( tokens[1].c_str() );
	      float exp = (float) num / (float) den;

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
    boost::recursive_mutex::scoped_lock lk( _shortcut_mutex );
    
    CMedia* img = fg->image();

    fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
    uiColorChannel->clear();
    
    const stringArray& layers = img->layers();

    stringArray::const_iterator i = layers.begin();
    stringArray::const_iterator e = layers.end();

    
    int v   = -1;
    int idx = 0;
    std::set< short > shortcuts;
    
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
            if ( root == _("Red") || root == _("Green") ||
                 root == _("Blue") || root == _("Alpha") ||
                 root == _("Alpha Overlay") ||
                 root == _("Lumma") ||
                 root == _("Z") )
                root = _("Color");
        }
    }

    bool group = false;
    std::string x;
    fltk::Group* g = NULL;
    fltk::Widget* o = NULL;

    for ( ; i != e; ++i, ++idx )
    {

        const std::string& name = *i;

        if ( o && x != _("Alpha") && name.find(x + '.') == 0 )
        {
            if ( group )
            {
                // Copy shortcut to group and replace leaf with group
                unsigned last = uiColorChannel->children()-1;
                fltk::Widget* w = uiColorChannel->child(last);
                unsigned s = w->shortcut();
                uiColorChannel->remove( w );
                g = uiColorChannel->add_group( x.c_str(), NULL );
                g->shortcut( s );
                group = false;
            }

            // Now add current leaf, but without # prefix and period
            std::string y = name;

            if ( x.size() != name.size() && x.size() < name.size() )
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
        if ( v == -1 && ( x == root || (channelName && name == channelName) ) )
        {
            v = idx;
        }

        // Get a shortcut to this layer
        short shortcut = get_shortcut( name.c_str() );

        if ( v >= 0 || shortcut == 'n' || shortcut == 'z' )
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

    if ( v >= 0 )
    {
        channel( (unsigned short) v );
    }


    uiColorChannel->redraw();

    CMedia* img = fg->image();

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
    

    
    CMedia* img = NULL;
    if (old)
    {
        CMedia* img = old->image();
        img->close_audio();
    }
    if ( fg ) 
    {
        img = fg->image();
      
        char bufs[256];  bufs[0] = 0;

        mrv::media bg = background();
        if ( bg )
        {
            sprintf( bufs, _("mrViewer    FG: %s   BG: %s"), 
                     img->name().c_str(),
                     bg->image()->name().c_str() );
        }
        else
        {
            sprintf( bufs, _("mrViewer    FG: %s"), 
                     img->name().c_str() );
        }
        uiMain->uiMain->copy_label( bufs );

        double fps = img->fps();
        if ( img->is_sequence() &&
             uiMain->uiPrefs->uiPrefsOverrideFPS->value() )
        {
            fps = uiMain->uiPrefs->uiPrefsFPS->value();
            LOG_INFO( _("Override timeline fps to ") << fps );
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
        uiMain->uiEndFrame->fps( fps );
        uiMain->uiEndFrame->timecode( tc );
        uiMain->uiFPS->value( img->play_fps() );
        
        if ( uiMain->uiPrefs->uiPrefsOverrideAudio->value() )
        {
            img->volume( _volume );

            CMedia* right = img->right_eye();
            if ( right ) right->volume( _volume );
        }       else
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
                fit_image();

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

    update_image_info();
    update_color_info( fg );

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
  mrv::media fg = foreground();

  CMedia* img = NULL;
  if (old && old != fg)
  {
      CMedia* img = old->image();
      img->close_audio();
  }
    
  _bg = bg;
  if ( bg ) 
    {
      img = bg->image();

      if ( fg )
      {
          sprintf( buf, _("mrViewer    FG: %s   BG: %s"), 
                   fg->image()->name().c_str(),
                   img->name().c_str() );
          uiMain->uiMain->copy_label( buf );
      }

      sprintf( buf, "CurrentBGImage \"%s\" %" PRId64 " %" PRId64, 
               img->fileroot(), img->first_frame(), img->last_frame() );
      send_network( buf );


      img->refresh();

      if ( img->is_sequence() )
          img->play_fps( fps() );
      
      img->image_damage( img->image_damage() | CMedia::kDamageContents );      

      if ( dynamic_cast< stubImage* >( img ) )
	{
	  create_timeout( 0.2 );
	}
    }
  else
  {
      if ( fg )
      {
          sprintf( buf, _("mrViewer    FG: %s"), fg->image()->name().c_str() );
          uiMain->uiMain->copy_label( buf );
      }
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
      w = 640; h = 480;
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

  if ( w > maxw ) { fit = true; w = maxw; }
  if ( h > maxh ) { fit = true; h = maxh; }

  if ( posX + w > maxx ) { posX = ( w + posX - maxw ) / 2; }
  if ( posX + w > maxx ) { posX = minx; w = maxw; }
  if ( posX < minx )     posX = minx;

  if ( posY + h > maxy ) { posY = ( h + posY - maxh ) / 2; }
  if ( posY + h > maxy ) { posY = miny; h = maxh; }
  if ( posY < miny )     posY = miny;
  
  if ( w < 640 )  w = 640;
  if ( h < 550 )  h = 550;

  //  std::cerr << "posY " << posY << " " << h << std::endl;
  //  std::cerr << "minY " << miny << " " << maxh << std::endl;
  
  fltk_main()->fullscreen_off( posX, posY, w, h );
#ifdef LINUX
  fltk_main()->hide();  // needed to show decorations under some window managers
  fltk_main()->resize( posX, posY, w, h );
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
      img->image_damage( img->image_damage() | CMedia::kDamageContents );
    }

  mrv::media bg = background();
  if (bg)
    {
      CMedia* img = bg->image();
      img->image_damage( img->image_damage() | CMedia::kDamageContents );
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
   redraw();
}


/** 
 * Toggle 3D LUT
 * 
 */
void ImageView::toggle_lut()
{
  _useLUT = !_useLUT;

  main()->uiLUT->value( _useLUT );

  char buf[128];
  sprintf( buf, "UseLUT %d", (int)_useLUT );
  send_network( buf );

  flush_caches();
  if ( _useLUT ) {
     if ( _engine )
     {
         _engine->refresh_luts();
     }
  }


  redraw();  // force a draw to refresh luts

  uiMain->uiLUT->value( _useLUT );

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
    if ( uiMain && uiMain->uiFrame )
        uiMain->uiFrame->frame(f);
    
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
    _preframe = f;

    if ( std::abs( f - frame() ) < fps() / 2.0 )
    {
        stop();
    }

    mrv::ImageBrowser* b = browser();
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

  if ( uiMain->uiVectorscope )
    {
      fltk::Window*  uiVectorscope = uiMain->uiVectorscope->uiMain;
      if ( uiVectorscope->visible() ) uiVectorscope->redraw();
    }

  if ( uiMain->uiHistogram )
    {
      fltk::Window*  uiHistogram   = uiMain->uiHistogram->uiMain;
      if ( uiHistogram->visible() ) uiHistogram->redraw();
    }
}

void ImageView::update_color_info() const
{
  mrv::media fg = uiMain->uiView->foreground();
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

  CMedia* img = fg->image();
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


   playback( dir );

   delete_timeout();

   double fps = uiMain->uiFPS->value();
   create_timeout( 0.5/fps );

   mrv::media fg = foreground();
   if ( fg )
   {
      DBG( "******* PLAY FG " << fg->image()->name() );
      fg->image()->play( dir, uiMain, true );
   }


   mrv::media bg = background();
   if ( bg && bg != fg ) 
   {
      DBG( "******* PLAY BG " << bg->image()->name() );
      bg->image()->play( dir, uiMain, false);
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
    if ( playback() != CMedia::kStopped &&
         playback() != CMedia::kScrubbing ) return;


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
    if ( playback() == CMedia::kStopped ) return;

    _playback = CMedia::kStopped;
    _last_fps = 0.0;
    _real_fps = 0.0;

    stop_playback();

    send_network( "stop" );

    if ( uiMain->uiPlayForwards )
        uiMain->uiPlayForwards->value(0);

    if ( uiMain->uiPlayBackwards )
        uiMain->uiPlayBackwards->value(0);


    seek( int64_t(timeline()->value()) );

    redraw();

    thumbnails();

}



double ImageView::fps() const
{
  mrv::media fg = foreground();
  if ( fg ) return fg->image()->play_fps();

  mrv::media bg = foreground();
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
  redraw();
}


} // namespace mrv

