/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// generated by Fast Light User Interface Designer (fluid) version 2.1000

#include <string.h>
#include <iostream>

#include "FL/Fl_Window.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Box.H"
#include "FL/Fl_Browser.H"
#include "FL/Fl_Text_Editor.H"
#include "FL/Enumerations.H"
#include "FL/fl_draw.H"
#include "FL/Fl.H"

#include "gui/mrvFontsWindowUI.h"

class FontDisplay : public Fl_Text_Editor {
public:
    FontDisplay(Fl_Boxtype B, int X, int Y, int W, int H, const char* L = 0) :
    Fl_Text_Editor(X,Y,W,H,L) {
        box(B);
    buffer( new Fl_Text_Buffer() );
    }
};


namespace mrv {

  Fl_Font  font_current = (Fl_Font) 0;
  unsigned    font_size = 32;
  std::string font_text = "Type here";

}


Fl_Double_Window *uiMain=(Fl_Double_Window *)0;
Fl_Value_Slider* uiFontSize = NULL;
FontDisplay *uiText=(FontDisplay *)0;
Fl_Browser * encobj;
Fl_Box* id_box;

void encoding_cb(Fl_Widget *, long) {
    int i = encobj->value();
    if (i < 0)
        return;
    // uiText->encoding = encobj->child(i)->label();
    uiText->redraw();
}



void new_text( Fl_Widget* w, void* data )
{
    Fl_Text_Buffer* buf = uiText->buffer();
    mrv::font_text = buf->text();
}

void new_font( Fl_Widget* w, void* data )
{
    Fl_Choice* c = (Fl_Choice*) w;
    int i = c->value();
    if ( i < 0 ) i = 0;

    Fl_Text_Buffer* buf = uiText->buffer();
    mrv::font_text = buf->text();
    mrv::font_current = (Fl_Font) i;
    uiText->textfont( (Fl_Font) i );
    uiText->redraw();
}

void new_size( Fl_Widget* w, void* data )
{
    Fl_Value_Slider* s = (Fl_Value_Slider*) w;
    mrv::font_size = s->value();
    uiText->textsize( s->value() );
    uiText->redraw();
}

static void cb_Accept(Fl_Button*, Fl_Window* v) {
    v->activate();
    v->hide();
}

static void cb_Cancel(Fl_Button*, Fl_Window* v) {
    v->deactivate();
    v->hide();
}


namespace mrv {

bool make_window() {
    Fl_Double_Window* w;
    {   Fl_Double_Window* o = uiMain = new Fl_Double_Window(405, 260);
        w = o;
        o->type(241);
        o->begin();
        {   Fl_Group* o = new Fl_Group(5, 5, 405, 235);
            o->begin();

            uiText = new FontDisplay(FL_ENGRAVED_BOX, 0, 0, 400, 100);
            {   Fl_Choice* o = new Fl_Choice(40, 120, 360, 25, "Font");
                int numfonts = Fl::set_fonts("-*");
                for (int i = 0; i < numfonts; ++i)
        {
            int t; // bold/italic flags
                    o->add(Fl::get_font_name( (Fl_Font)i, &t ) );
        }
        o->labelcolor( FL_BLACK );
        o->value(mrv::font_current);
                uiText->textfont( mrv::font_current );
                uiText->buffer()->text( mrv::font_text.c_str() );
                uiText->textsize( mrv::font_size );
                uiText->callback( new_text );
        uiText->textcolor( FL_BLACK );
                o->callback( new_font );
            }
            {   Fl_Value_Slider* o = uiFontSize = new Fl_Value_Slider(70, 160, 325, 25, "Font Size");
        o->type( FL_HORIZONTAL );
                o->minimum(8);
                o->maximum(100);
                o->step(1);
                o->value(mrv::font_size);
        o->labelcolor( FL_BLACK );
                o->align(FL_ALIGN_LEFT);
                o->callback( new_size );
            }
            o->end();

            {   Fl_Group* g = new Fl_Group( 0, 200, 205, 55 );
                g->begin();
                {   Fl_Button* o = new Fl_Button( 75, 200, 50, 25, "OK" );
                    o->callback((Fl_Callback*)cb_Accept, (void*)(w));
                }
                {   Fl_Button* o = new Fl_Button( 150, 200, 50, 25, "Cancel" );
                    o->callback((Fl_Callback*)cb_Cancel, (void*)(w));
                }
                g->end();
            }

            Fl_Group::current()->resizable(o);
        }

        o->end();
        o->set_modal();
    }

    w->show();
    while ( w->visible() )
    Fl::check();
    if ( ! w->active() ) {
    delete w;
    return false;
    }
    delete w;
    return true;
}

} // namespace mrv
