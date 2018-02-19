#ifndef FL_gl_H
#define FL_gl_H

#include <fltk/gl.h>

#define gl_start	fltk::glstart
#define	gl_finish	fltk::glfinish
#define gl_color	fltk::glsetcolor
#define	gl_rect		fltk::glstrokerect
#define gl_rectf	fltk::glfillrect
#define gl_font		fltk::glsetfont
inline float gl_height() {return fltk::glgetascent()+fltk::glgetdescent();}
#define gl_descent	fltk::glgetdescent
#define gl_width	fltk::glgetwidth
#define gl_draw		fltk::gldrawtext
//void gl_measure(const char*, int& x, int& y);
#define gl_draw_image	fltk::gldrawimage

#endif // !FL_gl_H
