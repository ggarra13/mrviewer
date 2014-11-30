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
 * @file   Sequence.h
 * @author 
 * @date   Thu Oct 12 20:09:31 2006
 * 
 * @brief  Routines used to parse image sequences
 * 
 * 
 */
#ifndef mrvSequence_h
#define mrvSequence_h

#include <vector>
#include <string>
#include <limits>

#include <boost/cstdint.hpp>      // for int64_t

#include "video/mrvGLShape.h"

namespace mrv
{
  
  extern const boost::int64_t kMinFrame;
  extern const boost::int64_t kMaxFrame;


struct Sequence
{
     std::string root;
     std::string number;
     std::string view;
     std::string ext;
};

typedef std::vector< Sequence > Sequences;

struct SequenceSort
{
     // true if a < b, else false
     bool operator()( const Sequence& a, const Sequence& b ) const
     {
        if ( a.root < b.root )  return true;
        else if ( a.root > b.root ) return false;

        if ( a.ext < b.ext ) return true;
        else if ( a.ext > b.ext ) return false;

        if ( a.view < b.view ) return true;
        else if ( a.view > b.view ) return false;

        size_t as = a.number.size();
        size_t bs = b.number.size();
        if ( as < bs ) return true;
        else if ( as > bs ) return false;

        if ( atoi( a.number.c_str() ) < atoi( b.number.c_str() ) )
           return true;
        return false;
     }
};


/**
   * Struct used to store information about stuff to load
   * 
   */
  struct LoadInfo
  {
      std::string filename;
      std::string right_filename;
      std::string audio;

      boost::int64_t first;
      boost::int64_t last;

      boost::int64_t start;
      boost::int64_t end;
      bool    reel;
      GLShapeList shapes;

      LoadInfo( const std::string& fileroot,
                const boost::int64_t sf, const boost::int64_t ef, 
                const boost::int64_t s = AV_NOPTS_VALUE,
                const boost::int64_t e = AV_NOPTS_VALUE,
                const std::string& a = "",
                const std::string& right = "" ) :
      filename( fileroot ),
      right_filename( right ),
      audio( a ),
      first( sf ),
      last( ef ),
      start( s ),
      end( e ),
      reel( false )
      {
      }

      LoadInfo( const std::string& fileroot,
                const boost::int64_t sf, const boost::int64_t ef, 
                const GLShapeList& shl,
                const boost::int64_t s = AV_NOPTS_VALUE,
                const boost::int64_t e = AV_NOPTS_VALUE,
                const std::string& a = "",
                const std::string& right = "" ) :
      filename( fileroot ),
      right_filename( right ),
      audio( a ),
      first( sf ),
      last( ef ),
      start( s ),
      end( e ),
      reel( false ),
      shapes( shl )
      {
      }

      LoadInfo( const std::string& reel ) :
      filename( reel ),
      start( kMinFrame ),
      end( kMaxFrame ),
      reel( true )
      {
      }

      LoadInfo( const LoadInfo& b ) :
      filename( b.filename ),
      right_filename( b.right_filename ),
      audio( b.audio ),
      start( b.start ),
      end( b.end ),
      first( b.first ),
      last( b.last ),
      reel( b.reel ),
      shapes( b.shapes )
      {
      }

  };

  typedef std::vector< LoadInfo > LoadList;

  /**
   * Given a filename with %hex characters, return string in ascii.
   */
  std::string hex_to_char_filename( std::string& f );

  /** 
   * Given a file, return root filename in %d format, and frame passed if
   * sequence.  If not sequence fileroot, return file as is.
   *
   * @param fileroot    fileroot in %*d format or file
   * @param file        original file
   * 
   * @return true if potential sequence, false if not
   */
  bool fileroot( std::string& fileroot, const std::string& file );

  /** 
   * Given a filename of a possible sequence, split it into
   * root name, frame string, and extension
   * 
   * @param root    root name of file sequence (root.)
   * @param frame   frame part of file name    (%d)
   * @param view    left or right for stereo images.  Empty if not stereo.
   * @param ext     extension of file sequence (.ext)
   * @param file    original filename, potentially part of a sequence.
   * 
   * @return true if a sequence, false if not.
   */
  bool split_sequence(
		      std::string& root, std::string& frame,
                      std::string& view,
		      std::string& ext, const std::string& file
		      );

  /** 
   * Parse a text .reel file and extract each sequence line from it.
   * Return true if all went well, false if file was not found or other error.
   * 
   * @param sequences list of sequences.  New sequences are appended in order.
   * @param edl       if sequence is an edl.
   * @param edlfile   filename of .reel file.
   */
bool parse_reel( LoadList& sequences, bool& edl,
                 const char* reelfile );

  /** 
   * Obtain the frame range of a sequence by scanning the directory where it
   * resides.
   * 
   * @param firstFrame   first frame of sequence 
   * @param lastFrame    last  frame of sequence
   * @param file         fileroot of sequence ( Example: mray.%04d.exr )
   * 
   * @return true if sequence limits found, false if not.
   */
  bool get_sequence_limits( boost::int64_t& firstFrame, 
			    boost::int64_t& lastFrame,
			    std::string& file );

  /** 
   * Given a filename extension, return whether the extension is
   * from a movie format.
   * 
   * @param ext Filename extension
   * 
   * @return true if a possible movie, false if not.
   */
   bool is_valid_movie( const char* ext );

  /** 
   * Given a filename extension, return whether the extension is
   * from a picture format.
   * 
   * @param ext Filename extension
   * 
   * @return true if a possible picture, false if not.
   */
   bool is_valid_picture( const char* ext );

  /** 
   * Given a single image filename, return whether the image is
   * a sequence on disk (ie. there are several images named with a
   * similar convention) 
   * 
   * @param file Filename of image
   * 
   * @return true if a possible sequence, false if not.
   */
  bool is_valid_sequence( const char* file );

  /** 
   * Given a single filename, return whether the file is
   * a directory on disk
   * 
   * @param file Filename or Directory
   * 
   * @return true if a directory, false if not.
   */
  bool is_directory( const char* file );


  /** 
   * Given a frame string like "0020", return the number of
   * padded digits
   * 
   * @param frame a string like "0020" or "14".
   * 
   * @return number of padded digits (4 for 0020, 1 for 14 ).
   */
  int  padded_digits( const std::string& frame );

}  // namespace mrv



#endif // mrvSequence_h
