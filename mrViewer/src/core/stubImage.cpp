/**
 * @file   stubImage.cpp
 * @author gga
 * @date   Fri Nov 10 14:29:35 2006
 * 
 * @brief  
 * 
 * 
 */


#include <cassert>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>

#include <fltk/run.h>
#include <boost/bind.hpp>

#include "stubImage.h"
#include "byteSwap.h"
#include "mrvString.h"
#include "mrvIO.h"


#undef max   // LOVE windows!

#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )

namespace 
{
  const char* kModule = "mpipe";
}


namespace mrv {

  using namespace std;

  unsigned stubImage::init = 0;

  struct stubData
  {
    stubData( MR_SOCKET id ) :
      socket(id)
    {
    }

    stubImage*       stub;
    const MR_SOCKET  socket;
  };


  struct fbData
  {
    int index;
    int type;
    int width;
    int height;
    int comps;
    int bits;
    std::string name;
  };

  typedef std::vector< fbData* > FrameBufferList;






  void mray_read( stubData* d )
  {
    const std::streamsize nMax = std::numeric_limits<streamsize>::max();

    MR_SOCKET theSocket = d->socket;
    stubImage* img = d->stub;

    // Get frame buffer list
    int nRet = send(theSocket,			// Connected socket
		    N_("fb_list\n"),			// Data buffer
		    8,		                // Length of data
		    0);				// Flags
    if (nRet == SOCKET_ERROR)
      {
	LOG_ERROR( _("send() failed") );
	mr_closesocket(theSocket); 
	return;
      }


#define BUF_LEN 256
    char szBuf[BUF_LEN+1];

    FrameBufferList buffers;

    bool stop = false;
    while ( !stop )
      {
	//
	// Wait for a reply
	//
	nRet = recv(theSocket,	// Connected socket
		    szBuf,	// Receive buffer
		    BUF_LEN,	// Size of receive buffer
		    0);		// Flags
	if (nRet == SOCKET_ERROR)
	  {
	    break;
	  }

	szBuf[nRet] = 0;

	std::istringstream parser( szBuf );
	std::string cmd;
	while ( std::getline(parser, cmd, ' ') )
	  {
	    if ( cmd == N_("fb_list:") )
	      {
		fbData* fb = new fbData;
		parser >> fb->index; // frame buffer #
		parser.ignore(nMax, ',');
		parser >> fb->type;
		parser >> fb->name;
		if ( fb->name.c_str()[0] == '-' ||
		     fb->name.c_str()[0] == '+' )
		  fb->name = fb->name.c_str() + 1;
		fb->name = fb->name.substr(0, fb->name.length()-1);
		std::transform(fb->name.begin(), 
			       fb->name.end(), fb->name.begin(), 
			       (int(*)(int)) toupper);
		parser >> fb->width;
		parser >> fb->height;
		parser.ignore(nMax, ',');
		parser >> fb->comps;
		parser >> fb->bits;
		parser.ignore(nMax, '\n');

		if ( ! buffers.empty() )
		   img->new_buffer( fb->index, fb->width, fb->height );

		buffers.push_back( fb );
	      }
	    else
	      {
		stop = true;
	      }
	  }
      }

    {
      char layername[256];
      FrameBufferList::iterator i = buffers.begin();
      FrameBufferList::iterator e = buffers.end();
      for ( ; i != e; ++i )
	{
	  const fbData* fb = *i;

	  if ( fb->index == 0 )
	    {
	      img->default_layers();
	      continue;
	    }

	  const stringArray& layers = img->layers();
	  if ( std::find( layers.begin(), layers.end(), fb->name ) != 
	       layers.end() )
	    {
	      sprintf( layername, N_("%s_%d"), fb->name.c_str(), fb->index );
	      img->add_layer( layername, fb->index );
	    }
	  else
	    {
	      img->add_layer( fb->name.c_str(), fb->index );
	    }
	}
    }


    //
    // Send data to the server
    //
    std::string cmd = N_("stream_begin");
    FrameBufferList::iterator i = buffers.begin();
    FrameBufferList::iterator e = buffers.end();
    char comma = ' ';
    for ( ; i != e; ++i )
      {
	const fbData* fb = *i;
	sprintf( szBuf, N_("%c%d"), comma, fb->index );
	comma = ',';
	cmd += szBuf;
      }
    cmd += N_("\n");


    nRet = send(theSocket,			// Connected socket
		cmd.c_str(),			// Command
		(int) cmd.size(),		// Length of data
		0);				// Flags
    if (nRet == SOCKET_ERROR)
      {
	LOG_ERROR( _("send() failed") );
	mr_closesocket(theSocket); 
	return;
      }


    {
      FrameBufferList::iterator i = buffers.begin();
      FrameBufferList::iterator e = buffers.end();
      for ( ; i != e; ++i )
	{
	  delete *i;
	}
    }

    stop = false;
    unsigned width, height;
    unsigned frame_ends = 0;
    bool aborted = false;

    while ( !aborted && !stop )
      {

	//
	// Wait for a reply
	//
	nRet = recv(theSocket,	// Connected socket
		    szBuf,	// Receive buffer
		    BUF_LEN,	// Size of receive buffer
		    0);		// Flags
	if (nRet == SOCKET_ERROR)
	  {
	    break;
	  }
	szBuf[nRet] = 0;

	aborted = img->aborted();



	std::istringstream parser( szBuf );
	std::string cmd;
	while ( !stop && std::getline(parser, cmd, ' ') )
	  {
	    if ( img->aborted() )
	      break;

	    stringArray tokens;
	    mrv::split_string( tokens, cmd, N_("\n") );


	    size_t numTokens = tokens.size();
	    for ( size_t i = 0; i < numTokens; ++i )
	      {
		cmd = tokens[i];

		if ( img->aborted() )
		  break;

		int xl, yl, xh, yh;
		if ( strncmp( cmd.c_str(), N_("rect_begin:"), 11 ) == 0 )
		  {
		    // ignore the rest of the line (which should 
		    // be empty really)
		    parser.ignore(nMax, '\n');
		    continue;
		  }
		else if ( strncmp( cmd.c_str(), N_("frame_begin:"), 12 ) == 0 )
		  {
		    int frame;
		    parser >> frame; // frame #
		    parser >> width;
		    parser >> height;

		    if ( img->width()  != width ||
			 img->height() != height )
		      {
			img->resize_buffers( width, height );
		      }

		    if ( frame_ends == 0 )
		      img->start_timer();
		    img->crosshatch();

		    // ignore the rest of the line (which should be empty really)
		    parser.ignore(nMax, '\n');
		    continue;
		  }
		else if ( strncmp( cmd.c_str(), N_("rect_data:"), 10 ) == 0 )
		  {
		    char c;
		    int size, fb, fbtype;
		    short comps, bits;

		    parser >> size >> c;
		    parser >> xl >> yl >> xh >> yh >> c;
		    parser >> fb >> fbtype >> c;
		    parser >> width >> height >> comps >> bits;

		    parser.ignore(nMax, '\n');

		    int w = xh - xl + 1;
		    int h = yh - yl + 1;

		    // size read from stream sometimes is > than this, 
		    // which is wrong.
		    size = w * h * comps * bits/8;


		    int pos = parser.tellg();
		    int read = nRet - pos;


		    boost::uint8_t* bytes = new boost::uint8_t[size];
		    if ( read > 0 )
		      {
			assert( read <= size );
			memcpy( bytes, szBuf + pos, read);
		      }
		    else
		      {
			read = 0;
		      }


		    boost::uint8_t* rest = bytes + read;
		    read = size - read;

		    int sum = 0;
		    int total = read;
		    assert( total >= 0 );

		    while ( sum < total )
		      {
			int data = recv(theSocket,	// Connected socket
					(char*)rest,	// Receive buffer
					read,	// Size of receive buffer
					0);	// Flags
			if (data == SOCKET_ERROR)
			  {
			    LOG_ERROR( _("recv() failed") );
			    stop = true;
			    break;
			  }
			sum  += data;
			rest += data;
			read -= data;
		      }

		    if ( sum != total )
		      {
			LOG_ERROR( _("Wrond data size received.") );
			LOG_ERROR( _("Got: ") << sum 
				   << _(" expected: ") << total );
			stop = true;
		      }
		    if ( stop ) break;

		    if ( img->width()  != width ||
			 img->height() != height )
		      continue;


		    mrv::image_type_ptr buffer = img->frame_buffer( fb );

		    CMedia::PixelType* pixels = (CMedia::PixelType*)buffer->data().get();
		    if ( pixels == NULL ) break;

		    // substract one from components to
		    // later do iteration thru pixels with one step
		    comps -= 1;

		    CMedia::PixelType p;
		    switch( bits )
		      {
		      case 1:
			{
			  boost::uint8_t* m;
			  boost::uint8_t* b = bytes;
			  for ( int y = yl; y <= yh; ++y )
			    {
			      for ( int x = xl; x <= xh; ++x )
				{
				  m = b++;
				  p.r = float(*m);
				  if ( comps > 0 ) m += w;
				  p.g = float(*m);
				  if ( comps > 1 ) m += w;
				  p.b = float(*m);
				  if ( comps > 2 ) m += w;
				  p.a = float(*m);
				  img->set_float_pixel( fb, x, y, p );
				}
			      b += w * comps;
			    }
			  break;
			}
		      case 8:
			{
			  boost::uint8_t* m;
			  boost::uint8_t* b = bytes;
			  for ( int y = yl; y <= yh; ++y )
			    {
			      for ( int x = xl; x <= xh; ++x )
				{
				  m = b++;
				  p.r = *m / 255.0f;
				  if ( comps > 0 ) m += w;
				  p.g = *m / 255.0f;
				  if ( comps > 1 ) m += w;
				  p.b = *m / 255.0f;
				  if ( comps > 2 ) m += w;
				  p.a = *m / 255.0f;
				  img->set_float_pixel( fb, x, y, p );
				}
			      b += w * comps;
			    }
			  break;
			}
		      case 16:
			{
			  float val;
			  unsigned short* m;
			  unsigned short* b = (unsigned short*)bytes;
			  for ( int y = yl; y <= yh; ++y )
			    {
			      for ( int x = xl; x <= xh; ++x )
				{
				  m = b++;
				  *m = ntohs( *m );
				  val = *m / 65535.0f;
				  p.r = val;
				  if ( comps > 0 ) 
				    {
				      m += w;
				      *m = ntohs( *m );
				      val = *m / 65535.0f;
				    }
				  p.g = val;
				  if ( comps > 1 ) 
				    {
				      m += w;
				      *m = ntohs( *m );
				      val = *m / 65535.0f;
				    }
				  p.b = val;
				  if ( comps > 2 ) 
				    {
				      m += w;
				      *m = ntohs( *m );
				      val = *m / 65535.0f;
				    }
				  p.a = val;
				  img->set_float_pixel( fb, x, y, p );
				}
			      b += w * comps;
			    }
			  break;
			}
		      case 32:
			{
			  float* m;
			  float* b = (float*) bytes;
			  for ( int y = yl; y <= yh; ++y )
			    {
			      for ( int x = xl; x <= xh; ++x )
				{
				  m = b++;
				  MAKE_BIGENDIAN( *m );
				  p.r = *m;
				  if ( comps > 0 ) 
				    {
				      m += w;
				      MAKE_BIGENDIAN( *m );
				    }
				  p.g = *m;
				  if ( comps > 1 ) 
				    {
				      m += w;
				      MAKE_BIGENDIAN( *m );
				    }
				  p.b = *m;
				  if ( comps > 2 ) 
				    {
				      m += w;
				      MAKE_BIGENDIAN( *m );
				    }
				  p.a = *m;
				  img->set_float_pixel( fb, x, y, p );
				}
			      b += w * comps;
			    }
			  break;
			}
		      default:
			LOG_ERROR( _("Unknown bit depth") );
			stop = true;
			break;
		      }

		    delete [] bytes;

		    yh = height - yh - 1;
		    img->refresh( mrv::Recti( xl, yh, w, h ) );
		    img->image_damage( img->image_damage() |
				       CMedia::kDamageThumbnail );
		    break;
		  }
		else if ( strncmp( cmd.c_str(), N_("rect_end:"), 9 ) == 0 )
		  {	    // ignore the rest of the line
		    parser.ignore(nMax, '\n');
		  }
		else if ( strncmp( cmd.c_str(), N_("frame_end:"), 10 ) == 0 )
		  {
		    ++frame_ends;
		    if ( frame_ends == 2 )
		      {
			frame_ends = 0;
			img->end_timer();
		      }
		    parser.ignore(nMax, '\n');
		    break;
		  }
	      }
	  }
      }

    mr_closesocket(theSocket);

    if ( !aborted )   img->thread_exit();
    delete d;
  }





