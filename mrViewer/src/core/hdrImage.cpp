/**
 * @file   hdrImage.cpp
 * @author gga
 * @date   Fri Jul 20 04:54:04 2007
 * 
 * @brief  Class used to load Radiance HDR format
 * 
 * 
 */


#include  <stdio.h>

#include <iostream>
#include <algorithm>

#include <fltk/run.h>

#include "ImathMath.h" // for Math:: functions
#include "hdrImage.h"
#include "mrvException.h"

#include "mrvOS.h"
#include "mrvIO.h"


using namespace std;

#define  RED		0
#define  GRN		1
#define  BLU		2
#define  CIEX		0	/* or, if input is XYZ... */
#define  CIEY		1
#define  CIEZ		2
#define  EXP		3	/* exponent same for either format */
#define  COLXS		128	/* excess used for exponent */
#define  WHT		3	/* used for RGBPRIMS type */


#define  MINELEN	8	/* minimum scanline length for encoding */
#define  MAXELEN	0x7fff	/* maximum scanline length for encoding */
#define  MINRUN		4	/* minimum run length */



#define  copycolr(c1,c2)	(c1[0]=c2[0],c1[1]=c2[1], \
				c1[2]=c2[2],c1[3]=c2[3])

namespace mrv {


  hdrImage::hdrImage() :
    CMedia(),
    cieXYZ(false),
    flipX( false ),
    flipY( false ),
    exposure(1.0f)
  {
  }

  hdrImage::~hdrImage()
  {
  }


  /*! Test a block of data read from the start of the file to see if it
    looks like the start of an .exr file. This returns true if the 
    data contains HDR's magic comment.
  */
  bool hdrImage::test(const boost::uint8_t *data, unsigned)
  {
    if ( strncmp( (char*)data, "#?RADIANCE", 10 ) != 0 ) return false;

    return true;
  }


