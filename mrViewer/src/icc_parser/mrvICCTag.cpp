/**
 * @file   mrvICCTag.cpp
 * @author gga
 * @date   Sat Feb 16 03:40:04 2008
 * 
 * @brief  
 * 
 * 
 */



#if defined(WIN32) || defined(WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>  // ntohl / ntohs
#endif

#include "mrvIO.h"
#include "mrvException.h"
#include "mrvICCTagTable.h"
#include "mrvICCTag.h"
#include "mrvICCIds.h"

namespace {
  const char* kModule = "icc";
}

namespace mrv {

  namespace icc {

    void DateTimeNumber::parse( const char* data, const unsigned size )
      {
	if ( size != sizeof(DateTimeNumber) )
	  {
	    LOG_ERROR( _("DateTimeNumber: wrong data size") );
	    return;
	  }
	year    = ntohs( *((unsigned short*)  data) );
	month   = ntohs( *((unsigned short*) (data+2)) );
	day     = ntohs( *((unsigned short*) (data+4)) );
	hours   = ntohs( *((unsigned short*) (data+6)) );
	minutes = ntohs( *((unsigned short*) (data+8)) );
	seconds = ntohs( *((unsigned short*) (data+10)) );
	if ( year < 1920 )
	  LOG_ERROR( _("DateTimeNumber: Invalid year ") << year);
	if ( month < 1 || month > 12 )
	  LOG_ERROR( _("DateTimeNumber: Invalid month ") << month);
	if ( day < 1 || day > 31 )
	  LOG_ERROR( _("DateTimeNumber: Invalid day ") << day);
	if ( hours > 23 )
	  LOG_ERROR( _("DateTimeNumber: Invalid hours ") << hours);
	if ( minutes > 59 )
	  LOG_ERROR( _("DateTimeNumber: Invalid minutes ") << minutes);
	if ( seconds > 59 )
	  LOG_ERROR( _("DateTimeNumber: Invalid seconds ") << seconds);
      }

    void response16Number::parse( const char* data, const unsigned size )
    {
      num = ntohs( *((unsigned short*) data) );
      s15Fixed16Number tmp;
      tmp.parse( data+4, size - 4 );
      measurement = tmp.to_f();
    }

    void
    ChromaticityType::inspect( std::ostringstream& o ) const
    {
      o << _("\t# of channels: ") << values.size() << std::endl
	<< _("\tColorant: ");
      switch( colorant_type )
	{
	case kITU_R_BT709:
	  o << N_("ITU-R BT.709"); break;
	case kSMPTE_RP145_1994:
	  o << N_("SMPTE RP 145-1994"); break;
	case kEBU_Tech_3213_E:
	  o << N_("EBU Tech. 3213-E"); break;
	case kP22:
	  o << N_("P22"); break;
	case kUnknown:
	default:
	  o << _("Unknown");
	}
      o << std::endl;
      V2fList::const_iterator i = values.begin();
      V2fList::const_iterator e = values.end();
      for ( ; i != e; ++i )
	{
	  o << *i << " ";
	}
    }

    void
    ChromaticityType::parse( const char* data, const unsigned size )
    {
      if ( size < 4 )
	{
	  LOG_ERROR( _("Chromaticity: missing data") );
	  return;
	}

      unsigned short num = ntohs( *((unsigned short*)  data)   );
      colorant_type = ntohs( *((unsigned short*) (data+2)) );

      values.resize( num );

      if ( size != 4 + num * sizeof(u16Fixed16Number) )
	{
	  LOG_ERROR( _("Chromaticity: wrong amount of data") );
	  return;
	}

      const char* d = data + 4;
      u16Fixed16Number tmp;
      for ( unsigned i = 0; i < num; ++i, d += sizeof(u16Fixed16Number) )
	{
	  tmp.parse(d, size - (d-data) );
	  values[i].x = tmp.to_f();
	  d += sizeof(u16Fixed16Number);

	  tmp.parse(d, size - (d-data) );
	  values[i].y = tmp.to_f();
	}
    }

    void
    ColorantOrderType::inspect( std::ostringstream& o ) const
    {
      o << _("\tOrder: ");
      OrderList::const_iterator i = colorants.begin();
      OrderList::const_iterator e = colorants.end();
      for ( ; i != e; ++i )
	{
	  o << ((short)(*i)) << " ";
	}
    }

