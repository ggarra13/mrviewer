/**
 * @file   shmapImage.h
 * @author gga
 * @date   Fri Sep 21 01:28:16 2007
 * 
 * @brief  Image reader for mental ray's standard shadow maps
 * 
 * 
 */

#ifndef shmapImage_h
#define shmapImage_h

#include <CMedia.h>

namespace mrv {

  class shmapImage : public CMedia 
  {
    shmapImage();
    ~shmapImage();

    static CMedia* create() { return new shmapImage(); }

  public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual const char* const format() const { return "mental images shadow map"; }

    virtual bool fetch(const boost::int64_t frame);
  };

} // namespace mrv


#endif // shmapImage_h

