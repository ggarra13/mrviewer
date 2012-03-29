
#include <sys/stat.h>
#include <cstdio>
#include <cassert>

#if defined(WIN32) || defined(WIN64)
#  include <windows.h>
#endif

#include <iostream>
#include <sstream>
#include <iomanip>


#include <SampleICC/IccTag.h>
#include <SampleICC/IccUtil.h>


#include "mrvColorProfile.h"
#include "mrvIO.h"

namespace {
  const char* kModule = "cprof";
}


namespace mrv
{

  colorProfile::ProfileData    colorProfile::profiles;
  colorProfile::MonitorProfile colorProfile::monitors;


#if defined(WIN32) || defined(WIN64)
  /** 
   * Given a Windows device context, return the name of 
   * the profile attached to it.
   * 
   * @param hDC  a Windows device context 
   * 
   * @return NULL or name of profile.
   */
  LPSTR GetDefaultICMProfile(HDC hDC)
  {
    BOOL bProfile;
    DWORD dwProfileLen;
    char szProfileName[MAX_PATH+1];
    
    dwProfileLen = MAX_PATH; szProfileName[0] = 0;
    SetICMMode(hDC, ICM_ON);
    bProfile = GetICMProfile(hDC, (LPDWORD) &dwProfileLen, szProfileName);

    if (!bProfile)
      return NULL;

    return strdup(szProfileName);
  }
#endif


  /** 
   * Clear all monitor profiles, releasing their data
   * 
   */
  void   colorProfile::clear()
  {
    ProfileData::iterator i = profiles.begin();
    ProfileData::iterator e = profiles.end();
    for ( ; i != e; ++i )
      {
	delete i->second;
      }
    profiles.clear();
  }

  /** 
   * Add and read a profile filename, storing its data
   * 
   * @param file 
   */
  void colorProfile::add( const char* file, const size_t size,
			  const char* data )
  {
    ProfileData::iterator i = profiles.find( file );
    if ( i != profiles.end() ) return;

    struct stat sbuf;
    int result = stat( file, &sbuf );
    if ( result == -1 ) return;

    assert( data != NULL );
    assert( size > 128 );

    CIccProfile* d = OpenIccProfile( (icUInt8Number*)data, size );
    if (!d) 
      {
	LOG_ERROR("Could not open ICC profile embedded in \"" << file << "\".");
	return;
      }

    profiles.insert( std::make_pair( file, d ) );
  }

  /** 
   * Add and read a profile filename, storing its data
   * 
   * @param file 
   */
  void colorProfile::add( const char* file )
  {
    ProfileData::iterator i = profiles.find( file );
    if ( i != profiles.end() ) return;


    CIccProfile* d = OpenIccProfile( file );
    if (!d) 
      {
	LOG_ERROR("Could not open ICC profile \"" << file << "\".");
	return;
      }

    profiles.insert( std::make_pair( file, d ) );
  }

  /** 
   * Get the profile data for a given file name
   * 
   * @param data profile data (output)
   * @param size profile size (output)
   * @param file input filename
   */
  CIccProfile* colorProfile::get( const char* file )
  {
    if ( file == NULL ) return NULL;
    ProfileData::iterator i = profiles.find( file );
    if ( i == profiles.end() ) return NULL;
    return i->second;
  }

  stringArray colorProfile::list()
  {
    stringArray r; r.reserve( profiles.size() );
    ProfileData::iterator i = profiles.begin();
    ProfileData::iterator e = profiles.end();
    for ( ; i != e; ++i )
      r.push_back( (*i).first );
    return r;
  }


  /** 
   * Add a new profile for a monitor (index)
   * 
   * @param file    profile
   * @param monitor monitor index
   */
  void   colorProfile::set_monitor_profile( const char* file, 
					    unsigned int monitor )
  {
    monitors.insert( std::make_pair( monitor, file ) );
  }

  /** 
   * Retrieve the profile data for a particular monitor
   * 
   * @param monitor  monitor index to retrieve data for
   */
  CIccProfile*   colorProfile::get_monitor_profile( unsigned int monitor )
  {
    MonitorProfile::iterator i = monitors.find( monitor );
    if ( i == monitors.end() ) return NULL;
    const std::string& file = i->second;
    return get( file.c_str() );
  }

  void colorProfile::dump_tag(std::ostringstream& o, 
			      CIccProfile *pIcc, icTagSignature sig)
  {
    using namespace std;
    CIccTag *pTag = pIcc->FindTag(sig);
    char buf[64];
    CIccInfo Fmt;
    
    std::string contents;

    if (pTag) {
      o << endl 
	<< "Contents of " << Fmt.GetTagSigName(sig) << " tag (" 
	<< hex << showbase << icGetSig(buf, sig) << ")" << endl
	<< "Type:   ";
      if (pTag->IsArrayType()) {
	o << "Array of ";
      }
      o << Fmt.GetTagTypeSigName(pTag->GetType()) << endl;
      pTag->Describe(contents);
      o << contents;
    }
    else {
      o << "Tag (" << icGetSig(buf, sig) << ") not found in profile" << endl;
    }
  }


