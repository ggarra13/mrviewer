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
 * @file   mapImage.cpp
 * @author gga
 * @date   Wed Jan 17 02:33:00 2007
 * 
 * @brief  A class used to read mentalray's map files.
 * 
 * 
 */

#include "mapImage.h"

#include <sys/stat.h>

#include <iostream>
#include <limits>

#if defined(WIN32) || defined(WIN64)
#  include <windows.h>  // for htonl, etc.
#  undef max
#else
#  include <netinet/in.h>  // for htonl, etc.
#endif

#include <fltk/run.h>

#include "byteSwap.h"
#include "mrvThread.h"


// #ifndef DEBUG
// #define DEBUG
// #endif



namespace {

  static const char* kLineOrders[] =
    {
      "Increasing Y",
      "Tiled",
    };

#define MAKE_TAG(a,b,c,d) (unsigned int) ( (unsigned int)(a << 24)	\
					   + (unsigned int)(b << 16)	\
					   + (unsigned int)(c << 8)	\
					   + (unsigned int)d)

  static const unsigned kMAP_MAGIC = MAKE_TAG('M', 'I', 't', 'x');


  //! Struct used for header of a mray shadow map file
  struct mapHeader
  {
    unsigned int magic;  // 'MItx'

    unsigned int something1;  // 2   version?
    unsigned int something2;  // 1

    unsigned crapA[13];     // 64bytes

    //   short    offset[14];  

    float    filter;    // 1.0f
    unsigned dirsize;   // valid # of filter levels

    int   offset[20];  /* byte offsets used by imf_disp -p */

    int width, height;
    int bits,  comp;
    unsigned char local;   
    unsigned char writable;
    unsigned char cacheable;
    unsigned char remap;
    int   type;

    float gamma;
    float aspect;

    //  unsigned char pad[72];  // to reach 256 bytes

    void swap()
    {
      width  = ntohl( width  );
      height = ntohl( height );
      bits   = ntohl( bits );
      comp   = ntohl( comp );
      //     levelX = ntohs( levelX );
      //     levelY = ntohs( levelY );

      //     ByteSwap( gamma );
      //     ByteSwap( aspect );
    }
  };

} // namespace


namespace mrv {

  using namespace std;

  mapImage::mapImage() :
    CMedia(),
    _stub( false ),
    _atime( 0 )
  {
  }

  mapImage::~mapImage()
  {
  }


  /*! Test a block of data read from the start of the file to see if it
    looks like the start of an .map file. This returns true if the 
    data contains MAP's magic number (0x10) and a short of 1 in the
    as the number of components.
  */
  bool mapImage::test(const boost::uint8_t *data, unsigned len)
  {
    mapHeader* header = (mapHeader*) data;
    if ( header->magic != kMAP_MAGIC && 
	 ntohl( header->magic ) != kMAP_MAGIC )
      return false;

    return true;
  }
 




