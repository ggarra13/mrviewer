/**
 * @file   mrvICCTagTable.cpp
 * @author gga
 * @date   Sat Feb 16 12:28:21 2008
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

#include <string.h>

#include <sstream>

#include "mrvICCTagTable.h"

namespace mrv {
  namespace icc {

    TagTable::TagTable()
    {
      name[4] = 0;
    }

    TagTable::TagTable( const char* d ) :
      offset( ntohl( *((unsigned*) (d + 4) ) ) ),
      size( ntohl( *((unsigned*) (d + 8) ) ) )
    {
      memcpy( &name, d, 4 );
      name[4] = 0;
    }

    void TagTable::inspect( std::ostringstream& o ) const
    {
      o << "[TagTable: " << name 
	<< " (" << std::hex << std::showbase << id << std::dec << ")"
	<< "offset: " << offset
	<< "size:   " << size << "]";
    }

  } // namespace icc

} // namespace mrv

