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
