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
#include "gui/mrvMainWindow.h"
#include "gui/mrvVersion.h"
#include "mrViewer.h"
#include "standalone/mrvCommandLine.h"

#ifdef _WIN32
#include <windows.h>


void alloc_console()
{
    AllocConsole();
    freopen("conin$", "r", stdin);
    freopen("conout$", "w", stdout);
    freopen("conout$", "w", stderr);
}

#else

void alloc_console() {}

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
      freopen("conin$", "r", stdin);
      freopen("conout$", "w", stdout);
      freopen("conout$", "w", stderr);
#endif
      std::string& cmd = c.getProgramName();
      std::cerr << cmd << " v" << c.getVersion()
                << std::endl << std::endl
                << "Usage:" << std::endl << std::endl
                << cmd;

      //
      // Output usage line
      //

      std::list<Arg*> args = c.getArgList();
      for (ArgListIterator it = args.begin(); it != args.end(); ++it)
        {
            std::cerr << " " << (*it)->shortID();
        }

      //
      // Output options in shorter format than TCLAP's default
      //
      std::cerr << std::endl << std::endl
                << _("Options:") << std::endl << std::endl;
 
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
          std::cerr << "  " << ID;

          for ( size_t i = ID.size(); i < len; ++i )
            {
                std::cerr << " ";
            }
          std::cerr << "   " << (*it)->getDescription().c_str() << std::endl;
        }

      std::cerr << std::endl
                << _("Names cannot contain spaces in their paths.")
                << std::endl << std::endl
                << _("Examples:") << std::endl << std::endl;
      std::cerr << "  > " << cmd << " background.dpx texture.png" << std::endl
                << "  > " << cmd << " beauty.001-020.iff background.%%04d.exr 1-20" << std::endl 
                << "  > " << cmd << " beauty.mov -a dialogue.wav beauty.@@.iff 1-20 beauty.avi" << std::endl;

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
                                        ext, *i );

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
               int64_t frameStart = kMaxFrame;
               int64_t frameEnd   = kMinFrame;
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
bool parse_command_line( const int argc, char** argv,
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

    MultiArg< int >
    aoffset( N_("o"), N_("audio_offset"),
             _("Set added audio offset."), false, "offset");

    ValueArg< std::string >
    abg( N_("b"), N_("bg"),
         _("Provide a sequence or movie for background."), false, "", "image");

#ifdef USE_STEREO
    MultiArg< std::string >
    astereo( N_("s"), N_("stereo"),
            _("Provide two sequences or movies for stereo."), false, "images");

    ValueArg< std::string >
    astereo_input( N_(""), N_("stereo-input"),
		   _("Select stereo input"), false, "",
		   "string" );
    
    ValueArg< std::string >
    astereo_output( N_(""), N_("stereo-output"),
    		    _("Select stereo output"), false, "",
		    "string" );
    
#endif

    MultiArg< std::string >
    asub( "u", N_("sub"),
          _("Set subtitle for movie(s)."), false, "subtitles");

    SwitchArg adebug("d", "debug",
                    _("Set log to debug mode, flushing each time and saving an errorlog in .filmaura directory") );
    
    SwitchArg aplay("P", "playback",
                    _("Play video or sequence automatically without pausing at the beginning (Autoplayback)") );

    SwitchArg arun( "r", "run",
                    _("Run mrViewer regardless of single instance setting") );

    ValueArg<int> athreads( "", "threads",
			    _("Run mrViewer using this number of threads. 0 is as many threads as possible"), false, -1, "value" );

    cmd.add(adebug);
    cmd.add(arun);
    cmd.add(aplay);
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
    cmd.add(astereo_input);
    cmd.add(astereo_output);
#endif
    cmd.add(athreads);
    cmd.add(asub);
    cmd.add(abg);
    cmd.add(afiles);

    //
    // Do the parsing
    //
    cmd.parse( argc, argv );


    //
    // Extract the options
    //
    opts.play  = aplay.getValue();
    opts.gamma = agamma.getValue();
    opts.gain  = again.getValue();
    opts.host = ahostname.getValue();
    opts.port = aport.getValue();
    opts.edl  = aedl.getValue();
    opts.fps  = afps.getValue();
    opts.bgfile = abg.getValue();
    opts.run    = arun.getValue();
    opts.debug  = adebug.getValue();
    opts.stereo_output = astereo_output.getValue();
    opts.stereo_input = astereo_input.getValue();

    
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

    size_t num = stereo.size();
    for ( size_t i = 0; i < num; ++i )
    {
       files.push_back( stereo[i] );
    }

#endif

    const stringArray& subs = asub.getValue();
    if ( subs.size() > files.size() )
      LOG_ERROR( "Too many subtitles for image file(s)." );

    const stringArray& audios = aaudio.getValue();
    const IntArray& aoffsets = aoffset.getValue();

    if ( audios.size() < aoffsets.size() )
        LOG_ERROR( "Too many audio offsets for fewer audios" );


    stringArray::const_iterator i = files.begin();
    stringArray::const_iterator e = files.end();
    stringArray::const_iterator ai = audios.begin();
    stringArray::const_iterator ae = audios.end();
    stringArray::const_iterator si = subs.begin();
    stringArray::const_iterator se = subs.end();
    IntArray::const_iterator oi = aoffsets.begin();
    IntArray::const_iterator oe = aoffsets.end();
    for ( ; i != e; ++i )
      {
        std::string arg = *i;

        if ( arg.find( "file:" ) == 0 )
        {
            arg = arg.substr( 5, arg.size() );
        }

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
                bool avoid_seq = !ui->uiPrefs->uiPrefsLoadSequenceOnAssoc->value();
                if ( avoid_seq )
                {
                    fileroot = arg;
                }
                else
                {
                    mrv::fileroot( fileroot, arg );
                    if ( mrv::is_valid_sequence( fileroot.c_str() ) )
                    {
                        // we use original file to get
                        // start and end in case of ILM format
                        if (! mrv::get_sequence_limits( start, end, fileroot ) )
                        {
                            fileroot = arg;
                        }
                    }
                }

                std::string sub;
                if ( si != se )
                  {
                    sub = *si;
                    ++si;
                  }

                std::string audio;
                if ( ai != ae )
                  {
                    audio = *ai;
                  }

                int offset = 0;
                if ( oi != oe ) {
                  offset = *oi;
                  ++oi;
                }


               if ( (size_t)(e - i) <= files.size() - normalFiles )
               {
                  // Add audio file to last stereo fileroot
                 opts.stereo.push_back( mrv::LoadInfo( fileroot, start,
                                                       end, start, end,
                                                       audio, "", offset,
                                                       sub ) );
               }
               else
               {
                 // Add audio file to last fileroot
                 opts.files.push_back( mrv::LoadInfo( fileroot, start,
                                                      end, AV_NOPTS_VALUE,
                                                      AV_NOPTS_VALUE,
                                                      audio, "", offset,
                                                      sub ) );
               }
            }
          }
      }

    int threads = athreads.getValue();
    if ( threads >= 0 )
    {
	ui->uiPrefs->uiPrefsVideoThreadCount->value( threads );
    }

  }
  catch ( TCLAP::ArgException& e )
    {
      std::cerr << e.error() << " " << e.argId() << std::endl;
      return false;
    }

  if ( opts.debug )
  {
      alloc_console();
      mrv::io::logbuffer* log =
      static_cast<mrv::io::logbuffer*>( mrv::io::info.rdbuf() );
      log->debug(true);
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

  return true;
}


} // namespace mrv