    void
    ColorantOrderType::parse( const char* data, const unsigned size )
    {
      if ( size < 4 )
	{
	  LOG_ERROR( _("colorantOrder: missing data") );
	  return;
	}
      unsigned num = ntohl( *((unsigned*) data) );
      colorants.resize( num );
      memcpy( &colorants[0], data+4, size - 4 );
    }

    void
    ColorantTableType::inspect( std::ostringstream& o ) const
    {
      o << _("\tColorants: ") << std::endl;
      ColorantList::const_iterator i = colorants.begin();
      ColorantList::const_iterator e = colorants.end();
      for ( ; i != e; ++i )
	{
	  const Colorant& c = *i;
	  o << "\t\t" << c.name << ": " << c.pcs << std::endl;
	}
    }

    void
    ColorantTableType::parse( const char* data, const unsigned size )
    {
      if ( size < 4 )
	{
	  LOG_ERROR( _("colorantTable: missing data") );
	  return;
	}
      unsigned num = ntohl( *((unsigned*) data) );
      colorants.resize( num );

      unsigned expected = 4 + num * 38;
      if ( expected != size )
	{
	  LOG_ERROR( _("colorantTable: Invalid size.") );
	  if ( size < expected )
	    {
	      expected = (size - 4) / 38;
	    }
	}
      const char* d = data + 4;
      uInt16Number tmp;
      for ( unsigned i = 0; i < num; ++i )
	{
	  memcpy( colorants[i].name, d, 32 ); d += 32;
	  tmp.parse( d, size - (d-data) ); d += 2;
	  colorants[i].pcs.x = tmp.to_f();
	  tmp.parse( d, size - (d-data) ); d += 2;
	  colorants[i].pcs.y = tmp.to_f();
	  tmp.parse( d, size - (d-data) ); d += 2;
	  colorants[i].pcs.z = tmp.to_f();
	}
    }

    void
    CurveBase::inspect( std::ostringstream& o ) const
    {
      FloatList::const_iterator i = values.begin();
      FloatList::const_iterator e = values.end();
      o << '\t';
      for ( ; i != e; ++i )
	o << *i << " ";
    }

    void CurveType::parse( const char* data, const unsigned size )
    {
      if ( size < 4 )
	{
	  LOG_ERROR( _("curveType: missing data") );
	  return;
	}
      unsigned num = ntohl( *((unsigned*)data) );

      values.resize(num, 0.0f);
      if (num == 0) return;

      if ( num == 1 )
	{
	  if ( size != (4 + sizeof(u8Fixed8Number)) )
	    LOG_ERROR( _("curveType: Invalid curve (gamma) size, "
			 "should be 8") );

	  // A gamma value as y = x ^ VALUE  (not an inverse)
	  u8Fixed8Number* tmp = (u8Fixed8Number*)(data + 4);
	  values[0] = tmp->to_f();
	}
      else
	{
	  unsigned expected_size = (4 + sizeof(uInt16Number) * num);
	  if ( size != expected_size )
	    LOG_ERROR( _("curveType: Invalid curve size ") 
		       << size << _(" should be ") 
		       << expected_size << _(" for ") << num << _(" values") );
	  if ( size < expected_size )
	    EXCEPTION( _("curveType: Missing data") );

	  uInt16Number tmp;
	  const char* d = (data+4);
	  for ( unsigned i = 0; i < num; ++i, d += sizeof(short) )
	    {
	      tmp.parse( d, size - (d-data) );
	      values[i] = tmp.to_f(); 
	    }
	}
    }

    void
    DataType::inspect( std::ostringstream& o ) const
    {
      o << '\t';
      if ( type == 0 )
	o << data;
      else
	o << _("binary data, size: ") << size;
    }

    void
    DataType::parse( const char* d, const unsigned s )
    {
      if ( s < 4 )
	{
	  LOG_ERROR( _("dataType: missing data") );
	  return;
	}
      type = ntohl( *((unsigned*) d) );
      size = s - 4;
      data = new char[size];
      memcpy( data, d, size );
    }

    void
    DateTimeType::inspect( std::ostringstream& o ) const
    {
      o << "\t" << date;
    }

    void
    DateTimeType::parse( const char* data, const unsigned size )
    {
      date.parse( data, size );
    }

    void
    Lut16Type::inspect( std::ostringstream& o ) const
    {
      o << '\t';
    }

    void
    Lut16Type::parse( const char* data, const unsigned size )
    {
    }

    void
    Lut8Type::inspect( std::ostringstream& o ) const
    {
      o << '\t';
    }

    void
    Lut8Type::parse( const char* data, const unsigned size )
    {
    }

