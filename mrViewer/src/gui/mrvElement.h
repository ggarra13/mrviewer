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

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Box.H>

#include "core/clonedImage.h"
#include "gui/mrvMedia.h"

namespace mrv {

static const int VMARGIN = 2;

class Element : public Fl_Group {
    Fl_Box *image;		// image part of widget
    Fl_Box *label;		// label part of widget
public:
Element(mrv::media& m) : Fl_Group(0,0,1,80+VMARGIN*2),
                         _elem( m )
    {		// VMARGIN makes group slightly larger than items
        // Create widget to hold image
        //     Assign different image to each instance of us for demo purposes..
        //

        CMedia* img = m->image();
        m->create_thumbnail();
        image->image( m->thumbnail() );
        image = new Fl_Box(0,VMARGIN,image->w(),image->h());
        // Create label
        label = new Fl_Box(image->w(), VMARGIN, 800, 80);
        char info[2048];
        if ( dynamic_cast< clonedImage* >( img ) != NULL )
        {
            sprintf( info,
                     _("Cloned Image\n"
                       "Name: %s\n"
                       "Date: %s\n"
                       "Resolution: %dx%d"),
                     img->name().c_str(),
                     img->creation_date().c_str(),
                     img->width(),
                     img->height()
                     );
        }
        else
        {
            sprintf( info,
                     _("Directory: %s\n"
                       "Name: %s\n"
                       "Resolution: %dx%d\n"
                       "Frames: %" PRId64 "-%" PRId64 " FPS %g"),
                     img->directory().c_str(),
                     img->name().c_str(),
                     img->width(),
                     img->height(),
                     img->start_frame(),
                     img->end_frame(),
                     img->fps()
                     );
        }
        label->copy_label( info );
        label->color(0xddddff00);
        label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        label->box(FL_FLAT_BOX);
        end();
    }
    void Label(const char *s) {
        if ( label ) label->copy_label(s);
    }
    // Draw ourself at a specific X,Y position
    void DrawAt(int X, int Y) {
        int xsave = x(), ysave = y();
        // Temporarily move widget to mouse position
        position(X,Y);
        init_sizes();
        redraw();
        // Draw widget at mouse position
        draw();				// draw widget's group
        // ..now move widget back to where it was
        position(xsave,ysave);
        init_sizes();
        redraw();
    }

    const mrv::media& element() const { return _elem; }

    void draw() {
        Fl_Group::draw();
    }

  protected:
    mrv::media _elem;
};



}
