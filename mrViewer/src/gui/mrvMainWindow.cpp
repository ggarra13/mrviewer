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
 * @file   mrvMainWindow.cpp
 * @author gga
 * @date   Wed Jul  4 23:17:46 2007
 * 
 * @brief  main window for mrViewer
 * 
 * 
 */

#include <iostream>

#include "core/mrvACES.h"
#include "core/CMedia.h"
#include "core/mrvI8N.h"
#include "core/stubImage.h"
#include "gui/mrvHotkey.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"
#include "gui/mrvColorInfo.h"
#include "gui/mrvImageInformation.h"
#include "gui/mrvFileRequester.h"
#include "gui/mrvMainWindow.h"
#include "gui/mrvIO.h"
// Must come before fltk/x11.h
#include "mrViewer.h"
#include "mrvAudioOffset.h"
#include "mrvIccProfileUI.h"
#include "mrvEDLWindowUI.h"
#include "mrvColorAreaUI.h"

// #include <FL/Fl_Cursor.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Enumerations.H>
#include <FL/Fl.H>


#if !defined(_WIN32)
#  include <FL/x.H>
#  include <X11/xpm.h>
#  include "icons/viewer16.xpm"
#else
#  include <windows.h>
#  include <FL/win32.H>
#  include "resource.h"
#endif

extern void clone_all_cb( Fl_Widget* o, mrv::ImageBrowser* b );
extern void clone_image_cb( Fl_Widget* o, mrv::ImageBrowser* b );

namespace {
const char* kModule = "menu"; 
}

void open_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow )
{
  uiReelWindow->open();
}

void open_single_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow )
{
  uiReelWindow->open_single();
}

void preload_image_cache_cb( Fl_Widget* o, mrv::ImageView* v )
{
    v->preload_caches();
}

void clear_image_cache_cb( Fl_Widget* o, mrv::ImageView* v )
{
    v->clear_caches();
}

void next_image_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
  b->next_image();
}

void previous_image_cb( Fl_Widget* o, mrv::ImageBrowser* b )
{
  b->previous_image();
}

void next_channel_cb( Fl_Widget* o, mrv::ImageView* v )
{
  v->next_channel();
}

void previous_channel_cb( Fl_Widget* o, mrv::ImageView* v )
{
  v->previous_channel();
}


void set_as_background_cb( Fl_Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( !fg ) return;

  view->background( fg );
}

