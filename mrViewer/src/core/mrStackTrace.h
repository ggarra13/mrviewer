/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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


#ifndef mrStackTrace_h
#define mrStackTrace_h

#if defined(_WIN32) || defined(_WIN64)
#include "mrStackTrace_win32.h"
#elif LINUX
//#include "mrStackTrace_linux.h"
#elif OSX
#else
#error Unknown OS for stack trace
#endif

#endif // mrStackTrace_h
