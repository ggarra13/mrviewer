#ifndef WidgetType_h
#define WidgetType_h

#include "FluidType.h"

struct Enumeration;

class FLUID_API WidgetType : public FluidType {
  virtual fltk::Widget *widget(int,int,int,int);
  virtual WidgetType *_make(); // virtual constructor
  virtual void setlabel(const char *);

  const char *extra_code_;
  const char *user_class_;
  bool hotspot_;

protected:

  void write_static();
  void write_code();
  void write_code1();
  void write_widget_code();
  void write_extra_code();
  void write_block_close();

public:

  bool set_xy;
  fltk::Widget *o;
  bool public_;
  
  Fluid_Image* image;
  void set_image(Fluid_Image* i);

  WidgetType();
  FluidType *make();
  void open();

  const char *extra_code() const {return extra_code_;}
  void extra_code(const char *);
  const char *user_class() const {return user_class_;}
  void user_class(const char *);
  bool hotspot() const {return hotspot_;}
  void hotspot(bool v) {hotspot_ = v;}
  bool resizable() const;
  void resizable(bool v);

  virtual const Enumeration* subtypes() const;

  virtual int is_widget() const;

  virtual void write_properties();
  virtual void read_property(const char *);
  virtual int read_fdesign(const char*, const char*);

  virtual ~WidgetType();
  void redraw();

  const char* subclass() const;
  const char* array_name() const;

  virtual const char *type_name() const;

  // live mode functionalities
  fltk::Widget *enter_live_mode(int top);
  void leave_live_mode();
  void copy_properties();
  
  virtual int textstuff(int w, fltk::Font* f, float& s, fltk::Color c);

  static float default_size;
  int pixmapID() { return 5;}
  
};

#define LOAD ((void*)9831)

FLUID_API extern WidgetType *current_widget; // one of the selected ones

#endif
