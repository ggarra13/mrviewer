// Represents an fltk::Group widget

#ifndef GroupType_h
#define GroupType_h

#include "WidgetType.h"

class FLUID_API GroupType : public WidgetType {
public:
  const Enumeration* subtypes() const;
  virtual ~GroupType();
  virtual const char *type_name() const;
  fltk::Widget *widget(int x,int y,int w,int h);
  WidgetType* _make();
  FluidType *make();
  void write_code();
  void add_child(FluidType*, FluidType*);
  void move_child(FluidType*, FluidType*);
  void remove_child(FluidType*);
  int is_parent() const;
  int is_group() const;

  // live mode functionalities
  fltk::Widget *enter_live_mode(int top);
  void leave_live_mode();
  void copy_properties();
  int pixmapID() { return 6; }
};

#endif
