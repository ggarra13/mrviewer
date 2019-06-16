#include "mrvElement.h"
#include "gui/mrvPreferences.h"

namespace {
    const char* kModule = "elem";
}

namespace mrv {

static const int VMARGIN = 6;

void Element::make_thumbnail()
{
    _elem->create_thumbnail();

    Fl_Image* b = NULL;
    if ( _elem->thumbnail() )
        b = _elem->thumbnail();
    else
    {
        uchar* d = new uchar[64*64];
        memset( d, 0, 64*64 );
        b = new Fl_RGB_Image( d, 64, 64, 1 );
    }
    if ( !b || b->w() < 1 )
    {
        LOG_ERROR( "Empty image in thumbnail"  );
    }
    if ( !image )
    {
        image = new Fl_Box(0,VMARGIN,b->w(), b->h()-VMARGIN);
        image->selection_color( FL_YELLOW );
    }
    if ( b ) image->image( b );

}

    Element::Element(mrv::media m) :
    Fl_Group(0,0,VMARGIN,64+VMARGIN*2),
    image(NULL),
    _elem( m )
    {	// VMARGIN makes group slightly larger than items
        // Create widget to hold image
        //
        if ( !m )
        {
            LOG_ERROR( _("Empty media provided to Element constructor") );
            return;
        }


        CMedia* img = m->image();
        if ( !img )
        {
            LOG_ERROR( _("Empty image provided to Element constructor") );
            return;
        }


        make_thumbnail();
        // Create label

        label = new Fl_Box(image->w(), VMARGIN, 800, 64);

        char info[2048];
        std::string name = img->name();
        int pos = -2;
        while ( ( pos = name.find( '@', pos+2 ) ) != std::string::npos )
        {
            name = name.substr( 0, pos + 1 ) + '@' +
                   name.substr( pos + 1, name.size() );
        }
        if ( dynamic_cast< clonedImage* >( img ) != NULL )
        {
            sprintf( info,
                     _("Cloned Image\n"
                       "Name: %s\n"
                       "Date: %s\n"
                       "Resolution: %dx%d"),
                     name.c_str(),
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
                     name.c_str(),
                     img->width(),
                     img->height(),
                     img->start_frame(),
                     img->end_frame(),
                     img->fps()
                     );
        }

        DBG;
        label->copy_label( info );

        label->color(0xddddff00);
        label->labelcolor( FL_BLACK );

        label->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

        label->box(FL_FLAT_BOX);

        end();

    }

Element::~Element()
{
    delete image; image = NULL;
    delete label; label = NULL;
}


   void Element::Label(const char *s) {
        if ( label ) label->copy_label(s);
    }

    // Draw ourself at a specific X,Y position
    void Element::DrawAt(int X, int Y) {

        int xsave = x(), ysave = y();
        // Temporarily move widget to mouse position
        position(X,Y);
        init_sizes();
        redraw();

        // Draw widget at mouse position
        draw();			// draw widget's group
        // ..now move widget back to where it was
        position(xsave,ysave);
        init_sizes();
        redraw();
    }

    const mrv::media& Element::media() const { return _elem; }

    void Element::draw() {
        Fl_Group::draw();
    }

}