  stubImage::stubImage() :
    CMedia(),
    _aborted( false ),
    _portA( 0 ),
    _portB( 0 ),
    _host( NULL ),
    id( -1 )
  {
    if ( init == 0 )
      {
	mr_init_socket_library();
      }

    ++init;

    _gamma = 1.0f;
  }



  stubImage::stubImage( const CMedia* other ) :
    CMedia(),
    _portA( 0 ),
    _portB( 0 ),
    _host( NULL ),
    id( -1 )
  {

    char* orig = strdup( other->filename() );
    for ( char* s = orig; *s != 0; ++s )
      {
	if ( *s == '\t' )
	  {
	    orig = s + 1; break;
	  }
      }

    char now[128];
    _ctime = ::time(NULL);
    strftime( now, 127, N_("%H:%M:%S - %d %b"), localtime(&_ctime) );

    char name[256];
    sprintf( name, N_("%s\t%s"), now, orig );
    filename( name );
    _filename = strdup( other->filename() );

    int ws = other->width();
    int wh = other->height();
    _gamma = other->gamma();

    image_size( ws, wh );
    _pixel_ratio = other->pixel_ratio();

    stubImage* oStub = static_cast< stubImage* >( const_cast< CMedia*>(other) );
    _layers = oStub->layers();
    _num_channels = oStub->number_of_channels();

    PixelBuffers::const_iterator i = oStub->_pixelBuffers.begin();
    PixelBuffers::const_iterator e = oStub->_pixelBuffers.end();
    for ( ; i != e; ++i )
      {
	new_buffer( i->first, ws, wh );

	mrv::image_type_ptr buffer = _pixelBuffers[i->first];
	mrv::image_type_ptr oBuffer = oStub->frame_buffer( i->first );
	*buffer = *oBuffer;
      }


    _layerBuffers = oStub->_layerBuffers;
    if ( oStub->channel() )
       _channel = strdup( oStub->channel() );

    const char* profile = other->icc_profile();
    if ( profile )  icc_profile( profile );

    const char* transform = other->look_mod_transform();
    if ( transform )  look_mod_transform( transform );

    transform = other->rendering_transform();
    if ( transform )  rendering_transform( transform );

    const char* lbl = other->label();
    if ( lbl )  _label = strdup( lbl );

    free( orig );
  }



