/**
 * @file   mrvICCTypes.h
 * @author gga
 * @date   Sat Feb 16 03:34:41 2008
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvICCTypes_h
#define mrvICCTypes_h

#include <cstring>
#include <vector>

#include <boost/cstdint.hpp>

#include <ImathVec.h>


namespace mrv {

  namespace icc {

    struct Id
    {
      Id() { name[4] = 0; }
      Id( const unsigned tag ) : id(tag) { name[4] = 0; }
      Id( const char* n );

      union {
	unsigned id;
	char name[5];
      };

      inline bool operator<( const Id& b ) const
      {
	return ( id < b.id );
      }

      inline bool operator==( const Id& b ) const
      {
	return ( id == b.id ); 
      }

      inline bool operator==( const char* b ) const
      {
	return ( strcmp( name, b ) == 0 ); 
      }

      friend
      std::ostream& operator<<( std::ostream& o, const Id& b );

    };


    struct DateTimeNumber
    {
      unsigned short year;
      unsigned short month;
      unsigned short day;
      unsigned short hours;
      unsigned short minutes;
      unsigned short seconds;

      void parse( const char* data, const unsigned size );
      
      friend
      std::ostream& operator<<( std::ostream& o, const DateTimeNumber& b );
    };

    struct s15Fixed16Number
    {
      short num;
      unsigned short frac;   //  / 65536

      void parse( const char* data, const unsigned size );

      friend
      std::ostream& operator<<( std::ostream& o, const s15Fixed16Number& b );

      inline float to_f() const
      {
	return (float) num + (float) frac / 65536.0f;
      } 
    };

    struct u16Fixed16Number
    {
      unsigned short num;
      unsigned short frac;   //  / 65536

      void parse( const char* data, const unsigned size );

      inline float to_f() const
      {
	return (float) num + (float) frac / 65536.0f;
      } 
    };

    struct u1Fixed15Number
    {
      unsigned char   num:1;
      unsigned short frac:15;   //  / 32768

      inline float to_f() const
      {
	return (float) num + (float) frac / 32768.0f;
      } 
    };

    struct u8Fixed8Number
    {
      unsigned char num;
      unsigned char frac;   //  / 256

      inline float to_f() const
      {
	return (float) num + (float) frac / 256.0f;
      } 
    };

    struct uInt16Number
    {
      unsigned short num;

      void parse( const char* data, const unsigned size );
      inline float to_f() const
      {
	return ( (float) num / 65535.0f );
      }
    };



    struct XYZNumber
    {
      s15Fixed16Number x;
      s15Fixed16Number y;
      s15Fixed16Number z;

      inline void parse( const char* data, const unsigned size )
      {
	x.parse( data, size );
	y.parse( data + 4, size-4 );
	z.parse( data + 8, size-8 );
      }

      inline Imath::V3f to_v3f()
      {
	return Imath::V3f( x.to_f(), y.to_f(), z.to_f() );
      }
    };



    struct Illuminant
    {
      enum Type
	{
	  kUnknown,
	  kD50,
	  kD65,
	  kD93,
	  kF2,
	  kD55,
	  kA,
	  kEquiPower,
	  kF8,
	};

      unsigned value;

      Illuminant() : value(kUnknown)                    {}
      Illuminant( const unsigned type ) : value( type ) {}

      Illuminant& operator=( const unsigned type )
      {
	value = type;
	return *this;
      }

      const char* type_name() const
      {
	switch( value )
	  {
	  case kD50:
	    return "D50";
	  case kD65:
	    return "D65";
	  case kD93:
	    return "D93";
	  case kF2:
	    return "F2";
	  case kD55:
	    return "D55";
	  case kA:
	    return "A";
	  case kEquiPower:
	    return "EquiPower";
	  case kF8:
	    return "F8";
	  case kUnknown:
	  default:
	    return "unknown";
	  }
      }

      std::string to_s() const
      {
	char buf[128];
	sprintf( buf, "%s (%0x)", type_name(), value );
	return buf;
      }
    };


    struct response16Number
    {
      unsigned short   num;
      float            measurement;
	
      void inspect( std::ostringstream& o ) const
      {
	o << num << " -> " << measurement;
      }

      void parse( const char* data, const unsigned size );
    };

    typedef std::vector< float  >          FloatList;
    typedef std::vector< unsigned char  >  UInt8List;
    typedef std::vector< unsigned short >  UInt16List;
    typedef std::vector< unsigned int   >  UInt32List;
    typedef std::vector< boost::uint64_t > UInt64List;


  } // namespace icc

} // namespace mrv


#endif // mrvICCTypes_h
