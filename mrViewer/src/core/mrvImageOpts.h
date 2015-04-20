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
#ifndef mrvImageOpts_h
#define mrvImageOpts_h

#include <string>

#include "ImfCompression.h"
#include "ImfPixelType.h"
#include <wand/MagickWand.h>

namespace mrv {

static const char* kModule = "imgopt";

class ImageOpts
{
  protected:
    bool _active;
    bool _opengl;
    bool _ACESmetadata;
  public:
    ImageOpts(bool aces) :
    _active( true ),
    _opengl( false ),
    _ACESmetadata( aces )
    {
    }

    virtual ~ImageOpts()
    {
    }

    bool active() const { return _active; }
    void active( bool f ) { _active = f; }

    bool opengl() const { return _opengl; }
    void opengl( bool f ) { _opengl = f; }

    bool ACES_metadata() const { return _ACESmetadata; }
    void ACES_metadata( bool p ) { _ACESmetadata = p; }

    static ImageOpts* build( std::string ext, bool aces );
};


class EXROpts : public ImageOpts
{
  protected:
    Imf::Compression _compression;
    Imf::PixelType   _pixel_type;
    float            _dwa_compression_level;
  public:
    EXROpts( bool aces ) : ImageOpts( aces ),
                           _compression( Imf::ZIPS_COMPRESSION ),
                           _pixel_type( Imf::HALF ),
                           _dwa_compression_level( 45.0f )
    {
    }

    Imf::Compression compression() const { return _compression; }
    void compression( Imf::Compression c ) { _compression = c; }

    Imf::PixelType pixel_type() const { return _pixel_type; }
    void pixel_type( Imf::PixelType p ) { _pixel_type = p; }

    float compression_level() const { return _dwa_compression_level; }
    void compression_level( float p ) { _dwa_compression_level = p; }


};

class WandOpts : public ImageOpts
{
  protected:
    StorageType  _pixel_type;
  public:
    WandOpts(bool aces) : ImageOpts(aces),
                          _pixel_type( CharPixel )
    {
    }

    StorageType pixel_type() const { return _pixel_type; }
    void pixel_type( StorageType& t ) { _pixel_type = t; }
};

}  // namespace mrv


#endif
