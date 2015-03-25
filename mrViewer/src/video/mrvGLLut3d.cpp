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
 * @file   mrvGLLut3d.cpp
 * @author gga
 * @date   Sat Feb  9 14:13:57 2008
 * 
 * @brief  
 * 
 * 
 */

#include <fstream>
#include <limits>

#include <Iex.h>
#include <half.h>
#include <ImfArray.h>
#include <ImfHeader.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfVecAttribute.h>
#include <ImfStandardAttributes.h>
#include <ImfVecAttribute.h>
#include <CtlExc.h>

#include <fltk/Cursor.h>

#include "IccProfile.h"
#include "IccCmm.h"
 

#include "ctlToLut.h"
#include "mrvIO.h"
#include "ACES_ASC_CDL.h"
#include "core/CMedia.h"
#include "core/mrvColorProfile.h"
#include "gui/mrvPreferences.h"
#include "mrViewer.h"

#include "mrvGLLut3d.h"
#include "mrvGLEngine.h"

namespace 
{
  const char* kModule = N_("cmm");
}


namespace mrv {
namespace {

void prepare_ACES( const CMedia* img, const std::string& name,
                   Imf::Header& h )
{
    using namespace Imf;
    using namespace Imath;

    const ACES::ASC_CDL& c = img->asc_cdl();

    std::string n = name.substr( 4, 7 );


    if ( n == "SOPNode" )
    {
        {
            V3fAttribute attr( V3f( c.slope(0), c.slope(1), c.slope(2) ) );
            h.insert( "slope", attr );
        }
        {
            V3fAttribute attr( V3f( c.offset(0), c.offset(1), c.offset(2) ) );
            h.insert( "offset", attr );
        }
        {
            V3fAttribute attr( V3f( c.power(0), c.power(1), c.power(2) ) );
            h.insert( "power", attr );
        }
    }
    else if ( n == "SatNode" )
    {
        FloatAttribute attr( c.saturation() );
        h.insert( "saturation", attr );
    }
}

inline void
indicesAndWeights (float r, int iMax, int &i, int &i1, float &u, float &u1)
{
    if (r >= 0)
    {
	if (r < iMax)
	{
	    //
	    // r is finite and in the interval [0, iMax]
	    //

	    i = int (r);
	    i1 = i + 1;
	    u  = r - (float) i;
	}
	else
	{
	    //
	    // r is greater than or equal to iMax
	    //

	    i = i1 = iMax;
	    u = 1;
	}
    }
    else
    {
	//
	// r is either NaN or less than 0
	//

	i = i1 = 0;
	u = 1;
    }

    u1 = 1 - u;
}

} // namespace

using namespace Imath;

V3f
lookup3D
    (const V4f table[],
     const int size,
     const V3f &p)
{
    int Max = size - 1;
    float r = (clamp (p.x, 0.0f, 1.0f) ) * (float) Max;

    int i, i1;
    float u, u1;
    indicesAndWeights (r, Max, i, i1, u, u1);

    float s = (clamp (p.y, 0.0f, 1.0f) ) * (float) Max;

    int j, j1;
    float v, v1;
    indicesAndWeights (s, Max, j, j1, v, v1);

    float t = (clamp (p.z, 0.0f, 1.0f) ) * (float) Max;

    int k, k1;
    float w, w1;
    indicesAndWeights (t, Max, k, k1, w, w1);

    const V4f &a = table[(k  * size + j ) * size + i ];
    const V4f &b = table[(k1 * size + j ) * size + i ];
    const V4f &c = table[(k  * size + j1) * size + i ];
    const V4f &d = table[(k1 * size + j1) * size + i ];
    const V4f &e = table[(k  * size + j ) * size + i1];
    const V4f &f = table[(k1 * size + j ) * size + i1];
    const V4f &g = table[(k  * size + j1) * size + i1];
    const V4f &h = table[(k1 * size + j1) * size + i1];

    V3f out(
    u1 * (v1 * (w1 * a.x + w * b.x) + v * (w1 * c.x + w * d.x)) +
    u  * (v1 * (w1 * e.x + w * f.x) + v * (w1 * g.x + w * h.x)),

    u1 * (v1 * (w1 * a.y + w * b.y) + v * (w1 * c.y + w * d.y)) +
    u  * (v1 * (w1 * e.y + w * f.y) + v * (w1 * g.y + w * h.y)),

    u1 * (v1 * (w1 * a.z + w * b.z) + v * (w1 * c.z + w * d.z)) +
    u  * (v1 * (w1 * e.z + w * f.z) + v * (w1 * g.z + w * h.z))
    );

    return out;
}


