/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2016  Gonzalo Garramuño

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
