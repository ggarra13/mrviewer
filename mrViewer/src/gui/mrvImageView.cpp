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


#if defined(WIN32) || defined(WIN64)
#  include <float.h>
#  define isnan(x) _isnan(x)
#endif

#include <iostream>
#include <sstream>
#include <limits>
#include <iomanip>
#include <sstream>
#include <set>


#include <fltk/visual.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/draw.h>
#include <fltk/run.h>

#include <fltk/Color.h>
#include <fltk/Cursor.h>
#include <fltk/Output.h>
#include <fltk/Choice.h>
#include <fltk/ValueOutput.h>
#include <fltk/Window.h>
#include <fltk/Menu.h>
#include <fltk/PopupMenu.h>
#include <fltk/Monitor.h>
#include <fltk/Button.h>


#include "ImathMath.h" // for Math:: functions


// CORE classes
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
#include "gui/mrvImageView.h"

// Widgets
#include "mrViewer.h"
#include "mrvIccProfileUI.h"
#include "mrvColorAreaUI.h"

// Video
#include "video/mrvGLEngine.h"  // should be dynamically chosen from prefs

// Audio


using namespace std;

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
       if ( ext == N_("COLOR") ) return 'c';
       if ( ext == N_("R") || ext == N_("RED")   ) return 'r';
       if ( ext == N_("G") || ext == N_("GREEN") ) return 'g';
       if ( ext == N_("B") || ext == N_("BLUE")  ) return 'b';
       if ( ext == N_("A") || ext == N_("ALPHA") ) return 'a';
    }

    oldChannel = channelName;

    return 0;
  }


}

extern void clone_all_cb( fltk::Widget* o, mrv::ImageBrowser* b );
extern void clone_image_cb( fltk::Widget* o, mrv::ImageBrowser* b );

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

void save_cb( fltk::Widget* o, mrv::ImageBrowser* uiReelWindow )
{
  uiReelWindow->save();
}

void window_cb( fltk::Widget* o, const mrv::ViewerUI* uiMain )
{
  const char* name = o->label();

  if ( strcmp( name, "Reels" ) == 0 )
    {
      uiMain->uiReelWindow->uiMain->show();
    }
  else if ( strcmp( name, "Media Info") == 0 )
    {
      uiMain->uiImageInfo->uiMain->show();
      uiMain->uiView->update_image_info();
    }
  else if ( strcmp( name, "Color Info") == 0 )
    {
      uiMain->uiColorArea->uiMain->show();
      uiMain->uiView->update_color_info();
    }
  else if ( strcmp( name, "Histogram") == 0 )
    {
      uiMain->uiHistogram->uiMain->show();
    }
  else if ( strcmp( name, "Vectorscope") == 0 )
    {
      uiMain->uiVectorscope->uiMain->show();
    }
  else if ( strcmp( name, "Preferences") == 0 )
    {
      uiMain->uiPrefs->uiMain->child_of( uiMain->uiMain );
      uiMain->uiPrefs->uiMain->show();
    }
  else if ( strcmp( name, "Logs") == 0 )
    {
      uiMain->uiLog->uiMain->child_of( uiMain->uiMain );
      uiMain->uiLog->uiMain->show();
    }
  else if ( strcmp( name, "About") == 0 )
    {
      uiMain->uiAbout->uiMain->show();
    }
  else if ( strcmp( name, "ICC Profiles" ) == 0 )
    {
      uiMain->uiICCProfiles->uiMain->show();
      uiMain->uiICCProfiles->fill();
    }
  else
    {
      LOG_ERROR("Unknown Window \"" << name << "\"");
    }
}