  void hdrImage::read_header( FILE* f ) 
  {
    char line[256];

    _num_channels = 0;
    _gamma = 1.0f;
    _layers.clear();

    cieXYZ = flipX = flipY = false;

    unsigned int w = 0;
    unsigned int h = 0;
    
    while ( fgets( line, 256, f ) != NULL )
      {
	char* s = line;
	while(isspace(*s)) ++s;   // skip spaces
	if ( !*s ) continue;


	if ( s[0] == '#' ) continue; // comment


	// remove \n \r
	char* e = s + strlen(s) - 1;
	for ( ; *e == ' ' || *e == '\r' || *e == '\n'; --e )
	  *e = 0;


	char* state;
	char* keyword = strtok_r( s, "=", &state );
	if (!keyword ) continue;

	if ( strcasecmp( keyword, "FORMAT" ) == 0 )
	  {
	    rgb_layers();
	    lumma_layers();

	    char* val = strtok_r( NULL, "=", &state );

	    if ( strcasecmp( val, "32-bit_rle_rgbe" ) == 0 )
	      {
		cieXYZ = false;
	      }
	    else if ( strcasecmp( val, "32-bit_rle_xyze" ) == 0 )
	      {
		cieXYZ = true;
	      }
	    else
	      {
		std::string err = "Unknown Radiance HDR format \"";
		err += val;
		err += "\"";
		EXCEPTION(err);
	      }
	  }
	else if ( strcasecmp( keyword, "OWNER" ) == 0 )
	  {
	    static const std::string key = "Owner";
	    std::string val = strtok_r( NULL, "=", &state );
	    _exif.insert( std::make_pair( key, val ) ); 
	  }
	else if ( strcasecmp( keyword, "CAPDATE" ) == 0 )
	  {
	    static const std::string key = "Capture Date";

	    struct tm tms;
	    if (sscanf(s, "%d:%d:%d %d:%d:%d",
		       &tms.tm_year, &tms.tm_mon, &tms.tm_mday,
		       &tms.tm_hour, &tms.tm_min, &tms.tm_sec) != 6)
	      continue;

	    tms.tm_mon--;
	    tms.tm_year -= 1900;
	    tms.tm_isdst = -1;	// ask mktime() to figure out DST
	    
	    time_t t = mktime( &tms );

	    char now[128];
	    strftime( now, 127, "%H:%M:%S - %d %b", localtime(&t) );
	    
	    std::string val = now;

	    _exif.insert( std::make_pair( key, val ) ); 
	    continue;
	  }
	else if ( strcasecmp( keyword, "EXPOSURE" ) == 0 )
	  {
	    static const std::string key = "Exposure";
	    char* val = strtok_r( NULL, "=", &state );

	    _exif.insert( std::make_pair( key, std::string(val) ) ); 

	    exposure = (float) atof( val );
	    continue;
	  }
	else if ( strcasecmp( keyword, "COLORCORR" ) == 0 )
	  {
	    const char* val = strtok_r( NULL, "=", &state );
	    if ( sscanf( val, "%f %f %f ",
			 &corr[0], &corr[1], &corr[2] ) != 3 )
	      continue;

	    continue;
	  }
	else if ( strcasecmp( keyword, "SOFTWARE" ) == 0 )
	  {
	    static const std:: string key = "Software";
	    std::string val = strtok_r( NULL, "=", &state );
	    _exif.insert( std::make_pair( key, val ) ); 
	    continue;
	  }
	else if ( strcasecmp( keyword, "PIXASPECT" ) == 0 )
	  {
	    const char* val = strtok_r( NULL, "=", &state );
	    _pixel_ratio = (float) atof( val );

	    if ( _pixel_ratio <= 0.0f ) _pixel_ratio = 1.0f;
	    continue;
	  }

	else if ( strcasecmp( s, "VIEW" ) == 0 )
	  {
	    // @todo:
	    continue;
	  }
	else if ( strcasecmp( s, "PRIMARIES" ) == 0 )
	  {
	    const char* val = strtok_r( NULL, "=", &state );
	    if ( sscanf( val, "%f %f %f %f %f %f %f %f",
			 &cieXY[0].x, &cieXY[0].y, 
			 &cieXY[1].x, &cieXY[1].y,
			 &cieXY[2].x, &cieXY[2].y,
			 &cieXY[3].x, &cieXY[3].y ) != 8 )
	      continue;


	    continue;
	  }
	else
	  {
	    // Split on spaces
	    state = NULL;
	    char* ptr = strtok_r( s, " ", &state ); // Y keyword
	    if ( !ptr || strlen(ptr) != 2 ) continue;


	    if ( ptr[1] == 'Y' )
	      {
		if ( ptr[0] != '-' ) flipY = true;

		ptr = strtok_r( NULL, " ", &state ); // Y value
		h = atoi( ptr );

		ptr = strtok_r( NULL, " ", &state ); // X keyword
		if ( !ptr || strlen(ptr) != 2 ) continue;

		if ( ptr[1] == 'X' )
		  {
		    if ( ptr[0] != '+' ) flipX = true;

		    ptr = strtok_r( NULL, " ", &state ); // X value
		    w = atoi( ptr );
		  }
		break;
	      }
	    else
	      {
		std::string err = "Unknown Radiance HDR keyword \"";
		err += keyword;
		err += "\"";
		EXCEPTION(err);
	      }
	  }
      }

    if ( w == 0 || h == 0 ) EXCEPTION("resolution not found");

    image_size( w, h );
  }


  int
  hdrImage::oldreadcolrs(COLR* scanline, int len, FILE* fp)
  {
    int  rshift;
    register int  i;
	
    rshift = 0;
	
    while (len > 0) {
      scanline[0][RED] = getc(fp);
      scanline[0][GRN] = getc(fp);
      scanline[0][BLU] = getc(fp);
      scanline[0][EXP] = getc(fp);
      if (feof(fp) || ferror(fp))
	return(-1);
      if (scanline[0][RED] == 1 &&
	  scanline[0][GRN] == 1 &&
	  scanline[0][BLU] == 1) {
	for (i = scanline[0][EXP] << rshift; i > 0; i--) {
	  copycolr(scanline[0], scanline[-1]);
	  scanline++;
	  len--;
	}
	rshift += 8;
      } else {
	scanline++;
	len--;
	rshift = 0;
      }
    }
    return(0);
  }


