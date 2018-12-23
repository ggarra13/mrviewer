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
 * @file   picImage.h
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 *
 * @brief  A simple wrapper class to read all of ImageMagick's image formats
 *
 *
 */

#ifndef picImage_h
#define picImage_h

#include <CMedia.h>


namespace mrv {

class picImage : public CMedia
{
    enum kCompressionType
    {
        kNone,
        kRLE,
        kMixed
    };

    static const char* kCompression[];

    typedef struct __Channel {
        uint8_t	size;
        uint8_t	type;
        uint8_t	channels;
        struct __Channel *next;
    } Channel;

    picImage();

    static CMedia* create() {
        return new picImage();
    }


public:
    static bool test(const uint8_t* data, const unsigned len );
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
        return CMedia::get(create, name, datas);
    }

    virtual ~picImage();

    virtual const char* const format() const {
        return "Softimage PIC";
    }

    /// Returns the image compression (if any)
    virtual const char* const compression() const {
        return kCompression[_compression];
    }


    bool fetch( const boost::int64_t frame );

    static bool save( const char* file, const CMedia* img,
                      const ImageOpts* opts );

protected:
    bool readScanlines(FILE *file, uint32_t *image, int32_t width, int32_t height, Channel *channel, uint32_t alpha);
    bool readScanline(FILE *file, uint8_t *scan, int32_t width, Channel *channel,  int32_t bytes);
    bool channelReadRaw(FILE *file, uint8_t *scan, int32_t width, int32_t noCol, int32_t *off, int32_t bytes);
    bool channelReadPure(FILE *file, uint8_t *scan, int32_t width, int32_t noCol, int32_t *off, int32_t bytes);
    bool channelReadMixed(FILE *file, uint8_t *scan, int32_t width, int32_t noCol, int32_t *off, int32_t bytes);

protected:
    kCompressionType _compression;
};

} // namespace mrv


#endif // picImage_h
