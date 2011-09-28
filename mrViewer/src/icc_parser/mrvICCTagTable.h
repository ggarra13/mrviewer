/**
 * @file   mrvICCTagTable.h
 * @author gga
 * @date   Sat Feb 16 12:26:57 2008
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvICCTagTable_h
#define mrvICCTagTable_h

namespace mrv {

  namespace icc {

    struct TagTable
    {
      TagTable();
      TagTable( const char* data );
  
      union {
	unsigned  id;
	char      name[5];
      };
  
      unsigned offset;
      unsigned size;

      void inspect( std::ostringstream& o ) const;

    };

  }
}

#endif // mrvICCTagTable_h