    void
    LutAtoBType::inspect( std::ostringstream& o ) const
    {
      o << '\t';
    }

    void
    LutAtoBType::parse( const char* data, const unsigned size )
    {
    }

    void
    LutBtoAType::inspect( std::ostringstream& o ) const
    {
      o << '\t';
    }

    void
    LutBtoAType::parse( const char* data, const unsigned size )
    {
    }

    void
    MeasurementType::inspect( std::ostringstream& o ) const
    {
      o << _("\tObserver: ");
      switch( observer )
	{
	case kCIE_1931:
	  o << N_("CIE 1931"); break;
	case kCIE_1964:
	  o << N_("CIE 1964"); break;
	case kUnknownObserver:
	default:
	  o << _("Unknown"); break;
	}
      o << std::endl
	<< _("\tBacking: ") << backing << std::endl
	<< _("\tGeometry: ");
      switch( geometry )
	{
	case k0_45:
	  o << _("0/45 or 45/0");
	  break;
	case k0_d:
	  o << _("0/d or d/0");
	  break;
	case kUnknownGeometry:
	default:
	  o << _("Unknown");
	  break;
	}
      o << std::endl
	<< _("\tFlare: ") << flare << std::endl
	<< _("\tIlluminant: ") << illuminant.to_s();
    }

    void
    MeasurementType::parse( const char* data, const unsigned size )
    {
      static const unsigned expected = 27;
      if ( size != expected )
	{
	  LOG_ERROR( _("measurementType: size ") << size 
		     << _(" is not ") << expected << "." );
	  if ( size < expected )
	    {
	      memset( this, 0, sizeof( MeasurementType ) );
	      return;
	    }
	}

      observer = ntohl( *((unsigned*) data) );
      XYZNumber tmp;
      tmp.parse( data + 4, size - 4 );
      backing  = tmp.to_v3f();
      geometry = ntohl( *((unsigned*) (data + 16)) );
      u16Fixed16Number tmp2;
      tmp2.parse( data + 20, size );
      flare = tmp2.to_f();
      illuminant = ntohl( *((unsigned*) (data + 24)) );
    }

    void
    MultiLocalizedUnicodeType::inspect( std::ostringstream& o ) const
    {
      o << '\t';
    }

    void
    MultiLocalizedUnicodeType::parse( const char* data, const unsigned size )
    {
    }

    void
    NamedColor2Type::inspect( std::ostringstream& o ) const
    {
      o << '\t';
    }

    void
    NamedColor2Type::parse( const char* data, const unsigned size )
    {
    }

    void ParametricCurveType::inspect( std::ostringstream& o ) const
    {
      o << _("\tType: ");
      switch( type )
	{
	case kGamma:
	  o << _("Gamma");
	case kCIE122_1966:
	  o << _("CIE122 1966"); break;
	case kIEC_61966_3:
	  o << _("IEC 6166-3"); break; 
	case ksRGB:
	  o << _("sRGB"); break;
	case kUnnamed:
	  o << _("Unnamed Formula"); break;
	default:
	  o << _("Unknown"); break;
	}
      o << std::endl << _("\tParameters: ");
      FloatList::const_iterator i = params.begin();
      FloatList::const_iterator e = params.end();
      for ( ; i != e; ++i )
	{
	  o << *i << " ";
	}
      o << std::endl << _("\tSampled:") << std::endl;
      CurveBase::inspect( o );
    }

