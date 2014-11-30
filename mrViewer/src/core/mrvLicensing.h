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
 * @file   mrvProtection.h
 * @author gga
 * @date   Sat Oct 28 05:53:19 2006
 * 
 * @brief  A simple copy-protection/license mechanism
 * 
 * 
 */

#include <ctime>

#include <string>

namespace mrv
{
  bool open_license(const char* prog);
  bool close_license();
  bool checkout_license();
  bool check_license_status();
  bool checkin_license();

}
