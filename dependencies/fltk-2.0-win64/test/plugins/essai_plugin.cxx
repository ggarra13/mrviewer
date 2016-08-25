#include <stdio.h>
#include <string.h>
#include <FL/Fl_Menu.H>
#include <FL/Fl_Window.H>
#include "Fluid_Plugins.h"
#include "Fl_Type.h"
#include "Fl_Pix_Button.cxx"

#include "play_buttons256.xpm"
#include "pause_buttons256.xpm"

#define S2I(a) ( int(a[0])+(int(a[1])<<8)+(int(a[2])<<16)+(int(a[3])<<24) )

// Description of the plugin : declared extern "C" to avoid C++ name encoding
// (MS Visual C does encode C++ name of variables and not only functions !! Wonder why ...)
extern "C" FLUID_PLUGIN_API Fluid_Plugin fluid_plugin;


void option_cb(Fl_Widget* o, void* p)
{
  printf("plugin option callback called !\n");
}

// Options menu : you can put whatever you want here
Fl_Menu_Item options_menu[] = {
{"Options",0,option_cb,0,0},
{ 0 }
};

#include <FL/Fl_Button.H>


//////////////////////////////////////////////////////////////////////
// Description of a new widget for fluid

// This object contains all the informations about the widgets and all methods
// to write and read datas from the .fl file
// (in this example we do not override the write and read methods !)
// (see below for a more complete example)

static Fl_Menu_Item buttontype_menu[] = {
  {"Normal",0,0,(void*)0},
  {"Toggle",0,0,(void*)FL_TOGGLE_BUTTON},
  {"Radio",0,0,(void*)FL_RADIO_BUTTON},
  {0}};

class Fl_Pix_Button_Type : public Fl_Widget_Type {
  // Return a pointer on the button type menu
  Fl_Menu_Item *subtypes() {return buttontype_menu;}
public:
  // Give the name of the widget type
  virtual const char *type_name() {return "Fl_Pix_Button";}

  // Actually create a widget of this type to be drawn in fluid
  // Set a default label name describing the widget type shortly
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Pix_Button(x,y,w,h,"pixbutton", new Fl_Pixmap(play_buttons256)); }

  // Create a new fluid object of this type
  Fl_Widget_Type *_make() {return new Fl_Pix_Button_Type();}

  // Give informations on this type
  int is_button() const {return 1;} // (all other is_foo methods reuturn 0 by default
};
static Fl_Pix_Button_Type Fl_Pix_Button_type; // template pixbutton



///////////////////////////////////////////////////////
// Another widget : the NSlider
// Here we override the write and read methods
#include "Fl_NSlider.cxx"
static Fl_Menu_Item slider_type_menu[] = { // Type menu definition
  {"Vertical",0,0,(void*)FL_VERT_SLIDER},
  {"Horizontal",0,0,(void*)FL_HOR_SLIDER},
  {"Vert Fill",0,0,(void*)FL_VERT_FILL_SLIDER},
  {"Horz Fill",0,0,(void*)FL_HOR_FILL_SLIDER},
  {"Vert Knob",0,0,(void*)FL_VERT_NICE_SLIDER},
  {"Horz Knob",0,0,(void*)FL_HOR_NICE_SLIDER},
  {"Vert Half",0,0,(void*)FL_VERT_HALF_SLIDER},
  {"Horz half",0,0,(void*)FL_HOR_HALF_SLIDER},
  {0}};
class Fl_NSlider_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return slider_type_menu;}
  // We use this kludge to identify that this is really an NSlider and not any valuator,
  // could there be any better way ?
  int is_valuator() const {return S2I("NSLD");}   
public:
  virtual const char *type_name() {return "Fl_NSlider";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_NSlider(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_NSlider_Type();}

  void write_properties() {
    Fl_Widget_Type::write_properties();
    Fl_NSlider* i = (Fl_NSlider*) o;
    if(i->majorTickPrecision()) {
      write_indent(level+1);
      write_string("majorTickPrecision %d", i->majorTickPrecision());
    }
  }
  void read_property(const char* c) {
    if(!strcmp(c, "majorTickPrecision")) {
      Fl_NSlider* i = (Fl_NSlider*) o;
      int mtp;
      if(sscanf(read_word(), "%d", &mtp) == 1) i->majorTickPrecision(mtp);
    } else
      Fl_Widget_Type::read_property(c);
  }
  void write_widget_code() {
    Fl_Widget_Type::write_widget_code();
    Fl_NSlider* i = (Fl_NSlider*) o;
    if(i->majorTickPrecision()) write_c("%so->majorTickPrecision(%d);\n", indent(), 
					i->majorTickPrecision());
  }
};
static Fl_NSlider_Type Fl_NSlider_type;



// Submenu in the new menu : only pointers on the the template objects is needed,
// fluid will fill in the rest (name, callback action, etc ...)
Fl_Menu_Item new_menu[] = {
  {0,0,0,(void*)&Fl_Pix_Button_type},
  {0,0,0,(void*)&Fl_NSlider_type},
{ 0 }
};



// Callback for the panel settings
// fluid set the user_data parameter to let us know if we have to set or load the settings
class Fl_Slider;
void major_tick_precision_cb(Fl_Slider* o, void* v)
{
  Fl_Valuator* i = (Fl_Valuator*) o;
  if (v == LOAD) { // Case LOAD
    if (current_widget->is_valuator() != S2I("NSLD")) // Is it a NSlider widget ?
      {i->hide(); return;}                       // no, we hide the controller
    else { 
      fluid_plugin.please_show_panel=1; // yes : tell to fluid that we use the panel
      i->show();                        // and we show the controller
    } 

    // Actually load the value in the controller
    i->value(((Fl_NSlider*)current_widget->o)->majorTickPrecision()); 

  } else {         // Case SET

    for (Fl_Type *o = Fl_Type::first; o; o = o->next)
      if (o->selected && o->is_widget() && o->is_valuator() == S2I("NSLD")) { // is it a NSlider
	Fl_Widget_Type* q = (Fl_Widget_Type*)o;
	((Fl_NSlider*)q->o)->majorTickPrecision(i->value()); // yep : set the widget setting
	q->o->redraw();
      }
  }
}



// This function create the panel for the specific settings of this set of widgets
#include "essai_panel.cxx"
void make_panel()
{
  fluid_plugin.panel = make_window();
}




// This is the description of the plugin
FLUID_PLUGIN_API Fluid_Plugin fluid_plugin = {
  "Essai",       // Name of the plugin
  options_menu,  // pointer on the 'option' menu
  new_menu,      // pointer on the 'new' menu
  make_panel,    // function that creates the settings window
};
