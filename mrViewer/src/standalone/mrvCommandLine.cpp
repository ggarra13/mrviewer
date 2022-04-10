/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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

#include <ImfStringAttribute.h>

#include <tclap/CmdLine.h>

#include "core/mrvI8N.h"
#include "core/mrvString.h"
#include "core/mrvServer.h"
#include "gui/mrvIO.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvImageView.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvVersion.h"
#include "mrvPreferencesUI.h"
#include "mrViewer.h"
#include "standalone/mrvCommandLine.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <libintl.h>

namespace fs = boost::filesystem;

namespace {
const char* kModule = "parser";
}

#define USE_STEREO

extern ViewerUI* ui;

namespace mrv {

typedef std::vector<unsigned> UnsignedArray;
typedef std::vector<int> IntArray;
typedef std::vector<float> floatArray;

  class CmdLineOutput : public TCLAP::StdOutput
  {
      bool newconsole;
  public:
      void open_console()
      {
#ifdef _WIN32
          newconsole = false;
          AttachConsole( -1 );
          DWORD err = GetLastError();
          if ( err == ERROR_INVALID_HANDLE ||
               err == ERROR_INVALID_PARAMETER )
          {
              newconsole = true;
              AllocConsole();
          }
          freopen("conout$", "w", stdout);
          freopen("conout$", "w", stderr);
#endif
      }

      void close_console()
      {
#if defined(_WIN32)||defined(_WIN64)
          if ( newconsole )
          {
              fclose(stdout);
              fclose(stderr);
              Sleep(INFINITE);
          }
#endif
      }

    virtual void version(TCLAP::CmdLineInterface& c)
      {
          open_console();
          std::cerr << std::endl
                    << c.getProgramName() << " v" << c.getVersion()
                    << std::endl
                    << mrv::build_date() << std::endl;
          close_console();
      }
    virtual void usage(TCLAP::CmdLineInterface& c)
    {
      using namespace TCLAP;
      using namespace std;

#ifdef _WIN32
      open_console();
#endif

      std::string cmd = c.getProgramName();
      fprintf( stderr, N_("\n%s v%s\n\n"), cmd.c_str(), c.getVersion().c_str() );

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
      fprintf( stderr, "%s", _("\n\nOptions:\n\n"));

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
      fprintf( stderr, "%s",
               _("Names cannot contain spaces in their paths.\n\n") );
      fprintf( stderr, "%s", _("Examples:\n\n") );
      fprintf( stderr, "  > %s background.dpx texture.png\n", cmd.c_str() );
      fprintf( stderr, "  > %s beauty.001-020.iff background.%%04d.exr 1-20\n", cmd.c_str() );
      fprintf( stderr, "  > %s beauty.mov -a dialogue.wav beauty.@@.iff 1-20 beauty.avi\n", cmd.c_str() );

      close_console();

    }


  };


void parse_directory( const std::string& fileroot,
                      mrv::Options& opts )
{

   std::string oldfile;
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
                      seqname += _itoa( int((*i).number.size()), buf, 10 );
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
              boost::int64_t frameStart = AV_NOPTS_VALUE;
              boost::int64_t frameEnd   = AV_NOPTS_VALUE;
              std::string file = (*i).root;
              get_sequence_limits( frameStart, frameEnd, file );
              opts.files.push_back( mrv::LoadInfo( file, frameStart,
                                                   frameEnd ) );
           }

        }
      }
   }
}

void add_attributes( LoadInfo& info, TCLAP::SwitchArg& rattrs,
                     TCLAP::MultiArg<std::string>& aattrs )
{
    info.replace_attrs = rattrs.getValue();
    stringArray attrs = aattrs.getValue();
    stringArray::const_iterator atf = attrs.begin();
    stringArray::const_iterator aef = attrs.end();
    for ( ; atf != aef; ++atf )
    {
        size_t pos = atf->find('=');
        if ( pos == std::string::npos )
        {
            LOG_ERROR( _("Invalid attribute ") << *atf
                       << _(".  Attribute must be of the form attr=value.") );
            continue;
        }
        std::string attr, value;
        attr = atf->substr(0,pos);
        value = atf->substr(pos+1, atf->size() );
        Imf::StringAttribute* val = new Imf::StringAttribute(value);
        CMedia::Attributes& cm = info.attrs;
        cm.insert( std::make_pair( attr, val ) );
    }
}

