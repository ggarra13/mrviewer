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
 * @file   mainWindow.h
 * @author gga
 * @date   Fri Jul  6 14:31:50 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvMainWindow_h
#define mrvMainWindow_h


#include <FL/Fl_Double_Window.H>

class ViewerUI;
class Fl_Menu_Button;

extern void open_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow );
extern void open_single_cb( Fl_Widget* o, mrv::ImageBrowser* uiReelWindow );

extern void preload_image_cache_cb( Fl_Widget* o, mrv::ImageView* v );
extern void clear_image_cache_cb( Fl_Widget* o, mrv::ImageView* v );

extern void next_image_cb( Fl_Widget* o, mrv::ImageBrowser* b );
extern void previous_image_cb( Fl_Widget* o, mrv::ImageBrowser* b );

extern void next_channel_cb( Fl_Widget* o, mrv::ImageView* v );
extern void previous_channel_cb( Fl_Widget* o, mrv::ImageView* v );

extern void set_as_background_cb( Fl_Widget* o, mrv::ImageView* view );
extern void toggle_background_cb( Fl_Widget* o, mrv::ImageView* view );
extern void open_clip_xml_metadata_cb( Fl_Widget* o, 
                                       mrv::ImageView* view );
extern void open_stereo_cb( Fl_Widget* o, 
                            mrv::ImageBrowser* uiReelWindow );
extern void save_clip_xml_metadata_cb( Fl_Widget* o, 
                                       mrv::ImageView* view );
extern void save_cb( Fl_Widget* o, mrv::ImageView* view );
extern void save_reel_cb( Fl_Widget* o, mrv::ImageView* view );
extern void save_snap_cb( Fl_Widget* o, mrv::ImageView* view );
extern void save_sequence_cb( Fl_Widget* o, mrv::ImageView* view );

extern void window_cb( Fl_Widget* w, const ViewerUI* uiMain );

extern void masking_cb( Fl_Widget* w, ViewerUI* uiMain );

extern void change_subtitle_cb( Fl_Widget* o, mrv::ImageView* view );

extern void hud_cb( Fl_Widget* w, ViewerUI* uiMain );
extern void safe_areas_cb( Fl_Widget* o, mrv::ImageView* view );

extern void display_window_cb( Fl_Widget* o, mrv::ImageView* view );
extern void data_window_cb( Fl_Widget* o, mrv::ImageView* view );
extern void attach_color_profile_cb( Fl_Widget* o, mrv::ImageView* view );

extern void flip_x_cb( Fl_Widget* o, mrv::ImageView* view );
extern void flip_y_cb( Fl_Widget* o, mrv::ImageView* view );

extern void attach_ctl_script_cb( Fl_Widget* o, mrv::ImageView* view );
extern void attach_ctl_lmt_script_cb( Fl_Widget* o, mrv::ImageView* view );
extern void attach_ctl_idt_script_cb( Fl_Widget* o, mrv::ImageView* view );

extern void monitor_icc_profile_cb( Fl_Widget* o, void* data );

extern void monitor_ctl_script_cb( Fl_Widget* o, void* data );

extern void copy_pixel_rgba_cb( Fl_Widget* o, mrv::ImageView* view );

extern void attach_audio_cb( Fl_Widget* o, mrv::ImageView* view );
extern void detach_audio_cb( Fl_Widget* o, mrv::ImageView* view );

namespace mrv {

class ImageViewer;
class ImageBrowser;


  class MainWindow : public Fl_Double_Window
  {
  public:
    MainWindow( int W, int H, const char* title );
    ~MainWindow();
    
    void main( ViewerUI* m ) { uiMain = m; };

      void setup_menu();
      // @todo: fltk1.3
      // virtual void layout();
    
    virtual int handle( int event );

      virtual void resize( int x, int y, int w, int h );

    /** 
     * Make window appear always on top of others
     * 
     */
    void always_on_top();

    /** 
     * Change window's icon to mrViewer's icon
     * 
     */
    void set_icon();

    /** 
     * Iconize all windows
     * 
     */
    void iconize_all();

    protected:
      mrv::ImageBrowser* browser();
      mrv::ImageView*    view();
    

  protected:
      Fl_Menu_Button* m;
    ViewerUI* uiMain;
  };


} // namespace mrv


#endif  // mrvMainWindow_h




