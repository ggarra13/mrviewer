/**
 * @file   mrvImageList.h
 * @author gga
 * @date   Wed Oct 18 11:09:58 2006
 * 
 * @brief  An image list is a sequence of images
 * 
 * 
 */

#ifndef mrvImageList_h
#define mrvImageList_h

#include <vector>

#include <boost/shared_ptr.hpp>

#include "gui/mrvMedia.h"


namespace mrv
{
  typedef std::vector< media > MediaList;

} // namespace mrv


#endif // mrvImageList_h
