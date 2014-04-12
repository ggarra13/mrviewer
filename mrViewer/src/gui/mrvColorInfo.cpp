/**
 * @file   mrvColorInfo.cpp
 * @author gga
 * @date   Wed Nov 08 05:32:32 2006
 * 
 * @brief  Color Info text display
 * 
 * 
 */
#include <string>
#include <sstream>
#include <limits>
#include <cmath>  // for isnan

#if defined(WIN32) || defined(WIN64)
# include "float.h"  // for isnan
# define isnan(x) _isnan(x)
#endif

#include <fltk/events.h>
#include <fltk/Menu.h>
#include <fltk/PopupMenu.h>
#include <fltk/Group.h>


#include "core/mrvThread.h"
#include "core/mrvColorSpaces.h"
#include "core/mrvString.h"
#include "core/mrvI8N.h"
#include "core/mrvColor.h"

#include "gui/mrvMedia.h"
#include "mrViewer.h"
#include "gui/mrvImageView.h"
#include "gui/mrvColorInfo.h"


namespace
{

  void copy_color_cb( void*, mrv::Browser* w )
  {
    size_t last;
    std::string line( w->child( w->value() )->label() );
    size_t start = line.find( '\t', 0 );
    line = line.substr( start + 1, line.size()-1 );
    while ( (start = line.find( '@', 0 )) != std::string::npos )
      {
	last = line.find( ';', start );
	if ( last == std::string::npos ) break;

	if ( start > 0 )
	  line = ( line.substr( 0, start-1) + 
		   line.substr( last + 1, line.size()-1 ) );
	else
	  line = line.substr( last + 1, line.size()-1 );
      }

    std::string copy = " ";
    last = 0;
    while ( (start = line.find_first_not_of( "\t ", last )) != 
	    std::string::npos )
      {
	last = line.find_first_of( "\t ", start );
	if ( last == std::string::npos ) last = line.size();

	copy += line.substr( start, last-start ) + " ";
      }

    // Copy text to both the clipboard and to X's XA_PRIMARY
    fltk::copy( copy.c_str(), unsigned( copy.size() ), true );
    fltk::copy( copy.c_str(), unsigned( copy.size() ), false );
  }

}

namespace mrv
{

  extern std::string float_printf( float x );


  class ColorBrowser : public mrv::Browser
  {
  public:
    ColorBrowser( int x, int y, int w, int h, const char* l = 0 ) :
      mrv::Browser( x, y, w, h, l )
    {
    }

    int mousePush( int x, int y )
    {
      if ( value() < 0 ) return 0;

      fltk::Menu menu(0,0,0,0);

      menu.add( _("Copy/Color"), 
	       fltk::COMMAND + 'C', 
	       (fltk::Callback*)copy_color_cb, (void*)this, 0);

      menu.popup( fltk::Rectangle( x, y, 80, 1) );
      return 1;
    }

