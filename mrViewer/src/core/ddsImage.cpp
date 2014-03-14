
/**
 * @file   ddsImage.cpp
 * @author gga
 * @date   Sun Mar 11 04:56:19 2007
 * 
 * @brief  
 * 
 * 
 */

#include "ddsImage.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <fltk/run.h>

#include <half.h>
#include <halfFunction.h>

#include "mrvIO.h"

namespace {

static const char* kDDS_MAGIC_NUMBER = "DDS ";

}

#define DDPF_ALPHAPIXELS                        0x00000001l
#define DDPF_ALPHA                              0x00000002l
#define DDPF_FOURCC                             0x00000004l
#define DDPF_PALETTEINDEXED4                    0x00000008l
#define DDPF_PALETTEINDEXEDTO8                  0x00000010l
#define DDPF_PALETTEINDEXED8                    0x00000020l
#define DDPF_RGB                                0x00000040l
#define DDPF_COMPRESSED                         0x00000080l
#define DDPF_RGBTOYUV                           0x00000100l
#define DDPF_YUV                                0x00000200l
#define DDPF_ZBUFFER                            0x00000400l
#define DDPF_PALETTEINDEXED1                    0x00000800l
#define DDPF_PALETTEINDEXED2                    0x00001000l
#define DDPF_ZPIXELS                            0x00002000l
#define DDPF_STENCILBUFFER                      0x00004000l
#define DDPF_ALPHAPREMULT                       0x00008000l
#define DDPF_LUMINANCE                          0x00020000l
#define DDPF_BUMPLUMINANCE                      0x00040000l
#define DDPF_BUMPDUDV                           0x00080000l

struct DDPFPIXELFORMAT
{
  unsigned long dwSize;
  unsigned long dwFlags;
  unsigned long dwFourCC;
  unsigned long dwRGBBitCount;
  unsigned long dwRBitMask;
  unsigned long dwGBitMask;
  unsigned long dwBBitMask;
  unsigned long dwABitMask;
};

#define DDSCAPS_COMPLEX                         0x00000008l
#define DDSCAPS_ALPHA                           0x00000002l
#define DDSCAPS_TEXTURE                         0x00001000l
#define DDSCAPS_MIPMAP                          0x00400000l

#define DDSCAPS2_CUBEMAP                        0x00000200L
#define DDSCAPS2_CUBEMAP_POSITIVEX              0x00000400L
#define DDSCAPS2_CUBEMAP_NEGATIVEX              0x00000800L
#define DDSCAPS2_CUBEMAP_POSITIVEY              0x00001000L
#define DDSCAPS2_CUBEMAP_NEGATIVEY              0x00002000L
#define DDSCAPS2_CUBEMAP_POSITIVEZ              0x00004000L
#define DDSCAPS2_CUBEMAP_NEGATIVEZ              0x00008000L
#define DDSCAPS2_VOLUME                         0x00200000L

typedef struct _DDSCAPS2
{
  unsigned long dwCaps1;
  unsigned long dwCaps2;
  unsigned long dwCaps3;
  unsigned long dwCaps4;
} DDSCAPS2;

#define DDSD_CAPS               0x00000001l
#define DDSD_HEIGHT             0x00000002l
#define DDSD_WIDTH              0x00000004l
#define DDSD_PITCH              0x00000008l
#define DDSD_PIXELFORMAT        0x00001000l
#define DDSD_MIPMAPCOUNT        0x00020000l
#define DDSD_LINEARSIZE         0x00080000l
#define DDSD_DEPTH              0x00800000l

struct DDSURFACEDESC2
{
  unsigned long dwSize;
  unsigned long dwFlags;
  unsigned long dwHeight;
  unsigned long dwWidth;
  unsigned long dwPitchOrLinearSize;
  unsigned long dwDepth;
  unsigned long dwMipMapCount;
  unsigned long dwAlphaBitDepth;
  unsigned long dwReserved[ 5 ];
  DDPFPIXELFORMAT ddpfPixelFormat;
  DDSCAPS2 ddsCaps;
  unsigned long dwTextureStage;
};


/* use cast to struct instead of RGBA_MAKE as struct is much */
struct Color8888
{
  unsigned char r;		/* change the order of names to change the */
  unsigned char g;		/*  order of the output ARGB or BGRA, etc... */
  unsigned char b;		/*  Last one is MSB, 1st is LSB. */
  unsigned char a;
};


struct Color888
{
  unsigned char r;		/* change the order of names to change the */
  unsigned char g;		/*  order of the output ARGB or BGRA, etc... */
  unsigned char b;		/*  Last one is MSB, 1st is LSB. */
};

struct Color565
{
  unsigned int nBlue  : 5;		/* order of names changes */
  unsigned int nGreen : 6;		/*  byte order of output to 32 bit */
  unsigned int nRed   : 5;
};

#define MAKE_FOURCC(a,b,c,d) ((((unsigned int)d) << 24) +	\
			      (((unsigned int)c) << 16) +	\
			      (((unsigned int)b) << 8)  + a)

static const unsigned int DDS_DXT1 = MAKE_FOURCC('D', 'X', 'T', '1');
static const unsigned int DDS_DXT2 = MAKE_FOURCC('D', 'X', 'T', '2');
static const unsigned int DDS_DXT3 = MAKE_FOURCC('D', 'X', 'T', '3');
static const unsigned int DDS_DXT4 = MAKE_FOURCC('D', 'X', 'T', '4');
static const unsigned int DDS_DXT5 = MAKE_FOURCC('D', 'X', 'T', '5');

