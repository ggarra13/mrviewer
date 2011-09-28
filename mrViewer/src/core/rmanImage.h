/**
 * @file   rmanImage.h
 * @author gga
 * @date   Fri Nov 10 14:21:44 2006
 * 
 * @brief  A simple class to hook to a renderman socket stub using
 *         liquid's display drivers.
 * 
 * 
 */

#ifndef rmanImage_h
#define rmanImage_h

#include "stubImage.h"

namespace mrv {

  class rmanImage : public stubImage 
  {
    rmanImage();
    ~rmanImage();

    static CMedia* create() { return new rmanImage(); }

  public:
    rmanImage( const CMedia* other );

    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }
    bool fetch();

    const char* host() { return _host; }

    unsigned int port() { return _portB; }

    virtual bool has_changed();

    virtual const char* const format() const { return "Renderman Liquid stub"; }

  };

} // namespace mrv


#endif // rmanImage_h