    int handle( int event )
    {
      switch( event )
	{
	case fltk::PUSH:
	  if ( fltk::event_button() == 3 )
	    return mousePush( fltk::event_x(), fltk::event_y() );
	default:
	  int ok = fltk::Browser::handle( event );
	  
	  int line = value();
	  if (( line < 1 || line > 10 ) ||
	      ( line > 4 && line < 7 ))
	    {
	      value(-1);
	      return 0;
	    }
	  
	  return ok;
	}
    }
  };


ColorInfo::ColorInfo( int x, int y, int w, int h, const char* l ) :
  fltk::Group( x, y, w, h, l )
{
  area = new fltk::Widget( 0, 0, w, 50 );
  area->align( fltk::ALIGN_CENTER | fltk::ALIGN_INSIDE );

  int w5 = w / 5;
  static int col_widths[] = { w5, w5, w5, w5, w5, 0 };
  browser = new ColorBrowser( 0, area->h(), w, h - area->h() );
  browser->column_widths( col_widths );
  browser->resizable(browser);
  resizable(this);
}

void ColorInfo::update()
{
  mrv::media fg = uiMain->uiView->foreground();
  if (!fg) return;

  CMedia* img = fg->image();
  mrv::Rectd selection = uiMain->uiView->selection();
  update( img, selection );
}

void ColorInfo::update( const CMedia* src,
			const mrv::Rectd& selection )
{
  if ( !visible_r() ) return;

  browser->clear();
  area->copy_label( "" );

  std::ostringstream text;
  if ( src && (selection.w() > 0 || selection.h() < 0) )
    {
      CMedia::Pixel hmin( std::numeric_limits<float>::max(),
			      std::numeric_limits<float>::max(),
			      std::numeric_limits<float>::max(),
			      std::numeric_limits<float>::max() );

      CMedia::Pixel pmin( std::numeric_limits<float>::max(),
			      std::numeric_limits<float>::max(),
			      std::numeric_limits<float>::max(),
			      std::numeric_limits<float>::max() );
      CMedia::Pixel pmax( std::numeric_limits<float>::min(),
			      std::numeric_limits<float>::min(),
			      std::numeric_limits<float>::min(),
			      std::numeric_limits<float>::min() );
      CMedia::Pixel hmax( std::numeric_limits<float>::min(),
			      std::numeric_limits<float>::min(),
			      std::numeric_limits<float>::min(),
			      std::numeric_limits<float>::min() );
      CMedia::Pixel pmean( 0, 0, 0, 0 );
      CMedia::Pixel hmean( 0, 0, 0, 0 );


      unsigned int W = src->width();
      unsigned int H = src->height();

      unsigned count = 0;

      unsigned int xmin = (unsigned int)(W * selection.x());
      unsigned int ymin = (unsigned int)(H * selection.y());

      unsigned int xmax = xmin + (unsigned int)(W * selection.w()) - 1;
      unsigned int ymax = ymin + (unsigned int)(H * selection.h()) - 1;

      if ( xmax < xmin ) {
	 unsigned tmp = xmax;
	 xmax = xmin;
	 xmin = xmax;
      }

      if ( ymax < ymin ) { 
	 unsigned tmp = ymax;
	 ymax = ymin;
	 ymin = ymax;
      }

      assert( xmax <= W );
      assert( ymax <= H );
      assert( xmin <= W );
      assert( ymin <= H );
      assert( xmax <= xmin );
      assert( ymax <= ymin );

      
      mrv::BrightnessType brightness_type = (mrv::BrightnessType) 
	uiMain->uiLType->value();

      mrv::image_type_ptr pic = src->hires();
      if (!pic) return;

      mrv::image_type_ptr right = src->right();

      float gain  = uiMain->uiView->gain();
      float gamma = uiMain->uiView->gamma();
      float one_gamma = 1.0f / gamma;

      for ( unsigned y = ymin; y <= ymax; ++y )
	{
	  for ( unsigned x = xmin; x <= xmax; ++x, ++count )
	    {
               CMedia::Pixel rp;
               if ( x > W )
               {
                  rp = right->pixel( x-W, y );
               }
               else
               {
                  rp = pic->pixel( x, y );
               }

	      if ( isnan(rp.r) || isnan(rp.g) || isnan(rp.b) ||
		   isnan(rp.a) ) continue;

              rp.r = pow(rp.r * gain, one_gamma);
              rp.g = pow(rp.g * gain, one_gamma);
              rp.b = pow(rp.b * gain, one_gamma);

	      if ( rp.r < pmin.r ) pmin.r = rp.r;
	      if ( rp.g < pmin.g ) pmin.g = rp.g;
	      if ( rp.b < pmin.b ) pmin.b = rp.b;
	      if ( rp.a < pmin.a ) pmin.a = rp.a;

	      if ( rp.r > pmax.r ) pmax.r = rp.r;
	      if ( rp.g > pmax.g ) pmax.g = rp.g;
	      if ( rp.b > pmax.b ) pmax.b = rp.b;
	      if ( rp.a > pmax.a ) pmax.a = rp.a;

	      pmean.r += rp.r;
	      pmean.g += rp.g;
	      pmean.b += rp.b;
	      pmean.a += rp.a;
	      
	      CMedia::Pixel hsv;
	      
	      switch( uiMain->uiBColorType->value()+1 )
	      {
		 case color::kITU_709:
		    hsv = color::rgb::to_ITU709( rp );
		    break;
		 case color::kITU_601:
		    hsv = color::rgb::to_ITU601( rp );
		    break;
		 case color::kYDbDr:
		    hsv = color::rgb::to_YDbDr( rp );
		    break;
		 case color::kYIQ:
		    hsv = color::rgb::to_yiq( rp );
		    break;
		 case color::kYUV:
		    hsv = color::rgb::to_yuv( rp );
		    break;
		 case color::kCIE_Luv:
		    hsv = color::rgb::to_luv( rp );
		    break;
		 case color::kCIE_Lab:
		    hsv = color::rgb::to_lab( rp );
		    break;
		 case color::kCIE_xyY:
		    hsv = color::rgb::to_xyY( rp );
		    break;
		 case color::kCIE_XYZ:
		    hsv = color::rgb::to_xyz( rp );
		    break;
		 case color::kHSL:
		    hsv = color::rgb::to_hsl( rp );
		    break;
		 default:
		 case color::kHSV:
		    hsv = color::rgb::to_hsv( rp );
		    break;
	      }

	      hsv.a = calculate_brightness( rp, brightness_type );

	      if ( hsv.r < hmin.r ) hmin.r = hsv.r;
	      if ( hsv.g < hmin.g ) hmin.g = hsv.g;
	      if ( hsv.b < hmin.b ) hmin.b = hsv.b;
	      if ( hsv.a < hmin.a ) hmin.a = hsv.a;

	      if ( hsv.r > hmax.r ) hmax.r = hsv.r;
	      if ( hsv.g > hmax.g ) hmax.g = hsv.g;
	      if ( hsv.b > hmax.b ) hmax.b = hsv.b;
	      if ( hsv.a > hmax.a ) hmax.a = hsv.a;

	      hmean.r += hsv.r;
	      hmean.g += hsv.g;
	      hmean.b += hsv.b;
	      hmean.a += hsv.a;
	    }
	}

      pmean.r /= count;
      pmean.g /= count;
      pmean.b /= count;
      pmean.a /= count;

      hmean.r /= count;
      hmean.g /= count;
      hmean.b /= count;
      hmean.a /= count;

      static const char* kR = "@C0xFF808000;";
      static const char* kG = "@C0x80FF8000;";
      static const char* kB = "@C0x8080FF00;";
      static const char* kA = "@C0xB0B0B000;";

      static const char* kH = "@C0xB0B00000;";
      static const char* kS = "@C0xB0B00000;";
      static const char* kV = "@C0xB0B00000;";
      static const char* kL = "@C0xB0B0B000;";

      unsigned spanX = (xmax-xmin+1);
      unsigned spanY = (ymax-ymin+1);
      unsigned numPixels = spanX * spanY;

      text << std::endl
	   << _("Area") << ": (" << xmin << ", " << ymin 
	   << ") - (" << xmax 
	   << ", " << ymax << ")" << std::endl
	   << _("Size") << ": [ " << spanX << "x" << spanY << " ] = " 
	   << numPixels << " "
	   << ( numPixels == 1 ? _("pixel") : _("pixels") )
	   << std::endl;
      area->copy_label( text.str().c_str() );


      text.str("");
      text << "@b;\t"
	   << kR
	   << _("R") << "\t"
	   << kG
	   << _("G") << "\t"
	   << kB
	   << _("B") << "\t"
	   << kA
	   << _("A") << "@n;" 
	   << std::endl
	   << _("Maximum") << ":\t"
	   << float_printf(pmax.r) << "\t" 
	   << float_printf(pmax.g) << "\t" 
	   << float_printf(pmax.b) << "\t" 
	   << float_printf(pmax.a) << std::endl
	   << _("Minimum") << ":\t" 
	   << float_printf(pmin.r) << "\t" 
	   << float_printf(pmin.g) << "\t" 
	   << float_printf(pmin.b) << "\t" 
	   << float_printf(pmin.a) << std::endl;

      CMedia::Pixel r(pmax);
      r.r -= pmin.r;
      r.g -= pmin.g;
      r.b -= pmin.b;
      r.a -= pmin.a;

      text << _("Range") << ":\t" 
	   << float_printf(r.r) << "\t" 
	   << float_printf(r.g) << "\t" 
	   << float_printf(r.b) << "\t" 
	   << float_printf(r.a) << std::endl
	   << "@b;" << _("Mean") << ":@n;\t" 
	   << kR
	   << float_printf(pmean.r) << "\t" 
	   << kG
	   << float_printf(pmean.g) << "\t" 
	   << kB
	   << float_printf(pmean.b) << "\t" 
	   << kA
	   << float_printf(pmean.a) << std::endl
	   << std::endl
	   << "@b;\t";

      switch( uiMain->uiBColorType->value()+1 )
      {
	 case color::kITU_709:
	    text << kH << N_("7") << "\t"
		 << kS << N_("0") << "\t"
		 << kL << N_("9");
	    break;
	 case color::kITU_601:
	    text << kH << N_("6") << "\t"
		 << kS << N_("0") << "\t"
		 << kL << N_("1");
	    break;
	 case color::kYIQ:
	    text << kH << N_("Y") << "\t"
		 << kS << N_("I") << "\t"
		 << kL << N_("Q");
	    break;
	 case color::kYDbDr:
	    text << kH << N_("Y") << "\t"
		 << kS << N_("Db") << "\t"
		 << kL << N_("Dr");
	    break;
	 case color::kYUV:
	    text << kH << N_("Y") << "\t"
		 << kS << N_("U") << "\t"
		 << kL << N_("V");
	    break;
	 case color::kCIE_Luv:
	    text << kH << N_("L") << "\t"
		 << kS << N_("u") << "\t"
		 << kL << N_("v");
	    break;
	 case color::kCIE_Lab:
	    text << kH << N_("L") << "\t"
		 << kS << N_("a") << "\t"
		 << kL << N_("b");
	    break;
	 case color::kCIE_xyY:
	    text << kH << N_("x") << "\t"
		 << kS << N_("y") << "\t"
		 << kL << N_("Y");
	    break;
	 case color::kCIE_XYZ:
	    text << kH << N_("X") << "\t"
		 << kS << N_("Y") << "\t"
		 << kL << N_("Z");
	    break;
	 case color::kHSL:
	    text << kH << N_("H") << "\t"
		 << kS << N_("S") << "\t"
		 << kL << N_("L");
	    break;
	 case color::kHSV:
	 default:
	    text << kH << N_("H") << "\t"
		 << kS << N_("S") << "\t"
		 << kV << N_("V");
	    break;
      }

      text << "\t" << kL;

      switch( brightness_type )
	{
	case kAsLuminance:
	  text << N_("Y");
	  break;
	case kAsLumma:
	  text << N_("Y'");
	  break;
	case kAsLightness:
	  text << N_("L");
	  break;
	}

      text << "@n;" 
	   << std::endl
	   << _("Maximum") << ":\t"
	   << float_printf(hmax.r) << "\t" 
	   << float_printf(hmax.g) << "\t" 
	   << float_printf(hmax.b) << "\t" 
	   << float_printf(hmax.a) << std::endl
	   << _("Minimum") << ":\t" 
	   << float_printf(hmin.r) << "\t" 
	   << float_printf(hmin.g) << "\t" 
	   << float_printf(hmin.b) << "\t" 
	   << float_printf(hmin.a) << std::endl;

      r = hmax;
      r.r -= hmin.r;
      r.g -= hmin.g;
      r.b -= hmin.b;
      r.a -= hmin.a;

      text << _("Range") << ":\t" 
	   << float_printf(r.r) << "\t" 
	   << float_printf(r.g) << "\t" 
	   << float_printf(r.b) << "\t" 
	   << float_printf(r.a) << std::endl
	   << "@b;" << _("Mean") << ":@n;\t" 
	   << kH
	   << float_printf(hmean.r) << "\t" 
	   << kS
	   << float_printf(hmean.g) << "\t" 
	   << kV
	   << float_printf(hmean.b) << "\t" 
	   << kL
	   << float_printf(hmean.a);
    }

  stringArray lines;
  mrv::split_string( lines, text.str(), "\n" );
  stringArray::iterator i = lines.begin();
  stringArray::iterator e = lines.end();
  area->redraw_label();
  for ( ; i != e; ++i )
    {
      fltk::Widget* w = browser->add( (*i).c_str() );
      w->align( fltk::ALIGN_CENTER );
    }
}

}  // namespace mrv

