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
 * @file   Sequence.cpp
 * @author gga
 * @date   Sat Jul 21 04:03:15 2007
 *
 * @brief
 *
 *
 */
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64 macro

#include "core/mrvFrame.h"

#include <sys/types.h>
#include <sys/stat.h>

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#define getcwd _getcwd
#define S_ISDIR(x) _S_IFDIR(x)
#else
#include <unistd.h>
#endif



#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <limits>

#include <FL/Fl.H>
#include <FL/fl_utf8.h>
#include <FL/fl_draw.H>

using namespace std;

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
namespace fs = boost::filesystem;


// #define LOG_ERROR(x) std::cerr << x << std::endl
#include <opentimelineio/clip.h>
#include <opentimelineio/deserialization.h>
#include <opentimelineio/effect.h>
#include <opentimelineio/freezeFrame.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/linearTimeWarp.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/transition.h>
#include "opentimelineio/externalReference.h"
#include <opentimelineio/stackAlgorithm.h>


#include <iostream>

namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;
namespace otime = opentime::OPENTIME_VERSION;


#include "gui/mrvImageBrowser.h"
#include "gui/mrvImageView.h"
#include "video/mrvGLShape.h"
#include "core/Sequence.h"
#include "core/mrvString.h"
#include "core/mrvTransition.h"
#include "mrvI8N.h"
#include "mrvOS.h"

#include "gui/mrvIO.h"
#include "mrvPreferencesUI.h"
#include "mrViewer.h"

namespace {
const char* kModule = "seq";
}

