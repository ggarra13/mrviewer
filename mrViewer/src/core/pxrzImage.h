/**
 * @file   pxrzImage.h
 * @author gga
 * @date   Fri Sep 21 01:25:12 2007
 * 
 * @brief  Image class to read Pixar's .z files
 * 
 * 
 */

#ifndef pxrzImage_h
#define pxrzImage_h

#include <CMedia.h>

namespace mrv {

  class pxrzImage : public CMedia 
  {
    pxrzImage();
    ~pxrzImage();

    static CMedia* create() { return new pxrzImage(); }

  public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual const char* const format() const { return "Pixar shadow map"; }

    virtual bool fetch( const boost::int64_t frame );
  };

} // namespace mrv


#endif // pxrzImage_h

