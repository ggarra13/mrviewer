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
/**
 * @file   YouTube.h
 * @author gga
 * @date   Sat Jan 10 10:00:02 2020
 *
 * @brief  A function to use youtube-dl and turn an url into a playable url.
 *
 *
 */


#ifndef YouTube_h
#define YouTube_h

namespace mrv
{
    bool YouTube( const std::string& url, std::string& videourl,
                  std::string& audiourl, std::string& title );
}

#endif // YouTube_h
