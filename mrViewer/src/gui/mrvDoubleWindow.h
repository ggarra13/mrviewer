
#ifndef mrvDoubleWindow_h
#define mrvDoubleWindow_h

#include "FL/Fl_Double_Window.H"

class mrvDoubleWindow : public Fl_Double_Window
{
  public:
    mrvDoubleWindow( int x, int y, int w, int h, const char* l = "" );
    mrvDoubleWindow( int w, int h, const char* l = "" );

    bool exec();

    void make_exec_return( bool b ) { _exec = b; }

  protected:
    bool _exec;
};


#endif // mrvDoubleWindow_h
