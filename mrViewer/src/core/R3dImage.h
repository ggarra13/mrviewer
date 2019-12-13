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
 * @file   R3dImage.h
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 *
 * @brief  A simple wrapper class to read all of ImageMagick's image formats
 *
 *
 */

#ifndef R3dImage_h
#define R3dImage_h

#include <CMedia.h>
#include <R3DSDK.h>

namespace mrv {

class R3dImage : public CMedia
{

    R3dImage();

    static CMedia* create() {
        return new R3dImage();
    }


public:
    typedef std::vector< mrv::image_type_ptr > video_cache_t;
public:
    static bool test(const char* file);
    //static bool test(const uint8_t* data, const unsigned len );
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
        return CMedia::get(create, name, datas);
    }

    virtual ~R3dImage();

    virtual bool initialize();
    virtual bool finalize();

    /// Returns true if image has an alpha channel
    virtual bool  has_alpha() const {
        return (_num_channels == 4);
    }

    void Kelvin( float f )   { _Kelvin = f; refetch(); }
    void Tint( float f )     { _Tint = f; refetch(); }
    void Exposure( float f ) { _Exposure = f; refetch(); }

    float Kelvin() const { return _Kelvin; }
    float Tint() const { return _Tint; }
    float Exposure() const { return _Exposure; }

    void iso_index( size_t i );
    size_t iso_index() const;

    void load_camera_settings();
    void load_rmd_sidecar();

    int color_version() const;

    std::string color_space() const;
    std::string gamma_curve() const;

    bool is_hdr() const { return _hdr; }

    R3DSDK::HdrMode hdr_mode() const { return _hdr_mode; }
    void hdr_mode( R3DSDK::HdrMode t ) { _hdr_mode = t; refetch(); }

    void trackNo( size_t f ) { _trackNo = f; refetch(); }
    size_t trackNo() const { return _trackNo; }

    float Bias()         { return _Bias; }
    void Bias( float f ) { _Bias = f; refetch(); }

    virtual bool has_video() const { return true; }

    ////////////////// Refresh frame as it has changed on disk
    virtual bool has_changed();

    virtual const char* const format() const { return "RED3D"; }

    virtual const char* const compression() const { return "RED3D CORE"; }

    virtual void clear_cache();

    virtual Cache is_cache_filled( int64_t frame );

    virtual bool fetch( mrv::image_type_ptr& canvas,
                        const boost::int64_t frame );

    virtual bool           frame( const int64_t f );
    inline int64_t frame() const { return _frame; }

    virtual bool find_image( const int64_t frame );
    virtual void do_seek();

    void timed_limit_store( const int64_t frame );
    void limit_video_store( const int64_t frame );

protected:
    R3DSDK::Clip* clip;
    R3DSDK::ImageProcessingSettings* iproc;
    video_cache_t _images;
    bool          _new_grade;
    bool          _hdr;
    int           _scale;
    size_t        _ISO;
    float         _Kelvin;
    float         _Tint;
    float         _Exposure;
    float         _Bias;
    size_t        _trackNo;
    R3DSDK::HdrMode  _hdr_mode;
public:
    static bool init;
};

} // namespace mrv


#endif // R3dImage_h
