/**
 * @file   exrImage.cpp
 * @author gga
 * @date   Fri Jul 20 04:54:04 2007
 * 
 * @brief  Class used to load and save ILM's OpenEXR images
 * 
 * 
 */


#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>       // for quietNaN

#include <Iex.h>
#include <ImfVersion.h> // for MAGIC
#include <ImfChannelList.h>
#include <ImfPartType.h>
#include <ImfInputFile.h>
#include <ImfInputPart.h>
#include <ImfTiledInputFile.h>
#include <ImfMultiPartInputFile.h>
#include <ImfStandardAttributes.h>
#include <ImfOutputFile.h>
#include <ImathMath.h> // for Math:: functions
#include <ImathBox.h>  // for Box2i
#include <ImfIntAttribute.h>
#include <ImfRgbaYca.h>

#include "core/mrvThread.h"
#include "core/exrImage.h"
#include "core/mrvImageOpts.h"
#include "gui/mrvIO.h"

using namespace Imf;
using namespace Imath;
using namespace std;


namespace 
{
  const char* kModule = "exr";
}


#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )



namespace mrv {

  const char* exrImage::kCompression[] = {
    _("None"),
    _("RLE"),
    N_("Zips"),
    N_("Zip"),
    N_("Piz"),
    N_("Pxr24"),
    N_("B44"),
    N_("B44A"),
    N_("DWAA"),
    N_("DWAB"),
    0
  };

  const char* exrImage::kLineOrder[] = {
    _("Increasing Y"),
    _("Decreasing Y"),
    _("Random Y"),
    0
  };

  image_type::PixelType 
  exrImage::pixel_type_conversion( Imf::PixelType pixel_type )
  {
    switch( pixel_type )
      {
      case Imf::UINT:
	return image_type::kInt;
      case Imf::HALF:
	return image_type::kHalf;
      case Imf::FLOAT:
	return image_type::kFloat;
      default:
	LOG_ERROR("Unknown Imf::PixelType");
	return image_type::kFloat;
      }
  }

  Imf::PixelType 
  exrImage::pixel_type_to_exr( image_type::PixelType pixel_type )
  {
    switch( pixel_type )
    {
        case image_type::kByte:
        case image_type::kShort:
            return Imf::NUM_PIXELTYPES;
        case image_type::kInt:
            return Imf::UINT;
        case image_type::kHalf:
            return Imf::HALF;
	case image_type::kFloat:
            return Imf::FLOAT;
        default:
            LOG_ERROR("Unknown image_type::PixelType " << pixel_type);
            return Imf::HALF;
    }
  }

  exrImage::exrImage() :
  CMedia(),
  _levelX( 0 ),
  _levelY( 0 ),
  _multiview( false ),
  _has_yca( false ),
  _use_yca( false ),
  _has_left_eye( false ),
  _has_right_eye( false ),
  _left_red( false ),
  _curpart( -1 ),
  _numparts( -1 ),
  _num_layers( 0 ),
  _lineOrder( (Imf::LineOrder) 0 ),
  _compression( (Imf::Compression) 0 )
  {
  }

  exrImage::~exrImage()
  {
  }

  stringArray exrImage::valid_compressions() const
  {
    stringArray ret;
    const char** s = kCompression;
    for ( ; *s != 0; ++s )
      {
	ret.push_back( *s );
      }
    return ret;
  }

