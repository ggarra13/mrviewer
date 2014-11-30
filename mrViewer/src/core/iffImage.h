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
 * @file   iffImage.h
 * @author gga
 * @date   Sat Jul  7 22:46:07 2007
 * 
 * @brief  Class used to load a Maya IFF image
 * 
 * 
 */

#ifndef iffImage_h
#define iffImage_h

#include <CMedia.h>


namespace mrv {

  struct iffHeader;
  struct iffChunk;

  class iffImage : public CMedia 
  {
    iffImage();
    ~iffImage();

    static CMedia* create() { return new iffImage(); }

    static const char* kCompression[];

    enum IFFCompression {
      kNoCompression,
      kRLECompression
    };

  public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual const char* const format() const { return "Maya IFF"; }
    virtual const char* const compression() const { 
      return kCompression[_compression]; 
    }

    virtual bool fetch( const boost::int64_t frame );

  protected:
    void decompress_rle_tile( boost::uint8_t* data, const boost::uint8_t* comp,
			      const unsigned compsize, const unsigned width );

    void store_tile( boost::uint8_t* data, const unsigned x, const unsigned y,
		     const unsigned width, const unsigned height, 
		     const short depth, const short bytes, bool z );

    void end_read_chunk( FILE* f, iffChunk& chunk );
    void read_pixel_chunk( FILE* f, const int depth, const int bytes,
			   const iffChunk& chunk );

    void read_uncompressed_tile( FILE* file, boost::uint8_t* data,
				 const unsigned int compsize,
				 const unsigned x, const unsigned y,
				 const unsigned width, const unsigned height, 
				 const short depth, const short bytes, 
				 const bool z);
    void read_chunk( FILE* f, iffChunk& chunk );

    IFFCompression _compression;
  };

} // namespace mrv


#endif // iffImage_h