  stubImage::~stubImage()
  {
    _aborted = true;
    free( _host ); _host = NULL;

    if ( id != -1 ) mr_closesocket( id );
    --init;
    if ( init == 0 )
      {
	mr_cleanup_socket_library();
      }

    wait_for_rthreads();

    clear_buffers();
  }



void stubImage::wait_for_rthreads()
{
  // Wait for all threads to exit
   thread_pool_t::iterator i = _rthreads.begin();
   thread_pool_t::iterator e = _rthreads.end();
   for ( ;i != e; ++i )
   {
      // std::cerr << "join thread" << std::endl;
      // (*i)->join();
      // std::cerr << "joined thread" << std::endl;
      delete *i;
   }

   _rthreads.clear();
}

void stubImage::thread_exit()
{
  thread_pool_t::iterator i = _rthreads.begin();
  thread_pool_t::iterator e = _rthreads.end();
  for ( ; i != e; ++i )
    {
      delete *i;
    }

  _rthreads.clear();
}
  /*! Test a block of data read from the start of the file to see if it
    looks like the start of an .stub file. This returns true if the 
    data contains STUB's magic number and a "channels" string in the 8th
    position.
  */
  bool stubImage::test(const boost::uint8_t *data, unsigned num)
  {
    if ( num != 128 ) return false;
    return (strstr((char*) data, N_("ray") ) != 0);
  }

