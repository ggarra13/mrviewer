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
#include <ImfInputFile.h>
#include <ImfTiledInputFile.h>
#include <ImfStandardAttributes.h>
#include <ImfOutputFile.h>
#include <ImathMath.h> // for Math:: functions
#include <ImathBox.h>  // for Box2i
#include <ImfStandardAttributes.h>
#include <ImfIntAttribute.h>
#include <ImfRgbaYca.h>

#include "mrvThread.h"
#include "exrImage.h"
#include "mrvIO.h"

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


  exrImage::exrImage() :
  CMedia(),
  _levelX( 0 ),
  _levelY( 0 )
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
   int dw = dataWindow.max.x - dataWindow.min.x + 1;
   int dh = dataWindow.max.y - dataWindow.min.y + 1;
   if ( dw <= 0 || dh <= 0 )  return false;
   int dx = dataWindow.min.x;
   int dy = dataWindow.min.y;

   int order[4];
   order[0] = order[1] = order[2] = order[3] = 0;

   // First, count the number of channels
   int idx = 0;
   Imf::PixelType imfPixelType = Imf::UINT;
   std::vector< std::string > channelList;
   Imf::ChannelList::ConstIterator i;
   unsigned int numChannels = 0;
   for ( i = s; i != e; ++i, ++idx )
   {
      const std::string layerName = i.name();
      const Imf::Channel* ch = channels.findChannel( 
						    layerName.c_str()
						     );
      if ( !ch ) continue;
   
      if ( ch->type > imfPixelType ) imfPixelType = ch->type;
      
      std::string ext = layerName;
      int pos = layerName.rfind( N_(".") );
      if ( pos != string::npos )
      {
	 ext = ext.substr( pos+1, ext.size() );
      }

      std::transform( ext.begin(), ext.end(), ext.begin(),
		      (int(*)(int)) toupper);
      if ( ext == N_("R") || ext == N_("RED") || 
	   ext == N_("Y")  ) order[0] = idx;
      if ( ext == N_("G") || ext == N_("GREEN") || 
	   ext == N_("RY") ) order[1] = idx;
      if ( ext == N_("B") || ext == N_("BLUE")|| 
	   ext == N_("BY") ) order[2] = idx;
      if ( ext == N_("A") || ext == N_("ALPHA") ) order[3] = idx;
      channelList.push_back( layerName );
      ++numChannels;
   }

   if ( numChannels == 0 && channel() )
   {
      mrvALERT( _("Image file \"") << filename() << 
		_("\" has no channels named with prefix \"") 
		<< channel() << "\"." );
      return false;
   }
   else if ( numChannels > 4 && channel() )
   {
      mrvALERT( _("Image file \"") << filename() 
		<< _("\" contains more than 4 channels "
		     "named with prefix \"") 
		<< channel() << "\"" );
      numChannels = 4;
   }
   
   // Prepare format
   image_type::Format format = VideoFrame::kLumma;
   int offsets[4];
   offsets[0]  = 0;
   if ( _has_yca )
   {
      unsigned size  = dw * dh;
      unsigned size2 = dw * dh / 4;
      offsets[1]  = size;
      offsets[2]  = size + size2;
      offsets[3]  = size + size2 * 2;
      if ( numChannels >= 3 && has_alpha() )
      {
	 format = VideoFrame::kYByRy420A;
	 numChannels = 4;
      }
      else if ( numChannels >= 2 )
      {
	 format = VideoFrame::kYByRy420;
	 numChannels = 3;
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
   
   allocate_pixels( frame, numChannels, format,
		    pixel_type_conversion( imfPixelType ) );
   
   static size_t xs[4], ys[4];
   if ( _has_yca )
   {
      for ( int i = 0; i < numChannels; ++i )
	 xs[i] = _hires->pixel_size();
      
      unsigned int dw2 = dw / 2;
      ys[0] = xs[0] * dw;
      ys[1] = xs[0] * dw2;
      ys[2] = xs[0] * dw2;
      ys[3] = xs[0] * dw;
   }
   else
   {
      
      for ( int i = 0; i < numChannels; ++i )
      {
	 xs[i] = _hires->pixel_size() * numChannels;
	 ys[i] = xs[i] * dw;
      }
   }
   


   boost::uint8_t* pixels = (boost::uint8_t*)_hires->data().get();
   memset( pixels, 0, _hires->data_size() );
   

   // Then, prepare frame buffer for them
   int start = ( (-dx - dy * dw) * _hires->pixel_size() *
		 _hires->channels() );
   boost::uint8_t* base = pixels + start;

   Imf::Channel* ch = NULL;
   idx = 0;
   for ( i = s; i != e && idx < 4; ++i, ++idx )
   {
      int k = order[idx];
      const std::string& layerName = channelList[k];


      ch = channels.findChannel( layerName.c_str() );
      
      if ( !ch ) continue;
      
      char* buf = (char*)base + offsets[idx] * _hires->pixel_size();
      
      fb.insert( layerName.c_str(), 
		 Slice( imfPixelType, buf, xs[idx], ys[idx],
			ch->xSampling, ch->ySampling) );
   }

   return true;
}

void exrImage::ycc2rgba( const Imf::Header& hdr, const boost::int64_t frame )
{
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
	 CMedia::PixelType p = _hires->pixel( x, y );
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
	
	std::string fileName = sequence_filename(frame);

	TiledInputFile in( fileName.c_str() );

	if (!in.isValidLevel (_levelX, _levelY))
	{
	   THROW (Iex::InputExc, "Level (" << _levelX << ", " 
		  << _levelY << ") does "
		  "not exist in file " << fileName << ".");
	}

	Imf::Header h = in.header();
	h.dataWindow() = in.dataWindowForLevel(_levelX, _levelY);
	h.displayWindow() = h.dataWindow();

	read_header_attr( h, frame );

	FrameBuffer fb;
	bool ok = find_channels( h, fb, frame );
	if ( !ok ) return false;

	in.setFrameBuffer(fb);


	int tx = in.numXTiles( _levelX );
	int ty = in.numYTiles( _levelY );
	
	//
	// For maximum speed, try to read the tiles in
	// the same order as they are stored in the file.
	//

	if (in.header().lineOrder() == INCREASING_Y)
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
     
     }
     catch( const std::exception& e )
     {
	mrvALERT( e.what() );
	return false;
     }
  }

