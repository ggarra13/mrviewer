/**
 * @file   mrvICCProfile.h
 * @author gga
 * @date   Thu Feb 14 11:37:46 2008
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvICCProfile_h
#define mrvICCProfile_h

#include <vector>
#include <map>
#include <iostream>

#include "icc/mrvICCTagTable.h"
#include "icc/mrvICCTag.h"
#include "icc/mrvICCHeader.h"

namespace mrv {

  namespace icc {

    class Profile
    {
    public:
      typedef std::map< icc::Id, Tag_ptr > TagMap;
      typedef std::vector< TagTable >      TagTableList;
      typedef std::vector< std::string >   StringList;

    public:
      Profile( const char* data, const size_t size,
	       const char* filename = "" );
      ~Profile();
    
      StringList tags() const;

      inline std::string filename() const   { return _header.filename(); }

      bool       has_tag( const unsigned id ) const;
      const Tag* find( const unsigned id ) const;

      bool       has_tag( const char* name ) const;
      const Tag* find( const char* name ) const;

      Imath::M33f chromatic_adaptation() const;
      Imath::V3f white_point() const;
      Imath::V3f black_point() const;
      Imath::V3f illuminant() const;

      std::string describe( bool verbose = false ) const;

    protected:
      void parse( const char* data, const size_t size );

    protected:
      Header       _header;
      TagTableList _entries;
      TagMap       _tags;
      std::string  _filename;
    };


} // namespace icc

} // namespace mrv


#endif // mrvICCProfile_h
