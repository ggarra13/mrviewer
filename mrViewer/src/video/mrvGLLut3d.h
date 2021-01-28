/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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
 * @file   mrvGLLut3d.h
 * @author gga
 * @date   Sat Feb  9 14:10:47 2008
 *
 * @brief
 *
 *
 */

#ifndef mrvGLLut3d_h
#define mrvGLLut3d_h

#include <vector>
#include <map>
#include <string>

#include <GL/glew.h>

#include <half.h>
#include <ImfHeader.h>
#include <ImathVec.h>
#include <ImfArray.h>


#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

#include <boost/shared_ptr.hpp>
#include "core/mrvFrame.h"

class ViewerUI;
class PreferencesUI;


namespace mrv {

class CMedia;

void prepare_ACES( const CMedia* img, const std::string& name,
                   Imf::Header& h );

class GLLut3d
{
public:

    static unsigned NUM_STOPS;

    enum LutSizes
    {
        kNoBake,
        kLut32,
        kLut64,
        kLut96,
        kLut128,
        kLut192,
        kLut256,
    };


    typedef boost::shared_ptr< GLLut3d >         GLLut3d_ptr;
    // typedef std::multimap< std::string, GLLut3d_ptr > LutsMap;
    typedef std::map< std::string, GLLut3d_ptr > LutsMap;

public:
    GLLut3d( const ViewerUI* v, const unsigned N );
    // GLLut3d( const GLLut3d& b );
    virtual ~GLLut3d();

    void enable();
    void disable();


    unsigned edge_len() const {
        return _lutN;
    }

    // calculate lutMin and lutMax based on image
    void calculate_range( const mrv::image_type_ptr& pic );

    // Evaluate a pixel color and return the pixel color from the active LUT
    void evaluate( const Imath::V3f& rgba, Imath::V3f& out ) const;

    bool calculate_ocio( const CMedia* img );

    void clear_lut();
    void create_gl_texture();

    inline bool inited() const {
        return _inited;
    }
    inline void inited(bool x) {
        _inited = x;
    }

protected:

    // Returns size of lut with 3 or 4 channels
    unsigned lut_size() const {
        return _channels * _lutN * _lutN * _lutN;
    }

    void init_pixel_values( Imf::Array< float >& pixelValues );

public:

    static GLLut3d_ptr factory( const ViewerUI* ui, const CMedia* img );
    static void     clear();

protected:

public:
    float lutMin, lutMax, lutM, lutT, lutF; //!< The lut calculated parameters
    //OCIO::GpuShaderDescRcPtr shaderDesc;

protected:
    const ViewerUI* view;
    GLuint texId;                          //!< The lut opengl texture index
    unsigned  short  _channels;
    unsigned         _lutN;                //!< Size of lut (one axis)
    Imf::Array<float> lut;                  //!< The lut data
    bool _inited;

    // OCIO
    std::string g_display;
    std::string g_transformName;
    std::string g_inputColorSpace;

    static LutsMap _luts;                   //!< The list of luts

private:
    GLLut3d( const GLLut3d& b ) {};
};

} // namespace mrv


#endif // mrvGLLut3d_h
