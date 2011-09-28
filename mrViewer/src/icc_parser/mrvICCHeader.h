/**
 * @file   mrvICCHeader.h
 * @author gga
 * @date   Sat Feb 16 14:00:30 2008
 * 
 * @brief  
 * 
 * 
 */


#ifndef mrvICCHeader_h
#define mrvICCHeader_h

#include <string>

#include <ImathVec.h>

#include "mrvICCTypes.h"

namespace mrv {

  namespace icc {

    class Header
    {
    public:
      enum RenderingIntent
	{
	  kPerceptual,
	  kMediaRelative,
	  kSaturation,
	  kICCAbsoluteColorimetric
	};

    public:
      Header( const char* file );
      void parse( const char* data, const size_t size );

      unsigned type() const { return _type.id; }

      const std::string& filename() const { return _filename; }

      Imath::V3f  illuminant() const { return _illuminant; }

      friend
      std::ostream& operator<<( std::ostream& o, const Header& b );

    protected:
      std::string    _filename;
      unsigned       _size;            // ICC - 7.2.2
      Id             _cmm;             // ICC - 7.2.3
      unsigned       _version;         // ICC - 7.2.4
      unsigned short _major_version;
      unsigned short _minor_version;
      unsigned short _teeny_version;
      Id             _type;            // ICC - 7.2.5
      Id             _colorspace;      // ICC - 7.2.6
      Id             _pcs;             // ICC - 7.2.7
      DateTimeNumber _date;               // ICC - 7.2.8
      Id             _magic;           // ICC - 7.2.9
      Id             _platform;        // ICC - 7.2.10
      unsigned       _flags;              // ICC - 7.2.11
      Id             _manufacturer;     // ICC - 7.2.12
      Id             _device_model;     // ICC - 7.2.13
      unsigned       _device_attrs[2];     // ICC - 7.2.14
      unsigned       _rendering_intent;    // ICC - 7.2.15
      Imath::V3f     _illuminant;    // ICC - 7.2.16
      Id             _creator;          // ICC - 7.2.17
      unsigned       _md5checksum;         // ICC - 7.2.18
    };

  }
}


#endif // mrvICCHeader_h
