#ifndef fltk_damage_h
#define fltk_damage_h

namespace fltk {

/*!
  Values of the bits stored in Widget::damage().

  When redrawing your widgets you should look at the damage bits to
  see what parts of your widget need redrawing. The Widget::handle()
  method can then set individual damage bits to limit the amount of
  drawing that needs to be done, and the Widget::draw() method can
  test these bits to decide what to draw:

\code
MyClass::handle(int event) {
  ...
  if (change_to_part1) damage(1);
  if (change_to_part2) damage(2);
  if (change_to_part3) damage(4);
}

MyClass::draw() {
  if (damage() & fltk::DAMAGE_ALL) {
    ... draw frame/box and other static stuff ...
  }
  if (damage() & (fltk::DAMAGE_ALL | 1)) draw_part1();
  if (damage() & (fltk::DAMAGE_ALL | 2)) draw_part2();
  if (damage() & (fltk::DAMAGE_ALL | 4)) draw_part3();
}
\endcode

  Except for DAMAGE_ALL, each widget is allowed to assign any meaning
  to any of the bits it wants. The enumerations are just to provide
  suggested meanings.
*/
enum {
  DAMAGE_VALUE		= 0x01,
  DAMAGE_PUSHED		= 0x02,
  DAMAGE_SCROLL		= 0x04,
  DAMAGE_OVERLAY	= 0x04, // reused value
  DAMAGE_HIGHLIGHT	= 0x08,
  DAMAGE_CHILD		= 0x10,
  DAMAGE_CHILD_LABEL	= 0x20,
  DAMAGE_EXPOSE		= 0x40,
  DAMAGE_CONTENTS	= 0x40, // reused value
  DAMAGE_ALL		= 0x80
};

}

#endif
