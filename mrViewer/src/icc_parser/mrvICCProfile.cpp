/**
 * @file   mrvICCProfile.cpp
 * @author gga
 * @date   Thu Feb 14 13:37:24 2008
 * 
 * @brief  
 * 
 * 
 */


#include <cassert>



#if defined(WIN32) || defined(WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>  // ntohl / ntohs
#endif

#include <sstream>
#include <algorithm>

#include <ImathVec.h>
#include <ImathMatrix.h>

#include "mrvException.h"
#include "mrvIO.h"
#include "mrvICCProfile.h"
#include "mrvICCIds.h"
#include "mrvICCWhitePoint.h"
#include "core/mrvI8N.h"

namespace 
{
  const char* kModule = "icc";
}


namespace mrv {

  namespace icc {


    Profile::Profile( const char* data, const size_t size,
		      const char* file ) :
      _header( file )
    {
      parse( data, size );
    }

    Profile::~Profile()
    {
    }



    std::string Profile::describe( bool verbose ) const
    {
      std::ostringstream o; o.str().reserve(2048);

      o << _header << std::endl
	<< _("Chromatic Adaptation: ") << std::endl
	<< chromatic_adaptation()
	<< _("White Point: ") << white_point() << std::endl
	<< _("Black Point: ") << black_point() << std::endl
	<< std::endl;

      if ( !_entries.empty() )
	{
	  o << _("\tName\t    ID    \tOffset\t Size") << std::endl
	    << _("\t----\t----------\t------\t-----") << std::endl;

	  TagTableList::const_iterator i = _entries.begin();
	  TagTableList::const_iterator e = _entries.end();
	  for ( ; i != e; ++i )
	    {
	      const TagTable& entry = *i;
	      o << "\t" << entry.name << "\t" 
		<< std::hex << std::showbase << entry.id 
		<< std::dec << "\t" << entry.offset << "\t" << entry.size
		<< std::endl;
	    }
	  o << std::endl;
	}

      StringList tag_names = tags();
      StringList::const_iterator i = tag_names.begin();
      StringList::const_iterator e = tag_names.end();

      o << _("Tags: ");
      bool comma = false;
      for ( ; i != e; ++i, comma = true )
	{
	  if ( comma ) o << ", ";
	  o << *i;
	}

      if ( verbose )
	{
	  o << std::endl << std::endl;
	  for ( i = tag_names.begin(); i != e; ++i )
	    {
	      const Tag*  tag = find( (*i).c_str() );
	      o << _("Tag: ") << *i << std::endl;
	      if (!tag)
		{
		  o << _("\tInvalid") << std::endl;
		}
	      else
		{
		  o << "\t" << *tag << std::endl;
		}
	    }
	}

      return o.str();
    }

    void Profile::parse( const char* data, const size_t size )
    {
      std::ostringstream o;

      try 
	{
	  _header.parse( data, size );
	}
      catch( const std::exception& e )
	{
	  LOG_ERROR( e.what() );
	  return;
	}

      const char* tag_table = data + 128;
      unsigned numTags = ntohl(  *((unsigned*) tag_table) );

      const char* d = tag_table + 4;
      unsigned expected_size = (d-data) + 12 * numTags;
      if ( expected_size >= size )
	{
	  LOG_ERROR("Invalid tag table, expected size " << expected_size
		    << " but was " << size << ".");
	  return;
	}

      for ( unsigned i = 0; i < numTags; ++i, d += 12 )
	{
	  TagTable entry( d );
	  _entries.push_back( entry );
	}

      TagTableList::const_iterator i = _entries.begin();
      TagTableList::const_iterator e = _entries.end();

      for ( ; i != e; ++i )
	{
	  const TagTable& entry = *i;
	  const char*     tag_data = data + entry.offset;

	  try 
	    {
	      if ( tag_data >= data + size )
		{
		  EXCEPTION( "Invalid offset in tag table" );
		}

	      Tag_ptr tag( Tag::factory( entry.id, tag_data, entry.size ) );
	      Id id( entry.id );
	      _tags.insert( std::make_pair( id, tag ) );
	    }
	  catch( const std::exception& e )
	    {
	      o << e.what() << std::endl;
	    }
	}

      std::cerr << describe(true) << std::endl;

      Imath::V3f white = white_point();
      Imath::V3f black = black_point();
      if ( white != kD50WhitePoint )
	{
	  std::cerr << "not normal white point" << std::endl;
	  std::cerr << "Chromatic Adaptation: "<< std::endl
		    << chromatic_adaptation() << std::endl;
#if 0
	  std::cerr << white << " != " << kD50WhitePoint << std::endl;
// 	  Imath::V3f mult = kD50WhitePoint / white;
	  Imath::V3f mult = white / kD50WhitePoint;
// 	  Imath::V3f mult = white;
	  std::cerr << "mult: " << mult << std::endl;

	  const Tag* tag;
	  XYZType* xyz;
	  static const char* keys[] = {
	    "rXYZ",
	    "gXYZ",
	    "bXYZ",
	    "rTRC",
	    "gTRC",
	    "bTRC",
	    NULL
	  };

	  const char** key = keys;
	  for ( ; *key != NULL; ++key )
	    {
	      tag = find( *key );
	      xyz = const_cast< XYZType* >( dynamic_cast< const XYZType* >( tag ) );
	      if ( xyz )
		{
		  unsigned num = xyz->values.size();
		  for (unsigned i = 0; i < num; ++i )
		    xyz->values[i] *= mult;
		}
	    }
#endif
	}
      if ( black != kD50BlackPoint )
	{
	  std::cerr << "not normal black point" << std::endl;
	}

      if ( ! o.str().empty() )
	{
	  LOG_ERROR( o.str() );
	}
      else
	{ 
	  // All OK, we don't need offset tables anymore
	  _entries.clear();
	}
    }

