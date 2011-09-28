/**
 * @file   mrayImage.cpp
 * @author 
 * @date   Thu Oct 12 03:35:52 2006
 * 
 * @brief  Handle most of mental ray image file formats
 *         (.ct, .mt, .st, .nt, .zt, .bit)
 * 
 */

#include <iostream>
#include <limits>

#include <sys/stat.h>   // for stat

#if defined(WIN32) || defined(WIN64)
#include <windows.h>    // for ntohs, etc.
#else
#include <netinet/in.h> // for ntohs, etc
#endif


#include "mrayImage.h"
#include "byteSwap.h"

#undef max

namespace {

  const char kAlphaType    = 0x04;  // 
  const char kZType        = 0x08;
  const char kNormalType   = 0x09;
  const char kTagType      = 0x0a;
  const char kColorType    = 0x0b;  // RGBA
  const char kMotionType   = 0x0c;
  const char kCoverageType = 0x0f;
  const char kBitType      = 0x0d; // ????

  //! Struct used for header of a mray shadow mt file
  struct mrayHeader
  {
    char  type;
    char  pad;
    unsigned short width;
    unsigned short height;
    short unknown;

    void swap()
    {
      width   = ntohs( width  );
      height  = ntohs( height );
      unknown = ntohs( unknown );
    }
  };



  const char* kMrayFormats[] = {
    "mental images' color image",
    "mental images' scalar image",
    "mental images' motion image",
    "mental images' normal image",
    "mental images' coverage image",
    "mental images' depth image",
    "mental images' tag image",
    "mental images' bit image",
  };

} // namespace


namespace mrv {

  mrayImage::mrayImage() :
    CMedia(),
    _format( 0 )
  {
  }

  mrayImage::~mrayImage()
  {
  }


  /*! Test a block of data read from the start of the file to see if it
    looks like the start of an .mt file. This returns true if the 
    data contains mentalray's magic numbers for specific image type
    and a short of 0 as the unknown number.
  */
  bool mrayImage::test(const boost::uint8_t *data, unsigned len)
  {
    mrayHeader* header = (mrayHeader*) data;
    header->swap();
    switch( header->type )
      {
      case kZType:
      case kMotionType:
      case kTagType:
      case kBitType:
      case kColorType:
      case kNormalType:
      case kAlphaType:
      case kCoverageType:
	break;
      default:
	return false;
      }

    if ( header->pad != 0 || header->unknown != 0 )
      return false;

    return true;
  }





  bool mrayImage::fetch( const boost::int64_t& frame ) 
  {
    int dw, dh;

    struct stat sbuf;
    std::string file = sequence_filename(frame);
    int result = stat( file.c_str(), &sbuf );
    if ( result == -1 ) return false;

    FILE* f = fopen( file.c_str(), "rb" );
    mrayHeader header;
    fread( &header, sizeof(header), 1, f );

  
    // This is the real byte size of file on disk
    size_t size = sbuf.st_size;
    size -= sizeof( mrayHeader );


    header.swap();

    dw = header.width;
    dh = header.height;

    image_size( dw, dh );
    allocate_pixels( frame );

    _layers.clear();
    _num_channels = 0;

    short comps = 3;
    int bits = 1;
    _format = 0;
    switch( header.type )
      {
      case kBitType:
	_format = 7;
	_layers.push_back( "Bits" );
	bits = sizeof(char); comps = 1; break;
      case kTagType:
	_format = 6;
	_layers.push_back( "Tag" );
	bits = sizeof(unsigned int); comps = 1; break;
      case kAlphaType:
	_format = 1;
	default_layers(); comps = 1;
	bits = sizeof(float);
	if ( size == dw * dh * comps * sizeof(short) )
	  bits = sizeof(short);
	else if ( size == dw * dh * comps * sizeof(char) )
	  bits = sizeof(char);
	break;
      case kColorType:
	_format = 0;
	default_layers(); comps = 4;
	bits = sizeof(float);
	if ( size == dw * dh * comps * sizeof(short) )
	  bits = sizeof(short);
	else if ( size == dw * dh * comps * sizeof(char) )
	  bits = sizeof(char);
	break;
      case kCoverageType:
	_format = 4;
	_layers.push_back( "Coverage" );
	bits = sizeof(float); comps = 1; break;
      case kZType:
	_format = 5;
	_layers.push_back( "Z Depth" );
	bits = sizeof(float); comps = 1; break;
      case kNormalType:
	_format = 3;
	_layers.push_back( "Normals" );
	bits = sizeof(float); break;
      case kMotionType:
	_format = 2;
	_layers.push_back( "Motion" );
	bits = sizeof(float); break;
      default:
	break;
      }

    // Read pixel values
    size_t total = dw * dh * bits * comps;

    boost::uint8_t* data = new boost::uint8_t[total]; 
    float* bufF = (float*) data;
    unsigned short* bufS = (unsigned short*) data;
    unsigned int*  bufI = (unsigned int*) data;

    size_t sum = 0;
    while ( !feof(f) && (sum < total) )
      {
	sum += fread( &data[sum], sizeof(char), total, f );
      }

    fclose(f);


    PixelType* pixels = (PixelType*)_hires->data().get();

    // Copy pixel values
    for (int y = 0, i = 0; y < dh; ++y)
      {
	int offset  = (dh - y - 1) * dw * comps;
	for (int x = 0; x < dw; ++x, ++i)
	  {
	    int j  = offset + x * comps;

	    float t[4] = { 0.f, 0.f, 0.f, 0.f };

	    switch( bits )
	      {
	      case sizeof(char):
		t[0] = t[1] = t[2] = data[j] / 255.0f;  
	      if ( comps > 1 ) t[1] = data[j + 1] / 255.0f;
	      if ( comps > 2 ) t[2] = data[j + 2] / 255.0f;
	      if ( comps > 3 ) t[3] = data[j + 3] / 255.0f;
	      break;
	      case sizeof(short):
		t[0] = t[1] = t[2] = ntohs( bufS[j] ) / 65535.0f;  
	      if ( comps > 1 ) t[1] = ntohs( bufS[j + 1] ) / 65535.0f;
	      if ( comps > 2 ) t[2] = ntohs( bufS[j + 2] ) / 65535.0f;
	      if ( comps > 3 ) t[3] = ntohs( bufS[j + 3] ) / 65535.0f;
	      break;
	      case sizeof(float):
		if ( header.type == kTagType )
		  {
		    t[0] = t[1] = t[2] = (float) bufI[j];  
		    if ( comps > 1 ) t[1] = (float) bufI[j + 1];
		    if ( comps > 2 ) t[2] = (float) bufI[j + 2];
		    if ( comps > 3 ) t[3] = (float) bufI[j + 3];
		  }
		else
		  {
		    t[0] = t[1] = t[2] = bufF[j];  
		    if ( comps > 1 ) t[1] = bufF[j + 1];
		    if ( comps > 2 ) t[2] = bufF[j + 2];
		    if ( comps > 3 ) t[3] = bufF[j + 3];
		  }
	      MAKE_BIGENDIAN(t[0]);
	      MAKE_BIGENDIAN(t[1]);
	      MAKE_BIGENDIAN(t[2]);
	      MAKE_BIGENDIAN(t[3]);
	      break;
	      }


	    pixels[i].r = t[0];
	    pixels[i].g = t[1];
	    pixels[i].b = t[2];
	    pixels[i].a = t[3];
	  }
      }


    delete [] data;

    refresh();
    return true;
  }



  const char* const mrayImage::format() const
  {
    return kMrayFormats[ _format ];
  }


} // namespace mrv
