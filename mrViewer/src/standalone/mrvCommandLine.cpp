/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

#include <tclap/CmdLine.h>

#include "core/mrvI8N.h"
#include "core/mrvString.h"
#include "core/mrvServer.h"
#include "gui/mrvIO.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvImageView.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvVersion.h"
#include "mrViewer.h"
#include "standalone/mrvCommandLine.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = boost::filesystem;

namespace {
const char* kModule = "parser";
}

#define USE_STEREO

namespace mrv {

typedef std::vector<unsigned> UnsignedArray;
typedef std::vector<int> IntArray;

  class CmdLineOutput : public TCLAP::StdOutput
  {
  public:

    virtual void usage(TCLAP::CmdLineInterface& c)
    {
      using namespace TCLAP;
      using namespace std;

#if defined(_WIN32) || defined(_WIN64)
      AllocConsole();
      freopen("CONIN$", "r", stdin );
      freopen("CONOUT$", "w", stdout);
      freopen("CONOUT$", "w", stderr);
#endif

      std::string cmd = c.getProgramName();
      fprintf( stderr, N_("\n%s %s\n\n"), cmd.c_str(), c.getVersion().c_str() );

      //
      // Output usage line
      //
      fprintf( stderr, _("Usage:\n\n  %s "), cmd.c_str() );

      std::list<Arg*> args = c.getArgList();
      for (ArgListIterator it = args.begin(); it != args.end(); ++it)
	{
	  fprintf(stderr, "%s ", (*it)->shortID().c_str() );
	}

      //
      // Output options in shorter format than TCLAP's default
      //
      fprintf( stderr, _("\n\nOptions:\n\n"));

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
	  fprintf( stderr, "  %s", ID.c_str() );

	  for ( size_t i = ID.size(); i < len; ++i )
	    {
	      fprintf( stderr, " " );
	    }
	  fprintf( stderr, "   %s\n", (*it)->getDescription().c_str() );
	}

      fprintf( stderr, "\n" );
      fprintf( stderr, _("Names cannot contain spaces in their paths.\n\n") );
      fprintf( stderr, _("Examples:\n\n") );
      fprintf( stderr, "  > %s background.dpx texture.png\n", cmd.c_str() );
      fprintf( stderr, "  > %s beauty.001-020.iff background.%%04d.exr 1-20\n", cmd.c_str() );
      fprintf( stderr, "  > %s beauty.mov -a dialogue.wav beauty.@@.iff 1-20 beauty.avi\n", cmd.c_str() );

#if defined(_WIN32)||defined(_WIN64)
       Sleep(INFINITE);
#endif

    }


  };


void parse_directory( const std::string& fileroot,
                      mrv::Options& opts )
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

   {
      stringArray::iterator i = files.begin();
      stringArray::iterator e = files.end();

      std::string root, frame, view, ext;
      Sequences tmpseqs;

      for ( ; i != e; ++i )
      {
         std::string fileroot;

         mrv::fileroot( fileroot, *i );
         bool ok = mrv::split_sequence( root, frame, view,
                                        ext, fileroot );
         

         if ( mrv::is_valid_movie( ext.c_str() ) )
         {
            opts.files.push_back( mrv::LoadInfo( fileroot, kMaxFrame,
                                                 kMinFrame, kMaxFrame,
                                                 kMinFrame ) );
            opts.edl = true;
            continue;
         }
         else if ( ok )
         {
            Sequence s;
            s.root  = root;
            s.view  = view;
            s.number = frame;
            s.ext   = ext;

            tmpseqs.push_back( s );
         }
         else
         {
            opts.files.push_back( mrv::LoadInfo( *i, kMaxFrame, kMinFrame ) );
         }
      }


      //
      // Then, sort sequences and collapse them into a single file entry
      //
      std::sort( tmpseqs.begin(), tmpseqs.end(), SequenceSort() );


      {
	std::string root;
	std::string first;
	std::string number;
        std::string view;
	std::string ext;
	int zeros = -1;

	std::string seqname;
	Sequences seqs;

	{
	  Sequences::const_iterator i = tmpseqs.begin();
	  Sequences::const_iterator e = tmpseqs.end();
	  for ( ; i != e; ++i )
	    {

	      const char* s = (*i).number.c_str();
	      int z = 0;
	      for ( ; *s == '0'; ++s )
		++z;

	      if ( (*i).root != root || (*i).view != view ||
                   (*i).ext != ext || ( zeros != z && z != zeros-1 ) )
              {
		  // New sequence
		  if ( root != "" )
		    {
		      Sequence seq;
		      seq.root = seqname;
                      seq.view = (*i).view;
		      seq.number = seq.ext = first;
		      if ( first != number )
			{
			  seq.ext = number;
			}
		      seqs.push_back( seq );
		    }


		  root   = (*i).root;
		  zeros  = z;
		  number = first = (*i).number;
                  view   = (*i).view;
		  ext    = (*i).ext;


		  seqname  = root;
		  if ( z == 0 )
		    seqname += "%d";
		  else
		    {
		      seqname += "%0";
		      char buf[19]; buf[18] = 0;
#ifdef WIN32
		      seqname += itoa( int((*i).number.size()), buf, 10 );
#else
		      sprintf( buf, "%ld", (*i).number.size() );
		      seqname += buf;
#endif
		      seqname += "d";
		    }
                  seqname += view;
		  seqname += ext;
		}
	      else
		{
		  zeros  = z;
		  number = (*i).number;
		}
	    }
	}

	if ( root != "" )
	  {
	    Sequence seq;
	    seq.root = seqname;
            seqs.push_back( seq );
	  }

        {
           Sequences::const_iterator i = seqs.begin();
           Sequences::const_iterator e = seqs.end();

           for ( ; i != e; ++i )
           {
              boost::int64_t frameStart = kMaxFrame;
              boost::int64_t frameEnd   = kMinFrame;
              std::string file = (*i).root;
              get_sequence_limits( frameStart, frameEnd, file );
              opts.files.push_back( mrv::LoadInfo( file, frameStart, 
                                                   frameEnd ) );
           }

        }
      }
   }
}

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
    ahostname( "t", N_("host"), 
	       _("Override viewer's default client hostname."), false, 
	       opts.host, "string");

    ValueArg< unsigned short > 
    aport( N_("p"), N_("port"), 
	     _("Set viewer's default server/client port."), false, 
	     opts.port, "string");

    UnlabeledMultiArg< std::string > 
    afiles(_("files"),
	   _("Images, sequences or movies to display."), false, "images");

    MultiArg< std::string > 
    aaudio( N_("a"), N_("audio"), 
            _("Set each movie/sequence default audio."), false, "audio files");

    MultiArg< unsigned > 
    aoffset( N_("o"), N_("audio_offset"), 
             _("Set added audio offset."), false, "offset");

