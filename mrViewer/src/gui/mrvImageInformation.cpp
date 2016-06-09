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

#include "mrvImageInformation.h"
#include "mrvFileRequester.h"
#include "mrvPreferences.h"
#include "mrvMath.h"
#include "mrvMedia.h"
#include "mrvImageView.h"
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
        int v = w->ivalue();
        if ( v < img->start_frame() )
            v = img->start_frame();
        if ( v > img->last_frame() )
            v = img->last_frame();
        w->value( v );

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
          img->eye_separation( w->fvalue() );
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
        int v = w->ivalue();
        if ( v < img->first_frame() )
            v = img->first_frame();
        if ( v > img->end_frame() )
            v = img->end_frame();
        w->value( v );

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

double ImageInformation::to_memory( double value,
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

void ImageInformation::fill_data()
{

    m_curr = add_browser(m_image);


    
    add_text( _("Directory"), img->directory() );

    char buf[1024];
    add_text( _("Filename"), img->name().c_str() );


    ++group;

    unsigned int num_video_streams = unsigned( img->number_of_video_streams() );
    unsigned int num_audio_streams = unsigned( img->number_of_audio_streams() );
    unsigned int num_subtitle_streams = unsigned( img->number_of_subtitle_streams() );
    if ( img->has_video() || img->has_audio() )
      {
	add_int( _("Video Streams"), num_video_streams );
	add_int( _("Audio Streams"), num_audio_streams );
	add_int( _("Subtitle Streams"), num_subtitle_streams );
      }
    else
      {
	add_bool( _("Sequence"), img->is_sequence() );
      }

    if ( img->first_frame() != img->last_frame() )
      {
          add_int( _("First Frame"), (int)img->first_frame(), true,
		  (fltk::Callback*)change_first_frame_cb, 1, 50 );
          add_int( _("Last Frame"), (int)img->last_frame(), true,
		  (fltk::Callback*)change_last_frame_cb, 2, 55 );
      }


    
    add_int64( _("Frame Start"), img->start_frame() );
    add_int64( _("Frame End"), img->end_frame() );


    add_float( _("FPS"), (float) img->fps(), true, 
               (fltk::Callback*)change_fps_cb, 1.0f, 100.0f );


    ++group;
    
    add_int( _("Width"), img->width() );
    add_int( _("Height"), img->height() );


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
    add_text( _("Aspect Ratio"), buf );
    add_float( _("Pixel Ratio"), float(img->pixel_ratio()), true,
	       (fltk::Callback*)change_pixel_ratio_cb, 0.01f, 4.0f );


    const mrv::Recti& window = img->data_window();
    const mrv::Recti& dwindow = img->display_window();
    if ( dwindow != window )
    {
        add_rect( _("Data Window"), window );
    }


    add_rect( _("Display Window"), dwindow );


    if ( ! img->is_stereo() )
    {
        const mrv::Recti& window2 = img->data_window2();
        if ( window != window2 )
        {
            add_rect( _("Data Window 2"), window2 );
        }

        const mrv::Recti& dwindow2 = img->display_window2();
        if ( dwindow != dwindow2 )
        {
            add_rect( _("Display Window 2"), dwindow2 );
        }


        if ( img->right() )
            add_float( _("Eye Separation"), img->eye_separation(), true,
                       (fltk::Callback*)eye_separation_cb, -20.0f, 20.0f );
    }
    else
    {
        add_float( _("Eye Separation"), img->eye_separation(), true,
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


    add_text( _("Depth"), depth );
    add_int( _("Image Channels"), img->number_of_channels() );

    aviImage* avi = dynamic_cast< aviImage* >( img );
    if ( avi )
    {
        add_enum( _("Color Space"), avi->colorspace_index(), kColorSpaces, 
                  11, true, (fltk::Callback*)change_colorspace );
        add_text( _("Color Range"), _(avi->color_range()) );
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

    add_text( _("Render Pixel Format"), format );



    
    static const char* kRenderingIntent[] = {
      _("Undefined"),
      _("Saturation"),
      _("Perceptual"),
      _("Absolute"),
      _("Relative"),
    };

    

    add_text( _("Rendering Intent"), 
	      kRenderingIntent[ (int) img->rendering_intent() ] );
 
    
    

    add_float( _("Gamma"), img->gamma(), true, 
	       (fltk::Callback*)change_gamma_cb, 0.01f,	4.0f );


    if ( img->has_chromaticities() )
    {
        const Imf::Chromaticities& c = img->chromaticities(); 
        sprintf( buf, _("R: %g %g    G: %g %g    B: %g %g"),
                 c.red.x, c.red.y, c.green.x, c.green.y,
                 c.blue.x, c.blue.y );
        add_text( _("CIExy Chromaticities"), buf );
        sprintf( buf, _("W: %g %g"),c.white.x, c.white.y );
        add_text( _("CIExy White Point"), buf );
    }

    

    add_ctl_idt( _("Input Device Transform"), img->idt_transform() );

    

        clear_callback_data();

        
    {

        unsigned count = (unsigned) img->number_of_lmts();
        for ( unsigned i = 0; i <= count; ++i )
        {
            sprintf( buf, _("LMTransform %u"), i+1 );
            add_ctl_lmt( strdup( buf ), img->look_mod_transform(i), i );
        }
    }
    


    add_ctl( _("Render Transform"), img->rendering_transform() );
    

    add_icc( _("ICC Profile"), img->icc_profile() );
    

    ++group;
    

    add_text( _("Format"), img->format() );
    
    
    if ( !img->has_video() )
      {
	add_text( _("Line Order"), img->line_order() );
      }



    if ( !img->has_video() )
    {
        ++group;

        add_text( _("Compression"), img->compression() );

    }

    ++group;


    const char* space_type = NULL;
    double memory_space = double( to_memory( img->memory(), space_type ) );
    sprintf( buf, N_("%.3f %s"), memory_space, space_type );
    add_text( _("Memory"), buf );


    
    

    if ( img->disk_space() >= 0 )
      {
	double disk_space = double( to_memory( img->disk_space(),
						     space_type ) );
	double pct   = double( 100.0 * ( (long double) img->disk_space() /
                                         (long double) img->memory() ) );
	

	sprintf( buf, N_("%.3f %s  (%.2f %% of memory size)"), 
		 disk_space, space_type, pct );
	add_text( _("Disk space"), buf );


        if ( !img->has_video() )
        {
            double ratio = 100.0 - double(pct);
            sprintf( buf, _("%.2g %%"), ratio );
            add_text( _("Compression Ratio"), buf );
        }
      }
    

    ++group;
    add_text( _("Creation Date"), img->creation_date() );

    

    
    m_curr->relayout();

    


    m_image->relayout();
    m_image->show();


    


    CMedia::Attributes attrs = img->iptc();
    if ( ! attrs.empty() )
      {

	m_curr = add_browser(m_iptc);
	CMedia::Attributes::const_iterator i = attrs.begin();
	CMedia::Attributes::const_iterator e = attrs.end();
	for ( ; i != e; ++i )
	   add_text( i->first.c_str(), i->second.c_str(), false );

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
                      add_int( _("Mipmap Level"), exr->levelX(), true,
                               (fltk::Callback*)change_mipmap_cb, 0, 20 );
                      exr->levelY( exr->levelX() );
                  }
              }
              if ( i->first == _("X Ripmap Levels") )
              {
                  exrImage* exr = dynamic_cast< exrImage* >( img );
                  if ( exr )
                  {
                      add_int( _("X Ripmap Level"), exr->levelX(), true,
                               (fltk::Callback*)change_x_ripmap_cb, 0, 20 );
                  }
              }
              if ( i->first == _("Y Ripmap Levels") )
              {
                  exrImage* exr = dynamic_cast< exrImage* >( img );
                  if ( exr )
                  {
                      add_int( _("Y Ripmap Level"), exr->levelY(), true,
                               (fltk::Callback*)change_y_ripmap_cb, 0, 20 );
                  }
              }

	    add_text( i->first.c_str(), i->second.c_str(), false );
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
	    add_bool( _("Known Codec"), s.has_codec );
	    add_text( _("Codec"), s.codec_name );
	    add_text( _("FourCC"), s.fourcc );
	    add_bool( _("B Frames"), s.has_b_frames );
	    ++group;

	    add_text( _("Pixel Format"), s.pixel_format );
	    ++group;



	    const char* name = "";
	    if      ( is_equal( s.fps, 29.97 ) )     name = "(NTSC)";
	    else if ( is_equal( s.fps, 30.0 ) )      name = "(60hz HDTV)";
	    else if ( is_equal( s.fps, 25.0 ) )      name = "(PAL)";
	    else if ( is_equal( s.fps, 24.0 ) )      name = "(Film)";
	    else if ( is_equal( s.fps, 50.0 ) )      name = _("(PAL Fields)");
	    else if ( is_equal( s.fps, 59.940059 ) ) name = _("(NTSC Fields)");

	    sprintf( buf, "%g %s", s.fps, name );
	    add_text( _("FPS"), buf );


	    ++group;
	    add_time( _("Start"), s.start, s.fps );
	    add_time( _("Duration"), s.duration, s.fps );

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


	    add_bool( _("Known Codec"), s.has_codec );
	    add_text( _("Codec"), s.codec_name );
	    add_text( _("FourCC"), s.fourcc );
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

	    add_text( _("Format"), s.format );
	    add_text( _("Channels"), channels );
	    sprintf( buf, _("%d Hz."), s.frequency );
	    add_text( _("Frequency"), buf );
	    sprintf( buf, _("%d kb/s"), s.bitrate/1000 );
	    add_text( _("Avg. Bitrate"), buf );

	    ++group;
	    add_text( _("Language"), s.language );

	    ++group;

	    add_time( _("Start"), s.start, img->fps() );
	    add_time( _("Duration"), s.duration, img->fps() );

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

	    add_bool( _("Known Codec"), s.has_codec );
	    add_text( _("Codec"), s.codec_name );
	    add_text( _("FourCC"), s.fourcc );
	    ++group;

	    sprintf( buf, _("%d kb/s"), s.bitrate/1000 );
	    add_text( _("Avg. Bitrate"), buf );

	    ++group;
	    add_text( _("Language"), s.language );

	    ++group;
	    add_time( _("Start"), s.start, img->fps() );
	    add_time( _("Duration"), s.duration, img->fps() );

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
				  const char* content,
				  const bool editable,
				  fltk::Callback* callback )
  { 
    if ( !content ) 
        content = _("None");

    if ( !editable )
      return add_text( name, content );

    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, name );
      widget->box( fltk::FLAT_BOX );
      widget->color( colA );
      widget->labelcolor( fltk::BLACK );
      g->add( widget );
    }

    {
      fltk::Group* sg = new fltk::Group( kMiddle, 0, g->w()-kMiddle, hh );

      fltk::Input* widget = new fltk::Input( 0, 0, sg->w()-50, hh );
      widget->value( content );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
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
				  const char* content,
				  const bool editable,
				  fltk::Callback* callback )
  { 
    if ( !editable )
      return add_text( name, content );

    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, name );
      widget->box( fltk::FLAT_BOX );
      widget->color( colA );
      widget->labelcolor( fltk::BLACK );
      g->add( widget );
    }

    {
      fltk::Group* sg = new fltk::Group( kMiddle, 0, g->w()-kMiddle, hh );

      fltk::Input* widget = new fltk::Input( 0, 0, sg->w()-50, hh );
      widget->value( content );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
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
                                      const char* content,
                                      const bool editable,
                                      fltk::Callback* callback )
  { 
    if ( !editable )
      return add_text( name, content );

    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, name );
      widget->box( fltk::FLAT_BOX );
      widget->color( colA );
      widget->labelcolor( fltk::BLACK );
      g->add( widget );
    }

    {
      fltk::Group* sg = new fltk::Group( kMiddle, 0, g->w()-kMiddle, hh );

      fltk::Input* widget = new fltk::Input( 0, 0, sg->w()-50, hh );
      widget->value( content );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );
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
                                      const char* content,
                                      const size_t idx,
                                      const bool editable,
                                      fltk::Callback* callback )
  { 
    if ( !editable )
      return add_text( name, content );

    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, name );
      widget->box( fltk::FLAT_BOX );
      widget->color( colA );
      widget->labelcolor( fltk::BLACK );
      g->add( widget );
    }

    {
      fltk::Group* sg = new fltk::Group( kMiddle, 0, g->w()-kMiddle, hh );

      fltk::Input* widget = new fltk::Input( 0, 0, sg->w()-50, hh );
      widget->value( content );
      widget->align(fltk::ALIGN_LEFT);
      widget->box( fltk::FLAT_BOX );
      widget->color( colB );


      CtlLMTData* c = new CtlLMTData;
      c->widget = this;
      c->idx    = idx;

      widget_data.push_back( c );

      if ( callback )
	widget->callback( callback, (void*)c );

      sg->add( widget );
      
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
				   const char* content,
				   const bool editable,
				   fltk::Callback* callback )
  { 
    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, 
					       strdup( name ) );
      widget->box( fltk::FLAT_BOX );
      widget->color( colA );
      widget->labelcolor( fltk::BLACK );
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
      if ( !editable )
	{
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


  void ImageInformation::add_text( const char* name,
				   const std::string& content,
				   const bool editable,
				   fltk::Callback* callback )
  {
    add_text( name, content.c_str(), editable );
  }

  void ImageInformation::add_int( const char* name,
				  const int content, const bool editable,
				  fltk::Callback* callback, 
				  const int minV, const int maxV )
  {

     fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, name );
      widget->box( fltk::FLAT_BOX );
      widget->labelcolor( fltk::BLACK );
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
	}
      else 
	{
	  fltk::IntInput* widget = new fltk::IntInput( 0, 0, 50, hh );
	  widget->value( content );
	  widget->align(fltk::ALIGN_LEFT);
	  widget->color( colB );

	  if ( callback ) widget->callback( callback, this );

	  fltk::Slider* slider = new fltk::Slider( 50, 0, p->w()-40, hh );
	  slider->type(fltk::Slider::TICK_ABOVE);
	  slider->minimum( minV );
	  unsigned maxS = maxV;

	  if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
	  else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
	  else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
	  else if ( content > 100 && maxV <= 100 ) maxS = 1000;
          else if ( content > maxV ) maxS = content+50;
	  slider->maximum( maxS );

	  slider->value( content );
	  slider->step( 1.0 );
	  slider->linesize(1);
	  // slider->slider_size(10);
	  slider->when( fltk::WHEN_CHANGED );
	  slider->callback( (fltk::Callback*)int_slider_cb, widget );

	  p->resizable(slider);
	}
      p->end();
      g->add( p );
      g->resizable( p );
    }
    m_curr->add( g );
  }

  void ImageInformation::add_enum( const char* name,
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
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, name );
      widget->box( fltk::FLAT_BOX );
      widget->labelcolor( fltk::BLACK );
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

    add_enum( name, index, opts, num, editable, callback );

    delete [] opts;
  }




  void ImageInformation::add_int( const char* name,
				  const unsigned int content,
				  const bool editable,
				  fltk::Callback* callback, 
				  const unsigned int minV,
				  const unsigned int maxV )
  {
     fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, name );
      widget->box( fltk::FLAT_BOX );
      widget->labelcolor( fltk::BLACK );
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
	}
      else 
	{
	  fltk::IntInput* widget = new fltk::IntInput( 0, 0, 50, hh );
	  widget->value( (int)content );
	  widget->align(fltk::ALIGN_LEFT);
	  widget->color( colB );

	  if ( callback ) widget->callback( callback, this );

	  fltk::Slider* slider = new fltk::Slider( 50, 0, p->w()-40, hh );
	  slider->type(fltk::Slider::TICK_ABOVE);
	  slider->minimum( minV );

	  unsigned maxS = maxV;
	  if ( content > 100000 && maxV <= 100000 ) maxS = 1000000;
	  else if ( content > 10000 && maxV <= 10000 ) maxS = 100000;
	  else if ( content > 1000 && maxV <= 1000 ) maxS = 10000;
	  else if ( content > 100 && maxV <= 100 ) maxS = 1000;
          else if ( content > maxV ) maxS = content+50;
	  slider->maximum( maxS );
	  slider->value( content );
	  slider->step( 1.0 );
	  slider->linesize(1);
	  // slider->slider_size(10);
	  slider->when( fltk::WHEN_CHANGED );
	  slider->callback( (fltk::Callback*)int_slider_cb, widget );

	  p->resizable(slider);
	}
      p->end();
      g->add( p );
      g->resizable( p );
    }
    m_curr->add( g );

  }


  void ImageInformation::add_time( const char* name, const double content,
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

    add_text( name, text, false );
  }

  void ImageInformation::add_int64( const char* name,
				    const int64_t content )
  {
    char buf[128];
    sprintf( buf, N_("%" PRId64), content );
    add_text( name, buf, false );
  }

  void ImageInformation::add_rect( const char* name, const mrv::Recti& content, 
				   const bool editable, fltk::Callback* callback )
  {
    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, name );
      widget->box( fltk::FLAT_BOX );
      widget->labelcolor( fltk::BLACK );
      widget->color( colA );
      g->add( widget );
    }

    unsigned dw = (w() - kMiddle) / 6;
    fltk::Group* g2 = new fltk::Group( kMiddle, 0, w()-kMiddle, hh );
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
				    const float content, const bool editable,
				    fltk::Callback* callback, 
				    const float minV, float maxV )
  {
    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );
    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, name );
      widget->box( fltk::FLAT_BOX );
      widget->labelcolor( fltk::BLACK );
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
	}
      else 
	{
	  fltk::FloatInput* widget = new fltk::FloatInput( 0, 0, 50, hh );
	  widget->value( content );
	  widget->align(fltk::ALIGN_LEFT);
	  widget->color( colB );

	  if ( callback ) widget->callback( callback, this );

	  fltk::Slider* slider = new fltk::Slider( 50, 0, p->w()-40, hh );
	  slider->type(fltk::Slider::TICK_ABOVE);
	  slider->step( 0.01 );
	  slider->minimum( minV );
	  slider->maximum( maxV );
	  slider->value( content );
	  slider->linesize(1);
	  // slider->slider_size(10);
	  slider->when( fltk::WHEN_CHANGED );
	  slider->callback( (fltk::Callback*)float_slider_cb, widget );

	  p->resizable(slider);
	}
      p->end();
      g->add( p );
      g->resizable( p );
    }
    m_curr->add( g );
  }

  void ImageInformation::add_bool( const char* name,
				   const bool content,
				   const bool editable,
				   fltk::Callback* callback )
  {
    fltk::Color colA = get_title_color();
    fltk::Color colB = get_widget_color();

    int hh = line_height();
    fltk::Group* g = new fltk::Group( 0, 0, w(), hh );

    {
      fltk::Widget* widget = new fltk::Widget( 0, 0, kMiddle, hh, name );
      widget->box( fltk::FLAT_BOX );
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
