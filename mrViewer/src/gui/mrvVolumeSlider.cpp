#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <FL/Fl.H>

#include "mrvVolumeSlider.h"

namespace mrv {

int VolumeSlider::handle( int event )
{
    int err = Fl_Slider::handle( event );
    switch (event) {
        case FL_FOCUS:
        case FL_UNFOCUS:
            return 1;
        case FL_MOUSEWHEEL: {
            float v = value();
            float dx = Fl::event_dx();
            float dy = Fl::event_dy();
            float d = dx + dy;
            if ( d > 0 ) v += 0.1;
            else if ( d < 0 ) v -= .1;
            value(v);
            do_callback();
            redraw();
            return 1;
        }
    }
    return err;
}

} // namespace mrv
