

#include "mrvString.h"

namespace mrv
{
 bool matches_chars( const char* src, const char* charlist )
 {
   const char* s = src;
   for ( ; *s != 0; ++s )
     {
       const char* d = charlist;
       bool found = false;
       for ( ; *d != 0; ++d )
	 {
	   if ( *s == *d ) { found = true; break; }
	 }
       if (!found) return false;
     }
   return true;
 }

  void split_string(stringArray& output,
		    const std::string& str, const std::string& delim
		    )
  {
    size_t offset = 0;
    size_t delimIndex = 0;
    
    delimIndex = str.find(delim, offset);

    output.clear();
    while (delimIndex != std::string::npos)
      {
        output.push_back(str.substr(offset, delimIndex - offset));
        offset += delimIndex - offset + delim.length();
        delimIndex = str.find(delim, offset);
      }

    output.push_back(str.substr(offset));
  }

} // namespace mrv