  void stubImage::clear_to_NANs()
  {
    mrv::image_type_ptr frame = _hires;
    static const PixelType p( std::numeric_limits<float>::quiet_NaN(), 
			      std::numeric_limits<float>::quiet_NaN(), 
			      std::numeric_limits<float>::quiet_NaN(), 
			      std::numeric_limits<float>::quiet_NaN() );

    unsigned w = width();
    unsigned h = height();
    for ( unsigned int y = 0; y < h; ++y )
      {
	for ( unsigned int x = 0; x < w; ++x )
	  {
	    frame->pixel( x, y, p );
	  }
      }
  }

  void stubImage::parse_stub()
  {
    FILE* f = fltk::fltk_fopen( filename(), N_("rb") );
    char data[129];
    size_t num = fread(data, sizeof(char), 129, f);
    if ( num != 128 ) {
      IMG_ERROR( _("Not a mental ray stub file anymore.") );
      _host = NULL;
      fclose(f);
      return;
    }

    fclose(f);


    std::string stub( data + 3 );
    stringArray tokens;
    mrv::split_string( tokens, stub, N_(",") );

    _version = (float) atof( tokens[0].c_str() );
    unsigned int W = atoi( tokens[1].c_str() );
    unsigned int H = atoi( tokens[2].c_str() );
    if ( W != _w || H != _h )
      {
	clear_buffers();
	image_size( W, H );
	allocate_pixels(1);
	clear_to_NANs();
	_pixelBuffers.insert( std::make_pair( 0, _hires ) );
      }
    _host  = strdup( (char*) tokens[3].c_str() );
    _portA = atoi( tokens[6].c_str() );
    _portB = atoi( tokens[7].c_str() );
  }



