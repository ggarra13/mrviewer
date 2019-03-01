
//
//
//


#ifndef mrvValue_Input_h
#define mrvValue_Input_h

#include <cstdio>
#include <cmath>
#include "core/mrvI8N.h"
#include <FL/Fl_Value_Input.H>

namespace mrv {

class Value_Input : public Fl_Value_Input
{
public:
    Value_Input( int x, int y, int w, int h, char* l = 0 ) :
    Fl_Value_Input( x, y, w, h, l )
    {
    }

    virtual int format(char* buffer) {
        double v = value();
        if (!step()) return sprintf(buffer, "%g", v);
        double s = step()-floor(step());
        if (!s)
            return sprintf(buffer, "%ld", long(v));
        int i=1;
        for (; i < 8; i++) {
            if (s >= .099) break;
            s *= 10;
        }
        return sprintf(buffer, "%.*f", i, v);
    }
};

}; // namespace mrv


#endif
