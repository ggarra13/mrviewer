/**
 * @file   mrayImage.h
 * @author gga
 * @date   Fri Sep 21 01:19:27 2007
 * 
 * @brief  Custom image loader for mental ray image formats
 *         (st, ct, etc)
 * 
 */

#ifndef mrayImage_h
#define mrayImage_h

#include <CMedia.h>

namespace mrv {

  class mrayImage : public CMedia 
  {
    mrayImage();
    ~mrayImage();

    static CMedia* create() { return new mrayImage(); }

  public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual const char* const format() const;

    virtual bool fetch( const boost::int64_t& frame );

  protected:


    unsigned short _format;
  };


} // namespace mrv


#endif // mrayImage_h

