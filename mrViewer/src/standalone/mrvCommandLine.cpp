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

#include "tclap/CmdLine.h"
#include "mrvCommandLine.h"
#include "mrvPreferences.h"
#include "mrvString.h"
#include "mrvImageView.h"
#include "mrViewer.h"
#include "mrvVersion.h"


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
	   << "Usage:"
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
	   << "Options:" << endl << endl;

      //
      // Find longest argument for formatting
      //
      unsigned len = 0;
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

	  for ( unsigned i = ID.size(); i < len; ++i )
	    {
	      cerr << " ";
	    }
	  cerr << "   "
	       << (*it)->getDescription() << endl;
	}

      cerr << endl
	   << "Examples:" << endl << endl
	   <<	"  > " << cmd << " background.dpx texture.map" << endl
	   <<	"  > " << cmd << " beauty.1-20.iff background.%04d.exr 1-20" << endl
	   <<	"  > " << cmd << " beauty.mov beauty.@@.iff 1-20 beauty.avi" << endl;
    }
  };

//
// Command-line parser
//
void parse_command_line( const int argc, char** argv,
			 mrv::ViewerUI* ui, 
			 mrv::LoadList& load )
{
  // Some default values
  double gamma = ui->uiPrefs->uiPrefsViewGamma->value();
  double gain  = ui->uiPrefs->uiPrefsViewGain->value();
  std::string db_driver = "postgresql";

  try {
    using namespace TCLAP;

    CmdLine cmd( 
		"A professional image and movie viewer\n"
		"Examples:\n"
		 , ' ', mrv::version() );

    //
    // set the output
    //
    mrv::CmdLineOutput my;
    cmd.setOutput(&my);

    //
    // The command-line arguments (parsed in order)
    //
    ValueArg< double > 
      agamma("g", "gamma", 
	     "Override viewer's default gamma.", false, gamma, "float");

    ValueArg< double > 
      again( "", "gain", 
	    "Override viewer's default gain.", false, gain, "float");

    ValueArg< std::string > 
      adbdriver( "", "dbd", 
		 "Override viewer's default database driver.", false, 
		 db_driver, "string");

    UnlabeledMultiArg< std::string > 
      afiles("files",
	     "Images, sequences or movies to display.", false, "images");

    cmd.add(agamma);
    cmd.add(again);
    cmd.add(adbdriver);
    cmd.add(afiles);

    //
    // Do the parsing
    //
    cmd.parse( argc, argv );


    //
    // Extract the options
    //
    gamma = agamma.getValue();
    gain  = again.getValue();
    db_driver = adbdriver.getValue();

    //
    // Parse image list to split into sequences/images and reels
    //
    typedef  std::vector< std::string > StringList;
    const StringList& files = afiles.getValue();
    StringList::const_iterator i = files.begin();
    StringList::const_iterator e = files.end();
    for ( ; i != e; ++i )
      {
	const std::string& arg = *i;

	// Check if string is a range.  if so, change last sequence
	if ( !load.empty() && (load.back().reel == false) &&
	     mrv::matches_chars( arg.c_str(), "0123456789-") )
	  {
	    stringArray tokens;
	    mrv::split_string( tokens, arg, "-" );

	    // frame range
	    load.back().start = atoi( tokens[0].c_str() );
	    if ( tokens.size() > 1 )
	      load.back().end = atoi( tokens[1].c_str() );
	    continue;
	  }

	size_t len = arg.size();
	if ( len > 5 && arg.substr(len - 5, 5) == ".reel" )
	  {
	    load.push_back( mrv::LoadInfo(arg) );
	  }
	else
	  {
	    boost::int64_t start = mrv::kMinFrame, end = mrv::kMaxFrame;
	    std::string fileroot( arg );
	    if ( mrv::is_valid_sequence( fileroot.c_str() ) )
	      {
		mrv::get_sequence_limits( start, end, fileroot );
	      }

	    load.push_back( mrv::LoadInfo( fileroot, start, end ) );
	  }
      }

  }
  catch ( TCLAP::ArgException& e )
    {
      std::cerr << e.error() << " " << e.argId() << std::endl;
      exit(1);
    }

  // Load default preferences
  // mrv::Preferences::run(ui);

  //
  // UI command-line overrides
  //
  ui->uiView->gamma( gamma );
  ui->uiGammaInput->value( gamma );
  ui->uiGamma->value( gamma );

  ui->uiGainInput->value( gain );
  ui->uiGain->value( gain );
  ui->uiView->gain( gain );

}


} // namespace mrv