  void stubImage::crosshatch( const mrv::Recti& rect )
  {
    Mutex::scoped_lock lk( _mutex );

    int w = width();

    int xl = rect.x();
    int yl = rect.y();

    int xh = rect.r();
    int yh = rect.b();


    mrv::image_type_ptr frame = _hires;

    static const PixelType p( std::numeric_limits<float>::quiet_NaN(), 
			      std::numeric_limits<float>::quiet_NaN(), 
			      std::numeric_limits<float>::quiet_NaN(), 
			      std::numeric_limits<float>::quiet_NaN() );
    for ( int y = yl; y < yh; ++y )
      {
	int offset = y * w;
	for ( int x = xl; x < xh; ++x )
	  {
	    int i = offset + x;
	    if ( (i % 4) == (y % 4) )
	      {
		frame->pixel( x, y, p );
	      }
	  }
      }
    CMedia::refresh();
  }


  bool stubImage::has_changed()
  {
    struct stat sbuf;
    int result = stat( filename(), &sbuf );
    if ( result == -1 ) return false;

    if ( _ctime != sbuf.st_mtime )
      {
	if ( sbuf.st_size == 128 )
	  {
	    unsigned int portA = _portA;
	    unsigned int portB = _portB;
	    char* host = NULL;
	    if ( _host ) host = strdup( _host );
	    parse_stub();
	    bool ret = false;
	    if ( _host && host && strcmp( host, _host ) != 0 ) ret = true;
	    else if ( _host && !host )  ret = true;
	    else if ( portA != _portA ) ret = true;
	    else if ( portB != _portB ) ret = true;
	    if ( ret == true )
	      {
		_ctime = sbuf.st_mtime;
		if ( ! _rthreads.empty() )
		  {
		    _aborted = true;
		    thread_exit();
		  }
		return true;
	      }
	    else
	      {
		_ctime = sbuf.st_mtime;
		return false;
	      }
	  }
	else
	  {
	    // if not connected to host already, return true
	    // Otherwise, image is already being refreshed thanks to stub
	    // connection (no need to reload image)
	    if ( _rthreads.empty() ) return true;
	  }
      }
    return false;
  }

