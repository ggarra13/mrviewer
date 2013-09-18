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

#include "SampleICC/IccProfile.h"

#include "mrvString.h"

namespace mrv
{

  class colorProfile
  {
  public:
    typedef std::map< std::string, CIccProfile* > ProfileData;
    typedef std::map< unsigned int, std::string > MonitorProfile;

  protected:
    static ProfileData    profiles;
    static MonitorProfile monitors;

  public:
    static std::string header( CIccProfile* );
    static std::string info( CIccProfile* );

    static void   clear();
    static void   add( const char* file, const size_t size,
		       const char* data );
    static void   add( const char* file );
    static CIccProfile* get( const char* file );

    static stringArray  list();

    static void   set_monitor_profile( const char* file, 
				       unsigned int monitor = 0 );
    static CIccProfile* get_monitor_profile( const unsigned int monitor = 0 );

    static std::string dump_tag(CIccProfile*, icTagSignature sig );

  protected:
    static void dump_tag( std::ostringstream& o, CIccProfile*, icTagSignature sig );
  };

}


#endif // mrvColorProfile_h
