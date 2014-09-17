#ifndef mrvImageOpts_h
#define mrvImageOpts_h

#include <string>

#include "ImfCompression.h"
#include "ImfPixelType.h"
#include <wand/MagickWand.h>

namespace mrv {


class ImageOpts
{
  protected:
    bool _active;
    bool _opengl;
  public:
    ImageOpts()
    {
    }

    ~ImageOpts()
    {
    }

    bool active() const { return _active; }
    void active( bool f ) { _active = f; }

    bool opengl() const { return _opengl; }
    void opengl( bool f ) { _opengl = f; }


    static ImageOpts* build( std::string ext );
};


class EXROpts : public ImageOpts
{
  protected:
    Imf::Compression _compression;
    Imf::PixelType   _pixel_type;
    float            _dwa_compression_level;
  public:
    EXROpts() : _compression( Imf::ZIPS_COMPRESSION ),
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
    WandOpts() : _pixel_type( FloatPixel )
    {
    }

    StorageType pixel_type() const { return _pixel_type; }
};

}


#endif