  GLLut3d::LutsMap GLLut3d::_luts;


  GLLut3d::GLLut3d( const unsigned N ) :
    lutMin( 0 ),
    lutMax( 0 ),
    lutM( 0 ),
    lutT( 0 ),
    lutF( 0 ),
    texId( 0 ),
    _lutN( N ),
    _inited( false )
  {
      glGenTextures( 1, &texId );
  }



  GLLut3d::~GLLut3d()
  {
      disable();
      glDeleteTextures( 1, &texId );
  }



  void GLLut3d::enable()
  {
    if ( GLEngine::maxTexUnits() > 3 )
      glActiveTexture(GL_TEXTURE3);

    glBindTexture( GL_TEXTURE_3D, texId );
    glEnable( GL_TEXTURE_3D );
  }



  void GLLut3d::disable()
  {
    if ( GLEngine::maxTexUnits() > 3 )
      glActiveTexture(GL_TEXTURE3);
    glDisable( GL_TEXTURE_3D );
  }



  //
  // Initialize array of output pixel values to zero.
  //
  void GLLut3d::clear_lut()
  {
     _inited = false;
     // @bug: CTL would crash if pixel values was set to lut_size()
     //       We pad here 4 more bytes and that prevents the crash.
     unsigned long num = lut_size() + 4;
     lut.resizeErase( num );
     for ( unsigned long i = 0; i < num; ++i )
         lut[i] = 0;
  }



  //
  // Create opengl texture from the log of lut values
  //
  void GLLut3d::create_gl_texture()
  {
    //
    // Take the logarithm of the output values that were
    // produced by the CTL transforms.
    //

      size_t num = lut_size();
      for ( size_t i = 0; i < num; ++i )
      {
          if ( lut[i] >= std::numeric_limits<float>::min()
               && lut[i] <= std::numeric_limits<float>::max() )
	  {
	    //
	    // lut[i] is finite and positive.
	    //
            lut[i] = (float) logf(lut[i]);
	  }
          else
	  {
	    //
	    // lut[i] is zero, negative or not finite;
	    // log (lut[i]) is undefined.
	    //
             lut[i] = (float) logf( std::numeric_limits<float>::min() );
	  }
      }

    //
    // Convert the output values into a 3D texture.
    //
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    glActiveTexture( GL_TEXTURE3 );

    glBindTexture( GL_TEXTURE_3D, texId );

    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    GLenum gl_clamp = GL_CLAMP;
    if ( GLEW_EXT_texture_edge_clamp )
      gl_clamp = GL_CLAMP_TO_EDGE;

    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, gl_clamp );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, gl_clamp );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, gl_clamp );


    glTexImage3D( GL_TEXTURE_3D,
		  0,			// level
                  GL_RGBA32F,           // internal format
		  _lutN, _lutN, _lutN,	// width, height, depth
		  0,			// border
		  GL_RGBA,		// format
		  GL_FLOAT,	// type
		  (char *) &lut[0] );
  }