    const Tag* Profile::find( const char* name ) const
    {
      TagMap::const_iterator i = _tags.find( name );
      if ( i == _tags.end() ) return NULL;
      return i->second.get();
    }

    const Tag* Profile::find( const unsigned id ) const
    {
      TagMap::const_iterator i = _tags.find( id );
      if ( i == _tags.end() ) return NULL;
      return i->second.get();
    }

    Profile::StringList Profile::tags() const
    {
      StringList r; r.reserve( _tags.size() );

      // Get tags
      TagMap::const_iterator i = _tags.begin();
      TagMap::const_iterator e = _tags.end();
      for ( ; i != e; ++i )
	{
	  r.push_back( i->first.name );
	}

      // Sort tags alphabetically
      std::sort( r.begin(), r.end() );

      return r;
    }

    bool Profile::has_tag( const char* name ) const
    {
      return ( find( name ) != NULL );
    }

    bool Profile::has_tag( const unsigned id ) const
    {
      return ( find( id ) != NULL );
    }

    Imath::V3f Profile::white_point() const
    {
      // D50 is default
      static const Imath::V3f defaultValue( kD50WhitePoint );
      const Tag* tag = find( mediaWhitePointTag );
      if ( !tag )
	{
	  return defaultValue;
	}

      const XYZType* xyz = dynamic_cast< const XYZType* >( tag );
      if ( !xyz || xyz->values.empty() )
	{
	  return defaultValue;
	}
      
      return xyz->values[0];
    }

    Imath::V3f Profile::illuminant() const
    {
      return _header.illuminant();
    }

    Imath::V3f Profile::black_point() const
    {
      static const Imath::V3f defaultValue( 0.f, 0.f, 0.f );
      const Tag* tag = find( mediaBlackPointTag );
      if ( !tag )
	{
	  return defaultValue;
	}

      const XYZType* xyz = dynamic_cast< const XYZType* >( tag );
      if ( !xyz || xyz->values.empty() )
	{
	  return defaultValue;
	}
      
      return xyz->values[0];
    }

    Imath::M33f Profile::chromatic_adaptation() const
    {
      const Tag* tag = find( chromaticAdaptationTag );
      const S15Fixed16ArrayType* t = dynamic_cast< const S15Fixed16ArrayType* >( tag );

      if ( !t || t->values.size() != 9 )
	{
	  Imath::M33f defaultValue; // identity
	  if ( _header.type() == displayDeviceTag )
	    {
	      adaptation_matrix( defaultValue,
				 illuminant(),
				 white_point() );
	    }
	  return defaultValue;
	}
      
      Imath::M33f m;
      for ( int y = 0; y < 3; ++y )
	for ( int x = 0; x < 3; ++x )
	  m[y][x] = t->values[x+y*3];
      return m;
    }

  } // namespace icc

} // namespace mrv
