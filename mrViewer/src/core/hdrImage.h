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

    void colr2color( Pixel& col, COLR clr );

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

