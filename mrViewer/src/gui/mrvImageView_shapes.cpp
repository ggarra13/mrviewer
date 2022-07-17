#include "core/CMedia.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"

size_t find_index( const mrv::Reel& r, const mrv::media& m )
{
    for ( size_t i = 0; i < r->images.size(); ++i )
    {
        if ( r->images[i] == m ) return i;
    }
    return 0;
}


void next_shape_frame( mrv::ImageView* view )
{
    using namespace mrv;

    const mrv::media& m = view->foreground();
    if ( !m) return;

    mrv::ImageBrowser* b = view->browser();
    const mrv::Reel& reel = b->current_reel();


    mrv::media fg = m;

    while ( 1 )
    {
        size_t idx = find_index( reel, fg );

        CMedia* img = fg->image();

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
            if ( idx >= reel->images.size()-1 ) return;
            fg = reel->images[++idx];
        }
    }
}

void previous_shape_frame( mrv::ImageView* view )
{
    using namespace mrv;


    const mrv::media& m = view->foreground();
    if ( !m) return;

    mrv::ImageBrowser* b = view->browser();
    const mrv::Reel& reel = b->current_reel();

    mrv::media fg = m;

    while ( 1 )
    {
        size_t idx = find_index( reel, fg );

        CMedia* img = fg->image();

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
            if ( idx == 0 ) return;
            fg = reel->images[--idx];
        }
    } while ( fg != m );
}