namespace mrv
{
const boost::int64_t kMinFrame = AV_NOPTS_VALUE;
const boost::int64_t kMaxFrame = AV_NOPTS_VALUE;

bool is_valid_frame( const std::string& framespec )
{
    if ( framespec.size() > 17 ) return false;

    const char* c = framespec.c_str();
    if ( *c == '.' ) ++c;

    for ( ; *c != 0; ++c )
    {
        if ( *c == '+' || *c == '-' || (*c >= '0' && *c <= '9') ) continue;

        return false;
    }

    return true;
}

bool is_valid_frame_spec( std::string& framespec )
{
    const char* c;
    if ( framespec.substr(0,1) == "." )
        c = framespec.c_str() + 1;
    else
        c = framespec.c_str();

    if ( framespec.size() > 17 ) return false;


    if ( *c == '%' )
    {
        bool d = false;
        bool l = false;
        for ( ++c; *c != 0; ++c )
        {
            if (( *c >= '0' && *c <= '9' ) )
                continue;
            if ( !d && *c == 'd' )
            {
                d = true;
                continue;
            }
            if ( !l && *c == 'l' )
            {
                l = true;
                continue;
            }
            return false;
        }
        return true;
    }
    else if ( *c == '#' || *c == '@' )
    {
        char t = *c;
        for ( ++c; *c != 0; ++c )
        {
            if ( *c != t ) return false;
        }
        return true;
    }

    int idx = 0;
    bool range_found = false;
    for ( ++c; *c != 0; ++c )
    {
        if ( !range_found ) {
            ++idx;
            if ( *c == '-' ) {
                range_found = true;
                continue;
            }
        }
        if ( *c != '+' && *c >= '0' && *c <= '9' ) continue;

        return false;
    }

    framespec = framespec.substr(0, idx);


    return range_found;
}


// Given a frame extension, return true if a possible movie file.
bool is_valid_movie( const char* ext )
{
    if ( ext == NULL ) return false;

    std::string tmp = ext;
    std::transform( tmp.begin(), tmp.end(), tmp.begin(),
                    (int(*)(int)) tolower);
    if ( tmp[0] != '.' ) tmp = '.' + tmp;

    if ( tmp == ".3gp"   || tmp == ".asf"   ||
         tmp == ".avc"   || tmp == ".avchd" ||
         tmp == ".avi"   || tmp == ".braw"  ||
         tmp == ".divx"  || tmp == ".dv"    ||
         tmp == ".flv"   || tmp == ".gif"   ||
         tmp == ".m2ts"  || tmp == ".m2t"   ||
         tmp == ".m4v"   || tmp == ".mkv"   ||
         tmp == ".mov"   || tmp == ".mp4"   ||
         tmp == ".mpeg"  || tmp == ".mpg"   ||
         tmp == ".mvb"   || tmp == ".mxf"   ||
         tmp == ".ogg"   || tmp == ".ogm"   ||
         tmp == ".ogv"   || tmp == ".qt"    ||
         tmp == ".r3d"   || tmp == ".reel"  ||
         tmp == ".session" ||
         tmp == ".rm"    || tmp == ".ts"    ||
         tmp == ".vob"   || tmp == ".vp9"   ||
         tmp == ".webm"  || tmp == ".wmv"  )
    {
        return true;
    }
    return false;
}


// Given a frame extension, return true if a possible audio file.
bool is_valid_audio( const char* ext )
{
    std::string tmp = ext;
    std::transform( tmp.begin(), tmp.end(), tmp.begin(),
                    (int(*)(int)) tolower);
    if ( tmp[0] != '.' ) tmp = '.' + tmp;

    if ( tmp == N_(".aiff") ||
         tmp == N_(".flac") ||
         tmp == N_(".mp3") ||
         tmp == N_(".ogg") ||
         tmp == N_(".opus") ||
         tmp == N_(".snd") ||
         tmp == N_(".vorbis") ||
         tmp == N_(".wav") )
        return true;

    return false;
}

// Given a frame extension, return true if a possible audio file.
bool is_valid_subtitle( const char* ext )
{
    std::string tmp = ext;
    std::transform( tmp.begin(), tmp.end(), tmp.begin(),
                    (int(*)(int)) tolower);
    if ( tmp[0] != '.' ) tmp = '.' + tmp;

    if ( tmp == N_(".srt")  ||
         tmp == N_(".sub")  ||
         tmp == N_(".ass")  ||
         tmp == N_(".vtt") )
        return true;

    return false;
}

// Given a frame extension, return true if a possible picture file.
bool is_valid_picture( const char* ext )
{
    std::string tmp = ext;
    std::transform( tmp.begin(), tmp.end(), tmp.begin(),
                    (int(*)(int)) tolower);

    if ( tmp == ".iff"   || tmp == ".pic" || tmp == ".tif" ||
            tmp == ".tiff"  || tmp == ".png" || tmp == ".jpg" ||
            tmp == ".jpeg"  || tmp == ".tga" || tmp == ".exr" ||
            tmp == ".dpx"   || tmp == ".cin" || tmp == ".bmp" ||
            tmp == ".bit"   || tmp == ".sxr" || tmp == ".ct"  ||
            tmp == ".sgi"   || tmp == ".st"  || tmp == ".map" ||
            tmp == ".sxr"   || tmp == ".nt"  || tmp == ".mt"  ||
            tmp == ".psd"   || tmp == ".rgb" || tmp == ".rpf" ||
            tmp == ".shmap" || tmp == ".zt" )
        return true;

    return false;
}

std::string hex_to_char_filename( std::string& f )
{
    std::string r;
    size_t loc;

    while ( ( loc = f.find('%') ) != std::string::npos )
    {

        r += f.substr(0, loc);

        std::string hex = f.substr( loc+1, 2 );

        int dec = (int) strtoul( hex.c_str(), 0, 16 );

        char buf[2];
        buf[1] = 0;
        sprintf( buf, "%c", dec );

        r += buf;

        f = f.substr( loc+3, f.size() );
    }

    r += f;

    return r;
}

std::string get_short_view( bool left )
{
    const char* pairs = fl_getenv("MRV_STEREO_CHAR_PAIRS");
    if ( ! pairs ) pairs = "L:R";

    std::string view = pairs;
    size_t idx = view.find( ':' );
    if ( idx == std::string::npos )
    {
        LOG_ERROR( "MRV_STEREO_CHAR_PAIRS does not have two letters separated by colon" );
        if ( left )
            return "L";
        else
            return "R";
    }

    if (left)
        return view.substr( 0, idx );
    else
        return view.substr( idx+1, view.size() );
}

std::string get_long_view( bool left )
{
    const char* pairs = fl_getenv("MRV_STEREO_NAME_PAIRS");
    if ( ! pairs ) pairs = "left:right";

    std::string view = pairs;
    size_t idx = view.find( ':' );
    if ( idx == std::string::npos )
    {
        LOG_ERROR( "MRV_STEREO_NAME_PAIRS does not have two names separated by colon" );
        if ( left )
            return "left";
        else
            return "right";
    }

    if ( left )
        return view.substr( 0, idx );
    else
        return view.substr( idx+1, view.size() );
}

bool replace_view( std::string& view )
{
    if ( view.empty() ) return false;

    if ( view.substr( view.size()-1, view.size() ) == "." )
        view = view.substr( 0, view.size()-1 );


    if ( view == "%v" ||
            view == get_short_view(true) ||
            view == get_short_view(false) )
    {
        view = "%v";
        return true;
    }

    if ( view == "%V" ||
            view == get_long_view(true) ||
            view == get_long_view(false) )
    {
        view = "%V";
        return true;
    }
    return false;
}

bool is_valid_view( std::string view )
{
    return replace_view( view );
}

/**
 * Given a filename of a possible sequence, split it into
 * root name, frame string, view, and extension
 *
 * @param root        root name of file sequence
 * @param frame       frame part of file (must be @ or # or %d or a number)
 * @param view        view of image (%v or %V from left or right or L and R )
 * @param ext         extension of file sequence
 * @param file        original filename, potentially part of a sequence.
 * @param change_view change view to %v or %V if left/right or L/R is found.
 * @param change_frame change frame into frame spec
 *
 * @return true if a sequence, false if not.
 */
bool split_sequence(
                    std::string& root,
                    std::string& frame,
                    std::string& view,
                    std::string& ext,
                    const std::string& file,
                    const bool change_view,
                    const bool change_frame
                    )
{
    std::string f = file;

    root = frame = view = ext = "";

    const char* e = f.c_str();
    const char* i = e + f.size() - 1;
    for ( ; i >= e; --i )
    {
        if ( *i == '/' || *i == '\\' ) break;
    }

    size_t len = i - e + 1;
    f = f.substr( len, f.size() );

    stringArray periods;
    split(periods, f, '.');

    if ( periods.size() == 4 )
    {
        root = file.substr( 0, len ) + periods[0] + ".";
        if ( root.find( "file://" ) == 0 )
            root = root.substr(7, root.size());
        view = periods[1];
        frame = periods[2];
        ext = '.' + periods[3];

        if ( change_view )
        {
            bool ok = replace_view( view );
        }

        if ( !view.empty() ) view += ".";
        if ( mrv::is_valid_movie( ext.c_str() ) ||
             mrv::is_valid_audio( ext.c_str() ) )
        {
            if ( frame != "" && ( ext == ".gif" || ext == ".GIF" ) )
                return true;
            root += view + frame + ext;
            view = "";
            frame = "";
            ext = "";
            return false;
        }
        else
        {
            if ( is_valid_frame( frame ) ||
                 is_valid_frame_spec( frame ) )
                return true;
            return false;
        }
    }
    else if ( periods.size() == 3 )
    {
        root = file.substr( 0, len ) + periods[0] + ".";
        if ( root.find( "file://" ) == 0 )
            root = root.substr( 7, root.size() );
        frame = periods[1];
        ext = '.' + periods[2];
        if ( mrv::is_valid_movie( ext.c_str() ) ||
             mrv::is_valid_audio( ext.c_str() ) )
        {
            if ( frame != "" && ( ext == ".gif" || ext == ".GIF" ) )
                return true;

            if ( ! mrv::is_valid_frame( frame ) &&
                 ! mrv::is_valid_frame_spec( frame ) &&
                 mrv::is_valid_view( frame ) )
            {
                view = periods[1];
                if ( change_view ) replace_view( view );
                if ( change_frame ) frame = "";
            }
            root += view;
            root += frame;
            root += ext;
            frame = ext = view = "";
            return false;
        }
        if ( !change_frame ) return true;
    }


    f = file;

    int idx[2];
    int count = 0;  // number of periods found (from end)

    int minus_idx = -1; // index where last - sign was found.
    int minus = 0; // number of minus signs found

    e = f.c_str();
    i = e + f.size() - 1;
    for ( ; i >= e; --i )
    {
        if ( *i == '/' || *i == '\\' ) break;
        if ( *i == '.' || ( count > 0 && (*i == '_') ) )
        {
            idx[count] = (int)( i - e );
            ++count;
            if ( count == 2 ) break;
            continue;
        }

        if ( count == 1 && (*i != '@' && *i != '#' && *i != 'd' &&
                            *i != 'l' && *i != '%' && *i != '-' &&
                            *i != 'I' && (*i < '0' || *i > '9')) )
            break;
        if ( count == 1 && *i == '-' )
        {
            minus_idx = (int)(i - e);
            minus++;
        }
    }

    if ( count == 1 && minus == 1 )
    {
        idx[count] = minus_idx;
        ++count;
    }

    if ( count == 0 ) return false;

    if ( count == 2 && minus < 2 )
    {
        root  = f.substr( 0, idx[1]+1 );
        if ( root.find( "file://" ) == 0 )
            root = root.substr( 7, root.size() );
        frame = f.substr( idx[1]+1, idx[0]-idx[1]-1 );
        ext   = f.substr( idx[0], file.size()-idx[0] );

        bool ok = is_valid_frame( frame );
        if ( ok && ( !is_valid_movie( ext.c_str() ) ||
                     mrv::is_valid_audio( ext.c_str() ) ) )
        {
            return true;
        }


        ok = is_valid_frame( ext );
        if ( ok )
        {
            frame = ext;
            ext.clear();
        }

        if ( is_valid_movie( ext.c_str() ) ||
             mrv::is_valid_audio( ext.c_str() ) )
        {
            if ( frame != "" && ( ext == ".gif" || ext == ".GIF" ) )
                return true;

            root = "";
            return false;
        }

        ok = is_valid_frame_spec( frame );
        return ok;
    }
    else
    {
        root = f.substr( 0, idx[0]+1 );
        if ( root.find( "file://" ) == 0 )
            root = root.substr( 7, root.size() );
        ext  = f.substr( idx[0]+1, file.size() );

        if ( is_valid_movie( ext.c_str() ) ||
             is_valid_audio( ext.c_str() ) )
        {
            frame = "";
            return false;
        }

        bool ok = is_valid_frame_spec( ext );
        if (ok)
        {
            frame = ext;
            ext.clear();
            return true;
        }

        ok = is_valid_frame( ext );
        if (ok)
        {
            frame = ext;
            ext.clear();
            return false;
        }

        //
        // Handle image0001.exr
        //
        std::string tmp = '.' + ext;
        bool valid = is_valid_movie( tmp.c_str() ) ||
                     mrv::is_valid_audio( ext.c_str() );
        size_t len = root.size();
        if ( len >= 2 && !valid )
        {
            if ( root.find( "file://" ) == 0 )
                root = root.substr( 7, root.size() );
            size_t pos;
            std::string fspec;
            if ( ( pos = root.rfind('%') ) != std::string::npos ||
                 ( pos = root.find('@') ) != std::string::npos ||
                 ( pos = root.rfind('#') ) != std::string::npos )
            {
                fspec = root.substr( pos, root.size() - pos - 1 );
                if ( is_valid_frame_spec( fspec ) )
                {
                    root  = root.substr( 0, pos );
                    frame = fspec;
                    ext   = tmp;
                    return true;
                }
            }

            int count = 0;
            int i = (int)len - 2; // account for period
            const char* c = root.c_str();
            while ( c[i] >= '0' && c[i] <= '9' )
            {
                --i;
                ++count;
                if ( i == -1 ) break;
            }

            if ( count > 0 && i < int( root.size() - 2 ) )
            {
                ++i;
                frame = root.substr( i, count );
                root  = root.substr( 0, i );

                size_t pos = root.rfind('/');
                size_t pos2 = root.rfind('\\');
                size_t pos3 = root.find( ':' );
                if ( pos == std::string::npos ||
                     ( pos2 != std::string::npos && pos2 > pos ) ) pos = pos2;
                if ( pos == std::string::npos ||
                     ( pos3 != std::string::npos && pos3 > pos ) ) pos = pos3;

                if ( root.empty() || pos != std::string::npos )
                {
                    // Check if filename is empty, like image: 324.exr
                    std::string file;
                    if ( !root.empty() )
                        file = root.substr( pos+1, root.size() );
                    if ( file.empty() ) {
                        if ( is_valid_frame( frame ) ||
                             is_valid_frame_spec( frame ) )
                        {
                            ext = tmp;
                            return true;
                        }
                        else
                        {
                            root = frame = ext = "";
                        }
                        return false;
                    }
                }
                ext   = tmp;
                return true;
            }
        }

        frame = "";
        return false;
    }
}


bool get_sequence_limits( boost::int64_t& frameStart,
                          boost::int64_t& frameEnd,
                          std::string& fileroot,
                          const bool error )
{
    if ( (frameStart != AV_NOPTS_VALUE &&
          frameEnd != AV_NOPTS_VALUE) ) {
        return true;
    }

    if ( fileroot.find( "http" ) == 0 ||
         fileroot.find( "bluray" ) == 0 ||
         fileroot.find( "dvd" ) == 0 ||
         fileroot.find( "/dev/sr0" ) == 0 ||
         fileroot.find( "rtmp" ) == 0 ||
         fileroot.find( "rtp" ) == 0 ||
         fileroot.find( "srtp" ) == 0 ||
         fileroot.find( "youtube" ) == 0 ||
         fileroot.find( "www." ) == 0 )
    {
        return false;
    }

    // Global encoding type taken from environment
    fs::path::imbue( std::locale() );

    fs::path file = fs::path( fileroot.c_str() );
    fs::path dir = file.branch_path();

    char buf[1024];
    if ( dir.string() == "" ) {
        dir = fs::path( getcwd(buf,1024) );
    }

    if ( ( !fs::exists( dir ) ) || ( !fs::is_directory( dir ) ) )
    {
        if ( error )
        {
            LOG_ERROR( _("Directory ") << dir <<
                       _(" does not exist or no directory") );
        }
        return false;
    }

    // Check if sequence is in ILM format first  ( image.1-30.exr )
    stringArray tokens;
    split( tokens, fileroot, '.' );
    if ( tokens.size() > 2 )
    {
        int idx = 2;

        const std::string& range = tokens[ tokens.size()-idx ];

        if ( mrv::matches_chars(range.c_str(), "0123456789-") )
        {
            stringArray frames;
            split( frames, range, '-' );
            if ( frames.size() > 1 && !frames[0].empty())
            {
                unsigned digits = (unsigned) frames[0].size();

                frameStart = atoi( frames[0].c_str() );
                frameEnd   = atoi( frames[1].c_str() );

                if ( frameEnd < frameStart ) {
                    frameEnd = frameStart = AV_NOPTS_VALUE;
                    return false;
                }

                stringArray::iterator i = tokens.begin();
                stringArray::iterator e = tokens.end();
                fileroot = tokens[0];
                ++i;
                for ( ; i != e; ++i )
                {
                    fileroot += ".";
                    if ( *i == range )
                    {
                        char buf[64];
                        const char* pr = PRId64;
                        if ( digits < 10 ) pr = "d";
                        sprintf( buf, "%%0%d%s", digits, pr );
                        fileroot += buf;
                    }
                    else
                    {
                        fileroot += *i;
                    }
                }


                return true;
            }
        }
    }


    std::string root, frame, view, ext;
    if ( ! split_sequence( root, frame, view, ext, file.leaf().string() ) )
    {
        return false; // NOT a recognized sequence
    }


    std::string croot, cview, cframe, cext;
    unsigned pad = 0;
    if ( is_valid_frame_spec( frame ) )
    {
        pad = padded_digits( frame );
    }


    fs::directory_iterator e; // default constructor yields path iter. end
    for ( fs::directory_iterator i( dir ) ; i != e; ++i )
    {
        if ( !fs::exists( *i ) || fs::is_directory( *i ) ) continue;

        std::string tmp = (*i).path().leaf().generic_string();


        // Do not continue on false return of split_sequence
        if ( ! split_sequence( croot, cframe, cview, cext, tmp ) )
        {
            continue;
        }

        if ( cext != ext || croot != root || cview != view )
        {
            continue;  // not this sequence
        }

        if ( cframe[0] == '0' && cframe.size() > 1 && pad == 0 )
            pad = (unsigned) cframe.size();


        boost::int64_t f = atoi( cframe.c_str() );

        if ( f < frameStart || frameStart == AV_NOPTS_VALUE ) frameStart = f;
        if ( f > frameEnd || frameEnd == AV_NOPTS_VALUE  )   frameEnd   = f;
    }

    const char* prdigits = PRId64;
    if ( pad < 10 ) prdigits = "d";

    sprintf( buf, "%%0%d%s", pad, prdigits );



    if ( ! split_sequence( root, frame, view, ext, fileroot ) )
    {
        return false;
    }

    fileroot = root;
    fileroot += view;
    fileroot += buf;
    fileroot += ext;

    return true;
}


bool parse_reel( mrv::LoadList& sequences, bool& edl,
                 short int& ghost_previous, short int& ghost_next,
                 const char* reelfile)
{
    edl = false;
    ghost_previous = ghost_next = 5;


    char* oldloc = av_strdup( setlocale( LC_NUMERIC, NULL ) );
    setlocale( LC_NUMERIC, "C" );

    FILE* f = fl_fopen( reelfile, "r" );
    if (!f ) {
        setlocale( LC_NUMERIC, oldloc );
        av_free( oldloc );
        return false;
    }

    double version = 1.0;
    char buf[16000];
    while ( !feof(f) )
    {
        char* c;
        while ( (c = fgets( buf, 15999, f )) )
        {
            if ( c[0] == '#' ) continue;  // comment line
            while ( *c != 0 && ( *c == ' ' || *c == '\t' ) ) ++c;
            if ( strlen(c) <= 1 ) continue; // empty line
            c[ strlen(c)-1 ] = 0;  // remove newline

            if ( strncmp( "audio: ", c, 7 ) == 0 )
            {
                if ( !sequences.empty() )
                {
                    std::string audio = c+7;
                    if (c[7] != '/' && c[8] != ':' )
                    {
                        fs::path dir = reelfile;
                        dir = dir.parent_path();
                        std::string path = dir.generic_string();
                        if ( !path.empty() ) path += '/';
                        audio = path + audio;
                    }
                    if ( audio.rfind('\r') == audio.size() - 1 )
                    {
                        audio = audio.substr( 0, audio.size() - 1 );
                    }
                    sequences.back().audio = audio;
                }
                continue;
            }

            if ( strncmp( "audio offset: ", c, 13 ) == 0 )
            {
                if ( !sequences.empty() )
                {
                    int64_t offset = atoi( c+14 );
                    sequences.back().audio_offset = offset;
                }
                continue;
            }

            if ( strncmp( "stereo: ", c, 8 ) == 0 )
            {
                if ( !sequences.empty() )
                {
                    std::string file = c+8;
                    if ( file.rfind('\r') == file.size() - 1 )
                    {
                        file = file.substr( 0, file.size() - 1 );
                    }
                    sequences.back().right_filename = c+8;
                }
                continue;
            }

            boost::int64_t first = kMinFrame;
            boost::int64_t last  = kMaxFrame;


            std::string st = c;
            std::istringstream is( st );
            is.imbue( std::locale("C") );
            std::string cmd;
            is >> cmd;

            if ( cmd == "EDL" )
            {
                edl = true;
                continue;
            }
            else if ( cmd == "Version" )
            {
                is >> version;
                continue;
            }
            else if ( cmd == "Ghosting" )
            {
                is >> ghost_previous >> ghost_next;
                continue;
            }
            else if ( cmd == "GLPathShape" )
            {
                Point xy;
                std::string points;
                GLPathShape* shape = new GLPathShape;
                std::getline( is, points );
                is.str( points );
                is.clear();
                is >> shape->r >> shape->g >> shape->b >> shape->a
                   >> shape->pen_size
                   >> shape->frame;
                while ( is >> xy.x >> xy.y )
                {
                    shape->pts.push_back( xy );
                }
                if ( !sequences.empty() )
                {
                    sequences.back().shapes.push_back(
                        mrv::shape_type_ptr(shape) );
                }
                continue;
            }
            else if ( cmd == "GLCircleShape" )
            {
                Point xy;
                std::string points;
                GLCircleShape* shape = new GLCircleShape;
                is.clear();
                std::getline( is, points );
                is.str( points );
                is.clear();
                is >> shape->r >> shape->g >> shape->b >> shape->a
                   >> shape->pen_size >> shape->frame;
                is >> shape->center.x >> shape->center.y;
                is >> shape->radius;
                if ( !sequences.empty() )
                {
                    sequences.back().shapes.push_back(
                        mrv::shape_type_ptr(shape) );
                }
                continue;
            }
            else if ( cmd == "GLArrowShape" )
            {
                Point xy;
                std::string points;
                GLArrowShape* shape = new GLArrowShape;
                is.clear();
                std::getline( is, points );
                is.str( points );
                is.clear();
                is >> shape->r >> shape->g >> shape->b >> shape->a
                   >> shape->pen_size >> shape->frame;
                while ( is >> xy.x >> xy.y )
                {
                    shape->pts.push_back( xy );
                }
                if ( !sequences.empty() )
                {
                    sequences.back().shapes.push_back(
                        mrv::shape_type_ptr(shape) );
                }
                continue;
            }
            else if ( cmd == "GLRectangleShape" )
            {
                Point xy;
                std::string points;
                GLRectangleShape* shape = new GLRectangleShape;
                is.clear();
                std::getline( is, points );
                is.str( points );
                is.clear();
                is >> shape->r >> shape->g >> shape->b >> shape->a
                   >> shape->pen_size >> shape->frame;
                while ( is >> xy.x >> xy.y )
                {
                    shape->pts.push_back( xy );
                }
                if ( !sequences.empty() )
                {
                    sequences.back().shapes.push_back(
                        mrv::shape_type_ptr(shape) );
                }
                continue;
            }
            else if ( cmd == "GLErasePathShape" )
            {
                Point xy;
                std::string points;
                GLErasePathShape* shape = new GLErasePathShape;
                is.clear();
                std::getline( is, points );
                is.str( points );
                is.clear();
                is >> shape->pen_size >> shape->frame;
                while ( is >> xy.x >> xy.y )
                {
                    shape->pts.push_back( xy );
                }
                if ( !sequences.empty() )
                {
                    sequences.back().shapes.push_back(
                        mrv::shape_type_ptr(shape) );
                }
                continue;
            }
            else if ( cmd == "GLTextShape" )
            {
                Point xy;
                std::string font, text, t;
                unsigned font_size;
                std::getline( is, font, '"' ); // skip first quote
                std::getline( is, font, '"' );
                std::getline( is, text, '^' ); // skip first quote
                std::getline( is, text, '^' );
                while ( is.eof() )
                {
                    c = fgets( buf, 15999, f );
                    if (!c) break;
                    std::string st = c;
                    is.str( st );
                    is.clear();
                    std::string s;
                    std::getline( is, s, '^' );
                    text += "\n";
                    text += s;
                }

                GLTextShape* shape;
                shape = new GLTextShape;

                shape->text( text );

                unsigned i;
                unsigned num = Fl::set_fonts("-*");
                for ( i = 0; i < num; ++i )
                {
                    int t;
                    const char* name = Fl::get_font_name((Fl_Font)i,&t);
                    if ( font == name ) break;
                }
                if ( i >= num ) i = 0;


                shape->font( (Fl_Font) i );
                is >> font_size >> shape->r >> shape->g >> shape->b
                   >> shape->a
                   >> shape->frame;
                is >> xy.x >> xy.y;
                shape->size( font_size );
                shape->pts.clear();
                shape->pts.push_back( xy );
                if ( !sequences.empty() )
                {
                    sequences.back().shapes.push_back(
                        mrv::shape_type_ptr(shape) );
                }
                continue;
            }

            if ( version == 1.0 )
            {
                char* root = c;
                char* range = NULL;
                char* s = c + strlen(c) - 1;

                for ( ; s != c; --s )
                {

                    if ( *s == ' ' || *s == '\t' )
                    {
                        range = s + 1;
                        for ( ; (*s == ' ' || *s == '\t') && s != c; --s )
                            *s = 0;
                        break;
                    }
                    else if ( *s != '-' && (*s < '0' || *s > '9') ) break;
                }


                if ( range && range[0] != 0 )
                {
                    s = range;
                    for ( ; *s != 0; ++s )
                    {
                        if ( *s == '-' )
                        {
                            *s = 0;
                            first = last = atoi( range );
                            if ( *(s+1) != 0 )
                                last = atoi( s + 1 );
                            break;
                        }
                    }

                    if ( last <= first )
                    {
                        sequences.push_back( LoadInfo( root ) );
                    }
                    else
                    {
                        sequences.push_back( LoadInfo( root, first, last,
                                                       first, last ) );
                    }

                }
            }
            else
            {
                is.str( st );
                is.clear();
                std::string root;
                std::getline( is, root, '"' );
                std::getline( is, root, '"' );

                fs::path dir  = reelfile;
                dir = dir.parent_path();
                fs::path file = root;

                if ( root[0] != '/' && root[1] != ':' &&
                     root != _("Black Gap") &&
                     root.substr(0,9) != _("Checkered") &&
                     root != _("SMPTE NTSC Color Bars") &&
                     root != _("PAL Color Bars") &&
                     root != _("PAL HDTV Color Bars") &&
                     root != _("NTSC HDTV Color Bars") &&
                     root != _("PAL HDTV Color Bars") &&
                     root != _("Linear Gradient") &&
                     root != _("Luminance Gradient") &&
                     root != _("Gamma 1.4 Chart") &&
                     root != _("Gamma 1.8 Chart") &&
                     root != _("Gamma 2.2 Chart") &&
                     root != _("Gamma 2.4 Chart")
                    )
                {
                    dir /= file;
                    root = dir.generic_string();
                }

                double fps;
                boost::int64_t start = AV_NOPTS_VALUE, end = AV_NOPTS_VALUE;
                is >> first >> last >> start >> end;
                is >> fps;
                sequences.push_back( LoadInfo( root, first, last, start, end,
                                               fps ) );
            }

        }
    }

    fclose(f);

    setlocale( LC_NUMERIC, oldloc );
    av_free( oldloc );

    return true;
}


bool is_valid_sequence( const char* filename )
{
    if ( strlen(filename) == 0 ) return true;
    std::string root, frame, view, ext;
    bool ok = split_sequence( root, frame, view, ext, filename );
    if ( ext == "reel" || ext == "otio" ) return false;
    return ok;
}

bool is_directory( const char* dir )
{
    return fs::is_directory( dir );
}

int  padded_digits( const std::string& frame )
{
    if ( frame == "#" && frame.size() == 1 ) return 4;
    size_t pos;
    if ( ( pos = frame.find( '-' ) ) != std::string::npos )
        return frame.substr(0,pos).size();
    std::string c = frame.substr(0, 1);
    if ( c == "@" || c == "#" || c == "0" ) return (int)frame.size();
    if ( c == "%" ) return atoi(frame.substr(1, frame.size()-2).c_str());
    return 1;
}


bool fileroot( std::string& fileroot, const std::string& file,
               const bool change_view, const bool change_frame )
{
    std::string root, frame, view, ext;

    bool ok = split_sequence( root, frame, view, ext, file, change_view,
                              change_frame );
    if ( !ok || frame == "" || is_valid_movie( ext.c_str() ) ||
         mrv::is_valid_audio( ext.c_str() ) )
    {
        fileroot = file;
        return false;
    }

    if ( ! change_frame )
    {
        root += view;
        root += frame;
        root += ext;
        fileroot = root;
        return true;
    }


    const char* digits = PRId64;
    int pad = padded_digits(frame);
    if ( pad < 10 )
    {
        digits = "d";
    }


    char full[1024];
    if ( pad == 0 )
    {
        sprintf( full, "%s%s%%%s%s", root.c_str(), view.c_str(), digits,
                 ext.c_str() );
    }
    else
    {
        sprintf( full, "%s%s%%0%d%s%s", root.c_str(), view.c_str(), pad, digits,
                 ext.c_str() );
    }

    fileroot = full;
    return true;
}


std::string relative_path( const std::string& root, const std::string& parent )
{

    std::string path = root;

    if ( Preferences::uiMain->uiPrefs->uiPrefsRelativePaths->value() )
    {
        fs::path parentPath = parent;
        parentPath = parentPath.parent_path();
        fs::path childPath = root;
        fs::path relativePath = fs::relative( childPath, parentPath );
        path = relativePath.string();
        if ( path.empty() ) path = root;
    }

    std::replace( path.begin(), path.end(), '\\', '/' );

    return path;
}

} // namespace mrv
