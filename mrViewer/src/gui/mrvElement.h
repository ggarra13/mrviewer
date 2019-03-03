/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuno

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

#ifndef mrvElement_h
#define mrvELement_h

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Box.H>

#include "core/clonedImage.h"
#include "gui/mrvMedia.h"


namespace mrv {


class Element : public Fl_Group {
    Fl_Box *image;		// image part of widget
    Fl_Box *label;		// label part of widget
public:
    Element(mrv::media& m);

    void Label(const char *s);

    // Draw ourself at a specific X,Y position
    void DrawAt(int X, int Y);

    const mrv::media& element() const;

    void draw();

  protected:
    mrv::media _elem;
};



}


#endif //