  /*! Test a block of data read from the start of the file to see if it
    looks like the start of an .exr file. This returns true if the 
    data contains EXR's magic number and a "channels" string in the 8th
    position.
  */
  bool exrImage::test(const boost::uint8_t *data, unsigned)
  {
    int magic = *((int*)data);

    if ( magic != MAGIC ) return false;

    return true; // (strstr((char*) data + 8, "channels") != 0);
  }


bool exrImage::channels_order(
			      const boost::int64_t frame,
			      Imf::ChannelList::ConstIterator s,
			      Imf::ChannelList::ConstIterator e,
			      Imf::ChannelList& channels,
			      const Imf::Header& h, 
			      Imf::FrameBuffer& fb
			      )
{
   const Box2i& dataWindow = h.dataWindow();
   const Box2i& displayWindow = h.displayWindow();

   // int wi = displayWindow.max.x - displayWindow.min.x + 1;
   // int he = displayWindow.max.y - displayWindow.min.y + 1;
   int dw = dataWindow.max.x - dataWindow.min.x + 1;
   int dh = dataWindow.max.y - dataWindow.min.y + 1;
   if ( dw <= 0 || dh <= 0 )  return false;
   int dx = dataWindow.min.x;
   int dy = dataWindow.min.y;

   int order[4];
   order[0] = order[1] = order[2] = order[3] = -1;

   // First, count and store the channels
   int idx = 0;
   Imf::PixelType imfPixelType = Imf::UINT;
   std::vector< std::string > channelList;
   Imf::ChannelList::ConstIterator i;
   for ( i = s; i != e; ++i, ++idx )
   {
      const std::string& layerName = i.name();

      const Imf::Channel* ch = channels.findChannel( 
						    layerName.c_str()
						     );
      if ( !ch ) {
         LOG_ERROR( "Channel " << layerName << " not found" );
         continue;
      }


      if ( ch->type > imfPixelType ) imfPixelType = ch->type;

      std::string ext = layerName;
      size_t pos = layerName.rfind( N_(".") );
      if ( pos != string::npos )
      {
	 ext = ext.substr( pos+1, ext.size() );
      }

      std::transform( ext.begin(), ext.end(), ext.begin(),
		      (int(*)(int)) toupper);
      if ( order[0] == -1 && (ext == N_("R") || ext == N_("RED") ||
			      ext == N_("Y") || ext == N_("U") ) )
         order[0] = idx;
      if ( order[1] == -1 && (ext == N_("G") || ext == N_("GREEN") ||
			      ext == N_("RY") || ext == N_("V") ) )
         order[1] = idx;
      if ( order[2] == -1 && (ext == N_("B") || ext == N_("BLUE")||
			      ext == N_("BY") || ext == N_("W") ) )
         order[2] = idx;
      if ( order[3] == -1 && (ext == N_("A") ||
			      ext == N_("ALPHA") ) ) order[3] = idx;

      channelList.push_back( layerName );
   }


   size_t numChannels = channelList.size();

   if ( numChannels == 0 && channel() )
   {
      LOG_ERROR( _("Image file \"") << filename() << 
                 _("\" has no channels named with prefix \"") 
                 << channel() << "\"." );
      return false;
   }
   else if ( numChannels > 4 && channel() )
   {
      numChannels = 4;
   }

   if ( numChannels == 1 ) order[0] = 0;
   
   // Prepare format
   image_type::Format format = VideoFrame::kLumma;
   int offsets[4];
   offsets[0] = 0;
   if ( _has_yca )
   {
      unsigned size  = dw * dh;
      unsigned size2 = dw * dh / 4;
      offsets[1]  = size;
      offsets[2]  = size + size2;
      offsets[3]  = 0;
      if ( numChannels >= 3 && has_alpha() )
      {
	 format = VideoFrame::kYByRy420A;
	 numChannels = 4;
	 offsets[3]  = size + size2 * 2;
      }
      else if ( numChannels >= 1 )
      {
	 numChannels = 3;
	 format = VideoFrame::kYByRy420;
      }
   }
   else
   {
      offsets[1]  = 1;
      offsets[2]  = 2;
      offsets[3]  = 3;

      if ( numChannels >= 3 && has_alpha() )
      {
	 format = VideoFrame::kRGBA;
	 numChannels = 4;
      }
      else if ( numChannels >= 2 )
      {
	 format = VideoFrame::kRGB;
	 numChannels = 3;
      }
   }
   
   allocate_pixels( frame, unsigned(numChannels), format,
		    pixel_type_conversion( imfPixelType ) );
   
   size_t xs[4], ys[4];
   if ( _has_yca )
   {
      for ( unsigned j = 0; j < numChannels; ++j )
	 xs[j] = _hires->pixel_size();

      unsigned int dw2 = dw / 2;
      ys[0] = xs[0] * dw;
      ys[1] = xs[0] * dw2;
      ys[2] = xs[0] * dw2;
      ys[3] = xs[0] * dw;
   }
   else
   {
      for ( unsigned j = 0; j < numChannels; ++j )
      {
	 xs[j] = _hires->pixel_size() * numChannels;
	 ys[j] = xs[j] * dw;
      }
   }


   char* pixels = (char*)_hires->data().get();
   memset( pixels, 0, _hires->data_size() ); // Needed

   // Then, prepare frame buffer for them
   int start = ( (-dx - dy * dw) * _hires->pixel_size() *
		 _hires->channels() );

   char* base = pixels + start;

   Imf::Channel* ch = NULL;
   idx = 0;
   for ( i = s; i != e && idx < 4; ++i, ++idx )
   {
      int k = order[idx];
      if ( k == -1 ) continue;

      const std::string& layerName = channelList[k];

      ch = channels.findChannel( layerName.c_str() );

      if ( !ch ) {
         LOG_ERROR( "Channel " << layerName << " not found" );
         continue;
      }

      char* buf = (char*)base + offsets[idx] * _hires->pixel_size();

      fb.insert( layerName.c_str(), 
		 Slice( imfPixelType, buf, xs[idx], ys[idx],
			ch->xSampling, ch->ySampling) );
   }

   return true;
}

bool exrImage::channels_order_multi(
				    const boost::int64_t frame,
				    Imf::ChannelList::ConstIterator s,
				    Imf::ChannelList::ConstIterator e,
				    Imf::ChannelList& channels,
				    const Imf::Header& h, 
				    Imf::FrameBuffer& fb
				    )
{
   const Box2i& dataWindow = h.dataWindow();
   const Box2i& displayWindow = h.displayWindow();
   int wi = displayWindow.max.x - displayWindow.min.x + 1;
   int he = displayWindow.max.y - displayWindow.min.y + 1;
   int dw = dataWindow.max.x - dataWindow.min.x + 1;
   int dh = dataWindow.max.y - dataWindow.min.y + 1;
   if ( dw <= 0 || dh <= 0 )  return false;
   int dx = dataWindow.min.x;
   int dy = dataWindow.min.y;

   int order[4];
   order[0] = order[1] = order[2] = order[3] = -1;

   Imf::ChannelList::ConstIterator sr, er, sl, el;

   unsigned idx = 0;
   Imf::PixelType imfPixelType = Imf::UINT;
   std::vector< std::string > channelList;
   Imf::ChannelList::ConstIterator i;




   // First, gather all the channels
   for ( i = s; i != e; ++i )
   {
      const std::string layerName = i.name();
      const Imf::Channel* ch = channels.findChannel( layerName.c_str() );
      if ( !ch ) {
         LOG_ERROR( "Channel " << layerName << " not found" );
         continue;
      }

      channelList.push_back( layerName );
   }

   // Then gather the channel we will use in left eye
   for ( i = s; i != e; ++i, ++idx )
   {
      const std::string layerName = i.name();
      const Imf::Channel* ch = channels.findChannel( 
						    layerName.c_str()
						     );
      if ( !ch ) {
         LOG_ERROR( "Channel " << layerName << " not found" );
         continue;
      }

      if ( ch->type > imfPixelType ) imfPixelType = ch->type;
      
      std::string ext = layerName;
      std::string root = "";
      size_t pos = ext.rfind( N_(".") );
      if ( pos != string::npos )
      {
	 root = ext.substr( 0, pos );
	 ext = ext.substr( pos+1, ext.size() );
      }
      
      std::transform( root.begin(), root.end(), root.begin(),
		      (int(*)(int)) toupper);
      std::transform( ext.begin(), ext.end(), ext.begin(),
		      (int(*)(int)) toupper);
      
      if ( (root == N_("LEFT")) || ( root == "" && !_has_left_eye ) )
      { 
	 if ( _left_red )
	 {
	    if ( order[0] == -1 && (ext == N_("R") || ext == N_("RED") || 
				    ext == N_("Y") ) ) 
	    {
	       order[0] = idx;
	       continue;
	    }
	 }
	 else
	 {
	    if ( order[1] == -1 && (ext == N_("G") || ext == N_("GREEN") || 
				    ext == N_("RY") || ext == N_("Y") ) )
	    {
	       order[1] = idx;
	       if ( ext == N_("Y") ) _use_yca = false; 
	    }
	    if ( order[2] == -1 && (ext == N_("B") || ext == N_("BLUE")|| 
				    ext == N_("BY") ) ) order[2] = idx;
	    if ( order[3] == -1 && (ext == N_("A") || 
				    ext == N_("ALPHA") ) ) order[3] = idx;
	 }
      }
   }

   // We have the left eye in order[0] or order[1]
   assert( order[0] != -1 || order[1] != -1 );

   idx = 0;
   for ( i = s; i != e; ++i, ++idx )
   {
      const std::string& layerName = i.name();
      const Imf::Channel* ch = channels.findChannel( layerName.c_str() );
      if ( !ch ) {
         LOG_ERROR( "Missing channel " << layerName );
         continue;
      }

      std::string ext = layerName;
      std::string root = "";
      size_t pos = layerName.rfind( N_(".") );
      if ( pos != string::npos )
      {
	 root = ext.substr( 0, pos );
	 ext = ext.substr( pos+1, ext.size() );
      }

      std::transform( root.begin(), root.end(), root.begin(),
		      (int(*)(int)) toupper);
      std::transform( ext.begin(), ext.end(), ext.begin(),
		      (int(*)(int)) toupper);


      if ( (root == "RIGHT") || ( root == "" && !_has_right_eye ) )
      {
	 if ( !_left_red )
	 {
	    if ( order[0] == -1 && (ext == N_("R") || ext == N_("RED") || 
				    ext == N_("Y") ) ) 
	    {
	       order[0] = idx;
	       break;
	    }
	 }
	 else
	 {
	    if ( order[1] == -1 && (ext == N_("G") || ext == N_("GREEN") || 
				    ext == N_("RY") || ext == N_("Y")) )
	    {
	       order[1] = idx;
	       if ( ext == N_("Y") ) _use_yca = false;
	    }

	    if ( order[2] == -1 && (ext == N_("B") || ext == N_("BLUE")|| 
				    ext == N_("BY") ) ) order[2] = idx;
	    if ( order[3] == -1 && (ext == N_("A") || 
				    ext == N_("ALPHA") ) ) order[3] = idx;
	 }
      }

   }

   // order[0-1] should be set by now.

   size_t numChannels = channelList.size();

   if ( numChannels > 4 )
   {
      numChannels = 4;
   }


   if ( numChannels == 1 ) {
      order[0] = 0;
   }


   // Prepare format
   image_type::Format format = VideoFrame::kLumma;
   int offsets[4];
   offsets[0] = 0;
   offsets[1] = 1;
   offsets[2] = 2;
   offsets[3] = 3;

   if ( numChannels >= 3 && has_alpha() )
   {
      format = VideoFrame::kRGBA;
      numChannels = 4;
   }
   else if ( numChannels >= 2 )
   {
      if ( numChannels == 2 && _use_yca == false )
      {
	 order[2] = order[1];
      }

      format = VideoFrame::kRGB;
      numChannels = 3;
   }


   allocate_pixels( frame, unsigned(numChannels), format,
		    pixel_type_conversion( imfPixelType ) );

   size_t xs[4], ys[4];
   for ( unsigned j = 0; j < numChannels; ++j )
   {
      xs[j] = _hires->pixel_size() * numChannels;
      ys[j] = xs[j] * dw;
   }


   boost::uint8_t* pixels = (boost::uint8_t*)_hires->data().get();
   memset( pixels, 0, _hires->data_size() ); // Needed

   // Then, prepare frame buffer for them
   int start = ( (-dx - dy * dw) * _hires->pixel_size() *
		 _hires->channels() );


   boost::uint8_t* base = pixels + start;


   Imf::Channel* ch = NULL;
   for ( idx = 0; idx < numChannels; ++idx )
   {
      int k = order[idx];
      if ( k == -1 ) continue;

      const std::string& layerName = channelList[k];

      ch = channels.findChannel( layerName.c_str() );

      if ( !ch ) {
         LOG_ERROR( "Channel " << layerName << " not found" );
         continue;
      }

      char* buf = (char*)base + offsets[idx] * _hires->pixel_size();

      fb.insert( layerName.c_str(),
		 Slice( imfPixelType, buf, xs[idx], ys[idx],
			ch->xSampling, ch->ySampling) );
   }


   return true;
}

void exrImage::ycc2rgba( const Imf::Header& hdr, const boost::int64_t frame )
{
   SCOPED_LOCK( _mutex );

   Imf::Chromaticities cr;
   if ( Imf::hasChromaticities( hdr ) )
      cr = Imf::chromaticities( hdr );
   
   V3f yw = Imf::RgbaYca::computeYw(cr);
   
   unsigned w = width();
   unsigned h = height();
   
   image_type::Format format = ( has_alpha() ? 
				 image_type::kRGBA : 
				 image_type::kRGB );
   
   mrv::image_type_ptr rgba( new image_type( frame, w, h, 
					     3 + 1*has_alpha(),
					     format, 
					     image_type::kFloat ) );
   
   
   for ( unsigned y = 0; y < h; ++y )
   {
      for ( unsigned x = 0; x < w; ++x )
      {
	 CMedia::Pixel p = _hires->pixel( x, y );
	 rgba->pixel( x, y, p );
      }
   }

   _hires = rgba;
}

