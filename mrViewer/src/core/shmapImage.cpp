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
 * @file   shmapImage.cpp
 * @author gga
 * @date   Fri Sep 21 01:28:25 2007
 * 
 * @brief  Image reader for mental ray shadow maps.
 * 
 * 
 */

#include <iostream>
#include <limits>


#if defined(WIN32) || defined(WIN64)
#include <windows.h>  // for htonl, etc.
#undef max
#else
#include <netinet/in.h>
#endif

#include <fltk/run.h>

#include "shmapImage.h"
#include "byteSwap.h"



namespace {


  //! Struct used for header of a mray shadow map file
  struct shadowHeader
  {
    char x;         // magic number: 0x10
    short width;
    short height;
    short comps;
  };

} // namespace


namespace mrv {


  using namespace std;

  /** 
   * Constructor
   * 
   */
  shmapImage::shmapImage() :
    CMedia()
  {
  }



  /** 
   * Destructor
   * 
   */
  shmapImage::~shmapImage()
  {
  }



  /*! Test a block of data read from the start of the file to see if it
    looks like the start of an .map file. This returns true if the 
    data contains SHMAP's magic number (0x10) and a short of 1 in the
    as the number of components.
  */
  bool shmapImage::test(const boost::uint8_t *data, unsigned len)
  {
    if (!data || len < 7) return false;
    if ( data[0] != 0x10 ) return false;

    shadowHeader* header = (shadowHeader*) data;

    // For shadow map, components have to be one.
    if ( ntohs( header->comps ) != 1 || ntohs( header->width ) <= 0 ||
	 ntohs( header->height) <= 0 )
      return false;

    return true;
  }




  /** 
   * Fetch the shadow map image
   * 
   * 
   * @return true on success, false if not
   */
  bool shmapImage::fetch(const boost::int64_t frame) 
  {
    int dw, dh;

    FILE* f = fltk::fltk_fopen( sequence_filename(frame).c_str(), "rb" );
    shadowHeader header;
    size_t sum = fread( &header, sizeof(shadowHeader), 1, f );
    if ( sum != 1 )
      {
	fclose(f);
	cerr << "Could not load shadow map image" << endl;
	return false;
      }

    dw = ntohs( header.width );
    dh = ntohs( header.height );

    image_size( dw, dh );
    allocate_pixels(frame);

    _layers.clear();
    _layers.push_back( "Z Depth" );

    _num_channels = 1;
    _pixel_ratio = 1.0;
    _gamma = 1.0f;

    const char* ch = channel();
    if ( !ch || (strcmp( ch, "Z Depth" ) != 0) )
      {
	fclose(f);
	return true;
      }

    // Read pixel values
    size_t total = dw * dh;
    float* buf = new float[total]; 
    sum = 0;
    while ( !feof(f) && (sum < total) )
      {
	sum += fread( &buf[sum], sizeof(float), total, f );
      }

    // Copy pixel values
    Pixel* pixels = (Pixel*)_hires->data().get();
    for (int y = 0, i = 0; y < dh; ++y)
      {
	int offset = (dh - y - 1) * dw;
	for (int x = 0; x < dw; ++x, ++i)
	  {
	    float t = buf[offset + x];
	    MAKE_BIGENDIAN( t );
	    if ( t > 0.0f )
	      {
		pixels[i].r = pixels[i].g = pixels[i].b = t;
		pixels[i].a = 1.0f;
	      }
	    else
	      {
		pixels[i].b = 0.5f;
		pixels[i].r = pixels[i].g = 
		  pixels[i].a = std::numeric_limits<float>::quiet_NaN();
	      }
	  }
      }

    delete [] buf;


    fclose(f);
    return true;
  }


} // namespace mrv
