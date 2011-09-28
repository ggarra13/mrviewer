/**
 * @file   colorProfiles.h
 * @author 
 * @date   Wed Oct 04 21:23:20 2006
 * 
 * @brief  Create mapping of color profile filename
 *         to boost::uint8_t color profile data.
 * 
 * 
 */

#ifndef mrvColorProfile_h
#define mrvColorProfile_h

#include <map>
#include <string>
#include <cstdio>
#include <cstdlib>

#include "mrvICCProfile.h"

namespace mrv
{

  class colorProfile
  {
  public:
    typedef std::map< std::string, icc::Profile* > ProfileData;
    typedef std::map< unsigned int, std::string > MonitorProfile;

  protected:
    static ProfileData    profiles;
    static MonitorProfile monitors;

  public:
    static void   clear();
    static void   add( const char* file, const size_t size,
		       const char* data );
    static void   add( const char* file );
    static icc::Profile* get( const char* file );

    static void   set_monitor_profile( const char* file, 
				       unsigned int monitor = 0 );
    static icc::Profile* get_monitor_profile( const unsigned int monitor = 0 );
  };

}


#endif // mrvColorProfile_h
