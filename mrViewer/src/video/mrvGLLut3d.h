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

#include "IccCmm.h"

#include <boost/shared_ptr.hpp>


class CIccProfile;


namespace mrv {


  class CMedia;
  class PreferencesUI;

  class GLLut3d
  {
  public:
    struct Transform
    {
      enum Type
	{
	  kCTL = 'C',
	  kICC = 'I'
	};

      std::string       name;
      Type              type;
      icRenderingIntent intent;

      Transform( const std::string& n, const Type t,
		 const icRenderingIntent i = icPerceptual ) : 
	name(n), type(t), intent(i) 
      {
      }
    };

    enum XformFlags
      {
	kXformNone  = 0,
	kXformFirst = 1,
	kXformLast  = 2,
      };

    enum LutSizes
      {
	kNoBake,
	kLut32,
	kLut64,
        kLut96,
	kLut128,
      };

    typedef std::vector< Transform >         Transforms;
    typedef std::vector< std::string  >      TransformNames;

    typedef boost::shared_ptr< GLLut3d >         GLLut3d_ptr;
    typedef std::multimap< std::string, GLLut3d_ptr > LutsMap;

  public:
    GLLut3d( const unsigned N );
    virtual ~GLLut3d();

    void enable();
    void disable();

      void evaluate( const Imath::V3f& rgba, Imath::V3f& out ) const;

    virtual bool calculate_ctl( const Transforms::const_iterator& start,
				const Transforms::const_iterator& end,
				const Imf::Header& header,
				const XformFlags flags );

    virtual bool calculate_icc( const Transforms::const_iterator& start,
				const Transforms::const_iterator& end,
				const XformFlags flags );
      
    void create_gl_texture();

  protected:
    void clear_lut();
    void icc_cmm_error( const char* prefix,
			const icStatusCMM& status );

    // Returns size of lut with 4 channels
    unsigned lut_size() const { return 4 * _lutN * _lutN * _lutN; }
    unsigned lut_size2() const { return 4 + 4 * _lutN * _lutN * _lutN; }

    void init_pixel_values( Imf::Array< float >& pixelValues );

  public:

    static bool calculate(
			  GLLut3d_ptr lut,
			  const Transforms::const_iterator& start,
			  const Transforms::const_iterator& end,
			  const Imf::Header& header,
			  const XformFlags flags
			  );
    static GLLut3d* factory( const PreferencesUI* prefs, const CMedia* img );
    static void     clear();
      static void   transform_names( Transforms& t,
                                     const CMedia* img );

  protected:
    static bool     RT_ctl_transforms( std::string& key,
				       Transforms& transforms,
				       const CMedia* img,
				       const bool warn = false );

    static bool     RT_icc_transforms( std::string& key,
				       Transforms& transforms,
				       const CMedia* img,
					const bool warn = false );

    static bool     ODT_ctl_transforms( std::string& key,
					Transforms& transforms,
					const CMedia* img,
					const bool warn = false );
    
    static bool     ODT_icc_transforms( std::string& key,
					Transforms& transforms,
					const CMedia* img,
					const bool warn = false );

  public:
    float lutMin, lutMax, lutM, lutT, lutF; //!< The lut calculated parameters

  protected:
    GLuint texId;                          //!< The lut opengl texture index
    unsigned         _lutN;                //!< Size of lut (one axis)
    Imf::Array<float> lut;                  //!< The lut data
    bool _inited;

    static LutsMap _luts;                   //!< The list of luts
  };

} // namespace mrv


#endif // mrvGLLut3d_h
