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
 * @file   Sequence.cpp
 * @author gga
 * @date   Sat Jul 21 04:03:15 2007
 * 
 * @brief  
 * 
 * 
 */

#include <sys/types.h>
#include <sys/stat.h>

#if defined(WIN32) || defined(WIN64)
#include <direct.h>
#define getcwd _getcwd
#define S_ISDIR(x) _S_IFDIR(x)
#else
#include <unistd.h>
#endif


#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64 macro

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <limits>

#include <fltk/run.h>
#include <fltk/Font.h>

using namespace std;

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
namespace fs = boost::filesystem;


#define LOG_ERROR(x) std::cerr << x << std::endl

#include "gui/mrvImageView.h"
#include "video/mrvGLShape.h"
#include "core/Sequence.h"
#include "core/mrvString.h"
#include "mrvI8N.h"
#include "mrvOS.h"

typedef std::vector< std::string > StringList;

namespace mrv
{
  const boost::int64_t kMinFrame = std::numeric_limits< boost::int64_t >::min();
  const boost::int64_t kMaxFrame = std::numeric_limits< boost::int64_t >::max();

  bool is_valid_frame( const std::string framespec )
  {
    const char* c = framespec.c_str();

    for ( ++c; *c != 0; ++c )
      {
	 if ( *c == '+' || *c == '-' || (*c >= '0' && *c <= '9') ) continue;

	return false;
      }

    return true;
  }

  bool is_valid_frame_spec( const std::string framespec )
  {
     const char* c;
     if ( framespec.substr(0,1) == "." )
        c = framespec.c_str() + 1;
     else
        c = framespec.c_str();

    if ( *c == '%' || *c == '#' || *c == '@' ) return true;

    bool range_found = false;
    for ( ++c; *c != 0; ++c )
      {
	 if ( !range_found && *c == '-' ) { range_found = true; continue; }

	if ( *c != '+' && *c >= '0' && *c <= '9' ) continue;

	return false;
      }

    return range_found;
  }


void split(const std::string &s, char delim, StringList& elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

// Given a frame extension, return true if a possible movie file.
bool is_valid_movie( const char* ext )
{
   std::string tmp = ext;
   std::transform( tmp.begin(), tmp.end(), tmp.begin(),
                   (int(*)(int)) tolower);

   if ( tmp == ".avi" || tmp == ".mov"  || tmp == ".divx" ||
        tmp == ".wmv" || tmp == ".mpeg" || tmp == ".mpg"  ||
        tmp == ".qt"  || tmp == ".mp4"  || tmp == ".vob"  ||
        tmp == ".rm"  || tmp == ".flv" )
      return true;

   return false;
}


// Given a frame extension, return true if a possible picture file.
bool is_valid_picture( const char* ext )
{
   std::string tmp = ext;
   std::transform( tmp.begin(), tmp.end(), tmp.begin(),
                   (int(*)(int)) tolower);

   if ( tmp == ".iff"  || tmp == ".pic" || tmp == ".tif" ||
        tmp == ".tiff" || tmp == ".png" || tmp == ".jpg" ||
        tmp == ".jpeg" || tmp == ".tga" || tmp == ".exr" ||
        tmp == ".dpx"  || tmp == ".cin" || tmp == ".bmp" ||
        tmp == ".bit"  || tmp == ".sxr" || tmp == ".ct"  ||
        tmp == ".sgi"  || tmp == ".st"  || tmp == ".map" ||
        tmp == ".nt"   || tmp == ".mt"  || tmp == ".psd" ||
        tmp == ".rgb"  || tmp == ".rpf" || tmp == ".shmap" ||
        tmp == ".zt" )
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

      char buf[2]; buf[1] = 0;
      sprintf( buf, "%c", dec );

      r += buf;

      f = f.substr( loc+3, f.size() );
   }

   r += f;

   return r;
}

