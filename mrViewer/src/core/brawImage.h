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
 * @file   brawImage.h
 * @author gga
 * @date   Fri Nov 03 15:38:30 2006
 *
 * @brief  A simple wrapper class to read the R3D movie format
 *
 *
 */

#ifndef brawImage_h
#define brawImage_h

#include "CMedia.h"
#ifdef _WIN32
#include "BlackmagicRawAPIDispatch.h"
#else
#include "BlackmagicRawAPI.h"
#endif

namespace mrv {


class brawImage : public CMedia
{

    brawImage();

    static CMedia* create() {
        return new brawImage();
    }


public:
    typedef std::vector< mrv::image_type_ptr > video_cache_t;
public:
    static bool test(const char* file);
    //static bool test(const uint8_t* data, const unsigned len );
    static CMedia* get(const char* name, const boost::uint8_t* datas = 0) {
        return CMedia::get(create, name, datas);
    }

    virtual ~brawImage();

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


    virtual bool has_video() const { return true; }

    ////////////////// Refresh frame as it has changed on disk
    virtual bool has_changed();

    virtual const char* const format() const { return "BRAW"; }

    virtual const char* const compression() const { return "BRAW"; }

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
                        const int64_t frame );

    virtual bool           frame( const int64_t f );
    inline int64_t frame() const { return _frame; }

    short scale() { return _scale; }
    void scale( short s ) { _scale = s; }

    bool store_image( mrv::image_type_ptr& canvas,
                      int64_t frame, unsigned w, unsigned h, void* data );

    virtual bool find_image( const int64_t frame );
    virtual void do_seek();

    virtual void audio_stream( int idx );

    virtual DecodeStatus decode_audio( int64_t& frame );

    void timed_limit_store( const int64_t frame );
    void limit_video_store( const int64_t frame );

    virtual void debug_video_stores( const int64_t frame,
                                     const char* title = "",
                                     const bool detail = false );

protected:
    void copy_values();
    void parse_metadata(int64_t frame, IBlackmagicRawMetadataIterator* iter);
    static HRESULT create_factory();

protected:
    Mutex         _load_mutex;
    video_cache_t _images;
    video_info_list_t     _video_info;

    short _scale;
    short _old_scale;

    uint64_t audioSamples;
    uint32_t bitDepth;

    int8_t* audiobuffer;
    bool    audioinit;

    static IBlackmagicRawFactory* factory;
    IBlackmagicRaw* codec;
    IBlackmagicRawClip* clip;
    IBlackmagicRawJob* readJob;
    IBlackmagicRawClipAudio* audio;
public:
    static bool init;
};

} // namespace mrv


#endif // brawImage_h
