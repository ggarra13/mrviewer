/**
 * @file   mrvRoot.h
 * @author gga
 * @date   Fri Jan 11 02:04:55 2008
 * 
 * @brief  sets the MRV_ROOT variable if not defined by trying to find
 *         the executable's root directory.
 * 
 */

namespace mrv {

  void  set_root_path( const int argc = 0, char** argv = NULL );

}