    void ParametricCurveType::parse( const char* data, const unsigned size )
    {
      if ( size < 2 )
	{
	  LOG_ERROR( _("parametricCurve: missing data") );
	  values.clear();  params.clear();
	  return;
	}
      type = ntohs( *((unsigned*)data) );

      unsigned num = 1;
      switch( type )
	{
	case kGamma:
	  num = 1; break;
	case kCIE122_1966:
	  num = 3; break;
	case kIEC_61966_3:
	  num = 4; break; 
	case ksRGB:
	  num = 5; break;
	case kUnnamed:
	  num = 7; break;
	default:
	  LOG_ERROR( _("Unknown parametric curve type") );
	  num = 1; 
	  break;
	}

      params.resize(num, 0.0f);

      unsigned expected_size = (2 + sizeof(uInt16Number) * num);
      if ( size != expected_size )
	LOG_ERROR( _("parametricCurve: Invalid curve size ")
		   << size << _(" should be ") 
		   << expected_size << _(" for ") << num << _(" values") );
      if ( size < expected_size )
	EXCEPTION( _("parametricCurve: Missing data") );

      s15Fixed16Number tmp;
      const char* d = data + 2;
      for ( unsigned i = 0; i < num; ++i, d += sizeof(s15Fixed16Number) )
	{
	  tmp.parse( d, size - (d - data) );
	  params[i+1] = tmp.to_f();
	}

      // Sample the function into discrete steps
      const unsigned steps = 64;
      values.resize( steps, 0 );
      float xMin = 0.0f;
      float xMax = 4.0f;
      float step = xMax - xMin / steps;
      float x = xMin;
      for ( unsigned i = 0; i < steps; ++i, x += step )
	{
	  float val = 0.0f;
	  switch( type )
	    {
	    case kGamma:
	      val = pow( x, params[0] ); break;
	    case kCIE122_1966:
	      if ( x >= -params[1]/params[0] )
		{
		  val = pow( params[1] * x + params[2], params[0] ); 
		}
	      else
		{
		  val = 0.0f;
		}
	      break;
	    case kIEC_61966_3:
	      if ( x >= -params[1]/params[0] )
		{
		  val  = pow( params[1] * x + params[2], params[0] );
		  val += params[3];
		}
	      else
		{
		  val = params[3];
		}
	      break; 
	    case ksRGB:
	      if ( x >= params[4] )
		{
		  val  = pow( params[1] * x + params[2], params[0] );
		}
	      else
		{
		  val = params[3] * x;
		}
	      break;
	    case kUnnamed:
	      if ( x >= params[4] )
		{
		  val  = pow( params[1] * x + params[2], params[0] );
		  val += params[5];
		}
	      else
		{
		  val = params[3] * x + params[6];
		}
	      break;
	    default:
	      break;
	    }
	  values[i] = val;
	}
    }

    void ProfileDescriptionType::inspect( std::ostringstream& o ) const
    {
      o << '\t' << ascii;
    }

    void ProfileDescriptionType::parse( const char* data, const unsigned size )
    {
      unsigned num = ntohl( *((unsigned*)data) );
      if ( num > size ) num = size;

      const char* d = data + 4;
      ascii = d;

      if ( num < size )
	{
	  // skip unicode, scriptcode
	}

      
    }

    void ProfileSequenceDescType::inspect( std::ostringstream& o ) const
    {
      o << '\t';
    }

    void ProfileSequenceDescType::parse( const char* data, const unsigned size )
    {
      unsigned num = ntohl( *((unsigned*)data) );
      descriptions.resize(num);

      const char* d = data + 4;
      for ( unsigned i = 0; i < num; ++i )
	{
	  if ( d + 16 >= data + size )
	    {
	      LOG_ERROR( _("profileSequenceDesc: Too little data"
			   "for description") );
	      break;
	    }
	  descriptions[i].manufacturer  = ntohl( *((unsigned*)d) );
	  descriptions[i].model         = ntohl( *((unsigned*)d + 4)  );
	  descriptions[i].attributes[0] = ntohl( *((unsigned*)d + 8)  );
	  descriptions[i].attributes[1] = ntohl( *((unsigned*)d + 12) );
	  descriptions[i].technology    = ntohl( *((unsigned*)d + 16) );
	  d += 16;

	  if ( d + 16 >= data + size )
	    {
	      TagTable* table = new TagTable( d );
	      d += 16;

	      if ( table->id == 0x646D6E64 )
		descriptions[i].deviceMfgDesc = table;
	      else if ( table->id == 0x646D6464 )
		descriptions[i].deviceModelDesc = table;
	      else
		{
		  LOG_ERROR( _("profileSequenceDesc::ProfileDescription "
			       "unexpected table ") << table->name);
		  break;
		}

	      if ( !descriptions[i].deviceModelDesc && d + 16 >= data + size )
		{
		  descriptions[i].deviceModelDesc = table = new TagTable( d );
		  if ( table->id == 0x646D6464 )
		    {
		      LOG_ERROR( _("profileSequenceDesc::ProfileDescription "
				   "unexpected table ") 
				 << table->name 
				 << _(", expected deviceModelDesc") );
		      break;
		    }
		}
	    }
	}
    }

