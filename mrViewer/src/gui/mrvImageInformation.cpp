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
 * @file   mrvImageInformation.cpp
 * @author gga
 * @date   Wed Jul 11 18:47:58 2007
 *
 * @brief  Class used to display information about an image
 *
 *
 */


#include <iostream>
using namespace std;

#include <algorithm>

#include <fltk/events.h>
#include <fltk/Symbol.h>
#include <fltk/Browser.h>
#include <fltk/Button.h>
#include <fltk/FloatInput.h>
#include <fltk/IntInput.h>

#include <fltk/PackedGroup.h>
#include <fltk/Slider.h>
#include <fltk/PopupMenu.h>

#include <fltk/InvisibleBox.h>

#include <fltk/ItemGroup.h>
#include <fltk/PackedGroup.h>

#include <ImfTimeCodeAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfDoubleAttribute.h>
#include <ImfStandardAttributes.h>

#include "mrvImageInformation.h"
#include "mrvFileRequester.h"
#include "mrvPreferences.h"
#include "mrvMath.h"
#include "mrvMedia.h"
#include "mrvImageView.h"
#include "gui/mrvHotkey.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvIPTCGroup.h"
#include "mrViewer.h"
#include "CMedia.h"
#include "core/aviImage.h"
#include "core/exrImage.h"
#include "mrvIO.h"

#include "mrvI8N.h"


namespace {
const char* kModule = "info";
}

namespace mrv
{


struct CtlLMTData
{
    fltk::Widget* widget;
    size_t idx;
};

typedef std::vector< CtlLMTData* > LMTData;

  static const fltk::Color kTitleColors[] = {
    0x608080ff,
    0x808060ff,
    0x606080ff,
    0x608060ff,
    0x806080ff,
  };


  static LMTData widget_data;

  static const unsigned int kSizeOfTitleColors = ( sizeof(kTitleColors) / 
						   sizeof(fltk::Color) );

  static const fltk::Color kRowColors[] = {
    0x202020ff,
    0x303030ff,
  };

  static const unsigned int kSizeOfRowColors = ( sizeof(kRowColors) /
						 sizeof(fltk::Color) );

