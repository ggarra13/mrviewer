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
 * @file   mapImage.h
 * @author gga
 * @date   Fri Sep 21 01:18:01 2007
 * 
 * @brief  Custom image reader for mental ray's map images.
 * 
 * 
 */

#ifndef mapImage_h
#define mapImage_h

#include <CMedia.h>

namespace mrv {

  class mapImage : public CMedia 
  {
    mapImage();
    ~mapImage();

    static CMedia* create() { return new mapImage(); }

  public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual const char* const format() const { return "mental images texture"; }

    /// Returns the image line order (if any)
    virtual const char* const line_order() const;

    virtual bool has_changed();

    virtual bool fetch(const boost::int64_t frame);

  protected:
    bool is_stub();

  protected:
    short _lineOrder;
    bool   _stub;
    time_t _atime;
  };

} // namespace mrv


#endif // mapImage_h