    void 
    ResponseCurveSet16Type::Curve::inspect( std::ostringstream& o ) const
    {
      o << name << " (" << std::hex << std::showbase << id 
	<< std::dec << ")" << std::endl;

      {
	V3fList::const_iterator i = values.begin();
	V3fList::const_iterator e = values.end();
	o << _("Measurements: ") << std::endl;
	for ( ; i != e; ++i )
	  {
	    o << *i << " ";
	  }
      }

      {
	response16ListArray::const_iterator i = responses.begin();
	response16ListArray::const_iterator e = responses.end();
	o << _("Responses: ") << std::endl;
	unsigned idx = 0;
	for ( ; i != e; ++i, ++idx )
	  {
	    o << _("Response #1") << std::endl;
	    response16List::const_iterator ri = (*i).begin();
	    response16List::const_iterator re = (*i).end();
	    for ( ; ri != re; ++ri )
	      {
		(*ri).inspect( o );
	      }
	    o << std::endl;
	  }
      }
    }

    const char* 
    ResponseCurveSet16Type::Curve::parse( const unsigned short num_measurements,
					  const unsigned short num_channels,
					  const char* data, 
					  const unsigned size )
    {
      values.resize( num_channels );
      responses.resize( num_channels );

      memcpy( name, data, 4 );

      const char* d = data + 4;

      UInt32List measurements( num_measurements );
      for ( unsigned i = 0; i < num_measurements; ++i, d += 4 )
	{
	  if ( d >= data + size ) 
	    {
	      LOG_ERROR( _("responseCurveSet16::Curve too little data "
			   "for measurements") );
	      return data + size;
	    }
	  measurements[i] = ntohl( *((unsigned*)d) );
	}

      XYZNumber tmp;
      for ( unsigned i = 0; i < num_channels; ++i, d += sizeof(XYZNumber) )
	{
	  if ( d >= data + size ) 
	    {
	      LOG_ERROR( _("responseCurveSet16::Curve too little data "
			   "for XYZNumbers") );
	      return data + size;
	    }
	  tmp.parse( d, size - ( d - data) );
	  values[i].x = tmp.x.to_f();
	  values[i].y = tmp.y.to_f();
	  values[i].z = tmp.z.to_f();
	}

      for ( unsigned i = 0; i < num_channels; ++i )
	{
	  unsigned num = measurements[i];
	  responses[i].resize( num );

	  for ( unsigned j = 0; j < num; ++j, d += 8 )
	    {
	      if ( d >= data + size ) 
		{
		  LOG_ERROR( _("responseCurveSet16::Curve too little data "
			       "for response16Numbers") );
		  return data + size;
		}

	      responses[i][j].parse( d, size );
	    }
	}

      return d;
    }

    void ResponseCurveSet16Type::inspect( std::ostringstream& o ) const
    {
      o << _("\t# of channels: ") << num_channels << std::endl
	<< _("\t# of measurement types: ") << curves.size() << std::endl;
      CurveList::const_iterator i = curves.begin();
      CurveList::const_iterator e = curves.end();

      for ( ; i != e; ++i )
	{
	  (*i).inspect( o );
	}
    }

    void ResponseCurveSet16Type::parse( const char* data, const unsigned size )
    {
      if ( size < 4 )
	{
	  LOG_ERROR( _("responseCurveSet16: missing data") );
	  curves.clear();  num_channels = 0;
	  return;
	}
      num_channels = ntohs( *((unsigned short*)data) );
      unsigned short num = ntohs( *((unsigned short*)data + 2) );

      curves.resize( num );
      
      // Skip offsets, as we don't use them
      const char* d = data + 4;
      for ( unsigned i = 0; i < num; ++i, d += 4 )
	{
	}

      for ( unsigned i = 0; i < num; ++i )
	{
	  if ( d - data >= size )
	    {
	      LOG_ERROR( _("responseCurveSet16 - too little data") );
	      break;
	    }
	  unsigned s = size - ( d - data );
	  d = curves[i].parse( num, num_channels, d, s ); 
	}
    }

    void S15Fixed16ArrayType::inspect( std::ostringstream& o ) const
    {
      FloatList::const_iterator i = values.begin();
      FloatList::const_iterator e = values.end();
      o << '\t';
      for ( ; i != e; ++i )
	o << *i << " ";
    }

    void S15Fixed16ArrayType::parse( const char* data, const unsigned size )
    {      
      static const unsigned mult = sizeof(s15Fixed16Number);
      if ( size % mult != 0 )
	LOG_ERROR( _("Not valid s15Fixed16ArrayType data, size ") << size 
		   << _(" should be a multiple of ") << mult);

      unsigned num = size / mult;
      values.resize(num);

      s15Fixed16Number tmp;
      const char* d = data;
      for ( unsigned i = 0; i < num; ++i, d += sizeof(s15Fixed16Number) )
	{
	  tmp.parse( d, size - (d-data) );
	  values[i] = tmp.to_f();
	}
    }

