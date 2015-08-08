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
 * @file   slateImage.cpp
 * @author gga
 * @date   Fri Sep 21 01:30:40 2007
 * 
 * @brief  Simple image slate generator
 * 
 * 
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

#include <iostream>
using namespace std;

#include "slateImage.h"
#include "mrvIO.h"
#include "mrvString.h"
#include "mrvException.h"
#include <wand/magick-wand.h>

namespace 
{
  const char* kModule = "slate";
}

namespace mrv {


  slateImage::slateImage( const CMedia* src ) :
    CMedia()
  {
    _gamma = 1.0f;
    _internal = true;


    std::string name = "Slate ";
    name += src->fileroot();

    _fileroot = strdup( name.c_str() );

    _w = src->width();
    _h = src->height();


    _fstart = src->first_frame();
    _fend   = src->last_frame();
    _ctime = time(NULL);

    _frameStart = _frame_start = 1;
    _frameEnd   = _frame_end = 60;

    image_size( _w, _h );
    allocate_pixels( _fstart );

    default_layers();
  }


bool slateImage::initialize()
{ 
   return true;
}

bool slateImage::release()
{
   return true;
}

  void slateImage::draw_text( double x, double y, const char* text )
  { 
    // Draw shadow first
    PixelSetColor( pwand, "#000000" );
    DrawSetStrokeColor( dwand, pwand );
    DrawSetFillColor( dwand, pwand );
    DrawAnnotation( dwand, x+2, y+2, (boost::uint8_t*) text );

    // Draw text
    PixelSetColor( pwand, "#f0f0f0" );
    DrawSetFillColor( dwand, pwand );
    PixelSetColor( pwand, "#f0f0f000" );
    DrawSetStrokeColor( dwand, pwand );
    DrawAnnotation( dwand, x, y, (boost::uint8_t*) text );
  }

  void slateImage::draw_bars()
  {  
    unsigned int W = width();
    unsigned int H = height();

    double xinc = W / 4;
    double xh   = W / 8;
    double yh   = H * 0.15f;
    double x;

    DrawSkewX( dwand, -45 );

    PixelSetColor( pwand, N_("#d0d0d0") );
    DrawSetFillColor( dwand, pwand );

  
    for ( x = 0; x < W+xh; x += xinc )
      {
	DrawRectangle( dwand, x, 0, x+xh, yh );

        ExceptionType type = DrawGetExceptionType( dwand );
        if ( type != UndefinedException )
        {
            IMG_ERROR( DrawGetException( dwand, &type ) );
            break;
        }

      }

    for ( x = xh*4; x < W+xh*6; x += xinc )
      {
	DrawRectangle( dwand, x, H-yh+1, x+xh, H );

        ExceptionType type = DrawGetExceptionType( dwand );
        if ( type != UndefinedException )
        {
            IMG_ERROR( DrawGetException( dwand, &type ) );
            break;
        }
      }
    DrawSkewX( dwand, 45 );

  }

  void slateImage::draw_gradient()
  {  
    unsigned int W = width();
    unsigned int H = height();

    double pct = 0.15f;
    double steps = (double) H * (1.0 - pct * 2);

    double start[3] = { 0.866, 0.866, 0.866 };
    double stop[3]  = { 0.555, 0.555, 0.555 };
    double colorize[3] = { 0.1294, 0.247, 0.55f };
    for ( int i = 0; i < 3; ++i )
      {
	start[i] *= colorize[i];
	stop[i]  *= colorize[i];
      }

    double red_step =   (stop[0] - start[0]) / steps;
    double green_step = (stop[1] - start[1]) / steps;
    double blue_step =  (stop[2] - start[2]) / steps;

    PixelIterator* iter = NewPixelIterator(wand);
    PixelSetIteratorRow( iter, long(H*pct) );

    for ( unsigned y = 0; y < steps; ++y )
      {
	double red   = start[0] + red_step   * y;
	double green = start[1] + green_step * y;
	double blue  = start[2] + blue_step  * y;

	size_t num;
	PixelWand** pixels = PixelGetNextIteratorRow(iter, &num );

	for (unsigned x=0; x < W; ++x)
	  {
	    PixelSetRed(   pixels[x], red );
	    PixelSetGreen( pixels[x], green );
	    PixelSetBlue(  pixels[x], blue );
	    PixelSetAlpha( pixels[x], 1.0 );
	  }

	PixelSyncIterator(iter);
      }

    DestroyPixelIterator(iter);
  }

