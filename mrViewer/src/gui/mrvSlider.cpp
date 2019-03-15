

#include <math.h>
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include "mrvSlider.h"

namespace mrv {


void Slider::draw_ticks(const mrv::Recti& r, int min_spacing)
{
    int x1, y1, x2, y2, dx, dy, w;
    x1 = x2 = r.x()+(slider_size()-1)/2;
    dx = 1;
    y1 = r.y();
    y2 = r.b()-1;
    dy = 0;
    w = r.w();


    fl_push_clip( r.x(), r.y(), r.w(), r.h() );

    if (w <= 0) return;

    double A = minimum();
    double B = maximum();
    if (A > B) {
	A = B;
	B = minimum();
    }
    //if (!finite(A) || !finite(B)) return;

    if (min_spacing < 1) min_spacing = 10; // fix for fill sliders
    // Figure out approximate size of min_spacing at zero:

    double mul = 1; // how far apart tick marks are
    double div = 1;
    int smallmod = 5; // how many tick marks apart "larger" ones are
    int nummod = 15; // how many tick marks apart numbers are


    int powincr = 10000;

    double derivative = (B-A)*min_spacing/w;
    if (derivative < step()) derivative = step();
    while (mul*5 <= derivative) mul *= 10;
    while (mul > derivative*2*div) div *= 10;
    if (derivative*div > mul*2) {
        mul *= 5;
        smallmod = 2;
    }
    else if (derivative*div > mul) {
        mul *= 2;
        nummod /= 2;
    }
    if ( nummod <= 1 ) nummod = 1;

    Fl_Color textcolor = this->labelcolor();
    Fl_Color linecolor = FL_BLACK;

    fl_color(linecolor);
    char buffer[128];
    for (int n = 0; ; n++) {
        // every ten they get further apart for log slider:
        if (n > powincr) {
            mul *= 10;
            n = (n-1)/10+1;
        }
        double v = mul*n/div;
        if (v > fabs(A) && v > fabs(B)) break;
        int sm = n%smallmod ? 3 : 0;
        if (v >= A && v <= B) {
            int t = slider_position(v, w);
            fl_line(x1+dx*t+dy*sm, y1+dy*t+dx*sm, x2+dx*t, y2+dy*t);
            if (n-1 != 0 && (n-1)%nummod == 0) {
		sprintf( buffer, "%g", v );
                char* p = buffer;
                fl_font(labelfont(), labelsize());
                fl_color(textcolor);
                int wt = 0, ht = 0;
                fl_measure( p, wt, ht );
                fl_draw(p, float(x1+dx*t-wt/2),
                        float(y1+dy*t+fl_height()-fl_descent()));
                fl_color(linecolor);
            }
        }
        if (v && -v >= A && -v <= B) {
            int t = slider_position(-v, w);
            fl_line(x1+dx*t+dy*sm, y1+dy*t+dx*sm, x2+dx*t, y2+dy*t);
            if (n%nummod == 0) {
		sprintf( buffer, "%g", -v );
                char* p = buffer;
                fl_font(labelfont(), labelsize());
                fl_color(textcolor);
                // int wt = 0, ht = 0;
                // measure( p, wt, ht );
                fl_draw(p, float(x1+dx*t),
                         float(y1+dy*t+fl_height()-fl_descent()));
                fl_color(linecolor);
            }
        }
    }

    fl_pop_clip();
}

int Slider::slider_position( double value, int w )
{
    double A = minimum();
    double B = maximum();
    if (B == A) return 0;
    bool flip = B < A;
    if (flip) {A = B; B = minimum();}
    if (!horizontal()) flip = !flip;
    // if both are negative, make the range positive:
    if (B <= 0) {flip = !flip; double t = A; A = -B; B = -t; value = -value;}
    double fraction;
    if (!(slider_type() & kLOG)) {
	// linear slider
    fraction = (value-A)/(B-A);
  } else if (A > 0) {
    // logatithmic slider
    if (value <= A) fraction = 0;
    else fraction = (::log(value)-::log(A))/(::log(B)-::log(A));
  } else if (A == 0) {
    // squared slider
    if (value <= 0) fraction = 0;
    else fraction = sqrt(value/B);
  } else {
    // squared signed slider
    if (value < 0) fraction = (1-sqrt(value/A))*.5;
    else fraction = (1+sqrt(value/B))*.5;
  }
  if (flip) fraction = 1-fraction;
  w -= slider_size(); if (w <= 0) return 0;
  if (fraction >= 1) return w;
  else if (fraction <= 0) return 0;
  else return int(fraction*w+.5);
}


double Slider::position_value(int X, int w) {
    w -= slider_size(); if (w <= 0) return minimum();
    double A = minimum();
    double B = maximum();
    bool flip = B < A;
    if (flip) {A = B; B = minimum();}
    if (!horizontal()) flip = !flip;
    if (flip) X = w-X;
    double fraction = double(X)/w;
    if (fraction <= 0) return A;
    if (fraction >= 1) return B;
    // if both are negative, make the range positive:
    flip = (B <= 0);
    if (flip) {double t = A; A = -B; B = -t; fraction = 1-fraction;}
    double value;
    double derivative;
    if (!log()) {
	// linear slider
	value = fraction*(B-A)+A;
	derivative = (B-A)/w;
    } else if (A > 0) {
	// log slider
	double d = (::log(B)-::log(A));
	value = exp(fraction*d+::log(A));
	derivative = value*d/w;
    } else if (A == 0) {
	// squared slider
	value = fraction*fraction*B;
	derivative = 2*fraction*B/w;
    } else {
	// squared signed slider
	fraction = 2*fraction - 1;
	if (fraction < 0) B = A;
	value = fraction*fraction*B;
	derivative = 4*fraction*B/w;
    }
    // find nicest multiple of 10,5, or 2 of step() that is close to 1 pixel:
    if (step() && derivative > step()) {
	double w = log10(derivative);
	double l = ceil(w);
	int num = 1;
	int i; for (i = 0; i < l; i++) num *= 10;
	int denom = 1;
	for (i = -1; i >= l; i--) denom *= 10;
	if (l-w > 0.69897) denom *= 5;
	else if (l-w > 0.30103) denom *= 2;
	value = floor(value*denom/num+.5)*num/denom;
    }
    if (flip) return -value;
    return value;
}

int Slider::handle( int event )
{
    if ( slider_type() != kLOG )
	return Fl_Slider::handle(event);

    mrv::Recti r( x(), y(), w(), h() );
    
    switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
	    damage( FL_DAMAGE_ALL );
	    redraw();
	    return 1;
	case FL_PUSH:
	    damage( FL_DAMAGE_EXPOSE );  // DAMAGE_HIGHLIGHT
	    redraw(); 
	    handle_push();
	case FL_DRAG: {
	    // figure out the space the slider moves in and where the event is:
	    int w,mx;
	    if (horizontal()) {
		w = r.w();
		mx = Fl::event_x()-r.x();
	    } else {
		w = r.h();
		mx = Fl::event_y()-r.y();
	    }
	    if (w <= slider_size()) return 1;
	    static int offcenter;
	    int X = slider_position(value(), w);
	    if (event == FL_PUSH) {
		offcenter = mx-X;
		// we are done if they clicked on the slider:
		if (offcenter >= (slider_size() ? 0 : -8) &&
		    offcenter <= slider_size())
		    return 1;
		if (Fl::event_button() > FL_LEFT_MOUSE) {
		    // Move the near end of the slider to the cursor.
		    // This is good for scrollbars.
		    offcenter = (offcenter < 0) ? 0 : slider_size();
		} else {
		    // Center the slider under the cursor, what most toolkits do
		    offcenter = slider_size()/2;
		}
	    }
	    double v;
		      RETRY:
	    X = mx-offcenter;
	    if (X < 0) {
		X = 0;
		offcenter = mx; if (offcenter < 0) offcenter = 0;
	    } else if (X > (w-slider_size())) {
		X = w-slider_size();
		offcenter = mx-X;
		if (offcenter > slider_size()) offcenter = slider_size();
	    }
	    v = position_value(X, w);
	    handle_drag(v);
	    // make sure a click outside the sliderbar moves it:
	    if (event == FL_PUSH && value() == previous_value()) {
		offcenter = slider_size()/2;
		event = FL_DRAG;
		goto RETRY;
	    }
	    return 1;}
	case FL_RELEASE:
	    handle_release();
	    redraw(); //DAMAGE_HIGHLIGHT);
	    return 1;
	case FL_KEYBOARD:
	    // Only arrows in the correct direction are used.  This allows the
	    // opposite arrows to be used to navigate between a set of parellel
	    // sliders.
	    switch (Fl::event_key()) {
		case FL_Up:
		case FL_Down:
		    if (horizontal()) return 0;
		    break;
		case FL_Left:
		case FL_Right:
		    if (!horizontal()) return 0;
	    }
	default:
	    return Fl_Valuator::handle(event);
    }
    return 1;
}

void Slider::draw()
{
    draw_box();
    
    mrv::Recti r( x() + Fl::box_dx(box()),
		  y() + Fl::box_dy(box()),
		  w() - Fl::box_dw(box()),
		  h() - Fl::box_dh(box()) );
    draw_ticks( r, 10 );

    int X = r.x() + slider_position( value(), r.w() - 10 );
    int Y = r.y();
    int W = 10;
    int H = r.h();
    Fl_Color c = color();
    draw_box( FL_ROUND_UP_BOX, X, Y, W, H, c );
    clear_damage();
}

} // namespace mrv
