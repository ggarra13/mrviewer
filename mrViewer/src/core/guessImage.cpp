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
#include "mrvColorBarsImage.h"
#include "iffImage.h"
#include "mrayImage.h"
#include "ddsImage.h"
#include "hdrImage.h"
#include "oiioImage.h"
#include "picImage.h"
#include "rawImage.h"
#include "smpteImage.h"

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
      //{ hdrImage::test,   NULL,            hdrImage::get }, // broken
      { picImage::test,   NULL,            picImage::get },
      { aviImage::test,   NULL,            aviImage::get },
      { NULL,             rawImage::test,  rawImage::get },
      { NULL,             oiioImage::test, oiioImage::get },
      { NULL,             wandImage::test, wandImage::get },
      { ddsImage::test,   NULL,            ddsImage::get },
      { shmapImage::test, NULL,            shmapImage::get },
      { mrayImage::test,  NULL,            mrayImage::get },
      { pxrzImage::test,  NULL,            pxrzImage::get },
      { NULL, NULL, NULL },
    };


  CMedia* test_image( const char* name,
                      boost::uint8_t* datas, int size,
                      const bool is_seq )
  {
    ImageTypes* type = image_filetypes;
    for ( ; type->get; ++type )
      {
          // if ( is_seq && type->test == aviImage::test )
          //     continue;
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
              {
                  return type->get( name, datas );
              }
          }
      }
    return NULL;
  }

std::string parse_view( const std::string& root, bool left )
{
    int idx = root.find( "%V" );
    std::string tmp = root;
    if ( idx != std::string::npos )
    {
        tmp = root.substr( 0, idx );
        tmp += get_long_view( left );
        tmp += root.substr( idx+2, root.size() );
    }
    else
    {
        idx = root.find( "%v" );
        if ( idx != std::string::npos )
        {
            tmp = root.substr( 0, idx );
            tmp += get_short_view( left );
            tmp += root.substr( idx+2, root.size() );
        }
    }
    return tmp;
}


void verify_stereo_resolution( const CMedia* const left,
                               const CMedia* const right )
{
    const mrv::Recti& dpw1 = left->display_window();
    const mrv::Recti& dpw2 = right->display_window();
    if ( dpw1 != dpw2 )
    {
        LOG_WARNING( "\"" << left->name() << "\""
                     << _( " has different display window than " )
                     << "\"" << right->name() << "\"" );
        LOG_WARNING( dpw1
                     << _(" vs. ")
                     << dpw2 );
        LOG_WARNING( _("3D Stereo will most likely not work properly.") );
    }

    if ( right->fps() != left->fps() )
    {
        LOG_WARNING( _("Images in stereo have different velocities (fps)." ) );

        double d1 = ( left->last_frame() - left->first_frame() + 1) /
                    left->fps();

        double d2 = ( right->last_frame() - right->first_frame() + 1) /
                    right->fps();

        if ( d1 != d2 )
        {
            LOG_WARNING( _("Images in stereo have different lengths.  "
                           "They will not loop properly." ) );
        }
    }
    else
    {
        if ( right->last_frame() - right->first_frame() + 1 !=
             left->last_frame() - left->first_frame() + 1 )
        {
            LOG_WARNING( _("Images in stereo have different frame lengths.  "
                           "They will not loop properly." ) );
        }
    }
}

