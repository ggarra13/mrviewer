/*
   mrViewer - the professional movie and flipbook playback
   Copyright (C) 2007-2022  Gonzalo Garramuño

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
 * @brief  A simple wrapper class to read the R3D movie format
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

    bool refetch( const int64_t f )
        {
            return CMedia::refetch( f );
        }

    bool refetch()
        {
            _dts = _frame.load();
            return refetch( _dts );
        }

    void Brightness( float f ) { _Brightness = f; refetch(); }
    void Contrast( float f ) { _Contrast = f; refetch(); }
    void Denoise( R3DSDK::ImageDenoise f )  { _Denoise = f; refetch(); }
    void Detail( R3DSDK::ImageDetail f )  { _Detail = f; refetch(); }
    void ExposureAdjust( float f ) { _ExposureAdjust = f; refetch(); }
    void ExposureCompensation( float f ) {
        _ExposureCompensation = f;
        refetch();
    }
    void Flut( float f )   { _Flut = f; refetch(); }
    void GainBlue( float f ) { _GainRed = f; refetch(); }
    void GainGreen( float f ) { _GainRed = f; refetch(); }
    void GainRed( float f ) { _GainRed = f; refetch(); }
    void Kelvin( float f )   { _Kelvin = f; refetch(); }
    void Saturation( float f ) { _Saturation = f; refetch(); }
    void Shadow( float f ) { _Shadow = f; refetch(); }
    void Sharpness( R3DSDK::ImageOLPFCompensation f )
        { _Sharpness = f; refetch(); }
    void Tint( float f )     { _Tint = f; refetch(); }

    float Brightness() const   { return _Brightness; }
    float Contrast() const   { return _Contrast; }
    R3DSDK::ImageDenoise Denoise() const   { return _Denoise; }
    R3DSDK::ImageDetail Detail() const   { return _Detail; }
    float ExposureAdjust() const { return _ExposureAdjust; }
    float ExposureCompensation() const { return _ExposureCompensation; }
    float Flut() const { return _Flut; }
    float GainBlue() const { return _GainBlue; }
    float GainGreen() const { return _GainGreen; }
    float GainRed() const { return _GainRed; }
    float Kelvin() const { return _Kelvin; }
    float Saturation() const   { return _Saturation; }
    float Shadow()   const { return _Shadow; }
    R3DSDK::ImageOLPFCompensation Sharpness()   const { return _Sharpness; }
    float Tint() const { return _Tint; }

    void iso_index( size_t i );
    size_t iso_index() const;

    void load_camera_settings();
    void load_rmd_sidecar();

    void set_ics_based_on_color_space_and_gamma();

    int color_version() const;

    R3DSDK::ImagePipeline pipeline() { return _pipeline; }
    void pipeline( R3DSDK::ImagePipeline t ) { _pipeline = t; refetch(); }

    short scale() const { return _scale; }
    void scale( int t );

    size_t real_width() const { return _real_width; }
    size_t real_height() const { return _real_height; }

    void color_spaces( stringArray& options ) const;
    std::string color_space() const;
    void color_space( unsigned idx );

    void gamma_curves( stringArray& options ) const;
    std::string gamma_curve() const;
    void gamma_curve( unsigned idx );

    bool is_hdr() const { return _hdr; }

    R3DSDK::HdrMode hdr_mode() const   { return _hdr_mode; }
    void hdr_mode( R3DSDK::HdrMode t ) {
        _hdr_mode = t;
        clear_cache();
        refetch();
    }

    void trackNo( size_t f ) { _trackNo = f; refetch(); }
    size_t trackNo() const   { return _trackNo; }

    float Bias()  const  { return _Bias; }
    void Bias( float f ) { _Bias = f; refetch(); }


    virtual bool has_video() const { return true; }

    ////////////////// Refresh frame as it has changed on disk
    virtual bool has_changed();

    virtual const char* const format() const { return "RED3D"; }

    virtual const char* const compression() const { return "RED3D CORE"; }

    virtual const video_info_t& video_info( unsigned int i ) const
    {
        assert( i < _video_info.size() );
        return _video_info[i];
    }

    virtual size_t number_of_video_streams() const {
        return _video_info.size();
    }

    virtual void clear_cache();

    virtual Cache is_cache_filled( int64_t frame );

    virtual bool fetch( mrv::image_type_ptr& canvas,
                        const boost::int64_t frame );

    virtual bool           frame( const int64_t f );
    inline int64_t frame() const { return _frame; }

    virtual bool find_image( const int64_t frame );
    virtual void do_seek();

    virtual void audio_stream( int idx );
    virtual DecodeStatus decode_audio( int64_t& frame );

    void timed_limit_store( const int64_t frame );
    void limit_video_store( const int64_t frame );

protected:
    unsigned int audio_bytes_per_frame();
    void copy_values();

protected:
    Mutex         _load_mutex;
    R3DSDK::Clip* clip;
    R3DSDK::ImageProcessingSettings* iproc;
    R3DSDK::ImagePipeline            _pipeline;
    R3DSDK::ImageColorSpace          _color_space;
    R3DSDK::ImageGammaCurve          _gamma_curve;
    video_cache_t _images;
    video_info_list_t     _video_info;
    bool          _hdr;
    short         _old_scale;
    short         _scale;
    int           _color_version;
    size_t        _real_width, _real_height;
    float         _Bias;
    float         _Brightness;
    float         _Contrast;
    R3DSDK::ImageDenoise  _Denoise;
    R3DSDK::ImageDetail   _Detail;
    float         _ExposureAdjust;
    float         _ExposureCompensation;
    float         _Flut;
    float         _GainBlue;
    float         _GainGreen;
    float         _GainRed;
    size_t        _ISO;
    float         _Kelvin;
    float         _old_Bias;
    size_t        _old_trackNo;
    R3DSDK::HdrMode _old_hdr_mode;
    float         _Saturation;
    float         _Shadow;
    R3DSDK::ImageOLPFCompensation _Sharpness;
    float         _Tint;
    size_t        _trackNo;
    R3DSDK::HdrMode  _hdr_mode;
    size_t           maxAudioBlockSize;
    size_t           adjusted;
    uint8_t*         audiobuffer;
    bool             audioinit;
    unsigned long long _total_samples;
public:
    static bool init;
};

} // namespace mrv


#endif // R3dImage_h
