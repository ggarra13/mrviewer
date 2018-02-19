// Represents an fltk::Window. Also lots of methods for the fluid ui
// for selecting and moving widgets.

#ifndef WindowType_h
#define WindowType_h

#include "GroupType.h"

class FLUID_API WindowType : public GroupType {
  const Enumeration* subtypes() const;

  friend class Overlay_Window;
  int mx,my;		// mouse position during dragging
  int x1,y1;		// initial position of selection box
  int bx,by,br,bt;	// bounding box of selection
  int dx,dy;
  int recalc;		// set by fix_overlay()
public:
  int drag;		// which parts of bbox are being moved
  int numselected;	// number of children selected

  enum {LEFT=1,RIGHT=2,BOTTOM=4,TOP=8,DRAG=16,BOX=32};
  void draw_overlay();
  void newdx();
  void newposition(WidgetType *,int &x,int &y,int &w,int &h);
  int handle(int);
  virtual void setlabel(const char *);
  void write_code();
  WidgetType *_make() {return 0;} // we don't call this
  fltk::Widget *widget(int,int,int,int) {return 0;}
  void moveallchildren();
  void move_children(FluidType*, int);
  WidgetType* clicked_widget();

protected:
  void make_fltk_window();

public:
  WindowType();

  bool modal, non_modal, border;

  FluidType *make();
  virtual const char *type_name() const;

  void open();

  void fix_overlay();	// update the bounding box, etc

  virtual void write_properties();
  virtual void read_property(const char *);
  virtual int read_fdesign(const char*, const char*);

  void add_child(FluidType*, FluidType*);
  void move_child(FluidType*, FluidType*);
  void remove_child(FluidType*);

  int is_parent() const {return 1;}
  int is_group() const {return 1;}
  int is_window() const {return 1;}

  // live mode functionalities
  fltk::Widget *enter_live_mode(int top);
  void leave_live_mode();
  void copy_properties();
  
  int sr_min_w, sr_min_h, sr_max_w, sr_max_h; // size_range related

  int pixmapID() { return 1; }
};

#endif
