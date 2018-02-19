#include "WidgetClassType.h"
#include "PrefsData.h"
#include <fltk/Window.h>

using namespace fltk;

WidgetClassType Widget_Class_type;
WidgetClassType *current_widget_class = 0;

FluidType *WidgetClassType::make() {
  FluidType *p = FluidType::current;
  while (p && !p->is_decl_block()) p = p->parent;
  WidgetClassType *myo = new WidgetClassType();
  myo->name("UserInterface");

  if (!this->o) {// template widget
    this->o = new Window(100,100);
    Group::current(0);
  }
  // Set the size ranges for this window; in order to avoid opening the
  // X display we use an arbitrary maximum size...
  ((Window *)(this->o))->size_range(prefs.gridx(), prefs.gridy(),
      3072, 2048,prefs.gridx(), prefs.gridy());
  myo->factory = this;
  myo->drag = 0;
  myo->numselected = 0;
  myo->make_fltk_window();
  myo->add(p);
  myo->modal = 0;
  myo->non_modal = 0;
  myo->wc_relative = 0;

  return myo;
}

void WidgetClassType::write_properties() {
  WindowType::write_properties();
  if (wc_relative) write_string("position_relative");
}

void WidgetClassType::read_property(const char *c) {
  if (!strcmp(c,"position_relative")) {
    wc_relative = 1;
  } else {
    WindowType::read_property(c);
  }
}

void WidgetClassType::write_code() {
#if 0
  WidgetType::write_code1();
#endif // 0

  current_widget_class = this;
  write_public_state = 1;

  const char *c = subclass();
  if (!c) c = "Group";

  write_h("\nclass %s : public %s {\n", name(), c);
  if (!strcmp(c, "Window") ||
      !strcmp(c, "Fl_Double_Window") ||
      !strcmp(c, "Fl_Gl_Window") ||
      !strcmp(c, "Fl_Overlay_Window")) {
    write_h("  void _%s();\n", name());
    write_h("public:\n");
    write_h("  %s(int X, int Y, int W, int H, const char *L = 0);\n", name());
    write_h("  %s(int W, int H, const char *L = 0);\n", name());

    write_c("%s::%s(int X, int Y, int W, int H, const char *L)\n", name(), name());
    write_c("  : %s(X, Y, W, H, L) {\n", c);
    write_c("  _%s();\n", name());
    write_c("}\n\n");

    write_c("%s::%s(int W, int H, const char *L)\n", name(), name());
    write_c("  : %s(0, 0, W, H, L) {\n", c);
    write_c("  clear_flag(16);\n");
    write_c("  _%s();\n", name());
    write_c("}\n\n");

    write_c("void %s::_%s() {\n", name(), name());
    write_c("  %s *w = this;\n", name());
  } else {
    write_h("public:\n");
    write_h("  %s(int X, int Y, int W, int H, const char *L = 0);\n", name());

    write_c("%s::%s(int X, int Y, int W, int H, const char *L)\n", name(), name());
    if (wc_relative)
      write_c("  : %s(0, 0, W, H, L) {\n", c);
    else
      write_c("  : %s(X, Y, W, H, L) {\n", c);
  }

  write_c("  %s *o = this;\n", name());

  write_widget_code();
}

void WidgetClassType::write_code1() {
  write_extra_code();
  if (wc_relative) write_c("%sposition(X, Y);\n", indent());
  if (modal) write_c("%sset_modal();\n", indent());
  else if (non_modal) write_c("%sset_non_modal();\n", indent());
  if (!((Window*)o)->border()) write_c("%sclear_border();\n", indent());
  if (user_class()) {
    write_c("%sclass(", indent());
    write_cstring(user_class());
    write_c(");\n");
  }
  write_c("%send();\n", indent());
  if (((Window*)o)->resizable() == o)
    write_c("%sresizable(this);\n", indent());
  write_c("}\n");
}


