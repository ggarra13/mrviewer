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
  public:
    ImageOpts()
    {
    }

    ~ImageOpts()
    {
    }

    bool active() const { return _active; }
    void active( bool f ) { _active = f; }

    static ImageOpts* build( std::string ext );
};


class EXROpts : public ImageOpts
{
  protected:
    Imf::Compression _compression;
    Imf::PixelType   _pixel_type;
  public:
    EXROpts() : _compression( Imf::ZIPS_COMPRESSION ),
                _pixel_type( Imf::HALF )
    {
    }

    Imf::Compression compression() const { return _compression; }
    void compression( Imf::Compression c ) { _compression = c; }

    Imf::PixelType pixel_type() const { return _pixel_type; }
    void pixel_type( Imf::PixelType p ) { _pixel_type = p; }

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
