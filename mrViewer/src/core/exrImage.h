/**
 * @file   exrImage.h
 * @author gga
 * @date   Fri Sep 21 01:13:09 2007
 * 
 * @brief  Exr Image Loader
 * 
 * 
 */

#ifndef exrImage_h
#define exrImage_h

#include <CMedia.h>

#include <ImfArray.h>
#include <ImfLineOrder.h>
#include <ImfCompression.h>
#include <ImfPixelType.h>
#include <ImfHeader.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>



namespace mrv {

  class exrImage : public CMedia 
  {
    exrImage();
    ~exrImage();

    static CMedia* create() { return new exrImage(); }

    static const char* kCompression[];

    static const char* kLineOrder[];

  public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
      return CMedia::get(create, name, datas);
    }

    virtual const char* const format() const { return "ILM OpenEXR"; }

    virtual void compression( const unsigned idx ) { 
      _compression = (Imf::Compression) idx;
    };

    /// Returns the image compression (if any)
    virtual const char* const compression() const { return kCompression[_compression]; }

    virtual stringArray valid_compressions() const;

    /// Returns the image line order (if any)
    virtual const char* const line_order() const { return kLineOrder[_lineOrder]; }


    virtual bool fetch( const boost::int64_t frame );

      static bool save( const char* file, const CMedia* img, 
                        const ImageOpts* const opts );

      std::string type() const { return _type; }

       int current_part() const { return _curpart; }
       void current_part( unsigned x ) { _curpart = x; }

       int levelX() { return _levelX; }
       int levelY() { return _levelY; }

       void levelX( const int lx ) { _levelX = lx; }
       void levelY( const int ly ) { _levelY = ly; }



      void loadDeepData( int& zsize,
                         Imf::Array<float*>&       zbuff,
                         Imf::Array<unsigned int>& sampleCount );

      void findZBound( float& zmin, float& zmax, float farPlane,
                       int zsize,
                       Imf::Array<float*>&       zbuff,
                       Imf::Array<unsigned int>& sampleCount );

  protected:
      void loadDeepTileImage( int &zsize,
                              Imf::Array<float*>&       zbuff,
                              Imf::Array<unsigned int>& sampleCount,
                              bool deepComp );

      void loadDeepScanlineImage (int &zsize,
                                  Imf::Array<float*>&       zbuff,
                                  Imf::Array<unsigned int>& sampleCount,
                                  bool deepComp);

       bool find_layers( const Imf::Header& h );
       bool handle_side_by_side_stereo( const boost::int64_t frame,
                                        const Imf::Header& hdr, 
                                        Imf::FrameBuffer& fb );
       bool channels_order( 
			   const boost::int64_t frame,
			   Imf::ChannelList::ConstIterator s,
			   Imf::ChannelList::ConstIterator e,
			   Imf::ChannelList& channels,
			   const Imf::Header& hdr, 
			   Imf::FrameBuffer& fb
			    );
       bool channels_order_multi( 
			   const boost::int64_t frame,
			   Imf::ChannelList::ConstIterator s,
			   Imf::ChannelList::ConstIterator e,
			   Imf::ChannelList& channels,
			   const Imf::Header& hdr, 
			   Imf::FrameBuffer& fb
			    );
       void ycc2rgba( const Imf::Header& hdr, const boost::int64_t frame );
       bool fetch_mipmap( const boost::int64_t frame );
       bool fetch_multipart( const boost::int64_t frame );
       bool find_channels( const Imf::Header& h, Imf::FrameBuffer& fb,
			   boost::int64_t frame );
       void read_header_attr( const Imf::Header& h, boost::int64_t frame );

      static image_type::PixelType pixel_type_conversion( Imf::PixelType pixel_type );
      static Imf::PixelType pixel_type_to_exr( image_type::PixelType pixel_type );

     protected:

       int _levelX, _levelY; //<- r/mipmap levels
       bool _multiview;
       bool _has_yca, _use_yca, _has_left_eye, _has_right_eye, _left_red;

       int st[2];
       int _curpart;
       int _numparts;
       unsigned _num_layers;

       Imf::LineOrder   _lineOrder;
       Imf::Compression _compression;

      // Deep data
      std::string         _type;
      float               farPlane;
      bool                deepComp;

  };

}

#endif // exrImage_h

