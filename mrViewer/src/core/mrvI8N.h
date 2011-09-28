/**
 * @file   mrvI8N.h
 * @author gga
 * @date   Thu Jul 26 08:36:58 2007
 * 
 * @brief  Some macros used for gettext() internationalization
 * 
 * 
 */

#ifndef mrvI8N_h
#define mrvI8N_h

#ifdef USE_GETTEXT

#include <libintl.h>
#define _(String)  gettext(String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)

#else

#define _(String) (String)
#define N_(String) String
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)

#endif


#endif // mrvI8N_h