  bool slateImage::fetch( const boost::int64_t frame )
  {
    MagickBooleanType status;

    wand = NewMagickWand();
    if ( wand == NULL )
        EXCEPTION( _("Could not create wand") );

    unsigned int W = width();
    unsigned int H = height();

    MagickSetSize(wand, W, H);
    MagickReadImage(wand, N_("xc:black") ); 


    pwand = NewPixelWand();
    if ( pwand == NULL )
        EXCEPTION( _("Could not create pixel wand") );

    dwand = NewDrawingWand();
    if ( dwand == NULL )
        EXCEPTION( _("Could not create drawing wand") );
    DrawSetViewbox(dwand, 0, 0, W, H);




    draw_gradient();
    draw_bars();


    DrawSetFontSize( dwand, H / 13 );
    DrawSetFont( dwand, N_("Helvetica") );

    DrawSetTextAlignment( dwand, LeftAlign );

    double x = W * 0.10f;
    double y = H * 0.25f;
    double ybeg = y;
    double xbeg = x;
    double yinc = DrawGetFontSize( dwand );

    char buf[1024];

    const char* show = getenv( "SHOW" );
    if ( show == NULL ) show = getenv( "JOB" );
    if ( show == NULL ) show = "";


    //
    // Obtain seq, shot and take from filename
    //
    int take = 0;
    std::string file = name();
    stringArray tokens;

    mrv::split_string( tokens, file, "." );
    size_t numTokens = tokens.size();
    if ( numTokens >= 4 && numTokens <= 5 )
      {
	take = atoi( tokens[1].c_str() );
      }

    char* tmp = NULL;

    std::string seq, shot;
    tmp = getenv( "SEQ" );
    if ( tmp ) seq = tmp;
    else
    {
       seq = tokens[0];

       size_t pos = seq.find_first_of("0123456789");
       if ( pos != string::npos )
       {
	  seq = seq.substr( 0, pos );
       }
    }
       
    tmp = getenv( "SHOT" );
    if ( tmp ) shot = tmp;
    else shot = tokens[0];



    static const char* kTitles[] = {
      "Show",
      "Sequence",
      "Shot",
      "Take",
      "Date",
      "Artist",
      "Frames",
    };
    static unsigned kFrames = 6;

    static unsigned num = sizeof(kTitles) / sizeof(char*);
    for ( unsigned i = 0; i < num; ++i )
      {
	if ( i == kFrames && _fstart == _fend ) continue;

	sprintf( buf, "%s:", kTitles[i]);
	draw_text( x, y, buf );
	y += yinc;
      }

    y  = ybeg;
    x += W * 0.3f;


    DrawSetFontSize( dwand, H / 16 );
    PixelSetColor( pwand, "#f0f000" );
    DrawSetFillColor( dwand, pwand );

    draw_text(  x, y, show ); y += yinc;
    draw_text(  x, y, seq.c_str() );  y += yinc;
    draw_text(  x, y, shot.c_str() ); y += yinc;


    if ( take > 0 )
      {
	sprintf( buf, "%d", take);
	draw_text(  x, y, buf );
      }

    y += yinc;



    draw_text(  x, y, ::ctime(&_ctime) );
    y += yinc;


    char* user = getenv("USER");
    if ( user == NULL ) user = getenv("USERNAME");
    if ( user != NULL )
        draw_text( x, y, user );
    y += yinc;

    if ( _fstart != _fend )
      {
	sprintf( buf, "%" PRId64 "-%" PRId64 , _fstart, _fend );
	draw_text( x, y, buf );
	y += yinc;
      }

    DrawSetFontSize( dwand, H / 20 );
    yinc = DrawGetFontSize( dwand );

    x = xbeg;
    sprintf( buf, "%s", directory().c_str() );
    draw_text( x, y, buf );
    y += yinc;

    sprintf( buf, "%s", name().c_str() );
    draw_text( x, y, buf );
    y += yinc;

    try {

        status = MagickDrawImage( wand, dwand );
        if ( status != MagickTrue )
            EXCEPTION( _("Could not draw image") );
    }
    catch ( const mrv::exception& e )
    {
        ExceptionType severity;
        char* err = MagickGetException(wand, &severity);
        if ( severity != UndefinedException )
        {
            LOG_ERROR( err );
        }
    }


    MagickExportImagePixels(wand, 0, 0, W, H, "RGBA", FloatPixel, 
			    _hires->data().get() );

    DestroyPixelWand( pwand );
    DestroyDrawingWand( dwand );



    DestroyMagickWand( wand );

//     thumbnail_create();

    return true;
  }


} // namespace mrv
