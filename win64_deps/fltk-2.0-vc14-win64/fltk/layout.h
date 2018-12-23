#ifndef fltk_layout_h
#define fltk_layout_h

namespace fltk {

/*!
  Values of the bits stored in Widget::layout_damage().

  When a widget resized or moved (or when it is initially created),
  flags are set in Widget::layout_damage() to indicate the layout is
  damaged. This will cause the virtual function Widget::layout() to be
  called just before fltk attempts to draw the windows on the screen.
  This is useful because often calculating the new layout is quite
  expensive, this expense is now deferred until the user will actually
  see the new size.

  Some Group widgets such as fltk::PackedGroup will also use the
  virtual Widget::layout() function to find out how big a widget
  should be.  A Widget is allowed to change it's own dimensions in
  layout() (except it is not allowed to change it if called a second
  time with no changes other than it's x/y position). This allows
  widgets to resize to fit their contents.

  The layout bits are turned on by calling Widget::relayout().
*/
enum {
  LAYOUT_X	= 0x01, /*!< Widget::x() changed by resize() */
  LAYOUT_Y	= 0x02, /*!< Widget::y() changed by resize() */
  LAYOUT_XY	= 0x03, /*!< Same as LAYOUT_X|LAYOUT_Y */
  LAYOUT_W	= 0x04,	/*!< Widget::w() changed by resize() */
  LAYOUT_H	= 0x08,	/*!< Widget::h() changed by resize() */
  LAYOUT_WH	= 0x0C, /*!< Same as LAYOUT_W|LAYOUT_H */
  LAYOUT_XYWH	= 0x0F, /*!< Same as LAYOUT_XY|LAYOUT_WH */
  LAYOUT_CHILD	= 0x10, /*!< Widget::layout() needs to be called on a child of this group widget. */
  LAYOUT_USER   = 0x20, /*!< The moving/resizing is being caused by the user and not internal code. */
  LAYOUT_DAMAGE	= 0x80	/*!< Widget::relayout() was called. */
};

}

#endif