void GLLut3d::evaluate( const Imath::V3f& rgb, Imath::V3f& out ) const
{
    using namespace Imath;


    out.x = lutT + lutM * logf( Imath::clamp( rgb.x, lutMin, lutMax ) );
    out.y = lutT + lutM * logf( Imath::clamp( rgb.y, lutMin, lutMax ) );
    out.z = lutT + lutM * logf( Imath::clamp( rgb.z, lutMin, lutMax ) );


    out = lookup3D( (V4f*)(&lut[0]), _lutN, out );

    out.x = expf( out.x );
    out.y = expf( out.y );
    out.z = expf( out.z );

}

  void GLLut3d::init_pixel_values( Imf::Array< float >& pixelValues )
  {
    //
    // Compute lutMin, lutMax, and scale and offset
    // values, lutM and lutT, so that
    //
    //	lutM * lutMin + lutT == 0
    //	lutM * lutMax + lutT == 1
    //

    static const int NUM_STOPS = 7;
    static const float MIDDLE_GRAY = 0.18f;

    lutMin = MIDDLE_GRAY / (1 << NUM_STOPS);
    lutMax = MIDDLE_GRAY * (1 << NUM_STOPS);

    float logLutMin = logf (lutMin);
    float logLutMax = logf (lutMax);

    lutM = 1 / (logLutMax - logLutMin);
    lutT = -lutM * logLutMin;

    //
    // Build a 3D array of RGB input pixel values.
    // such that R, G and B are between lutMin and lutMax.
    //
    for (size_t ib = 0; ib < _lutN; ++ib)
    {
        float b = float(ib) / float(_lutN - 1.0);
        half B = expf((b - lutT) / lutM);

	for (size_t ig = 0; ig < _lutN; ++ig)
	  {
              float g = float(ig) / float(_lutN - 1.0);
              half G = expf ((g - lutT) / lutM);

	    for (size_t ir = 0; ir < _lutN; ++ir)
	      {
                  float r = float(ir) / float(_lutN - 1.0);
                  half R = expf ((r - lutT) / lutM);

                  size_t i = (ib * _lutN * _lutN + ig * _lutN + ir) * 4;
                  pixelValues[i + 0] = R;
                  pixelValues[i + 1] = G;
                  pixelValues[i + 2] = B;
                  pixelValues[i + 3] = 1.0f;
	      }
	  }
      }
  }



  bool GLLut3d::calculate_ctl( 
			      const Transforms::const_iterator& start,
			      const Transforms::const_iterator& end,
			      const CMedia* img,
			      const XformFlags flags
			       )
  {
    //
    // We build a 3D color lookup table by running a set of color
    // samples through a series of CTL transforms.
    //
    // The 3D lookup table covers a range from lutMin to lutMax or
    // NUM_STOPS f-stops above and below 0.18 or MIDDLE_GRAY.  The
    // size of the table is _lutN by _lutN by _lutN samples.
    //
    // In order make the distribution of the samples in the table
    // approximately perceptually uniform, the Cg shaders that use
    // the table perform lookups in "log space":
    // In a Cg shader, the lookup table is represented as a 3D texture.
    // In order to apply the table to a pixel value, the Cg shader takes
    // the logarithm of the pixel value and scales and offsets the result
    // so that lutMin and lutMax map to 0 and 1 respectively.  The scaled
    // value is used to perform a texture lookup and the shader computes
    // e raised to the power of the result of the texture lookup.
    //

    Imf::Array<float> pixelValues( lut_size() );

    //
    // Generate output pixel values by applying CTL transforms
    // to the pixel values.  (If the CTL transforms fail to
    // write to the output values, zero-initialization, above,
    // causes the displayed image to be black.)
    //
    const char** channelNames;

    static const char* InRGBAchannels[4] = { N_("rIn"), N_("gIn"), N_("bIn"),
                                             N_("aIn") };

    channelNames = InRGBAchannels;

    TransformNames transformNames;
    Transforms::const_iterator i = start;
    for ( ; i != end; ++i )
      {

          Imf::Header header( img->width(), img->height(),
                              float( img->pixel_ratio() ) );
          Imf::addChromaticities( header, img->chromaticities() );

          if ( !_inited )
          {
              //
              // Init lut table to 0
              //
              clear_lut();

              //
              // Init table of pixel values
              //
              init_pixel_values( pixelValues );
          }
          else
          {
              //
              // Copy rOut, gOut, bOut, aOut to rIn, gIn, bIn, aIn
              //
              memcpy( &pixelValues[0], &lut[0], lut_size() * sizeof(float) );
          }

          transformNames.clear();
          transformNames.push_back( (*i).name );

          prepare_ACES( img, (*i).name, header );

          try
          {
              ctlToLut( transformNames, header, lut_size(), pixelValues, lut,
                        channelNames );
              _inited = true;
          }
          catch( const std::exception& e )
          {
              LOG_ERROR( e.what() );
              return false;
          }
          catch( ... )
          {
              LOG_ERROR( _("Unknown error returned from ctlToLut") );
              return false;
          }
      }


    return true;
  }



  void GLLut3d::icc_cmm_error( const char* prefix,
			       const icStatusCMM& status )
  {
    std::string err;
    switch( status )
      {
      case icCmmStatAllocErr:
	err = "Allocation error"; break;
      case icCmmStatCantOpenProfile:
	err = "Can't open profile"; break;
      case icCmmStatBadXform:
	err = "Bad transform"; break;
      case icCmmStatInvalidLut:
	err = "Invalid Lut"; break;
      case icCmmStatProfileMissingTag:
	err = "Profile Missing Tag"; break;
      case icCmmStatColorNotFound:
	err = "Color not found"; break;
      case icCmmStatIncorrectApply:
	err = "Incorrect Apply"; break;
      case icCmmStatBadColorEncoding:
	err = "Bad color encoding"; break;
      case icCmmStatBadLutType:
	err = "Bad color encoding"; break;
      case icCmmStatBadSpaceLink:
	err = "Bad color space link"; break;
      default:
	err = "Unknown error";
      }

    LOG_ERROR( prefix << err );
  }



  bool GLLut3d::calculate_icc( const Transforms::const_iterator& start,
			       const Transforms::const_iterator& end,
			       const XformFlags flags )
  {
    //
    // We build a 3D color lookup table by running a set of color
    // samples through a series of CTL transforms.
    //
    // The 3D lookup table covers a range from lutMin to lutMax or
    // NUM_STOPS f-stops above and below 0.18 or MIDDLE_GRAY.  The
    // size of the table is _lutN by _lutN by _lutN samples.
    //
    // In order make the distribution of the samples in the table
    // approximately perceptually uniform, the Cg shaders that use
    // the table perform lookups in "log space":
    // In a Cg shader, the lookup table is represented as a 3D texture.
    // In order to apply the table to a pixel value, the Cg shader takes
    // the logarithm of the pixel value and scales and offsets the result
    // so that lutMin and lutMax map to 0 and 1 respectively.  The scaled
    // value is used to perform a texture lookup and the shader computes
    // e raised to the power of the result of the texture lookup.
    //

    Imf::Array<float> pixelValues ( lut_size() );
    if ( !_inited )
      {
	clear_lut();
	init_pixel_values( pixelValues );
      }
    else
      {
	float* dst = pixelValues;
	for (unsigned i = 0; i < lut_size(); ++i )
	  {
	    dst[i] = lut[i];
	  }
      }

    //
    // Generate output pixel values by applying CMM ICC transforms
    // to the pixel values. 
    //
    CIccCmm cmm( icSigUnknownData, 
		 icSigUnknownData,
// 		 (flags & kXformLast) ? icSigRgbData : icSigXYZData, 
		 (flags & kXformFirst) );


    icStatusCMM status;
    {
      Transforms::const_iterator i = start;
      for ( ; i != end; ++i )
	{
	  const char* name = (*i).name.c_str();
	  CIccProfile* pIcc = colorProfile::get( name );
	  if ( !pIcc )
	    {
	      LOG_ERROR( _("Could not locate ICC profile \"") 
			 << name << N_("\"") );
	      return false;
	    }

	  status = cmm.AddXform( pIcc, (*i).intent );
	  if ( status != icCmmStatOk )
	    {
	      char err[1024];
	      sprintf( err, _("Could not add profile \"%s\" to CMM: "),
		       name );
	      icc_cmm_error( err, status );
	      return false;
	    }
	}
    }


    status = cmm.Begin();
    if ( status != icCmmStatOk ) 
      {
	icc_cmm_error( _("Invalid Profile for CMM: "), status );
	return false;
      }

    unsigned src_space = cmm.GetSourceSpace();
    
    if ( !((src_space==icSigRgbData)  ||
	   (src_space==icSigGrayData) ||
	   (src_space==icSigLabData)  ||
	   (src_space==icSigXYZData)  ||
	   (src_space==icSigCmykData) ||
	   (src_space==icSigMCH4Data) ||
	   (src_space==icSigMCH5Data) ||
	   (src_space==icSigMCH6Data)) ) 
    {
	LOG_ERROR( _("Invalid source profile/image pixel format") );
	return false;
    }

    bool convert = false;
    unsigned dst_space = cmm.GetDestSpace();
    unsigned channels  = 3;
    switch( dst_space )
      {
	 case icSigXYZData:
	    if ( (flags & kXformLast) ) convert = true;  
	    // fall through - No break here
	 // case icSigGrayData:
	 case icSigRgbData:
	 case icSigLabData:
	 case icSigCmyData:
	    channels = 3; break;
	 case icSig4colorData:
	 case icSigCmykData:
	    channels = 4; break;
	 case icSig5colorData:
	    channels = 5; break;
	 case icSig6colorData:
	    channels = 6; break;
	 case icSig7colorData:
	    channels = 7; break;
	 case icSig8colorData:
	    channels = 8; break;
	 default:
	    LOG_ERROR( _("Invalid destination profile/image format") );
	    return false;
      }

    if ( channels > 3 )
    {
       LOG_WARNING( _("Destination color space has more than 3 channels - "
		      "only first 3 will be shown.") );
    }
       
    if ( dst_space != icSigRgbData && dst_space != icSigXYZData )
    {
       LOG_WARNING( _("Destination color space is not RGB or XYZ.  "
		      "Colors may look weird displayed as RGB.") );
    }

    status = cmm.Begin();
    if ( status != icCmmStatOk )
    {
       icc_cmm_error(  _("Could not init cmm: "), status );
       return false;
    }

    float* p = new float[channels];
    for (size_t i = 0; i < lut_size()/4; ++i) 
      {
	 size_t j = i*4;
	 status = cmm.Apply( p, &(pixelValues[j]) );
	 if ( status != icCmmStatOk) {
	    icc_cmm_error( _("Apply: ") , status );
	    return false;
	}
	 
	if ( convert )
	  {
	    icXyzFromPcs( p );
	    icXYZtoLab( p );
	    icLabToPcs( p );
	  }

	lut[j]   = p[0];
	lut[j+1] = p[1];
	lut[j+2] = p[2];
      }

    _inited = true;
    delete [] p;

    return true;
  }



  bool GLLut3d::calculate( 
			  GLLut3d::GLLut3d_ptr lut,
			  const Transforms::const_iterator& start,
			  const Transforms::const_iterator& end,
			  const CMedia* img,
			  const GLLut3d::XformFlags flags
			   )
  {
    switch( (*start).type )
      {
	 case Transform::kCTL:
	    return lut->calculate_ctl( start, end, img, flags );
	 case Transform::kICC:
	    return lut->calculate_icc( start, end, flags );
	 default:
	    return false;
      }
  }



  bool     GLLut3d::RT_ctl_transforms( std::string& key,
				       Transforms& transforms,
				       const CMedia* img,
				       const bool warn )
  {
    bool ok = false;
    if ( img->idt_transform() )
      {
	std::string name = img->idt_transform();
	Transform t( name, Transform::kCTL );
	if ( !key.empty() ) key += " -> ";
	key += name;
	key += " (C)";
	transforms.push_back( t );
	ok = true;
      }

    if ( img->number_of_lmts() )
      {
          size_t i = 0;
          size_t num = img->number_of_lmts();

          for ( ; i < num; ++i )
          {
              std::string name = img->look_mod_transform(i);
              Transform t( name, Transform::kCTL );
              if ( !key.empty() ) key += " -> ";
              key += name;
              key += " (C)";
              transforms.push_back( t );
              ok = true;
          }
      }

    if ( img->rendering_transform() )
      {
	std::string name = img->rendering_transform();
	Transform t( name, Transform::kCTL );
	if ( !key.empty() ) key += " -> ";
	key += name;
	key += " (C)";
	transforms.push_back( t );

	static const CMedia* lastImg = NULL;

	if ( lastImg != img )
	{  
	   if ( warn )
	   {
	      LOG_WARNING( _("RT Lut is set to prefer ICC profile but only "
			     "CTL script found in \"") 
			 << img->name() << N_("\"") );
	   }
	   lastImg = img;
	}
	ok = true;
      }

    return ok;
  }



  bool    GLLut3d::RT_icc_transforms( std::string& key,
				      Transforms& transforms,
				      const CMedia* img,
				      const bool warn )
  {
    const char* profile = img->icc_profile();
    CIccProfile* pIcc = colorProfile::get( profile );
    if ( pIcc )
      {
	Transform t( profile, Transform::kICC, 
		     (icRenderingIntent) img->rendering_intent() );
	if ( !key.empty() ) key += " -> ";
	key += profile;
	key += " (I)";
	transforms.push_back( t );
	if ( warn )
	  {
	    LOG_WARNING( _("RT Lut is set to prefer CTL but only "
			   "ICC profile found in \"") << img->name() << N_("\"") );
	  }
	return true;
      }
    return false;
  }


  
  bool    GLLut3d::ODT_ctl_transforms( std::string& key,
					Transforms& transforms,
					const CMedia* img,
					const bool warn )
  {
    if ( !mrv::Preferences::ODT_CTL_transform.empty() )
      {
	const std::string& name = mrv::Preferences::ODT_CTL_transform;
	Transform t( name, Transform::kCTL );
	if ( !key.empty() ) key += " -> ";
	key += name;
	key += " (C)";
	transforms.push_back( t );
	if ( warn )
	  {
	    LOG_WARNING( _("ODT Lut is set to prefer ICC profile but only "
			   "CTL script found in ODT.") );
	  }
	return true;
      }
    return false;
  }

    
  bool     GLLut3d::ODT_icc_transforms( std::string& key,
					Transforms& transforms,
					const CMedia* img,
					const bool warn )
  {
    if ( !mrv::Preferences::ODT_ICC_profile.empty() )
      {
	const std::string& name = mrv::Preferences::ODT_ICC_profile;
	Transform t( name, Transform::kICC );
	if ( !key.empty() ) key += " -> ";
	key += name;
	key += " (I)";
	transforms.push_back( t );
	if ( warn )
	  {
	    LOG_WARNING( _("ODT Lut is set to prefer CTL script but only "
			   "ICC profile found in ODT.") );
	  }
	return true;
      }
    return false;
  }