  /** 
   * Given a filename of a possible sequence, split it into
   * root name, frame string, view, and extension
   * 
   * @param root    root name of file sequence
   * @param frame   frame part of file name (must be @ or # or %d )
   * @param view    view of image (left or right or L and R )
   * @param ext     extension of file sequence
   * @param file    original filename, potentially part of a sequence.
   * 
   * @return true if a sequence, false if not.
   */
  bool split_sequence(
		      std::string& root,
		      std::string& frame,
                      std::string& view,
		      std::string& ext,
		      const std::string& file
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

    int64_t len = i - e + 1;
    f = f.substr( len, f.size() );

    StringList periods;
    split(f, '.', periods);

    if ( periods.size() == 4 )
    {
       root = file.substr( 0, len ) + periods[0] + ".";
       frame = periods[1];
       view = "." + periods[2];
       ext = "." + periods[3];

       if ( view == ".%v" || view == ".%V" || 
            view == ".l" || view == ".r" ||
            view == ".L" || view == ".R" ||
            view == ".left" || view == ".right" ||
            view == ".Left" || view == ".Right" )
       {
          return true;
       }
    }
    else
    {
       f = file;
    }

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
	frame = f.substr( idx[1]+1, idx[0]-idx[1]-1 );
	ext   = f.substr( idx[0], file.size()-idx[0] );


	bool ok = is_valid_frame( ext );
	if ( ok )
	{
	   frame = ext;
	   ext.clear();
	}

        if ( is_valid_movie( ext.c_str() ) )
        {
           root = "";
           return false;
        }

	ok = is_valid_frame_spec( frame );
	return ok;
      }
    else
      {
	root = f.substr( 0, idx[0]+1 );
	ext  = f.substr( idx[0]+1, file.size() );

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

	frame = "";
	return false;
      }
  }


  bool get_sequence_limits( boost::int64_t& frameStart, 
			    boost::int64_t& frameEnd,
			    std::string& fileroot )
  {

      frameStart = mrv::kMaxFrame; 
      frameEnd = mrv::kMinFrame;
  
    // My encoding type
    // Create and install global locale
    std::locale::global(boost::locale::generator().generate(""));

    fs::path::imbue( std::locale() );

    fs::path file = fs::path( fileroot.c_str() );
    fs::path dir = file.branch_path();

    char buf[1024];
    if ( dir.string() == "" ) {
       dir = fs::path( getcwd(buf,1024) );
    }

    if ( ( !fs::exists( dir ) ) || ( !fs::is_directory( dir ) ) )
    {
       LOG_ERROR( _("Directory '") << dir << 
		  _("' does not exist or no directory") );
       return false;
    }

    // Check if sequence is in ILM format first  ( image.1-30.exr )
    stringArray tokens;
    mrv::split_string( tokens, fileroot, "." );
    if ( tokens.size() > 2 )
      {
         int idx = 2;
         if ( tokens[2] == "l" || tokens[2] == "L" ||
              tokens[2] == "r" || tokens[2] == "R" ||
              tokens[2] == "left" || tokens[2] == "Left" ||
              tokens[2] == "right" || tokens[2] == "Right" )
            idx = 3;

	const std::string& range = tokens[ tokens.size()-idx ];

	if ( mrv::matches_chars(range.c_str(), "0123456789-") )
	  {
	    stringArray frames;
	    mrv::split_string( frames, range, "-" );
	    if ( frames.size() > 1 && frames[0].size() > 0)
	      {

		unsigned digits = (unsigned) frames[0].size();

		frameStart = atoi( frames[0].c_str() );
		frameEnd   = atoi( frames[1].c_str() );

		stringArray::iterator i = tokens.begin();
		stringArray::iterator e = tokens.end();
		fileroot = tokens[0]; ++i;
		for ( ; i != e; ++i )
		  {
		    fileroot += ".";
		    if ( *i == range )
		      {
			 char buf[64];
                         const char* pr = PRId64;
                         if ( digits < 4 ) pr = "d";
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
    frameStart = mrv::kMaxFrame;
    unsigned pad = 1;

    fs::directory_iterator e; // default constructor yields path iter. end
    for ( fs::directory_iterator i( dir ) ; i != e; ++i )
      {
	if ( !fs::exists( *i ) || fs::is_directory( *i ) ) continue;

	split_sequence( croot, cframe, cview, cext,
                        (*i).path().leaf().string() );
	if ( cext != ext || croot != root || cview != view ) 
           continue;  // not this sequence


	if ( cframe[0] == '0' && cframe.size() > 1 )
            pad = (unsigned) cframe.size();


	boost::int64_t f = atoi( cframe.c_str() );
	if ( f < frameStart ) frameStart = f;
	if ( f > frameEnd )   frameEnd   = f;
      }


    const char* prdigits = PRId64;
    if ( pad < 5 ) prdigits = "d";

    sprintf( buf, "%%0%d%s", pad, prdigits );

    split_sequence( root, frame, view, ext, fileroot );


    fileroot = root;
    fileroot += buf;
    fileroot += view;
    fileroot += ext;

    return true;
  }


bool parse_reel( mrv::LoadList& sequences, bool& edl,
		 const char* reelfile)
  {
     edl = false;

     setlocale( LC_NUMERIC, "C" );

     FILE* f = fltk::fltk_fopen( reelfile, "r" );
     if (!f ) return false;

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
                      sequences.back().audio = c+7;
                  continue;
              }

              boost::int64_t first = kMinFrame;
              boost::int64_t last  = kMaxFrame;


              std::string st = c;
              std::istringstream is( st );
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
                  sequences.back().shapes.push_back( 
                  mrv::shape_type_ptr(shape) );
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
                  sequences.back().shapes.push_back( 
                  mrv::shape_type_ptr(shape) );
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

                  fltk::Font** fonts;
                  unsigned i;
                  unsigned num = fltk::list_fonts(fonts);
                  for ( i = 0; i < num; ++i )
                  {
                      if ( font == fonts[i]->name() ) break;
                  }
                  if ( i >= num ) i = 0;
                  

                  shape->font( fonts[i] );
                  is >> font_size >> shape->r >> shape->g >> shape->b 
                     >> shape->a
                     >> shape->frame;
                  is >> xy.x >> xy.y;
                  shape->size( font_size );
                  shape->pts.clear();
                  shape->pts.push_back( xy );
                  sequences.back().shapes.push_back( 
                  mrv::shape_type_ptr(shape) );
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

                      sequences.push_back( LoadInfo( root, first, last,
                                                     first, last ) );
                  }
              }
              else
              {
                  is.str( st );
                  is.clear();
                  std::string root;
                  std::getline( is, root, '"' );
                  std::getline( is, root, '"' );

                  boost::int64_t start = AV_NOPTS_VALUE, end = AV_NOPTS_VALUE;
                  is >> first >> last >> start >> end;

                  sequences.push_back( LoadInfo( root, first, last, start, end ) );
              }

	  }
      }

    fclose(f);

    setlocale( LC_NUMERIC, "" );

    return true;
  }


  bool is_valid_sequence( const char* filename )
  {
     std::string root, frame, view, ext;
     return split_sequence( root, frame, view, ext, filename );
  }

  bool is_directory( const char* dir )
  {
     return fs::is_directory( dir );
  }

  int  padded_digits( const std::string& frame )
  {
    if ( frame == "#" ) return 4;
    std::string c = frame.substr(0, 1);
    if ( c == "@" || c == "0" ) return (int)frame.size();
    if ( c == "%" ) return atoi(frame.substr(1, frame.size()-2).c_str());
    return 1;
  }


  bool fileroot( std::string& fileroot, const std::string& file )
  {
     std::string root, frame, view, ext;
     fs::path path = fs::path( file );


     split_sequence( root, frame, view, ext, file );
     if ( root == "" || frame == "" ) 
     {
        fileroot = file;
        return false;
     }


    const char* digits = PRId64;
    int pad = padded_digits(frame);
    if ( pad < 5 )
    {
        digits = "d";
    }

    char full[1024];
    if ( pad == 0 )
    {
        sprintf( full, "%s%%%s%s%s", root.c_str(), digits, view.c_str(),
                 ext.c_str() );
    }
    else
    {
        sprintf( full, "%s%%0%d%s%s%s", root.c_str(), pad, digits, view.c_str(),
                ext.c_str() );
    }

    fileroot = full;
    return true;
  }

} // namespace mrv

