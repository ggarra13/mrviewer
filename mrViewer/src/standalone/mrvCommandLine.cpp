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
#include "mrvI8N.h"
#include "mrvCommandLine.h"
#include "mrvPreferences.h"
#include "mrvString.h"
#include "mrvImageView.h"
#include "mrViewer.h"
#include "mrvVersion.h"
#include "mrvServer.h"


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
	   <<	"  > " << cmd << " beauty.mov beauty.@@.iff 1-20 beauty.avi" << endl;
    }
  };

//
// Command-line parser
//
void parse_command_line( const int argc, char** argv,
			 mrv::ViewerUI* ui, 
			 mrv::LoadList& load,
			 std::string& host,
			 std::string& group)
{
  // Some default values
  float gamma = (float)ui->uiPrefs->uiPrefsViewGamma->value();
  float gain  = (float)ui->uiPrefs->uiPrefsViewGain->value();
  std::string db_driver = "postgresql";
  host = "";
  group = "239.255.0.1";

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
    ValueArg< float > 
      agamma("g", "gamma", 
	     _("Override viewer's default gamma."), false, gamma, "float");

    ValueArg< float > 
      again( "", "gain", 
	     _("Override viewer's default gain."), false, gain, "float");

    ValueArg< std::string > 
      adbdriver( "", "dbd", 
		 _("Override viewer's default database driver."), false, 
		 db_driver, "string");

    ValueArg< std::string > 
    ahostname( "t", N_("host"), 
	       _("Override viewer's default client hostname."), false, 
	       host, "string");

    ValueArg< std::string > 
    agroup( N_("p"), N_("group"), 
	     _("Override viewer's default server/client group."), false, 
	     group, "string");

    UnlabeledMultiArg< std::string > 
    afiles(_("files"),
	   _("Images, sequences or movies to display."), false, "images");

    cmd.add(agamma);
    cmd.add(again);
    cmd.add(adbdriver);
    cmd.add(ahostname);
    cmd.add(agroup);
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
    host = ahostname.getValue();
    group = agroup.getValue();


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
  mrv::Preferences::run(ui);
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