  static const int kMiddle = 200;

void change_stereo_image( fltk::Button* w, mrv::ImageInformation* info )
{
    static CMedia* last = NULL;
    CMedia* img = info->get_image();
    if ( img->is_stereo() && img != last && img->right_eye() )
    {
        last = img;
        img = img->right_eye();
        info->set_image( img );
        w->label( _("Right View") );
    }
    else
    {
        if ( last )
        {
            info->set_image( last );
            last = NULL;
            w->label( _("Left View") );
        }
    }
    info->refresh();
}

fltk::Choice *uiType=(fltk::Choice *)0;

fltk::Input *uiKey=(fltk::Input *)0;

fltk::Input *uiValue=(fltk::Input *)0;

fltk::Choice *uiKeyRemove=(fltk::Choice *)0;

void cb_OK(fltk::Button*, fltk::Window* v) {
  v->make_exec_return(true);
}

void cb_Cancel(fltk::Button*, fltk::Window* v) {
  v->make_exec_return(false);
}


static void cb_uiType(fltk::Choice* o, void*) {
    std::string type = o->child( o->value() )->label();
    if ( type == _("String") )
        uiValue->text( _("Text") );
    else if ( type == _("Integer") )
        uiValue->text( N_("15") );
    else if ( type == _("Float") || type == _("Double") )
        uiValue->text( _("2.2") );
    else if ( type == _("Timecode") )
        uiValue->text( _("00:00:00:00") );
    else if ( type == _("Vector2 Integer") )
        uiValue->text( N_("2 5") );
    else if ( type == _("Vector2 Float") )
        uiValue->text( _("2.2 5.1") );
    else if ( type == _("Vector3 Integer") )
        uiValue->text( N_("2 5 1") );
    else if ( type == _("Vector3 Float") )
        uiValue->text( _("2.2 5.1 1.4") );
}

static void cb_Quick(fltk::Choice* o, void*) {
  uiKey->text( o->child( o->value() )->label() );
}

fltk::Window* make_iptc_add_window() {
  fltk::Window* w;
   {fltk::Window* o = new fltk::Window(435, 260, _("Add IPTC Attribute"));
    w = o;
    o->shortcut(0xff1b);
    o->begin();
     {fltk::Choice* o = new fltk::Choice(70, 53, 270, 26, _("Quick Selection"));
      o->callback((fltk::Callback*)cb_Quick);
      o->align(fltk::ALIGN_TOP);
      o->begin();
      new fltk::Item(_("Image Name"));
      new fltk::Item(_("Edit Status"));
      new fltk::Item(_("Priority"));
      new fltk::Item(_("Category"));
      new fltk::Item(_("Supplemental Category"));
      new fltk::Item(_("Fixture Identifier"));
      new fltk::Item(_("Keyword"));
      new fltk::Item(_("Release Date"));
      new fltk::Item(_("Release Time"));
      new fltk::Item(_("Special Instructions"));
      new fltk::Item(_("Reference Service"));
      new fltk::Item(_("Reference Date"));
      new fltk::Item(_("Reference Number"));
      new fltk::Item(_("Created Date"));
      new fltk::Item(_("Created Time"));
      new fltk::Item(_("Originating Program"));
      new fltk::Item(_("Program Version"));
      new fltk::Item(_("Object Cycle"));
      new fltk::Item(_("Byline"));
      new fltk::Item(_("Byline Title"));
      new fltk::Item(_("City"));
      new fltk::Item(_("Province State"));
      new fltk::Item(_("Country Code"));
      new fltk::Item(_("Country"));
      new fltk::Item(_("Original Transmission Reference"));
      new fltk::Item(_("Headline"));
      new fltk::Item(_("Credit"));
      new fltk::Item(_("Src"));
      new fltk::Item(_("Copyright String"));
      new fltk::Item(_("Caption"));
      new fltk::Item(_("Local Caption"));
      new fltk::Item(_("Caption Writer"));
      o->end();
    }
     {fltk::Input* o = uiKey = new fltk::Input(70, 101, 270, 27, _("Keyword"));
      o->align(fltk::ALIGN_TOP);
    }
     {fltk::Input* o = uiValue = new fltk::Input(70, 148, 270, 27, _("Value"));
      o->align(fltk::ALIGN_TOP);
    }
     {fltk::Button* o = new fltk::Button(115, 190, 86, 41, _("OK"));
      o->callback((fltk::Callback*)cb_OK, (void*)(w));
    }
     {fltk::Button* o = new fltk::Button(224, 190, 93, 41, _("Cancel"));
      o->callback((fltk::Callback*)cb_Cancel, (void*)(w));
    }
    o->end();
    o->set_modal();
    o->resizable(o);
  }
  return  w;
}

fltk::Window* make_exif_add_window() {
  fltk::Window* w;
   {fltk::Window* o = new fltk::Window(405, 150);
    w = o;
    o->shortcut(0xff1b);
    o->label( _("Add EXIF Attribute") );
    o->begin();
    { fltk::Choice* o = uiType = new fltk::Choice( 10, 30, 192, 25, _("Type") );
        o->align(fltk::ALIGN_TOP);
        o->begin();
        new fltk::Item( _("String") );
        new fltk::Item( _("Integer") );
        new fltk::Item( _("Float") );
        new fltk::Item( _("Double") );
        // new fltk::Item( _("Keycode") );
        // new fltk::Item( _("Matrix33") );
        // new fltk::Item( _("Matrix44") );
        new fltk::Item( _("Timecode") );
        new fltk::Item( _("Vector2 Integer") );
        new fltk::Item( _("Vector2 Float") );
        new fltk::Item( _("Vector3 Integer") );
        new fltk::Item( _("Vector3 Float") );
        o->end();
        o->callback( (fltk::Callback*) cb_uiType, (void*)w );
        o->value( 4 );
    }
    {fltk::Input* o = uiKey = new fltk::Input(10, 70, 192, 25, _("Name"));
        o->text( N_("timecode") );
        o->align(fltk::ALIGN_TOP);
    }
    {fltk::Input* o = uiValue = new fltk::Input(208, 70, 192, 25, _("Value"));
        o->text( N_("00:00:00:00") );
        o->align(fltk::ALIGN_TOP);
    }
     {fltk::Button* o = new fltk::Button(115, 100, 86, 41, _("OK"));
      o->callback((fltk::Callback*)cb_OK, (void*)(w));
    }
     {fltk::Button* o = new fltk::Button(224, 100, 93, 41, _("Cancel"));
      o->callback((fltk::Callback*)cb_Cancel, (void*)(w));
    }
    o->end();
    o->set_modal();
    o->resizable(o);
  }
  return  w;
}

fltk::Window* make_remove_window( CMedia::Attributes& attrs, const char* lbl ) {
  fltk::Window* w;
   {fltk::Window* o = new fltk::Window(405, 100);
    w = o;
    o->shortcut(0xff1b);
    char buf[128];
    sprintf( buf, _( "Remove %s Attribute" ), lbl );
    o->label( buf );
    o->begin();
    {fltk::Choice* o =
        uiKeyRemove = new fltk::Choice(10, 35, 192, 25, _("Name"));
        o->align(fltk::ALIGN_TOP);
        o->begin();
        CMedia::Attributes::const_iterator i = attrs.begin();
        CMedia::Attributes::const_iterator e = attrs.end();
        for ( ; i != e; ++i )
        {
            new fltk::Item( i->first.c_str() ); 
        }
        o->end();
    }
     {fltk::Button* o = new fltk::Button(115, 60, 86, 41, _("OK"));
      o->callback((fltk::Callback*)cb_OK, (void*)(w));
    }
     {fltk::Button* o = new fltk::Button(224, 60, 93, 41, _("Cancel"));
      o->callback((fltk::Callback*)cb_Cancel, (void*)(w));
    }
    o->end();
    o->set_modal();
    o->resizable(o);
  }
  return  w;
}

void add_attribute( CMedia::Attributes& attrs,
                    CMedia::Attributes& other,
                    CMedia* img )
{
    std::string type = uiType->child( uiType->value() )->label();
    std::string key = uiKey->value();
    std::string value = uiValue->value();
    if ( attrs.find( key ) != attrs.end() || other.find( key ) != other.end() )
    {
        char buf[128];
        sprintf( buf, _("'%s' attribute already exists"), key.c_str() );
        mrvALERT( buf );
        return;
    }
    else if ( type == _("Timecode") )
    {
        if ( attrs.find( N_("Video timecode") ) != attrs.end() ||
             other.find( N_("Video timecode") ) != other.end() )
        {
            mrvALERT( _("Video timecode attribute already exists") );
            return;
        }

        Imf::TimeCode t = CMedia::str2timecode( value );
        if ( key.find("timecode") != std::string::npos )
            img->process_timecode( t );
        img->image_damage( img->image_damage() | CMedia::kDamageTimecode );
        Imf::TimeCodeAttribute attr( t );
        attrs.insert( std::make_pair(key, attr.copy() ) );
    }
    else if ( type == _("String") )
    {
        Imf::StringAttribute attr( value );
        attrs.insert( std::make_pair(key, attr.copy() ) );
    }
    else if ( type == _("Integer") )
    {
        int v = atoi( value.c_str() );
        Imf::IntAttribute attr( v );
        attrs.insert( std::make_pair(key, attr.copy() ) );
    }
    else if ( type == _("Float") )
    {
        float v = (float)atof( value.c_str() );
        Imf::FloatAttribute attr( v );
        attrs.insert( std::make_pair(key, attr.copy() ) );
    }
    else if ( type == _("Double") )
    {
        double v = atof( value.c_str() );
        Imf::DoubleAttribute attr( v );
        attrs.insert( std::make_pair(key, attr.copy() ) );
    }
    else if ( type == _("Vector2 Integer") )
    {
        int x,y;
        int n = sscanf( value.c_str(), "%d %d", &x, &y );
        if ( n != 2 )
        {
            mrvALERT("Could not find two integers for vector" );
            return;
        }
        Imf::V2iAttribute attr( Imath::V2i( x, y ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
    }
    else if ( type == _("Vector2 Float") )
    {
        double x,y;
        int n = sscanf( value.c_str(), "%g %g", &x, &y );
        if ( n != 2 )
        {
            mrvALERT("Could not find two floats for vector" );
            return;
        }
        Imf::V2fAttribute attr( Imath::V2f( x, y ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
    }
    else if ( type == _("Vector3 Integer") )
    {
        int x,y,z;
        int n = sscanf( value.c_str(), "%d %d %d", &x, &y, &z );
        if ( n != 3 )
        {
            mrvALERT("Could not find three integers for vector" );
            return;
        }
        Imf::V3iAttribute attr( Imath::V3i( x, y, z ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
    }
    else if ( type == _("Vector3 Float") )
    {
        double x,y,z;
        int n = sscanf( value.c_str(), "%g %g %g", &x, &y, &z );
        if ( n != 3 )
        {
            mrvALERT("Could not find three floats for vector" );
            return;
        }
        Imf::V3fAttribute attr( Imath::V3f( x, y, z ) );
        attrs.insert( std::make_pair(key, attr.copy() ) );
    }
}

void add_iptc_string_cb( fltk::Widget* widget, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if (!img) return;

    fltk::Window* w = make_iptc_add_window();
    if ( ! w->exec() ) return;

  
    CMedia::Attributes& attrs = img->iptc();
    CMedia::Attributes& exifs = img->exif();
    add_attribute( attrs, exifs, img );
    info->refresh();
    delete w;
}


void toggle_modify_iptc_exif_cb( fltk::Widget* widget, ImageInformation* info )
{
    std::string key = widget->label();
    fltk::Group* g = widget->parent();
    for ( ; g != NULL; g = g->parent() )
    {
        if ( !g->label() ) break;
        std::string label = g->label();
        if ( label == "IPTC" || label == "EXIF" ) break;
        key = label + "/" + key;
    }

    g = (fltk::Group*)info->m_iptc->child(1);
    g = (fltk::Group*)g->child(0);
    for ( int i = 0; i < g->children(); ++i )
    {
        fltk::Group* sg = (fltk::Group*)g->child(i);
        fltk::Widget* w = sg->child(0);
        if ( key == w->label() )
        {
            if ( sg->active() )
                sg->deactivate();
            else
                sg->activate();
        }
    }
}

void add_exif_string_cb( fltk::Widget* widget, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if (!img) return;

    fltk::Window* w = make_exif_add_window();
    if ( ! w->exec() ) return;

    std::string key = uiKey->value();
    std::string value = uiValue->value();
  
    CMedia::Attributes& attrs = img->exif();
    CMedia::Attributes& iptcs = img->iptc();
    add_attribute( attrs, iptcs, img );
    info->refresh();
    delete w;
}


void remove_exif_string_cb( fltk::Widget* widget, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if (!img) return;
    
    CMedia::Attributes& attrs = img->exif();

    fltk::Window* w = make_remove_window(attrs, _("EXIF") );
    if ( ! w->exec() ) return;

    std::string key = uiKeyRemove->child( uiKeyRemove->value() )->label();
  
    CMedia::Attributes::iterator i = attrs.find( key );
    if ( i == attrs.end() )
    {
        char buf[128];
        sprintf( buf, _("No attribute named '%s'"), key.c_str() );
        mrvALERT( buf );
        return;
    }
    if ( key.find( "timecode" ) != std::string::npos )
    {
        img->timecode( 0 );
        img->image_damage( img->image_damage() | CMedia::kDamageTimecode );
    }
    attrs.erase( i );
    info->refresh();
}


void remove_iptc_string_cb( fltk::Widget* widget, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if (!img) return;
    
    CMedia::Attributes& attrs = img->iptc();

    fltk::Window* w = make_remove_window(attrs, "IPTC");
    if ( ! w->exec() ) return;

    std::string key = uiKeyRemove->child( uiKeyRemove->value() )->label();
  
    CMedia::Attributes::iterator i = attrs.find( key );
    if ( i == attrs.end() )
    {
        char buf[128];
        sprintf( buf, _("No attribute named '%s'"), key.c_str() );
        mrvALERT( buf );
        return;
    }
    if ( key.find( "timecode" ) != std::string::npos )
    {
        img->timecode( 0 );
        img->image_damage( img->image_damage() | CMedia::kDamageTimecode );
    }
    attrs.erase( i );
    info->refresh();
}

ImageInformation::ImageInformation( int x, int y, int w, int h, 
                                    const char* l ) :
ImageInfoParent( x, y, w, h, l ),
img( NULL ),
m_main( NULL )
{
    begin();



    Rectangle r(w, h);
    box()->inset(r);


    m_all = new fltk::PackedGroup( r.x(), r.y(), r.w()-20, r.h() );
    m_all->set_vertical();
    m_all->spacing(10);

    m_all->begin();

    m_button = new fltk::Button( 0, 0, w, 20, _("Left View") );
    m_button->callback( (fltk::Callback*)change_stereo_image, this );
    m_button->hide();

    m_image = new mrv::CollapsableGroup( 0, 0, w, 400, _("Main")  );
    m_iptc  = new mrv::CollapsableGroup( 0, 0, w, 200, _("IPTC")  );
    m_exif  = new mrv::CollapsableGroup( 0, 0, w, 200, _("EXIF")  );
    m_video = new mrv::CollapsableGroup( 0, 0, w, 100, _("Video") );
    m_audio = new mrv::CollapsableGroup( 0, 0, w, 100, _("Audio") );
    m_subtitle = new mrv::CollapsableGroup( 0, 0, w, 100, _("Subtitle") );

    m_all->end();

    //   resizable( m_all );  // this seems broken, that's why I redo layout
    end();

    hide_tabs();

  }


  int ImageInformation::handle( int event )
  {
     if ( event == fltk::MOUSEWHEEL )
     {
        fltk::e_dy = fltk::event_dy() * 8;
     }

     if ( event == fltk::PUSH && fltk::event_button() == 3 && img )
     {
     
	 fltk::Menu menu(0,0,0,0);

	  menu.add( _("Add/EXIF/Metadata"), 0,
                    (fltk::Callback*)add_exif_string_cb,
                    this);
          
	  menu.add( _("Add/IPTC/Metadata"), 0,
                    (fltk::Callback*)add_iptc_string_cb,
                    this);

          {
              CMedia::Attributes& attrs = img->iptc();
              CMedia::Attributes::iterator i = attrs.begin();
              CMedia::Attributes::iterator e = attrs.end();

              for ( ; i != e; ++i )
              {
                  char buf[256];
                  sprintf( buf,  _("Toggle Modify/IPTC/%s"), i->first.c_str() );
                  menu.add( buf, 0, (fltk::Callback*)toggle_modify_iptc_exif_cb,
                            this );
              }
          }

          {
              CMedia::Attributes& attrs = img->exif();
              CMedia::Attributes::iterator i = attrs.begin();
              CMedia::Attributes::iterator e = attrs.end();

              for ( ; i != e; ++i )
              {
                  char buf[256];
                  sprintf( buf,  _("Toggle Modify/EXIF/%s"), i->first.c_str() );
                  menu.add( buf, 0, (fltk::Callback*)toggle_modify_iptc_exif_cb,
                            this );
              }
          }

	  menu.add( _("Remove/EXIF/Metadata"), 0,
                    (fltk::Callback*)remove_exif_string_cb,
                    this);
          
	  menu.add( _("Remove/IPTC/Metadata"), 0,
                    (fltk::Callback*)remove_iptc_string_cb,
                    this);
         
         menu.popup( fltk::Rectangle( fltk::event_x(),
                                      fltk::event_y(), 80, 1) );
         return 1;
     }
     
     return ImageInfoParent::handle( event );
  }

  void ImageInformation::layout()
  {
    if ( w()-20 != m_all->w() )
      m_all->resize( 0, 0, w()-20, m_all->h() );
    ImageInfoParent::layout();
  }

  struct aspectName_t
  {
    double     ratio;
    const char* name;
  };

  static const aspectName_t kAspectRatioNames[] =
    {
      { 640.0/480.0, _("Video") },
      { 680.0/550.0, _("PAL Video") },
      { 720.0/576.0, _("PAL Video") },
      { 768.0/576.0, _("PAL Video") },
      { 720.0/486.0, _("NTSC Video") },
      { 720.0/540.0, _("NTSC Video") },
      { 1.5,  _("NTSC Video") },
      { 1.37, _("35mm Academy") },
      { 1.56, _("Widescreen (HDTV + STV)") },
      { 1.66, _("35mm European Widescreen") },
      { 1.75, _("Early 35mm") },
      { 1.77, _("HDTV / Widescreen 16:9") },
      { 1.85, _("35mm Flat") },
      { 2.2,  _("70mm") },
      { 2.35, _("35mm Anamorphic") },
      { 2.39, _("35mm Panavision") },
      { 2.55, _("Cinemascope") },
      { 2.76, _("MGM Camera 65") },
    };

void ImageInformation::enum_cb( fltk::PopupMenu* m, ImageInformation* v )
{
    m->label( m->child( m->value() )->label() );
}

static void timecode_cb( fltk::Input* w, ImageInformation* info )
{
    CMedia* img = dynamic_cast<CMedia*>( info->get_image() );
    if ( !img ) return;

    CMedia::Attributes& attrs = img->exif();
    CMedia::Attributes::iterator i = attrs.begin();
    CMedia::Attributes::iterator e = attrs.end();
    for ( ; i != e; ++i )
    {
        if ( i->first == N_("timecode") ||
             i->first == N_("Video timecode") ||
             i->first == N_("Timecode") )
        {
            const Imf::TimeCode& t = CMedia::str2timecode( w->text() );
            Imf::TimeCodeAttribute attr( t );
            i->second = attr.copy();
        }
    }

    CMedia::Attributes& atts = img->iptc();
    i = atts.begin();
    e = atts.end();
    for ( ; i != e; ++i )
    {
        if ( i->first == N_("timecode") ||
             i->first == N_("Video timecode") ||
             i->first == N_("Timecode") )
        {
            const Imf::TimeCode& t = CMedia::str2timecode( w->text() );
            Imf::TimeCodeAttribute attr( t );
            i->second = attr.copy();
        }
    }
    img->process_timecode( w->text() );
    img->image_damage( img->image_damage() | CMedia::kDamageData |
                       CMedia::kDamageTimecode );
}

// Update int slider from int input
static void update_int_slider( fltk::IntInput* w )
{
    fltk::Group* g = w->parent();
    fltk::Slider* s = (fltk::Slider*)g->child(1);
    s->value( w->ivalue() );
}

// Update float slider from float input
static void update_float_slider( fltk::FloatInput* w )
{
    fltk::Group* g = w->parent();
    fltk::Slider* s = (fltk::Slider*)g->child(1);
    s->value( w->fvalue() );
}

void ImageInformation::float_slider_cb( fltk::Slider* s, void* data )
{
    fltk::FloatInput* n = (fltk::FloatInput*) data;
    n->value( s->value() );
    n->do_callback();
}

void ImageInformation::int_slider_cb( fltk::Slider* s, void* data )
{
    fltk::IntInput* n = (fltk::IntInput*) data;
    n->value( (int)s->value() );
    n->do_callback();
}

static bool modify_string( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    Imf::StringAttribute attr( w->text() );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v2i( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    int x, y;
    int num = sscanf( w->text(), "%d %d", &x, &y );
    if ( num != 2 ) return false;

    Imath::V2i val( x, y );
    Imf::V2iAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v2f( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    float x, y;
    int num = sscanf( w->text(), "%g %g %g", &x, &y );
    if ( num != 2 ) return false;

    Imath::V2f val( x, y );
    Imf::V2fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v2d( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    double x, y;
    int num = sscanf( w->text(), "%g %g %g", &x, &y );
    if ( num != 2 ) return false;

    Imath::V2d val( x, y );
    Imf::V2dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v3i( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    int x, y, z;
    int num = sscanf( w->text(), "%d %d %d", &x, &y, &z );
    if ( num != 3 ) return false;

    Imath::V3i val( x, y, z );
    Imf::V3iAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v3f( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    float x, y, z;
    int num = sscanf( w->text(), "%g %g %g", &x, &y, &z );
    if ( num != 3 ) return false;

    Imath::V3f val( x, y, z );
    Imf::V3fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_v3d( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    double x, y, z;
    int num = sscanf( w->text(), "%g %g %g", &x, &y, &z );
    if ( num != 3 ) return false;

    Imath::V3d val( x, y, z );
    Imf::V3dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m33f( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    float m00,m01,m02,m10,m11,m12,m20,m21,m22;
    int num = sscanf( w->text(), 
                      "%g %g %g  %g %g %g  %g %g %g",
                      &m00, &m01, &m02,
                      &m10, &m11, &m12,
                      &m20, &m21, &m22 );
    if ( num != 9 ) return false;
    
    Imath::M33f val( m00,m01,m02,
                     m10,m11,m12,
                     m20,m21,m22 );
    Imf::M33fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m33d( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    double m00,m01,m02,m10,m11,m12,m20,m21,m22;
    int num = sscanf( w->text(), 
                      "%g %g %g  %g %g %g  %g %g %g",
                      &m00, &m01, &m02,
                      &m10, &m11, &m12,
                      &m20, &m21, &m22 );
    if ( num != 9 ) return false;
    
    Imath::M33d val( m00,m01,m02,
                     m10,m11,m12,
                     m20,m21,m22 );
    Imf::M33dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m44f( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    float m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,m23,m30,m31,m32,m33;
    int num = sscanf( w->text(), 
                      "%g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g",
                      &m00, &m01, &m02, &m03, 
                      &m10, &m11, &m12, &m13, 
                      &m20, &m21, &m22, &m23, 
                      &m30, &m31, &m32, &m33 );
    if ( num != 16 ) return false;
    
    Imath::M44f val( m00,m01,m02,m03,
                     m10,m11,m12,m13,
                     m20,m21,m22,m23,
                     m30,m31,m32,m33 );
    Imf::M44fAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_m44d( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    double m00,m01,m02,m03,m10,m11,m12,m13,m20,m21,m22,m23,m30,m31,m32,m33;
    int num = sscanf( w->text(), 
                      "%g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g",
                      &m00, &m01, &m02, &m03, 
                      &m10, &m11, &m12, &m13, 
                      &m20, &m21, &m22, &m23, 
                      &m30, &m31, &m32, &m33 );
    if ( num != 16 ) return false;
    
    Imath::M44d val( m00,m01,m02,m03,
                     m10,m11,m12,m13,
                     m20,m21,m22,m23,
                     m30,m31,m32,m33 );
    Imf::M44dAttribute attr( val );
    delete i->second;
    i->second = attr.copy();
    return true;
}

static bool modify_value( fltk::Input* w, CMedia::Attributes::iterator& i)
{
    if ( dynamic_cast< Imf::StringAttribute* >( i->second ) != NULL )
        return modify_string( w, i );
    else if ( dynamic_cast< Imf::M44dAttribute* >( i->second ) != NULL )
        return modify_m44d( w, i );
    else if ( dynamic_cast< Imf::M44fAttribute* >( i->second ) != NULL )
        return modify_m44f( w, i );
    else if ( dynamic_cast< Imf::M33dAttribute* >( i->second ) != NULL )
        return modify_m33d( w, i );
    else if ( dynamic_cast< Imf::M33fAttribute* >( i->second ) != NULL )
        return modify_m33f( w, i );
    else if ( dynamic_cast< Imf::V3dAttribute* >( i->second ) != NULL )
        return modify_v3d( w, i );
    else if ( dynamic_cast< Imf::V3fAttribute* >( i->second ) != NULL )
        return modify_v3f( w, i );
    else if ( dynamic_cast< Imf::V3iAttribute* >( i->second ) != NULL )
        return modify_v3i( w, i );
    else if ( dynamic_cast< Imf::V2dAttribute* >( i->second ) != NULL )
        return modify_v2d( w, i );
    else if ( dynamic_cast< Imf::V2fAttribute* >( i->second ) != NULL )
        return modify_v2f( w, i );
    else if ( dynamic_cast< Imf::V2iAttribute* >( i->second ) != NULL )
        return modify_v2i( w, i );
    else
        LOG_ERROR( _("Unknown attribute to convert from string") );
}

static bool modify_int( fltk::IntInput* w, CMedia::Attributes::iterator& i)
{
    Imf::IntAttribute attr( atoi( w->text() ) );
    delete i->second;
    i->second = attr.copy();
    update_int_slider( w );
    return true;
}

static bool modify_float( fltk::FloatInput* w, CMedia::Attributes::iterator& i)
{
    Imf::FloatAttribute attr( w->fvalue() );
    delete i->second;
    i->second = attr.copy();
    update_float_slider( w );
    return true;
}

static void change_float_cb( fltk::FloatInput* w, ImageInformation* info )
{
    CMedia* img = dynamic_cast<CMedia*>( info->get_image() );
    if ( !img ) return;

    fltk::Group* g = (fltk::Group*)w->parent()->parent();
    fltk::Widget* widget = g->child(0);
    if ( !widget->label() ) return;

    std::string key = widget->label();
    CMedia::Attributes& exif = img->exif();
    CMedia::Attributes::iterator i = exif.find( key );
    if ( i != exif.end() )
    {
        modify_float( w, i );
        return;
    }
    CMedia::Attributes& iptc = img->iptc();
    i = iptc.find( key );
    if ( i != iptc.end() )
    {
        modify_float( w, i );
        return;
    }
}

static void change_string_cb( fltk::Input* w, ImageInformation* info )
{
    CMedia* img = dynamic_cast<CMedia*>( info->get_image() );
    if ( !img ) {
        LOG_ERROR( "Image is invalid" );
        return;
    }

    fltk::Group* g = (fltk::Group*)w->parent();
    fltk::Widget* widget = g->child(0);

    if ( !widget->label() ) {
        LOG_ERROR( "Widget has no label" );
        return;
    }

    std::string key = widget->label();
    CMedia::Attributes& exif = img->exif();
    CMedia::Attributes::iterator i = exif.find( key );
    if ( i != exif.end() )
    {
        bool ok = modify_value( w, i );
        if (!ok) {
            LOG_ERROR( _("Could not convert EXIF ") << i->first
                       << _(" from string") );
            info->refresh();
        }
        return;
    }
    CMedia::Attributes& iptc = img->iptc();
    i = iptc.find( key );
    if ( i != iptc.end() )
    {
        bool ok = modify_value( w, i );
        if (!ok) {
            LOG_ERROR( _("Could not convert IPTC ") << i->first 
                       << _( " from string" ) );
            info->refresh();
        }
        return;
    }
}

static void change_int_cb( fltk::IntInput* w, ImageInformation* info )
{
    CMedia* img = dynamic_cast<CMedia*>( info->get_image() );
    if ( !img ) return;

    fltk::Group* g = (fltk::Group*)w->parent()->parent();
    fltk::Widget* widget = g->child(0);
    if ( !widget->label() ) return;

    std::string key = widget->label();
    CMedia::Attributes& exif = img->exif();
    CMedia::Attributes::iterator i = exif.find( key );
    if ( i != exif.end() )
    {
        modify_int( w, i );
        return;
    }
    CMedia::Attributes& iptc = img->iptc();
    i = iptc.find( key );
    if ( i != iptc.end() )
    {
        modify_int( w, i );
        return;
    }
}

static void change_colorspace( fltk::PopupMenu* w, ImageInformation* info )
{
    aviImage* img = dynamic_cast<aviImage*>( info->get_image() );
    if ( !img ) return;

    int v = w->value();
    w->label( _(kColorSpaces[v]) );
    img->colorspace_index( (int)v );
    img->image_damage( mrv::CMedia::kDamageAll );
}

static void change_mipmap_cb( fltk::IntInput* w, ImageInformation* info )
{
    exrImage* img = dynamic_cast<exrImage*>( info->get_image() );
    if ( img )
    {
        mrv::ImageView* view = info->main()->uiView;
        img->levelX( w->ivalue() );
        img->levelY( w->ivalue() );
        update_int_slider( w );
        bool ok = img->fetch( view->frame() );
        if (ok)
        {
            img->refresh();
            view->fit_image();
            view->redraw();
        }
    }
}

static void change_x_ripmap_cb( fltk::IntInput* w, ImageInformation* info )
{
    exrImage* img = dynamic_cast<exrImage*>( info->get_image() );
    if ( img )
    {
        mrv::ImageView* view = info->main()->uiView;
        img->levelX( w->ivalue() );
        update_int_slider( w );
        bool ok = img->fetch( view->frame() );
        if (ok)
        {
            img->refresh();
            view->fit_image();
            view->redraw();
        }
    }
}

static void change_y_ripmap_cb( fltk::IntInput* w, ImageInformation* info )
{
    exrImage* img = dynamic_cast<exrImage*>( info->get_image() );
    if ( img )
    {
        mrv::ImageView* view = info->main()->uiView;
        img->levelY( w->ivalue() );
        update_int_slider( w );
        bool ok = img->fetch( view->frame() );
        if (ok)
        {
            img->refresh();
            view->fit_image();
            view->redraw();
        }
    }
}

static void change_first_frame_cb( fltk::IntInput* w, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if ( img )
    {
        int64_t v = w->ivalue();
        if ( v < img->start_frame() )
            v = img->start_frame();
        if ( v > img->last_frame() )
            v = img->last_frame();
        w->value( (int)v );

        img->first_frame( v );
        update_int_slider( w );
        mrv::ImageView* view = info->main()->uiView;
        view->redraw();
    }
}

  static void eye_separation_cb( fltk::FloatInput* w, ImageInformation* info )
  {
      CMedia* img = info->get_image();
      if ( img )
      {
          img->eye_separation( (float)w->fvalue() );
          update_float_slider( w );
          info->main()->uiView->redraw();
      }
  }

static void change_fps_cb( fltk::FloatInput* w, ImageInformation* info )
{
    CMedia* img = info->get_image();
    if ( img )
    {
        img->fps( w->fvalue() );
        update_float_slider( w );
    }
}

static void change_last_frame_cb( fltk::IntInput* w,
                                  ImageInformation* info )
{
    CMedia* img = info->get_image();
    if ( img )
    {
        int64_t v = w->ivalue();
        if ( v < img->first_frame() )
            v = img->first_frame();
        if ( v > img->end_frame() )
            v = img->end_frame();
        w->value( (int)v );

        img->last_frame( v );
        info->main()->uiView->redraw();
        update_int_slider( w );
    }
}

static void change_pixel_ratio_cb( fltk::FloatInput* w,
                                   ImageInformation* info )
{
    CMedia* img = info->get_image();
    img->pixel_ratio( w->fvalue() );
    info->main()->uiView->redraw();
    update_float_slider( w );
}

static void change_gamma_cb( fltk::FloatInput* w, ImageInformation* info )
{
    CMedia* img = info->get_image();
    img->gamma( float(w->fvalue()) );
    update_float_slider( w );

    mrv::ImageView* view = info->main()->uiView;
    view->gamma( float(w->fvalue()) );
    view->redraw();
}

double ImageInformation::to_memory( long double value,
                                    const char*& extension )
{
    if ( value >= 1099511627776 )
    {
	value /= 1099511627776;
	extension = N_("Tb");
    }
    else if ( value >= 1073741824 )
    {
	value /= 1073741824;
	extension = N_("Gb");
    }
    else if ( value >= 1048576 )
    {
	value /= 1048576;
	extension = N_("Mb");
    }
    else if ( value >= 1024 )
    {
	value /= 1024;
	extension = N_("Kb");
    }
    else
    {
	extension = N_("bytes");
    }
    return value;
}

void ImageInformation::set_image( CMedia* i )
{
    img = i;
    refresh();
}

void ImageInformation::clear_callback_data()
{

    LMTData::iterator i = widget_data.begin();
    LMTData::iterator e = widget_data.end();

    for ( ; i != e; ++i)
    {
        delete *i;
    }

    widget_data.clear();
}

void ImageInformation::hide_tabs()
{
    m_image->hide();
    m_video->hide();
    m_audio->hide();
    m_subtitle->hide();
    m_iptc->hide();
    m_exif->hide();
}

void ImageInformation::process_attributes( mrv::CMedia::Attributes::const_iterator& i )
{
    {
        Imf::TimeCodeAttribute* attr =
        dynamic_cast< Imf::TimeCodeAttribute*>( i->second );
        if (attr)
        {
            mrv::Timecode::Display d =
            mrv::Timecode::kTimecodeNonDrop;
            if ( attr->value().dropFrame() )
                d = mrv::Timecode::kTimecodeDropFrame;
            char* buf = new char[64];
            int n = mrv::Timecode::format( buf, d, img->frame(),
                                           img->timecode(),
                                           img->play_fps(), true );
            add_text( i->first.c_str(),
                      "Timecode start (editable) press TAB to accept",
                      buf, true, true, (fltk::Callback*)timecode_cb );
            return;
        }
    }
    {
        Imf::DoubleAttribute* attr =
        dynamic_cast< Imf::DoubleAttribute* >( i->second );
        if ( attr )
        {
            add_float( i->first.c_str(), i->first.c_str(),
                       attr->value(), true, false,
                       (fltk::Callback*)change_float_cb );
            return;
        }
    }
    {
        Imf::FloatAttribute* attr =
        dynamic_cast< Imf::FloatAttribute* >( i->second );
        if ( attr )
        {
            add_float( i->first.c_str(), i->first.c_str(),
                       attr->value(), true, false,
                       (fltk::Callback*)change_float_cb );
            return;
        }
    }
    {
        Imf::IntAttribute* attr =
        dynamic_cast< Imf::IntAttribute* >( i->second );
        if ( attr )
        {
            add_int( i->first.c_str(), i->first.c_str(),
                     attr->value(), true, false,
                     (fltk::Callback*)change_int_cb );
            return;
        }
    }
    {
        Imf::V2iAttribute* attr =
        dynamic_cast< Imf::V2iAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V2i& v = attr->value();
            sprintf( buf, "%d %d", v.x, v.y );
            add_text( i->first.c_str(), i->first.c_str(),
                      buf, true, false, (fltk::Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::V2dAttribute* attr =
        dynamic_cast< Imf::V2dAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V2d& v = attr->value();
            sprintf( buf, "%g %g", v.x, v.y );
            add_text( i->first.c_str(), i->first.c_str(),
                      buf, true, false, (fltk::Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::V2fAttribute* attr =
        dynamic_cast< Imf::V2fAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V2f& v = attr->value();
            sprintf( buf, "%g %g", v.x, v.y );
            add_text( i->first.c_str(), i->first.c_str(),
                      buf, true, false, (fltk::Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::V3iAttribute* attr =
        dynamic_cast< Imf::V3iAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V3i& v = attr->value();
            sprintf( buf, "%d %d %d", v.x, v.y, v.z );
            add_text( i->first.c_str(), i->first.c_str(),
                      buf, true, false, (fltk::Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::V3dAttribute* attr =
        dynamic_cast< Imf::V3dAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V3d& v = attr->value();
            sprintf( buf, "%g %g %g", v.x, v.y, v.z );
            add_text( i->first.c_str(), i->first.c_str(),
                      buf, true, false, (fltk::Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::V3fAttribute* attr =
        dynamic_cast< Imf::V3fAttribute* >( i->second );
        if ( attr )
        {
            char buf[64];
            const Imath::V3f& v = attr->value();
            sprintf( buf, "%g %g %g", v.x, v.y, v.z );
            add_text( i->first.c_str(), i->first.c_str(),
                      buf, true, false, (fltk::Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::M33dAttribute* attr =
        dynamic_cast< Imf::M33dAttribute* >( i->second );
        if ( attr )
        {
            char buf[256];
            const Imath::M33d& v = attr->value();
            sprintf( buf, "%g %g %g  %g %g %g  %g %g %g", 
                     v[0][0], v[0][1], v[0][2],
                     v[1][0], v[1][1], v[1][2],
                     v[2][0], v[2][1], v[2][2] );
            add_text( i->first.c_str(), i->first.c_str(),
                      buf, true, false, (fltk::Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::M33fAttribute* attr =
        dynamic_cast< Imf::M33fAttribute* >( i->second );
        if ( attr )
        {
            char buf[256];
            const Imath::M33f& v = attr->value();
            sprintf( buf, "%g %g %g  %g %g %g  %g %g %g", 
                     v[0][0], v[0][1], v[0][2],
                     v[1][0], v[1][1], v[1][2],
                     v[2][0], v[2][1], v[2][2] );
            add_text( i->first.c_str(), i->first.c_str(),
                      buf, true, false, (fltk::Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::M44dAttribute* attr =
        dynamic_cast< Imf::M44dAttribute* >( i->second );
        if ( attr )
        {
            char buf[256];
            const Imath::M44d& v = attr->value();
            sprintf( buf, "%g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g", 
                     v[0][0], v[0][1], v[0][2], v[0][3],
                     v[1][0], v[1][1], v[1][2], v[1][3],
                     v[2][0], v[2][1], v[2][2], v[2][3],
                     v[3][0], v[3][1], v[3][2], v[3][3] );
            add_text( i->first.c_str(), i->first.c_str(),
                      buf, true, false, (fltk::Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::M44fAttribute* attr =
        dynamic_cast< Imf::M44fAttribute* >( i->second );
        if ( attr )
        {
            char buf[256];
            const Imath::M44f& v = attr->value();
            sprintf( buf, "%g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g", 
                     v[0][0], v[0][1], v[0][2], v[0][3],
                     v[1][0], v[1][1], v[1][2], v[1][3],
                     v[2][0], v[2][1], v[2][2], v[2][3],
                     v[3][0], v[3][1], v[3][2], v[3][3] );
            add_text( i->first.c_str(), i->first.c_str(),
                      buf, true, false, (fltk::Callback*) change_string_cb );
            return;
        }
    }
    {
        Imf::StringAttribute* attr =
        dynamic_cast< Imf::StringAttribute* >( i->second );
        if ( attr )
        {
            add_text( i->first.c_str(), i->first.c_str(),
                      attr->value().c_str(), true, false,
                      (fltk::Callback*) change_string_cb );
            return;
        }
    }
}

void ImageInformation::fill_data()
{

    m_curr = add_browser(m_image);


    add_text( _("Directory"), _("Directory where clip resides"), img->directory() );

    char buf[1024];
    add_text( _("Filename"), _("Filename of the clip"), img->name().c_str() );


    ++group;

    unsigned int num_video_streams = unsigned( img->number_of_video_streams() );
    unsigned int num_audio_streams = unsigned( img->number_of_audio_streams() );
    unsigned int num_subtitle_streams = unsigned( img->number_of_subtitle_streams() );
    if ( img->has_video() || img->has_audio() )
      {
          add_int( _("Video Streams"), _("Number of video streams in file"),
                   num_video_streams );
          add_int( _("Audio Streams"), _("Number of audio streams in file"),
                   num_audio_streams );
          add_int( _("Subtitle Streams"), 
                   _("Number of subtitle streams in file"), 
                   num_subtitle_streams );
      }
    else
      {
          add_bool( _("Sequence"), _("Clip is a sequence of frames"), img->is_sequence() );
      }

    if ( img->first_frame() != img->last_frame() )
      {
          add_int( _("First Frame"), _("First frame of clip - User selected"), 
                   (int)img->first_frame(), true, true,
		  (fltk::Callback*)change_first_frame_cb, 1, 50 );
          add_int( _("Last Frame"), _("Last frame of clip - User selected"),
                   (int)img->last_frame(), true, true,
                   (fltk::Callback*)change_last_frame_cb, 2, 55 );
      }


    
    add_int64( _("Frame Start"), _("Frame start of clip in file"), img->start_frame() );
    add_int64( _("Frame End"), _("Frame end of clip in file"), img->end_frame() );


    add_float( _("FPS"), _("Frames Per Second"), (float) img->fps(), true, true,
               (fltk::Callback*)change_fps_cb, 1.0f, 100.0f );


    ++group;
    
    add_int( _("Width"), _("Width of clip"), img->width() );
    add_int( _("Height"), _("Height of clip"), img->height() );


    double aspect_ratio = 0;
    const mrv::Recti& dpw = img->display_window();
    if ( dpw.h() )
    {
        aspect_ratio = ( (double) dpw.w() / (double) dpw.h() );
    }
    else
        if ( img->height() > 0 )
            aspect_ratio = ( img->width() / (double) img->height() );


    const char* name = _("Unknown");
    int num = sizeof( kAspectRatioNames ) / sizeof(aspectName_t);
    for ( int i = 0; i < num; ++i )
      {
	static const float fuzz = 0.005f;
	if ( aspect_ratio > kAspectRatioNames[i].ratio - fuzz &&
	     aspect_ratio < kAspectRatioNames[i].ratio + fuzz)
	  {
              name = _( kAspectRatioNames[i].name ); break;
	  }
      }


    
    sprintf( buf, N_("%g (%s)"), aspect_ratio, name );
    add_text( _("Aspect Ratio"), _("Aspect ratio of clip"), buf );
    add_float( _("Pixel Ratio"), _("Pixel ratio of clip"),
               float(img->pixel_ratio()), true, true,
	       (fltk::Callback*)change_pixel_ratio_cb, 0.01f, 4.0f );


    const mrv::Recti& window = img->data_window();
    if ( dpw != window && dpw.w() != 0 )
    {
        add_rect( _("Data Window"), _("Data Window of Clip"), window );
    }


    add_rect( _("Display Window"), _("Display Window of Clip"), dpw );


    if ( img->is_stereo() )
    {
        const mrv::Recti& window2 = img->data_window2();
        if ( window != window2 )
        {
            add_rect( _("Data Window 2"), _("Second Data Window of Clip"),
                      window2 );
        }

        const mrv::Recti& dwindow2 = img->display_window2();
        if ( dpw != dwindow2 )
        {
            add_rect( _("Display Window 2"), _("Second Display Window of Clip"),
                      dwindow2 );
        }


        if ( img->right() )
            add_float( _("Eye Separation"), _("Stereo eye separation"),
                       img->eye_separation(), true, true,
                       (fltk::Callback*)eye_separation_cb, -20.0f, 20.0f );
    }


    ++group;

    const char* depth;
    switch( img->depth() )
      {
      case VideoFrame::kByte:
	depth = _("unsigned byte (8-bits per channel)"); break;
      case VideoFrame::kShort:
	depth = _("unsigned short (16-bits per channel)"); break;
      case VideoFrame::kInt:
	depth = _("unsigned int (32-bits per channel)"); break;
      case VideoFrame::kHalf:
	depth = _("half float (16-bits per channel)"); break;
      case VideoFrame::kFloat:
	depth = _("float (32-bits per channel)"); break;
      default:
	depth = _("Unknown bit depth"); break;
      }


    add_text( _("Depth"), _("Bit depth of clip"), depth );
    add_int( _("Image Channels"), _("Number of channels in clip"),
             img->number_of_channels() );

    aviImage* avi = dynamic_cast< aviImage* >( img );
    if ( avi )
    {
        add_enum( _("Color Space"), _("YUV Color Space conversion.  This value is extracted from the movie file.  To lock it to always use the same color space, set the value in Preferences->Video->YUV Conversion.  That value shall take precedence upon loading of the movie file."),
                  avi->colorspace_index(), kColorSpaces, 
                  11, true, (fltk::Callback*)change_colorspace );
        add_text( _("Color Range"), _("YUV Color Range"),
                  _(avi->color_range()) );
    }
    


    ++group;

    const char* format;
    switch( img->pixel_format() )
      {
      case VideoFrame::kLumma:
	format = _("Lumma"); break;
      case VideoFrame::kRGB:
	format = N_("RGB"); break;
      case VideoFrame::kRGBA:
	format = N_("RGBA"); break;
      case VideoFrame::kBGR:
	format = N_("BGR"); break;
      case VideoFrame::kBGRA:
	format = N_("BGRA"); break;

      case VideoFrame::kITU_601_YCbCr444:
	format = N_("ITU.601 YCbCr444"); break;
      case VideoFrame::kITU_601_YCbCr444A:
	format = N_("ITU.601 YCbCr444 A"); break;
      case VideoFrame::kITU_601_YCbCr422:
	format = N_("ITU.601 YCbCr422"); break;
      case VideoFrame::kITU_601_YCbCr422A:
	format = N_("ITU.601 YCbCr422 A"); break;
      case VideoFrame::kITU_601_YCbCr420:
	format = N_("ITU.601 YCbCr420"); break;
      case VideoFrame::kITU_601_YCbCr420A:
	format = N_("ITU.601 YCbCr420 A"); break;

      case VideoFrame::kITU_709_YCbCr444A:
	format = N_("ITU.709 YCbCr444 A"); break;
      case VideoFrame::kITU_709_YCbCr444:
	format = N_("ITU.709 YCbCr444"); break;
      case VideoFrame::kITU_709_YCbCr422A:
	format = N_("ITU.709 YCbCr422 A"); break;
      case VideoFrame::kITU_709_YCbCr422:
	format = N_("ITU.709 YCbCr422"); break;
      case VideoFrame::kITU_709_YCbCr420:
	format = N_("ITU.709 YCbCr420"); break;
      case VideoFrame::kITU_709_YCbCr420A:
	format = N_("ITU.709 YCbCr420 A"); break;

      case VideoFrame::kYByRy420:
	format = N_("Y BY RY 420"); break;
      case VideoFrame::kYByRy420A:
	format = N_("Y BY RY 420 A"); break;
      case VideoFrame::kYByRy422:
	format = N_("Y BY RY 422"); break;
      case VideoFrame::kYByRy422A:
	format = N_("Y BY RY 422 A"); break;
      case VideoFrame::kYByRy444:
	format = N_("Y BY RY 444"); break;
      case VideoFrame::kYByRy444A:
	format = N_("Y BY RY 444 A"); break;
      default:
	format = _("Unknown render pixel format"); break;
      }

    add_text( _("Render Pixel Format"), _("Render Pixel Format"), format );



    
    static const char* kRenderingIntent[] = {
      _("Undefined"),
      _("Saturation"),
      _("Perceptual"),
      _("Absolute"),
      _("Relative"),
    };

    

    add_text( _("Rendering Intent"), _("ICC Rendering Intent"),
	      kRenderingIntent[ (int) img->rendering_intent() ] );
 
    
    

    add_float( _("Gamma"), _("Display Gamma of Image"), img->gamma(), true, 
	       true, (fltk::Callback*)change_gamma_cb, 0.01f,	4.0f );


    if ( img->has_chromaticities() )
    {
        const Imf::Chromaticities& c = img->chromaticities(); 
        sprintf( buf, _("R: %g %g    G: %g %g    B: %g %g"),
                 c.red.x, c.red.y, c.green.x, c.green.y,
                 c.blue.x, c.blue.y );
        add_text( _("CIExy Chromaticities"), _("CIExy Chromaticities"), buf );
        sprintf( buf, _("W: %g %g"),c.white.x, c.white.y );
        add_text( _("CIExy White Point"), _("CIExy White Point"), buf );
    }



    add_ctl_idt( _("Input Device Transform"), 
                 _("(IDT) Input Device Transform"), 
                 img->idt_transform() );

    clear_callback_data();

    {

        unsigned count = (unsigned) img->number_of_lmts();
        for ( unsigned i = 0; i <= count; ++i )
        {
            sprintf( buf, _("LMTransform %u"), i+1 );
            add_ctl_lmt( strdup( buf ), _("(LMT) Look Mod Transform"), 
                         img->look_mod_transform(i), i );
        }
    }
    


    add_ctl( _("Render Transform"), _("(RT) Render Transform"), 
             img->rendering_transform() );
    

    add_icc( _("ICC Profile"), _("ICC Profile"), img->icc_profile() );
    

    ++group;


    add_text( _("Format"), _("Format"), img->format() );

    if ( !img->has_video() )
    {
        add_text( _("Line Order"), _("Line order in file"), 
                  img->line_order() );
    }


    if ( !img->has_video() )
    {
        ++group;

        add_text( _("Compression"), _("Clip Compression"), img->compression() );

    }

    ++group;


    const char* space_type = NULL;
    double memory_space = double( to_memory( (long double)img->memory(),
                                             space_type ) );
    sprintf( buf, N_("%.3f %s"), memory_space, space_type );
    add_text( _("Memory"), _("Memory without Compression"), buf );

    if ( img->disk_space() >= 0 )
      {
          double disk_space = double( to_memory( (long double)img->disk_space(),
                                                 space_type ) );
	double pct   = double( 100.0 * ( (long double) img->disk_space() /
                                         (long double) img->memory() ) );

	sprintf( buf, N_("%.3f %s  (%.2f %% of memory size)"), 
		 disk_space, space_type, pct );
	add_text( _("Disk space"), _("Disk space"), buf );


        if ( !img->has_video() )
        {
            double ratio = 100.0 - double(pct);
            sprintf( buf, _("%.2g %%"), ratio );
            add_text( _("Compression Ratio"), _("Compression Ratio"), buf );
        }
      }
    

    ++group;
    add_text( _("Creation Date"), _("Creation Date"), img->creation_date() );

    

    
    m_curr->relayout();

    


    m_image->relayout();
    m_image->show();


    


    CMedia::Attributes attrs = img->iptc();
    if ( ! attrs.empty() )
      {

	m_curr = add_browser(m_iptc);

        exrImage* exr = dynamic_cast< exrImage* >( img );
        if ( exr )
        {
            std::string date = exr->capture_date( img->frame() );
            add_text( _("Capture Date"), _("Capture Date"), date.c_str() );
        }
        
	CMedia::Attributes::const_iterator i = attrs.begin();
	CMedia::Attributes::const_iterator e = attrs.end();
	for ( ; i != e; ++i )
        {
            process_attributes( i );
        }

	m_curr->relayout();
	m_curr->parent()->relayout();
      }
    

    attrs = img->exif();
    if ( ! attrs.empty() )
      {
	m_curr = add_browser( m_exif );

	CMedia::Attributes::const_iterator i = attrs.begin();
	CMedia::Attributes::const_iterator e = attrs.end();
	for ( ; i != e; ++i )
	  {
            if ( i->first == _("Mipmap Levels") )
              {
                  exrImage* exr = dynamic_cast< exrImage* >( img );
                  if ( exr )
                  {
                      add_int( _("Mipmap Level"), _("Mipmap Level"),
                               exr->levelX(), true, true,
                               (fltk::Callback*)change_mipmap_cb, 0, 20 );
                      exr->levelY( exr->levelX() );
                  }
              }
              else if ( i->first == _("X Ripmap Levels") )
              {
                  exrImage* exr = dynamic_cast< exrImage* >( img );
                  if ( exr )
                  {
                      add_int( _("X Ripmap Level"), _("X Ripmap Level"), 
                               exr->levelX(), true, true,
                               (fltk::Callback*)change_x_ripmap_cb, 0, 20 );
                  }
              }
              else if ( i->first == _("Y Ripmap Levels") )
              {
                  exrImage* exr = dynamic_cast< exrImage* >( img );
                  if ( exr )
                  {
                      add_int( _("Y Ripmap Level"), _("Y Ripmap Level"), 
                               exr->levelY(), true, true,
                               (fltk::Callback*)change_y_ripmap_cb, 0, 20 );
                  }
              }
              else
              {
                  process_attributes( i );
              }
	  }
	m_curr->relayout();

	m_curr->parent()->relayout();

      }



    if ( num_video_streams > 0 )
      {
	for ( unsigned i = 0; i < num_video_streams; ++i )
	  {
	    char buf[256];
	    sprintf( buf, _("Video Stream #%d"), i+1 );
	    m_curr = add_browser( m_video );
	    m_curr->copy_label( buf );


	    const CMedia::video_info_t& s = img->video_info(i);
	    add_bool( _("Known Codec"), _("mrViewer knows codec used"),
                      s.has_codec );
	    add_text( _("Codec"), _("Codec Name"), s.codec_name );
	    add_text( _("FourCC"), _("Four letter ID"), s.fourcc );
	    add_bool( _("B Frames"), _("Video has B frames"), s.has_b_frames );
	    ++group;

	    add_text( _("Pixel Format"), _("Pixel Format"), s.pixel_format );
	    ++group;



	    const char* name = "";
	    if      ( is_equal( s.fps, 29.97 ) )     name = "(NTSC)";
	    else if ( is_equal( s.fps, 30.0 ) )      name = "(60hz HDTV)";
	    else if ( is_equal( s.fps, 25.0 ) )      name = "(PAL)";
	    else if ( is_equal( s.fps, 24.0 ) )      name = "(Film)";
	    else if ( is_equal( s.fps, 50.0 ) )      name = _("(PAL Fields)");
	    else if ( is_equal( s.fps, 59.940059 ) ) name = _("(NTSC Fields)");

	    sprintf( buf, "%g %s", s.fps, name );
	    add_text( _("FPS"), _("Frames per Second"), buf );
	    ++group;
	    add_text( _("Language"), _("Language if known"), s.language );
            add_text( _("Disposition"), _("Disposition of Track"),
                      s.disposition );

	    ++group;
	    add_time( _("Start"), _("Start of Video"), s.start, s.fps );
	    add_time( _("Duration"), _("Duration of Video"), 
                      s.duration, s.fps );

	    m_curr->relayout();

	    m_curr->parent()->relayout();
	  }
      }

    if ( num_audio_streams > 0 )
      {
	for ( unsigned i = 0; i < num_audio_streams; ++i )
	  {
	    char buf[256];

	    m_curr = add_browser( m_audio );
	    sprintf( buf, _("Audio Stream #%d"), i+1 );
	    m_curr->copy_label( buf );

	    const CMedia::audio_info_t& s = img->audio_info(i);


	    add_bool( _("Known Codec"), _("mrViewer knows the codec used"),
                      s.has_codec );
	    add_text( _("Codec"), _("Codec Name"), s.codec_name );
	    add_text( _("FourCC"), _("Four letter ID"), s.fourcc );
	    ++group;

	    const char* channels = "Stereo";
	    if ( s.channels == 1 )      channels = "Mono";
	    else if ( s.channels == 2 ) channels = "Stereo";
	    else if ( s.channels == 6 ) channels = "5:1";
	    else if ( s.channels == 8 ) channels = "7:1";
	    else {
	      sprintf( buf, N_("%d"), s.channels );
	      channels = buf;
	    }

	    add_text( _("Format"), _("Format"), s.format );
	    add_text( _("Channels"), _("Number of audio channels"), channels );
            sprintf( buf, _("%d Hz."), s.frequency );
	    add_text( _("Frequency"), _("Frequency of audio"), buf );
	    sprintf( buf, _("%d kb/s"), s.bitrate/1000 );
            add_text( _("Avg. Bitrate"), _("Avg. Bitrate"), buf );

	    ++group;
	    add_text( _("Language"), _("Language if known"), s.language );
	    ++group;
            add_text( _("Disposition"), _("Disposition of Track"),
                                          s.disposition);
	    ++group;

	    add_time( _("Start"), _("Start of Audio"), s.start, img->fps() );
	    add_time( _("Duration"), _("Duration of Audio"), 
                      s.duration, img->fps() );

	    m_curr->relayout();
	    m_curr->parent()->relayout();
	  }

	m_audio->parent()->show();
      }

    if ( num_subtitle_streams > 0 )
      {
	for ( unsigned i = 0; i < num_subtitle_streams; ++i )
	  {
	    char buf[256];
	    m_curr = add_browser( m_subtitle );
	    sprintf( buf, _("Subtitle Stream #%d"), i+1 );
	    m_curr->copy_label( buf );

	    const CMedia::subtitle_info_t& s = img->subtitle_info(i);

	    add_bool( _("Known Codec"), _("mrViewer knows the codec used"),
                      s.has_codec );
	    add_text( _("Codec"), _("Codec name"), s.codec_name );
	    add_text( _("FourCC"), _("Four letter ID"), s.fourcc );
	    ++group;

	    sprintf( buf, _("%d kb/s"), s.bitrate/1000 );
	    add_text( _("Avg. Bitrate"), _("Avg. Bitrate"), buf );

	    ++group;
	    add_text( _("Language"), _("Language if known"), s.language );
	    ++group;
            add_text( _("Disposition"), _("Disposition of Track"),
                      s.disposition );

	    ++group;
	    add_time( _("Start"), _("Start of Subtitle"), s.start, img->fps() );
	    add_time( _("Duration"), _("Duration of Subtitle"), 
                      s.duration, img->fps() );

	    m_curr->relayout();
	    m_curr->parent()->relayout();
	  }

	m_subtitle->parent()->show();
      }


    relayout();
}

  void ImageInformation::refresh()
  {
    hide_tabs();

    m_image->clear();
    m_video->clear();
    m_audio->clear();
    m_subtitle->clear();
    m_iptc->clear();
    m_exif->clear();

    if ( img == NULL || !visible_r() ) return;

    if ( img->is_stereo() && (img->right_eye() || !img->is_left_eye()) )
        m_button->show();
    else
        m_button->hide();

    fill_data();


  }



  mrv::Browser* ImageInformation::add_browser( mrv::CollapsableGroup* g )
  {
      if (!g) return NULL;

      mrv::Browser* browser = new mrv::Browser( 0, 0, w(), 400 );
      browser->column_separator(true);
      browser->auto_resize( true );


      static const char* headers[] = { _("Attribute"), _("Value"), 0 };
      browser->column_labels( headers );
      int widths[] = { kMiddle, -1, 0 };
      browser->column_widths( widths );
      browser->align(fltk::ALIGN_CENTER|fltk::ALIGN_TOP);
      
      g->add( browser );
      if ( g->children() == 1 )
      {
          g->spacing( 0 );
      }
      else
      {
          g->spacing( int(browser->labelsize() + 4) );
      }

      g->show();

      group = row = 0; // controls line colors

      return browser;
  }

  int ImageInformation::line_height()
  {
    return 24;
  }

  fltk::Color ImageInformation::get_title_color()
  {
    return kTitleColors[ group % kSizeOfTitleColors ];
  }

  fltk::Color ImageInformation::get_widget_color()
  {
    fltk::Color col = kRowColors[ row % kSizeOfRowColors ];
    ++row;
    return col;
  }

  void ImageInformation::icc_callback( fltk::Widget* t, ImageInformation* v )
  {
    attach_icc_profile( v->get_image() );
    v->refresh(); // @todo: move this somewhere else
  }

  void ImageInformation::ctl_callback( fltk::Widget* t, ImageInformation* v )
  {
    attach_ctl_script( v->get_image() );
  }

  void ImageInformation::ctl_idt_callback( fltk::Widget* t,
                                         ImageInformation* v )
  {
    attach_ctl_idt_script( v->get_image() );
  }

  void ImageInformation::ctl_lmt_callback( fltk::Widget* t,
                                           CtlLMTData* c )
  {
      ImageInformation* v = (ImageInformation*) c->widget;
      size_t idx = c->idx;

      attach_ctl_lmt_script( v->get_image(), idx );
  }

  void ImageInformation::compression_cb( fltk::PopupMenu* t, ImageInformation* v )
  {
    unsigned   idx = t->value();
    CMedia* img = v->get_image();
    img->compression( idx );
    t->label( t->child(idx)->label() );
  }

  void ImageInformation::add_icc( const char* name,
                                  const char* tooltip,
				  const char* content,
				  const bool editable,
				  fltk::Callback* callback )
  { 
    if ( !content ) 
        content = _("None");

    if ( !editable )
        return add_text( name, tooltip, content );

    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
        fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->color( colA );
      widget->labelcolor( fltk::BLACK );
      widget->copy_label( name );
      g->add( widget );
    }

    {
      fltk::Group* sg = new fltk::Group( kMiddle, 0, g->w()-kMiddle, hh );

      fltk::Input* widget = new fltk::Input( 0, 0, sg->w()-50, hh );
      widget->value( content );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
      widget->tooltip( tooltip );
      if ( callback )
	widget->callback( (fltk::Callback*)icc_callback, (void*)this );

      sg->add( widget );

      fltk::Button* pick = new fltk::Button( sg->w()-50, 0, 50, hh, _("Load") );
      pick->callback( (fltk::Callback*)icc_callback, (void*)this );
      sg->add( pick );
      sg->resizable(widget);

      g->add( sg );
      g->resizable( sg );
    }

    m_curr->add( g );
  }

  void ImageInformation::add_ctl( const char* name,
                                  const char* tooltip,
				  const char* content,
				  const bool editable,
				  fltk::Callback* callback )
  { 
    if ( !editable )
        return add_text( name, tooltip, content );

    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
        fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->color( colA );
      widget->labelcolor( fltk::BLACK );
      widget->copy_label( name );
      g->add( widget );
    }

    {
      fltk::Group* sg = new fltk::Group( kMiddle, 0, g->w()-kMiddle, hh );

      fltk::Input* widget = new fltk::Input( 0, 0, sg->w()-50, hh );
      widget->value( content );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
      widget->tooltip( tooltip );
      if ( callback )
	widget->callback( (fltk::Callback*)ctl_callback, (void*)this );

      sg->add( widget );

      fltk::Button* pick = new fltk::Button( sg->w()-50, 0, 50, hh, _("Pick") );
      pick->callback( (fltk::Callback*)ctl_callback, this );
      sg->add( pick );
      sg->resizable(widget);

      g->add( sg );
      g->resizable( sg );
    }

    m_curr->add( g );
  }



  void ImageInformation::add_ctl_idt( const char* name,
                                      const char* tooltip,
                                      const char* content,
                                      const bool editable,
                                      fltk::Callback* callback )
  { 
    if ( !editable )
        return add_text( name, tooltip, content );

    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
        fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->color( colA );
      widget->labelcolor( fltk::BLACK );
      widget->copy_label( name );
      g->add( widget );
    }

    {
      fltk::Group* sg = new fltk::Group( kMiddle, 0, g->w()-kMiddle, hh );

      fltk::Input* widget = new fltk::Input( 0, 0, sg->w()-50, hh );
      widget->value( content );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
      widget->tooltip( tooltip );
      if ( callback )
	widget->callback( (fltk::Callback*)ctl_idt_callback, (void*)this );

      sg->add( widget );
      
      fltk::Button* pick = new fltk::Button( sg->w()-50, 0, 50, hh, _("Pick") );
      pick->callback( (fltk::Callback*)ctl_idt_callback, this );
      sg->add( pick );
      sg->resizable(widget);

      g->add( sg );
      g->resizable( sg );
    }

    m_curr->add( g );
  }



  void ImageInformation::add_ctl_lmt( const char* name,
                                      const char* tooltip,
                                      const char* content,
                                      const size_t idx,
                                      const bool editable,
                                      fltk::Callback* callback )
  { 
    if ( !editable )
        return add_text( name, tooltip, content );

    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->color( colA );
      widget->labelcolor( fltk::BLACK );
      widget->copy_label( name );
      g->add( widget );
    }

    {
      fltk::Group* sg = new fltk::Group( kMiddle, 0, g->w()-kMiddle, hh );

      fltk::Input* widget = new fltk::Input( 0, 0, sg->w()-50, hh );
      widget->value( content );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
      widget->tooltip( tooltip );

      CtlLMTData* c = new CtlLMTData;
      c->widget = this;
      c->idx    = idx;

      widget_data.push_back( c );

      if ( callback )
	widget->callback( callback, (void*)c );

      sg->add( widget );

      if ( content )
      {
          std::string n = content;
          n = n.substr( 4, 7 );
          if ( n == "SOPNode" || n == "SatNode" )
          {
              widget->w( sg->w() - 100 );
              fltk::Button* modify = new fltk::Button( sg->w()-100, 0, 50, hh,
                                                       _("Values") );
              mrv::ImageView* view = main()->uiView;
              modify->callback( (fltk::Callback*)modify_sop_sat_cb, view );
              sg->add( modify );
          }
      }
      
      fltk::Button* pick = new fltk::Button( sg->w()-50, 0, 50, hh, _("Pick") );
      pick->callback( (fltk::Callback*)ctl_lmt_callback, c );
      sg->add( pick );
      sg->resizable(widget);

      g->add( sg );
      g->resizable( sg );
    }

    m_curr->add( g );
  }


  void ImageInformation::add_text( const char* name,
                                   const char* tooltip,
				   const char* content,
				   const bool editable,
				   const bool active,
				   fltk::Callback* callback )
  { 
    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();


    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->color( colA );
      widget->labelcolor( fltk::BLACK );
      widget->copy_label( name );
      g->add( widget );
    }

    {
       fltk::Widget* widget = NULL;
       if ( !editable )
       {
	  fltk::Output* o = new fltk::Output( kMiddle, 0, w()-kMiddle, hh );
	  widget = o;
	  o->value( content );
       }
       else
       {
	  fltk::Input* o = new fltk::Input( kMiddle, 0, w()-kMiddle, hh );
	  widget = o;
	  o->value( content );
       }
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
      widget->tooltip( strdup(tooltip) );
      if ( !editable )
	{
	  widget->box( fltk::FLAT_BOX );
          g->deactivate();
	}
      else 
	{
	  if ( callback )
              widget->callback( callback, this );
          if (!active) g->deactivate();
	}
      g->add( widget );
      g->resizable( widget );
    }

    m_curr->add( g );
  }


  void ImageInformation::add_text( const char* name,
                                   const char* tooltip,
				   const std::string& content,
				   const bool editable,
                                   const bool active,
				   fltk::Callback* callback )
  {
      add_text( name, tooltip, content.c_str(), editable, active, callback );
  }

void ImageInformation::add_int( const char* name, const char* tooltip,
                                const int content, const bool editable,
                                const bool active,
                                fltk::Callback* callback, 
                                const int minV, const int maxV )
{

    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
        fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->labelcolor( fltk::BLACK );
      widget->copy_label( name );
      widget->color( colA );
      g->add( widget );
    }

    {
      fltk::Group* p = new fltk::Group( kMiddle, 0, w()-kMiddle, hh );
      p->box( fltk::FLAT_BOX );
      p->set_horizontal();
      p->begin();

      if ( !editable )
	{
	  fltk::IntInput* widget = new fltk::IntInput( 0, 0, p->w(), hh );
	  widget->value( (int)content );
	  widget->align(fltk::ALIGN_LEFT);
	  widget->color( colB );
	  widget->deactivate();
	  widget->box( fltk::FLAT_BOX );
          widget->tooltip( tooltip );
	}
      else 
	{
	  fltk::IntInput* widget = new fltk::IntInput( 0, 0, 50, hh );
	  widget->value( content );
	  widget->align(fltk::ALIGN_LEFT);
	  widget->color( colB );
          widget->tooltip( tooltip );

	  if ( callback ) widget->callback( callback, this );

	  fltk::Slider* slider = new fltk::Slider( 50, 0, p->w()-40, hh );
	  slider->type(fltk::Slider::TICK_ABOVE);
	  slider->minimum( minV );
	  int maxS = maxV;

	  if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
	  else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
	  else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
	  else if ( content > 100 && maxV <= 100 ) maxS = 1000;
          else if ( content > maxS ) maxS = content+50;
	  slider->maximum( maxS );

	  slider->value( content );
	  slider->step( 1.0 );
	  slider->linesize(1);
	  // slider->slider_size(10);
          slider->tooltip( tooltip );
	  slider->when( fltk::WHEN_RELEASE );
	  slider->callback( (fltk::Callback*)int_slider_cb, widget );

	  p->resizable(slider);
	}
      p->end();
      g->add( p );
      g->resizable( p );
      if ( !active ) g->deactivate();
    }
    m_curr->add( g );
  }

  void ImageInformation::add_enum( const char* name,
                                   const char* tooltip,
				   const size_t content,
				   const char* const* options,
				   const size_t num,
				   const bool editable,
				   fltk::Callback* callback 
				   )
  {
    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
        fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->labelcolor( fltk::BLACK );
      widget->copy_label( name );
      widget->color( colA );
      g->add( widget );
    }

    {
      fltk::PopupMenu* widget = new fltk::PopupMenu( kMiddle, 0, w()-kMiddle, hh );
      widget->type( 0 );
      widget->align( fltk::ALIGN_LEFT | fltk::ALIGN_INSIDE );
      widget->color( colB );
      for ( size_t i = 0; i < num; ++i )
	{
            widget->add( _( options[i] ) );
	}
      widget->value( unsigned(content) );
      widget->copy_label( _( options[content] ) );
      widget->tooltip( tooltip );

      if ( !editable )
	{
	  widget->deactivate();
	  widget->box( fltk::FLAT_BOX );
	}
      else 
	{
	  if ( callback )
	    widget->callback( callback, this );
	}
      g->add( widget );
      g->resizable( widget );
    }
    m_curr->add( g );

  }


  void ImageInformation::add_enum( const char* name,
                                   const char* tooltip,
				   const std::string& content,
				   stringArray& options,
				   const bool editable,
				   fltk::Callback* callback 
				   )
  {
    size_t index;
    stringArray::iterator it = std::find( options.begin(), options.end(),
					  content );
    if ( it != options.end() )
      {
	index = std::distance( options.begin(), it );
      }
    else
      {
	index = options.size();
	options.push_back( content );
      }

    size_t num = options.size();
    const char** opts = new const char*[num];
    for ( size_t i = 0; i < num; ++i )
      opts[i] = options[i].c_str();

    add_enum( name, tooltip,index, opts, num, editable, callback );

    delete [] opts;
  }




void ImageInformation::add_int( const char* name,
                                const char* tooltip,
                                const unsigned int content,
                                const bool editable,
                                const bool active,
                                fltk::Callback* callback, 
                                const unsigned int minV,
                                const unsigned int maxV )
{
    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
        fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->labelcolor( fltk::BLACK );
      widget->copy_label( name );
      widget->color( colA );
      g->add( widget );
    }

    {
      fltk::Group* p = new fltk::Group( kMiddle, 0, w()-kMiddle, hh );
      p->box( fltk::FLAT_BOX );
      p->set_horizontal();
      p->begin();

      if ( !editable )
	{
	  fltk::IntInput* widget = new fltk::IntInput( 0, 0, p->w(), hh );
	  widget->value( (int)content );
	  widget->align(fltk::ALIGN_LEFT);
	  widget->color( colB );
	  widget->deactivate();
	  widget->box( fltk::FLAT_BOX );
          widget->tooltip( tooltip );
	}
      else 
	{
	  fltk::IntInput* widget = new fltk::IntInput( 0, 0, 50, hh );
	  widget->value( (int)content );
	  widget->align(fltk::ALIGN_LEFT);
	  widget->color( colB );
          widget->tooltip( tooltip );

	  if ( callback ) widget->callback( callback, this );

	  fltk::Slider* slider = new fltk::Slider( 50, 0, p->w()-40, hh );
	  slider->type(fltk::Slider::TICK_ABOVE);
	  slider->minimum( minV );

	  unsigned maxS = maxV;
	  if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
	  else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
	  else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
	  else if ( content > 100 && maxV <= 100 ) maxS = 1000;
	  else if ( content > 10 && maxV <= 10 ) maxS = 100;
          else if ( content > maxS ) maxS = content+50;


	  slider->maximum( maxS );
	  slider->value( content );
	  slider->step( 1.0 );
	  slider->linesize(1);
          slider->tooltip( tooltip );
	  // slider->slider_size(10);
	  slider->when( fltk::WHEN_RELEASE );
	  slider->callback( (fltk::Callback*)int_slider_cb, widget );

	  p->resizable(slider);
	}
      p->end();
      g->add( p );
      g->resizable( p );
    }
    m_curr->add( g );

  }


void ImageInformation::add_time( const char* name, const char* tooltip,
                                 const double content,
                                 const double fps, const bool editable )
{
    boost::int64_t seconds = (boost::int64_t) content;
    int ms = (int) ((content - (double) seconds) * 1000);

    char buf[128];

    boost::int64_t frame = boost::int64_t( content * fps + 0.5 );
    if ( frame == 0 ) frame = 1;

    sprintf( buf, _( "Frame %" PRId64 " " ), frame );
    std::string text = buf;

    sprintf( buf, _("%" PRId64 " seconds %d ms."), seconds, ms );
    text += buf;

    if ( content > 60.0 )
      {
	int64_t hours, minutes;
	hours    = seconds / 3600;
	seconds -= hours * 3600;
	minutes  = seconds / 60;
	seconds -= minutes * 60;

	sprintf( buf, 
		 _(" ( %02" PRId64 ":%02" PRId64 ":%02" PRId64 "  %d ms. )"), 
		 hours, minutes, seconds, ms );
	text += buf;
      }

    add_text( name, tooltip, text, false );
}

void ImageInformation::add_int64( const char* name,
                                  const char* tooltip,
                                  const int64_t content )
{
    char buf[128];
    sprintf( buf, N_("%" PRId64), content );
    add_text( name, tooltip, buf, false );
}

void ImageInformation::add_rect( const char* name, const char* tooltip,
                                 const mrv::Recti& content,
                                 const bool editable, fltk::Callback* callback )
{
    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
        fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->labelcolor( fltk::BLACK );
      widget->copy_label( name );
      widget->color( colA );
      g->add( widget );
    }

    unsigned dw = (w() - kMiddle) / 6;
    fltk::Group* g2 = new fltk::Group( kMiddle, 0, w()-kMiddle, hh );
    g2->tooltip( tooltip );
    {
      fltk::IntInput* widget = new fltk::IntInput( 0, 0, dw, hh );
      widget->value( content.l() );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
      if ( !editable )
	{
	  widget->deactivate();
	  widget->box( fltk::FLAT_BOX );
	}
      else 
	{
	  if ( callback )
	    widget->callback( callback, img );
	}
      g2->add( widget );
    }
    {
      fltk::IntInput* widget = new fltk::IntInput( dw, 0, dw, hh );
      widget->value( content.t() );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
      if ( !editable )
	{
	  widget->deactivate();
	  widget->box( fltk::FLAT_BOX );
	}
      else 
	{
	  if ( callback )
	    widget->callback( callback, img );
	}
      g2->add( widget );
    }
    {
      fltk::IntInput* widget = new fltk::IntInput( dw*2, 0, dw, hh );
      widget->value( content.r() );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
      if ( !editable )
	{
	  widget->deactivate();
	  widget->box( fltk::FLAT_BOX );
	}
      else 
	{
	  if ( callback )
	    widget->callback( callback, img );
	}
      g2->add( widget );
    }
    {
      fltk::IntInput* widget = new fltk::IntInput( dw*3, 0, dw, hh );
      widget->value( content.b() );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
      if ( !editable )
	{
	  widget->deactivate();
	  widget->box( fltk::FLAT_BOX );
	}
      else 
	{
	  if ( callback )
	    widget->callback( callback, img );
	}
      g2->add( widget );
    }
    {
        fltk::IntInput* widget = new fltk::IntInput( dw*4, 0, dw, hh, "W:" );
        widget->value( content.w() );
        widget->align(fltk::ALIGN_LEFT);
        widget->box( fltk::FLAT_BOX );
        widget->color( colB );
        widget->deactivate();
        widget->box( fltk::FLAT_BOX );
        g2->add( widget );
    }
    {
        fltk::IntInput* widget = new fltk::IntInput( dw*5, 0, dw, hh, "H:" );
        widget->value( content.h() );
        widget->align(fltk::ALIGN_LEFT);
        widget->box( fltk::FLAT_BOX );
        widget->color( colB );
        widget->deactivate();
        widget->box( fltk::FLAT_BOX );
        g2->add( widget );
    }
    g->add( g2 );
    g->resizable( g2 );
    m_curr->add( g );
  }

  void ImageInformation::add_float( const char* name,
                                    const char* tooltip,
				    const float content, const bool editable,
                                    const bool active,
				    fltk::Callback* callback, 
				    const float minV, float maxV )
  {
    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->labelcolor( fltk::BLACK );
      widget->copy_label( name );
      widget->color( colA );
      g->add( widget );
    }

    {
      fltk::Group* p = new fltk::Group( kMiddle, 0, w()-kMiddle, hh );
      p->box( fltk::FLAT_BOX );
      p->set_horizontal();
      p->begin();

      if ( !editable )
	{
	  fltk::FloatInput* widget = new fltk::FloatInput( 0, 0, p->w(), hh );
	  widget->value( content );
	  widget->align(fltk::ALIGN_LEFT);
	  widget->color( colB );
	  widget->deactivate();
	  widget->box( fltk::FLAT_BOX );
          widget->tooltip( tooltip );
	}
      else 
	{
	  fltk::FloatInput* widget = new fltk::FloatInput( 0, 0, 50, hh );
	  widget->value( content );
	  widget->align(fltk::ALIGN_LEFT);
	  widget->color( colB );
          widget->tooltip( tooltip );

	  if ( callback ) widget->callback( callback, this );

	  fltk::Slider* slider = new fltk::Slider( 50, 0, p->w()-40, hh );
	  slider->type(fltk::Slider::TICK_ABOVE);
	  slider->step( 0.01 );
	  slider->minimum( minV );

	  double maxS = maxV;
	  if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
	  else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
	  else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
	  else if ( content > 100 && maxV <= 100 ) maxS = 1000;
	  else if ( content > 10 && maxV <= 10 ) maxS = 100;
          else if ( content > maxS ) maxS = content+50;

	  slider->maximum( maxS );
	  slider->value( content );
	  slider->linesize(1);
	  // slider->slider_size(10);
          slider->tooltip( tooltip );
	  slider->when( fltk::WHEN_CHANGED );
	  slider->callback( (fltk::Callback*)float_slider_cb, widget );

	  p->resizable(slider);
	}
      p->end();
      g->add( p );
      g->resizable( p );
      if ( !active ) g->deactivate();
    }
    m_curr->add( g );
  }

void ImageInformation::add_bool( const char* name,
                                 const char* tooltip,
                                 const bool content,
                                 const bool editable,
                                 fltk::Callback* callback )
{
    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );

    {
        fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh );
      widget->box( fltk::FLAT_BOX );
      widget->copy_label( name );
      widget->labelcolor( fltk::BLACK );
      widget->color( colA );
      g->add( widget );
    }

    {
      fltk::Input* widget = new fltk::Input( kMiddle, 0, w()-kMiddle, 20 );
      widget->value( content? _("Yes") : _("No") );
      widget->box( fltk::FLAT_BOX );
      widget->align(fltk::ALIGN_LEFT);
      widget->color( colB );
      widget->tooltip( tooltip );
      if ( !editable )
	{
	  widget->deactivate();
	  widget->box( fltk::FLAT_BOX );
	}
      else 
	{
	  if ( callback )
	    widget->callback( callback, img );
	}
      g->add( widget );
      g->resizable( widget );
    }

    m_curr->add( g );
  }


} // namespace mrv