static const unsigned int DDS_ATI1 = MAKE_FOURCC('A', 'T', 'I', '1');
static const unsigned int DDS_ATI2 = MAKE_FOURCC('A', 'T', 'I', '2');
static const unsigned int DDS_RXGB = MAKE_FOURCC('R', 'X', 'G', 'B');
static const unsigned int DDS_DOLL = MAKE_FOURCC('$', 0, 0, 0);
static const unsigned int DDS_o000 = MAKE_FOURCC('o', 0, 0, 0);
static const unsigned int DDS_p000 = MAKE_FOURCC('p', 0, 0, 0);
static const unsigned int DDS_q000 = MAKE_FOURCC('q', 0, 0, 0);
static const unsigned int DDS_r000 = MAKE_FOURCC('r', 0, 0, 0);
static const unsigned int DDS_s000 = MAKE_FOURCC('s', 0, 0, 0);
static const unsigned int DDS_t000 = MAKE_FOURCC('t', 0, 0, 0);

enum PixFormat
  {
    PF_ARGB,
    PF_RGB,
    PF_DXT1,
    PF_DXT2,
    PF_DXT3,
    PF_DXT4,
    PF_DXT5,
    PF_3DC,
    PF_ATI1N,
    PF_LUMINANCE,
    PF_LUMINANCE_ALPHA,
    PF_RXGB, /* Doom3 normal maps */
    PF_A16B16G16R16,
    PF_R16F,
    PF_G16R16F,
    PF_A16B16G16R16F,
    PF_R32F,
    PF_G32R32F,
    PF_A32B32G32R32F,
    PF_UNKNOWN = 0xFF
  };




namespace mrv {


  void ddsImage::ReadColors(const unsigned char* Data, Color8888* Out)
  {
    unsigned char r0, g0, b0, r1, g1, b1;

    b0 = Data[0] & 0x1F;
    g0 = ((Data[0] & 0xE0) >> 5) | ((Data[1] & 0x7) << 3);
    r0 = (Data[1] & 0xF8) >> 3;

    b1 = Data[2] & 0x1F;
    g1 = ((Data[2] & 0xE0) >> 5) | ((Data[3] & 0x7) << 3);
    r1 = (Data[3] & 0xF8) >> 3;

    Out[0].r = r0 << 3;
    Out[0].g = g0 << 2;
    Out[0].b = b0 << 3;

    Out[1].r = r1 << 3;
    Out[1].g = g1 << 2;
    Out[1].b = b1 << 3;
  }

  void ddsImage::ReadColor(unsigned short Data, Color8888* Out)
  {
    unsigned char r, g, b;

    b = Data & 0x1f;
    g = (Data & 0x7E0) >> 5;
    r = (Data & 0xF800) >> 11;
    Out->r = r << 3;
    Out->g = g << 2;
    Out->b = b << 3;
  }

  void ddsImage::DecompressDXT1(unsigned char* src )
  {
    unsigned int x, y; 
    int i, j, k, Select;
    unsigned char* Temp = src;
    Color8888	colours[4], *col;
    unsigned short	color_0, color_1;
    unsigned int	bitmask;

    colours[0].a = 0xFF;
    colours[1].a = 0xFF;
    colours[2].a = 0xFF;

    Pixel* pixels = (Pixel*)_hires->data().get();

    unsigned int dh = height();
    unsigned int dw = width();
    for (y = 0; y < dh; y += 4) {
      for (x = 0; x < dw; x += 4) {
	color_0 = *((unsigned short*)Temp);
	color_1 = *((unsigned short*)(Temp + 2));

#ifdef MRV_BIG_ENDIAN
	MSBOrderShort((unsigned char*)&color_0, sizeof(short));
	MSBOrderShort((unsigned char*)&color_1, sizeof(short));
#endif

	ReadColor(color_0, colours);
	ReadColor(color_1, colours + 1);
	bitmask = ((unsigned int*)Temp)[1];

#ifdef MRV_BIG_ENDIAN
	MSBOrderLong((unsigned char*)&bitmask, sizeof(long));
#endif

	Temp += 8;

	if (color_0 > color_1) {
	  /* Four-color block: derive the other two colors.
	     00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
	     These 2-bit codes correspond to the 2-bit fields 
	     stored in the 64-bit block. */
	  colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
	  colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
	  colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;

	  colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
	  colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
	  colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
	  colours[3].a = 0xFF;
	}
	else { 
	  /* Three-color block: derive the other color.
	     00 = color_0,  01 = color_1,  10 = color_2,
	     11 = transparent.
	     These 2-bit codes correspond to the 2-bit fields 
	     stored in the 64-bit block. */
	  colours[2].b = (colours[0].b + colours[1].b) / 2;
	  colours[2].g = (colours[0].g + colours[1].g) / 2;
	  colours[2].r = (colours[0].r + colours[1].r) / 2;

	  colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
	  colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
	  colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
	  colours[3].a = 0x00;
	}

	for (j = 0, k = 0; j < 4; j++) {
	  for (i = 0; i < 4; i++, k++) {

	    Select = (bitmask & (0x03 << k*2)) >> k*2;
	    col = &colours[Select];

	    if (((x + i) < dw) && ((y + j) < dh)) {
	      CMedia::Pixel& p = pixels[ x+i + (y+j) * dw ];
	      p.r = (float) col->r / 255.0f;
	      p.g = (float) col->g / 255.0f;
	      p.b = (float) col->b / 255.0f;
	      p.a = (float) col->a / 255.0f;
	    }
	  }
	}
      }
    }
  }

  void ddsImage::CorrectPreMult()
  {
    unsigned int x,y;
    unsigned int dh = height();
    unsigned int dw = width();
    Pixel* pixels = (Pixel*)_hires->data().get();
    for (y = 0; y < dh; ++y) {
      for (x = 0; x < dw; ++x) {
	CMedia::Pixel& p = pixels[ x + y * dw ];
	if ( p.a == 0.0f ) continue;

	p.r /= p.a;
	p.g /= p.a;
	p.b /= p.a;
      }
    }
  }