  int
  hdrImage::read_colors(COLR* scanline, int len, FILE* fp)
  {
    register int  i, j;
    int  code, val;
    /* determine scanline type */
    if ((len < MINELEN) | (len > MAXELEN))
      return(oldreadcolrs(scanline, len, fp));
    if ((i = getc(fp)) == EOF)
      return(-1);
    if (i != 2) {
      ungetc(i, fp);
      return(oldreadcolrs(scanline, len, fp));
    }
    scanline[0][GRN] = getc(fp);
    scanline[0][BLU] = getc(fp);
    if ((i = getc(fp)) == EOF)
      return(-1);
    if (scanline[0][GRN] != 2 || scanline[0][BLU] & 128) {
      scanline[0][RED] = 2;
      scanline[0][EXP] = i;
      return(oldreadcolrs(scanline+1, len-1, fp));
    }
    if ((scanline[0][BLU]<<8 | i) != len)
      return(-1);		/* length mismatch! */
    /* read each component */
    for (i = 0; i < 4; i++)
      for (j = 0; j < len; ) {
	if ((code = getc(fp)) == EOF)
	  return(-1);
	if (code > 128) {	/* run */
	  code &= 127;
	  if ((val = getc(fp)) == EOF)
	    return -1;
	  if (j + code > len)
	    return -1;	/* overrun */
	  while (code--)
	    scanline[j++][i] = val;
	} else {		/* non-run */
	  if (j + code > len)
	    return -1;	/* overrun */
	  while (code--) {
	    if ((val = getc(fp)) == EOF)
	      return -1;
	    scanline[j++][i] = val;
	  }
	}
      }
    return(0);
  }


  void hdrImage::colr2color( PixelType& col, COLR clr )
  {
    double  f;
    
    col.a = 1.0;

    if (clr[EXP] == 0)
      col.r = col.r = col.b = 0.0;
    else {
      f = ldexp(1.0, (int)clr[EXP]-(COLXS+8));
      col.r = float( (clr[RED] + 0.5)*f );
      col.g = float( (clr[GRN] + 0.5)*f );
      col.b = float( (clr[BLU] + 0.5)*f );
    }
  }


  /** 
   * Fetch the current HDR image
   * 
   * 
   * @return true if success, false if not
   */
  bool hdrImage::fetch( const boost::int64_t frame ) 
  {

    try {

       FILE* f = fltk::fltk_fopen( sequence_filename(frame).c_str(), "rb" );
       if ( f == NULL ) EXCEPTION("could not open file");

      read_header(f);
      allocate_pixels(frame);

      unsigned w = width();
      COLR* scanline = (COLR*) malloc( sizeof(COLR) * w );
      if ( !scanline ) EXCEPTION("could not allocate scanline");

      unsigned h = height();
      unsigned y = 0;
      unsigned ylast = h;
      int      ys = 1;

      if ( flipY )
	{
	  y = h;
	  ylast = 0;
	  ys = -1;
	}

      PixelType* pixels = (PixelType*)_hires->data().get();
      for ( ; y != ylast; y += ys )
	{
	  read_colors(scanline, w, f);

	  unsigned x = 0;
	  int xs = 1;
	  unsigned xlast = w;
	  if ( flipX )
	    {
	      x = w;
	      xlast = 0;
	      xs = -1;
	    }

	  for ( ; x != xlast; x += xs )
	    {
	      PixelType& p = pixels[ y * w + x];
	      colr2color( p, scanline[x] );
	    }


	}

      free( scanline );

      fclose(f);

    } 
    catch( const std::exception& e )
      {
	mrvALERT( e.what() );
	return false;
      }


    return true;
  }


} // namespace mrv
