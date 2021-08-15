#ifndef mrvButton_h
#define mrvButton_h

#include <FL/Fl_Button.H>
#include <FL/Fl_Toggle_Button.H>


namespace mrv
{
    class Button : public Fl_Button
    {
    public:
        Button( int X, int Y, int W, int H, const char* L = 0 );

        virtual void draw();
    };

    class Toggle_Button : public Fl_Toggle_Button
    {
    public:
        Toggle_Button( int X, int Y, int W, int H, const char* L = 0 );

        virtual void draw();
    };
}



#endif