  void ddsImage::DecompressDXT2(unsigned char* src )
  {
    DecompressDXT1( src );
    CorrectPreMult();
  }

  void ddsImage::DecompressDXT3( unsigned char* src )
  {
    unsigned int x, y;
    int i, j, k, Select;
    unsigned char* Temp = src;

    Color8888	 colours[4], *col;
    unsigned int	 bitmask;
    unsigned short word;
    unsigned char* alpha;

    Pixel* pixels = (Pixel*)_hires->data().get();

    unsigned int dw = width();
    unsigned int dh = height();
    for (y = 0; y < dh; y += 4) {
      for (x = 0; x < dw; x += 4) {

	/* Skip 64-bits of alpha data */
	alpha = Temp;
	Temp += 8;

	/* Read 64-bits of color data */
	ReadColors(Temp, colours);
	bitmask = ((unsigned int*)Temp)[1];

#ifdef MRV_BIG_ENDIAN
	MSBOrderLong((unsigned char*)&bitmask, 4);
#endif

	Temp += 8;
	/* Four-color block: derive the other two colors.
	   00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
	   These 2-bit codes correspond to the 2-bit fields
	   stored in the 64-bit block. */
	colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
	colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
	colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;

	colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
	colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
	colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;

	k = 0;

	for (j = 0; j < 4; j++) {
	  word = alpha[2*j] + 256*alpha[2*j+1];

	  for (i = 0; i < 4; i++, k++) {
	    Select = (bitmask & (0x03 << k*2)) >> k*2;
	    col = &colours[Select];
	    if (((x + i) < dw) && ((y + j) < dh)) 
	      {
		unsigned char pxl[4];
		pxl[0] = col->r;
		pxl[1] = col->g;
		pxl[2] = col->b;
		pxl[3] = word & 0x0F;
		pxl[3] = pxl[3] | (pxl[3] << 4);

		CMedia::Pixel& p = pixels[ x+i + (y+j)*dw ];
		p.r = (float) pxl[0] / 255.0f;
		p.g = (float) pxl[1] / 255.0f;
		p.b = (float) pxl[2] / 255.0f;
		p.a = (float) pxl[3] / 255.0f;
	      }
	  }

	  word >>= 4;
	}

      }
    }
  }


  void ddsImage::DecompressDXT4(unsigned char* src )
  {
    DecompressDXT3( src );
    CorrectPreMult();
  }

  void ddsImage::DecompressDXT5( unsigned char* src )
  {
    unsigned int x, y; 
    int i, j, k, Select;
    unsigned char* Temp = src;
    Color8888	colours[4], *col;
    unsigned int	bitmask;
    unsigned char	alphas[8], *alphamask;
    unsigned int	bits;

    Pixel* pixels = (Pixel*)_hires->data().get();

    unsigned int dw = width();
    unsigned int dh = height();
    for (y = 0; y < dh; y += 4) {
      for (x = 0; x < dw; x += 4) {

	alphas[0] = Temp[0];
	alphas[1] = Temp[1];
	alphamask = Temp + 2;
	Temp += 8;

	ReadColors(Temp, colours);

	bitmask = ((unsigned int*)Temp)[1];

#ifdef MRV_BIG_ENDIAN
	MSBOrderLong((unsigned char*)&bitmask, 4);
#endif

	Temp += 8;

	/* Four-color block: derive the other two colors.    
	   00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
	   These 2-bit codes correspond to the 2-bit fields 
	   stored in the 64-bit block. */
	colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
	colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
	colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;

	colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
	colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
	colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;

	k = 0;
	for (j = 0; j < 4; j++) {
	  for (i = 0; i < 4; i++, k++) {

	    Select = (bitmask & (0x03 << k*2)) >> k*2;
	    col = &colours[Select];

	    /* only put pixels out < width or height */
	    if (((x + i) < dw) && ((y + j) < dh)) {
	      CMedia::Pixel& p = pixels[ x+i + (y+j)*dw ];
	      p.r = ((float)col->r) / 255.0f;
	      p.g = ((float)col->g) / 255.0f;
	      p.b = ((float)col->b) / 255.0f;
	    }
	  }
	}

	/* 8-alpha or 6-alpha block? */
	if (alphas[0] > alphas[1]) {
	  /* 8-alpha block:  derive the other six alphas.    
	     Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated. */
	  alphas[2] = (6 * alphas[0] + 1 * alphas[1] + 3) / 7; /* bit code 010 */
	  alphas[3] = (5 * alphas[0] + 2 * alphas[1] + 3) / 7; /* bit code 011 */
	  alphas[4] = (4 * alphas[0] + 3 * alphas[1] + 3) / 7; /* bit code 100 */
	  alphas[5] = (3 * alphas[0] + 4 * alphas[1] + 3) / 7; /* bit code 101 */
	  alphas[6] = (2 * alphas[0] + 5 * alphas[1] + 3) / 7; /* bit code 110 */
	  alphas[7] = (1 * alphas[0] + 6 * alphas[1] + 3) / 7; /* bit code 111 */
	}
	else {
	  /* 6-alpha block.
	     Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated. */
	  alphas[2] = (4 * alphas[0] + 1 * alphas[1] + 2) / 5; /* bit code 010 */
	  alphas[3] = (3 * alphas[0] + 2 * alphas[1] + 2) / 5; /* bit code 011 */
	  alphas[4] = (2 * alphas[0] + 3 * alphas[1] + 2) / 5; /* bit code 100 */
	  alphas[5] = (1 * alphas[0] + 4 * alphas[1] + 2) / 5; /* bit code 101 */
	  alphas[6] = 0x00;	 /* bit code 110 */
	  alphas[7] = 0xFF;	 /* bit code 111 */
	}

	/* Note: Have to separate the next two loops,
	   it operates on a 6-byte system. */

	/* First three bytes */
	bits = (alphamask[0]) | (alphamask[1] << 8) | (alphamask[2] << 16);
	for (j = 0; j < 2; j++) {
	  for (i = 0; i < 4; i++) {
	    /* only put pixels out < width or height */
	    if (((x + i) < dw) && ((y + j) < dh)) {
	      CMedia::Pixel& p = pixels[ x+i + (y+j)*dw ];
	      p.a = (float) ( alphas[bits & 0x07] ) / 255.0f;
	    }
	    bits >>= 3;
	  }
	}

	/* Last three bytes */
	bits = (alphamask[3]) | (alphamask[4] << 8) | (alphamask[5] << 16);
	for (j = 2; j < 4; j++) {
	  for (i = 0; i < 4; i++) {
	    /* only put pixels out < width or height */
	    if (((x + i) < dw) && ((y + j) < dh)) {
	      CMedia::Pixel& p = pixels[ x+i + (y+j)*dw ];
	      p.a = (float) ( alphas[bits & 0x07] ) / 255.0f;
	    }
	    bits >>= 3;
	  }
	}
      }
    }
  }