bool exrImage::find_channels( const Imf::Header& h,
			      Imf::FrameBuffer& fb,
			      boost::int64_t frame )
{

   const Box2i& dataWindow = h.dataWindow();
   int dw = dataWindow.max.x - dataWindow.min.x + 1;
   int dh = dataWindow.max.y - dataWindow.min.y + 1;
   if ( dw <= 0 || dh <= 0 )  return false;
   int dx = dataWindow.min.x;
   int dy = dataWindow.min.y;

   //       int dpw = displayWindow.max.x - displayWindow.min.x + 1;
   //       if ( dpw > dw ) dw = dpw;
   //       int dph = displayWindow.max.y - displayWindow.min.y + 1;
   //       if ( dph > dh ) dh = dpw;

   image_size( dw, dh );

   Imf::ChannelList channels = h.channels();



   _layers.clear();
   _num_channels = 0;
   _gamma = 2.2f;

   bool has_rgb = false, has_alpha = false;
   if ( channels.findChannel( N_("R") ) ||
	channels.findChannel( N_("G") ) ||
	channels.findChannel( N_("B") ) )
   {
      has_rgb = true;
   }

   _has_yca = false;
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
	 rgb_layers(); _num_channels -= 3;
	 lumma_layers();
      }
   }
   else
   {
      rgb_layers();
      lumma_layers();
   }

   if ( channels.findChannel( "A" ) )
   {
      has_alpha = true;
      alpha_layers();
   }


   Imf::ChannelList::Iterator i = channels.begin();
   Imf::ChannelList::Iterator e = channels.end();

   // Deal with single channels first, like Tag, Z Depth, etc.
   for ( ; i != e; ++i )
   {
      std::string name( i.name() );
      // Make names all uppercase, to avoid confusion
      std::transform(name.begin(), name.end(), name.begin(), 
		     (int(*)(int)) toupper);
      if ( name == N_("R") || name == N_("G") || name == N_("B") ||
	   name == N_("A") || name == N_("Y") || name == N_("BY") || 
	   name == N_("RY") ||
	   name == N_("RED") || name == N_("GREEN") || name == N_("BLUE") || 
	   name == N_("ALPHA") ||
	   // international versions
	   name == _("RED") || name == _("GREEN") || name == _("BLUE") || 
	   name == _("ALPHA")
	   ) 
	 continue; // these channels are handled in shader directly

      if ( name.find( N_(".") ) != string::npos ) continue;
      _layers.push_back( i.name() );
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


      const char* channelPrefix = channel();
      if ( channelPrefix != NULL )
	{

	  unsigned int numChannels = 0;
	  
	  Imf::ChannelList::ConstIterator i;
	  Imf::ChannelList::ConstIterator s;
	  Imf::ChannelList::ConstIterator e;
	  channels.channelsWithPrefix( channelPrefix, s, e );

	  channels_order( frame, s, e, channels, h, fb );
	}
      else
	{
	  Imf::ChannelList::Iterator s = channels.begin();
	  Imf::ChannelList::Iterator e = channels.end();

	  channels_order( frame, s, e, channels, h, fb );
	}

      return true;

}

