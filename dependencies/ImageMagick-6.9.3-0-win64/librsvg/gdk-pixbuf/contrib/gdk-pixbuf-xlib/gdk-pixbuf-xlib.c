/* GdkPixbuf library - Initialization functions
 *
 * Author: John Harper <john@dcs.warwick.ac.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <X11/Xlib.h>
#include "gdk-pixbuf-xlib-private.h"

/**
 * SECTION:gdk-pixbuf-xlib-init
 * @Short_description: Initializing the gdk-pixbuf Xlib library.
 * @Title: gdk-pixbuf Xlib initialization
 * @See_also:    XlibRGB
 * 
 * In addition to the normal Gdk-specific functions, the &gdk-pixbuf;
 * package provides a small library that lets Xlib-only applications
 * use #GdkPixbuf structures and render them to X drawables.  The
 * functions in this section are used to initialize the &gdk-pixbuf;
 * Xlib library.  This library must be initialized near the beginning
 * of the program or before calling any of the other &gdk-pixbuf;
 * Xlib functions.
 */

Display *gdk_pixbuf_dpy = NULL;
int gdk_pixbuf_screen = -1;

/**
 * gdk_pixbuf_xlib_init:
 * @display: X display to use.
 * @screen_num: Screen number.
 * 
 * Initializes the gdk-pixbuf Xlib machinery by calling xlib_rgb_init().  This
 * function should be called near the beginning of your program, or before using
 * any of the gdk-pixbuf-xlib functions.
 **/
void
gdk_pixbuf_xlib_init (Display *display, int screen_num)
{
    xlib_rgb_init (display, ScreenOfDisplay (display, screen_num));
    gdk_pixbuf_dpy = display;
    gdk_pixbuf_screen = screen_num;
}

/**
 * gdk_pixbuf_xlib_init_with_depth:
 * @display: X display to use.
 * @screen_num: Screen number.
 * @prefDepth: Preferred depth for XlibRGB.
 * 
 * Similar to gdk_pixbuf_xlib_init(), but also lets you specify the preferred
 * depth for XlibRGB if you do not want it to use the default depth it picks.
 **/
void
gdk_pixbuf_xlib_init_with_depth (Display *display,
				 int screen_num, int prefDepth)
{
    xlib_rgb_init_with_depth (display, ScreenOfDisplay (display, screen_num),
			      prefDepth);
    gdk_pixbuf_dpy = display;
    gdk_pixbuf_screen = screen_num;
}
