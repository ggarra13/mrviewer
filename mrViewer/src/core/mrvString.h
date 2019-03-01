/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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
 * @file   mrvString.h
 * @author gga
 * @date   Wed Oct 18 21:09:07 2006
 *
 * @brief  Some std::string utilities
 *
 *
 */

#ifndef mrvString_h
#define mrvString_h

#include <sstream>
#include <string>
#include <vector>
#include <set>

typedef std::vector< std::string > stringArray;
typedef std::set   < std::string > stringSet;

namespace mrv
{
bool matches_chars( const char* src, const char* charlist );

void split_string(stringArray& output,
                  const std::string& str, const std::string& delim );

inline void split( stringArray& elems, const std::string &s, char delim ) {
    std::stringstream ss(s);
    std::string item;
    elems.clear();
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

} // namespace mrv


#endif // mrvString_h
