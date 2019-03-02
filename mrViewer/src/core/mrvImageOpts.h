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

#include "core/mrvI8N.h"
#include <string>

#include "ImfCompression.h"
#include "ImfPixelType.h"
#include "mrvFrame.h"
#include <MagickWand/MagickWand.h>

namespace mrv {

class ViewerUI;

class ImageOpts
{
protected:
    bool _active;
    bool _opengl;
    bool _ACESmetadata;
    bool _all_layers;
public:
    ImageOpts(bool aces, bool all_layers) :
        _active( true ),
        _opengl( false ),
        _ACESmetadata( aces ),
        _all_layers( all_layers )
    {
    }

    virtual ~ImageOpts()
    {
    }

    bool active() const {
        return _active;
    }
    void active( bool f ) {
        _active = f;
    }

    bool opengl() const {
        return _opengl;
    }
    void opengl( bool f ) {
        _opengl = f;
    }

    bool all_layers() const {
        return _all_layers;
    }
    void all_layers( bool f ) {
        _all_layers = f;
    }

    bool ACES_metadata() const {
        return _ACESmetadata;
    }
    void ACES_metadata( bool p ) {
        _ACESmetadata = p;
    }

    static ImageOpts* build( const mrv::ViewerUI* main, std::string ext,
                             const bool has_deep_data );
};


class EXROpts : public ImageOpts
{
protected:
    Imf::Compression _compression;
    Imf::PixelType   _pixel_type;
    float            _dwa_compression_level;
    bool             _save_deep_data;
public:
    EXROpts( bool aces, bool all_layers ) :
        ImageOpts( aces, all_layers ),
        _compression( Imf::ZIPS_COMPRESSION ),
        _pixel_type( Imf::HALF ),
        _dwa_compression_level( 45.0f ),
        _save_deep_data( false )
    {
    }

    Imf::Compression compression() const {
        return _compression;
    }
    void compression( Imf::Compression c ) {
        _compression = c;
    }

    Imf::PixelType pixel_type() const {
        return _pixel_type;
    }
    void pixel_type( Imf::PixelType p ) {
        _pixel_type = p;
    }

    float compression_level() const {
        return _dwa_compression_level;
    }
    void compression_level( float p ) {
        _dwa_compression_level = p;
    }

    bool save_deep_data() const {
        return _save_deep_data;
    }
    void save_deep_data( const bool t ) {
        _save_deep_data = t;
    }

};

class OIIOOpts : public ImageOpts
{
protected:
    image_type::PixelType _pixel_type;
    bool         _mipmap;
public:
    OIIOOpts( bool all_layers ) :
        ImageOpts( false, all_layers ),
        _pixel_type( image_type::kFloat ),
        _mipmap( false )
    {
    }

    inline bool mipmap() const {
        return _mipmap;
    }
    inline void mipmap( bool p ) {
        _mipmap = p;
    }

    image_type::PixelType pixel_type() const {
        return _pixel_type;
    }
    void pixel_type( image_type::PixelType p ) {
        _pixel_type = p;
    }
};

class WandOpts : public ImageOpts
{
protected:
    StorageType  _pixel_type;
public:
    WandOpts(bool aces, bool all_layers) :
        ImageOpts(aces, all_layers),
        _pixel_type( CharPixel )
    {
    }

    StorageType pixel_type() const {
        return _pixel_type;
    }
    void pixel_type( StorageType& t ) {
        _pixel_type = t;
    }
};

}  // namespace mrv


#endif