  bool stubImage::fetch(const int64_t frame) 
  {
    if ( !_rthreads.empty() ) return true;

    struct stat sbuf;
    int result = stat( filename(), &sbuf );
    if ( result == -1 ) {
      clear_buffers();
      image_size(0,0);
      return false;
    }

    // use TCP/IP to connect to machine and port
    // read available data so far
    parse_stub();

    if ( _host == NULL ) return false;


    if ( id != -1 ) mr_closesocket( id );

    id = mr_new_socket_client( _host, _portB );

    if ( ((int)id) < 0 ) 
      {
	LOG_ERROR( _("Socket error: Could not connect to ") << _host 
		   << _(" at port ") << _portB << ".");
	return false;
      }

    // start a thread to read image
    stubData* data = new stubData( id );
    data->stub = this;

    _rthreads.push_back( new boost::thread( boost::bind( mray_read, data ) ) );
    return true;
  }



  void stubImage::add_layer( const char* name, const int fb )
  {
    Mutex::scoped_lock lk( _mutex );
    _layers.push_back( name );
    ++_num_channels;
    _layerBuffers.insert( std::make_pair( name, fb ) );

    image_damage( image_damage() | kDamageLayers );
  }

  void stubImage::clear_buffers()
  {
    Mutex::scoped_lock lk( _mutex );
    _pixelBuffers.erase( _pixelBuffers.begin(), _pixelBuffers.end());
    _layerBuffers.erase( _layerBuffers.begin(), _layerBuffers.end());
    _hires.reset();
  }

  void stubImage::new_buffer( const unsigned int fb, 
			      const unsigned int ws, const unsigned int hs )
  {
    Mutex::scoped_lock lk( _mutex );

    mrv::image_type_ptr buf( new VideoFrame( _frame,
					     ws, hs,
					     4, VideoFrame::kBGRA,
					     VideoFrame::kFloat ) );
    
    _pixelBuffers.insert( std::make_pair( fb, buf ) );
    if ( _pixelBuffers.size() == 1 )
      {
	_hires = buf;
      }
  }

  void stubImage::resize_buffers( const unsigned int ws, 
				  const unsigned int hs )
  {
    int fb = 0;
    PixelBuffers::iterator i = _pixelBuffers.begin();
    PixelBuffers::iterator e = _pixelBuffers.end();
    for ( ; i != e; ++i )
      {
	if ( i->second == _hires )
	  {
	    fb = i->first;
	    break;
	  }
      }

  
    {
      Mutex::scoped_lock lock( _mutex );
      _w = ws;
      _h = hs;
      for ( i = _pixelBuffers.begin() ; i != e; ++i )
	{
	  i->second.reset( new VideoFrame( _frame,
					   ws,
					   hs,
					   4, VideoFrame::kBGRA,
					   VideoFrame::kFloat ) );
	}
    
      _hires = _pixelBuffers[fb];
    }
  }

