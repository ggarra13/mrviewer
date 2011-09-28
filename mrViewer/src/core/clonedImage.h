/**
 * @file   clonedImage.h
 * @author gga
 * @date   Fri Sep 21 01:16:30 2007
 * 
 * @brief  Image class used to clone other images
 * 
 * 
 */

#ifndef clonedImage_h
#define clonedImage_h

#include "CMedia.h"

namespace mrv {

  class clonedImage : public CMedia
  {
  public:
    clonedImage( const CMedia* other );

    virtual const char* const format() const { return "Cloned Image"; }

    virtual bool is_sequence() const { return false; }

    virtual bool has_changed() { return false; }

    ////////////////// Set the frame for the current image (sequence)
    virtual bool            frame( const boost::int64_t f ) { return true; };
    virtual boost::int64_t  frame() const { return _frame; }

  };

}

#endif // clonedImage_h

