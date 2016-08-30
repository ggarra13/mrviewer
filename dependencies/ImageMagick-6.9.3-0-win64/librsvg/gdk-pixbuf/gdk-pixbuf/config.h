/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#define ENABLE_NLS 1

/* Define if gio can sniff image data */
/*#define GDK_PIXBUF_USE_GIO_MIME 1*/

/* The prefix for our gettext translation domains. */
#define GETTEXT_PACKAGE "gdk-pixbuf"

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
#define HAVE_BIND_TEXTDOMAIN_CODESET 1

/* Define to 1 if you have the MacOS X function CFLocaleCopyCurrent in the
   CoreFoundation framework. */
/*#undef HAVE_CFLOCALECOPYCURRENT*/

/* Define to 1 if you have the MacOS X function CFPreferencesCopyAppValue in
   the CoreFoundation framework. */
/*#undef HAVE_CFPREFERENCESCOPYAPPVALUE*/

/* Define if the GNU dcgettext() function is already present or preinstalled.
   */
#define HAVE_DCGETTEXT 1

/* Define to 1 if you have the <dlfcn.h> header file. */
/*#undef HAVE_DLFCN_H*/

/* Define if the GNU gettext() function is already present or preinstalled. */
#define HAVE_GETTEXT 1

/* Define if you have the iconv() function and it works. */
#define HAVE_ICONV 1

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef _MSC_VER
#define HAVE_INTTYPES_H 1
#else
/*#define HAVE_INTTYPES_H*/
#endif

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 is libjpeg supports progressive JPEG */
#define HAVE_PROGRESSIVE_JPEG 1

/* Define to 1 if sigsetjmp is available */
/*#undef HAVE_SIGSETJMP*/

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef _MSC_VER
#define HAVE_STDINT_H 1
#else
#if (_MSC_VER >= 1600)
#define HAVE_STDINT_H 1
#endif
/*#undef HAVE_STDINT_H*/
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#ifndef _MSC_VER
#define HAVE_STRINGS_H 1
#else
/*#undef HAVE_STRINGS_H*/
#endif

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if sys/sysinfo.h is available */
/*#undef HAVE_SYS_SYSINFO_H*/

/* Define to 1 if sys/systeminfo.h is available */
/*#undef HAVE_SYS_SYSTEMINFO_H*/

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef _MSC_VER
#define HAVE_UNISTD_H 1
#else
/*#undef HAVE_UNISTD_H*/
#endif

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
#ifdef _MSC_VER
#define NO_MINUS_C_MINUS_O 1
#else
/*#undef NO_MINUS_C_MINUS_O 1*/
#endif

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://bugzilla.gnome.org/enter_bug.cgi?product=gdk-pixbuf"

/* Define to the full name of this package. */
#define PACKAGE_NAME "gdk-pixbuf"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "gdk-pixbuf 2.30.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "gdk-pixbuf"

/* Define to the home page for this package. */
#define PACKAGE_URL "http://www.gtk.org/"

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.30.1"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if gmodule works and should be used */
#undef USE_GMODULE 1

/* Whether to load modules via .la files rather than directly */
/*#undef USE_LA_MODULES*/

/* Define to 1 if medialib is available and should be used */
/*#undef USE_MEDIALIB*/

/* Define to 1 if medialib 2.5 is available */
/*#undef USE_MEDIALIB25*/

/* Define to 1 if MMX is available and should be used */
#ifdef _MSC_VER
/*#undef USE_MMX*/
#else
#define USE_MMX 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/*#undef _FILE_OFFSET_BITS*/

/* Define for large files, on AIX-style hosts. */
/*#undef _LARGE_FILES*/

/* Define to empty if `const' does not conform to ANSI C. */
/*#undef const*/

#define GDK_PIXBUF_BINARY_VERSION "2.10.0"
#define GDK_PIXBUF_PREFIX ""
#define PIXBUF_LIBDIR "."

#define INCLUDE_ani
#define INCLUDE_bmp
#define INCLUDE_gif
#define INCLUDE_ico
#define INCLUDE_icns
#define INCLUDE_jasper
#define INCLUDE_jpeg
#define INCLUDE_pcx
#define INCLUDE_png
#define INCLUDE_pnm
#define INCLUDE_qtif
#define INCLUDE_ras
#define INCLUDE_svg
#define INCLUDE_tga
#define INCLUDE_tiff
#define INCLUDE_xpm
#define INCLUDE_xbm
#define INCLUDE_wbmp
