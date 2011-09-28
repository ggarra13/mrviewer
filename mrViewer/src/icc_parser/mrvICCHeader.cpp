/**
 * @file   mrvICCHeader.cpp
 * @author gga
 * @date   Sat Feb 16 14:01:25 2008
 * 
 * @brief  
 * 
 * 
 */


#include <iostream>
#include <iomanip>


#if defined(WIN32) || defined(WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>  // ntohl / ntohs
#endif

#include "mrvICCWhitePoint.h"
#include "mrvICCHeader.h"
#include "mrvICCIds.h"
#include "core/mrvI8N.h"


namespace mrv {
  namespace icc {

    Header::Header( const char* file ) :
      _filename( file )
    {
    }

    void Header::parse( const char* data, const size_t size )
    {
      _size = ntohl( *((unsigned*)data) );
      memcpy( _cmm.name, data + 4, 4 );
      _version = ntohl( *((unsigned*) (data + 8)) );
      _major_version = _version >> 24;
      _minor_version = (_version << 8)  >> 28;
      _teeny_version = (_version << 16) >> 28;
      memcpy( _type.name,            data + 12, 4 );
      memcpy( _colorspace.name,      data + 16, 4 );
      memcpy( _pcs.name, data + 20, 4 );
      _date.parse( data + 24, 12 );
      memcpy( _magic.name,           data + 36, 4 );
      memcpy( _platform.name,        data + 40, 4 );
      memcpy( _manufacturer.name,    data + 48, 4 );
      memcpy( _device_model.name,    data + 52, 4 );
      _device_attrs[0] = *((unsigned*) (data + 56));
      _device_attrs[1] = *((unsigned*) (data + 60));
      _rendering_intent = ntohl( *((unsigned*) (data + 64)) );
      XYZNumber* tmp = (XYZNumber*) (data + 68);
      _illuminant = tmp->to_v3f();
      _illuminant = kD50WhitePoint;
      memcpy( _creator.name, data + 80, 4 );
      _md5checksum = ntohl( *((unsigned*) (data+84) ) );
    }

    std::ostream& operator<<( std::ostream& o, const Header& b )
    {
      o << _("Filename:\t") << b._filename << std::endl
	<< _("CMM:\t\t") << b._cmm << std::endl
	<< _("Version:\t") << b._major_version << N_(".") 
	<< b._minor_version << N_(".") << b._teeny_version 
	<< " (" << std::hex << std::showbase << b._version << N_(")") 
	<< std::endl
	<< _("Type:\t\t") << b._type << std::endl
	<< _("DCS:\t\t") << b._colorspace << std::endl
	<< _("PCS:\t\t") 
	<< b._pcs << std::endl
	<< _("Date:\t\t") << b._date << std::endl
	<< _("Magic Number:\t") << b._magic << std::endl
	<< _("Platform:\t") << b._platform << std::endl
	<< _("Manufacturer:\t") << b._manufacturer << std::endl
	<< _("Device Model:\t") << b._device_model << std::endl
	<< _("Device Attrs:\t") << b._device_attrs[0] << std::endl
	<< _("Render Intent:\t");

      switch( b._rendering_intent )
	{
	case Header::kPerceptual:
	  o << _("Perceptual"); break;
	case Header::kMediaRelative:
	  o << _("Media-Relative Colorimetric"); break;
	case Header::kSaturation:
	  o << _("Saturation"); break;
	case Header::kICCAbsoluteColorimetric:
	  o << _("ICC Absolute Colorimetric"); break;
	default:
	  o << _("Unknown"); break;
	}


      return o << " (" 
	       << std::hex << std::showbase 
	       << b._rendering_intent << std::dec
	       << N_(")") << std::endl 
	       << _("PCS Illuminant:\t") 
	       << b._illuminant << std::endl
	       << _("Creator:\t") << b._creator << std::endl
	       << _("Checksum:\t") << b._md5checksum << std::endl;
    }

  }
}