    void SignatureType::inspect( std::ostringstream& o ) const
    {
      o << '\t' << name << " (" << std::hex << std::showbase << id << std::dec
	<< ')';
    }

    void SignatureType::parse( const char* data, const unsigned size )
    {
      id = *((unsigned*)data);
    }

    void TextType::inspect( std::ostringstream& o ) const
    {
      o << '\t' << text;
    }

    void TextType::parse( const char* data, const unsigned size )
    {
      delete [] text;
      text = new char[size+1];
      text[size] = 0;
      memcpy( text, data, size );
    }


    void U16Fixed16ArrayType::inspect( std::ostringstream& o ) const
    {
      FloatList::const_iterator i = values.begin();
      FloatList::const_iterator e = values.end();
      o << '\t';
      for ( ; i != e; ++i )
	o << *i << " ";
    }

    void U16Fixed16ArrayType::parse( const char* data, const unsigned size )
    {      
      static const unsigned mult = sizeof(u16Fixed16Number);
      if ( size % mult != 0 )
	LOG_ERROR( _("Not valid u16Fixed16ArrayType data, size ") << size 
		   << _(" should be a multiple of ") << mult);

      unsigned num = size / mult;
      values.resize(num);

      u16Fixed16Number tmp;
      const char* d = data;
      for ( unsigned i = 0; i < num; ++i, d += sizeof(u16Fixed16Number) )
	{
	  tmp.parse( d, size - (d-data) );
	  values[i] = tmp.to_f();
	}
    }


    void UInt16ArrayType::inspect( std::ostringstream& o ) const
    {
      UInt16List::const_iterator i = values.begin();
      UInt16List::const_iterator e = values.end();
      o << '\t';
      for ( ; i != e; ++i )
	o << *i << " ";
    }

    void UInt16ArrayType::parse( const char* data, const unsigned size )
    {      
      static const unsigned mult = sizeof(unsigned short);
      if ( size % mult != 0 )
	LOG_ERROR( _("Not valid UInt16ArrayType data, size ") << size 
		   << _(" should be a multiple of ") << mult);

      unsigned num = size / mult;
      values.resize(num);

      unsigned short* tmp = (unsigned short*) data;
      for ( unsigned i = 0; i < num; ++i )
	{
	  values[i] = tmp[i];
	}
    }

    void UInt32ArrayType::inspect( std::ostringstream& o ) const
    {
      UInt32List::const_iterator i = values.begin();
      UInt32List::const_iterator e = values.end();
      o << '\t';
      for ( ; i != e; ++i )
	o << *i << " ";
    }

    void UInt32ArrayType::parse( const char* data, const unsigned size )
    {      
      static const unsigned mult = sizeof(unsigned int);
      if ( size % mult != 0 )
	LOG_ERROR( _("Not valid UInt32ArrayType data, size ") << size 
		   << _(" should be a multiple of ") << mult);

      unsigned num = size / mult;
      values.resize(num);

      unsigned int* tmp = (unsigned int*) data;
      for ( unsigned i = 0; i < num; ++i )
	{
	  values[i] = ntohl( tmp[i] );
	}
    }

    void UInt64ArrayType::inspect( std::ostringstream& o ) const
    {
      UInt64List::const_iterator i = values.begin();
      UInt64List::const_iterator e = values.end();
      o << '\t';
      for ( ; i != e; ++i )
	o << *i << " ";
    }

    void UInt64ArrayType::parse( const char* data, const unsigned size )
    {      
      static const unsigned mult = sizeof(boost::uint64_t);
      if ( size % mult != 0 )
	LOG_ERROR( _("Not valid UInt64ArrayType data, size ") << size 
		   << _(" should be a multiple of ") << mult);

      unsigned num = size / mult;
      values.resize(num);

      const char* d = data;
      for ( unsigned i = 0; i < num; ++i, d += 8 )
	{
	  char val[8];
	  for ( short j = 0; j < 8; ++j )
	    val[j] = d[7-j];
	  boost::uint64_t* ptr = (boost::uint64_t*)val;
	  values[i] = *ptr;
	}
    }

    void UInt8ArrayType::inspect( std::ostringstream& o ) const
    {
      UInt8List::const_iterator i = values.begin();
      UInt8List::const_iterator e = values.end();
      o << '\t';
      for ( ; i != e; ++i )
	o << ((unsigned short)(*i)) << " ";
    }

