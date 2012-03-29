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

#include <Iex.h>
#include <CtlExc.h>
#include <half.h>
#include <ImfArray.h>
#include <ImfHeader.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfVecAttribute.h>
#include <ImfStandardAttributes.h>

#include <SampleICC/IccProfile.h>
#include <SampleICC/IccCmm.h>


#include "ctlToLut.h"
#include "mrvIO.h"
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
    lut.resizeErase( lut_size() );
    for ( size_t i = 0; i < lut_size(); ++i )
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

    for ( size_t i = 0; i < lut_size(); ++i )
      {
	if ( lut[i] >= HALF_MIN && lut[i] <= HALF_MAX )
	  {
	    //
	    // lut[i] is finite and positive.
	    //

	    lut[i] = log (lut[i]);
	  }
	else
	  {
	    //
	    // lut[i] is zero, negative or not finite;
	    // log (lut[i]) is undefined.
	    //

	    lut[i] = log (HALF_MIN);
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
		  GL_RGBA16F_ARB,	        // internalFormat
		  _lutN, _lutN, _lutN,	// width, height, depth
		  0,			// border
		  GL_RGBA,		// format
		  GL_HALF_FLOAT_ARB,	// type
		  (char *) &lut[0] );
  }



  template< typename T >
  void GLLut3d::init_pixel_values( Imf::Array< T >& pixelValues )
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

    float logLutMin = log (lutMin);
    float logLutMax = log (lutMax);

    lutM = 1 / (logLutMax - logLutMin);
    lutT = -lutM * logLutMin;

    //
    // Build a 3D array of RGB input pixel values.
    // such that R, G and B are between lutMin and lutMax.
    //
    for (size_t ib = 0; ib < _lutN; ++ib)
      {
	float b = float(ib / (_lutN - 1.0));
	half B = exp ((b - lutT) / lutM);

	for (size_t ig = 0; ig < _lutN; ++ig)
	  {
	    float g = float(ig / (_lutN - 1.0));
	    half G = exp ((g - lutT) / lutM);

	    for (unsigned int ir = 0; ir < _lutN; ++ir)
	      {
		float r = float(ir / (_lutN - 1.0));
		half R = exp ((r - lutT) / lutM);

		size_t i = (ib * _lutN * _lutN + ig * _lutN + ir) * 4;
		pixelValues[i + 0] = R;
		pixelValues[i + 1] = G;
		pixelValues[i + 2] = B;
	      }
	  }
      }
  }



  bool GLLut3d::calculate_ctl( 
			      const Transforms::const_iterator& start,
			      const Transforms::const_iterator& end,
			      const Imf::Header& header,
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
    half* inited = lut;

    Imf::Array<half> pixelValues ( lut_size() );
    if ( !_inited )
      {
	 clear_lut();
	 init_pixel_values( pixelValues );
      }
    else
      {
	half* dst = pixelValues;
	memcpy( dst, inited, lut_size() * sizeof(half) );
      }


    //
    // Generate output pixel values by applying CTL transforms
    // to the pixel values.  (If the CTL transforms fail to
    // write to the output values, zero-initialization, above,
    // causes the displayed image to be black.)
    //
    const char** channelNames;

    static const char* RGBchannels[3] = { N_("R"), N_("G"), N_("B") };
    static const char* XYZchannels[3] = { N_("X_OCES"), N_("Y_OCES"), 
					  N_("Z_OCES") };

    if ( flags & kXformFirst || (!(flags & kXformLast)) )
      {
	channelNames = RGBchannels;
      }
    else
      {
	channelNames = XYZchannels;
      }

    TransformNames transformNames;
    Transforms::const_iterator i = start;
    for ( ; i != end; ++i )
      {
	transformNames.push_back( (*i).name );
      }

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
    half* inited = lut;

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
	int j = i*4;
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
			  GLLut3d::GLLut3d_ptr& lut,
			  const Transforms::const_iterator& start,
			  const Transforms::const_iterator& end,
			  const Imf::Header& header,
			  const GLLut3d::XformFlags flags
			   )
  {
    switch( (*start).type )
      {
      case Transform::kCTL:
	return lut->calculate_ctl( start, end, header, flags );
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
    if ( img->look_mod_transform() )
      {
	std::string name = img->look_mod_transform();
	Transform t( name, Transform::kCTL );
	if ( !key.empty() ) key += " -> ";
	key += name;
	key += " (C)";
	transforms.push_back( t );
	ok = true;
      }

    if ( img->rendering_transform() )
      {
	std::string name = img->rendering_transform();
	Transform t( name, Transform::kCTL );
	if ( !key.empty() ) key += " -> ";
	key += name;
	key += " (C)";
	transforms.push_back( t );
	if ( warn )
	  {
	    LOG_WARNING( _("RT Lut is set to prefer ICC profile but only "
			   "CTL script found in \"") 
			 << img->name() << N_("\"") );
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
        const char* err = N_("");
	if ( algorithm == Preferences::kLutPreferICC ||
	     algorithm == Preferences::kLutPreferCTL )
	  err = _("No CTL script or ICC profile");
	else if ( find_ctl )
	  err = _("No CTL script");
	else if ( find_icc )
	  err = _("No ICC profile");

	LOG_ERROR( _("No valid RT transform for \"") 
		   << img->name() << _("\": ") << err << N_(".") );
	return NULL;
      }

    unsigned num = transforms.size();

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
        const char* err = N_("");
	if ( algorithm == Preferences::kLutPreferICC ||
	     algorithm == Preferences::kLutPreferCTL )
	  err = _("No CTL script or ICC profile");
	else if ( find_ctl )
	  err = _("No CTL script");
	else if ( find_icc )
	  err = _("No ICC profile");

	LOG_ERROR( _("No valid ODT transform: ") << err << N_(".") );
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


    Imf::Header header( img->width(), img->height(),
			img->pixel_ratio() );
    Imf::addChromaticities( header, img->chromaticities() );

    unsigned size = 64;
    switch( uiPrefs->uiLUT_quality->value() )
      {
      case kLut32:
	size = 32; break;
      case kLut64:
	size = 64; break;
      case kLut128:
	size = 128; break;
      case kNoBake:
	break;
      }

    GLLut3d_ptr lut( new GLLut3d(size) );


    //
    // Log information about lut path
    //
    LOG_INFO( _("3D Lut for ") << img->name() << N_(":") );
    LOG_INFO( path );


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
	    if ( ! lut->calculate( lut, start, i, header, 
				   (XformFlags)flags ) )
	      return NULL;

	    start = i;
	    flags = kXformNone;
	  }
      }

    if ( (e - start) != 0 )
      {
	flags |= kXformLast;
	if ( ! lut->calculate( lut, start, e, header, (XformFlags)flags ) )
	  return NULL;
      }


    lut->create_gl_texture();

    _luts.insert( std::make_pair( path, lut ) );
    return lut.get();
  }


  void GLLut3d::clear()
  {
    _luts.clear();
  }

} // namespace mrv
