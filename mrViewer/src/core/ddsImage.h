/**
 * @file   wandImage.h
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 * 
 * @brief  A simple wrapper class to read all of ImageMagick's image formats
 * 
 * 
 */

#ifndef ddsImage_h
#define ddsImage_h

#include "CMedia.h"

struct Color8888;
struct DDPFPIXELFORMAT;
struct DDSURFACEDESC2;


namespace mrv {

  class ddsImage : public CMedia
  {

    ddsImage();

    static CMedia* create() { return new ddsImage(); }

  public:
    static bool test(const boost::uint8_t *data, unsigned len );
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual ~ddsImage();

    virtual const char* const format() const { return "DDS"; }

    /// Returns the image compression (if any)
    virtual const char* const compression() const;

    /// Returns true if image has an alpha channel
    virtual bool  has_alpha() const { return _alpha; }

    virtual bool fetch( const boost::int64_t frame );

  protected:
    void ReadColors(const unsigned char* Data, Color8888* Out);
    void ReadColor(unsigned short Data, Color8888* Out);
    void DecompressDXT1(unsigned char* src );
    void CorrectPreMult();
    void GetBitsFromMask(unsigned int Mask, 
			 unsigned int* ShiftLeft, 
			 unsigned int* ShiftRight);

    void DecompressDXT2( unsigned char* src );
    void DecompressDXT3( unsigned char* src );
    void DecompressDXT4( unsigned char* src );
    void DecompressDXT5( unsigned char* src );
    void DecompressARGB( unsigned char* src, DDPFPIXELFORMAT* Head );
    void DecompressAti1n( unsigned char* src );
    void Decompress3Dc( unsigned char* src );
    void DecompressRXGB( unsigned char* src );
    void UncompressedA16B16G16R16( unsigned char* src );
    void DecompressFloat( unsigned char* src, unsigned int CompFormat );

    void Decompress( unsigned char* src, unsigned int CompFormat,
		     DDSURFACEDESC2* ddsd );

    // Reading
    void GetBytesPerBlock( unsigned int* srcDataSize,
			   unsigned int* bytesPerBlock, unsigned int* CompFormat,
			   FILE* f,
			   const DDSURFACEDESC2* ddsd );
    void MSBOrderShort( unsigned char* s, int len );
    void MSBOrderLong( unsigned char* s, int len );

    unsigned long ReadBlobMSBLong( FILE* f );
    unsigned long ReadBlobLSBLong( FILE* f );
  protected:
    short     _compression;
    bool            _alpha;
  };


} // namespace mrv


#endif // ddsImage_h
