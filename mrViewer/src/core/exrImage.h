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
#include <ImfMultiPartInputFile.h>
#include <ImfFrameBuffer.h>



namespace mrv {

typedef std::vector< uint8_t* > Buffers;

class exrImage : public CMedia
{
    exrImage();
    ~exrImage();

    static CMedia* create() {
        return new exrImage();
    }

    static const char* kCompression[];

    static const char* kLineOrder[];


public:
    static bool test(const boost::uint8_t* datas, unsigned size=0);
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
        return CMedia::get(create, name, datas);
    }

    typedef std::map< int64_t, std::string > CapDates;

    virtual const char* const format() const {
        return "ILM OpenEXR";
    }

    virtual void compression( const unsigned idx ) {
        _compression = (Imf::Compression) idx;
    };

    /// Returns the image compression (if any)
    virtual const char* const compression() const {
        return kCompression[_compression];
    }

    virtual stringArray valid_compressions() const;

    /// Returns the image line order (if any)
    virtual const char* const line_order() const {
        return kLineOrder[_lineOrder];
    }


    virtual bool fetch( mrv::image_type_ptr& canvas,
			const boost::int64_t frame );

    static bool save( const char* file, const CMedia* img,
                      const ImageOpts* const opts );

    std::string type() const {
        return _type;
    }

    int current_part() const {
        return _curpart;
    }
    void current_part( unsigned x ) {
        _curpart = x;
    }

    int levelX() {
        return _levelX;
    }
    int levelY() {
        return _levelY;
    }

    void levelX( const int lx ) {
        _levelX = lx;
    }
    void levelY( const int ly ) {
        _levelY = ly;
    }



    void loadDeepData( int& zsize,
                       Imf::Array<float*>&       zbuff,
                       Imf::Array<unsigned int>& sampleCount );

    void findZBound( float& zmin, float& zmax, float farPlane,
                     int zsize,
                     Imf::Array<float*>&       zbuff,
                     Imf::Array<unsigned int>& sampleCount );

    const std::string& capture_date( const int64_t& f ) const {
        static std::string kUnknown = "";
        CapDates::const_iterator i = _cap_date.find(f);
        if ( i == _cap_date.end() ) return kUnknown;
        return i->second;
    }

protected:

    void loadDeepTileImage( mrv::image_type_ptr& canvas,
			    Imf::MultiPartInputFile& inmaster,
                            int &zsize,
                            Imf::Array<float*>&       zbuff,
                            Imf::Array<unsigned int>& sampleCount,
                            bool deepComp );

    void loadDeepScanlineImage (Imf::MultiPartInputFile& inmaster,
                                int &zsize,
                                Imf::Array<float*>&       zbuff,
                                Imf::Array<unsigned int>& sampleCount,
                                bool deepComp);

    bool find_layers( const Imf::Header& h );
    bool handle_stereo( mrv::image_type_ptr& canvas,
			const boost::int64_t& frame,
                        const Imf::Header& hdr,
                        Imf::FrameBuffer& fb );
    bool channels_order(
			mrv::image_type_ptr& canvas,
			const boost::int64_t& frame,
			Imf::ChannelList::ConstIterator& s,
			Imf::ChannelList::ConstIterator& e,
			const Imf::ChannelList& channels,
			const Imf::Header& hdr,
			Imf::FrameBuffer& fb
			);
    void ycc2rgba( const Imf::Header& hdr, const boost::int64_t& frame,
		   mrv::image_type_ptr& canvas );
    bool fetch_mipmap(  mrv::image_type_ptr& canvas,
			const boost::int64_t& frame );
    bool fetch_multipart(  mrv::image_type_ptr& canvas,
			   Imf::MultiPartInputFile& inmaster,
                          const boost::int64_t& frame );
    bool find_channels( mrv::image_type_ptr& canvas,
			const Imf::Header& h, Imf::FrameBuffer& fb,
                        const boost::int64_t& frame );
    void read_header_attr( const Imf::Header& h,
                           const boost::int64_t& frame );
    void read_forced_header_attr( const Imf::Header& h,
                                  const boost::int64_t& frame );

    /// Returns true if image has an alpha channel
    virtual bool  has_alpha() const {
        return _has_alpha;
    }


public:
    static void copy_pixel_data( mrv::image_type_ptr& pic,
                                 Imf::PixelType save_type,
                                 uint8_t* base,
                                 size_t total_size,
                                 bool use_alpha);
    static void add_attributes( const CMedia* img, Imf::Header& hdr );
    static image_type::PixelType pixel_type_conversion( Imf::PixelType pixel_type );
    static Imf::PixelType pixel_type_to_exr( image_type::PixelType pixel_type );

protected:

    CapDates _cap_date;

    int _levelX, _levelY; //<- r/mipmap levels
    bool _multiview;
    std::atomic<bool> _has_alpha;
    bool _has_yca, _use_yca;

    char* _has_left_eye;
    char* _has_right_eye;

    int st[2];
    int _curpart;
    int _clear_part;
    int _numparts;
    unsigned _num_layers;
    bool _read_attr;

    Imf::LineOrder   _lineOrder;
    Imf::Compression _compression;

    // ACES clip metadata present?
    bool _aces;

    // Info for reading layers
    stringSet layers;
    int order[4];

    // Stereo in same image
    bool _has_stereo;

    // Deep data
    std::string         _type;
    float               farPlane;
    bool                deepComp;

public:
    static float _default_gamma;
    static Imf::Compression _default_compression;
    static float _default_dwa_compression;
};

}

#endif // exrImage_h

