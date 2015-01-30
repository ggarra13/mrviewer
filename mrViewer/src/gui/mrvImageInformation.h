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
 * @file   ImageInformation.h
 * @author gga
 * @date   Fri Jan 26 07:53:52 2007
 * 
 * @brief  Area showing image information, with a list of requesters
 * 
 * 
 */

#ifndef mrvImageInformation_h
#define mrvImageInformation_h

#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

#include <fltk/ScrollGroup.h>

#include "core/mrvRectangle.h"
#include "core/mrvString.h"
#include "gui/mrvBrowser.h"
#include "gui/mrvCollapsableGroup.h"


namespace fltk
{
  class PackedGroup;
  class ItemGroup;
  class PopupMenu;
}


#define ImageInfoParent fltk::ScrollGroup


namespace mrv
{
  class ViewerUI;
  class CMedia;
  class CtlLMTData;

  class ImageInformation : public ImageInfoParent
  {
  public:
    ImageInformation( int x, int y, int w, int h, const char* l = NULL );

    CMedia* get_image() { return img; };
    void set_image( CMedia* img );

    void refresh();
    virtual void layout();
    virtual int handle( int event );

    void main( mrv::ViewerUI* m ) { uiMain = m; }

  protected:
    fltk::Color get_title_color();
    fltk::Color get_widget_color();

    void hide_tabs();

    static void ctl_callback( fltk::Widget* t, ImageInformation* v );
    static void ctl_lmt_callback( fltk::Widget* t, CtlLMTData* v );
    static void ctl_idt_callback( fltk::Widget* t, ImageInformation* v );
    static void icc_callback( fltk::Widget* t, ImageInformation* v );
    static void compression_cb( fltk::PopupMenu* t, ImageInformation* v );
    static void enum_cb( fltk::PopupMenu* w, ImageInformation* v );

    static void toggle_tab( fltk::Widget* w, void* data );
    static void int_slider_cb( fltk::Slider* w, void* data );
    static void float_slider_cb( fltk::Slider* w, void* data );

    int64_t to_memory( int64_t value, const char*& extension );

    mrv::Browser* add_browser( mrv::CollapsableGroup* g );

    void add_icc( const char* name, const char* content, 
		  const bool editable = true, 
		  fltk::Callback* callback = NULL );

    void add_ctl( const char* name, const char* content, 
		  const bool editable = true, 
		  fltk::Callback* callback = NULL );

    void add_ctl_lmt( const char* name, const char* content, 
                      const size_t idx,
                      const bool editable = true, 
                      fltk::Callback* callback = NULL );

    void add_ctl_idt( const char* name, const char* content, 
                      const bool editable = true, 
                      fltk::Callback* callback = NULL );


    void add_text( const char* name, const char* content, 
		   const bool editable = false, 
		   fltk::Callback* callback = NULL );
    void add_text( const char* name, const std::string& content, 
		   const bool editable = false,
		   fltk::Callback* callback = NULL );
    void add_float( const char* name, const float content, 
		    const bool editable = false, 
		    fltk::Callback* callback = NULL, 
		    const float minV = 0.0f, const float maxV = 1.0f );
    void add_rect( const char* name, const mrv::Recti& content, 
		   const bool editable = false, 
		   fltk::Callback* callback = NULL );

    void add_time( const char* name, const double content, 
		    const bool editable = false );

    void add_enum( const char* name, const size_t content, 
		   const char** options,
		   const size_t num, const bool editable = false,
		   fltk::Callback* callback = NULL );

    void add_enum( const char* name, const std::string& content, 
		   stringArray& options, const bool editable = false,
		   fltk::Callback* callback = NULL );

    void add_int64( const char* name, const int64_t content );

    void add_int( const char* name, const int content, 
		  const bool editable = false,
		  fltk::Callback* callback = NULL,
		  const int minV = -9999, const int maxV = 9999 );
    void add_int( const char* name, const unsigned int content, 
		  const bool editable = false, 
		  fltk::Callback* callback = NULL,
		  const unsigned int minV = 0, const unsigned int maxV = 9999 );
    void add_bool( const char* name, const bool content, 
		   const bool editable = false,
		   fltk::Callback* callback = NULL );

    int line_height();


    ViewerUI*    uiMain;
    CMedia*   img;

    mrv::CollapsableGroup*       m_image;
    mrv::CollapsableGroup*       m_video;
    mrv::CollapsableGroup*       m_audio;
    mrv::CollapsableGroup*       m_subtitle;
    mrv::CollapsableGroup*       m_iptc;
    mrv::CollapsableGroup*       m_exif;

    fltk::PackedGroup* m_all;
    fltk::PackedGroup* m_main;
    mrv::Browser*      m_curr;
    fltk::Color        m_color;
    unsigned int group;
    unsigned int row;
  };

} // namespace mrv


#endif
