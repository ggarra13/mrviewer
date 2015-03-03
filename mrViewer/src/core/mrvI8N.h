/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

#ifdef _WIN32
#undef fprintf
// #undef sprintf
#undef sscanf
#endif

#else

#define _(String) (String)
#define N_(String) String
#define textdomain(Domain)
#define bindtextdomain(Package, Directory)

#endif


#endif // mrvI8N_h
