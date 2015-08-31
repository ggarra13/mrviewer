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
 * @file   guessImage.cpp
 * @author gga
 * @date   Sat Aug 25 00:55:56 2007
 * 
 * @brief  
 * 
 * 
 */

#include <iostream>

#include <fltk/run.h>


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
#include "mrvOS.h"


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
      { iffImage::test,   NULL,            iffImage::get },
      { mapImage::test,   NULL,            mapImage::get },
      { hdrImage::test,   NULL,            hdrImage::get },
      { aviImage::test,   NULL,            aviImage::get },
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
	    else
	       if ( type->test_filename && type->test_filename( name ) )
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
			       const boost::int64_t end,
                               const bool use_threads )
  {
    int64_t lastFrame = end;
    int64_t frame = start;

    std::string tmp;
    const char* root = file;
    bool is_seq = false;
    if ( start != std::numeric_limits<boost::int64_t>::max() ||
         end   != std::numeric_limits<boost::int64_t>::min() )
    {
        if ( mrv::fileroot( tmp, std::string(file) ) )
        {
            is_seq = true;
            root = tmp.c_str();
        }
    }

    if (( strcmp( root + strlen(root) - 4, ".xml" ) == 0 ) ||
        ( strcmp( root + strlen(root) - 4, ".XML" ) == 0 ) ||
        ( strcmp( root + strlen(root) - 1, "~" ) == 0 ) )
        return NULL;

    char name[1024];
    if ( is_seq )
      {
	 sprintf( name, root, frame );
      }
    else
      {
	 strncpy( name, root, 1024 );
      }


    boost::uint8_t* read_data = 0;
    size_t size = len;
    const boost::uint8_t* test_data = datas;
    if (!datas) {
      size = 1024;
      FILE* fp = fltk::fltk_fopen(name, "rb");
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

                LOG_ERROR( _("Image sequence \"") << quoted 
                           << _("\" not found.") );
	    }
	  else
	    {
                LOG_ERROR( _("Image \"") << name << _("\" not found.") );
	    }
	  return NULL;
	}
      test_data = read_data = new boost::uint8_t[size + 1];
      read_data[size] = 0; // null-terminate so strstr() works
      size = fread(read_data, 1, size, fp);
      fclose(fp);
    }

    CMedia* image = test_image( name, (boost::uint8_t*)test_data, 
				(unsigned int)size );
    if ( image ) 
      {
	if ( is_seq )
	{
           image->sequence( root, frame, lastFrame, use_threads );
	}
	else
	{
	  image->filename( name );
	}
      }

    delete [] read_data;

    if (image == NULL )
          return NULL;

    return image;
  }



} // namespace mrv
