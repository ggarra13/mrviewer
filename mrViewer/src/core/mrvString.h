/**
 * @file   mrvString.h
 * @author gga
 * @date   Wed Oct 18 21:09:07 2006
 * 
 * @brief  Some std::string utilities
 * 
 * 
 */

#ifndef mrvString_h
#define mrvString_h

#include <string>
#include <vector>
#include <set>

typedef std::vector< std::string > stringArray;
typedef std::set   < std::string > stringSet;

namespace mrv
{
  bool matches_chars( const char* src, const char* charlist );

  void split_string(stringArray& output,
		    const std::string& str, const std::string& delim );

} // namespace mrv


#endif // mrvString_h
