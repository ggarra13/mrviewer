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


#define __STDC_FORMAT_MACROS
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

#include <GL/gl.h>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include <fltk/visual.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/draw.h>
#include <fltk/run.h>
#ifdef LINUX
#include <fltk/x11.h>
#endif

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


#include "ImathMath.h" // for Math:: functions


#include "core/CMedia.h"
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
#include "gui/mrvFontsWindowUI.h"
#include "gui/mrvImageView.h"


// Widgets
#include "mrViewer.h"
#include "mrvIccProfileUI.h"
#include "mrvColorAreaUI.h"

// Video
#include "video/mrvGLEngine.h"  // should be dynamically chosen from prefs

// Audio


using namespace std;

namespace fs = boost::filesystem;

// FLTK2 currently has a problem with timout's fltk::event_x/y not 
// taking into account other child widgets.  This works around it.
#define FLTK_TIMEOUT_EVENT_BUG





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
  { _("left"),  'c'         },
  { _("right"), 'c'         },
  { _("RGBA"),  'c'         },
  { _("RGB"),   'c'         },
  { _("Red"),   'r'         },
  { _("Green"), 'g'         },
  { _("Blue"),  'b'         },
  { _("Alpha"), 'a'         },
  { _("Alpha Overlay"), 'o' },
  { _("Lumma"), 'l' },
  { _("Z"), 'z' },
  { _("Z Depth"), 'z' },
  { _("N"), 'n' },
  { _("Normals"), 'n' },
  { _("Tag"), 't' },
  { _("UV"), 'u' },
  { _("ST"), 'u' },
};




namespace
{
  /** 
   * Get a shortcut for a channel by looking at the channel mappings
   * 
   * @param channel name of the channel (Color, Z Depth, etc)
   * 
   * @return a short value representing the fltk shortcut.
   */
  short get_shortcut( const char* channel )
  {
    for ( unsigned int i = 0; i < sizeof(shortcuts)/sizeof(ChannelShortcuts); ++i )
      {
	if ( strcmp( shortcuts[i].channel, channel ) == 0 )
	  return shortcuts[i].key;
      }

    static std::string oldChannel;
    std::string channelName = channel;
    size_t pos  = channelName.rfind( '.' );
    size_t pos2;
    if ( channelName.size() > oldChannel.size() )
    {
       pos2 = channelName.find( oldChannel );
       if ( pos2 == std::string::npos ) return 0; 
    }
    else
    {
       pos2 = oldChannel.find( channelName );
       if ( pos2 == std::string::npos ) return 0; 
    }

    if ( pos != std::string::npos )
    {
       std::string ext = channelName.substr( pos+1, channelName.size() );
       std::transform( ext.begin(), ext.end(), ext.begin(),
		       (int(*)(int)) toupper );
       if ( ext == N_("COLOR") || ext == N_("RGB") || ext == N_("RGBA")) 
          return 'c';
       if ( ext == N_("R") || ext == N_("RED")   ) return 'r';
       if ( ext == N_("G") || ext == N_("GREEN") ) return 'g';
       if ( ext == N_("B") || ext == N_("BLUE")  ) return 'b';
       if ( ext == N_("A") || ext == N_("ALPHA") ) return 'a';
       if ( ext == N_("Z") || ext == N_("Z DEPTH") ) return 'z';
    }

    oldChannel = channelName;

    return 0;
  }


}

extern void clone_all_cb( fltk::Widget* o, mrv::ImageBrowser* b );
extern void clone_image_cb( fltk::Widget* o, mrv::ImageBrowser* b );

void clear_image_cache_cb( fltk::Widget* o, mrv::ImageView* v )
{
    mrv::media m = v->foreground();
    if ( m )
    {
        mrv::CMedia* img = m->image();
        img->clear_cache();
    }

    m = v->background();
    if ( m )
    {
        mrv::CMedia* img = m->image();
        img->clear_cache();
    }

    v->timeline()->redraw();
}

void next_image_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
  b->next_image();
}

void previous_image_cb( fltk::Widget* o, mrv::ImageBrowser* b )
{
  b->previous_image();
}


void set_as_background_cb( fltk::Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( !fg ) return;

  mrv::CMedia* img = fg->image();

  view->background( fg );
}

void toggle_background_cb( fltk::Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( !fg ) return;
  view->toggle_background();
}

void open_cb( fltk::Widget* o, mrv::ImageBrowser* uiReelWindow )
{
  uiReelWindow->open();
}