  /** 
   * Fetch the current EXR image
   * 
   * @return true if success, false if not
   */
bool exrImage::fetch_mipmap( const boost::int64_t frame ) 
  {

     try {

	SCOPED_LOCK( _mutex );
	
	std::string fileName = sequence_filename(frame);

	TiledInputFile in( fileName.c_str() );

        int numXLevels = in.numXLevels();
        int numYLevels = in.numYLevels();
        if (_levelX > numXLevels-1 ) _levelX = numXLevels-1;
        if (_levelY > numYLevels-1 ) _levelY = numYLevels-1;

	if (!in.isValidLevel (_levelX, _levelY))
	{
	   THROW (Iex::InputExc, "Level (" << _levelX << ", " 
		  << _levelY << ") does "
		  "not exist in file " << fileName << ".");
	}

	Imf::Header h = in.header();
	h.dataWindow() = in.dataWindowForLevel(_levelX, _levelY);
	h.displayWindow() = h.dataWindow();

        const Imath::Box2i& dataWindow = h.dataWindow();
        const Imath::Box2i& displayWindow = h.displayWindow();
        data_window( dataWindow.min.x, dataWindow.min.y,
                     dataWindow.max.x, dataWindow.max.y );

        display_window( displayWindow.min.x, displayWindow.min.y,
                        displayWindow.max.x, displayWindow.max.y );


	if ( _exif.empty() && _iptc.empty() )
           read_header_attr( h, frame );

	FrameBuffer fb;
	bool ok = find_channels( h, fb, frame );
	if ( !ok ) return false;


	_pixel_ratio = h.pixelAspectRatio();
	_lineOrder   = h.lineOrder();
        _compression = h.compression();

	in.setFrameBuffer(fb);


	int tx = in.numXTiles( _levelX );
	int ty = in.numYTiles( _levelY );
	
	//
	// For maximum speed, try to read the tiles in
	// the same order as they are stored in the file.
	//

	if ( _lineOrder == INCREASING_Y )
	{
	   for (int y = 0; y < ty; ++y)
	      for (int x = 0; x < tx; ++x)
		 in.readTile (x, y, _levelX, _levelY);
	}
	else
	{
	   for (int y = ty - 1; y >= 0; --y)
	      for (int x = 0; x < tx; ++x)
		 in.readTile (x, y, _levelX, _levelY);
	}

	return true;
     
     }
     catch( const std::exception& e )
     {
	LOG_ERROR( e.what() );
	return false;
     }
  }

bool exrImage::find_layers( const Imf::Header& h )
{

   const Box2i& dataWindow = h.dataWindow();
   const Box2i& displayWindow = h.displayWindow();
   int dw  = dataWindow.max.x - dataWindow.min.x + 1;
   int dh  = dataWindow.max.y - dataWindow.min.y + 1;
   if ( dw <= 0 || dh <= 0 )  return false;


   image_size( dw, dh );


   Imf::ChannelList channels = h.channels();
   stringSet layers;
   channels.layers( layers );

   if ( layers.find( N_( "right" ) ) != layers.end() )
   {
      _has_right_eye = true;
   }

   if ( layers.find( N_("left") ) != layers.end() )
      _has_left_eye = true;

   if ( !_is_stereo && ( _has_left_eye || _has_right_eye ) )
   {
      _multiview = true;
      _is_stereo = true;
      _stereo_type = kNoStereo;
   }


   if ( _layers.empty() || _layers.size() == _num_layers )
   {
      _num_channels = 0;
      _gamma = 2.2f;

      bool has_rgb = false, has_alpha = false;
      if ( channels.findChannel( N_("R") ) ||
	   channels.findChannel( N_("G") ) ||
	   channels.findChannel( N_("B") ) )
      {
	 has_rgb = true;
	 rgb_layers();
	 lumma_layers();
      }
      
      _has_yca = false;
      _use_yca = false;
      if ( !has_rgb )
      {
	 if ( channels.findChannel( N_("Y") ) )
	 {
	    _layers.push_back( N_("Y") ); ++_num_channels;
	 }
	 if ( channels.findChannel( N_("RY") ) )
	 {
	    _layers.push_back( N_("RY") ); ++_num_channels;
	 }
	 if ( channels.findChannel( N_("BY") ) )
	 {
	    _layers.push_back( N_("BY") ); ++_num_channels;
	 }
	 
	 if ( ! _layers.empty() ) 
	 {
	    _has_yca = true;
            _use_yca = true;
	    rgb_layers(); _num_channels -= 3;
	    lumma_layers();
	 }
      }
      
      if ( channels.findChannel( N_("A") ) ||
	   channels.findChannel( N_("AR") ) ||
	   channels.findChannel( N_("AG") ) ||
	   channels.findChannel( N_("AB") ) )
      {
	 has_alpha = true;
	 alpha_layers();
      }
      
      if ( _has_left_eye || _has_right_eye )
      {
	 _layers.push_back( "left.anaglyph" );
	 _layers.push_back( "right.anaglyph" );
      }

      if ( _is_stereo )
      {
         _layers.push_back( "stereo.horizontal" );
         _layers.push_back( "stereo.crossed" );
      }


      Imf::ChannelList::ConstIterator i = channels.begin();
      Imf::ChannelList::ConstIterator e = channels.end();

      // Deal with single channels first, like Tag, Z Depth, etc.
      for ( ; i != e; ++i )
      {
         const std::string& layerName = i.name();
	 std::string name( layerName );
	 // Make names all uppercase, to avoid confusion
	 std::transform(name.begin(), name.end(), name.begin(), 
			(int(*)(int)) toupper);
	 if ( name == N_("R") || name == N_("G") || name == N_("B") ||
	      name == N_("A") || name == N_("Y") || name == N_("BY") || 
	      name == N_("RY") )
	    continue; // these channels are handled in shader directly

	 // Don't count layer.channel those are handled later
	 if ( name.find( N_(".") ) != string::npos ) continue;

	 _layers.push_back( layerName );
	 ++_num_channels;

      }


      // Deal with layers next like (Normals, Motion, etc)
      {
	 stringSet layerSet;
	 channels.layers( layerSet );

	 stringSet::const_iterator i = layerSet.begin();
	 stringSet::const_iterator e = layerSet.end();
	 for ( ; i != e; ++i )
	 {
	    _layers.push_back( (*i) );
	    ++_num_channels;

	    Imf::ChannelList::ConstIterator x;
	    Imf::ChannelList::ConstIterator s;
	    Imf::ChannelList::ConstIterator e;
	    channels.channelsWithPrefix( (*i).c_str(), s, e );
	    for ( x = s; x != e; ++x )
	    {
	       const char* layerName = x.name();
	       _layers.push_back( layerName );
	    }
	 }
      }
   }

   return true;
}

bool exrImage::handle_side_by_side_stereo( const boost::int64_t frame,
                                           const Imf::Header& h,
                                           Imf::FrameBuffer& fb )
{
   ChannelList channels = h.channels();

   char* ch = strdup( _channel );

   free( _channel );
   _channel = NULL;



   Imf::ChannelList::ConstIterator s;
   Imf::ChannelList::ConstIterator e;
   std::string prefix;
   if ( _multiview && _has_right_eye ) prefix = "right";

   // Find the iterators for a right channel prefix or all channels
   if ( prefix != "" )
   {
      channels.channelsWithPrefix( prefix.c_str(), s, e );
   }
   else
   {
      s = channels.begin();
      e = channels.end();
   }
   channels_order( frame, s, e, channels, h, fb );

   // If 3d is because of different headers exit now
   if ( !_multiview )
   {
      _channel = ch;
      return true;
   }

   _stereo[1] = _hires;

   //
   // Repeat for left eye
   //
   prefix = "";
   if ( _has_left_eye ) prefix = "left";
   if ( prefix != "" )
   {
      channels.channelsWithPrefix( prefix.c_str(), s, e );
   }
   else
   {
      s = channels.begin();
      e = channels.end();
   }
   channels_order( frame, s, e, channels, h, fb );
   _stereo[0] = _hires;

   _channel = ch;

   return true;
}

bool exrImage::find_channels( const Imf::Header& h,
			      Imf::FrameBuffer& fb,
			      boost::int64_t frame )
{
   bool ok = find_layers( h );
   if ( !ok ) return false;

   ChannelList channels = h.channels();

   char* channelPrefix = _channel;

   // If channel starts with #, we are dealing with a multipart exr
   if ( channelPrefix && channelPrefix[0] == '#' )
   {
      std::string part = channelPrefix;

      size_t idx = part.find( N_(".") );

      if ( idx == std::string::npos )
      {
         free( _channel );
         _channel = channelPrefix = NULL;
      }
      else
      {
         std::string root = part.substr( idx+1, part.size() );

         std::string ext = root;
         idx = root.rfind( N_(".") );
         if ( idx != std::string::npos )
         {
            ext = root.substr( idx+1, part.size() );
         }
 
         // When extension is one of RGBA we want to read RGBA together
         // so we remove the extension from prefix.
         if ( ext == "R" || ext == "G" || ext == "B" || ext == "A" )
         {
            ext = "";
            if ( idx == std::string::npos || idx == 1 )
               root = "";
            else
               root = root.substr( 0, idx-1 );
         }

         free( _channel );
         _channel = NULL;

         if ( root.size() ) _channel = strdup( root.c_str() );

         channelPrefix = _channel;
      }


      size_t pos = part.find( N_(" ") );
      part = part.substr( 1, pos );

      int oldpart = _curpart;

      _curpart = strtoul( part.c_str(), NULL, 10 );


      if ( oldpart != _curpart )
         return true;

   }


   if ( channelPrefix != NULL )
   {
      std::string ext = channelPrefix;
      std::string root = "";
      size_t pos = ext.rfind( N_(".") );
      if ( pos != std::string::npos )
      {
	 root = ext.substr( 0, pos );
	 ext = ext.substr( pos+1, ext.size() );
      }

      std::transform( root.begin(), root.end(), root.begin(),
		      (int(*)(int)) toupper);
      std::transform( ext.begin(), ext.end(), ext.begin(),
		      (int(*)(int)) toupper);

      _stereo_type = kNoStereo;

      if ( root == "STEREO" )
      {
         // Set the stereo type based on channel name extension
         if ( ext == "HORIZONTAL" )
            _stereo_type = kStereoSideBySide;
         else if ( ext == "CROSSED" )
            _stereo_type = kStereoCrossed;
         else
            LOG_ERROR( _("Unknown stereo type") );

         return handle_side_by_side_stereo(frame, h, fb);
      }
      else
      {
         Imf::ChannelList::ConstIterator s;
         Imf::ChannelList::ConstIterator e;

         if ( ext == "ANAGLYPH" && (_has_left_eye || _has_right_eye) )
         {
            if (root == "LEFT" ) _left_red = true;
            else _left_red = false;

            s = channels.begin();
            e = channels.end();

            if ( _multiview )
            {
               // When anaglyph, call channels order multi
               return channels_order_multi( frame, s, e, channels, h, fb );
            }
            else
            {
               _stereo_type = kStereoAnaglyph;
               return channels_order( frame, s, e, channels, h, fb );
            }
         }
         else
         {
            channels.channelsWithPrefix( channelPrefix, s, e );
            return channels_order( frame, s, e, channels, h, fb );
         }
      }
   }
   else
   {
      Imf::ChannelList::ConstIterator s = channels.begin();
      Imf::ChannelList::ConstIterator e = channels.end();

      return channels_order( frame, s, e, channels, h, fb );
   }
}

void exrImage::read_header_attr( const Imf::Header& h, boost::int64_t frame )
{
 
      const Imf::StringAttribute *attr =
	h.findTypedAttribute<Imf::StringAttribute> ( N_("renderingTransform") );
      if ( attr )
	{
	   if ( rendering_transform() &&
		attr->value() != rendering_transform() )
	      rendering_transform( attr->value().c_str() );
	}

      {
	const Imf::StringAttribute *attr =
	  h.findTypedAttribute<Imf::StringAttribute>( N_("lookModTransform") );
	if ( attr )
	  {
	     if ( look_mod_transform() &&
		  attr->value() != look_mod_transform() )
		look_mod_transform( attr->value().c_str() );
	  }
      }

      {
	const Imf::ChromaticitiesAttribute *attr =
	  h.findTypedAttribute<Imf::ChromaticitiesAttribute>( N_("chromaticities") );
	if ( attr )
	  {
	    chromaticities( attr->value() );
	  }
      }

      {
	const Imf::V2fAttribute *attr =
	  h.findTypedAttribute<Imf::V2fAttribute>( N_("adoptedNeutral") );
	if ( attr )
	  {
	  }
      }

      {
	const Imf::IntAttribute *attr =
	  h.findTypedAttribute<Imf::IntAttribute>( N_("imageState") );
	if ( attr )
	  {
	  }
      }

      {
	const Imf::StringAttribute *attr =
	  h.findTypedAttribute<Imf::StringAttribute>( N_("owner") );
	if ( attr )
	  {
	    _exif.insert( std::make_pair( _("Owner"), attr->value()) );
	  }
      }

      {
	const Imf::StringAttribute *attr =
	  h.findTypedAttribute<Imf::StringAttribute>(  N_("comments") );
	if ( attr )
	  {
	    _exif.insert( std::make_pair( _("Comments"), attr->value()) );
	  }
      }

      {
	const Imf::StringAttribute *attr =
	  h.findTypedAttribute<Imf::StringAttribute>( N_("capDate") );
	if ( attr )
	  {
	    _iptc.insert( std::make_pair( _("Capture Date"), attr->value()) );
	  }
      }

      {
	const Imf::FloatAttribute *attr =
	  h.findTypedAttribute<Imf::FloatAttribute>( N_("utcOffset") );
	if ( attr )
	  {
	    char buf[128];
	    sprintf( buf, N_("%f"), attr->value() );
	    _iptc.insert( std::make_pair( _("UTC Offset"), buf) );
	  }
      }

      {
	const Imf::FloatAttribute *attr =
	  h.findTypedAttribute<Imf::FloatAttribute>( N_("longitude") );
	if ( attr )
	  {
	    char buf[128];
	    sprintf( buf, N_("%f"), attr->value() );
	    _iptc.insert( std::make_pair( _("Longitude"), buf) );
	  }
      }

      {
	const Imf::FloatAttribute *attr =
	  h.findTypedAttribute<Imf::FloatAttribute>( N_("latitude") );
	if ( attr )
	  {
	    char buf[128];
	    sprintf( buf, N_("%f"), attr->value() );
	    _iptc.insert( std::make_pair( _("Latitude"), buf) );
	  }
      }

      {
	const Imf::FloatAttribute *attr =
	  h.findTypedAttribute<Imf::FloatAttribute>( N_("altitude") );
	if ( attr )
	  {
	    char buf[128];
	    sprintf( buf, N_("%f"), attr->value() );
	    _iptc.insert( std::make_pair( _("Altitude"), buf) );
	  }
      }

      {
	const Imf::FloatAttribute *attr =
	  h.findTypedAttribute<Imf::FloatAttribute>( N_("focus") );
	if ( attr )
	  {
	    char buf[128];
	    sprintf( buf, N_("%f"), attr->value() );
	    _exif.insert( std::make_pair( _("Focus"), buf) );
	  }
      }

      {
	const Imf::FloatAttribute *attr =
	  h.findTypedAttribute<Imf::FloatAttribute>( N_("expTime") );
	if ( attr )
	  {
	    char buf[128];
	    sprintf( buf, N_("%f"), attr->value() );
	    _exif.insert( std::make_pair( _("Exposure Time"), buf) );
	  }
      }

      {
	const Imf::FloatAttribute *attr =
	  h.findTypedAttribute<Imf::FloatAttribute>( N_("aperture") );
	if ( attr )
	  {
	    char buf[128];
	    sprintf( buf, N_("%f"), attr->value() );
	    _exif.insert( std::make_pair( _("Aperture"), buf) );
	  }
      }


      {
	const Imf::FloatAttribute *attr =
	  h.findTypedAttribute<Imf::FloatAttribute>( N_("isoSpeed") );
	if ( attr )
	  {
	    char buf[128];
	    sprintf( buf, N_("%f"), attr->value() );
	    _exif.insert( std::make_pair( _("ISO Speed"), buf) );
	  }
      }


      {
	const Imf::FloatAttribute *attr =
	  h.findTypedAttribute<Imf::FloatAttribute>( N_("keyCode") );
	if ( attr )
	  {
	    char buf[128];
	    sprintf( buf, N_("%f"), attr->value() );
	    _exif.insert( std::make_pair( _("Key Code"), buf) );
	  }
      }

      {
	const Imf::FloatAttribute *attr =
	h.findTypedAttribute<Imf::FloatAttribute>( N_("timeCode") );
	if ( attr )
	  {
	    char buf[128];
	    sprintf( buf, N_("%f"), attr->value() );
	    _exif.insert( std::make_pair( _("Timecode"), buf) );
	  }
      }

      {
	const Imf::StringAttribute *attr =
	h.findTypedAttribute<Imf::StringAttribute>( N_("wrapmodes") );
	if ( attr )
	  {
	    _exif.insert( std::make_pair( _("Wrap Modes"), attr->value()) );
	  }
      }

      if ( h.hasTileDescription() )
	{
	  const Imf::TileDescription& desc = h.tileDescription();
	  char buf[128];

	  switch( desc.mode )
	    {
	    case Imf::ONE_LEVEL:
	      break;
	    case Imf::MIPMAP_LEVELS:
	       {
		  TiledInputFile tin( sequence_filename(frame).c_str() );
		  sprintf( buf, N_("%d"), tin.numLevels() );
		  _exif.insert( std::make_pair( _("Mipmap Levels"), buf) );
		  break;
	       }
	     case Imf::RIPMAP_LEVELS:
		{
		   TiledInputFile tin( sequence_filename(frame).c_str() );
		   sprintf( buf, N_("%d"), tin.numXLevels() );
		   _exif.insert( std::make_pair( _("X Ripmap Levels"), buf) );
		   sprintf( buf, N_("%d"), tin.numYLevels() );
		   _exif.insert( std::make_pair( _("Y Ripmap Levels"), buf) );
		   break;
		}
	    default:
	      IMG_ERROR("Unknown mipmap mode");
	      break;
	    }

	  switch( desc.roundingMode )
	    {
	    case Imf::ROUND_DOWN:
	      _exif.insert( std::make_pair( _("Rounding Mode"), _("Down") ) );
	      break;
	    case Imf::ROUND_UP:
	      _exif.insert( std::make_pair( _("Rounding Mode"), _("Up") ) );
	      break;
	    default:
	      IMG_ERROR( _("Unknown rounding mode") );
	      break;
	    }
	}


      {
	const Imf::RationalAttribute* attr =
	  h.findTypedAttribute<Imf::RationalAttribute>("framesPerSecond");
	if ( attr )
	  {
	    _fps = (double) attr->value();
	  }

	if ( _play_fps <= 0 ) _play_fps = _fps;
      }

}

bool exrImage::fetch_multipart( const boost::int64_t frame )
{
   MultiPartInputFile inmaster ( sequence_filename(frame).c_str() );

   _stereo_type = kNoStereo;

   

   if ( _num_layers == 0 )
   {
   
      int i = 0;
      st[0] = st[1] = -1;
      for ( ; i < _numparts; ++i )
      {
         InputPart in( inmaster, i );
         Header header = in.header();

         if ( header.type() != DEEPSCANLINE &&
              header.type() != SCANLINEIMAGE &&
              header.type() != TILEDIMAGE ) continue;

         if ( _exif.empty() && _iptc.empty() )
            read_header_attr( header, frame );

	_pixel_ratio = header.pixelAspectRatio();
	_lineOrder   = header.lineOrder();
	_compression = header.compression(); 

         Imf::ChannelList channels = header.channels();

         Imf::ChannelList::ConstIterator s = channels.begin();
         Imf::ChannelList::ConstIterator e = channels.end();
         unsigned numChannels = 0;
         for ( ; s != e; ++s )
            ++numChannels;

         std::string name = header.name();
         size_t pos;
         while ( (pos = name.find( N_(".") )) != std::string::npos )
         {
            std::string n = name.substr( 0, pos-1 );
            n += "_";
            n += name.substr( pos+1, name.size() );
            name = n;
         }

         char buf[128];
         sprintf( buf, "#%d %s", i, name.c_str() );
         _layers.push_back( buf );
         ++_num_layers;

         std::string ext = name;
         if ( header.hasView() ) ext = header.view();

         std::transform( ext.begin(), ext.end(), ext.begin(),
                         (int(*)(int)) toupper);



         if ( numChannels >= 3 && st[1] == -1 &&
              ext.find( "RIGHT" ) != std::string::npos )
         {
            st[1] = i;
            _has_right_eye = true;
            _is_stereo = true;
         }
         if ( numChannels >= 3 && st[0] == -1 && 
              ext.find( "LEFT" ) != std::string::npos )
         {
            st[0] = i;
            _has_left_eye = true;
            _is_stereo = true;
         }


         for ( s = channels.begin(); s != e; ++s )
         {
            std::string layerName = buf;
            layerName += ".";
            layerName += s.name();
            _layers.push_back( layerName );
            ++_num_layers;
         }
         
      }

   }

   
   if ( _is_stereo && ( st[0] == -1 || st[1] == -1 ) )
   {
      IMG_ERROR( _("Could not find both stereo images in file") );
      return false;
   }


   
   if ( _is_stereo )
   {
      const char* channelPrefix = channel();
      if ( channelPrefix != NULL )
      {
          
         std::string ext = channelPrefix;
         std::string root = "";
         size_t pos = ext.rfind( N_(".") );
         if ( pos != std::string::npos )
         {
            root = ext.substr( 0, pos );
            ext = ext.substr( pos+1, ext.size() );
         }

          
         if ( root == "stereo" )
         {
            _stereo_type = kStereoSideBySide;
         }
         else if ( ext == "anaglyph" )
         {
            _stereo_type = kStereoAnaglyph;
         }
      }


      if ( _curpart == -1 ) _curpart = 0;

          
      for ( int i = 0 ; i < 2; ++i )
      {
          int oldpart = _curpart;
          if ( _stereo_type != kNoStereo ) _curpart = st[i];

          
          InputPart in( inmaster, _curpart );
          Header header = in.header();

          Box2i displayWindow = header.displayWindow();
          Box2i dataWindow = header.dataWindow();

          FrameBuffer fb;
          bool ok = find_channels( header, fb, frame );
          if (!ok) {
              IMG_ERROR( _("Could not locate channels in header") );
              return false;
          }
          


          if ( _curpart != oldpart )
          {
              InputPart ins( inmaster, _curpart );
              header = ins.header();

          
              FrameBuffer fb;
              bool ok = find_channels( header, fb, frame );
              if (!ok) {
                  IMG_ERROR( _("Could not locate channels in header") );
                  return false;
              }

          
              _pixel_ratio = header.pixelAspectRatio();
              _lineOrder   = header.lineOrder();
              displayWindow = header.displayWindow();
              dataWindow = header.dataWindow();


              if ( i == 0 || _stereo_type == kNoStereo )
              {
                  data_window( dataWindow.min.x, dataWindow.min.y,
                               dataWindow.max.x, dataWindow.max.y );

                  display_window( displayWindow.min.x, displayWindow.min.y,
                                  displayWindow.max.x, displayWindow.max.y );
              }
              else
              {
                  data_window2( dataWindow.min.x, dataWindow.min.y,
                                dataWindow.max.x, dataWindow.max.y );

                  display_window2( displayWindow.min.x, displayWindow.min.y,
                                   displayWindow.max.x, displayWindow.max.y );
              }



              ins.setFrameBuffer(fb);
              ins.readPixels( dataWindow.min.y, dataWindow.max.y );

          }
          else
          {
          
              _pixel_ratio = header.pixelAspectRatio();
              _lineOrder   = header.lineOrder();
              displayWindow = header.displayWindow();
              dataWindow = header.dataWindow();

              if ( i == 0 || _stereo_type == kNoStereo )
              {
                  data_window( dataWindow.min.x, dataWindow.min.y,
                               dataWindow.max.x, dataWindow.max.y );

                  display_window( displayWindow.min.x, displayWindow.min.y,
                                  displayWindow.max.x, displayWindow.max.y );
              }
              else
              {
                  data_window2( dataWindow.min.x, dataWindow.min.y,
                                dataWindow.max.x, dataWindow.max.y );

                  display_window2( displayWindow.min.x, displayWindow.min.y,
                                   displayWindow.max.x, displayWindow.max.y );
              }


              in.setFrameBuffer(fb);
              in.readPixels( dataWindow.min.y, dataWindow.max.y );

          }



          // Quick exit if stereo is off
          if ( _stereo_type == kNoStereo ) break;

         
          _stereo[i] = _hires;
      }

      

      if ( _stereo_type == kStereoAnaglyph )
      {
          
         make_anaglyph( _left_red );
      }
   }
   else
   {
      int oldpart = _curpart;

      InputPart in (inmaster, _curpart);
      Header header = in.header();
      Box2i& dataWindow = header.dataWindow();
      Box2i& displayWindow = header.displayWindow();

      data_window( dataWindow.min.x, dataWindow.min.y,
                   dataWindow.max.x, dataWindow.max.y );

      display_window( displayWindow.min.x, displayWindow.min.y,
                      displayWindow.max.x, displayWindow.max.y );

      FrameBuffer fb;
      bool ok = find_channels( header, fb, frame );
      if (!ok) {
         IMG_ERROR( _("Could not locate channels in header") );
         return false;
      }


      if ( _curpart != oldpart )
      {
         InputPart in (inmaster, _curpart);
         Header header = in.header();
         // const Box2i& displayWindow = header.displayWindow();
         dataWindow = header.dataWindow();
         displayWindow = header.displayWindow();

         data_window( dataWindow.min.x, dataWindow.min.y,
                      dataWindow.max.x, dataWindow.max.y );

         display_window( displayWindow.min.x, displayWindow.min.y,
                         displayWindow.max.x, displayWindow.max.y );

         FrameBuffer fb;
         bool ok = find_channels( header, fb, frame );
         if (!ok) {
            IMG_ERROR( _("Could not locate channels in header") );
            return false;
         }

         in.setFrameBuffer(fb);
         in.readPixels( dataWindow.min.y, dataWindow.max.y );
      }
      else
      {
         in.setFrameBuffer(fb);
         in.readPixels( dataWindow.min.y, dataWindow.max.y );
      }


   }

   return true;
}

/** 
 * Fetch the current EXR image
 * 
   * 
   * @return true if success, false if not
   */
  bool exrImage::fetch( const boost::int64_t frame ) 
  {

     try
     {

	if ( _levelX > 0 || _levelY > 0 )
	{
	   return fetch_mipmap( frame );
	}

        try
        {
           if ( _numparts < 0 )
           {
               MultiPartInputFile* infile = new MultiPartInputFile( 
               sequence_filename(frame).c_str() );

               _numparts = infile->parts();
               delete infile;
           }
        }
        catch(IEX_NAMESPACE::BaseExc &e)
        {
           IMG_ERROR( e.what() );
           return false;
        }


        if ( _numparts > 1 )
        {
           return fetch_multipart( frame );
        }

	InputFile in( sequence_filename(frame).c_str() );


	const Header& h = in.header();
	const Box2i& displayWindow = h.displayWindow();
	const Box2i& dataWindow = h.dataWindow();

        data_window( dataWindow.min.x, dataWindow.min.y,
                     dataWindow.max.x, dataWindow.max.y );

        display_window( displayWindow.min.x, displayWindow.min.y,
                        displayWindow.max.x, displayWindow.max.y );

	_rendering_intent = kRelativeIntent;

	if ( _exif.empty() && _iptc.empty() )
	   read_header_attr( h, frame );


	FrameBuffer fb;
	bool ok = find_channels( h, fb, frame );
	if (!ok) {
	   IMG_ERROR( _("Could not locate channels in header") );
	   return false;
	}

	_pixel_ratio = h.pixelAspectRatio();
	_lineOrder   = h.lineOrder();
	_compression = h.compression(); 


	in.setFrameBuffer(fb);

	in.readPixels( dataWindow.min.y, dataWindow.max.y );


	int dw = dataWindow.max.x - dataWindow.min.x + 1;
	int dh = dataWindow.max.y - dataWindow.min.y + 1;
	if ( dw <= 0 || dh <= 0 ) {
	   IMG_ERROR( _("Data window is negative") );
	   return false;
	}


	if ( _use_yca && !supports_yuv() )
	{
	   ycc2rgba( h, frame );
	}

      

    } 
    catch( const std::exception& e )
      {
	LOG_ERROR( e.what() );
	return false;
      }

    return true;
  }


bool exrImage::save( const char* file, const CMedia* img, 
                     const ImageOpts* const ipts )
{
    const EXROpts* const opts = (EXROpts*) ipts;

    mrv::image_type_ptr pic = img->hires();
    if (!pic) return false;


    mrv::Recti dpw = img->display_window();
    mrv::Recti daw = img->data_window();

    Box2i bdpw( V2i(dpw.x(), dpw.y()),
                V2i(dpw.x() + dpw.w() - 1, dpw.y() + dpw.h() - 1) );
    Box2i bdaw( V2i(daw.x(), daw.y()),
                V2i(daw.x() + daw.w() - 1, daw.y() + daw.h() - 1) );


    unsigned dw = daw.w();
    unsigned dh = daw.h();

    Header hdr( bdpw, bdaw );

    int dx = daw.x();
    int dy = daw.y();



    Imf::Compression comp = opts->compression();
    hdr.compression() = comp;


    Imf::PixelType pt = exrImage::pixel_type_to_exr( pic->pixel_type() );
    Imf::PixelType save_type = opts->pixel_type();

    hdr.channels().insert( N_("R"), Channel( save_type, 1, 1 ) );
    hdr.channels().insert( N_("G"), Channel( save_type, 1, 1 ) );
    hdr.channels().insert( N_("B"), Channel( save_type, 1, 1 ) );

    bool has_alpha = img->has_alpha();
    if ( has_alpha ) 
      {
	hdr.channels().insert( N_("A"), Channel( save_type, 1, 1 ) );
      }

    if ( img->look_mod_transform() )
      {
	Imf::StringAttribute attr( img->look_mod_transform() );
	hdr.insert( N_("lookModTransform"), attr );
      }

    if ( img->rendering_transform() )
      {
	Imf::StringAttribute attr( img->rendering_transform() );
	hdr.insert( N_("renderingTransform"), attr );
      }


    uint8_t* base = NULL;


    size_t size = 1;
    switch( save_type )
    {
        case Imf::UINT:
            size = sizeof(unsigned);
            break;
        case Imf::FLOAT:
            size = sizeof(float);
            break;
        default:
        case Imf::HALF:
            size = sizeof(half);
            break;
    }


    unsigned channels = pic->channels();
    unsigned total_size = dw*dh*size*channels;
    base = (uint8_t*) new uint8_t[total_size];


    if ( pt == save_type )
    {
        memcpy( base, pic->data().get(), total_size );
    }
    else
    {
       for ( unsigned y = 0; y < dh; ++y )
       {
	  for ( unsigned x = 0; x < dw; ++x )
    	  {
              CMedia::Pixel p = pic->pixel( x, y );
              unsigned yh = channels * ( x + y * dw );
              if ( save_type == Imf::HALF )
              {
                  half* s = (half*) base;
                  s[ yh ]   = p.r;
                  s[ yh+1 ] = p.g;
                  s[ yh+2 ] = p.b;
                  if ( has_alpha )
                      s[ yh+3 ] = p.a;
              }
              else if ( save_type == Imf::FLOAT )
              {
                  CMedia::Pixel* s = (CMedia::Pixel*) base;
                  if ( has_alpha )
                      s[ x + y * dw ] = p;
                  else
                  {
                      float* s = (float*) base;
                      s[ yh ] = p.r;
                      s[ yh + 1 ] = p.g;
                      s[ yh + 2 ] = p.b;
                  }
              }
              else if ( save_type == Imf::UINT )
              {
                  unsigned* s = (unsigned*) base;
                  s[ yh ] = p.r * 4294967295;
                  s[ yh + 1 ] = p.g * 4294967295;
                  s[ yh + 2 ] = p.b * 4294967295;
                  if ( has_alpha )
                      s[ yh + 3 ] = p.a * 4294967295;
              }
    	  }
       }
    }

    size_t xs = size * channels;
    size_t ys = xs * dw;

    int offset = xs*(-dx-dy*dw);

    FrameBuffer fb;
    fb.insert( N_("R"), Slice( save_type, (char*) &base[offset], xs, ys ) );
    fb.insert( N_("G"), Slice( save_type, 
                               (char*) &base[offset+size], xs, ys ) );
    fb.insert( N_("B"), Slice( save_type, 
                               (char*) &base[offset+2*size], xs, ys ) );

    if ( has_alpha )
      {
    	fb.insert( N_("A"), Slice( save_type, 
                                   (char*) &base[offset+3*size], xs, ys ) );
      }

    try {
      OutputFile out( file, hdr );
      out.setFrameBuffer( fb );
      out.writePixels( dh );
    }
    catch( std::exception& e )
      {
	LOG_ERROR( e.what() );
      }

    delete [] base;

    return true;
  }

} // namespace mrv