    void UInt8ArrayType::parse( const char* data, const unsigned size )
    {
      values.resize(size);

      unsigned char* tmp = (unsigned char*) data;
      for ( unsigned i = 0; i < size; ++i )
	{
	  values[i] = tmp[i];
	}
    }

    void ViewingConditionsType::inspect( std::ostringstream& o ) const
    {
      o << _("\tIlluminant: ") << illuminant << std::endl 
	<< _("\tSurround: ") << surround  << std::endl 
	<< _("\tIlluminant Type: ") << illuminant_type.to_s();
    }

    void ViewingConditionsType::parse( const char* data, const unsigned size )
    {
      if ( size < 27 )
	{
	  LOG_ERROR( _("ViewingConditionsType: too small data") );
	  return;
	}

      XYZNumber tmp;
      tmp.parse( data, size );
      illuminant.x = tmp.x.to_f();
      illuminant.y = tmp.y.to_f();
      illuminant.z = tmp.z.to_f();

      tmp.parse( data + sizeof(XYZNumber), size );
      surround.x = tmp.x.to_f();
      surround.y = tmp.y.to_f();
      surround.z = tmp.z.to_f();

      illuminant_type = *((unsigned int*) (data + 24));
    }

    void XYZType::inspect( std::ostringstream& o ) const
    {
      V3fList::const_iterator i = values.begin();
      V3fList::const_iterator e = values.end();
      o << '\t';
      for ( ; i != e; ++i )
	o << *i << " ";
    }

    void XYZType::parse( const char* data, const unsigned size )
    {
      static const unsigned mult = sizeof(XYZNumber);
      if ( size % mult != 0 )
	LOG_ERROR( _("Not valid XYZType data, size ") << size 
		   << _(" should be a multiple of ") << mult);

      unsigned num = size / mult;
      values.resize(num);

      XYZNumber tmp;
      const char* d = data;
      for ( unsigned i = 0; i < num; ++i, d += sizeof(XYZNumber) )
	{
	  tmp.parse( d, sizeof(XYZNumber) );

	  values[i].x = tmp.x.to_f();
	  values[i].y = tmp.y.to_f();
	  values[i].z = tmp.z.to_f();
	}
    }


    bool Tag::validate( const unsigned tag_id, const unsigned id )
    {
      switch( tag_id )
	{
	case AtoB0Tag:
	case AtoB1Tag:
	case AtoB2Tag:
	  {
	    if ( id == lut8TypeTag || id == lut16TypeTag ||
		 id == lutAtoBTypeTag ) return true;
	    return false;
	  }
	case gamutTag:
	case BtoA0Tag:
	case BtoA1Tag:
	case BtoA2Tag:
	case preview0Tag:
	case preview1Tag:
	case preview2Tag:
	  {
	    if ( id == lut8TypeTag || id == lut16TypeTag ||
		 id == lutBtoATypeTag ) return true;
	    return false;
	  }
	case blueMatrixColumnTag:
	case greenMatrixColumnTag:
	case redMatrixColumnTag:
	case luminanceTag:
	case mediaBlackPointTag:
	case mediaWhitePointTag:
	  {
	    if ( id == XYZTypeTag ) return true;
	    return false;
	  }
	case blueTRCTag:
	case redTRCTag:
	case greenTRCTag:
	case grayTRCTag:
	  {
	    if ( id == curveTypeTag || id == parametricCurveTypeTag ) 
	      return true;
	    return false;
	  }
	case calibrationDateTimeTag:
	  {
	    if ( id == dateTimeTypeTag ) 
	      return true;
	    return false;
	  }
	case charTargetTag:
	  {
	    if ( id == textTypeTag ) 
	      return true;
	    return false;
	  }
	case chromaticAdaptationTag:
	  {
	    if ( id == s15Fixed16ArrayTypeTag ) 
	      return true;
	    return false;
	  }
	case chromaticityTag:
	  {
	    if ( id == chromaticityTypeTag ) 
	      return true;
	    return false;
	  }
	case colorantOrderTag:
	  {
	    if ( id == colorantOrderTypeTag ) 
	      return true;
	    return false;
	  }
	case colorantTableTag:
	  {
	    if ( id == colorantTableTypeTag ) 
	      return true;
	    return false;
	  }
	case colorantTableOutTag:
	  {
	    if ( id == colorantTableTypeTag ) 
	      return true;
	    return false;
	  }
	case profileDescriptionTag:
	  {
	    if ( id == multiLocalizedUnicodeTypeTag ||
		 id == textTypeTag ||
		 id == profileDescriptionTypeTag ) 
	      return true;
	    return false;
	  }
	case copyrightTag:
	case deviceMfgDescTag:
	case deviceModelDescTag:
	case viewingCondDescTag:
	  {
	    if ( id == multiLocalizedUnicodeTypeTag ||
		 id == textTypeTag ) 
	      return true;
	    return false;
	  }
	case measurementTag:
	  {
	    if ( id == measurementTypeTag )
	      return true;
	    return false;
	  }
	case namedColor2Tag:
	  {
	    if ( id == namedColor2TypeTag )
	      return true;
	    return false;
	  }
	case outputResponseTag:
	  {
	    if ( id == responseCurveSet16TypeTag )
	      return true;
	    return false;
	  }
	case profileSequenceDescTag:
	  {
	    if ( id == profileSequenceDescTypeTag ) 
	      return true;
	    return false;
	  }
	case technologyTag:
	  {
	    if ( id == signatureTypeTag ) 
	      return true;
	    return false;
	  }
	case viewingConditionsTag:
	  {
	    if ( id == viewingConditionsTypeTag ) 
	      return true;
	    return false;
	  }
	case MicrosoftWCSTag:
	  {
	    if ( id == MS10TypeTag ) 
	      return true;
	    return false;
	  }
	default:
	  return true;
	}
      return false;
    }