void masking_cb( fltk::Widget* o, mrv::ViewerUI* uiMain )
{
  mrv::ImageView* view = uiMain->uiView;

  float mask = 1.0f;
  const char* fmt = o->label();

  sscanf( fmt, "%f", &mask );
  
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

  // if ( img->has_audio() )
  //   {
  //     view->browser()->add_audio( fg );
  //   }
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
  _channel( 0 ),
  _old_channel( 0 ),
  _channelType( kRGB ),
  _field( kFrameDisplay ),
  _showBG( true ),
  _showPixelRatio( false ),
  _useLUT( false ),
  _volume( 1.0f ),
  _flip( kFlipNone ),
  _timeout( NULL ),
  _selection( mrv::Rectd(0,0) ),
  _playback( kStopped ),
  _looping( kLooping ),
  _lastFrame( 0 )
{
  _timer.setDesiredSecondsPerFrame(0.0f);

  mode( fltk::RGB24_COLOR | fltk::DOUBLE_BUFFER | fltk::ALPHA_BUFFER );
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
  // make sure to stop any playback
  stop_playback();

  delete_timeout();
  delete _engine;
}

fltk::Window* ImageView::fltk_main()
{ 
  return uiMain->uiMain; 
}

const fltk::Window* ImageView::fltk_main() const
{ 
  return uiMain->uiMain; 
}


ImageBrowser* 
ImageView::browser() { 
  return uiMain->uiReelWindow->uiBrowser;
}


Timeline* 
ImageView::timeline() { 
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

  mrv::image_type_ptr pic;
  {
    CMedia::Mutex& m = img->video_mutex();
    SCOPED_LOCK(m);
    pic = img->hires();
  }
  if ( !pic ) return;


  double x = double(lastX);
  double y = double(lastY);

  if ( x < 0 || y < 0 || x >= this->w() || y >= this->h() )
    return;

  image_coordinates( pic, x, y );

  int w = pic->width();
  int h = pic->height();

  if ( x < 0 || y < 0 || x >= w || y >= h )
    return; // outside image

  CMedia::PixelType rgba = pic->pixel( (int)x, (int)y );

  char buf[256];
  sprintf( buf, "%g %g %g %g", rgba.r, rgba.g, rgba.b, rgba.a );

  // Copy text to both the clipboard and to X's XA_PRIMARY
  fltk::copy( buf, unsigned( strlen(buf) ), true );
  fltk::copy( buf, unsigned( strlen(buf) ), false );
}

/** 
 * Given window's x and y coordinates, return an image's
 * corresponding x and y coordinate
 * 
 * @param img image to find coordinates for
 * @param x   window's x position
 * @param y   window's y position
 */
void ImageView::image_coordinates( const mrv::image_type_ptr& img, 
				   double& x, double& y ) const
{
  int ww = img->width();
  int hh = img->height();
  if ( _showPixelRatio ) hh = (int) (hh / pixel_ratio());

  double tw = (ww / 2.0);
  double th = (hh / 2.0);

  x -= (w() - ww) / 2.0; 
  y += (h() - hh) / 2.0;

  y = (this->h() - y - 1);

  x -= tw; y -= th;
  x /= _zoom; y /= _zoom;
  x += tw; y += th;
  x -= xoffset; y -= yoffset;

  y = hh - y;
  if ( _showPixelRatio ) y *= pixel_ratio();
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
  if ( img->width() <= 0 ) return;

  double w = (double) fltk_main()->w();
  double z = w / (double)img->width();
  
  double h = (double) fltk_main()->h();
  if ( uiMain->uiTopBar->visible() )
    h -= uiMain->uiTopBar->h();
  if ( uiMain->uiPixelBar->visible() )
    h -= uiMain->uiPixelBar->h();
  if ( uiMain->uiBottomBar->visible() )
    h -= uiMain->uiBottomBar->h();

  h /= img->height();
  if ( _showPixelRatio ) h *= pixel_ratio();
  if ( h < z ) { z = h; }

  xoffset = yoffset = 0.0;
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


void ImageView::timeout()
{
  //
  // If in EDL mode, we check timeline to see if frame points to
  // new image.
  //
  mrv::Timeline* timeline = this->timeline();
  if ( timeline && timeline->edl() )
    {
      int64_t frame = boost::int64_t( timeline->value() );
      mrv::media newfg = timeline->media_at( frame );

      if ( newfg != foreground() ) foreground( newfg );
    }

  mrv::media fg = foreground();

  static double kMinDelay = 0.0001666;

  double delay = 0.005;
  if ( fg )
    {
      CMedia* img = fg->image();
      delay = 1.0 / (img->play_fps() * 2.0);
      if ( delay < kMinDelay ) delay = kMinDelay;
    }

  repeat_timeout( float(delay) );

  if ( should_update( fg ) ) 
    {
      update_color_info( fg );

      if ( timeline && timeline->visible() ) 
	{

	  int64_t frame;
	  if ( !timeline->edl() )
	    {
	      CMedia* img = fg->image();
	      
	      frame = (int64_t) img->frame();
	      timeline->value( double(frame) );
	      timeline->redraw();
	      if ( img && img->first_frame() != img->last_frame() &&
		   this->frame() != frame )
		{
		  // 		    this->frame( frame );
		}
	    }
	  else
	    {
	      frame = (int64_t) timeline->value();
	      if ( this->frame() != frame )
		{
		  // updating frame
		  this->frame( frame );
		}
	    }
	  
	  uiMain->uiFrame->value( frame );
	  uiMain->uiFrame->redraw();
	}

      redraw();
    }





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
  }


  mrv::media bg = background();
  mrv::media fg = foreground();


  ImageList images;
  images.reserve(2);

  if ( _showBG && bg && bg != fg && bg->image() )
    {
       if ( bg->image()->has_picture() )
	  images.push_back( bg->image() );
    }

  if ( fg && fg->image() )
    {
       if ( fg->image()->has_picture() )
	  images.push_back( fg->image() );
    }

  if ( images.empty() ) return;

  _engine->draw_images( images );

  if ( !fg || fg->image() == NULL ) return;

  if ( _masking != 0.0f )
    {
      _engine->draw_mask( _masking );
    }

  const char* label = fg->image()->label();
  if ( label )
    {
      uchar r, g, b;
      fltk::split_color( uiPrefs->uiPrefsViewText->color(), r, g, b );


      int dx, dy;
      dx = int( (double) w() / (double) 2 - strlen(label)*3 );
      dy = 24;

      _engine->color( r, g, b );
      _engine->draw_text( dx, dy, label );
    }

  if ( _selection.w() > 0 || _selection.h() > 0 )
    {
      uchar r, g,  b;
      fltk::split_color( uiPrefs->uiPrefsViewSelection->color(), r, g, b );
      _engine->color( r, g, b );
      _engine->draw_rectangle( _selection );
    }


  if ( _safeAreas ) 
    {
      const CMedia* img = fg->image();
      double aspect = (float) img->width() / (float) img->height();

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
	  float f = img->height() * 1.33f;
	  f = f / img->width();
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
	      float f = img->height() * 1.33f;
	      f = f / img->width();
	      _engine->color( 1.0f, 0.0f, 0.0f );
	      _engine->draw_safe_area( f * 0.9f, 0.9f, _("tv action") );
	      _engine->draw_safe_area( f * 0.8f, 0.8f, _("tv title") );
	    }
	}

    }



  if ( _hud == kHudNone )
    return;

  std::ostringstream hud;
  hud.str().reserve( 512 );

  uchar r, g,  b;
  fltk::split_color( uiPrefs->uiPrefsViewHud->color(), r, g, b );
  _engine->color( r, g, b );


  //
  // Draw filename
  //
  const CMedia* img = fg->image();

  int y = h()-25;
  int yi = 25;
  if ( _hud & kHudFilename )
    {
      hud << img->name();
    }

  if ( img->first_frame() != img->last_frame() && _hud & kHudFrameRange )
    {
      if ( !hud.str().empty() ) hud << " ";
      hud << img->first_frame() << " - " << img->last_frame();
    }

  if ( !hud.str().empty() )
    {
      _engine->draw_text( 5, y, hud.str().c_str() );
      y -= yi; hud.str("");
    }


  static char buf[256];

  if ( _hud & kHudResolution )
    {
      sprintf( buf, "%d x %d", img->width(), img->height() );
      _engine->draw_text( 5, y, buf );
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

      if ( _lastFrame != frame )
	{
	  int64_t frame_diff = ( frame - _lastFrame );

	  if ( playback() ) {
	     frame_diff *= playback();

	     int64_t absdiff = std::abs(frame_diff);
	     if ( absdiff > 1 && absdiff < 10 )
	     {
		unshown_frames += absdiff;

	     }
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
      _engine->draw_text( 5, y, hud.str().c_str() );
      y -= yi;
    }

  if ( _hud & kHudIPTC )
    {
      CMedia::Attributes::const_iterator i = img->iptc().begin();
      CMedia::Attributes::const_iterator e = img->iptc().end();
      for ( ; i != e; ++i )
	{
	  std::string text = i->first + ": " + i->second;
	  _engine->draw_text( 5, y, text.c_str() );
	  y -= yi;
	}
    }

}





/** 
 * Handle a mouse press
 * 
 * @param x fltk::event_x() coordinate
 * @param y fltk::event_y() coordinate
 */
void ImageView::leftMouseDown(int x, int y)	
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
	 return;
      }

      flags |= kMouseLeft;
      _selection = mrv::Rectd( 0, 0, 0, 0 );
      

      if ( _wipe_dir != kNoWipe )
      {
	 _wipe_dir = (WipeDirection) (_wipe_dir | kWipeFrozen);
	 window()->cursor(fltk::CURSOR_CROSS);
      }