void exrImage::read_header_attr( const Imf::Header& h, boost::int64_t frame )
{
 
      const Imf::StringAttribute *attr =
	h.findTypedAttribute<Imf::StringAttribute> ( N_("renderingTransform") );
      if ( attr )
	{
	  rendering_transform( attr->value().c_str() );
	}

      {
	const Imf::StringAttribute *attr =
	  h.findTypedAttribute<Imf::StringAttribute>( N_("lookModTransform") );
	if ( attr )
	  {
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

/** 
   * Fetch the current EXR image
   * 
   * 
   * @return true if success, false if not
   */
  bool exrImage::fetch( const boost::int64_t frame ) 
  {

     try {

      if ( _levelX > 0 || _levelY > 0 )
      {
	 return fetch_mipmap( frame );
      }

      InputFile in( sequence_filename(frame).c_str() );


      const Header& h = in.header();
      const Box2i& displayWindow = h.displayWindow();
      const Box2i& dataWindow = h.dataWindow();


      _pixel_ratio = h.pixelAspectRatio();
      _lineOrder   = h.lineOrder();
      _fps         = 24.0f;

      _compression = h.compression(); 



      _rendering_intent = kRelativeIntent;

      read_header_attr( h, frame );



      FrameBuffer fb;
      bool ok = find_channels( h, fb, frame );
      if (!ok) return false;

      in.setFrameBuffer(fb);

      in.readPixels( dataWindow.min.y, dataWindow.max.y );
 
      int dw = dataWindow.max.x - dataWindow.min.x + 1;
      int dh = dataWindow.max.y - dataWindow.min.y + 1;
      if ( dw <= 0 || dh <= 0 ) return false;


      if ( dataWindow.min.x != 0 || dataWindow.min.y != 0 ||
	   dataWindow.max.x != dw || dataWindow.max.y != dh )
	data_window( dataWindow.min.x, dataWindow.min.y,
		     dataWindow.max.x, dataWindow.max.y );

      if ( displayWindow != dataWindow )
	{
	  data_window( dataWindow.min.x, dataWindow.min.y,
		       dataWindow.max.x, dataWindow.max.y );
	  display_window( displayWindow.min.x, displayWindow.min.y,
			  displayWindow.max.x, displayWindow.max.y );
	}


      if ( _has_yca && !supports_yuv() )
	{
	   ycc2rgba( h, frame );
	}

    } 
    catch( const std::exception& e )
      {
	 cerr << e.what() << endl;
	mrvALERT( e.what() );
	return false;
      }

    return true;
  }


  bool exrImage::save( const char* file, const CMedia* orig )
  {
    unsigned dw = orig->width();
    unsigned dh = orig->height();

    // @TODO: add the ability to save all channels and choose compression
    //        and channel data-type (HALF)

    Header hdr( dw, dh );


    Imf::Compression comp = ZIP_COMPRESSION;

    unsigned idx = 0;
    std::string compression = orig->compression();
    const char** s = kCompression;
    for ( ; *s != 0; ++s, ++idx )
      {
	if ( compression == *s ) {
	  comp = (Imf::Compression) idx;
	  break;
	}
      }

    hdr.compression() = comp;

    size_t xs = sizeof(CMedia::PixelType);
    size_t ys = sizeof(CMedia::PixelType) * dw;

    hdr.channels().insert( N_("R"), Channel( Imf::FLOAT, 1, 1 ) );
    hdr.channels().insert( N_("G"), Channel( Imf::FLOAT, 1, 1 ) );
    hdr.channels().insert( N_("B"), Channel( Imf::FLOAT, 1, 1 ) );

    bool has_alpha = orig->has_alpha();
    if ( has_alpha ) 
      {
	hdr.channels().insert( N_("A"), Channel( Imf::FLOAT, 1, 1 ) );
      }

    if ( orig->look_mod_transform() )
      {
	Imf::StringAttribute attr( orig->look_mod_transform() );
	hdr.insert( N_("lookModTransform"), attr );
      }

    if ( orig->rendering_transform() )
      {
	Imf::StringAttribute attr( orig->rendering_transform() );
	hdr.insert( N_("renderingTransform"), attr );
      }

    mrv::image_type_ptr pic;
    {
      CMedia* img = const_cast< CMedia* >( orig );
      CMedia::Mutex& m = img->video_mutex();
      SCOPED_LOCK(m);
      pic = img->hires();
    }
    if (!pic) return false;

    CMedia::PixelType* base = new CMedia::PixelType[dw*dh];
    
    for ( unsigned y = 0; y < dh; ++y )
      {
	for ( unsigned x = 0; x < dw; ++x )
	  {
	    base[ x + y * dw ] = pic->pixel( x, y );
	  }
      }

    FrameBuffer fb; 
    fb.insert( N_("R"), Slice( Imf::FLOAT, (char*) &base->r, xs, ys ) );
    fb.insert( N_("G"), Slice( Imf::FLOAT, (char*) &base->g, xs, ys ) );
    fb.insert( N_("B"), Slice( Imf::FLOAT, (char*) &base->b, xs, ys ) );

    if ( has_alpha )
      {
	fb.insert( N_("A"), Slice( Imf::FLOAT, (char*) &base->a, xs, ys ) );
      }

    try {
      OutputFile out( file, hdr );
      out.setFrameBuffer( fb );
      out.writePixels( dh );
    }
    catch( std::exception& e )
      {
	mrvALERT( e.what() );
      }

    delete [] base;

    return true;
  }

} // namespace mrv