    Tag* Tag::factory( const unsigned tag,
		       const char* data, const unsigned size )
    {
      unsigned id = *((unsigned*)data);

      if ( !validate( tag, id ) )
	{
	  char tag_name[5]; tag_name[4] = 0;
	  char type_name[5]; type_name[4] = 0;
	  memcpy( tag_name, &tag, 4 );
	  memcpy( type_name, data, 4 );
	  LOG_ERROR(
		    "Invalid type id '" << type_name 
		    << "' (" 
		    << std::hex << std::showbase << id << std::dec 
		    << ") for tag '" << tag_name << "' (" 
		    << std::hex << std::showbase << tag << std::dec << ")"
		    );
	  return NULL;
	}


      Tag* result = NULL;
      switch( id )
	{
	case chromaticityTypeTag:
	  result = new ChromaticityType(); break;
	case colorantOrderTypeTag:
	  result = new ColorantOrderType(); break;
	case colorantTableTypeTag:
	  result = new ColorantTableType(); break;
	case curveTypeTag:
	  result = new CurveType(); break;
	case dataTypeTag:
	  result = new DataType(); break;
	case dateTimeTypeTag:
	  result = new DateTimeType(); break;
	case lut8TypeTag:
	  result = new Lut8Type(); break;
	case lut16TypeTag:
	  result = new Lut16Type(); break;
	case lutAtoBTypeTag:
	  result = new LutAtoBType(); break;
	case lutBtoATypeTag:
	  result = new LutBtoAType(); break;
	case measurementTypeTag:
	  result = new MeasurementType(); break;
	case multiLocalizedUnicodeTypeTag:
	  result = new MultiLocalizedUnicodeType(); break;
	case namedColor2TypeTag:
	  result = new NamedColor2Type(); break;
	case parametricCurveTypeTag:
	  result = new ParametricCurveType(); break;
	case profileDescriptionTypeTag:
	  result = new ProfileDescriptionType(); break;
	case profileSequenceDescTypeTag:
	  result = new ProfileSequenceDescType(); break;
	case responseCurveSet16TypeTag:
	  result = new ResponseCurveSet16Type(); break;
	case s15Fixed16ArrayTypeTag:
	  result = new S15Fixed16ArrayType(); break;
	case signatureTypeTag:
	  result = new SignatureType(); break;
	case textTypeTag:
	  result = new TextType(); break;
	case u16Fixed16ArrayTypeTag:
	  result = new U16Fixed16ArrayType(); break;
	case uInt16ArrayTypeTag:
	  result = new UInt16ArrayType(); break;
	case uInt32ArrayTypeTag:
	  result = new UInt32ArrayType(); break;
	case uInt64ArrayTypeTag:
	  result = new UInt64ArrayType(); break;
	case uInt8ArrayTypeTag:
	  result = new UInt8ArrayType(); break;
	case viewingConditionsTypeTag:
	  result = new ViewingConditionsType(); break;
	case XYZTypeTag:
	  result = new XYZType(); break;
	default:
	  result = new UnknownTagType();
	}
      result->read( data, size );
      return result;
    }

  } // namespace icc

} // namespace mrv
