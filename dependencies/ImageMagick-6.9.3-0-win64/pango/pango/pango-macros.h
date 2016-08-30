#ifndef __PANGO_MACROS_H__
#define __PANGO_MACROS_H__

#ifndef PANGO_EXTERN
#  if defined(_LIB)
#    define PANGO_EXTERN extern
#  else
#    ifdef PANGO_COMPILATION
#      define PANGO_EXTERN __declspec(dllexport)
#    else
#      define PANGO_EXTERN extern __declspec(dllimport)
#    endif
#  endif
#endif

#endif /* __PANGO_MACROS_H__ */
