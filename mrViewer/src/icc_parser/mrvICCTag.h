/**
 * @file   mrvICCTag.h
 * @author gga
 * @date   Sat Feb 16 03:32:31 2008
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvICCTag_h
#define mrvICCTag_h

#include <iostream>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include <ImathMatrix.h>
#include <ImathVec.h>

#include "icc/mrvICCTypes.h"


namespace mrv {

  namespace icc {

    // Base abstract type for all tags
    struct Tag
    {
      Tag()  {  name[4] = 0; };
      virtual ~Tag() {}

      static bool validate( const unsigned tag_id, const unsigned type_id );

      static Tag* factory( const unsigned id,
			   const char* data, const unsigned size );

      union {
	unsigned       id;
	char      name[5];
      };

      virtual void inspect( std::ostringstream& o ) const = 0;

      inline std::string to_s() const
      {
	std::ostringstream o; o.str().reserve(1024);
	o << "id: " << name 
	  << " (" << std::hex << std::showbase << id << std::dec << ")\n";
	inspect( o );
	return o.str();
      }

      inline 
      friend std::ostream& operator<<( std::ostream& o, const Tag& b )
      {
	return o << b.to_s();
      }

      virtual void parse( const char* data, const unsigned size ) = 0;
      inline  void read( const char* data, const unsigned size )
      {
	memcpy( name, data, 4 );
	parse( data + 8, size - 8 );
      }

    };

    typedef boost::shared_ptr< Tag > Tag_ptr;


    struct ChromaticityType : public Tag   // ICC - 10.2
    { 
      enum Colorant
	{
	  kUnknown,
	  kITU_R_BT709,
	  kSMPTE_RP145_1994,
	  kEBU_Tech_3213_E,
	  kP22,
	};

      typedef std::vector< Imath::V2f > V2fList;
      unsigned short colorant_type;
      V2fList        values;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct ColorantOrderType : public Tag // ICC - 10.3
    {
      typedef std::vector< unsigned char > OrderList;
      OrderList colorants;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct ColorantTableType : public Tag // ICC - 10.4
    {
      struct Colorant {
	Colorant() { name[0] = 0; }

	char       name[32];
	Imath::V3f pcs;
      };
      typedef std::vector< Colorant > ColorantList;
      ColorantList colorants;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct CurveBase : public Tag
    {
      FloatList       values;
      virtual void inspect( std::ostringstream& o ) const;
    };
    typedef std::vector< CurveBase* > CurveList;

    struct CurveType : public CurveBase // ICC - 10.5
    {
      virtual void parse( const char* data, const unsigned size );
    };

    struct DataType  : public Tag // ICC - 10.6
    {
      unsigned type;
      unsigned size;
      char*    data;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* d, const unsigned s );
    };

    struct DateTimeType : public Tag // ICC - 10.7
    {
      DateTimeNumber date;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct Lut16Type : public Tag          // ICC - 10.8
    {
      unsigned char   num_inputs;
      unsigned char   num_outputs;
      unsigned char   grid_size;
      unsigned char   pad;
      Imath::M33f     matrix;
      unsigned short  input_table_entries;
      unsigned short  output_table_entries;
      float*          input_tables;
      float*          clut_values;
      float*          output_tables;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };


    struct Lut8Type : public Tag          // ICC - 10.9
    {
      unsigned char   num_inputs;
      unsigned char   num_outputs;
      unsigned char   grid_size;
      unsigned char   pad;
      Imath::M33f     matrix;
      unsigned short  input_table_entries;
      unsigned short  output_table_entries;
      float*          input_tables;
      float*          clut_values;
      float*          output_tables;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct LutAtoBType : public Tag        // ICC - 10.10
    {
      unsigned char   num_inputs;
      unsigned char   num_outputs;
      CurveList       B_curves;
      Imath::M44f     matrix;
      CurveList       M_curves;
      FloatList       CLUT;
      CurveList       A_curves;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct LutBtoAType : public Tag        // ICC - 10.11
    {
      unsigned char   num_inputs;
      unsigned char   num_outputs;
      CurveList       B_curves;
      Imath::M44f     matrix;
      CurveList       M_curves;
      FloatList       CLUT;
      CurveList       A_curves;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct MeasurementType : public Tag // ICC - 10.12
    {
      enum Observer
	{
	  kUnknownObserver,
	  kCIE_1931,
	  kCIE_1964,
	};

      enum Geometry
	{
	  kUnknownGeometry,
	  k0_45,
	  k0_d
	};

      unsigned   observer;
      Imath::V3f backing;
      unsigned   geometry;
      float      flare;
      Illuminant illuminant;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };


    struct MultiLocalizedUnicodeType : public Tag  // ICC - 10.13
    {
      struct UnicodeText {
	unsigned short language;  // ISO-639
	unsigned short country;   // ISO-3166
	std::string    text;
      };

      typedef std::vector< UnicodeText* >  UnicodeTextList;
      UnicodeTextList  texts;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };


    struct NamedColor2Type : public Tag  // ICC - 10.14
    {
      unsigned int vendor_flag;
      unsigned int num_colors;
      unsigned int num_coords;
      char         prefix[32];
      char         suffix[32];

      struct NamedColor
      {
	char         name;
	Imath::V3f   CIExyz;
	std::vector< unsigned > coords;
      };
      typedef std::vector< NamedColor* > NamedColorList;
      
      NamedColorList colors;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct ParametricCurveType : public CurveBase // ICC - 10.15
    {
      enum Type
	{
	  kGamma,
	  kCIE122_1966,
	  kIEC_61966_3,
	  ksRGB,
	  kUnnamed
	};

      unsigned short  type;
      FloatList       params;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct ProfileDescriptionType : public Tag  // NOT ICC v4.2 compliant!
    {
      std::string ascii;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct ProfileSequenceDescType : public Tag  // ICC 10.16
    {
      struct ProfileDescription 
      {
	ProfileDescription() : 
	  deviceMfgDesc(NULL), deviceModelDesc(NULL)
	{
	}

	~ProfileDescription()
	{
	  delete deviceMfgDesc;
	  delete deviceModelDesc;
	}

	unsigned int manufacturer;
	unsigned int model;
	unsigned int attributes[2];
	unsigned int technology;
	TagTable*    deviceMfgDesc;
	TagTable*    deviceModelDesc;
      };

      typedef std::vector< ProfileDescription > ProfileDescriptionList;

      ProfileDescriptionList descriptions;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };


    struct ResponseCurveSet16Type : public Tag    // ICC 10.17
    {

      unsigned short num_channels;


      struct Curve
      {
	Curve() { name[4] = 0; }

	union {
	  unsigned int id;
	  char         name[5];
	};

	typedef std::vector< Imath::V3f > V3fList;
	V3fList             values;
	typedef std::vector< response16Number > response16List;
	typedef std::vector< response16List   > response16ListArray;
	response16ListArray responses;

	void inspect( std::ostringstream& o ) const;

	const char* parse( const unsigned short num_measurements,
			   const unsigned short num_channels,
			   const char* data, const unsigned size );
      };

      typedef std::vector< Curve > CurveList;
      CurveList curves;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct S15Fixed16ArrayType : public Tag    // ICC 10.18
    {
      FloatList values;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct SignatureType : public Tag     // ICC 10.19
    {
      union {
	unsigned int id;
	char name[5];
      };

      SignatureType() { name[4] = 0; }

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };


    struct TextType : public Tag    // ICC 10.20
    {
      char* text;

      TextType() : text(NULL) {}

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct U16Fixed16ArrayType : public Tag    // ICC 10.21
    {
      FloatList values;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct UInt16ArrayType : public Tag    // ICC 10.22
    {
      UInt16List values;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct UInt32ArrayType : public Tag    // ICC 10.23
    {
      UInt32List values;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct UInt64ArrayType : public Tag    // ICC 10.24
    {
      UInt64List values;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct UInt8ArrayType : public Tag    // ICC 10.25
    {
      UInt8List values;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };

    struct ViewingConditionsType : public Tag    // ICC 10.26
    {
      Imath::V3f illuminant;
      Imath::V3f surround;
      Illuminant illuminant_type;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };


    struct XYZType : public Tag   // ICC 10.27
    {
      typedef std::vector< Imath::V3f > V3fList;
      V3fList values;

      virtual void inspect( std::ostringstream& o ) const;
      virtual void parse( const char* data, const unsigned size );
    };


    struct UnknownTagType : public Tag
    {
      virtual void inspect( std::ostringstream& o ) const {};
      virtual void parse( const char* data, const unsigned size ) {};
    };

  }

}

#endif // mrvICCTag_h
