/**
 * @file   mrvProtection.h
 * @author gga
 * @date   Sat Oct 28 05:53:19 2006
 * 
 * @brief  A simple copy-protection/license mechanism
 * 
 * 
 */

#include <ctime>

#include <string>

namespace mrv
{
  bool open_license(const char* prog);
  bool close_license();
  bool checkout_license();
  bool check_license_status();
  bool checkin_license();

}
