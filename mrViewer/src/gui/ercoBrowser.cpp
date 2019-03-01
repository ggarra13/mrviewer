
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_draw.H>
//
// Demonstrate how to derive a class extending Fl_Browser with interactively resizable columns
// erco 1.10 12/09/2005
// erco 1.20 07/17/2016 -- fix scrollbar recalc, code simplifications
//
class ColResizeBrowser : public Fl_Browser {
    Fl_Color  _colsepcolor;     // color of column separator lines 
    int       _showcolsep;      // flag to enable drawing column separators
    Fl_Cursor _last_cursor;     // saved cursor state info
    int       _drag_col;        // col# user is dragging (-1 = not dragging)
    int      *_widths;          // pointer to user's width[] array
    int       _nowidths[1];     // default width array (non-const)
    // CHANGE CURSOR
    //     Does nothing if cursor already set to value specified.
    //
    void change_cursor(Fl_Cursor newcursor) {
        if ( newcursor == _last_cursor ) return;
        window()->cursor(newcursor);
        _last_cursor = newcursor;
    }
    // RETURN THE COLUMN MOUSE IS 'NEAR'
    //     Returns -1 if none.
    //
    int which_col_near_mouse() {
        int X,Y,W,H;
        Fl_Browser::bbox(X,Y,W,H);            // area inside browser's box()
        // EVENT NOT INSIDE BROWSER AREA? (eg. on a scrollbar)
        if ( ! Fl::event_inside(X,Y,W,H) ) {
            return(-1);
        }
        int mousex = Fl::event_x() + hposition();
        int colx = this->x();
        for ( int t=0; _widths[t]; t++ ) {
            colx += _widths[t];
            int diff = mousex - colx;
            // MOUSE 'NEAR' A COLUMN?
            //     Return column #
            //
            if ( diff >= -4 && diff <= 4 ) {
                return(t);
            }
        }
        return(-1);
    }
    // FORCE SCROLLBAR RECALC
    //    Prevents scrollbar from getting out of sync during column drags
    void recalc_hscroll() {
      int size = textsize();
      textsize(size+1);     // XXX: changing textsize() briefly triggers
      textsize(size);       // XXX: recalc Fl_Browser's scrollbars
      redraw();
    }
protected:
    // MANAGE EVENTS TO HANDLE COLUMN RESIZING
    int handle(int e) {
        // Not showing column separators? Use default Fl_Browser::handle() logic
        if ( !showcolsep() ) return(Fl_Browser::handle(e));
        // Handle column resizing
        int ret = 0;
        switch ( e ) {
            case FL_ENTER: {
                ret = 1;
                break;
            }
            case FL_MOVE: {
                change_cursor( (which_col_near_mouse() >= 0) ? FL_CURSOR_WE
                                                             : FL_CURSOR_DEFAULT);
                ret = 1;
                break;
            }
            case FL_PUSH: {
                int whichcol = which_col_near_mouse();
                if ( whichcol >= 0 ) {
                    // CLICKED ON RESIZER? START DRAGGING
                    _drag_col = whichcol;
                    change_cursor(FL_CURSOR_DEFAULT);
                    return 1;   // eclipse event from Fl_Browser's handle()
                }               // (prevents FL_PUSH from selecting item)
                break;
            }
            case FL_DRAG: {
                if ( _drag_col != -1 ) {
                    // Sum up column widths to determine position
                    int mousex = Fl::event_x() + hposition();
                    int newwidth = mousex - x();
                    for ( int t=0; _widths[t] && t<_drag_col; t++ ) {
                        newwidth -= _widths[t];
                    }
                    if ( newwidth > 0 ) {
                        // Apply new width, redraw interface
                        _widths[_drag_col] = newwidth;
                        if ( _widths[_drag_col] < 2 ) {
                            _widths[_drag_col] = 2;
                        }
                        recalc_hscroll();
                        redraw();
                    }
                    return 1;   // eclipse event from Fl_Browser's handle()
                }
                break;
            }
            case FL_LEAVE:
            case FL_RELEASE: {
                _drag_col = -1;                         // disable drag mode
                change_cursor(FL_CURSOR_DEFAULT);       // ensure normal cursor
                if ( e == FL_RELEASE ) return 1;        // eclipse event
                ret = 1;
                break;
            }
        }
        return(Fl_Browser::handle(e) ? 1 : ret);
    }
    void draw() {
        // DRAW BROWSER
        Fl_Browser::draw();
        if ( _showcolsep ) {
            // DRAW COLUMN SEPARATORS
            int colx = this->x() - hposition();
            int X,Y,W,H;
            Fl_Browser::bbox(X,Y,W,H);
            fl_color(_colsepcolor);
            for ( int t=0; _widths[t]; t++ ) {
                colx += _widths[t];
                if ( colx > X && colx < (X+W) ) {
                    fl_line(colx, Y, colx, Y+H-1);
                }
            }
        }
    }
public:
    // CTOR
    ColResizeBrowser(int X,int Y,int W,int H,const char*L=0) : Fl_Browser(X,Y,W,H,L) {
        _colsepcolor = Fl_Color(FL_GRAY);
        _last_cursor = FL_CURSOR_DEFAULT;
        _showcolsep  = 0;
        _drag_col    = -1;
        _nowidths[0] = 0;
        _widths      = _nowidths;
    }
    // GET/SET COLUMN SEPARATOR LINE COLOR
    Fl_Color colsepcolor() const {
        return(_colsepcolor);
    }
    void colsepcolor(Fl_Color val) {
        _colsepcolor = val;
    }
    // GET/SET DISPLAY OF COLUMN SEPARATOR LINES
    //     1: show lines, 0: don't show lines
    //
    int showcolsep() const {
        return(_showcolsep);
    }
    void showcolsep(int val) {
        _showcolsep = val;
    }
    // GET/SET COLUMN WIDTHS ARRAY
    //    Just like fltk method, but array is non-const.
    //
    int *column_widths() const {
        return(_widths);
    }
    void column_widths(int *val) {
        _widths = val;
        Fl_Browser::column_widths(val);
    }
};
int main() {
    Fl_Double_Window  *w = new Fl_Double_Window(900,300);
    int widths[] = { 50, 50, 50, 70, 70, 40, 40, 70, 70, 50, 0 };               // widths for each column
    ColResizeBrowser *b = new ColResizeBrowser(10,10,w->w()-20,w->h()-20);
    b->column_widths(widths);
    b->showcolsep(1);
    //b->colsepcolor(FL_RED);
    b->column_char('\t');                                                       // tabs as column delimiters
    b->type(FL_MULTI_BROWSER);

    //// SIMPLE UN-COLORED HEADING
    ////  b->add("USER\tPID\t%CPU\t%MEM\tVSZ\tRSS\tTTY\tSTAT\tSTART\tTIME\tCOMMAND"); 

    // NICER COLORED HEADING
    b->add("@B12@C7@b@.USER\t@B12@C7@b@.PID\t@B12@C7@b@.%CPU\t"                 // tab delimited columns with colors
           "@B12@C7@b@.%MEM\t@B12@C7@b@.VSZ\t@B12@C7@b@.RSS\t"
           "@B12@C7@b@.TTY\t@B12@C7@b@.STAT\t@B12@C7@b@.START\t"
           "@B12@C7@b@.TIME\t@B12@C7@b@.COMMAND");

    // COLUMNS OF DATA
    b->add("root\t2888\t0.0\t0.0\t1352\t0\ttty3\tSW\tAug15\t0:00\t@b@f/sbin/mingetty tty3");
    b->add("erco\t2889\t0.0\t13.0\t221352\t0\ttty3\tR\tAug15\t1:34\t@b@f/usr/local/bin/render a35 0004");
    b->add("uucp\t2892\t0.0\t0.0\t1352\t0\tttyS0\tSW\tAug15\t0:00\t@b@f/sbin/agetty -h 19200 ttyS0 vt100");
    b->add("root\t13115\t0.0\t0.0\t1352\t0\ttty2\tSW\tAug30\t0:00\t@b@f/sbin/mingetty tty2");
    b->add("root\t13464\t0.0\t0.0\t1352\t0\ttty1\tSW\tAug30\t0:00\t@b@f/sbin/mingetty tty1 --noclear");
    w->resizable(b);
    w->end();
    w->show();
    return(Fl::run());
}
