/**
 * @file   guessImage.cpp
 * @author gga
 * @date   Sat Aug 25 00:55:56 2007
 * 
 * @brief  
 * 
 * 
 */

#include <iostream>


// Image types
#include "exrImage.h"
#include "stubImage.h"
#include "mapImage.h"
#include "shmapImage.h"
#include "pxrzImage.h"
#include "wandImage.h"
#include "aviImage.h"
#include "iffImage.h"
#include "mrayImage.h"
#include "ddsImage.h"
#include "hdrImage.h"

#include "Sequence.h"
#include "mrvIO.h"



namespace {
  const char* kModule = "guess";
}


namespace mrv {



  using namespace std;

  /*! Description of an Image file format */
  struct ImageTypes {
    // Function to test the filetype
    bool (*test)(const boost::uint8_t* datas, unsigned size);
    // MagickWand-type Function to test the filetype, not the bytes
    bool (*test_filename)(const char* filename);
    // Function to get/create an image of this type
    CMedia* (*get)(const char* name, const boost::uint8_t* datas);
  };


  ImageTypes image_filetypes[] =
    {
      { stubImage::test,  NULL,            stubImage::get },
      { exrImage::test,   NULL,            exrImage::get },
      { aviImage::test,   NULL,            aviImage::get },
      { iffImage::test,   NULL,            iffImage::get },
      { mapImage::test,   NULL,            mapImage::get },
      { hdrImage::test,   NULL,            hdrImage::get },
      { NULL,             wandImage::test, wandImage::get },
      { ddsImage::test,   NULL,            ddsImage::get },
      { shmapImage::test, NULL,            shmapImage::get },
      { mrayImage::test,  NULL,            mrayImage::get },
      { pxrzImage::test,  NULL,            pxrzImage::get },
      { NULL, NULL, NULL }, 
    };


  CMedia* test_image( const char* name, 
			 boost::uint8_t* datas, int size )
  {
    ImageTypes* type = image_filetypes;
    for ( ; type->get; ++type )
      {
	if ( type->test )
	  {
	    if ( type->test( datas, size ) ) 
	      return type->get( name, datas );
	  }
	else
	  {
	    if ( type->test_filename( name ) ) 
	      return type->get( name, datas );
	  }
      }
    return NULL;
  }


  CMedia* CMedia::guess_image( const char* file,
			       const boost::uint8_t* datas,
			       const int len,
			       const boost::int64_t start,
			       const boost::int64_t end )
  {
    int64_t lastFrame = end;
    int64_t frame = start;

    bool is_seq = false;
    std::string tmp;

    const char* root = file;
    if ( mrv::fileroot( tmp, std::string(file) ) )
      {
	is_seq = true;
	if ( start == std::numeric_limits<boost::int64_t>::min() )
	{
	   bool ok = mrv::get_sequence_limits( frame, lastFrame, tmp );
	   if ( ok ) root = tmp.c_str();
	}
      }

    char name[1024];
    if ( is_seq )
      {
	sprintf( name, root, frame );
      }
    else
      {
	strncpy( name, root, 1024 );
      }

    std::cerr << "name " << name << std::endl;

    boost::uint8_t* read_data = 0;
    size_t size = len;
    const boost::uint8_t* test_data = datas;
    if (!datas) {
      size = 1024;
      FILE* fp = fopen(name, "rb");
      if (!fp) 
	{
	  if ( is_seq )
	    {
	      std::string quoted( root );

	      string::size_type loc = 0;
	      while ( ( loc = quoted.find( '%', loc ) ) != string::npos )
		{
		  quoted = ( quoted.substr(0, loc) + "%" + 
			     quoted.substr( loc, quoted.size() ) );
		  loc += 2;
		}

	      mrvALERT("Image sequence \"" << quoted << "\" not found");
	    }
	  else
	    {
	      mrvALERT("Image \"" << name << "\" not found");
	    }
	  return NULL;
	}
      test_data = read_data = new boost::uint8_t[size + 1];
      read_data[size] = 0; // null-terminate so strstr() works
      size = fread(read_data, 1, size, fp);
      fclose(fp);
    }

    CMedia* image = test_image( name, (boost::uint8_t*)test_data, 
				(int)size );
    std::cerr << "Image " << image << std::endl;
    if ( image ) 
      {
	if ( is_seq )
	  image->sequence( tmp.c_str(), frame, lastFrame );
	else
	  image->filename( name );
      }

    delete [] read_data;

    if (image == NULL) 
      {
	LOG_ERROR(name << ": not a recognized format.");
	return NULL;
      }

    return dynamic_cast< CMedia* >( image );
  }



} // namespace mrv