void toggle_background_cb( Fl_Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( !fg ) return;
  view->toggle_background();
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

void save_clip_xml_metadata_cb( Fl_Widget* o, 
                                mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( !fg ) return;

  mrv::CMedia* img = fg->image();
  save_clip_xml_metadata( img );
}

void save_cb( Fl_Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( !fg ) return;

  view->browser()->save();
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

  mrv::save_sequence_file( view->main(), NULL, true );

  // Return all to normal
  view->redraw();

}

void save_sequence_cb( Fl_Widget* o, mrv::ImageView* view )
{
  view->stop();
  view->browser()->save_sequence();
}

void window_cb( Fl_Widget* w, const ViewerUI* uiMain )
{
  Fl_Menu_* mw = (Fl_Menu_*)w;
  const Fl_Menu_Item* o = mw->mvalue();
  uiMain->uiMain->take_focus();

   int idx = -1;
   std::string menu = o->label();


   if ( menu == _("Reels") )
    {
       // Reel window
      uiMain->uiReelWindow->uiMain->show();
    }
   else if ( menu == _("Media Info") )
  {
       // Media Info
      uiMain->uiImageInfo->uiMain->show();
      uiMain->uiView->update_image_info();
      uiMain->uiView->send( "MediaInfoWindow 1" );
  }
   else if ( menu == _("Color Info") )
    {
       // Color Area
      uiMain->uiColorArea->uiMain->show();
      uiMain->uiView->update_color_info();
      uiMain->uiView->send( "ColorInfoWindow 1" );
    }
   else if ( menu == _("EDL Edit") )
  {
     uiMain->uiReelWindow->uiBrowser->set_edl();
     // uiMain->uiEDLWindow->uiMain->child_of( uiMain->uiMain );
     uiMain->uiEDLWindow->uiMain->show();
  }
   else if ( menu == _("3dView") )
  {
      uiMain->uiGL3dView->uiMain->show();
      uiMain->uiView->send( "GL3dView 1" );
  }
  else if ( menu == _("Paint Tools") )
    {
       // Paint Tools
      // uiMain->uiPaint->uiMain->child_of( uiMain->uiMain );
      uiMain->uiPaint->uiMain->show();
    }
  else if ( menu == _("Histogram") )
    {
      uiMain->uiHistogram->uiMain->show();
      uiMain->uiView->send( "HistogramWindow 1" );
    }
  else if ( menu == _("Vectorscope") )
    {
      uiMain->uiVectorscope->uiMain->show();
      uiMain->uiView->send( "VectorscopeWindow 1" );
    }
  else if ( menu == _("ICC Profiles") )
    {
      uiMain->uiICCProfiles->uiMain->show();
      uiMain->uiICCProfiles->fill();
    }
  else if ( menu == _("Connections") )
    {
      // uiMain->uiConnection->uiMain->child_of( uiMain->uiMain );
      uiMain->uiConnection->uiMain->show();
    }
  else if ( menu == _("Preferences") )
    {
      // uiMain->uiPrefs->uiMain->child_of( uiMain->uiMain );
      uiMain->uiPrefs->uiMain->show();
    }
  else if ( menu == _("Hotkeys") )
    {
      // uiMain->uiHotkey->uiMain->child_of( uiMain->uiMain );
      uiMain->uiHotkey->uiMain->show();
    }
  else if ( menu == _("Logs") )
    {
      // uiMain->uiLog->uiMain->child_of( uiMain->uiMain );
      uiMain->uiLog->uiMain->show();
    }
  else if ( menu == _("About") )
    {
      uiMain->uiAbout->uiMain->show();
    }
  else
    {
       const char* name = o->label();
       LOG_ERROR( _("Unknown Window \"") << name << "\"" );
    }
}

void masking_cb( Fl_Widget* w, ViewerUI* uiMain )
{
  Fl_Menu_* mw = (Fl_Menu_*)w;
  const Fl_Menu_Item* o = mw->mvalue();

  mrv::ImageView* view = uiMain->uiView;

  float mask = 1.0f;
  const char* fmt = o->label();


  mask = (float) atof( fmt );

  char buf[128];
  sprintf( buf, "Mask %g", mask );
  view->send( buf );

  view->masking( mask );


  view->redraw();
}

void change_subtitle_cb( Fl_Widget* o, mrv::ImageView* view )
{
   Fl_Group* g = o->parent();
   if ( !g ) return;

   Fl_Menu_Button* p = dynamic_cast< Fl_Menu_Button* >( g );
   if ( !p ) return;

   if ( !view) return;

   mrv::media fg = view->foreground();
   if ( !fg ) return;

   int i = (int)p->value() - 1;
   fg->image()->subtitle_stream(i);

}



void hud_cb( Fl_Widget* w, ViewerUI* uiMain )
{
  mrv::ImageView* view = uiMain->uiView;

  Fl_Menu_* mw = (Fl_Menu_*)w;
  const Fl_Menu_Item* o = mw->mvalue();

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

void attach_color_profile_cb( Fl_Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( ! fg ) return;

  attach_icc_profile( fg->image() );
}

void flip_x_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::ImageView::FlipDirection dir = view->flip();
    view->flip( (mrv::ImageView::FlipDirection)
                ( dir ^ mrv::ImageView::kFlipVertical ) );
    view->redraw();
}

void flip_y_cb( Fl_Widget* o, mrv::ImageView* view )
{
    mrv::ImageView::FlipDirection dir = view->flip();
    view->flip( (mrv::ImageView::FlipDirection) 
                ( dir ^ mrv::ImageView::kFlipHorizontal ) );
    view->redraw();
}

void attach_ctl_script_cb( Fl_Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( ! fg ) return;

  attach_ctl_script( fg->image() );
}


void attach_ctl_lmt_script_cb( Fl_Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( ! fg ) return;

  mrv::CMedia* img = fg->image();

  mrv::attach_ctl_lmt_script( img, img->number_of_lmts() );
}

void attach_ctl_idt_script_cb( Fl_Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( ! fg ) return;

  mrv::attach_ctl_idt_script( fg->image() );
}

void monitor_icc_profile_cb( Fl_Widget* o, void* data )
{
    mrv::monitor_icc_profile();
}

void monitor_ctl_script_cb( Fl_Widget* o, void* data )
{
    mrv::monitor_ctl_script();
}

void copy_pixel_rgba_cb( Fl_Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( ! fg ) return;

  view->copy_pixel();
}

void attach_audio_cb( Fl_Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( !fg ) return;

  const char* file = mrv::open_audio_file();
  if ( file == NULL ) return;

  mrv::CMedia* img = fg->image();
  if ( img == NULL ) return;

  img->audio_file( file );
  view->refresh_audio_tracks();

}


void detach_audio_cb( Fl_Widget* o, mrv::ImageView* view )
{
  mrv::media fg = view->foreground();
  if ( !fg ) return;

  view->stop();

  mrv::CMedia* img = fg->image();
  if ( img == NULL ) return;

  img->audio_file( NULL );
  view->refresh_audio_tracks();

}


using namespace std;

namespace mrv {

  MainWindow::MainWindow( int W, int H, const char* title ) :
  Fl_Double_Window( W, H, title ),
  m( NULL )
  {
  }

  MainWindow::~MainWindow()
  {
      DBG( "*****************************************************" );
      uiMain->uiView->stop();
  }

mrv::ImageBrowser* MainWindow::browser() 
{ 
    return uiMain->uiReelWindow->uiBrowser;
}

mrv::ImageView*    MainWindow::view() 
{ 
    return uiMain->uiView; 
}

void MainWindow::setup_menu()
{
    if ( m )
    {
        m->clear();
    }
    else 
    {
        m = new Fl_Menu_Button(0,0,0,0);
        m->type( Fl_Menu_Button::POPUP3 );
        this->add( m );
    }
    
    m->add( _("File/Open/Movie or Sequence"), kOpenImage.hotkey(),
            (Fl_Callback*)open_cb, browser(), 0 ); 
    m->add( _("File/Open/Single Image"), kOpenSingleImage.hotkey(),
            (Fl_Callback*)open_single_cb, browser(), 0 );

    mrv::media fg = view()->foreground();
    if ( fg )
    {
        m->add( _("File/Open/Stereo Sequence or Movie"),
                kOpenStereoImage.hotkey(),
                (Fl_Callback*)open_stereo_cb, browser(), 0 );
        m->add( _("File/Open/Clip XML Metadata"),
                kOpenClipXMLMetadata.hotkey(),
                (Fl_Callback*)open_clip_xml_metadata_cb, view(), 0 );
        m->add( _("File/Save/Movie or Sequence As"), 
                       kSaveSequence.hotkey(),
                       (Fl_Callback*)save_sequence_cb, view(), 0 ); 
	    m->add( _("File/Save/Reel As"), kSaveReel.hotkey(),
                       (Fl_Callback*)save_reel_cb, view(), 0 ); 
	    m->add( _("File/Save/Frame As"), kSaveImage.hotkey(),
                       (Fl_Callback*)save_cb, view(), 0  ); 
	    m->add( _("File/Save/GL Snapshots As"), kSaveSnapshot.hotkey(),
                       (Fl_Callback*)save_snap_cb, view(), 0 ); 
	    m->add( _("File/Save/Clip XML Metadata As"),
                       kSaveClipXMLMetadata.hotkey(),
                       (Fl_Callback*)save_clip_xml_metadata_cb, view(), 0 ); 
	 }

	 char buf[256];
	 const char* tmp; 
	 int item;
	 int num = uiMain->uiWindows->size()-1;
	 int i;
	 for ( i = 0; i < num; ++i )
	 {
	    tmp = uiMain->uiWindows->text(i);
	    sprintf( buf, _("Windows/%s"), tmp );
	    m->add( buf, 0, (Fl_Callback*)window_cb, uiMain, 0 );
	 }

	 if ( fg && fg->image()->has_picture() )
	 {

	    m->add( _("View/Safe Areas"), kSafeAreas.hotkey(), 
                       (Fl_Callback*)safe_areas_cb, view(), 0 );

            m->add( _("View/Display Window"), kDisplayWindow.hotkey(), 
                       (Fl_Callback*)display_window_cb, view(), 0 );

	    m->add( _("View/Data Window"), kDataWindow.hotkey(), 
                       (Fl_Callback*)data_window_cb, view(), 0 );

	    num = uiMain->uiPrefs->uiPrefsCropArea->size()-1;
	    for ( i = 0; i < num; ++i )
	    {
	       tmp = uiMain->uiPrefs->uiPrefsCropArea->text(i);
	       sprintf( buf, _("View/Mask/%s"), tmp );
	       item = m->add( buf, 0, (Fl_Callback*)masking_cb, uiMain, 
                                 FL_MENU_TOGGLE );
	       float mask = -1.0f;
               mask = (float) atof( tmp );
	       if ( mask == view()->masking() ) m->mode( item, 
                                                       FL_MENU_TOGGLE|
                                                       FL_MENU_VALUE );
               else m->mode( item, FL_MENU_TOGGLE );
	    }


	    num = uiMain->uiPrefs->uiPrefsHud->children();
	    for ( i = 0; i < num; ++i )
	    {
	       tmp = uiMain->uiPrefs->uiPrefsHud->child(i)->label();
	       sprintf( buf, _("View/Hud/%s"), tmp );
	       item = m->add( buf, 0, (Fl_Callback*)hud_cb, uiMain, 
                                FL_MENU_TOGGLE );
	       if ( view()->hud() & (1 << i) ) m->mode( item, 
                                                      FL_MENU_TOGGLE|
                                                      FL_MENU_VALUE );
	    }
	   
      	    m->add( _("Image/Next"), kNextImage.hotkey(), 
                       (Fl_Callback*)next_image_cb, browser(), 0 );
	    m->add( _("Image/Previous"), kPreviousImage.hotkey(), 
		      (Fl_Callback*)previous_image_cb, 
		      browser(), FL_MENU_DIVIDER);

	    const stubImage* img = dynamic_cast< const stubImage* >( image() );
	    if ( img )
	    {
	       m->add( _("Image/Clone"), kCloneImage.hotkey(),
			(Fl_Callback*)clone_image_cb, browser());
	       m->add( _("Image/Clone All Channels"), 0,
			(Fl_Callback*)clone_all_cb,
			browser(), FL_MENU_DIVIDER);
	    }
	    else
	    {
	       m->add( _("Image/Clone"), kCloneImage.hotkey(),
			(Fl_Callback*)clone_image_cb, browser(),
			FL_MENU_DIVIDER );
	    }
 

            item = m->add( _("Image/Preload Caches"), kPreloadCache.hotkey(),
                             (Fl_Callback*)preload_image_cache_cb, view(), 
                             FL_MENU_TOGGLE);
            if ( mrv::CMedia::preload_cache() ) m->mode( item, 
                                                         FL_MENU_TOGGLE |
                                                         FL_MENU_VALUE );
            else m->mode( item, FL_MENU_TOGGLE );

            m->add( _("Image/Clear Caches"), kClearCache.hotkey(),
                       (Fl_Callback*)clear_image_cache_cb, view(),
                       FL_MENU_DIVIDER );


	    m->add( _("Image/Attach CTL Input Device Transform"),
                       kIDTScript.hotkey(),
                       (Fl_Callback*)attach_ctl_idt_script_cb,
                       view(), 0);
	    m->add( _("Image/Add CTL Look Mod Transform"),
		      kLookModScript.hotkey(),
		      (Fl_Callback*)attach_ctl_lmt_script_cb,
                       view(), 0);
	    m->add( _("Image/Attach CTL Rendering Transform"),
		      kCTLScript.hotkey(),
		      (Fl_Callback*)attach_ctl_script_cb,
		      view(), FL_MENU_DIVIDER);
	    m->add( _("Image/Attach ICC Color Profile"),
		      kIccProfile.hotkey(),
		      (Fl_Callback*)attach_color_profile_cb,
                       view(), FL_MENU_DIVIDER);
	    m->add( _("Image/Mirror/Horizontal"),
		      kFlipX.hotkey(),
		      (Fl_Callback*)flip_x_cb,
                       view(), 0);
	    m->add( _("Image/Mirror/Vertical"),
		      kFlipY.hotkey(),
		      (Fl_Callback*)flip_y_cb,
		      view(), 0);
	    m->add( _("Image/Set as Background"), kSetAsBG.hotkey(),
                       (Fl_Callback*)set_as_background_cb,
                       (void*)view(), 0);
	    m->add( _("Image/Toggle Background"),
		      kToggleBG.hotkey(),
		      (Fl_Callback*)toggle_background_cb, (void*)view(), 0);

	    Image_ptr image = fg->image();

   
	    size_t num = image->number_of_subtitle_streams();
	    if ( num > 0 )
	    {
	       item = m->add( _("Subtitle/No Subtitle"), 0,
                                 (Fl_Callback*)change_subtitle_cb, view(), 
                                 FL_MENU_TOGGLE );
	       if ( image->subtitle_stream() == -1 )
                   m->mode( item, FL_MENU_TOGGLE|FL_MENU_VALUE );

	       for ( unsigned i = 0; i < num; ++i )
	       {
		  char buf[256];
		  sprintf( buf, _("Subtitle/Track #%d - %s"), i,
			   image->subtitle_info(i).language.c_str() );

		  item = m->add( buf, 0, (Fl_Callback*)change_subtitle_cb,
                                    view(), FL_MENU_TOGGLE );
		  if ( image->subtitle_stream() == i )
                      m->mode( item, FL_MENU_TOGGLE|FL_MENU_VALUE );
	       }
	    }

	    if ( 1 )
	    {
   
	       m->add( _("Audio/Attach Audio File"), kAttachAudio.hotkey(),
                          (Fl_Callback*)attach_audio_cb, view(), 0 );
	       m->add( _("Audio/Edit Audio Frame Offset"),
                          kEditAudio.hotkey(), 
                          (Fl_Callback*)edit_audio_cb, view(), 0 );
	       m->add( _("Audio/Detach Audio File"), kDetachAudio.hotkey(),
                          (Fl_Callback*)detach_audio_cb, view(), 0 );
	    }

	    m->add( _("Pixel/Copy RGBA Values to Clipboard"),
                       kCopyRGBAValues.hotkey(),
                       (Fl_Callback*)copy_pixel_rgba_cb, (void*)view(), 0);
	 }


	  m->add( _("Monitor/Attach CTL Display Transform"),
                     kMonitorCTLScript.hotkey(),
                     (Fl_Callback*)monitor_ctl_script_cb,
                     NULL, 0 );
	  m->add( _("Monitor/Attach ICC Color Profile"),
                     kMonitorIccProfile.hotkey(),
                     (Fl_Callback*)monitor_icc_profile_cb,
                     view(), FL_MENU_DIVIDER);
    

}

  void MainWindow::set_icon()
  {
    fl_open_display();  // Needed for icons 
    
#if defined(_WIN32) || defined(_WIN64)
    HICON data = LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1));
    this->icon(data);
#else

    // XpmCreatePixmapFromData comes from libXpm (libgd-xpm* on Debian)
    Pixmap p, mask;
    int ErrorStatus = XpmCreatePixmapFromData(fltk::xdisplay,
                                              DefaultRootWindow(fltk::xdisplay),
                                              (char**)viewer16_xpm,
                                              &p, &mask, 
                                              NULL);

    if ( ErrorStatus == XpmSuccess )
      {
         this->icon((const void*)&p);
      }
#endif

  }


  void MainWindow::always_on_top() 
  {
#if defined(_WIN32) || defined(_WIN64)
    // Microsoft (R) Windows(TM)
    SetWindowPos(fl_xid(this), HWND_TOPMOST, 
		 0, 0, w()+8, h()+27, 0);
#else
    // XOrg / XWindows(TM)
    XEvent ev;
    static const char* const names[2] = { "_NET_WM_STATE", 
					  "_NET_WM_STATE_ABOVE" };
    Atom atoms[ 2 ];
    fl_open_display();
    XInternAtoms(fl_display, (char**)names, 2, False, atoms );
    Atom net_wm_state = atoms[ 0 ];
    Atom net_wm_state_above = atoms[ 1 ];
    ev.type = ClientMessage;
    ev.xclient.window = fl_xid(this);
    ev.xclient.message_type = net_wm_state;
    ev.xclient.format = 32;
    ev.xclient.data.l[ 0 ] = active() ? 1 : 0;
    ev.xclient.data.l[ 1 ] = net_wm_state_above;
    ev.xclient.data.l[ 2 ] = 0;
    XSendEvent(fl_display, 
	       DefaultRootWindow(fl_display),  False, 
	       SubstructureNotifyMask|SubstructureRedirectMask, &ev);
#endif
  } // above_all function


  /** 
   * Iconize all windows
   * 
   */
  void MainWindow::iconize_all()
  {
    Fl_Window* uiReelWindow = uiMain->uiReelWindow->uiMain;
    if (uiReelWindow) uiReelWindow->iconize();
    return Fl_Double_Window::iconize();
  }

  /** 
   * Handle MainWindow's fltk::events
   * 
   * @param event fltk::event enumeration
   * 
   * @return 1 if handled, 0 if not.
   */
  int MainWindow::handle( int event )
  {
    return Fl_Double_Window::handle( event );
  }


void MainWindow::resize(int x, int y, int w, int h)
{
    Fl_Double_Window::resize(x, y, w, h);

   if ( uiMain->uiPrefs->uiPrefsAutoFitImage->value() )
       uiMain->uiView->fit_image();
}

} // namespace mrv