void open_single_cb( fltk::Widget* o, mrv::ImageBrowser* uiReelWindow )
{
  uiReelWindow->open_single();
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
kEDLEdit = 3,
kPaintTools = 4,
k3dView = 5,
kHistogram = 6,
kVectorscope = 7,
kICCProfiles = 8,
kConnections = 9,
kPreferences = 10,
kHotkeys = 11,
kLogs = 12,
kAbout = 13,
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
      uiMain->uiView->send( "MediaInfoWindow 1" );
  }
  else if ( idx == kColorInfo )
    {
       // Color Area
      uiMain->uiColorArea->uiMain->show();
      uiMain->uiView->update_color_info();
      uiMain->uiView->send( "ColorInfoWindow 1" );
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
      uiMain->uiView->send( "GL3dView 1" );
  }
  else if ( idx == kPaintTools )
    {
       // Paint Tools
      uiMain->uiPaint->uiMain->child_of( uiMain->uiMain );
      uiMain->uiPaint->uiMain->show();
    }
  else if ( idx == kHistogram )
    {
      uiMain->uiHistogram->uiMain->show();
      uiMain->uiView->send( "HistogramWindow 1" );
    }
  else if ( idx == kVectorscope )
    {
      uiMain->uiVectorscope->uiMain->show();
      uiMain->uiView->send( "VectorscopeWindow 1" );
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

  sscanf( fmt, "%f", &mask );

  char buf[128];
  sprintf( buf, "Mask %f", mask );
  view->send( buf );

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

   int i = (int)p->value() - 1;
   fg->image()->subtitle_stream(i);

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

static const float kMinZoom = 0.1f;
static const float kMaxZoom = 32.f;



namespace mrv {


static void attach_color_profile_cb( fltk::Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( ! fg ) return;

  attach_icc_profile( fg->image() );
}


static void attach_ctl_script_cb( fltk::Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( ! fg ) return;

  attach_ctl_script( fg->image() );
}



static void monitor_icc_profile_cb( fltk::Widget* o, void* data )
{
  monitor_icc_profile();
}


static void monitor_ctl_script_cb( fltk::Widget* o, void* data )
{
  monitor_ctl_script();
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

  img->audio_file( file );
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


void ImageView::send( std::string m )
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
  uiMain( NULL ),
  _engine( NULL ),
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
  posX( 4 ),
  posY( 22 ),
  flags( 0 ),
  _ghost_previous( true ),
  _ghost_next( true ),
  _channel( 0 ),
  _old_channel( 0 ),
  _channelType( kRGB ),
  _field( kFrameDisplay ),
  _stereo( CMedia::kNoStereo ),
  _displayWindow( true ),
  _dataWindow( true ),
  _showBG( true ),
  _showPixelRatio( false ),
  _useLUT( false ),
  _volume( 1.0f ),
  _flip( kFlipNone ),
  _timeout( NULL ),
  _fg_reel( -1 ),
  _bg_reel( -1 ),
  _mode( kSelection ),
  _selection( mrv::Rectd(0,0) ),
  _playback( kStopped ),
  _looping( kLooping ),
  _lastFrame( 0 )
{
  _timer.setDesiredSecondsPerFrame(0.0f);

  int stereo = 0; // fltk::STEREO;
  if ( !can_do( fltk::STEREO ) ) stereo = 0;

  mode( fltk::RGB24_COLOR | fltk::DOUBLE_BUFFER | fltk::ALPHA_BUFFER |
	fltk::STENCIL_BUFFER | stereo );

}


void ImageView::stop_playback()
{

  mrv::media fg = foreground();
  if ( fg && !fg->image()->stopped()) fg->image()->stop();

  mrv::media bg = background();
  if ( bg && !bg->image()->stopped()) bg->image()->stop();

}


ImageView::~ImageView()
{
   ParserList::iterator i = _clients.begin();
   ParserList::iterator e = _clients.end();
   for ( ; i != e; ++i )
   {
       (*i)->connected = false;
   }

   _clients.clear();

   // make sure to stop any playback
   stop_playback();

   delete_timeout();
   delete _engine; _engine = NULL;
}

fltk::Window* ImageView::fltk_main()
{ 
   assert( uiMain->uiMain );
   return uiMain->uiMain; 
}

const fltk::Window* ImageView::fltk_main() const
{ 
   assert( uiMain->uiMain );
   return uiMain->uiMain; 
}


ImageBrowser* 
ImageView::browser() {
   assert( uiMain->uiReelWindow );
   assert( uiMain->uiReelWindow->uiBrowser );
   return uiMain->uiReelWindow->uiBrowser;
}


Timeline* 
ImageView::timeline() { 
   assert( uiMain->uiTimeline );
   return uiMain->uiTimeline;
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

  CMedia::supports_yuv( _engine->supports_yuv() );
}



void ImageView::fg_reel(int idx) 
{
    _fg_reel = idx;

    char buf[128];
    sprintf( buf, "FGReel %d", idx );
    send( buf );
}

void ImageView::bg_reel(int idx)
{
    _bg_reel = idx;

    char buf[128];
    sprintf( buf, "BGReel %d", idx );
    send( buf );
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

  double x = double(lastX);
  double y = double(lastY);

  if ( x < 0 || y < 0 || x >= this->w() || y >= this->h() )
    return;


  double xf = (double) x;
  double yf = (double) y;
  data_window_coordinates( img, xf, yf );

  mrv::image_type_ptr pic = img->hires();

  if ( _stereo == CMedia::kStereoCrossed ) pic = img->right();

  if ( !pic ) return;

  mrv::Recti daw = img->data_window();
  mrv::Recti dpw = img->display_window();
  unsigned w = dpw.w();
  unsigned h = dpw.h();
  if ( w == 0 ) w = pic->width();
  if ( h == 0 ) h = pic->height();


  bool outside = false;

  if ( img->is_stereo() && _stereo & CMedia::kStereoSideBySide )
  {
      if ( x < 0 || y < 0 || x >= this->w() || y >= this->h() ||
           xf < 0 || yf < 0 || xf >= w*2 || yf >= h )
      {
          outside = true;
      }
  }
  else
  {
      if ( x < 0 || y < 0 || x >= this->w() || y >= this->h() ||
           xf < 0 || yf < 0 || xf >= w || yf >= h )
      {
          outside = true;
      }
  }

  unsigned xp = (unsigned)floor(xf);
  unsigned yp = (unsigned)floor(yf);

  if ( xp > w && _stereo & CMedia::kStereoSideBySide )
  {
      if ( _stereo == CMedia::kStereoCrossed ) pic = img->left();
      else pic = img->right();
      xp -= w;
  }


  if ( xp >= pic->width() || yp >= pic->height() ) {
      outside = true;
  }

  if ( outside ) return;

  CMedia::Pixel rgba = pic->pixel( xp, yp );

  char buf[256];
  sprintf( buf, "%g %g %g %g", rgba.r, rgba.g, rgba.b, rgba.a );

  // Copy text to both the clipboard and to X's XA_PRIMARY
  fltk::copy( buf, unsigned( strlen(buf) ), true );
  fltk::copy( buf, unsigned( strlen(buf) ), false );
}


void ImageView::data_window_coordinates( const Image_ptr img,
                                         double& x, double& y ) const
{
    image_coordinates( img, x, y );

    const mrv::Recti& dpw = img->display_window();
    unsigned W = dpw.w();
    if ( W == 0 ) W = img->width();
    unsigned H = dpw.h();
    if ( H == 0 ) H = img->height();
    x -= W/2.0; y -= H/2.0;


    const mrv::Recti& daw = img->data_window();
    x -= daw.x();
    y -= daw.y();
}

/** 
 * Given window's x and y coordinates, return an image's
 * corresponding x and y coordinate
 * 
 * @param img image to find coordinates for
 * @param x   window's x position
 * @param y   window's y position
 */
void ImageView::image_coordinates( const Image_ptr img, 
				   double& x, double& y ) const
{
    const mrv::Recti& dpw = img->display_window();

    double W = dpw.w();
    if ( W == 0 ) W = img->width();
    double H = dpw.h();
    if ( H == 0 ) H = img->height();

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

    if ( img && _stereo & CMedia::kStereoSideBySide )
    {
        int w = dpw.w();
        xoffset = (float)-w/2.0f + 0.5f;
    }
    else
    {
        xoffset = -dpw.x() - dpw.w() / 2.0;
    }

    yoffset = dpw.y() + dpw.h() / 2.0;

    char buf[128];
    sprintf( buf, N_("Offset %g %g"), xoffset, yoffset );
    send( buf );
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

  mrv::image_type_ptr pic = img->hires();
  if ( !pic ) return;

  mrv::Recti dpw;
  if ( display_window() )
  {
      dpw = img->display_window();
      if ( _stereo & CMedia::kStereoSideBySide )
      {
          dpw.merge( img->display_window2() );
      }
  }
  else
  {
      dpw = img->data_window();
      if ( _stereo & CMedia::kStereoSideBySide )
      {
          const mrv::Recti& dp = img->display_window();
          mrv::Recti daw = img->data_window2();
          daw.x( dp.w() + daw.x() );
          dpw.merge( daw );
      }
  }

  double W = dpw.w();
  if ( W == 0 ) W = pic->width();
  double H = dpw.h();
  if ( H == 0 ) H = pic->height();

  if ( display_window() && _stereo & CMedia::kStereoSideBySide )
      W *= 2;

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
  if ( _showPixelRatio ) h *= pixel_ratio();
  if ( h < z ) { z = h; }

  xoffset = -dpw.x()-W / 2.0;
  yoffset = (dpw.y()+H / 2.0) / pixel_ratio();

  char buf[128];
  sprintf( buf, "Offset %g %g", xoffset, yoffset );
  send( buf );
  zoom( float(z) );

  redraw();
}





/** 
 * Check foreground and background images for updates.
 * 
 * @return true if view needs redrawing, false if not.
 */
bool ImageView::should_update( mrv::media& fg )
{
  bool update = false;


  bool reload = (bool) uiMain->uiPrefs->uiPrefsAutoReload->value();
  if ( reload && fg )
    {
      CMedia* img = fg->image();

      bool video = img->has_video();

      bool check = false;
      if ( video && img->stopped() ) check = true;
      else if ( !video )             check = true;

      if ( check && img->has_changed() )
	{
	  char* root = strdup( img->fileroot() );
	  fg = browser()->replace( img->filename(), root );
	  foreground( fg );
	  free( root );
	}
    }


  if ( fg )
    {

      CMedia* img = fg->image();
      if ( img->image_damage() & CMedia::kDamageLayers )
	{
	  update_layers();
	}

      if ( img->image_damage() & CMedia::kDamageContents )
	{
	  update = true;
	}

      if ( img->image_damage() & CMedia::kDamageThumbnail )
	{
	  // Redraw browser to update thumbnail
	  browser()->redraw();
	}

      if ( img->image_damage() & CMedia::kDamageData )
	{
	  update_image_info();
	  uiMain->uiICCProfiles->fill();
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
  if ( bg && bg != fg )
    {
      CMedia* img = bg->image();

      if ( reload && img->has_picture() )
	{
	  bool video = img->has_video();

	  bool check = false;
	  if ( video && img->stopped() ) check = true;
	  else if ( !video )             check = true;

	  if ( check && img->has_changed() )
	    {
	      char* root = strdup( img->fileroot() );
	      bg = browser()->replace( img->filename(), root );
	      background( bg );
	      free( root );
	    }
	}


      if ( bg )  // check as image may have changed
	{
	  CMedia* img = bg->image();

	  if ( img->image_damage() & CMedia::kDamageContents )
	    {
	       // resize_background();
	      update = true;
	    }

	  if ( img->image_damage() & CMedia::kDamageThumbnail )
	    {
	      // Redraw browser to update thumbnail
	      browser()->redraw();
	    }
	}
    }


  if ( update && _playback != kStopped ) {
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


void ImageView::preload( const mrv::Reel& reel, const mrv::media& fg,
                         const int64_t tframe )
{
    if ( !reel || !fg ) return;

    int64_t f = reel->global_to_local( tframe );
    CMedia* img = fg->image();

    if ( !img->is_sequence() ) return;

    
    int64_t first = img->first_frame();
    int64_t last  = img->last_frame();
    int64_t i = f;
    bool found = false;

    // Find a frame to cache from timeline point on
    for ( ; i != last; ++i )
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
        img->dts( i );
        mrv::image_type_ptr pic = img->hires();
        img->find_image( i );  // this loads the frame if not present
        img->cache( img->hires() );
        if (pic) img->hires( pic );
        timeline()->redraw();
    }

}

void ImageView::timeout()
{

  //
  // If in EDL mode, we check timeline to see if frame points to
  // new image.
  //
   char bufs[256];  bufs[0] = 0;

   mrv::Timeline* timeline = this->timeline();
   if  (!timeline) return;

   mrv::Reel reel = browser()->reel_at( _fg_reel );
   mrv::Reel bgreel = browser()->reel_at( _bg_reel );

   mrv::media fg = foreground();

   int64_t tframe = boost::int64_t( timeline->value() );

   if ( reel && reel->edl )
   {
      
      fg = reel->media_at( tframe );

      if ( fg && fg != foreground() ) 
      {
         DBG( "CHANGE TO FG " << fg->image()->name() << " due to frame "
              << tframe );
         foreground( fg );

         fit_image();

         sprintf( bufs, "mrViewer    FG: %s", 
                  fg->image()->name().c_str() );
         uiMain->uiMain->copy_label( bufs );
      }
      
   }

   mrv::media bg = background();

   if ( bgreel && bgreel->edl )
   {
      bg = bgreel->media_at( tframe );

      if ( bg && bg != background() ) 
      {
         DBG( "CHANGE TO BG " << bg->image()->name() );
         background( bg );

         sprintf( bufs, "mrViewer    FG: %s   BG: %s", 
                  fg->image()->name().c_str(),
                  bg->image()->name().c_str() );
         uiMain->uiMain->copy_label( bufs );
      }

   }


   //
   // If playback is stopped, try to cache forward images
   //
   preload( reel, fg, tframe );

   static double kMinDelay = 0.0001666;

   double delay = 0.005;
   if ( fg )
   {
      CMedia* img = fg->image();
      delay = 1.0 / (img->play_fps() * 2.0);
      if ( delay < kMinDelay ) delay = kMinDelay;
   }

  repeat_timeout( float(delay) );

  if ( timeline && timeline->visible() ) 
  {
     
     int64_t frame;
     if ( reel && !reel->edl && fg )
     {
	CMedia* img = fg->image();


        frame = img->frame();

        // if ( playback() == kForwards )
        // {
        //     if ( img->audio_frame() > frame )
        //         frame = img->audio_frame();
        // }
        // else
        // {
        //     if ( img->audio_frame() < frame )
        //         frame = img->audio_frame();
        // }


	if ( this->frame() != frame )
	{
	   this->frame( frame );
	}
     }
  }

  if ( fg && should_update( fg ) )
    {
      update_color_info( fg );

      uiMain->uiEDLWindow->uiEDLGroup->redraw();
      redraw();
    }
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

        send( "RedoDraw" );
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
        send( "UndoDraw" );
        redraw();
    }

}

void ImageView::draw_text( unsigned char r, unsigned char g, unsigned char b,
			   double x, double y, const char* text )
{
   _engine->color( (uchar)0, (uchar)0, (uchar)0 );
   _engine->draw_text( int(x+1), int(y-1), text ); // draw shadow
   _engine->color( r, g, b );
   _engine->draw_text( int(x), int(y), text );  // draw text
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
    if ( fltk_main()->border() ) 
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
	  _engine->set_blend_function( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	  break;
       case kBlendPremult:
	  _engine->set_blend_function( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	  break;
    }
  }


  mrv::media bg = background();
  mrv::media fg = foreground();


  ImageList images;
  images.reserve(2);

  if ( _showBG && bg && bg != fg && bg->image()  )
    {
       if ( bg->image()->has_picture() )
	  images.push_back( bg->image() );
    }

  if ( fg && fg->image() )
    {
       CMedia* img = fg->image();
       if ( img->has_picture() )
       {
	  images.push_back( img );
          _stereo = img->stereo_type();
       }
    }
  else
  {
     return;
  }


  if ( images.empty() ) return;


  _engine->draw_images( images );


  if ( _masking != 0.0f )
    {
      _engine->draw_mask( _masking );
    }

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

  if ( _selection.w() > 0 || _selection.h() > 0 )
    {
        uchar r, g,  b;
        fltk::split_color( uiPrefs->uiPrefsViewSelection->color(), r, g, b );
        _engine->color( r, g, b, 255 );
        _engine->draw_rectangle( _selection );
    }



  if ( _safeAreas ) 
    {
      const CMedia* img = fg->image();
      const mrv::Recti& dpw = img->display_window();
      unsigned W = dpw.w();
      unsigned H = dpw.h();
      if ( W == 0 )
      {
          mrv::image_type_ptr pic = img->hires();
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

  if ( _hud == kHudNone )
    return;

  // _engine->end_fbo( images );

  std::ostringstream hud;
  hud.str().reserve( 512 );

  uchar r, g,  b;
  fltk::split_color( uiPrefs->uiPrefsViewHud->color(), r, g, b );
  _engine->color( r, g, b );


  //
  // Draw filename
  //

  int y = h()-25;
  int yi = 25;
  static char buf[1024];

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
      sprintf( buf, "%d x %d", img->width(), img->height() );
      draw_text( r, g, b, 5, y, buf );
      y -= yi;
    }

  if ( _hud & kHudFrame )
    {
      sprintf( buf, "% 4" PRId64, img->frame() );
      hud << _("F: ") << buf;
    }

  if ( _hud & kHudTimecode )
    {
      mrv::Timecode::format( buf, mrv::Timecode::kTimecodeNonDrop, 
			     img->frame(), img->play_fps(), true );
      if ( !hud.str().empty() ) hud << " ";
      hud << _("T: ") << buf;
    }

  if ( (_hud & kHudAVDifference) && img->has_audio() )
    {
       // double avdiff = img->audio_clock() - img->video_clock();
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

       if ( (playback() == kForwards && _lastFrame < frame) ||
            (playback() == kBackwards && _lastFrame > frame ) )
	{
	  int64_t frame_diff = ( frame - _lastFrame );
          int64_t absdiff = std::abs(frame_diff);
          if ( absdiff > 1 && absdiff < 10 )
          {
             unshown_frames += absdiff;
          }

	  _lastFrame = frame;
	}
  
      if ( img->real_fps() > 0 )
      {
         sprintf( buf, _(" UF: %" PRId64 " "), unshown_frames );
         hud << buf;

         sprintf( buf, _("FPS: %.2f" ), img->real_fps() );

         if ( !hud.str().empty() ) hud << " ";
         hud << buf;
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
      if (fltk::event_key_state(fltk::LeftAltKey) )
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


	 send( str );

	 add_shape( mrv::shape_type_ptr(s) );
      }

      if ( _wipe_dir != kNoWipe )
      {
   

	 _wipe_dir = (WipeDirection) (_wipe_dir | kWipeFrozen);
	 window()->cursor(fltk::CURSOR_CROSS);
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
	 if ( fg )
	 {
	    menu.add( _("File/Save/Movie or Sequence As"), 
                      kSaveSequence.hotkey(),
		      (fltk::Callback*)save_sequence_cb, this ); 
	    menu.add( _("File/Save/Reel As"), kSaveReel.hotkey(),
		      (fltk::Callback*)save_reel_cb, this ); 
	    menu.add( _("File/Save/Frame As"), kSaveImage.hotkey(),
		      (fltk::Callback*)save_cb, this ); 
	    menu.add( _("File/Save/GL Snapshot As"), kSaveSnapshot.hotkey(),
		      (fltk::Callback*)save_snap_cb, this ); 
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
	       sscanf( tmp, "%f", &mask );
	       if ( mask == _masking ) item->set();
	    }
	    
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

            menu.add( _("Image/Clear Cache"), kClearCache.hotkey(),
                      (fltk::Callback*)clear_image_cache_cb, this,
                      fltk::MENU_DIVIDER );


	    menu.add( _("Image/Attach CTL Rendering Transform"),
		      kCTLScript.hotkey(),
		      (fltk::Callback*)attach_ctl_script_cb,
		      this, fltk::MENU_DIVIDER);
	    menu.add( _("Image/Attach ICC Color Profile"),
		      kIccProfile.hotkey(),
		      (fltk::Callback*)attach_color_profile_cb,
		      this, fltk::MENU_DIVIDER);
	    menu.add( _("Image/Set as Background"), kSetAsBG.hotkey(),
		      (fltk::Callback*)set_as_background_cb,
		      (void*)this);
	    menu.add( _("Image/Toggle Background"),
		      kToggleBG.hotkey(),
		      (fltk::Callback*)toggle_background_cb, (void*)this);

	    Image_ptr image = fg->image();

   
	    size_t num = image->number_of_subtitle_streams();
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
		   NULL);
	  menu.add( _("Monitor/Attach ICC Color Profile"),
		    kMonitorIccProfile.hotkey(),
		    (fltk::Callback*)monitor_icc_profile_cb,
		    this, fltk::MENU_DIVIDER);

   
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
          send( s->send() );
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
          send( s->send() );
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
        send( s->send() );
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


/** 
 * Handle a mouse release
 * 
 * @param x fltk::event_x() coordinate
 * @param y fltk::event_y() coordinate
 */
void ImageView::mouseMove(int x, int y)	
{
  if ( !uiMain->uiPixelBar->visible() || !_engine ) return;



  mrv::media fg = foreground();
  if ( !fg ) return;

  CMedia* img = fg->image();

  double xf = (double) x;
  double yf = (double) y;

  data_window_coordinates( img, xf, yf );


  mrv::image_type_ptr pic = img->hires();

  if ( stereo_type() == CMedia::kStereoCrossed ) pic = img->right();

  if ( !pic ) return;

  mrv::Recti daw = img->data_window();
  mrv::Recti dpw = img->display_window();
  mrv::Recti dpw2 = img->display_window2();

  mrv::Recti dpm = dpw;
  dpm.merge( daw );
  unsigned w = dpm.w();
  unsigned h = dpm.h();
  if ( w == 0 ) w = pic->width();
  if ( h == 0 ) h = pic->height();


  CMedia::Pixel rgba;

  bool outside = false;

  if ( img->is_stereo() && _stereo & CMedia::kStereoSideBySide )
  {
      if ( x < 0 || y < 0 || x >= this->w() || y >= this->h() ||
           xf < 0 || yf < 0 || xf >= w+dpw2.w() || yf >= h )
      {
          outside = true;
      }
  }
  else
  {
      if ( x < 0 || y < 0 || x >= this->w() || y >= this->h() ||
           xf < 0 || yf < 0 || xf >= w || yf >= h )
      {
          outside = true;
      }
  }


  int xp = (int)floor(xf);
  int yp = (int)floor(yf);


  if ( xp > w && ( stereo_type() & CMedia::kStereoSideBySide ) )
  {
      if ( _stereo == CMedia::kStereoCrossed ) pic = img->left();
      else pic = img->right();
      xp -= w;
  }


  if ( xp >= pic->width() || yp >= pic->height() ) {
      outside = true;
  }


  char buf[40];
  sprintf( buf, "%5d, %5d", xp, yp );
  uiMain->uiCoord->text(buf);
  
  if ( outside )
  {
      rgba.r = rgba.g = rgba.b = rgba.a = std::numeric_limits< float >::quiet_NaN();
  }
  else
  {
      rgba = pic->pixel( xp, yp );

      //
      // To represent pixel properly, we need to do gain/gamma/lut 
      //
      rgba.r *= _gain;
      rgba.g *= _gain;
      rgba.b *= _gain;
      float one_gamma = 1.0f / _gamma;
      if ( rgba.r > 0.0001f )
          rgba.r = powf(rgba.r, one_gamma);
      if ( rgba.g > 0.0001f )
          rgba.g = powf(rgba.g, one_gamma);
      if ( rgba.b > 0.0001f )
          rgba.b = powf(rgba.b, one_gamma);

      // @todo: and the lut

      // double yp = yf;
      // if ( _showPixelRatio ) yp /= img->pixel_ratio();
  }
  
  CMedia* bgr = _engine->background();

  if ( _showBG && bgr && ( outside || rgba.a < 1.0f ) )
  {

      const mrv::image_type_ptr& picb = bgr->hires();
      const mrv::Recti& dpwb = bgr->display_window(picb->frame());
      const mrv::Recti& dawb = bgr->data_window(picb->frame());
      if ( picb )
      {
          w = dawb.w();
          h = dawb.h();
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
          if ( xp < 0 || yp < 0 || xp >= w || yp >= h )
          {
              outside2 = true;
          }
          else
          {

              float t = 1.0f - rgba.a;
              CMedia::Pixel bg = picb->pixel( xp, yp );

              float one_gamma = 1.0f / bgr->gamma();
              bg.r *= _gain;
              bg.g *= _gain;
              bg.b *= _gain;
              if ( bg.r > 0.0001f )
                  bg.r = powf(bg.r, one_gamma);
              if ( bg.g > 0.0001f )
                  bg.g = powf(bg.g, one_gamma);
              if ( bg.b > 0.0001f )
                  bg.b = powf(bg.b, one_gamma);

              // @todo:  and the lut

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
            zoom( _zoom + float(dx)*_zoom / 500.0f );
            lastX = x;
            lastY = y;
	}
      else if ( flags & kMouseMove )
	{
	   window()->cursor( fltk::CURSOR_MOVE );
	   xoffset += float(dx) / _zoom;
           yoffset -= float(dy) / _zoom;

	   char buf[128];
	   sprintf( buf, "Offset %g %g", xoffset, yoffset );
	   send( buf );

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
           if ( daw[0].w() == 0 )
           {
               daw[0] = mrv::Recti( 0, 0, img->width(), img->height() );
           }
           daw[1] = img->data_window2();
           if ( daw[1].w() == 0 )
           {
               daw[1] = mrv::Recti( 0, 0, img->width(), img->height() );
           }
           dpw[0] = img->display_window();
           if ( dpw[0].w() == 0 )
           {
               dpw[0] = mrv::Recti( 0, 0, img->width(), img->height() );
           }
           dpw[1] = img->display_window();
           if ( dpw[1].w() == 0 )
           {
               dpw[1] = mrv::Recti( 0, 0, img->width(), img->height() );
           }

	   double xn = double(x);
	   double yn = double(y);
	   data_window_coordinates( img, xn, yn );

           short idx = 0;

	   unsigned W = dpw[0].w();


           bool right = false;
           int diffX = 0;
           int diffY = 0;
           if ( xf >= W - daw[0].x() &&
                stereo_type() & CMedia::kStereoSideBySide )
           {
               right = true;
               xf -= W;
               xn -= W;
               diffX = daw[1].x() - daw[0].x();
               diffY = daw[1].y() - daw[0].y();

               idx = 1;
           }

	   W = dpw[idx].w();
	   unsigned H = dpw[idx].h();

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
               assert( xf <= xn );
               assert( yf <= yn );


               double X, XM, Y, YM;
               if ( display_window() )
               {
                   X = dpw[idx].l()-daw[idx].x();
                   XM = dpw[idx].r()-daw[idx].x();
                   Y = dpw[idx].t()-daw[idx].y();
                   YM = dpw[idx].b()-daw[idx].y();
               }
               else
               {
                   X = diffX;
                   XM = daw[idx].w() + diffX - 1;

                   Y = diffY;
                   YM = daw[idx].h() + diffY - 1;
               }

               if ( xf < X ) xf = X;
               else if ( xf > XM )  xf = XM;
               if ( yf < Y ) yf = Y;
               else if ( yf > YM ) yf = YM;

               if ( xn < X ) xn = X;
               else if ( xn > XM )  xn = XM;
               if ( yn < Y ) yn = Y;
               else if ( yn > YM ) yn = YM;


               double dx = (double) std::abs( xn - xf );
               double dy = (double) std::abs( yn - yf );
               if ( dx == 0.0 || dy == 0.0 )
               {
                   dx = 0.0;
                   dy = 0.0;
               }

               double xt = (xf + daw[0].x() + dpw[0].w() * right);
               double yt = yf + daw[0].y();
               _selection = mrv::Rectd( xt, yt, dx, dy );


               char buf[256];
               sprintf( buf, "Selection %g %g %g %g", _selection.x(),
                        _selection.y(), _selection.w(), _selection.h() );


               send( buf );

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
   else if ( kCloneImage.match( rawkey ) )
   {
      clone_image_cb( NULL, browser() );
      return 1;
   }
   else if ( kSaveImage.match( rawkey ) )
   {
      browser()->save();
      return 1;
   }
   else if ( kIccProfile.match( rawkey ) )
   {
      attach_color_profile_cb( NULL, this );
      return 1;
   }
   else if ( kCTLScript.match( rawkey ) )
   {
      attach_ctl_script_cb( NULL, this );
      return 1;
   }
   else if ( kMonitorCTLScript.match( rawkey ) )
   {
      monitor_ctl_script_cb( NULL, NULL );
      return 1;
   }
   else if ( kMonitorIccProfile.match( rawkey ) )
   {
      monitor_icc_profile_cb( NULL, NULL );
      return 1;
   }
   else if ( kSetAsBG.match( rawkey ) )
   {
      set_as_background_cb( NULL, this );
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
   else if ( rawkey == fltk::LeftAltKey ) 
   {
      flags |= kLeftAlt;
      return 1;
   }
   else if ( rawkey >= kZoomMin.key && rawkey <= kZoomMax.key ) 
   {
      if ( rawkey == kZoomMin.key )
      {
	 xoffset = yoffset = 0;
	 char buf[128];
	 sprintf( buf, "Offset %g %g", xoffset, yoffset );
	 send( buf );

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
      center_image();
     return 1;
  }
  else if ( kFitScreen.match( rawkey ) ) 
    {
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
  else if ( kWipe.match( rawkey ) )
  {
     if ( _wipe_dir == kNoWipe )  {
	_wipe_dir = kWipeVertical;
	_wipe = (float) fltk::event_x() / float( w() );

	char buf[128];
	sprintf( buf, "WipeVertical %g", _wipe );
	send( buf );

	window()->cursor(fltk::CURSOR_WE);
     }
     else if ( _wipe_dir & kWipeVertical )
     {
	_wipe_dir = kWipeHorizontal;
	_wipe = (float) (h() - fltk::event_y()) / float( h() );
	char buf[128];
	sprintf( buf, "WipeHorizontal %g", _wipe );
	send( buf );
	window()->cursor(fltk::CURSOR_NS);
     }
     else if ( _wipe_dir & kWipeHorizontal ) {
	_wipe_dir = kNoWipe;
	_wipe = 0.0f;
	char buf[128];
	sprintf( buf, "NoWipe" );
	send( buf );
	window()->cursor(fltk::CURSOR_CROSS);
     }

     redraw();
     return 1;
  }
  else if ( kFlipX.match( rawkey ) )
  {
     _flip = (FlipDirection)( (int) _flip ^ (int)kFlipVertical );
     redraw();
     return 1;
  }
  else if ( kFlipY.match( rawkey ) )
  {
     _flip = (FlipDirection)( (int) _flip ^ (int)kFlipHorizontal );
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
      if ( playback() == kBackwards )
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

      if ( playback() == kBackwards )
	 stop();
      else
	 play_backwards();
      return 1;
  }
  else if ( kPlayBack.match( rawkey ) ) 
    {

      mrv::media fg = foreground();
      if ( ! fg ) return 1;

      const CMedia* img = fg->image();
      double FPS = 24;
      if ( img ) FPS = img->play_fps();
      fps( FPS );

      if ( playback() == kBackwards )
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
      if ( playback() == kForwards )
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
      if ( playback() != kStopped )
	 stop();
      else
	 play_forwards();
      return 1;
    }
  else if ( kPlayFwd.match( rawkey ) ) 
    {
      mrv::media fg = foreground();
      if ( ! fg ) return 1;

      const CMedia* img = fg->image();
      double FPS = 24;
      if ( img ) FPS = img->play_fps();
      fps( FPS );

      if ( playback() != kStopped )
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
  else if ( kClearCache.match( rawkey ) )
  {
      clear_image_cache_cb( NULL, this );
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
  else
    {
      // Check if a menu shortcut
      fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;

      // check if a channel shortcut
      int num = uiColorChannel->children();
      for ( unsigned short c = 0; c < num; ++c )
	{
	  if ( rawkey == uiColorChannel->child(c)->shortcut() )
	    {
	       if ( c == _channel ) 
	       {
		  c = _old_channel;
	       }
	       channel( c );
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
  if ( key == fltk::LeftAltKey ) 
    {
      if ( _playback == kScrubbing ) 
	{
	  _playback = kStopped;
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
   send( buf );
}

/** 
 * Toggle between a fullscreen view and a normal view with window borders.
 * 
 */
void ImageView::toggle_fullscreen()
{
  bool full = false;
  // full screen...
  if ( fltk_main()->border() )
    {
      full = true;
      posX = fltk_main()->x();
      posY = fltk_main()->y();
      fltk_main()->fullscreen();
    }
  else
    { 
#ifdef LINUX
      fltk_main()->hide();  // @bug: window decoration is missing otherwise
#endif
      resize_main_window();
    }
  fltk_main()->relayout();
  
  fit_image();
  
  char buf[128];
  sprintf( buf, "FullScreen %d", full );
  send( buf );
}

void ImageView::toggle_presentation()
{
  fltk::Window* uiImageInfo = uiMain->uiImageInfo->uiMain;
  fltk::Window* uiColorArea = uiMain->uiColorArea->uiMain;
  fltk::Window* uiEDLWindow = uiMain->uiEDLWindow->uiMain;
  fltk::Window* uiReel  = uiMain->uiReelWindow->uiMain;
  fltk::Window* uiPrefs = uiMain->uiPrefs->uiMain;
  fltk::Window* uiAbout = uiMain->uiAbout->uiMain;

  bool presentation = false;

  static bool has_image_info, has_color_area, has_reel, has_edl_edit,
  has_prefs, has_about, has_top_bar, has_bottom_bar, has_pixel_bar;

  if ( fltk_main()->border() )
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
      fltk_main()->fullscreen();
      // XWindowAttributes xwa;
      // XGetWindowAttributes(fltk::xdisplay, DefaultRootWindow(fltk::xdisplay),
      //                      &xwa);
      // fltk_main()->resize(0, 0, xwa.width, xwa.height );
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

      if ( has_top_bar )    uiMain->uiTopBar->show();
      if ( has_bottom_bar)  uiMain->uiBottomBar->show();
      if ( has_pixel_bar )  uiMain->uiPixelBar->show();

#ifdef LINUX
      fltk_main()->hide();  // @bug: window decoration is missing otherwise
#endif
      resize_main_window();
    }

  fltk_main()->take_focus();
  //   window()->take_focus();
  //   fltk::focus( fltk_main() );

  take_focus();

  fltk::check();
  

  char buf[128];
  sprintf( buf, "PresentationMode %d", presentation );
  send( buf );

  fit_image();
}

/** 
 * Scrub the sequence 
 * 
 * @param dx > 0 scrub forwards, < 0 scrub backwards
 */
void ImageView::scrub( float dx )
{
  stop();

  //  _playback = kStopped; //kScrubbing;
  uiMain->uiPlayForwards->value(0);
  uiMain->uiPlayBackwards->value(0);

  step_frame( boost::int64_t(dx) );

  update_color_info();
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
  switch( event ) 
    {
       case fltk::TIMEOUT:
	  timeout();
	  return 1;
       case fltk::FOCUS:
	  return 1;
    case fltk::ENTER:
      focus(this);
      fltk::GlWindow::handle( event );
      window()->cursor(fltk::CURSOR_CROSS);
      return 1;
    case fltk::LEAVE:
      fltk::GlWindow::handle( event ); 
      window()->cursor(fltk::CURSOR_DEFAULT);
      return 1;
    case fltk::PUSH:
      return leftMouseDown(fltk::event_x(), fltk::event_y());
      break;
    case fltk::RELEASE:
      leftMouseUp(fltk::event_x(), fltk::event_y());
      return 1;
      break;
    case fltk::MOVE:
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
                 send( buf );
                 window()->cursor(fltk::CURSOR_WE);
		break;
	     case kWipeHorizontal:
                 _wipe = (float) (h() - fltk::event_y()) / (float)h();
                 sprintf( buf, "WipeHorizontal %g", _wipe );
                 send( buf );
                 window()->cursor(fltk::CURSOR_NS);
		break;
	     default:
		break;
	  }
	  redraw();
	  return 1;
       }
      if ( fltk::event_key_state( fltk::LeftShiftKey ) ||
	   fltk::event_key_state( fltk::RightShiftKey ) )
	{
            float dx = (float) (fltk::event_x() - lastX) / 5.0f;
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

      return 1;
      break;
    case fltk::DRAG:
       X = fltk::event_x();
       Y = fltk::event_y();
       mouseDrag( int(X), int(Y) );
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

/** 
 * Clear and refresh sequence
 * 
 */
void ImageView::flush_caches()
{
  mrv::media fg = foreground();
  if ( fg )
    {
      CMedia* img = fg->image();
      if ( img->is_sequence() && img->first_frame() != img->last_frame()
	   && (_engine->shader_type() == DrawEngine::kNone ) ) 
	{
	  img->clear_cache();
	  img->fetch(frame());
	}
    }

  mrv::media bg = background();
  if ( bg )
    {
      CMedia* img = bg->image();
      if ( img->is_sequence() && 
	   img->first_frame() != img->last_frame()
	   && (_engine->shader_type() == DrawEngine::kNone)  )
	{
	  img->clear_cache();
	  img->fetch(frame());
	}
    }
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



/** 
 * Change image display to a new channel (from channel list)
 * 
 * @param c index of channel
 */
void ImageView::channel( unsigned short c )
{ 
  _channel = c;


  fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;
  
  if ( c >= uiColorChannel->children() ) return;

  char buf[128];
  sprintf( buf, "Channel %d", c );
  send( buf );

  const char* lbl = uiColorChannel->child(c)->label();
  std::string channelName( lbl );


  static std::string oldChannel;

  std::string ext = channelName;
  std::string oext = oldChannel;



  size_t pos = ext.rfind('.');

  size_t pos2 = oldChannel.rfind('.');

  if ( pos != std::string::npos )
  {
     ext = ext.substr( pos+1, ext.size() );
  }

  if ( pos2 != std::string::npos )
  {
     oext = oext.substr( pos2+1, oext.size() );
  }


  uiColorChannel->value( c );
  uiColorChannel->label( lbl );
  uiColorChannel->redraw();

  _channelType = kRGB;
  if ( channelName == "Alpha Overlay" )
    {
      _channelType = kAlphaOverlay;
    }
  else if ( channelName == "Red" || ext == N_("R") )
    {
      _channelType = kRed;
    }
  else if ( channelName == "Green" || ext == N_("G") )
    {
      _channelType = kGreen;
    }
  else if ( channelName == "Blue"  || ext == N_("B"))
    {
      _channelType = kBlue;
    }
  else if ( channelName == "Alpha" || ext == N_("A") )
    {
      _channelType = kAlpha;
    }
  else if ( channelName == "Lumma" )
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
           ext = "";
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

  mrv::media fg = foreground();
  mrv::media bg = background();

  if ( ( ext != N_("R") && ext != N_("G") && ext != N_("B") &&
         ext != N_("A") ) )
  {
     if ( fg ) fg->image()->channel( lbl );
     if ( bg ) bg->image()->channel( lbl );
  }

  oldChannel = channelName;

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
  send( buf );

  refresh_fstop();
  flush_caches();
  smart_refresh();
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
  send( buf );

  uiMain->uiGamma->value( f );
  uiMain->uiGammaInput->value( f );

  flush_caches();
  smart_refresh();
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
  send( buf );

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
  if ( z == _zoom || z > kMaxZoom || z < kMinZoom ) return;
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
  if ( W == 0 ) W = img->width();

  int H = dpw.h();
  if ( H == 0 ) H = img->height();



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
  send( buf );

  mouseMove( x, y );
}


double ImageView::pixel_ratio() const
{
  mrv::media fg = foreground();
  if ( !fg ) return 1.0f;
  return fg->image()->pixel_ratio();
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
      uiColorChannel->remove_all();
      uiColorChannel->add( _("(no image)") );
      uiColorChannel->label( _("(no image)") );
      uiColorChannel->value(0);
      uiColorChannel->redraw();
      return;
    }

  int ch = uiColorChannel->value();

  char* channelName = NULL;
  

  if ( ch >= 0 && ch < uiColorChannel->children() )
    {
      channelName = strdup( uiColorChannel->child(ch)->label() );
    }

  uiColorChannel->remove_all();

  CMedia* img = fg->image();
  stringArray layers = img->layers();

  stringArray::const_iterator i = layers.begin();
  stringArray::const_iterator e = layers.end();

  int v   = -1;
  int idx = 0;
  std::set< short > shortcuts;

  if (!channelName) v = 0;

  for ( ; i != e; ++i, ++idx )
    {
      const std::string& name = (*i).c_str();
      fltk::Widget* o = uiColorChannel->add( name.c_str(), NULL );

      if ( channelName && ( name == channelName ) )
      {
	 v = idx;
	 _old_channel = (unsigned short)v;
      }

      if ( v >= 0 )
      {
	 short shortcut = get_shortcut( name.c_str() );
	 if ( shortcut && shortcuts.find( shortcut ) == shortcuts.end())
	 { 
	    o->shortcut( shortcut );
	    shortcuts.insert( shortcut );
	 }
      }

    }

  if ( v == -1 ) v = 0;

  free( channelName );

  if ( v < uiColorChannel->children() )
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



  delete_timeout();

  CMedia* img = NULL;
  if ( fg ) 
    {
      img = fg->image();
      
      double fps = img->fps();
      timeline()->fps( fps );
      uiMain->uiFrame->fps( fps );
      uiMain->uiStartFrame->fps( fps );
      uiMain->uiEndFrame->fps( fps );
      uiMain->uiFPS->value( img->play_fps() );
      
      img->volume( _volume );

    }

  _fg = fg;

  if ( fg ) 
    {
      CMedia* img = fg->image();
      
      if ( img )
      {
	 
	 // Per session gamma: requested by Willa
	 if ( img->gamma() > 0.0f ) gamma( img->gamma() );

	 refresh_fstop();
	 
	 if ( img->width() > 160 && !fltk_main()->border() ) {
            fit_image();
         }

         if ( uiMain->uiPrefs->uiPrefsAutoFitImage->value() )
             fit_image();

	 img->image_damage( img->image_damage() | CMedia::kDamageContents |
                            CMedia::kDamage3DData );

	 bool reload = (bool) uiMain->uiPrefs->uiPrefsAutoReload->value();
	 if ( dynamic_cast< stubImage* >( img ) || reload )
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
  mrv::media fg = foreground();
  if ( ! fg ) return;

  Playback p = playback();
  stop();

  CMedia* img = fg->image();

  unsigned int numAudioTracks = uiMain->uiAudioTracks->children();
  if ( idx >= numAudioTracks - 1 )
    img->audio_stream( -1 );
  else
    img->audio_stream( idx );

  if ( p != kStopped ) play( (CMedia::Playback) p );

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


  delete_timeout();

  char buf[1024];

  _bg = bg;
  if ( bg ) 
    {
      CMedia* img = bg->image();

      sprintf( buf, "CurrentBGImage \"%s\" %" PRId64 " %" PRId64, 
               img->fileroot(), img->first_frame(), img->last_frame() );
      send( buf );

      img->volume( _volume );
#if 0
      if ( playback() != kStopped ) 
	 img->play( (CMedia::Playback) playback(), uiMain, false );
      else 
#endif
	img->refresh();

      img->play_fps( fps() );
      img->image_damage( img->image_damage() | CMedia::kDamageContents );      

      bool reload = (bool) uiMain->uiPrefs->uiPrefsAutoReload->value();
      if ( dynamic_cast< stubImage* >( img ) || reload )
	{
	  create_timeout( 0.2 );
	}
    }
  else
  {
      sprintf( buf, "CurrentBGImage \"\"" );
      send( buf );
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

  fltk::Monitor monitor = fltk::Monitor::all();
  int minx = monitor.work.x();
  int miny = monitor.work.y() + 8;
  int maxh = monitor.work.h() - 8;
  int maxw = monitor.work.w();

  bool fit = false;

  if ( w > maxw ) { fit = true; w = maxw; }
  if ( w < 640 )  w = 640;

  if ( h > maxh ) { fit = true; h = maxh; }
  if ( h < 550 )  h = 550;

  if ( posX + w > maxw ) posX = maxw - w;
  if ( posX < minx )     posX = minx;

  if ( posY + h > maxh ) posY = maxh - h;
  if ( posY < miny )     posY = miny;



  fltk_main()->fullscreen_off( posX, posY, w, h);
  fltk_main()->show();

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
  send( buf );
}

void ImageView::display_window( const bool b ) 
{
  _displayWindow = b;

  char buf[128];
  sprintf( buf, "DisplayWindow %d", (int)b );
  send( buf );
}

void ImageView::safe_areas( const bool b ) 
{
  _safeAreas = b;

  char buf[128];
  sprintf( buf, "SafeAreas %d", (int)b );
  send( buf );
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
   send( buf );

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
   damage_contents();
   redraw();
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
   send( buf );

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

  char buf[128];
  sprintf( buf, "UseLUT %d", (int)_useLUT );
  send( buf );

  flush_caches();
  if ( _useLUT ) {
     damage_contents();
     _engine->refresh_luts();
  }
  smart_refresh();
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
   browser()->frame( f );
}



/** 
 * Jump to a frame in timeline, creating new thumbnails if needed.
 * 
 * @param f new frame in timeline units
 */
void ImageView::seek( const int64_t f )
{

  // Hmmm... this is somewhat inefficient.  Would be better to just
  // change fg/bg position
  browser()->seek( f );

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

  stop();

  int64_t start = (int64_t) timeline()->minimum();
  int64_t end   = (int64_t) timeline()->maximum();

  int64_t f = frame();

  ImageView::Looping loop = looping();

  if ( n > 0 )
    {
       if ( f + n > end )
       {
	  if ( loop == ImageView::kLooping )
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
	   if ( loop == ImageView::kLooping )
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


/// Change frame number to first frame of image
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

       if ( int64_t( uiMain->uiFrame->value() ) == f )
       {
	  browser()->previous_image();
          fit_image();
	  return;
       }

       uiMain->uiFrame->value( f );
    }

  int64_t t = int64_t( timeline()->minimum() );
  if ( t > f ) f = t;

  seek( f );
}

/// Change frame number to last frame of image
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
      if ( int64_t( uiMain->uiFrame->value() ) == f )
	{
	  browser()->next_image();
          fit_image();
	  return;
	}
      
      uiMain->uiFrame->value( f );

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

void ImageView::playback( const ImageView::Playback b )
{
  _playback = b;

  _lastFrame = frame();
  _last_fps = 0.0;

  if ( b == kForwards )
    {
      uiMain->uiPlayForwards->value(1);
      uiMain->uiPlayBackwards->value(0);
    }
  else if ( b == kBackwards )
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
      send("playfwd");
   }
   else if ( dir == CMedia::kBackwards )
   {
      send("playback");
   }
   else
   {
      LOG_ERROR( "Not a valid playback mode" );
      return;
   }


   playback( (Playback) dir );

   delete_timeout();

   double fps = uiMain->uiFPS->value();
   create_timeout( 1.0/(fps*2.0) );

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
  if ( playback() != kStopped &&
       playback() != kScrubbing ) return;

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
   if ( playback() == kStopped ) return;


   mrv::media m = foreground();
   if ( m ) m->image()->stop();

   m = background();
   if ( m ) m->image()->stop();

  _playback = kStopped;
  _last_fps = 0.0;
  _real_fps = 0.0;

  stop_playback();

  uiMain->uiPlayForwards->value(0);
  uiMain->uiPlayBackwards->value(0);

  send( "stop" );
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
  send( buf );

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
  if ( fg ) fg->image()->volume( v );

  mrv::media bg = background();
  if ( bg ) bg->image()->volume( v );

  uiMain->uiVolume->value( v );
  uiMain->uiVolume->redraw();


  char buf[128];
  sprintf( buf, "Volume %g", v );
  send( buf );

}

/// Set Playback looping mode
void  ImageView::looping( Looping x )
{
  _looping = x;

  uiMain->uiLoopMode->value( x );
  uiMain->uiLoopMode->label(uiMain->uiLoopMode->child(x)->label());
  uiMain->uiLoopMode->redraw();

  char buf[64];
  sprintf( buf, "Looping %d", x );
  send( buf );

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
    _("F"),
    _("T"),
    _("B")
  };

  uiMain->uiField->label( kFieldNames[_field] );

  char buf[128];
  sprintf( buf, "FieldDisplay %d", _field );
  send( buf );

  damage_contents();
  redraw();
}


} // namespace mrv

