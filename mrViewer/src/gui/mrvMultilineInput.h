#include <FL/Fl_Multiline_Input.H>
#include <FL/Enumerations.H>

namespace mrv
{

    class MultilineInput : public Fl_Multiline_Input
    {
    public:
        MultilineInput( int X, int Y, int W, int H, char* l = 0 ) :
            Fl_Multiline_Input( X, Y, W, H, l )
            {
                //box( FL_ROUNDED_BOX );
            };

        virtual ~MultilineInput() { };

        virtual int handle( int e );
        virtual void draw();
    };

} // namespace mrv
