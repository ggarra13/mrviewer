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

typedef std::vector< std::string > stringArray;

  struct Options
  {
       mrv::LoadList files;
       bool edl;
       float gamma;
       float gain;
       std::string host;
       stringArray audios;
       unsigned port;
       float fps;
  };


  //
  // Given a directory parse all sequences and movies from it.
  //
   void parse_directory( const std::string& dir,
                         mrv::Options& opts );

  //
  // Command-line parser
  //
  void parse_command_line( const int argc, char** argv,
			   mrv::ViewerUI* ui, 
                           mrv::Options& opts);
}


#endif // mrvCommandLine_h


