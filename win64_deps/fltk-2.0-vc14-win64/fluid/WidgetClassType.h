// Represents a user-defined subclass of fltk::Window or Group

#ifndef WidgetClassType_h
#define WidgetClassType_h

#include "WindowType.h"

class FLUID_API WidgetClassType : private WindowType {
public:
  WidgetClassType() {write_public_state = false;  wc_relative = false;}
  // state variables for output:
  bool write_public_state; // true when public: has been printed
  bool wc_relative; // if true, reposition all child widgets in an Fl_Group

  virtual void write_properties();
  virtual void read_property(const char *);

  void write_code();
  void write_code1();
  FluidType *make();
  virtual const char *type_name() const  {return "fltk::WidgetClass";}
  int is_parent() const {return 1;}
  int is_decl_block() const {return 1;}
  int is_class() const {return 1;}
  int pixmapID() { return 48; }
};

#endif
