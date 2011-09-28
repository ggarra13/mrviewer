/**
 * @file   mrvProtection.cpp
 * @author gga
 * @date   Mon Oct 30 04:55:39 2006
 * 
 * @brief  Functions used to read and check license file
 * 
 */

#include <iostream>
using namespace std;

#include "mrvLicensing.h"
// #include "mrvPC1.h"

// extern "C" {
//   // RLM files
// #include "license.h"
// #include "license_to_run.h"
// }

namespace mrv
{
  // namespace {

  //   //
  //   // 128 SHA-1 encode of 'gga_mrViewer_key'
  //   //
  //   // Key to PC1 algorithm
  //   //
  //   const char* kPC1_key = "fd0629758a5ead396c130a550185b865edc0163c";
  
  //   // "2764@127.0.0.1"
  //   const char* kDefaultServer = "hkldnbkjfnbjnidnpbmpmfheadpm";

  //   // "Error initializing license system:\n"
  //   const char* kLicenseInitError = 
  //     "anannemampdfkdbjdlokmcjdjkljlhiibmmencnhnomkodkffieimcomobbehomabicoll";

  //   // "License Error:\n"
  //   const char* kLicenseError = "aefaigpbgjbbejondkamhmdmmjbcgf";

  //   // "mrViewer"
  //   const char* kProduct = "cfmjmjmonjcdcpkh";

  //   // "2007.01"
  //   const char* kVersion = "hkledbdicaoigp";


  //   RLM_HANDLE   rh;
  //   RLM_LICENSE lic;

  //   mrv::cipher::PC1 cypher;

  // }


  // const char* decrypt( const char* text )
  // {
  //   cypher.ascipherlen = strlen(text);
  //   cypher.plainlen    = strlen(text);
  //   cypher.ascii_decrypt128( text, kPC1_key );
  //   return cypher.plainText;
  // }

  bool open_license(const char* prog)
  {
    // rh = rlm_init( ".", prog, decrypt( kDefaultServer ) );
    // int stat = rlm_stat(rh);
    // if ( stat )
    //   {
    // 	char errstring[RLM_ERRSTRING_MAX];

    // 	fprintf(stderr, decrypt( kLicenseInitError ) );
    // 	fprintf(stderr, "%s\n", rlm_errstring((RLM_LICENSE) NULL, rh, 
    // 					      errstring));
    // 	close_license();
    // 	exit(1);
    //   }

    return true;
  }

  // void printstat( RLM_HANDLE rh, RLM_LICENSE lic )
  // {
  //   char errstring[RLM_ERRSTRING_MAX];
  //   fprintf(stderr, decrypt( kLicenseError ) );
  //   fprintf(stderr, "%s\n", rlm_errstring(lic, rh, errstring));

  //   checkin_license();
  //   close_license();

  //   exit(1);
  // }

  bool checkout_license()
  {
     return true;
    // char* product = strdup( decrypt( kProduct ) );
    // char* version = strdup( decrypt( kVersion ) );

    // lic  = rlm_checkout(rh, product, version, 1);

    // free( product );
    // free( version );

    // return (lic != NULL);
  }

  bool check_license_status()
  {
    // if (!lic) {
    //   if ( !checkout_license() ) return false;
    // }

    // int stat = rlm_license_stat(lic);
    // if ( stat == 0 || stat == RLM_EL_INQUEUE || stat == RLM_EL_OVERSOFT)
       
      return true;

    // printstat( rh, lic );
    // return false;
  }

  bool checkin_license()
  {
    // if (lic)  rlm_checkin(lic);
    // lic = NULL;
    return true;
  }


  bool close_license()
  {
    // rlm_close(rh);
    // rh = NULL;
    return true;
  }

}
