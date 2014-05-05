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

    // Returns byte size of lut
    unsigned lut_size() const { return 4 * _lutN * _lutN * _lutN; }

    template< typename T >
    void init_pixel_values( Imf::Array< T >& pixelValues );

  public:

    static bool calculate(
			  GLLut3d_ptr& lut,
			  const Transforms::const_iterator& start,
			  const Transforms::const_iterator& end,
			  const Imf::Header& header,
			  const XformFlags flags
			  );
    static GLLut3d* factory( const PreferencesUI* prefs, const CMedia* img );
    static void     clear();

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
    Imf::Array<half> lut;                  //!< The lut data
    bool _inited;

    static LutsMap _luts;                   //!< The list of luts
  };

} // namespace mrv


#endif // mrvGLLut3d_h
