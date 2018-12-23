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
 * @file   mrayImage.h
 * @author gga
 * @date   Fri Sep 21 01:19:27 2007
 * 
 * @brief  Custom image loader for mental ray image formats
 *         (st, ct, etc)
 * 
 */

#ifndef mrayImage_h
#define mrayImage_h

#include <CMedia.h>

namespace mrv {

  class mrayImage : public CMedia 
  {
    mrayImage();
    ~mrayImage();

    static CMedia* create() { return new mrayImage(); }

  public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual const char* const format() const;

    virtual bool fetch( const boost::int64_t& frame );

  protected:


    unsigned short _format;
  };


} // namespace mrv


#endif // mrayImage_h