#ifdef USE_STEREO
    MultiArg< std::string > 
    astereo( N_("s"), N_("stereo"), 
            _("Provide two sequences or movies for stereo."), false, "images");
#endif

    cmd.add(agamma);
    cmd.add(again);
    cmd.add(ahostname);
    cmd.add(aport);
    cmd.add(aedl);
    cmd.add(afps);
    cmd.add(aaudio);
    cmd.add(aoffset);
#ifdef USE_STEREO
    cmd.add(astereo);
#endif
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
    opts.host = ahostname.getValue();
    opts.port = aport.getValue();
    opts.edl  = aedl.getValue();
    opts.fps  = afps.getValue();

    stringArray files = afiles.getValue();
    size_t normalFiles = files.size();

#ifdef USE_STEREO
    stringArray stereo = astereo.getValue();

    if ( stereo.size() % 2 != 0 )
    {
        std::cerr << _("--stereo flag needs to be specified a "
                       "multiple of two times.") << std::endl;
        exit(1);
    }

    //
    // Parse image list to split into sequences/images and reels
    //

    for ( int i = 0; i < stereo.size(); ++i )
    {
       files.push_back( stereo[i] ); 
    }

#endif

    const stringArray& audios = aaudio.getValue();
    const UnsignedArray& aoffsets = aoffset.getValue();

    if ( audios.size() < aoffsets.size() )
        LOG_ERROR( "Too many audio offsets for fewer audios" );

    stringArray::const_iterator i = files.begin();
    stringArray::const_iterator e = files.end();
    stringArray::const_iterator ai = audios.begin();
    stringArray::const_iterator ae = audios.end();
    UnsignedArray::const_iterator oi = aoffsets.begin();
    UnsignedArray::const_iterator oe = aoffsets.end();
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
	    boost::int64_t start = mrv::kMaxFrame, end = mrv::kMinFrame;
	    std::string fileroot;


            if ( mrv::is_directory( arg.c_str() ) )
            {
               parse_directory( arg, opts );
               if ( opts.files.size() > 1 ) opts.edl = true;
            }
            else
            {
               mrv::fileroot( fileroot, arg );

               if ( mrv::is_valid_sequence( fileroot.c_str() ) )
               {
                   mrv::get_sequence_limits( start, end, fileroot );
               }

               if ( (size_t)(e - i) <= files.size() - normalFiles )
               {
                  // Add audio file to last stereo fileroot
                  if ( ai != ae )
                  {
                      unsigned offset = 0;
                      if ( oi != oe ) {
                          offset = *oi;
                          ++oi;
                      }

                      opts.stereo.push_back( mrv::LoadInfo( fileroot, start, 
                                                            end, start, end,
                                                            *ai, "", offset ) );
                     ++ai;
                  }
                  else
                  {
                     opts.stereo.push_back( mrv::LoadInfo( fileroot, start,
                                                           end, start, end ) );
                  }
               }
               else
               {
                  // Add audio file to last fileroot
                  if ( ai != ae )
                  {
                      unsigned offset = 0;
                      if ( oi != oe ) {
                          offset = *oi;
                          ++oi;
                      }

                      opts.files.push_back( mrv::LoadInfo( fileroot, start, 
                                                           end, AV_NOPTS_VALUE,
                                                           AV_NOPTS_VALUE, 
                                                           *ai, "", offset ) );
                     ++ai;
                  }
                  else
                  {
                     opts.files.push_back( mrv::LoadInfo( fileroot, start, 
                                                          end ) );
                  }
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

