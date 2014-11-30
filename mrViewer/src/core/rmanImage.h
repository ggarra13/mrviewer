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
 * @file   rmanImage.h
 * @author gga
 * @date   Fri Nov 10 14:21:44 2006
 * 
 * @brief  A simple class to hook to a renderman socket stub using
 *         liquid's display drivers.
 * 
 * 
 */

#ifndef rmanImage_h
#define rmanImage_h

#include "stubImage.h"

namespace mrv {

  class rmanImage : public stubImage 
  {
    rmanImage();
    ~rmanImage();

    static CMedia* create() { return new rmanImage(); }

  public:
    rmanImage( const CMedia* other );

    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }
    bool fetch();

    const char* host() { return _host; }

    unsigned int port() { return _portB; }

    virtual bool has_changed();

    virtual const char* const format() const { return "Renderman Liquid stub"; }

  };

} // namespace mrv


#endif // rmanImage_h

