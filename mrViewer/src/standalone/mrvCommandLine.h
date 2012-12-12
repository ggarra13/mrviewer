/**
 * @file   mrvCommandLine.h
 * @author gga
 * @date   Mon Aug  6 12:26:40 2007
 * 
 * @brief  Command-line parser for mrViewer.
 * 
 * 
 */

#ifndef mrvCommandLine_h
#define mrvCommandLine_h

#include "Sequence.h"

namespace mrv {

  class ViewerUI;

  //
  // Command-line parser
  //
  void parse_command_line( const int argc, char** argv,
			   mrv::ViewerUI* ui, 
			   mrv::LoadList& files,
			   std::string& host,
			   std::string& group );
}


#endif // mrvCommandLine_h