  // Sets a pixel of frame buffer fb.
  bool stubImage::set_float_pixel( const unsigned int fb, 
				   const unsigned int x, 
				   const unsigned int y, 
				   const PixelType& c )
  {
    Mutex::scoped_lock lk( _mutex );

    if ( _pixelBuffers.find(fb) == _pixelBuffers.end() )
      {
	LOG_ERROR( _("No such framebuffer - cannot set pixel") );
	return false;
      }

    if ( _pixelBuffers[fb] == NULL )
      {
	LOG_ERROR( _("Buffer was NULL - internal error") );
	return false;
      }

    unsigned int dw = width();
    unsigned int dh = height();

    if ( x >= dw || y >= dh )
      {
	LOG_ERROR( _("Invalid pixel buffer coordinates ") << x << ", " << y );
	return false;
      }

    //   assert( c.r != 1.0f || (c.r == 1.0f && c.r-c.g < 0.5f) );

    _pixelBuffers[fb]->pixel( x, dh-y-1, c );

    return true;
  }

  mrv::image_type_ptr stubImage::frame_buffer( int idx )
  {
    if ( _pixelBuffers.find(idx) == _pixelBuffers.end() )
      return mrv::image_type_ptr();
    return _pixelBuffers[idx];
  }

  void stubImage::channel( const char* chinput )
  {
    const char* c = chinput;
    std::string ch( chinput );
    if ( ch == N_("Color") || ch == N_("Red") || ch == N_("Green") ||
	 ch == N_("Blue") ||  ch == N_("Alpha") || ch == N_("Alpha Overlay") ||
	 ch == N_("Lumma") )
      c = NULL;

    if ( c == _channel ) return;

    free( _channel );

    if ( c )
      {
	_channel = strdup( c );
      }
    else
      {
	_channel = NULL;
      }

    // ...change image buffer...
    {
      Mutex::scoped_lock lk( _mutex );
      if ( _channel == NULL )
	{
	  _hires = _pixelBuffers[0];
	}
      else
	{
	  int fb = _layerBuffers[ _channel ];
	  _hires = _pixelBuffers[fb];
	}
    }

    CMedia::refresh();
  }

  /** 
   * 
   * 
   * @param f 
   */
  bool stubImage::frame( const int64_t f )
  {
    assert( _fileroot != NULL );

    _frame = f;

    if ( _filename == NULL )
      {
	timestamp();
	fetch(f);
      }

    return true;
  }


  void stubImage::start_timer()
  {
    _startRenderTime = ::time(NULL);
  }

  void stubImage::end_timer()
  {
    time_t endRenderTime = ::time(NULL);
    struct tm* a;
    a = localtime( &_startRenderTime );
    unsigned int startSecs = a->tm_sec + a->tm_min * 60 + a->tm_hour * 3600;
    a = localtime( &endRenderTime );
    unsigned int endSecs = a->tm_sec + a->tm_min * 60 + a->tm_hour * 3600;


    _startRenderTime = endRenderTime;

    if ( endSecs < startSecs )
      {
	// Render began late at night and ended the morning of next day
	endSecs = (24 * 3600 - startSecs) + endSecs;
      }

    unsigned int renderSecs = endSecs - startSecs;

    int hours, mins, secs;
    hours = renderSecs / 3600;
    renderSecs -= (hours * 3600);
    mins = renderSecs / 60;
    renderSecs -= (mins * 60);
    secs = renderSecs;

    char render_time[256];
    sprintf( render_time, N_("%02d:%02d:%02d"), hours, mins, secs );

    free(_label);

    char buf[256];
    sprintf( buf, N_("Render Time: %s"), render_time );

    _label = (char*) malloc( strlen(buf) + 1 );
    strcpy( _label, buf );

    _exif[ N_("Render Time") ] = render_time;
  }


} // namespace mrv