  std::string colorProfile::header( CIccProfile* pIcc )
  {
    using namespace std;
    std::ostringstream o; o.str().reserve(2048);

    icHeader& hdr = pIcc->m_Header;
    CIccInfo  Fmt;
    char buf[64];

    o << "Profile ID:       ";
    if(Fmt.IsProfileIDCalculated(&(hdr.profileID)))
       o << Fmt.GetProfileID(&(hdr.profileID)) << endl;
    else
      o << "Profile ID not calculated." << endl;

    // Note: Fmt uses a static buffer for strings. That's why we repeat the
    //       'o << stuff' instead of concatenating directly.
    o << "Size:             " << hdr.size 
      << " (" << hex << showbase << hdr.size << dec << ") bytes" << endl
      << endl
      << "Header" << endl
      << "------" << endl
      << "Attributes:       " << Fmt.GetDeviceAttrName(hdr.attributes) << endl;
    o << "Cmm:              " << Fmt.GetCmmSigName((icCmmSignature)(hdr.cmmId))
      << endl    
      << "Creation Date:    " 
      << hdr.date.month << "/" << hdr.date.day << "/" << hdr.date.year
      << "  " << setw(2) << setfill('0') << hdr.date.hours 
      << ":" << setw(2) << setfill('0') << hdr.date.minutes 
      << ":" << setw(2) << setfill('0') << hdr.date.seconds
      << endl
      << "Creator:          " << icGetSig(buf, hdr.creator) << endl;
    o << "Data Color Space: " << Fmt.GetColorSpaceSigName(hdr.colorSpace) << endl;
    o << "Flags             " << Fmt.GetProfileFlagsName(hdr.flags) << endl;
    o << "PCS Color Space:  " << Fmt.GetColorSpaceSigName(hdr.pcs) << endl;
    o << "Platform:         " << Fmt.GetPlatformSigName(hdr.platform) << endl;
    o << "Rendering Intent: " 
      << Fmt.GetRenderingIntentName((icRenderingIntent)(hdr.renderingIntent)) 
      << endl;
    o << "Type:             " << Fmt.GetProfileClassSigName(hdr.deviceClass) 
      << endl;
    o << "Version:          " << Fmt.GetVersionName(hdr.version) << endl
      << "Illuminant:     "
      << "  X=" 
      << setiosflags(ios::fixed) << setprecision(4) << icFtoD(hdr.illuminant.X)
      << ", Y=" 
      << setiosflags(ios::fixed) << setprecision(4) << icFtoD(hdr.illuminant.Y) 
      << ", Z=" 
      << setiosflags(ios::fixed) << setprecision(4) << icFtoD(hdr.illuminant.Z) 
      << endl;
    return o.str();
  }

  std::string colorProfile::info( CIccProfile* pIcc )
  {
    using namespace std;

    std::ostringstream o; o.str().reserve(2048);

    CIccInfo  Fmt;
    char buf[64];

    o << header( pIcc )
      << endl
      << "Profile Tags" << endl
      << "------------" << endl
      << setfill(' ')
      << setw(25) << "Tag" << "    ID    " 
      << setw(8) << "Offset" << '\t' << setw(8) << "Size" << endl
      << setw(25) << "----" << "  ------  " << setw(8) 
      << "------" << '\t' << setw(8) << "----" << endl;

    int n;
    TagEntryList::iterator i;

    for (n=0, i=pIcc->m_Tags->begin(); i!=pIcc->m_Tags->end(); i++, n++) {
      o << setw(25) << Fmt.GetTagSigName(i->TagInfo.sig) << "  "
	<< icGetSig(buf, i->TagInfo.sig, false) << "  "
	<< setw(8) << i->TagInfo.offset << '\t'
	<< setw(8) << i->TagInfo.size
	<< endl;
    }

    for (n=0, i=pIcc->m_Tags->begin(); i!=pIcc->m_Tags->end(); i++, n++) 
      {
	dump_tag(o, pIcc, i->TagInfo.sig);
      }

    return o.str();
  }

  std::string colorProfile::dump_tag(CIccProfile *pIcc, icTagSignature sig)
  {
    std::ostringstream o; o.str().reserve(1024);
    dump_tag( o, pIcc, sig );
    return o.str();
  }

} // namespace mrv

