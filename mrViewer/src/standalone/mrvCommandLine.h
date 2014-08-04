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
       mrv::LoadList stereo;
       bool edl;
       float gamma;
       float gain;
       std::string host;
       stringArray audios;
       unsigned short port;
       float fps;

  Options() : edl(false), gamma(1.0f), gain( 1.0f ), port( 0 ), fps( 0 )
    {}
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