//
// Command-line parser
//
void parse_command_line( const int argc, const char** argv,
                         mrv::Options& opts )
{

  // Some default values
    opts.single = false;
  opts.gamma = (float)1.0; //ui->uiPrefs->uiPrefsViewGamma->value();
  opts.gain  = (float)1.0; //ui->uiPrefs->uiPrefsViewGain->value();
  opts.fps   = -1.f;
  opts.host = "";
  opts.port = 0;

  try {
    using namespace TCLAP;


    DBG;
    CmdLine cmd(
                _("A professional image and movie viewer\n"
                  "Examples:\n")
                 , ' ', mrv::version() );

    DBG;
    //
    // set the output
    //
    mrv::CmdLineOutput my;
    DBG;
    cmd.setOutput(&my);
    DBG;

    ValueArg<int> adebug( "d", "debug", _("Turn on debugging console." ),
                          false, -1, "int" );

    DBG;
    SwitchArg aplay("P", "playback",
                    _("Play video or sequence automatically without pausing at the beginning (Autoplayback).") );

    SwitchArg asingle("S", "single_image",
                      _("Load a single image even if there's a sequence present.") );

    SwitchArg arun( "r", "run",
                    _("Run mrViewer regardless of single instance setting.") );

    //
    // The command-line arguments (parsed in order)
    //
    SwitchArg aedl("e", "edl",
                   _("Turn on EDL when loading multiple images.") );


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

    ValueArg< std::string >
        abg( N_("b"), N_("bg"),
             _("Provide a sequence or movie for background."), false, "", "image");

    MultiArg< std::string >
    acolorspace( N_("c"), N_("colorspace"),
            _("Set each input color space for each image."), false, "color space");

    MultiArg< std::string >
    aaudio( N_("a"), N_("audio"),
            _("Set each movie/sequence default audio."), false, "audio file");

    SwitchArg
    rattrs( N_("R"), N_("replace_attr"),
            _("Replace all attribute/values to display in the hud, leaving only those specified in the command line."), false);

    MultiArg< std::string >
    aattrs( N_("A"), N_("attr"),
            _("Set an attribute/value to display in the hud."), false,
            "attr=value");

    MultiArg< int >
    aoffset( N_("o"), N_("audio_offset"),
             _("Set added audio offset."), false, "offset");

#ifdef USE_STEREO
    MultiArg< std::string >
    astereo( N_("s"), N_("stereo"),
            _("Provide two sequences or movies for stereo."), false, "image");


    ValueArg< std::string >
        astereo_input( N_(""), N_("stereo-input"),
                       _("Select stereo input"), false, "",
                       "string" );

    ValueArg< std::string >
        astereo_output( N_(""), N_("stereo-output"),
                        _("Select stereo output"), false, "",
                        "string" );
#endif

    cmd.add(adebug);
    DBG;
    cmd.add(arun);
    cmd.add(aplay);
    cmd.add(asingle);
    cmd.add(agamma);
    cmd.add(again);
    cmd.add(ahostname);
    cmd.add(aport);
    cmd.add(aedl);
    cmd.add(afps);
    cmd.add(aaudio);
    cmd.add(rattrs);
    cmd.add(aattrs);
    cmd.add(acolorspace);
    cmd.add(aoffset);
#ifdef USE_STEREO
    cmd.add(astereo);
    cmd.add(astereo_input);
    cmd.add(astereo_output);
#endif
    cmd.add(abg);
    cmd.add(afiles);

    //
    // Do the parsing
    DBG;
    //
    cmd.parse( argc, argv );

    DBG;

    //
    // Extract the options
    //
    opts.play   = aplay.getValue();
    opts.single = asingle.getValue();
    opts.gamma  = agamma.getValue();
    opts.gain   = again.getValue();
    opts.host = ahostname.getValue();
    opts.port = aport.getValue();
    opts.edl  = aedl.getValue();
    opts.fps  = afps.getValue();
    opts.bgfile = abg.getValue();
    opts.run    = arun.getValue();
    opts.stereo_output = astereo_output.getValue();
    opts.stereo_input = astereo_input.getValue();


#if defined(OSX)
    stringArray infiles = afiles.getValue();
    stringArray files;
    stringArray::const_iterator f = infiles.begin();
    stringArray::const_iterator ef = infiles.end();
    for ( ; f != ef; ++f )
    {
        if ( f->substr( 0, 5 ) == "-psn_" )
            continue;
        files.push_back( *f );
    }
#else
    stringArray files = afiles.getValue();
#endif

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

    /// CREATE THE MAIN INTERFACE
    ui = new ViewerUI;


    int debug = adebug.getValue();

    Preferences::debug = debug;
    if ( debug > 0 ) mrv::io::logbuffer::debug( true );

    const stringArray& colorspaces = acolorspace.getValue();
    if ( colorspaces.size() != files.size() && !colorspaces.empty() )
    {
        LOG_ERROR( "Color spaces must match the number of files" );
    }
    const stringArray& audios = aaudio.getValue();
    const IntArray& aoffsets = aoffset.getValue();

    if ( audios.size() < aoffsets.size() )
        LOG_ERROR( "Too many audio offsets for fewer audios" );

    stringArray::const_iterator i = files.begin();
    stringArray::const_iterator e = files.end();
    stringArray::const_iterator ci = colorspaces.begin();
    stringArray::const_iterator ce = colorspaces.end();
    stringArray::const_iterator ai = audios.begin();
    stringArray::const_iterator ae = audios.end();
    IntArray::const_iterator oi = aoffsets.begin();
    IntArray::const_iterator oe = aoffsets.end();
    for ( ; i != e; ++i )
      {
        const std::string& arg = *i;

        // Check if string is a range.  if so, change last sequence
        if ( !opts.files.empty() && (opts.files.back().reel == false) &&
             mrv::matches_chars( arg.c_str(), "0123456789-") )
          {
            continue;
          }


        size_t len = arg.size();
        if ( len > 5 && ( arg.substr(len - 5, 5) == ".reel" ||
                          arg.substr(len - 5, 5) == ".otio" ) )
          {
            opts.files.push_back( mrv::LoadInfo(arg) );

            add_attributes( opts.files.back(), rattrs, aattrs );

          }
        else
        {
            int64_t start = AV_NOPTS_VALUE, end = AV_NOPTS_VALUE;
            std::string fileroot;

            if ( mrv::is_directory( arg.c_str() ) )
            {
               parse_directory( arg, opts );
               if ( opts.files.size() > 1 ) opts.edl = true;
            }
            else
            {
                if ( (i+1) != e )
                {
                    const std::string& f = *(i+1);
                    if ( mrv::matches_chars( f.c_str(), "0123456789-") )
                    {
                        stringArray tokens;
                        mrv::split_string( tokens, f, "-" );

                        // frame range
                        start = atoi( tokens[0].c_str() );
                        if ( tokens.size() > 1 )
                            end = atoi( tokens[1].c_str() );
                    }
                }

                mrv::fileroot( fileroot, arg, true, false );
                if ( mrv::is_valid_sequence( fileroot.c_str() ) )
                {
                   mrv::get_sequence_limits( start, end, fileroot );
                }


                if ( (size_t)(e - i) <= files.size() - normalFiles )
                {
                    // Add audio file to last stereo fileroot
                    if ( ai != ae )
                    {
                        int offset = 0;
                        if ( oi != oe ) {
                            offset = *oi;
                            ++oi;
                        }

                        opts.stereo.push_back( mrv::LoadInfo( fileroot, start,
                                                              end, start, end,
                                                              opts.fps,
                                                              *ai, "",
                                                              offset ) );
                        ++ai;
                    }
                    else
                    {
                      opts.stereo.push_back( mrv::LoadInfo( fileroot, start,
                                                            end, start, end,
                                                            opts.fps ) );
                    }

                    if ( ci != ce )
                    {
                        opts.stereo.back().colorspace = *ci; ++ci;
                    }
                }
               else
               {
                  // Add audio file to last fileroot
                  if ( ai != ae )
                  {
                      int offset = 0;
                      if ( oi != oe ) {
                          offset = *oi;
                          ++oi;
                      }

                      opts.files.push_back( mrv::LoadInfo( fileroot, start,
                                                           end, AV_NOPTS_VALUE,
                                                           AV_NOPTS_VALUE,
                                                           opts.fps,
                                                           *ai, "", offset ) );
                     ++ai;
                  }
                  else
                  {
                      if ( ui->uiPrefs->uiPrefsLoadSequenceOnAssoc->value() &&
                          ! opts.single )
                      {
                          opts.files.push_back( mrv::LoadInfo( fileroot, start,
                                                               end,
                                                               start, end,
                                                               opts.fps
                                                    ) );
                      }
                      else
                      {
                          opts.files.push_back( mrv::LoadInfo( arg ) );
                      }
                  }

                  if ( ci != ce )
                  {
                      opts.files.back().colorspace = *ci; ++ci;
                  }

                  if ( opts.files.empty() ) continue;

                  add_attributes( opts.files.back(), rattrs, aattrs );
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


  if ( opts.gamma == 1.0f ) opts.gamma = ui->uiPrefs->uiPrefsViewGamma->value();
  if ( opts.gain  == 1.0f ) opts.gain = ui->uiPrefs->uiPrefsViewGain->value();

  ui->uiView->gamma( opts.gamma );
  ui->uiGammaInput->value( opts.gamma );
  ui->uiGamma->value( opts.gamma );

  ui->uiView->gain( opts.gain );

}


} // namespace mrv
