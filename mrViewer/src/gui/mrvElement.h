
#include <fltk/Item.h>

#include "gui/mrvMedia.h"

namespace mrv {

  class Element : public fltk::Item
  {
  public:
    Element( mrv::media& );

       const mrv::media& element() { return _elem; }
       mrv::media element() const { return _elem; }

     public:
       mrv::media _elem;
  };

}
