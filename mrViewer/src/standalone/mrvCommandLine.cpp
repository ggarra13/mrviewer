/**
 * @file   mrvCommandLine.cpp
 * @author gga
 * @date   Mon Aug  6 12:28:19 2007
 * 
 * @brief  command-line parser for mrViewer
 * 
 * 
 */

#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>

#include "tclap/CmdLine.h"
#include "mrvI8N.h"
#include "mrvCommandLine.h"
#include "mrvPreferences.h"
#include "mrvString.h"
#include "gui/mrvImageView.h"
#include "gui/mrvTimeline.h"
#include "mrViewer.h"
#include "mrvVersion.h"
#include "mrvServer.h"

namespace fs = boost::filesystem;

namespace mrv {

  class CmdLineOutput : public TCLAP::StdOutput
  {
  public:

    virtual void usage(TCLAP::CmdLineInterface& c)
    {
      using namespace TCLAP;
      using namespace std;

      std::string cmd = c.getProgramName();
      cerr << endl
	   << cmd << " " << c.getVersion();

      //
      // Output usage line
      //
      cerr << endl << endl
	   << _("Usage:")
	   << endl << endl
	   << "  " << cmd << " ";

      std::list<Arg*> args = c.getArgList();
      for (ArgListIterator it = args.begin(); it != args.end(); ++it)
	{
	  cerr << (*it)->shortID() << " ";
	}

      //
      // Output options in shorter format than TCLAP's default
      //
      cerr << endl << endl
	   << _("Options:") << endl << endl;

      //
      // Find longest argument for formatting
      //
      size_t len = 0;
      for (ArgListIterator it = args.begin(); it != args.end(); ++it)
	{
	   if ( (*it)->longID().size() > len ) len = (*it)->longID().size();
	}

      //
      // Output the options
      //
      for (ArgListIterator it = args.begin(); it != args.end(); ++it)
	{
	  const std::string& ID = (*it)->longID();
	  cerr << "  " << ID;

	  for ( size_t i = ID.size(); i < len; ++i )
	    {
	      cerr << " ";
	    }
	  cerr << "   "
	       << (*it)->getDescription() << endl;
	}

      cerr << endl
	   << _("Names cannot contain spaces in their paths.") << endl << endl
	   << _("Examples:") << endl << endl
	   <<	"  > " << cmd << " background.dpx texture.png" << endl
	   <<	"  > " << cmd << " beauty.001-020.iff background.%04d.exr 1-20" << endl
	   <<	"  > " << cmd << " beauty.mov -a dialogue.wav beauty.@@.iff 1-20 beauty.avi" << endl;
    }
  };

//
// Command-line parser
//
void parse_command_line( const int argc, char** argv,
			 mrv::ViewerUI* ui, 
                         mrv::Options& opts )
{
  // Some default values
  opts.gamma = (float)ui->uiPrefs->uiPrefsViewGamma->value();
  opts.gain  = (float)ui->uiPrefs->uiPrefsViewGain->value();
  opts.fps   = -1.f;
  std::string db_driver = "postgresql";
  opts.host = "";
  opts.port = 0;

  try {
    using namespace TCLAP;

    CmdLine cmd( 
		_("A professional image and movie viewer\n"
		  "Examples:\n")
		 , ' ', mrv::version() );

    //
    // set the output
    //
    mrv::CmdLineOutput my;
    cmd.setOutput(&my);

    //
    // The command-line arguments (parsed in order)
    //
    SwitchArg aedl("e", "edl",
		   _("Turn on EDL when loading multiple images") );


    ValueArg< float > 
      afps("f", "fps", 
           _("Override sequence default fps."), false, opts.fps, "float");

    ValueArg< float > 
      agamma("g", "gamma", 
	     _("Override viewer's default gamma."), false, opts.gamma, "float");

    ValueArg< float > 
      again( "", "gain", 
	     _("Override viewer's default gain."), false, opts.gain, "float");

    ValueArg< std::string > 
      adbdriver( "", "dbd", 
		 _("Override viewer's default database driver."), false, 
		 db_driver, "string");

    ValueArg< std::string > 
    ahostname( "t", N_("host"), 
	       _("Override viewer's default client hostname."), false, 
	       opts.host, "string");

    ValueArg< unsigned > 
    aport( N_("p"), N_("port"), 
	     _("Set viewer's default server/client port."), false, 
	     opts.port, "string");

    UnlabeledMultiArg< std::string > 
    afiles(_("files"),
	   _("Images, sequences or movies to display."), false, "images");

    MultiArg< std::string > 
    aaudio( N_("a"), N_("audio"), 
            _("Set sequence default audio."), false, "audio files");

    cmd.add(agamma);
    cmd.add(again);
    cmd.add(adbdriver);
    cmd.add(ahostname);
    cmd.add(aport);
    cmd.add(aedl);
    cmd.add(afps);
    cmd.add(aaudio);
    cmd.add(afiles);

    //
    // Do the parsing
    //
    cmd.parse( argc, argv );


    //
    // Extract the options
    //
    opts.gamma = agamma.getValue();
    opts.gain  = again.getValue();
    db_driver = adbdriver.getValue();
    opts.host = ahostname.getValue();
    opts.port = aport.getValue();
    opts.edl  = aedl.getValue();
    opts.fps  = afps.getValue();

    //
    // Parse image list to split into sequences/images and reels
    //
    const stringArray& files = afiles.getValue();
    const stringArray& audios = aaudio.getValue();
    stringArray::const_iterator i = files.begin();
    stringArray::const_iterator e = files.end();
    stringArray::const_iterator ai = audios.begin();
    stringArray::const_iterator ae = audios.end();
    for ( ; i != e; ++i )
      {
	const std::string& arg = *i;

	// Check if string is a range.  if so, change last sequence
	if ( !opts.files.empty() && (opts.files.back().reel == false) &&
	     mrv::matches_chars( arg.c_str(), "0123456789-") )
	  {
	    stringArray tokens;
	    mrv::split_string( tokens, arg, "-" );


	    // frame range
	    opts.files.back().start = atoi( tokens[0].c_str() );
	    if ( tokens.size() > 1 )
               opts.files.back().end = atoi( tokens[1].c_str() );
	    continue;
	  }

	size_t len = arg.size();
	if ( len > 5 && arg.substr(len - 5, 5) == ".reel" )
	  {
	    opts.files.push_back( mrv::LoadInfo(arg) );
	  }
	else
	{
	    boost::int64_t start = mrv::kMinFrame, end = mrv::kMaxFrame;
	    std::string fileroot( arg );

            if ( mrv::is_directory( fileroot.c_str() ) )
            {
               std::string oldfile;
               boost::int64_t frameStart = mrv::kMaxFrame;
               boost::int64_t frameEnd = mrv::kMinFrame;
               stringArray files;


               {
                  // default constructor yields path iter. end
                  fs::directory_iterator e;
                  for ( fs::directory_iterator i( fileroot ) ; i != e; ++i )
                  {
                     if ( !fs::exists( *i ) || fs::is_directory( *i ) )
                        continue;

                     std::string file = (*i).path().string();
                     files.push_back( file );
                  }
               }

               std::sort( files.begin(), files.end() );

               {
                  stringArray::iterator i = files.begin();
                  stringArray::iterator e = files.end();

                  std::string root, frame, view, ext;
                  std::string oldroot;

                  for ( ; i != e; ++i )
                  {
                     std::string file = (*i);

                     bool ok = mrv::split_sequence( root, frame, view,
                                                    ext, file );


                     if ( mrv::is_valid_movie( ext.c_str() ) )
                     {
                        opts.files.push_back( mrv::LoadInfo( file, kMinFrame,
                                                             kMaxFrame ) );
                        opts.edl = true;
                        continue;
                     }

                     if ( root != "" && frame != "" && oldroot != root )
                     {
                        oldroot = root;
                        std::string fileroot;
                        mrv::fileroot( fileroot, file );
                        mrv::get_sequence_limits( start, end, fileroot );
                        opts.files.push_back( mrv::LoadInfo( fileroot, start,
                                                             end ) );
                        opts.edl = true;
                     }
                  }
               }
               
            }
            else
            {

               if ( mrv::is_valid_sequence( fileroot.c_str() ) )
               {
                  mrv::get_sequence_limits( start, end, fileroot );
               }
               
               if ( ai != ae )
               {
                  opts.files.push_back( mrv::LoadInfo( fileroot, start, 
                                                       end, *ai ) );
                  ++ai;
               }
               else
               {
                  opts.files.push_back( mrv::LoadInfo( fileroot, start, end ) );
               }
            }
	  }
      }


  }
  catch ( TCLAP::ArgException& e )
    {
      std::cerr << e.error() << " " << e.argId() << std::endl;
      exit(1);
    }

  // Load default preferences
  mrv::Preferences::run(ui);
  //
  // UI command-line overrides
  //
  
  

  ui->uiView->gamma( opts.gamma );
  ui->uiGammaInput->value( opts.gamma );
  ui->uiGamma->value( opts.gamma );

  ui->uiGainInput->value( opts.gain );
  ui->uiGain->value( opts.gain );
  ui->uiView->gain( opts.gain );

}


} // namespace mrv

