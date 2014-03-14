/**
 * @file   iffImage.cpp
 * @author gga
 * @date   Thu Jul 19 13:02:54 2007
 * 
 * @brief  Class used to read Maya IFF files
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

#include "iffImage.h"
#include "byteSwap.h"
#include "mrvIO.h"
#include "mrvThread.h"

namespace 
{
  const char* kModule = "iff";
}


namespace mrv {

  const char* iffImage::kCompression[] = {
    _("None"),
    _("RLE"),
  };

#define MAKE_TAG(a,b,c,d) (unsigned int) ( (unsigned int)(a << 24)	\
					   + (unsigned int)(b << 16)	\
					   + (unsigned int)(c << 8)	\
					   + (unsigned int)d)

  static const unsigned int kFORM_TAG = MAKE_TAG('F','O','R','M');
  static const unsigned int kCAT_TAG  = MAKE_TAG('C','A','T',' ');


  // Common IFF tags
  static const unsigned int kCIMG_TAG = MAKE_TAG('C','I','M','G');
  static const unsigned int kRGBA_TAG = MAKE_TAG('R','G','B','A');

  // Amiga IFF tags
  static const unsigned int kBMHD_TAG = MAKE_TAG('B','M','H','D');  // Amiga IFF header info
  static const unsigned int kILBM_TAG = MAKE_TAG('I','L','B','M');

  // Maya IFF tags
  static const unsigned int kFOR4_TAG = MAKE_TAG('F','O','R','4');
  static const unsigned int kIFF_MAGIC = kFOR4_TAG;
  static const unsigned int kTBHD_TAG = MAKE_TAG('T','B','H','D');  // Maya  IFF header info
  static const unsigned int kAUTH_TAG = MAKE_TAG('A','U','T','H');
  static const unsigned int kDATE_TAG = MAKE_TAG('D','A','T','E');
  static const unsigned int kCLPZ_TAG = MAKE_TAG('C','L','P','Z');  // clipping planes, two floats
  static const unsigned int kESXY_TAG = MAKE_TAG('E','S','X','Y');  // eye x-y ratios
  static const unsigned int kHIST_TAG = MAKE_TAG('H','I','S','T');
  static const unsigned int kVERS_TAG = MAKE_TAG('V','E','R','S');
  static const unsigned int kTBMP_TAG = MAKE_TAG('T','B','M','P');
  static const unsigned int kZBUF_TAG = MAKE_TAG('Z','B','U','F');

  /// Ignored tags
  static const unsigned int kPICS_TAG = MAKE_TAG('P','I','C','S');
  static const unsigned int kTEXT_TAG = MAKE_TAG('T','E','X','T');
  static const unsigned int kPROP_TAG = MAKE_TAG('P','R','O','P');
  static const unsigned int kLIST_TAG = MAKE_TAG('L','I','S','T');

  enum Flags
    {
      kHasRGB = 1,
      kHasAlpha = 2,
      kHasZ = 4,
      kHas12Bit = 8192,
    };


  struct iffHeader
  {
    unsigned int width;
    unsigned int height;
    unsigned short prnum;
    unsigned short prden;
    unsigned int flags;
    unsigned short bytes;
    unsigned short tiles;
    unsigned int compress;
    int orgx, orgy;

    int depth;

    iffHeader() :
      orgx( 0 ),
      orgy( 0 )
    {
    }

    void swap()
    {
      width  = ntohl( width );
      height = ntohl( height );
      prnum  = ntohs( prnum );
      prden  = ntohs( prden );
      flags  = ntohl( flags );
      bytes  = ntohs( bytes );
      tiles  = ntohs( tiles );
      compress = ntohl( compress );
      orgx   = ntohl( orgx );
      orgy   = ntohl( orgy );

      depth = 3 * ((flags & kHasRGB) != 0) + 1 * ((flags & kHasAlpha) != 0 );

#ifdef DEBUG_IFF
      using namespace std;
      cerr << "width: " << width << endl;
      cerr << "height: " << height << endl;
      cerr << "prnum: " << prnum << endl;
      cerr << "prden: " << prden << endl;
      cerr << "flags: " << flags << endl;
      cerr << "bytes: " << bytes << endl;
      cerr << "tiles: " << tiles << endl;
      cerr << "compress: " << compress << endl;
      cerr << "orgx: " << orgx << endl;
      cerr << "orgy: " << orgy << endl;
#endif
    }
  };

  struct iffChunk
  {
    unsigned int tag;
    unsigned int size;

    void swap()
    {
      tag   = ntohl( tag );
      size  = ntohl( size );
    }
  };


  struct iffPixelBlock
  {
    unsigned short x1, y1, x2, y2;
    void swap()
    {
      x1 = ntohs( x1 );
      x2 = ntohs( x2 );
      y1 = ntohs( y1 );
      y2 = ntohs( y2 );
    }
  };



  using namespace std;


  iffImage::iffImage() :
    CMedia()
  {
  }

  iffImage::~iffImage()
  {
  }


  /*! Test a block of data read from the start of the file to see if it
    looks like the start of an .iff file. This returns true if the 
    data contains IFF's magic number (0x10) and a short of 1 in the
    as the number of components.
  */
  bool iffImage::test(const boost::uint8_t *data, unsigned len)
  {
    iffChunk* header = (iffChunk*) data;
    if ( ntohl( header->tag ) != kIFF_MAGIC )
      return false;

    return true;
  }





  void iffImage::end_read_chunk( FILE* f, iffChunk& chunk )
  {
    int size = chunk.size % 4;
    if ( size == 0 ) return;
    fseek( f, 4-size, SEEK_CUR );
  }

  void iffImage::read_chunk( FILE* f, iffChunk& chunk )
  {
    size_t r = fread( &chunk, sizeof(iffChunk), 1, f );
    if ( r == 0 ) return;
    chunk.swap();
  }

  void iffImage::read_uncompressed_tile( FILE* file, boost::uint8_t* src, 
					 const unsigned int compsize,
					 const unsigned x1, const unsigned y1,
					 const unsigned w, 
					 const unsigned h, 
					 const short depth, const short bytes, 
					 const bool z ) 
  {
    size_t r = fread( src, compsize, 1, file ); 

    unsigned int dw = width();
    unsigned int dh = height() - 1;

    Pixel* pixels = (Pixel*)_hires->data().get();

    for ( unsigned y = 0; y < h; ++y )
      {
	unsigned offset = (dh - (y + y1)) * dw + x1;
	Pixel* p = pixels + offset;

	for ( unsigned x = 0; x < w; ++x, ++p )
	  {
	    for ( short c = 0; c < depth; ++c, ++src )
	      {
		boost::uint8_t t[4];
		boost::uint8_t* d = src;
		for ( short b = 0; b < bytes; ++b )
		  {
		    t[bytes-b-1] = *d;
		    d += w * h * depth;
		  }
	      
		float v = 0.f;
		switch( bytes )
		  {
		  case 1:
		    v = (*src) / 255.0f;
		    break;
		  case 2:
		    {
		      unsigned short val = ( (t[1] << 8) +
					     (t[0] << 0) );
		      v = (float)val / 65535.0f;
		      break;
		    }
		  case 4:
		    {
		      v = *((float*)t);
		      break;
		    }
		  }
	      
		float* rgba = (float*) p;
		rgba[3-c] = v;
	      }
	  }
      }
  }



  void iffImage::decompress_rle_tile( boost::uint8_t* data,
				      const boost::uint8_t* comp, 
				      const unsigned compsize, 
				      const unsigned width )
  {
    unsigned i, x = 0;
    const boost::uint8_t* last = comp + compsize;
    while ( x < width && comp < last )
      {
	unsigned char type = *comp++;
	unsigned char count = (type & 0x7f) + 1;

	if ( type & 0x80 )
	  {
	    unsigned char c = *comp++;
	    // we check x < width as sometimes count overruns by 1
	    for ( i = 0; x < width && i < count; ++i )
	      {
		data[x++] = c;
	      }
	  }
	else
	  {
	    // we check x < width as sometimes count overruns by 1
	    for ( i = 0; x < width && i < count; ++i )
	      {
		data[x++] = *comp++;
	      }
	  }
      }
  }


  void iffImage::store_tile( boost::uint8_t* data, 
			     const unsigned x1, const unsigned y1,
			     const unsigned w, const unsigned h,
			     const short depth, const short bytes, bool z )
  {
    unsigned int dw = width();
    unsigned int dh = height() - 1;


    Pixel* pixels = (Pixel*)_hires->data().get();
    boost::uint8_t* src = data;
    if ( z )
      {
	for ( unsigned y = 0; y < h; ++y )
	  {
	    unsigned offset = (dh - (y + y1)) * dw + x1;
	    Pixel* p = pixels + offset;
	    unsigned int s = y * w * bytes;

	    for ( unsigned x = 0; x < w; s += bytes, ++x, ++p )
	      {
		boost::uint8_t t[4];
		boost::uint8_t* d = src;
		for ( short b = 0; b < bytes; ++b )
		  {
		    t[b] = *d;
		    d += w * h;
		  }

		if ( t[0] == 0 )
		  {
		    p->b = 0.5f;
		    p->r = p->g = p->a = std::numeric_limits<float>::quiet_NaN();
		  }
		else
		  {
		    unsigned int v = ( (t[0] << 24) +
				       (t[1] << 16) +
				       (t[2] << 8) +
				       (t[3] << 0) );
		    p->r = p->g = p->b = float(v);
		    p->a = 0.0f;
		  }
	      }
	  }
      }
    else
      {
	for ( short c = 0; c < depth; ++c )
	  {
	    for ( unsigned y = 0; y < h; ++y )
	      {
		unsigned offset = (dh - (y + y1)) * dw + x1;
		Pixel* p = pixels + offset;
	      
		for ( unsigned x = 0; x < w; ++x, ++src, ++p )
		  {
		    boost::uint8_t t[4];
		    boost::uint8_t* d = src;
		    for ( short b = 0; b < bytes; ++b )
		      {
			t[bytes-b-1] = *d;
			d += w * h * depth;
		      }

		    float v = 0.f;
		    switch( bytes )
		      {
		      case 1:
			v = (*src) / 255.0f;
			break;
		      case 2:
			{
			  unsigned short val = ( (t[1] << 8) +
						 (t[0] << 0) );
			  v = (float)val / 65535.0f;
			  break;
			}
		      case 4:
			{
			  v = *((float*)t);
			  break;
			}
		      }

		    float* rgba = (float*) p;
		    rgba[3-c] = v;
		  }
	      }
	  }
      }

  }


  void iffImage::read_pixel_chunk( FILE* f, 
				   const int depth, const int bytes, 
				   const iffChunk& chunk )
  {		  
    iffPixelBlock block;
    size_t r = fread( &block, sizeof(iffPixelBlock), 1, f );
    block.swap();
    unsigned dw = (block.x2 - block.x1)+1;
    unsigned dh = (block.y2 - block.y1)+1;
    unsigned compsize = chunk.size - sizeof(iffPixelBlock);

    if ( dw < width() && dh < height() && 
	 block.x2 > block.x1 && block.y2 > block.y1 )
    { 
      unsigned size = dw * dh * depth * bytes + 8; // +8 padding needed

      boost::uint8_t* data = new boost::uint8_t[ size ];
      memset( data, 0, size );

      bool compressed = true;
      if ( size == chunk.size )
	compressed = false;

      if ( compressed )
	{
	  _compression = kRLECompression;
	  boost::uint8_t* comp = new boost::uint8_t[ compsize ];
	  size_t r = fread( comp, compsize, 1, f );
	  decompress_rle_tile( data, comp, compsize, size );
	  delete [] comp;
	  store_tile( data, block.x1, block.y1, dw, dh, depth, bytes,
		      (chunk.tag == kZBUF_TAG) );
	}
      else
	{
	  read_uncompressed_tile( f, data, compsize,
				  block.x1, block.y1, 
				  dw, dh, depth, bytes,
				  (chunk.tag == kZBUF_TAG) );
	}

      delete [] data;
    }
    else
      {
	LOG_ERROR( _("Problem with tile") << " [" 
		   << block.x1 << "," << block.y1 << "]-[" 
		   << block.x2 << "," << block.y2 << "]");
	fseek( f, compsize, SEEK_CUR );
      }
  }

  bool iffImage::fetch(const boost::int64_t frame) 
  {
    _gamma = 1.0f;
    _num_channels = 0;
    _compression = kNoCompression;

    SCOPED_LOCK( _mutex );

    FILE* f = fltk::fltk_fopen( sequence_filename(frame).c_str(), "rb" );
    if (!f) return false;

    iffHeader header;
    bool found = false;
    unsigned tile = 0;
    char buf[1024];
    while( !feof(f) )
      {
	iffChunk chunk;
	read_chunk( f, chunk );
	if ( chunk.tag == kTBHD_TAG )
	  {
	    found = true;
	    size_t r = fread( &header, chunk.size, 1, f );
	    header.swap();

	    // _depth = 8 * (header.bytes+1);
	    _layers.clear();

	    image_size( header.width, header.height );
	    allocate_pixels( frame );

	    if ( header.flags & kHasRGB )
	      {
		rgb_layers();
		lumma_layers();
	      }
	    if ( header.flags & kHasAlpha )
	      {
		alpha_layers();
	      }
	    if ( header.flags & kHas12Bit )
	      {
// 		_depth = 12;
	      }
	    if ( header.flags & kHasZ )
	      {
		_layers.push_back( _("Z Depth") );
		++_num_channels;
	      }
	  }
	else if ( chunk.tag == kAUTH_TAG )
	  {
	    size_t r = fread( buf, chunk.size, 1, f );
	    buf[chunk.size] = 0;
	    end_read_chunk( f, chunk );
	  }
	else if ( chunk.tag == kDATE_TAG )
	  {
	    size_t r = fread( buf, chunk.size, 1, f );
	    buf[chunk.size] = 0;
	    end_read_chunk( f, chunk );
	  }
	else if ( chunk.tag == kFOR4_TAG )
	  {
	    unsigned int type;
	    size_t r = fread( &type, 1, sizeof(unsigned int), f );
	    type = ntohl( type );
	    if ( type != kTBMP_TAG ) {
	      end_read_chunk( f, chunk );
	      continue;
	    }

	    while ( !feof(f) && (tile < header.tiles) )
	      { 
		iffChunk chunk;
		read_chunk( f, chunk );
		if ( chunk.tag == kRGBA_TAG && _channel == NULL )
		  {
		    ++tile;
		    read_pixel_chunk( f, header.depth, header.bytes+1, chunk );
		  }
		else if ( chunk.tag == kZBUF_TAG && _channel && 
			  strcmp( _channel, _("Z Depth") ) == 0 )
		  {
		    ++tile;
		    read_pixel_chunk( f, 4, 1, chunk );
		  }
		else
		  {
		    // Skip unknown data or not being shown
		    fseek( f, chunk.size, SEEK_CUR );
		  }
		end_read_chunk( f, chunk );
	      } // while

	    break; // finished reading all image data
	  }
	else
	  {
	    // We don't recognize this chunk.  Skip it.
	    int size = chunk.size % 4;
	    if ( size != 0 ) chunk.size += 4 - size;
	    fseek( f, chunk.size, SEEK_CUR );
	  }
      }

    fclose(f);

    if (!found) return false;

    refresh();
    return true;
  }


} // namespace mrv
