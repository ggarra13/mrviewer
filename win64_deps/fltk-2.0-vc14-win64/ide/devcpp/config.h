/*
 * "$Id: config.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
 *
 * Configuration file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2006 by Bill Spitzak and others.
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems on the following page:
 *
 *    http://www.fltk.org/str.php
 */

#include <stdarg.h>
#include <sys/types.h>
#include <stddef.h>

/*
 * BORDER_WIDTH:
 *
 * Thickness of FL_UP_BOX and FL_DOWN_BOX.  Current 1,2, and 3 are
 * supported.
 *
 * 3 is the historic FLTK look.
 * 2 is the default and looks like Microsoft Windows, KDE, and Qt.
 * 1 is a plausible future evolution...
 *
 * Note that this may be simulated at runtime by redefining the boxtypes
 * using Fl::set_boxtype().
 */

#define BORDER_WIDTH 3

/*
 * HAVE_GL:
 *
 * Do you have OpenGL? Set this to 0 if you don't have or plan to use
 * OpenGL, and FLTK will be smaller.
 */

#define HAVE_GL 1

/*
 * USE_COLORMAP:
 *
 * Setting this to zero will save a good deal of code (especially for
 * fl_draw_image), but FLTK will only work on TrueColor visuals.
 */

#define USE_COLORMAP 0

/*
 * HAVE_XDBE:
 *
 * Do we have the X double-buffer extension?
 */

#define HAVE_XDBE 0

/*
 * USE_XDBE:
 *
 * Actually try to use the double-buffer extension?  Set this to zero
 * disable use of XDBE without breaking the list_visuals program.
 */

#define USE_XDBE HAVE_XDBE

/*
 * HAVE_OVERLAY:
 *
 * Use the X overlay extension?  FLTK will try to use an overlay
 * visual for Fl_Overlay_Window, the Gl_Window overlay, and for the
 * menus.  Setting this to zero will remove a substantial amount of
 * code from FLTK.  Overlays have only been tested on SGI servers!
 */

#define HAVE_OVERLAY 0

/*
 * HAVE_GL_OVERLAY:
 *
 * It is possible your GL has an overlay even if X does not.  If so,
 * set this to 1.
 */

#define HAVE_GL_OVERLAY HAVE_OVERLAY

/*
 * WORDS_BIGENDIAN:
 *
 * Byte order of your machine: 1 = big-endian, 0 = little-endian.
 */

#define WORDS_BIGENDIAN 0

/*
 * U16, U32, U64:
 *
 * Types used by fl_draw_image.  One of U32 or U64 must be defined.
 * U16 is optional but FLTK will work better with it!
 */

#define U16 unsigned short
#define U32 unsigned
/* #undef U64 */

/*
 * HAVE_DIRENT_H, HAVE_SYS_NDIR_H, HAVE_SYS_DIR_H, HAVE_NDIR_H, HAVE_SCANDIR:
 *
 * Where is <dirent.h> (used only by fl_file_chooser and scandir).
 */

#define HAVE_DIRENT_H 1
#define HAVE_SYS_NDIR_H 0
#define HAVE_SYS_DIR_H 0
#define HAVE_NDIR_H 0
#define HAVE_SCANDIR 0


#if defined(_MSC_VER)
#	define HAVE_VSNPRINTF 1
#	define HAVE_SNPRINTF 1
#	define HAVE_VSPRINTF 1
#	include <stdio.h>
#	define fl_snprintf _snprintf
#	define fl_vsnprintf _vsnprintf
#	define snprintf _snprintf
#	define vsnprintf _vsnprintf
#else

/*
 * possibly missing sprintf-style functions:
 */
#define HAVE_VSNPRINTF 0
#define HAVE_SNPRINTF 0
#define HAVE_VSPRINTF 1
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int snprintf(char* str, size_t size, const char* fmt, ...);
extern int vsnprintf(char* str, size_t size, const char* fmt, va_list ap);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#define fl_snprintf snprintf
#define fl_vsnprintf vsnprintf

#endif

/*
 * HAVE_SYS_SELECT_H:
 *
 * Whether or not select() call has its own header file.
 */

#define HAVE_SYS_SELECT_H 0

/*
 * HAVE_POLL:
 *
 * Use poll() if we don't have select().
 */

#define HAVE_POLL 0

/*
 * HAVE_PNG
 *
 * Do we have libpng ?
 *
 */
 
#define HAVE_LIBPNG 1
#define HAVE_LIBZ 1
#define HAVE_LOCAL_PNG_H 1

/*
 * HAVE_JPEG
 *
 * Do we have libjpeg ?
 *
 */
  
#define HAVE_LIBJPEG 1
#define HAVE_LOCAL_JPEG_H 1

/*
 * Do we use exceptions?
 */
#define HAVE_EXCEPTIONS 1

/*
 * CONFIGDIR
 *
 * Global configuration to look for files not in ~/.fltk
 *
*/
#define CONFIGDIR ""

/*
 * CONF_CACHED
 *
 * define this to give reading config files a serious speed boost
 * but you will need to call conf_clear_cache() to see changes
 *
*/

#define CONF_CACHED

/*
 * End of "$Id: config.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
 */