  bool mapImage::fetch(const boost::int64_t frame ) 
  {
    int dw, dh;

     SCOPED_LOCK( _mutex );

    _stub = is_stub();

    FILE* f = fltk::fltk_fopen( sequence_filename(frame).c_str(), "rb" );
    mapHeader header;

    size_t ok = fread( &header, sizeof(header), 1, f );
    if (!ok) return false;

    bool swap = false;

    if ( header.magic != kMAP_MAGIC )
      swap = true;


    if ( _stub )
      {
	// if it is a mray stub... store access time
	struct stat sbuf;
	int result = stat( sequence_filename(frame).c_str(), &sbuf );
	if ( result == -1 ) return false;
	_atime = sbuf.st_atime;
      }

    if ( swap )
      {
	header.swap();
      }


    dw = header.width;
    dh = header.height;
    _lineOrder = 0;

    image_size( dw, dh );
    allocate_pixels(frame);


    // Skip other data we don't know ( this offset is still wrong
    // for many images)
    int bits   = header.bits / 8;


#ifdef DEBUG
    cerr << "sizeof header: " << sizeof(mapHeader) << endl;
    cerr << "swap: " << swap << endl;
#endif


    int      h = dh % 480;  // 480

    // 800x362  w = 0
  

    //   int w = dw / 640? 640 : 0;

    int n = dw / 640;
    int w = n ? 640 * n : 0;

    if ( w < 640 ) {
      h = dh%640;
    }

    int offset = w * 12 + h * 16;

#ifdef DEBUG
    cerr << endl << name() << endl;
    cerr << "w,h: " << dw << "," << dh << endl;
    cerr << "bits: " << header.bits << endl;
    cerr << "comp: " << header.comp << endl;

    //   cerr << "local: " << header.local << endl;
    //   cerr << "write: " << header.writable << endl;
    //   cerr << "cache: " << header.cacheable << endl;
    //   cerr << "remap: " << header.remap << endl;
    //   cerr << "type: " << header.type << endl;

    //   cerr << "levX: " << header.levelX << endl;
    //   cerr << "levY: " << header.levelY << endl;

    //   for ( unsigned i = 0; i < 13; ++i )
    //     cerr << "mipmap crap: " << header.crapA[i] << endl;

    //   for ( unsigned i = 0; i < 19; ++i )
    //     cerr << "mipmap offset: " << header.offset[i] << endl;

    cerr << "offset w: " << w << endl;
    cerr << "offset h: " << h << endl;
    cerr << "offset  : " << offset << endl;
#endif

    //   fseek( f, 288, SEEK_CUR );

    fseek( f, offset, SEEK_CUR );


    // Read pixel values
    short  comps = header.comp;
    size_t total = dw * dh * bits * comps;

    unsigned short*  bufS;
    float* bufF;
    boost::uint8_t* data = new boost::uint8_t[total]; 
    bufF = (float*) data;
    bufS = (unsigned short*) data;

    size_t sum = 0;
    while ( !feof(f) && (sum < total) )
      {
	sum += fread( &data[sum], sizeof(char), total, f );
      }


    Pixel* pixels = (Pixel*)_hires->data().get();

    // Copy pixel values
    for (int y = 0, i = 0; y < dh; ++y)
      {
	int offset  = (dh - y - 1) * dw * comps;
	for (int x = 0; x < dw; ++x, ++i)
	  {
	    int j  = offset + x;
	    float t[4] = { 0.f, 0.f, 0.f, 0.f };

	    switch( header.bits )
	      {
	      case 8:
		t[0] = t[1] = t[2] = data[j] / 255.0f;  
		if ( comps > 1 ) t[1] = data[j + dw] / 255.0f;
		if ( comps > 2 ) t[2] = data[j + dw*2] / 255.0f;
		if ( comps > 3 ) t[3] = data[j + dw*3] / 255.0f;
		break;
	      case 16:
		t[0] = t[1] = t[2] = ntohs( bufS[j] ) / 65535.0f;  
		if ( comps > 1 ) t[1] = ntohs( bufS[j + dw] ) / 65535.0f;
		if ( comps > 2 ) t[2] = ntohs( bufS[j + dw*2] ) / 65535.0f;
		if ( comps > 3 ) t[3] = ntohs( bufS[j + dw*3] ) / 65535.0f;
		break;
	      case 32:
		t[0] = t[1] = t[2] = bufF[j];  
		if ( comps > 1 ) t[1] = bufF[j + dw];
		if ( comps > 2 ) t[2] = bufF[j + dw*2];
		if ( comps > 3 ) t[3] = bufF[j + dw*3];
		ByteSwap( t[0] );
		ByteSwap( t[1] );
		ByteSwap( t[2] );
		ByteSwap( t[3] );
		break;
	      }

	    pixels[i].r = t[0];
	    pixels[i].g = t[1];
	    pixels[i].b = t[2];
	    pixels[i].a = t[3];
	  }
      }


    delete [] data;


    fclose(f);

    _layers.clear();
    _num_channels = 0;
    default_layers();

    return true;
  }

 
  const char* const mapImage::line_order() const
  {
    return kLineOrders[_lineOrder];
  }


  bool mapImage::has_changed()
  {
    if ( CMedia::has_changed() )
      return true;

    if ( !_stub ) return false;

    // it is a mray stub... check access time too
    struct stat sbuf;
    int result = stat( filename(), &sbuf );
    if ( result == -1 ) return false;

    if ( _atime == sbuf.st_atime )
      return false;

    return true;
  }

  bool mapImage::is_stub()
  {
    const std::string& fname = name();
    if ( fname.length() < 8 )
      return false;

    if ( fname[0] != 'f' ||
	 fname[1] != 'b' ||
	 fname[2] < '0'  ||
	 fname[2] > '9'  ||
	 fname[3] < '0'  ||
	 fname[3] > '9'  ||
	 fname[4] < '0'  ||
	 fname[4] > '9' )
      return false;

    return true;
  }


} // namespace mrv
