/**
 * @file   mapImage.h
 * @author gga
 * @date   Fri Sep 21 01:18:01 2007
 * 
 * @brief  Custom image reader for mental ray's map images.
 * 
 * 
 */

#ifndef mapImage_h
#define mapImage_h

#include <CMedia.h>

namespace mrv {

  class mapImage : public CMedia 
  {
    mapImage();
    ~mapImage();

    static CMedia* create() { return new mapImage(); }

  public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual const char* const format() const { return "mental images texture"; }

    /// Returns the image line order (if any)
    virtual const char* const line_order() const;

    virtual bool has_changed();

    virtual bool fetch(const boost::int64_t frame);

  protected:
    bool is_stub();

  protected:
    short _lineOrder;
    bool   _stub;
    time_t _atime;
  };

} // namespace mrv


#endif // mapImage_h