//       if ( fltk::event_is_click() && fltk::event_clicks() > 0 )
// 	{
// 	  toggle_fullscreen();
// 	  fltk::event_clicks(0);
// 	}
      redraw();
    }

  else if ( button == 2 )
    {
       // handle MMB moves
       flags |= kMouseMove;
       flags |= kMouseMiddle;
    }
  else
    {
      if ( (flags & kLeftAlt) == 0 )
      {
	 fltk::Menu menu(0,0,0,0);
	 menu.add( _("File/Open"), fltk::CTRL + 'o',
		   (fltk::Callback*)open_cb, browser() ); 
	 mrv::media fg = foreground();
	 if ( fg )
	    menu.add( _("File/Save Frame As"), fltk::CTRL + 's',
		      (fltk::Callback*)save_cb, browser() ); 
	 
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

	    menu.add( "View/Safe Areas", 's', 
		      (fltk::Callback*)safe_areas_cb, this );
	    
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
	    
      	    menu.add( _("Image/Next"), fltk::PageDownKey, 
		      (fltk::Callback*)next_image_cb, browser());
	    menu.add( _("Image/Previous"), fltk::PageUpKey, 
		      (fltk::Callback*)previous_image_cb, 
		      browser(), fltk::MENU_DIVIDER);

	    const stubImage* img = dynamic_cast< const stubImage* >( image() );
	    if ( img )
	    {
	       menu.add("Image/Clone", 0, 
			(fltk::Callback*)clone_image_cb, browser());
	       menu.add("Image/Clone All Channels", 0, 
			(fltk::Callback*)clone_all_cb, 
			browser(), fltk::MENU_DIVIDER);
	    }
	    else
	    {
	       menu.add("Image/Clone", 0, 
			(fltk::Callback*)clone_image_cb, browser(),
			fltk::MENU_DIVIDER);
	    }
	    
	    menu.add( _("Image/Attach CTL Rendering Transform"), 
		      fltk::COMMAND + 'T', 
		      (fltk::Callback*)attach_ctl_script_cb, 
		      this, fltk::MENU_DIVIDER);
	    menu.add( _("Image/Attach ICC Color Profile"), 
		      fltk::COMMAND + 'I', 
		      (fltk::Callback*)attach_color_profile_cb, 
		      this, fltk::MENU_DIVIDER);
	    menu.add( _("Image/Set as Background"), 0, 
		      (fltk::Callback*)set_as_background_cb, 
		      (void*)this);
	    menu.add( _("Image/Toggle Background"), fltk::TabKey, 
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

	    if ( fg->image()->is_sequence() )
	    {
	        menu.add( _("Audio/Attach Audio File"), 0,
	     		 (fltk::Callback*)attach_audio_cb, this );
	    }

	    menu.add( _("Pixel/Copy RGBA Values to Clipboard"), 
		      fltk::COMMAND + 'c', 
		      (fltk::Callback*)copy_pixel_rgba_cb, (void*)this);
	 }



	  menu.add( _("Monitor/Attach CTL Display Transform"), 
		   fltk::COMMAND + 'M', 
		   (fltk::Callback*)monitor_ctl_script_cb, 
		   NULL);
	  menu.add( _("Monitor/Attach ICC Color Profile"), 
		   fltk::COMMAND + 'N', 
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
   }

  if ( (flags & kLeftAlt) && (flags & kMouseLeft) && (flags & kMouseMiddle) )
    {
      flags |= kZoom;
    }
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
      static std::string empty( "        " );
      return empty;
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

  if ( x >=  0 && y >= 0 && x < this->w() && y < this->h() )
    {
      focus(this);
    }

  double xf = (double) x;
  double yf = (double) y;

  mrv::media fg = foreground();
  if ( !fg ) return;

  CMedia* img = fg->image();
  mrv::image_type_ptr pic;
  {
    CMedia::Mutex& m = img->video_mutex();
    SCOPED_LOCK(m);
    pic = img->hires();
  }
  if ( !pic ) return;

  image_coordinates( pic, xf, yf );

  unsigned w = pic->width();
  unsigned h = pic->height();
  CMedia::PixelType rgba;


  bool outside = false;
  if ( x < 0 || y < 0 || x >= this->w() || y >= this->h() ||
       xf < 0 || yf < 0 || xf >= w || yf >= h )
    {
      outside = true;
    }

  if ( outside )
    {
      uiMain->uiCoord->text("");
      rgba.r = rgba.g = rgba.b = rgba.a = std::numeric_limits< float >::quiet_NaN();
    }
  else
    {
      unsigned xp = (unsigned)floor(xf);
      unsigned yp = (unsigned)floor(yf);

      char buf[40];
      sprintf( buf, "%5d, %5d", xp, yp );
      uiMain->uiCoord->text(buf);

//       float yp = yf;
//       if ( _showPixelRatio ) yp /= img->pixel_ratio();

      rgba = pic->pixel( xp, yp );

      CMedia* bgr = _engine->background();
      if ( _showBG && bgr && rgba.a < 1.0f &&
	   bgr->width() == img->width() &&
	   bgr->height() == img->height() )
	{ 
	  float t = 1.0f - rgba.a;

	  {
	    CMedia::Mutex& m = bgr->video_mutex();
	    SCOPED_LOCK(m);
	    pic = bgr->hires();
	  }
	  if ( pic )
	    {
	      CMedia::PixelType bg = pic->pixel( xp, yp );
	      rgba.r += bg.r * t;
	      rgba.g += bg.g * t;
	      rgba.b += bg.b * t;
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
  // To represent pixel properly, we need to do gain/gamma/lut 
  //
  float one_gamma = 1.0f / _gamma;
  rgba.r = pow(rgba.r * _gain, one_gamma);
  rgba.g = pow(rgba.g * _gain, one_gamma);
  rgba.b = pow(rgba.b * _gain, one_gamma);
  if ( rgba.r > 1.0f ) rgba.r = 1.0f;
  else if ( rgba.r < 0.0f ) rgba.r = 0.0f;
  if ( rgba.g > 1.0f ) rgba.g = 1.0f;
  else if ( rgba.g < 0.0f ) rgba.g = 0.0f;
  if ( rgba.b > 1.0f ) rgba.b = 1.0f;
  else if ( rgba.b < 0.0f ) rgba.b = 0.0f;

  //
  // Show this pixel as 8-bit fltk color box
  //
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





  CMedia::PixelType hsv;
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
    case color::kITU_702:
      hsv = color::rgb::to_ITU702( rgba );   break;
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
	  zoom( _zoom + dx*_zoom / 500.0f );
	  lastX = x;
	  lastY = y;
	} 
      else if ( flags & kMouseMove )
	{
	  xoffset += dx / _zoom;
	  yoffset -= dy / _zoom;
	  lastX = x;
	  lastY = y;
	}
      else
	{
	  mrv::media fg = foreground();
	  if ( ! fg ) return;

	  CMedia* img = fg->image();

	  mrv::image_type_ptr pic;
	  {
	    CMedia::Mutex& m = img->video_mutex();
	    SCOPED_LOCK(m);
	    pic = img->hires();
	  }
	  if ( !pic ) return;

	  unsigned int texWidth = pic->width();
	  unsigned int texHeight = pic->height();

	  double xf = double(lastX);
	  double yf = double(lastY);
	  image_coordinates( pic, xf, yf );
	  if ( xf < 0 ) xf = 0;
	  else if ( xf > texWidth )  xf = double(texWidth);
	  if ( yf < 0 ) yf = 0;
	  else if ( yf > texHeight ) yf = double(texHeight);

	  double xn = double(x);
	  double yn = double(y);
	  image_coordinates( pic, xn, yn );
	  if ( xn < 0 ) xn = 0;
	  else if ( xn > texWidth )  xn = double(texWidth);
	  if ( yn < 0 ) yn = 0;
	  else if ( yn > texHeight ) yn = double(texHeight);

	  xf = floor(xf);
	  yf = floor(yf);
	  xn = floor(xn+0.5f);
	  yn = floor(yn+0.5f);

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

	  unsigned dx = (unsigned) std::abs( xn - xf );
	  unsigned dy = (unsigned) std::abs( yn - yf );


	  // store selection square
	  unsigned W = texWidth;
	  unsigned H = texHeight;
	  if ( dx > W ) dx = W;
	  if ( dy > H ) dy = H;

	  _selection = mrv::Rectd( (double)xf/(double)W, 
				   (double)yf/(double)H, 
				   (double)dx/(double)W, 
				   (double)dy/(double)H );
	  assert( _selection.x() >= 0.0 && _selection.x() <= 1.0);
	  assert( _selection.y() >= 0.0 && _selection.y() <= 1.0);
	  assert( _selection.w() >= 0.0 && _selection.w() <= 1.0);
	  assert( _selection.h() >= 0.0 && _selection.h() <= 1.0);

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

  if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
       fltk::event_key_state( fltk::RightCtrlKey ) )
  {
     if ( rawkey == 'o' )
     {
	browser()->open();
	return 1;
     }
     if ( rawkey == 's' )
     {
	browser()->save();
	return 1;
     }
     if ( rawkey == 'm' )
     {
	monitor_ctl_script_cb( NULL, NULL );
	return 1;
     }
     if ( rawkey == 'n' )
     {
	monitor_icc_profile_cb( NULL, NULL );
	return 1;
     }
  }

  std::string key( fltk::event_text() );

  if ( key == "]" )
    {
      exposure_change( 0.5f );
    }
  else if ( key == "[" )
    {
      exposure_change( -0.5f );
    }
  else if ( key == ")" )
    {
      gamma( gamma() + 0.1f );
    }
  else if ( key == "(" )
    {
      gamma( gamma() - 0.1f );
    }
  else if ( rawkey == fltk::LeftAltKey ) 
    {
      flags |= kLeftAlt;
      return 1;
    } 
  else if ( rawkey >= '0' && rawkey <= '9' ) 
    {
      if ( rawkey <= '1' )
	{
	  xoffset = yoffset = 0;
	  zoom( 1.0f );
	}
      else
	{
	  float z = (float) (rawkey - '0');
	  if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
	       fltk::event_key_state( fltk::RightCtrlKey ) )
	    z = 1.0f / z;
	  zoom_under_mouse( z, fltk::event_x(), fltk::event_y() );
	}
      return 1;
    }
  else if ( rawkey == '.' )
    {
      zoom_under_mouse( _zoom * 2.0f, 
			fltk::event_x(), fltk::event_y() );
      return 1;
    }
  else if ( rawkey == ',' )
    {
      zoom_under_mouse( _zoom * 0.5f, 
			fltk::event_x(), fltk::event_y() );
      return 1;
    }
  else if ( rawkey == ' ' ) 
    {
      // full screen...
      if ( fltk_main()->border() )
	{
	  posX = fltk_main()->x();
	  posY = fltk_main()->y();
	  fltk_main()->fullscreen();
	}
      else
      { 
	 fltk_main()->border(true);
	 resize_main_window();
      }
      fltk_main()->relayout();
      xoffset = yoffset = 0;
      return 1;
    }
  else if ( rawkey == 'f' ) 
    {
      fit_image();
      return 1;
    }
  else if ( rawkey == 's' )
    {
      _safeAreas ^= true;
      redraw();
      return 1;
    }
  else if ( rawkey == 'w' )
  {
     if ( _wipe_dir == kNoWipe )  {
	_wipe_dir = kWipeVertical;
	_wipe = fltk::event_x() / float( w() );
	window()->cursor(fltk::CURSOR_WE);
     }
     else if ( _wipe_dir & kWipeVertical )
     {
	_wipe_dir = kWipeHorizontal;
	_wipe = (h() - fltk::event_y()) / float( h() );
	window()->cursor(fltk::CURSOR_NS);
     }
     else if ( _wipe_dir & kWipeHorizontal ) {
	_wipe_dir = kNoWipe;
	_wipe = 0.0f;
	window()->cursor(fltk::CURSOR_CROSS);
     }

     redraw();
     return 1;
  }
  else if ( rawkey == 'x' )
  {
     _flip = (FlipDirection)( (int) _flip ^ (int)kFlipVertical );
     redraw();
     return 1;
  }
  else if ( rawkey == 'y' )
  {
     _flip = (FlipDirection)( (int) _flip ^ (int)kFlipHorizontal );
     redraw();
     return 1;
  }
  else if ( rawkey == fltk::LeftKey || rawkey == fltk::Keypad4 ) 
    {
      if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
	   fltk::event_key_state( fltk::RightCtrlKey ) )
	{
	  mrv::media fg = foreground();
	  if ( ! fg ) return 1;
	  
	  const CMedia* img = fg->image();

	  double fps = 24;
	  if ( img ) fps = img->play_fps();

	  step_frame( int64_t(-fps) );
	}
      else
	{
	  step_frame( -1 );
	}
      return 1;
    }
  else if ( rawkey == fltk::RightKey || rawkey == fltk::Keypad6 ) 
    {
      if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
	   fltk::event_key_state( fltk::RightCtrlKey ) )
	{
	  mrv::media fg = foreground();
	  if ( ! fg ) return 1;
	  
	  const CMedia* img = fg->image();

	  double fps = 24;
	  if ( img ) fps = img->play_fps();

	  step_frame( int64_t(fps) );
	}
      else
	{
	  step_frame( 1 );
	}
      return 1;
    }
  else if ( rawkey == fltk::UpKey || rawkey == fltk::Keypad8 ) 
    {
      mrv::media fg = foreground();
      if ( ! fg ) return 1;

      const CMedia* img = fg->image();

      double FPS = 24;
      if ( img ) FPS = img->play_fps();

      if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
	   fltk::event_key_state( fltk::RightCtrlKey ) )
	fps( FPS * 2 );
      else if ( fltk::event_key_state( fltk::LeftShiftKey ) ||
		fltk::event_key_state( fltk::RightShiftKey )  )
	fps( FPS / 2 );
      else
	fps( FPS );
      play_backwards();
      return 1;
    }
  else if ( rawkey == fltk::DownKey || rawkey == fltk::Keypad2 ) 
    {
      mrv::media fg = foreground();
      if ( ! fg ) return 1;

      const CMedia* img = fg->image();
      double FPS = 24;
      if ( img ) FPS = img->play_fps();

      if ( fltk::event_key_state( fltk::LeftCtrlKey ) ||
	   fltk::event_key_state( fltk::RightCtrlKey ) )
	fps( FPS * 2 );
      else if ( fltk::event_key_state( fltk::LeftShiftKey ) ||
		fltk::event_key_state( fltk::RightShiftKey ) )
	fps( FPS / 2 );
      else
	fps( FPS );
      play_forwards();
      return 1;
    }
  else if ( rawkey == fltk::ReturnKey || rawkey == fltk::Keypad5 ) 
    {
      stop();
      return 1;
    }
  else if ( rawkey == fltk::PageUpKey ) 
    {
      previous_image_cb(this, browser());
      mouseMove( fltk::event_x(), fltk::event_y() );
      return 1;
    }
  else if ( rawkey == fltk::PageDownKey ) 
    {
      next_image_cb(this, browser());
      mouseMove( fltk::event_x(), fltk::event_y() );
      return 1;
    }
  else if ( rawkey == fltk::HomeKey ) 
    {
      if ( fltk::event_key_state( fltk::LeftCtrlKey )  ||
	   fltk::event_key_state( fltk::RightCtrlKey ) )
	first_frame_timeline();
      else
	first_frame();
      mouseMove( fltk::event_x(), fltk::event_y() );
      return 1;
    }
  else if ( rawkey == fltk::EndKey ) 
    {
      if ( fltk::event_key_state( fltk::LeftCtrlKey )  ||
	   fltk::event_key_state( fltk::RightCtrlKey ) )
	last_frame_timeline();
      else
	last_frame();
      mouseMove( fltk::event_x(), fltk::event_y() );
      return 1;
    }
  else if ( rawkey == fltk::TabKey ) 
    {
      toggle_background();
      mouseMove( fltk::event_x(), fltk::event_y() );
      return 1;
    }
  else if ( rawkey == fltk::F1Key )
    {
      if ( uiMain->uiTopBar->visible() ) uiMain->uiTopBar->hide();
      else uiMain->uiTopBar->show();
      uiMain->uiRegion->relayout( fltk::LAYOUT_XYWH |
				  fltk::LAYOUT_DAMAGE |
				  fltk::LAYOUT_CHILD );
      uiMain->uiRegion->redraw();
      return 1;
    }
  else if ( rawkey == fltk::F2Key )
    {
      if ( uiMain->uiPixelBar->visible() ) uiMain->uiPixelBar->hide();
      else uiMain->uiPixelBar->show();
      uiMain->uiRegion->relayout( fltk::LAYOUT_XYWH |
				  fltk::LAYOUT_DAMAGE |
				  fltk::LAYOUT_CHILD );
      uiMain->uiRegion->redraw();
      return 1;
    }
  else if ( rawkey == fltk::F3Key )
    {
      if ( uiMain->uiBottomBar->visible() ) uiMain->uiBottomBar->hide();
      else uiMain->uiBottomBar->show();
      uiMain->uiRegion->relayout( fltk::LAYOUT_XYWH |
				  fltk::LAYOUT_DAMAGE |
				  fltk::LAYOUT_CHILD );
      uiMain->uiRegion->redraw();
      return 1;
    }
  else if ( rawkey == fltk::F12Key )
    {
      toggle_fullscreen();
      return 1;
    }
  else
    {
      // Check if a menu shortcut
      fltk::PopupMenu* uiColorChannel = uiMain->uiColorChannel;

      // check if a channel shortcut
      int num = uiColorChannel->children();
      for ( int c = 0; c < num; ++c )
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

/** 
 * Toggle between a fullscreen view and a normal view with window borders.
 * 
 */
void ImageView::toggle_fullscreen()
{
  fltk::Window* uiImageInfo = uiMain->uiImageInfo->uiMain;
  fltk::Window* uiColorArea = uiMain->uiColorArea->uiMain;
  fltk::Window* uiReel  = uiMain->uiReelWindow->uiMain;
  fltk::Window* uiPrefs = uiMain->uiPrefs->uiMain;
  fltk::Window* uiAbout = uiMain->uiAbout->uiMain;

  static bool has_image_info, has_color_area, has_reel, has_prefs, has_about,
    has_top_bar, has_bottom_bar, has_pixel_bar;
  if ( fltk_main()->border() )
    {
      posX = fltk_main()->x();
      posY = fltk_main()->y();

      has_image_info = uiImageInfo->visible();
      has_color_area = uiColorArea->visible();
      has_reel       = uiReel->visible();
      has_prefs      = uiPrefs->visible();
      has_about      = uiAbout->visible();
      has_top_bar    = uiMain->uiTopBar->visible();
      has_bottom_bar = uiMain->uiBottomBar->visible();
      has_pixel_bar  = uiMain->uiPixelBar->visible();

      uiImageInfo->hide();
      uiReel->hide();
      uiColorArea->hide();
      uiAbout->hide();
      uiPrefs->hide();
      uiMain->uiTopBar->hide();
      uiMain->uiBottomBar->hide();
      uiMain->uiPixelBar->hide();

      fltk_main()->fullscreen();
    }
  else
    {
      if ( has_image_info ) uiImageInfo->show();
      if ( has_color_area ) uiColorArea->show();
      if ( has_reel  )      uiReel->show();
      if ( has_prefs )      uiPrefs->show();
      if ( has_about )      uiAbout->show();

      if ( has_top_bar )    uiMain->uiTopBar->show();
      if ( has_bottom_bar)  uiMain->uiBottomBar->show();
      if ( has_pixel_bar )  uiMain->uiPixelBar->show();

      resize_main_window();
      fltk_main()->show();
    }


  fltk::check();
  
  // fltk_main()->take_focus();
  //   window()->take_focus();
  //   fltk::focus( fltk_main() );

  take_focus();

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
      leftMouseDown(fltk::event_x(), fltk::event_y());
      break;
    case fltk::RELEASE:
      leftMouseUp(fltk::event_x(), fltk::event_y());
      break;
    case fltk::MOVE:
       if ( _wipe_dir != kNoWipe )
       {
	  switch( _wipe_dir )
	  {
	     case kWipeVertical:
		_wipe = fltk::event_x() / (float)w();
		window()->cursor(fltk::CURSOR_WE);
		break;
	     case kWipeHorizontal:
		_wipe = (h() - fltk::event_y()) / (float)h();
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
	  float dx = (fltk::event_x() - lastX) / 10.0f;
	  if ( std::abs(dx) > 1.0 )
	    { 
	      scrub( dx );
	      lastX = fltk::event_x();
	    }
	}
      else
	{
	  mouseMove(fltk::event_x(), fltk::event_y());
	}
      break;
    case fltk::DRAG:
      mouseDrag(fltk::event_x(), fltk::event_y());
      break;
      //     case fltk::SHORTCUT:
    case fltk::KEY:
      lastX = fltk::event_x();
      lastY = fltk::event_y();
      focus(this);
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

  return 1;
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
//       if ( playback() != kStopped ) 
// 	{
// // 	  cerr << img->filename() << " curr: " << frame() << endl;
// // 	  frame( frame() );
// 	  img->play( (CMedia::Playback) playback(), uiMain );
// 	}
      img->refresh();
    }

  mrv::media bg = background();
  if ( bg )
    {
      CMedia* img = bg->image();
//       if ( playback() != kStopped ) 
// 	{
// // 	  frame( frame() );
// 	  img->play();
// 	}
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
	  img->clear_sequence();
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
	  img->clear_sequence();
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
  
  const char* lbl = uiColorChannel->child(c)->label();
  std::string channelName( lbl );


  static std::string oldChannel;

  std::string ext = channelName;
  size_t pos = ext.rfind('.');

  size_t pos2 = oldChannel.rfind('.');

  if ( pos != std::string::npos )
  {
     ext = ext.substr( pos+1, ext.size() );
  }

  if ( pos != pos2 && channelName.size() > oldChannel.size() )
  {
     pos2 = channelName.find( oldChannel );
     if ( pos2 == std::string::npos ) ext = ""; 
  }
  else
  {
     if ( pos != pos2 )
     {
	pos2 = oldChannel.find( channelName );
	if ( pos2 == std::string::npos ) ext = ""; 
     }
     else
     {
	std::string temp1 = channelName.substr( 0, pos );
	std::string temp2 = oldChannel.substr( 0, pos2 );
	if ( temp1 != temp2 ) ext = ""; 
     }
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
  
  mrv::media fg = foreground();
  mrv::media bg = background();

  if ( ext != N_("R") && ext != N_("G") && ext != N_("B") &&
       ext != N_("A") )
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

  static char tmp[10];
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
  float v = base + int( -e );

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
  char m[20];
  sprintf( m, "%+1.1f  f/%1.1f", exposure, fstop );
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
  mrv::image_type_ptr pic;
  {
    CMedia::Mutex& m = img->video_mutex();
    SCOPED_LOCK(m);
    pic = img->hires();
  }
  if (!pic) return;

  image_coordinates( pic, xf, yf );

  zoom( z );

  int w2 = pic->width()  / 2;
  int h2 = pic->height() / 2;

  xoffset = w2 - xf;
  int   h = pic->height();
  yoffset = h2 - ( h - yf - 1);
  xoffset -= (offx / _zoom);
  double ratio = 1.0f;
  if ( _showPixelRatio ) ratio = img->pixel_ratio();
  yoffset += (offy / _zoom * ratio);
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
      uiColorChannel->add("(no image)");
      uiColorChannel->label("(no image)");
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
	 _old_channel = v;
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
      channel( v );
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

#if 0
  if ( old && playback() != kStopped )
    {
      old->image()->stop();
    }
#endif

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
      if ( img->gamma() > 0.0f ) gamma( img->gamma() );
      refresh_fstop();

      if ( img->width() > 160 && !fltk_main()->border() )  fit_image();

      img->image_damage( img->image_damage() | CMedia::kDamageContents );


      bool reload = (bool) uiMain->uiPrefs->uiPrefsAutoReload->value();
      if ( dynamic_cast< stubImage* >( img ) || reload )
	{
	  create_timeout( 0.2 );
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

  CMedia* img = fg->image();

  unsigned int numAudioTracks = uiMain->uiAudioTracks->children();
  if ( idx >= numAudioTracks - 1 )
    img->audio_stream( -1 );
  else
    img->audio_stream( idx );
}


/** 
 * Change current background image
 * 
 * @param img new background image or NULL
 */
void ImageView::background( mrv::media bg )
{
  if ( bg == _bg ) return;

  delete_timeout();

  _bg = bg;
  if ( bg ) 
    {
      CMedia* img = bg->image();

      img->volume( _volume );
      if ( playback() != kStopped ) 
	img->play( (CMedia::Playback) playback(), uiMain );
      else 
	img->refresh();

      img->image_damage( img->image_damage() | CMedia::kDamageContents );      

      bool reload = (bool) uiMain->uiPrefs->uiPrefsAutoReload->value();
      if ( dynamic_cast< stubImage* >( img ) || reload )
	{
	  create_timeout( 0.2 );
	}
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
  int miny = monitor.work.y();
  int maxh = monitor.work.h();
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
  fltk_main()->border( true );

  if ( fit ) fit_image();
}


/** 
 * Toggle background image.
 * 
 */
void ImageView::toggle_background()
{
  _showBG = !_showBG;
  damage_contents();
  redraw();
}

/**
 * Returns normalize state
 * 
 */
bool ImageView::normalize() const
{
  return (bool) uiMain->uiNormalize->value();
}

void ImageView::damage_contents()
{
  mrv::media fg = foreground();
  mrv::media bg = background();

  if (fg)
    {
      CMedia* img = fg->image();
      img->image_damage( img->image_damage() | CMedia::kDamageContents );
    }

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
  _normalize = !_normalize;
  damage_contents();
  redraw();
}


/** 
 * Toggle pixel ratio stretching
 * 
 */
void ImageView::toggle_pixel_ratio()
{
  _showPixelRatio = !_showPixelRatio;
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

/// Returns current frame number in view
int64_t ImageView::frame() const
{
  return uiMain->uiFrame->frame();
}

/** 
 * Change view's frame number
 * 
 * @param f new frame
 */
void ImageView::frame( const int64_t f )
{
  // Hmmm... this is somewhat inefficient.  Would be better to just
  // change fg/bg position
  browser()->frame( f );
}



/** 
 * Change view's frame number
 * 
 * @param f new frame
 */
void ImageView::seek( const int64_t f )
{

  // Hmmm... this is somewhat inefficient.  Would be better to just
  // change fg/bg position
  browser()->seek( f );
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
  uiMain->uiFrame->frame(f);
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
      f = timeline()->location(img);
      if ( uiMain->uiFrame->value() == f )
	{
	  browser()->previous_image();
	  last_frame();
	  return;
	}
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
      f += timeline()->location(img);
      if ( uiMain->uiFrame->value() == f )
	{
	  browser()->next_image();
	  return;
	}
    }

  int64_t t = int64_t( timeline()->maximum() );
  if ( t < f ) f = t;

  seek( f );
}

/// Change frame number to end of timeline
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
  play( CMedia::kForwards );
}

/** 
 * Play image sequence backwards.
 * 
 */
void ImageView::play( const CMedia::Playback dir ) 
{ 
  playback( (Playback) dir );

  delete_timeout();
  double fps = uiMain->uiFPS->fvalue();

  create_timeout( 1.0/fps*2 );

  mrv::media fg = foreground();
  if ( fg ) 
    {
      fg->image()->play( dir, uiMain);
    }

  mrv::media bg = background();
  if ( bg ) bg->image()->play( dir, uiMain);


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
  _playback = kStopped;
  _last_fps = 0.0;
  _real_fps = 0.0;

  stop_playback();

  uiMain->uiPlayForwards->value(0);
  uiMain->uiPlayBackwards->value(0);

  delete_timeout();
  redraw();
  thumbnails();
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

//   timeline()->fps( x );
//   uiMain->uiFrame->fps( x );
//   uiMain->uiStartFrame->fps( x );
//   uiMain->uiEndFrame->fps( x );
  uiMain->uiFPS->value( x );
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
}

/// Set Playback looping mode
void  ImageView::looping( Looping x )
{
  _looping = x;
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

  damage_contents();
  redraw();
}


} // namespace mrv