void GLLut3d::transform_names( GLLut3d::Transforms& t, const CMedia* img )
{
    std::string path;

    RT_ctl_transforms( path, t, img, false );
    ODT_ctl_transforms( path, t, img );
}

  GLLut3d* GLLut3d::factory( const mrv::PreferencesUI* uiPrefs, 
			     const CMedia* img )
  {
      std::string path; 
      GLLut3d::Transforms transforms;

      unsigned int algorithm = uiPrefs->RT_algorithm->value();

      bool find_ctl = (algorithm == Preferences::kLutOnlyCTL ||
                       algorithm == Preferences::kLutPreferCTL );
      bool find_icc = (algorithm == Preferences::kLutOnlyICC ||
                       algorithm == Preferences::kLutPreferICC );

      bool found;
      if ( find_ctl )
      {
	found = RT_ctl_transforms( path, transforms, img );
	if ( !found && algorithm == Preferences::kLutPreferCTL )
	  RT_icc_transforms( path, transforms, img, true );
      }
    else
      {
	found = RT_icc_transforms( path, transforms, img );
	if ( !found && algorithm == Preferences::kLutPreferICC )
	  RT_ctl_transforms( path, transforms, img, true );
      }

    if ( transforms.empty() ) 
      {
	 static const CMedia* lastImg = NULL;

	 if ( img != lastImg )
	 {

	    const char* err = N_("");
	    if ( algorithm == Preferences::kLutPreferICC ||
		 algorithm == Preferences::kLutPreferCTL )
	       err = _("No CTL script or ICC profile");
	    else if ( find_ctl )
	       err = _("No CTL script");
	    else if ( find_icc )
	       err = _("No ICC profile");
	    
	    LOG_ERROR( _("No valid RT transform for \"") 
		       << img->name() << N_("\": ") << err << N_(".") );
	 }

	lastImg = img;

	return NULL;
      }

    size_t num = transforms.size();

    algorithm = uiPrefs->ODT_algorithm->value();
    find_ctl = (algorithm == Preferences::kLutOnlyCTL ||
		algorithm == Preferences::kLutPreferCTL );
    find_icc = (algorithm == Preferences::kLutOnlyICC ||
		algorithm == Preferences::kLutPreferICC );

    if ( find_ctl )
      {
	found = ODT_ctl_transforms( path, transforms, img );
	if ( !found && algorithm == Preferences::kLutPreferCTL )
	  ODT_icc_transforms( path, transforms, img, true );
      }
    else
      {
	found = ODT_icc_transforms( path, transforms, img );
	if ( !found && algorithm == Preferences::kLutPreferICC )
	  ODT_ctl_transforms( path, transforms, img, true );
      }

    if ( transforms.size() == num )
      {
	 static const CMedia* lastImg = NULL;

	 if ( img != lastImg )
	 {
	    const char* err = N_("");
	    if ( algorithm == Preferences::kLutPreferICC ||
		 algorithm == Preferences::kLutPreferCTL )
	       err = _("No CTL script or ICC profile");
	    else if ( find_ctl )
	       err = _("No CTL script");
	    else if ( find_icc )
	       err = _("No ICC profile");

	    LOG_ERROR( _("No valid ODT transform: ") << err << N_(".") );
	 }

	lastImg = img;
	return NULL;
      }

    //
    // Check if this lut path was already calculated by some other
    // image.
    //
    {
      LutsMap::const_iterator i = _luts.find( path );
      if ( i != _luts.end() )
        {
          // this lut was already created, return it.
          return i->second.get();
        }
    }



    unsigned size = 64;
    unsigned lut_type = uiPrefs->uiLUT_quality->value();
    switch( lut_type )
      {
          case kLut32:
              size = 32; break;
          default:
          case kLut64:
              size = 64; break;
          case kLut96:
              size = 96; break;
          case kLut128:
              size = 128; break;
          case kNoBake:
              break;
      }

    //
    // Log information about lut path
    //
    LOG_INFO( _("3D Lut for ") << img->name() << N_(":") );
    LOG_INFO( path );


    GLLut3d_ptr lut( new GLLut3d(size) );



    //
    // Create Luts
    unsigned int flags = kXformFirst;

    Transforms::const_iterator i = transforms.begin();
    Transforms::const_iterator e = transforms.end();
    Transforms::const_iterator start = i;

    for ( ++i; i != e; ++i )
      {
	if ( (*i).type != (*start).type )
	  {
	    if ( ! lut->calculate( lut, start, i, img, 
				   (XformFlags)flags ) )
            {
              LOG_ERROR( "Lut calculate failed" );
	      return NULL;
            }
	    start = i;
	    flags = kXformNone;
	  }
      }

    if ( (e - start) != 0 )
      {
          flags |= kXformLast;
          if ( ! lut->calculate( lut, start, e, img, (XformFlags)flags ) )
          {
              LOG_ERROR( "Lut calculate failed" );
              return NULL;
          }
      }

    lut->create_gl_texture();

    _luts.insert( std::make_pair<std::string, GLLut3d_ptr>( path, lut ) );
    return lut.get();
  }


  void GLLut3d::clear()
  {
      _luts.clear();
  }

} // namespace mrv
