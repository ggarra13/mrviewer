#include "core/CMedia.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvImageView.h"

void next_shape_frame( mrv::ImageView* view )
{
    using namespace mrv;

    const mrv::media& m = view->foreground();
    if ( !m) return;

    mrv::media fg;

    do
    {
        fg = view->foreground();
        if ( !fg) return;

        CMedia* img = fg->image();
        if (!img) return;

        int64_t current_frame = view->frame();
        int64_t max_frame = std::numeric_limits<int64_t>::max();

        const GLShapeList& shapes = img->shapes();
        GLShapeList::const_iterator i = shapes.begin();
        GLShapeList::const_iterator e = shapes.end();
        for ( ; i != e; ++i )
        {
            const shape_type_ptr& s = *i;
            if ( s->frame > current_frame && s->frame < max_frame )
            {
                max_frame = s->frame;
            }
        }

        if ( max_frame < std::numeric_limits<int64_t>::max() )
        {
            view->seek( max_frame );
            break;
        }
        else {
            max_frame = img->position() + 1 +
                        img->last_frame() - img->first_frame();
            if ( max_frame > view->timeline()->maximum() )
                max_frame = view->timeline()->minimum();
            view->seek( max_frame );
            fg = view->foreground();
        }
    }
    while ( fg != m );
}

void previous_shape_frame( mrv::ImageView* view )
{
    using namespace mrv;


    const mrv::media& m = view->foreground();
    if ( !m) return;

    mrv::media fg;

    do
    {
        fg = view->foreground();
        if ( !fg) return;

        CMedia* img = fg->image();
        if (!img) return;

        int64_t current_frame = view->frame();
        int64_t min_frame = std::numeric_limits<int64_t>::min();

        const GLShapeList& shapes = img->shapes();
        GLShapeList::const_iterator i = shapes.begin();
        GLShapeList::const_iterator e = shapes.end();
        for ( ; i != e; ++i )
        {
            const shape_type_ptr& s = *i;
            if ( s->frame < current_frame && s->frame > min_frame )
            {
                min_frame = s->frame;
            }
        }

        if ( min_frame > std::numeric_limits<int64_t>::min() )
        {
            view->seek( min_frame );
            break;
        }
        else {
            min_frame = img->position() - 1; // - img->first_frame();
            if ( min_frame < view->timeline()->minimum() )
                min_frame = view->timeline()->maximum();
            view->seek( min_frame );
            fg = view->foreground();
        }
    } while ( fg != m );
}
