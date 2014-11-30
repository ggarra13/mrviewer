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
 * @file   wandImage.h
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 * 
 * @brief  A simple wrapper class to read all of ImageMagick's image formats
 * 
 * 
 */

#ifndef wandImage_h
#define wandImage_h

#include <CMedia.h>

extern "C" {
#include <wand/magick-wand.h>
}

namespace mrv {

  class wandImage : public CMedia
  {
    wandImage();

    static CMedia* create() { return new wandImage(); }


  public:
    static bool test(const char* file);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual ~wandImage();

    virtual const char* const format() const { return _format; }

    /// Returns the image compression (if any)
    virtual const char* const compression() const { 
       return ""; // MagickOptionToMnemonic(CompressionType, _compression); 
    }

    virtual bool initialize();
    virtual bool release();

    bool fetch( const boost::int64_t frame );

  protected:
    char* _format;
    CompressionType _compression;
  };

} // namespace mrv


#endif // wandImage_h