  void ddsImage::GetBitsFromMask(unsigned int Mask, 
				 unsigned int* ShiftLeft, 
				 unsigned int* ShiftRight)
  {
    unsigned int Temp, i;

    if (Mask == 0) {
      *ShiftLeft = *ShiftRight = 0;
      return;
    }

    Temp = Mask;
    for (i = 0; i < 32; i++, Temp >>= 1) {
      if (Temp & 1)
	break;
    }
    *ShiftRight = i;

    /* Temp is preserved, so use it again: */
    for (i = 0; i < 8; i++, Temp >>= 1) {
      if (!(Temp & 1))
	break;
    }
    *ShiftLeft = 8 - i;

    return;
  }

  void ddsImage::DecompressARGB( unsigned char* src, DDPFPIXELFORMAT* Head ) 
  {
    unsigned int ReadI = 0, TempBpp, x, y;
    unsigned int RedL, RedR;
    unsigned int GreenL, GreenR;
    unsigned int BlueL, BlueR;
    unsigned int AlphaL, AlphaR;
    unsigned char* Temp = src;


    GetBitsFromMask(Head->dwRBitMask, &RedL, &RedR);
    GetBitsFromMask(Head->dwGBitMask, &GreenL, &GreenR);
    GetBitsFromMask(Head->dwBBitMask, &BlueL, &BlueR);
    GetBitsFromMask(Head->dwABitMask, &AlphaL, &AlphaR);
    TempBpp = Head->dwRGBBitCount / 8;

    Pixel* pixels = (Pixel*)_hires->data().get();

    unsigned int dw = width();
    unsigned int dh = height();
    for (y = 0; y < dh; ++y) 
      {
	for (x = 0; x < dw; ++x) 
	  {
	    ReadI = Temp[0] | (Temp[1] << 8) | (Temp[2] << 16) | (Temp[3] << 24);
	    Temp += TempBpp;

	    CMedia::Pixel& p = pixels[ x + y*dw ];
	    p.r = (float) (((ReadI & Head->dwRBitMask) >> 
			    RedR) << RedL) / 255.0f;
	  
	    if( Head->dwGBitMask ) {
	      p.g = (float) (((ReadI & Head->dwGBitMask) >> 
			      GreenR) << GreenL) / 255.0f;
	    }

	    if( Head->dwBBitMask ) {
	      p.g  = (float) (((ReadI & Head->dwBBitMask) >> 
			       BlueR) << BlueL) / 255.0f;
	    }
	  
	    if ( Head->dwABitMask ) {
	      unsigned char alpha = 0;
	      alpha = (unsigned char) 
		((ReadI & Head->dwABitMask) >> AlphaR) << AlphaL;

	      if (AlphaL >= 7) {
		alpha = alpha ? 0xFF : 0x00;
	      }
	      else if (AlphaL >= 4) {
		alpha = alpha | (alpha >> 4);
	      }
	      p.a = (float) alpha / 255.0f;
	    }
	  }
      }
  }


  void ddsImage::DecompressAti1n( unsigned char* src )
  {
    unsigned int x, y;
    int			i, j, k, t1, t2;
    unsigned char		Colours[8];
    unsigned int		bitmask;
    unsigned char* Temp = src;

    Pixel* pixels = (Pixel*)_hires->data().get();

    unsigned int dw = width();
    unsigned int dh = height();
    for (y = 0; y < dh; y += 4) {
      for (x = 0; x < dw; x += 4) {
	/* Read palette */
	t1 = Colours[0] = Temp[0];
	t2 = Colours[1] = Temp[1];
	Temp += 2;
	if (t1 > t2)
	  for (i = 2; i < 8; ++i)
	    Colours[i] = t1 + ((t2 - t1)*(i - 1))/7;
	else {
	  for (i = 2; i < 6; ++i)
	    Colours[i] = t1 + ((t2 - t1)*(i - 1))/5;
	  Colours[6] = 0;
	  Colours[7] = 255;
	}

	/* decompress pixel data */
	for (k = 0; k < 4; k += 2) {
	  /* First three bytes */
	  bitmask = ( ((unsigned int)(Temp[0]) << 0) | 
		      ((unsigned int)(Temp[1]) << 8) | 
		      ((unsigned int)(Temp[2]) << 16) );
	  for (j = 0; j < 2; j++) {
	    /* only put pixels out < height */
	    for (i = 0; i < 4; i++) {
	      /* only put pixels out < width */
	      CMedia::Pixel& p = pixels[ x+i + (y+k+j)*dw ];
	      p.r = p.g = p.b = (float) ( Colours[bitmask & 0x07] ) / 255.0f;
	      bitmask >>= 3;
	    }
	  }
	  Temp += 3;
	}
      }
    }
  }