CMedia* guess( bool is_stereo, bool is_seq, bool left,
               const std::string& root, const int64_t frame,
               const boost::uint8_t* datas, const int len,
               const int64_t& lastFrame,
               const bool is_thumbnail = false )
{
    std::string tmp;
    char buf[1024]; buf[0] = 0;
    char *name = buf;
    if ( is_stereo )
    {
        tmp = parse_view( root, left );

        if ( is_seq )
        {
            sprintf( name, tmp.c_str(), frame );
        }
        else
        {
            strncpy( name, tmp.c_str(), 1024 );
        }
    }
    else
    {
        if ( is_seq )
        {
            sprintf( name, root.c_str(), frame );  // eliminate file:
        }
        else
        {
            strncpy( name, root.c_str(), 1024 );  // eliminate file:
        }
    }

    if ( strncmp( name, "file:", 5 ) == 0 )
    {
        name += 5;
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
                LOG_ERROR( _("Image sequence \"") << root
                           << _("\" not found. ") );
            }
            else
            {
                CMedia* img = NULL;
                if ( strcmp( name, _("SMPTE NTSC Color Bars") ) == 0 )
                {
                    img = new ColorBarsImage( ColorBarsImage::kSMPTE_NTSC );
                }
                else if ( strcmp( name, _("PAL Color Bars") ) == 0 )
                {
                    img = new ColorBarsImage( ColorBarsImage::kPAL );
                }
                else if ( strcmp( name, _("NTSC HDTV Color Bars") ) == 0 )
                {
                    img = new ColorBarsImage( ColorBarsImage::kSMPTE_NTSC_HDTV );
                }
                else if ( strcmp( name, _("PAL HDTV Color Bars") ) == 0 )
                {
                    img = new ColorBarsImage( ColorBarsImage::kPAL_HDTV );
                }
                else if ( strcmp( name, _("Checkered") ) == 0 )
                {
                    img = new smpteImage( smpteImage::kCheckered,
                                          1280, 960 );
                }
                else if ( strcmp( name, _("Linear Gradient") ) == 0 )
                {
                    img = new smpteImage( smpteImage::kLinearGradient,
                                          1280, 960 );
                }
                else if ( strcmp( name, _("Luminance Gradient") ) == 0 )
                {
                    img = new smpteImage( smpteImage::kLuminanceGradient,
                                          1280, 960 );
                }
                else if ( strcmp( name, _("Gamma 1.4 Chart") ) == 0 )
                {
                    img = new smpteImage( smpteImage::kGammaChart,
                                          1280, 960 );
                    img->gamma( 1.4 );
                }
                else if ( strcmp( name, _("Gamma 1.8 Chart") ) == 0 )
                {
                    img = new smpteImage( smpteImage::kGammaChart,
                                          1280, 960 );
                    img->gamma( 1.8 );
                }
                else if ( strcmp( name, _("Gamma 2.2 Chart") ) == 0 )
                {
                    img = new smpteImage( smpteImage::kGammaChart,
                                          1280, 960 );
                    img->gamma( 2.2 );
                }
                else if ( strcmp( name, _("Gamma 2.4 Chart") ) == 0 )
                {
                    img = new smpteImage( smpteImage::kGammaChart,
                                          1280, 960 );
                    img->gamma( 2.4 );
                }
                // @todo: slate image cannot be created since it needs info
                //        from other image.
                if ( img )
                {
                    img->filename( name );
                    img->fetch(1);
                    img->default_icc_profile();
                    img->default_rendering_transform();
                    return img;
                }
                else
                {
                    LOG_ERROR( _("Image \"") << name << _("\" not found.") );
                }
            }
            return NULL;
        }
        test_data = read_data = new boost::uint8_t[size + 1];
        read_data[size] = 0; // null-terminate so strstr() works
        size = fread(read_data, 1, size, fp);
        fclose(fp);
    }


    CMedia* image = test_image( name, (boost::uint8_t*)test_data,
                                (unsigned int)size, is_seq );
    if ( image )
    {
        image->is_thumbnail( is_thumbnail );
        image->is_left_eye( left );

        if ( is_seq )
        {
            image->sequence( root.c_str(), frame, lastFrame, false );
        }
        else
        {
            image->filename( name );
        }
    }

    delete [] test_data;

    return image;
}

  CMedia* CMedia::guess_image( const char* file,
                               const boost::uint8_t* datas,
                               const int len,
                               const bool is_thumbnail,
                               const int64_t start,
                               const int64_t end,
                               const bool avoid_seq )
  {
    int64_t lastFrame = end;
    int64_t frame = start;

    std::string tmp;
    std::string root = file;


    bool is_stereo = false;
    bool is_seq = false;

    if ( root.find( "%V" ) != std::string::npos ||
         root.find( "%v" ) != std::string::npos )
    {
        is_stereo = true;
    }

    if ( start != AV_NOPTS_VALUE ||
         end   != AV_NOPTS_VALUE )
    {
        if ( mrv::fileroot( tmp, root ) )
        {
            is_seq = true;
            root = tmp;
        }
    }

    if ( (root.size() > 4 &&
          ( root.substr( root.size() - 4, root.size()) == ".xml" ||
            root.substr( root.size() - 4, root.size()) == ".XML" ) ) ||
         ( root.size() > 1 &&
           ( root.substr( root.size() - 1, root.size()) == "~" )) )
        return NULL;

    CMedia* right = NULL;
    CMedia* image = NULL;


    try {
        image = guess( is_stereo, is_seq, true, root, frame, datas, len,
                       lastFrame, is_thumbnail );
        if ( is_stereo && image )
        {
            right = guess( is_stereo, is_seq, false, root, frame,
                           NULL, 0, lastFrame, is_thumbnail );
            if ( right )
            {
                verify_stereo_resolution( image, right );

                image->right_eye( right );
                image->is_stereo( true );
                image->is_left_eye( true );
                right->is_stereo( true );
                right->is_left_eye( false );
                aviImage* aviL = dynamic_cast< aviImage* >( image );
                aviImage* aviR = dynamic_cast< aviImage* >( right );
                if ( aviL && aviR )
                {
                    aviR->subtitle_file( aviL->subtitle_file().c_str() );
                }
            }
        }
    }
    catch( const std::exception& e )
    {
        LOG_ERROR( e.what() );
    }

    DBG("Loaded " << image->name() );

    return image;
  }



} // namespace mrv
