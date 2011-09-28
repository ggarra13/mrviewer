/**
 * @file   hdrImage.h
 * @author gga
 * @date   Fri Sep 21 01:13:09 2007
 * 
 * @brief  Exr Image Loader
 * 
 * 
 */

#ifndef hdrImage_h
#define hdrImage_h

#include <CMedia.h>


namespace mrv {

  class hdrImage : public CMedia 
  {
    hdrImage();
    ~hdrImage();

    static CMedia* create() { return new hdrImage(); }


  public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual const char* const format() const { return "Radiance HDR"; }


    bool fetch( const boost::int64_t frame );
  protected:

    typedef unsigned char COLR[4];

    void read_header( FILE* f );
    int read_colors(COLR* scanline, int len, FILE* f);
    int oldreadcolrs(COLR* scanline, int len, FILE* fp);

    void colr2color( PixelType& col, COLR clr );

  protected:


    bool  cieXYZ;
    bool  flipX;
    bool  flipY;

    float exposure;

    struct CIE {
      float x, y;
    };

    CIE cieXY[4];
    float corr[3];
  };

}

#endif // hdrImage_h

