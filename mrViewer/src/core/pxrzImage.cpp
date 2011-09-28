// Information taken from affine toolkit:
//
//           When using the "zfile" driver to output a Z-file of 300x300,
//           I noticed that the size was about right for uncompressed IEEE
//           floating point data.  Checking 300 * 300 * 4 equals 360000 
//           and the Z file size was 360136 it looked like the file format 
//           could be just a header and the raw floating point data.  
//           
//           It appears that the header has some 32bit number, xres and 
//           yres followed by two transformation matrices and the IEEE 
//           floats in big-endian form.

#include <iostream>
#include <limits>

#if defined(WIN32) || defined(WIN64)
#  include <float.h>    // for isnan(), finite()
#  include <winsock2.h> // for ntohs(), ntohl()
#  define isnanf(x) _isnan(x)
#  define isinff(x) !_finite(x)
#  undef max
#else
#  include <netinet/in.h> // for ntohs(), ntohl()
#  include <cmath>        // for isnan(), isinf()
#endif


#include "pxrzImage.h"
#include "byteSwap.h"



namespace {

  const unsigned kPIXAR_MAGIC = 0x2f0867ab;

  //! Struct used for header of a Pixar shadow map file
  struct shadowHeader
  {
    unsigned int magic;
    short width;
    short height;
    float m1[16];
    float m2[16];
  };

} // namespace



namespace mrv {

  using namespace std;

  /** 
   * Constructor
   * 
   */
  pxrzImage::pxrzImage() :
    CMedia()
  {
  }



  /** 
   * Destructor
   * 
   */
  pxrzImage::~pxrzImage()
  {
  }



  /*! Test a block of data read from the start of the file to see if it
    looks like the start of an .map file. This returns true if the 
    data contains PXRZ's magic number (0x10) and a short of 1 in the
    as the number of components.
  */
  bool pxrzImage::test(const boost::uint8_t *data, unsigned len)
  {
    if (!data || len < 7) return false;

    if ( ntohl( *((unsigned int*)data) ) != kPIXAR_MAGIC )
      return false;

    return true;
  }




  /** 
   * Fetch the shadow map image
   * 
   * 
   * @return true on success, false if not
   */
  bool pxrzImage::fetch(const boost::int64_t frame)
  {
    int dw, dh;

    FILE* f = fopen( filename(), "rb" );
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
    PixelType* pixels = (PixelType*)_hires->data().get();
    for (int y = 0, i = 0; y < dh; ++y)
      {
	int offset = y * dw;
	for (int x = 0; x < dw; ++x, ++i)
	  {
	    float t = buf[offset + x];
	    MAKE_BIGENDIAN( t );
	    if ( isinff(t) || isnanf(t) || 
		 t > 10000000000000000000000000.0f )
	      {
		pixels[i].r = pixels[i].g = pixels[i].b = 
		  pixels[i].a = std::numeric_limits<float>::quiet_NaN();
	      }
	    else
	      {
		pixels[i].r = pixels[i].g = pixels[i].b = t;
		pixels[i].a = 1.0f;
	      }
	  }
      }

    delete [] buf;

    fclose(f);
    return true;
  }



} // namespace mrv 