  void ddsImage::Decompress3Dc( unsigned char* src )
  {
    unsigned int x, y;
    int	i, j, k, t1, t2;
    unsigned char* Temp = src, *Temp2;
    unsigned char		XColours[8], YColours[8];
    unsigned int		bitmask, bitmask2;

    Pixel* pixels = (Pixel*)_hires->data().get();
    unsigned int dw = width();
    unsigned int dh = height();
    for (y = 0; y < dh; y += 4) {
      for (x = 0; x < dw; x += 4) {
	Temp2 = Temp + 8;

	/* Read Y palette */
	t1 = YColours[0] = Temp[0];
	t2 = YColours[1] = Temp[1];
	Temp += 2;
	if (t1 > t2)
	  for (i = 2; i < 8; ++i)
	    YColours[i] = t1 + ((t2 - t1)*(i - 1))/7;
	else {
	  for (i = 2; i < 6; ++i)
	    YColours[i] = t1 + ((t2 - t1)*(i - 1))/5;
	  YColours[6] = 0;
	  YColours[7] = 255;
	}

	/* Read X palette */
	t1 = XColours[0] = Temp2[0];
	t2 = XColours[1] = Temp2[1];
	Temp2 += 2;
	if (t1 > t2)
	  for (i = 2; i < 8; ++i)
	    XColours[i] = t1 + ((t2 - t1)*(i - 1))/7;
	else {
	  for (i = 2; i < 6; ++i)
	    XColours[i] = t1 + ((t2 - t1)*(i - 1))/5;
	  XColours[6] = 0;
	  XColours[7] = 255;
	}

	/* decompress pixel data */
	for (k = 0; k < 4; k += 2) {
	  /* First three bytes */
	  bitmask = ( ((unsigned int)(Temp[0]) << 0) | 
		      ((unsigned int)(Temp[1]) << 8) | 
		      ((unsigned int)(Temp[2]) << 16) );
	  bitmask2 = ( ((unsigned int)(Temp2[0]) << 0) | 
		       ((unsigned int)(Temp2[1]) << 8) | 
		       ((unsigned int)(Temp2[2]) << 16) );
	  for (j = 0; j < 2; j++) {
	    /* only put pixels out < height */
	    if ((y + k + j) < dh) {
	      for (i = 0; i < 4; i++) {
		/* only put pixels out < width */
		if ((x + i) < dw) {
		  int t, tx, ty;
		  CMedia::Pixel& p = pixels[ x+i + (y+k+j)*dw ];

		  p.g = (float) ( ty = YColours[bitmask  & 0x07] ) / 255.0f;
		  p.r   = (float) ( tx = YColours[bitmask2 & 0x07] ) / 255.0f;

		  /* calculate b (z) component 
		     ((r/255)^2 + (g/255)^2 + (b/255)^2 = 1 */
		  t = 127*128 - (tx - 127)*(tx - 128) - (ty - 127)*(ty - 128);
		  if (t > 0)
		    p.b = (float) ( sqrt((double)t) + 128 ) / 255.0f;
		  else
		    p.b = (float) 0.5f;
		}
		bitmask >>= 3;
		bitmask2 >>= 3;
	      }
	    }
	  }
	  Temp += 3;
	  Temp2 += 3;
	}

	/* skip bytes that were read via Temp2 */
	Temp += 8;
      }
    }
  }

