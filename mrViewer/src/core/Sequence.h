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

namespace mrv
{
  
  extern const boost::int64_t kMinFrame;
  extern const boost::int64_t kMaxFrame;

  /**
   * Struct used to store information about stuff to load
   * 
   */
  struct LoadInfo
  {
    std::string filename;
    std::string audio;
    boost::int64_t start;
    boost::int64_t end;
    bool    reel;

    LoadInfo( const std::string& fileroot, 
	      const boost::int64_t sf, const boost::int64_t ef ) :
      filename( fileroot ),
      start( sf ),
      end( ef ),
      reel( false )
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
      audio( b.audio ),
      start( b.start ),
      end( b.end ),
      reel( b.reel )
    {
    }

  };

  typedef std::vector< LoadInfo > LoadList;

  /** 
   * Given a file, return root filename in %d format, and frame passed if
   * sequence.  If not sequence, return file as is.
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
   * @param root    root name of file sequence
   * @param frame   frame part of file name
   * @param ext     extension of file sequence
   * @param file    original filename, potentially part of a sequence.
   * 
   * @return true if a sequence, false if not.
   */
  bool split_sequence(
		      std::string& root, std::string& frame,
		      std::string& ext, const std::string& file
		      );

  /** 
   * Parse a text .reel file and extract each sequence line from it
   * 
   * @param sequences list of sequences.  New sequences are appended in order.
   * @param edl       if sequence is an edl.
   * @param edlfile   filename of .reel file.
   */
void parse_reel( LoadList& sequences, bool& edl, const char* reelfile );

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