  void ddsImage::DecompressRXGB( unsigned char* src )
  {
    unsigned int x, y;
    int i, j, k, Select;
    Color565* color_0, *color_1;
    Color8888	colours[4], *col;
    unsigned int  bitmask;
    unsigned char alphas[8], *alphamask;
    unsigned int  bits;
    unsigned char* Temp = src;

    Pixel* pixels = (Pixel*)_hires->data().get();
    unsigned int dw = width();
    unsigned int dh = height();
    for (y = 0; y < dh; y += 4) {
      for (x = 0; x < dw; x += 4) {

	alphas[0] = Temp[0];
	alphas[1] = Temp[1];
	alphamask = Temp + 2;
	Temp += 8;
	color_0 = ((Color565*)Temp);
	color_1 = ((Color565*)(Temp+2));
	bitmask = ((unsigned int*)Temp)[1];
	Temp += 8;

	colours[0].r = color_0->nRed << 3;
	colours[0].g = color_0->nGreen << 2;
	colours[0].b = color_0->nBlue << 3;
	colours[0].a = 0xFF;

	colours[1].r = color_1->nRed << 3;
	colours[1].g = color_1->nGreen << 2;
	colours[1].b = color_1->nBlue << 3;
	colours[1].a = 0xFF;

	/* Four-color block: derive the other two colors.    
	   00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
	   These 2-bit codes correspond to the 2-bit fields 
	   stored in the 64-bit block. */
	colours[2].b = (2 * colours[0].b + colours[1].b + 1) / 3;
	colours[2].g = (2 * colours[0].g + colours[1].g + 1) / 3;
	colours[2].r = (2 * colours[0].r + colours[1].r + 1) / 3;
	colours[2].a = 0xFF;

	colours[3].b = (colours[0].b + 2 * colours[1].b + 1) / 3;
	colours[3].g = (colours[0].g + 2 * colours[1].g + 1) / 3;
	colours[3].r = (colours[0].r + 2 * colours[1].r + 1) / 3;
	colours[3].a = 0xFF;

	k = 0;
	for (j = 0; j < 4; j++) {
	  for (i = 0; i < 4; i++, k++) {

	    Select = (bitmask & (0x03 << k*2)) >> k*2;
	    col = &colours[Select];

	    CMedia::Pixel& p = pixels[ x+i + (y+j)*dw ];
	    p.r = (float) ( col->r ) / 255.0f;
	    p.g = (float) ( col->g ) / 255.0f;
	    p.b = (float) ( col->b ) / 255.0f;
	  }
	}

	/* 8-alpha or 6-alpha block? */    
	if (alphas[0] > alphas[1]) {    
	  /* 8-alpha block:  derive the other six alphas.    
	     Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated. */
	  alphas[2] = (6 * alphas[0] + 1 * alphas[1] + 3) / 7;	/* bit code 010 */
	  alphas[3] = (5 * alphas[0] + 2 * alphas[1] + 3) / 7;	/* bit code 011 */
	  alphas[4] = (4 * alphas[0] + 3 * alphas[1] + 3) / 7;	/* bit code 100 */
	  alphas[5] = (3 * alphas[0] + 4 * alphas[1] + 3) / 7;	/* bit code 101 */
	  alphas[6] = (2 * alphas[0] + 5 * alphas[1] + 3) / 7;	/* bit code 110 */
	  alphas[7] = (1 * alphas[0] + 6 * alphas[1] + 3) / 7;	/* bit code 111 */
	}
	else {
	  /* 6-alpha block.
	     Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated. */
	  alphas[2] = (4 * alphas[0] + 1 * alphas[1] + 2) / 5;	/* Bit code 010 */
	  alphas[3] = (3 * alphas[0] + 2 * alphas[1] + 2) / 5;	/* Bit code 011 */
	  alphas[4] = (2 * alphas[0] + 3 * alphas[1] + 2) / 5;	/* Bit code 100 */
	  alphas[5] = (1 * alphas[0] + 4 * alphas[1] + 2) / 5;	/* Bit code 101 */
	  alphas[6] = 0x00;		/* Bit code 110 */
	  alphas[7] = 0xFF;		/* Bit code 111 */
	}


	/* Note: Have to separate the next two loops,
	   it operates on a 6-byte system. */
	/* First three bytes */
	bits = *((unsigned int*)alphamask);
	for (j = 0; j < 2; j++) {
	  for (i = 0; i < 4; i++) {
	    CMedia::Pixel& p = pixels[ x+i + (y+j)*dw ];
	    if ( p.r == 0.0f )
	      p.r = (float) ( alphas[bits & 0x07] ) / 255.0f;
	    else
	      p.a = (float) ( alphas[bits & 0x07] ) / 255.0f;

	    bits >>= 3;
	  }
	}

	/* Last three bytes */
	bits = *((unsigned int*)&alphamask[3]);
	for (j = 2; j < 4; j++) {
	  for (i = 0; i < 4; i++) {

	    CMedia::Pixel& p = pixels[ x+i + (y+j)*dw ];
	    if ( p.r == 0.0f )
	      p.r = (float) ( alphas[bits & 0x07] ) / 255.0f;
	    else
	      p.a = (float) ( alphas[bits & 0x07] ) / 255.0f;

	    bits >>= 3;
	  }
	}


      }
    }
  }

  void ddsImage::UncompressedA16B16G16R16( unsigned char* src )
  {
    unsigned int x, y;
    unsigned short* temp = (unsigned short*) src;
    unsigned int dw = width();
    unsigned int dh = height();
    Pixel* pixels = (Pixel*)_hires->data().get();
    for (y = 0; y < dh; ++y) {
      for (x = 0; x < dw; ++x) {
	CMedia::Pixel& p = pixels[ x + y*dw ];
	p.a = (float) ( *temp++ ) / 65535.0f;
	p.b = (float) ( *temp++ ) / 65535.0f;
	p.g = (float) ( *temp++ ) / 65535.0f;
	p.r = (float) ( *temp++ ) / 65535.0f;
      }
    }
  }



  void ddsImage::DecompressFloat( unsigned char* src, unsigned int CompFormat )
  {
    unsigned int x, y;
    float* temp = (float*) src;
    half* h = (half*) src;
    unsigned int dw = width();
    unsigned int dh = height();
    Pixel* pixels = (Pixel*)_hires->data().get();
    for (y = 0; y < dh; ++y) {
      for (x = 0; x < dw; ++x) {
	CMedia::Pixel& p = pixels[ x + y*dw ];
	switch ( CompFormat )
	  {
	  case PF_G32R32F:
	    p.g = *temp++;
	    p.r = *temp++;
	    break;
	  case PF_R32F:
	    p.r = p.g = p.b = *temp++;
	    break;
	  case PF_A32B32G32R32F:
	    p.a = *temp++;
	    p.b = *temp++;
	    p.g = *temp++;
	    p.r = *temp++;
	    break;
	  case PF_G16R16F:
	    p.g = *h++;
	    p.r = *h++;
	    break;
	  case PF_R16F:
	    p.r = p.g = p.b = *h++;
	    break;
	  case PF_A16B16G16R16F:
	    p.a = *h++;
	    p.b = *h++;
	    p.g = *h++;
	    p.r = *h++;
	    break;
	  }
      }
    }
  }

  void ddsImage::Decompress( unsigned char* src, unsigned int CompFormat,
			     DDSURFACEDESC2* ddsd )
  {
#ifdef PATENTS
    if ( CompFormat == PF_DXT1 || CompFormat == PF_DXT2 ||
	 CompFormat == PF_DXT3 || CompFormat == PF_DXT4 ||
	 CompFormat == PF_DXT5 )
      {
	const char* dpx = getenv( "MRV_DXT" );
	if ( dpx == NULL || strlen( dpx ) == 0 )
	  {
	    mrvALERT( "%s: Patented DXT codec, setenv MRV_DXT",
		      filename() );
	    return;
	  }
      }
#endif

    switch (CompFormat)
      {		
      case PF_ARGB:
      case PF_RGB:
      case PF_LUMINANCE:
      case PF_LUMINANCE_ALPHA:
	return DecompressARGB( src, &ddsd->ddpfPixelFormat );
      case PF_DXT1:
	alpha_layers();
	return DecompressDXT1( src );
      case PF_DXT2:
	alpha_layers();
	return DecompressDXT2( src );
      case PF_DXT3:
	alpha_layers();
	return DecompressDXT3( src );
      case PF_DXT4:
	alpha_layers();
	return DecompressDXT4( src );
      case PF_DXT5:
	alpha_layers();
	return DecompressDXT5( src );
      case PF_ATI1N:
	return DecompressAti1n( src );
      case PF_3DC:
	return Decompress3Dc( src );
      case PF_RXGB:
	alpha_layers();
	return DecompressRXGB( src );
      case PF_A16B16G16R16F:
      case PF_A32B32G32R32F:
	alpha_layers();
      case PF_R16F:
      case PF_G16R16F:
      case PF_R32F:
      case PF_G32R32F:
	return DecompressFloat( src, CompFormat );
      case PF_A16B16G16R16:
      case PF_UNKNOWN:
	alpha_layers();
	return UncompressedA16B16G16R16( src );
      }
  }

  void ddsImage::GetBytesPerBlock( unsigned int* srcDataSize,
				   unsigned int* bytesPerBlock, 
				   unsigned int* CompFormat,
				   FILE* f,
				   const DDSURFACEDESC2* ddsd )
  {
    int dw = width();
    int dh = height();
    if ( ddsd->ddpfPixelFormat.dwFlags & DDPF_FOURCC )
      {
	*srcDataSize = ((dw + 3)/4) * ((dh + 3)/4) * ddsd->dwDepth;
	switch ( ddsd->ddpfPixelFormat.dwFourCC )
	  {
	  case DDS_DXT1:
	    *CompFormat = PF_DXT1;
	    *bytesPerBlock = 8;
	    break;
	  case DDS_DXT2:
	    *CompFormat = PF_DXT2;
	    *bytesPerBlock = 16;
	    break;
	  case DDS_DXT3:
	    *CompFormat = PF_DXT3;
	    *bytesPerBlock = 16;
	    break;
	  case DDS_DXT4:
	    *CompFormat = PF_DXT4;
	    *bytesPerBlock = 16;
	    break;
	  case DDS_DXT5:
	    *CompFormat = PF_DXT5;
	    *bytesPerBlock = 16;
	    break;
	  case DDS_ATI1:
	    *CompFormat = PF_ATI1N;
	    *bytesPerBlock = 16;
	    break;
	  case DDS_ATI2:
	    *CompFormat = PF_3DC;
	    *bytesPerBlock = 16;
	    break;
	  case DDS_RXGB:
	    *CompFormat = PF_RXGB;
	    *bytesPerBlock = 16;
	    break;
	  case DDS_DOLL:
	    *CompFormat = PF_A16B16G16R16;
	    *srcDataSize   = dw * dh * ddsd->dwDepth;
	    *bytesPerBlock = 8;
	    break;
	  case DDS_o000:
	    *CompFormat = PF_R16F;
	    *srcDataSize   = dw * dh * ddsd->dwDepth;
	    *bytesPerBlock = 2;
	    break;
	  case DDS_p000:
	    *CompFormat = PF_G16R16F;
	    *srcDataSize   = dw * dh * ddsd->dwDepth;
	    *bytesPerBlock = 4;
	    break;
	  case DDS_q000:
	    *CompFormat = PF_A16B16G16R16F;
	    *srcDataSize   = dw * dh * ddsd->dwDepth;
	    *bytesPerBlock = 8;
	    break;
	  case DDS_r000:
	    *CompFormat = PF_R32F;
	    *srcDataSize   = dw * dh * ddsd->dwDepth;
	    *bytesPerBlock = 4;
	    break;
	  case DDS_s000:
	    *CompFormat = PF_G32R32F;
	    *srcDataSize   = dw * dh * ddsd->dwDepth;
	    *bytesPerBlock = 8;
	    break;
	  case DDS_t000:
	    *CompFormat = PF_A32B32G32R32F;
	    *srcDataSize   = dw * dh * ddsd->dwDepth;
	    *bytesPerBlock = 16;
	    break;
	  default:
	    *CompFormat = PF_UNKNOWN;
	    *bytesPerBlock = 16;
	    break;
	  }
      }
    else
      {
	if ( ddsd->ddpfPixelFormat.dwFlags & DDPF_LUMINANCE )
	  {
	    if ( ddsd->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS )
	      *CompFormat = PF_LUMINANCE_ALPHA;
	    else
	      *CompFormat = PF_LUMINANCE;
	  }
	else
	  {
	    if ( ddsd->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS )
	      *CompFormat = PF_ARGB;
	    else
	      *CompFormat = PF_RGB;
	  }
	*srcDataSize   = dw * dh * ddsd->dwDepth;
	*bytesPerBlock = (ddsd->ddpfPixelFormat.dwRGBBitCount >> 3);
      }

    *srcDataSize *= *bytesPerBlock;      
  }

  void ddsImage::MSBOrderShort( unsigned char* p, int len )
  {
    int c;

    register unsigned char *q;

    assert(p != (unsigned char *) NULL);
    q=p+len;
    while (p < q)
      {
	c=(int) (*p);
	*p=(*(p+1));
	p++;
	*p++=(unsigned char) c;
      }
  }

  void ddsImage::MSBOrderLong( unsigned char* buffer, int len )
  {
    int
      c;

    register unsigned char
      *p,
      *q;

    assert(buffer != (unsigned char *) NULL);
    q=buffer+len;
    while (buffer < q)
      {
	p=buffer+3;
	c=(int) (*p);
	*p=(*buffer);
	*buffer++=(unsigned char) c;
	p=buffer+1;
	c=(int) (*p);
	*p=(*buffer);
	*buffer++=(unsigned char) c;
	buffer+=2;
      }
  }

  unsigned long ddsImage::ReadBlobMSBLong( FILE* f )
  {
    unsigned long value;
    unsigned char buffer[4];
    unsigned char* p = buffer;

    size_t count = fread( buffer, 1, 4, f );
    if ( count == 0 ) return 0;

    value=(*p++) << 24;
    value|=(*p++) << 16;
    value|=(*p++) << 8;
    value|=(*p++);
    return(value & 0xffffffff);
  }

  unsigned long ddsImage::ReadBlobLSBLong( FILE* f )
  {
    unsigned long value;
    unsigned char buffer[4];
    unsigned char* p = buffer;

    size_t count = fread( buffer, 1, 4, f );
    if ( count == 0 ) return 0;

    value=(*p++);
    value|=(*p++) << 8;
    value|=(*p++) << 16;
    value|=(*p++) << 24;
    return(value & 0xffffffff);
  }

  ddsImage::ddsImage() :
    CMedia(),
    _alpha( false )
  {
  }

  ddsImage::~ddsImage()
  {
  }

  bool ddsImage::test(const boost::uint8_t *data, unsigned len )
  {
    if ( memcmp( data, kDDS_MAGIC_NUMBER, 4 ) != 0 )
      return false;
    return true;
  }

  const char* const ddsImage::compression() const
  {
    static const char* kCompressionType[] =
      {
	"ARGB",
	"RGB",
	"DXT1 (S3TC)",
	"DXT2 (S3TC)",
	"DXT3 (S3TC)",
	"DXT4 (S3TC)",
	"DXT5 (S3TC)",
	"3DC",
	"ATI1N",
	"LUMINANCE",
	"LUMINANCE_ALPHA",
	"RXGB", /* Doom3 normal maps */
	"Half",
	"Half",
	"Half",
	"Half",
	"None",
	"None",
	"None",
	"Unknown"
      };
    return kCompressionType[ _compression ];
  }

  bool ddsImage::fetch( const boost::int64_t frame )
  {
    DDSURFACEDESC2 ddsd;
    unsigned char* data;
    size_t size;
    unsigned int bytesPerBlock, sourceDataSize, compFormat;

    /*
      Open image file.
    */
    FILE* f = fltk::fltk_fopen( sequence_filename(frame).c_str(), "r" );
    if ( !f ) return false;

    /*
      Read DDS header.
    */
    ReadBlobLSBLong(f);  /* magic */
    ddsd.dwSize = ReadBlobLSBLong(f);
    ddsd.dwFlags = ReadBlobLSBLong(f);
    ddsd.dwHeight = ReadBlobLSBLong(f);
    ddsd.dwWidth = ReadBlobLSBLong(f);
    ddsd.dwPitchOrLinearSize = ReadBlobLSBLong(f);
    ddsd.dwDepth = ReadBlobLSBLong(f);
    if ( ddsd.dwDepth <= 0 ) ddsd.dwDepth = 1;
    ddsd.dwMipMapCount   = ReadBlobLSBLong(f);
    ddsd.dwAlphaBitDepth = ReadBlobLSBLong(f);
    size_t ok = fread( ddsd.dwReserved, 1, sizeof(ddsd.dwReserved), f );
    if (!ok) return false;

    ddsd.ddpfPixelFormat.dwSize = ReadBlobLSBLong(f);
    ddsd.ddpfPixelFormat.dwFlags = ReadBlobLSBLong(f);
    ddsd.ddpfPixelFormat.dwFourCC = ReadBlobLSBLong(f);


    ddsd.ddpfPixelFormat.dwRGBBitCount = ReadBlobLSBLong(f);
    ddsd.ddpfPixelFormat.dwRBitMask = ReadBlobLSBLong(f);
    ddsd.ddpfPixelFormat.dwGBitMask = ReadBlobLSBLong(f);
    ddsd.ddpfPixelFormat.dwBBitMask = ReadBlobLSBLong(f);
    ddsd.ddpfPixelFormat.dwABitMask = ReadBlobLSBLong(f);
			
    ddsd.ddsCaps.dwCaps1 = ReadBlobLSBLong(f);
    ddsd.ddsCaps.dwCaps2 = ReadBlobLSBLong(f);
    ddsd.ddsCaps.dwCaps3 = ReadBlobLSBLong(f);
    ddsd.ddsCaps.dwCaps4 = ReadBlobLSBLong(f);
    ddsd.dwTextureStage = ReadBlobLSBLong(f);

    image_size( ddsd.dwWidth, ddsd.dwHeight );
    allocate_pixels(frame);

    _layers.clear();
    _num_channels = 0;
    _gamma = 1.0f;
    rgb_layers();
    lumma_layers();

    /*
      Decode scanlines
    */
    GetBytesPerBlock( &sourceDataSize, &bytesPerBlock, &compFormat, f, &ddsd );
    data = (unsigned char*) malloc( sourceDataSize );
    memset( data, 0, sourceDataSize );

    size = fread( data, 1, sourceDataSize, f );

    /*
      Decompress data
    */
    _compression = compFormat;
    Decompress( data, compFormat, &ddsd );

    free(data);

    fclose(f);
    return true;
  }


} // namespace mrv
