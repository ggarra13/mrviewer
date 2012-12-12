/**
 * @file   rmanImage.cpp
 * @author gga
 * @date   Fri Nov 10 14:29:26 2006
 * 
 * @brief  
 * 
 * 
 */

#include <sys/stat.h>

#include <cassert>
#include <ctime>
#include <iostream>
#include <strstream>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>

#include <fltk/run.h>

#include "mrThread.h"
#include "stubImage.h"
#include "mrvImageBrowser.h"
#include "mrvImageView.h"
#include "byteSwap.h"
#include "mrvString.h"


#undef max   // LOVE windows!



// #define DBG(x) cerr << x << endl;
#define DBG(x) 
#define LOG_ERROR(x) cerr << x << endl;


namespace {

using namespace std;


extern ImageBrowser* imageList;
extern ImageView*    imageView;

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

extern "C" MR_THREAD_RETURN rman_read( void* data )
{
  const std::streamsize nMax = std::numeric_limits<streamsize>::max();

  stubData* d = (stubData*) data;
  MR_SOCKET theSocket = d->socket;
  stubImage* img = d->stub;

  int nRet;

#if 0

  // Get frame buffer list
  nRet = send(theSocket,			// Connected socket
	      "fb_list\n",			// Data buffer
	      8,		                // Length of data
	      0);				// Flags
  if (nRet == SOCKET_ERROR)
    {
      LOG_ERROR( "send() failed" );
      mr_closesocket(theSocket); 
      MR_THREAD_EXIT( nRet );
    }
#endif


#define BUF_LEN 4096
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

       istrstream parser( szBuf );
       std::string cmd;
       while ( std::getline(parser, cmd, ' ') )
	 {
	   if ( cmd == "fb_list:" )
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
	     sprintf( layername, "%s_%d", fb->name.c_str(), fb->index );
	     img->add_layer( layername, fb->index );
	   }
	 else
	   {
	     img->add_layer( fb->name.c_str(), fb->index );
	   }
       }
   }

   imageView->update_layers();

#if 0
   //
   // Send data to the server
   //
   std::string cmd = "stream_begin";
   FrameBufferList::iterator i = buffers.begin();
   FrameBufferList::iterator e = buffers.end();
   char comma = ' ';
   for ( ; i != e; ++i )
     {
       const fbData* fb = *i;
       sprintf( szBuf, "%c%d", comma, fb->index );
       comma = ',';
       cmd += szBuf;
     }
   cmd += "\n";


   nRet = send(theSocket,			// Connected socket
	       cmd.c_str(),			// Command
	       (int) cmd.size(),		// Length of data
	       0);				// Flags
   if (nRet == SOCKET_ERROR)
   {
     LOG_ERROR("send() failed");      ;
     mr_closesocket(theSocket); 
     MR_THREAD_EXIT( nRet );
   }
#endif


   {
     FrameBufferList::iterator i = buffers.begin();
     FrameBufferList::iterator e = buffers.end();
     for ( ; i != e; ++i )
       {
	 delete *i;
       }
   }

   stop = false;
   int width, height;

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


      istrstream parser( szBuf );

      std::string cmd;
      while ( !stop && std::getline(parser, cmd, ' ') )
	{
	  stringArray tokens;
	  mrv::split_string( tokens, cmd, "\n" );

	  size_t numTokens = tokens.size();
	  for ( size_t i = 0; i < numTokens; ++i )
	    {
	      cmd = tokens[i];

	      int xl, yl, xh, yh;
	      if ( strncmp( cmd.c_str(), "rect_begin:", 11 ) == 0 )
		{
		  // ignore the rest of the line (which should be empty really)
		  parser.ignore(nMax, '\n');
		  continue;
		}
	      else if ( strncmp( cmd.c_str(), "frame_begin:", 12 ) == 0 )
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

		  img->start_timer();
		  img->crosshatch();

		  // ignore the rest of the line (which should be empty really)
		  parser.ignore(nMax, '\n');
		  continue;
		}
	      else if ( strncmp( cmd.c_str(), "rect_data:", 10 ) == 0 )
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

		  boost::uint8_t* bytes = new boost::uint8_t[size];

		  int read;
		  int pos = parser.tellg();
		  read = nRet - pos;
		  if ( read > 0 )
		    {
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

		  while ( sum < total )
		    {
		      int data = recv(theSocket,	// Connected socket
				      (char*)rest,	// Receive buffer
				      read,	// Size of receive buffer
				      0);	// Flags
		      if (data == SOCKET_ERROR)
			{
			  LOG_ERROR("recv() " );
			  stop = true;
			  break;
			}
		      sum  += data;
		      rest += data;
		      read -= data;
		    }

		  if ( sum != total )
		    {
		      LOG_ERROR( "WRONG DATA SIZE RECEIVED." );
		      LOG_ERROR( "GOT: " << sum << " expected: " << total );
		      stop = true;
		    }
		  if ( stop ) break;

		  if ( img->width()  != width ||
		       img->height() != height )
		    continue;


		  CMedia::PixelType* pixels = img->frame_buffer( fb );
		  if ( pixels == NULL ) break;

		  comps -= 1;

		  switch( bits )
		    {
		    case 1:
		      {
			boost::uint8_t* m;
			boost::uint8_t* b = bytes;
			for ( int y = yl; y <= yh; ++y )
			  {
			    int offset = (height - y - 1) * width;
			    for ( int x = xl; x <= xh; ++x, ++b )
			      {
				CMedia::PixelType& p = pixels[ offset + x ];
				m = b;
				p.r = *m;
				if ( comps > 0 ) m += w;
				p.g = *m;
				if ( comps > 1 ) m += w;
				p.b = *m;
				if ( comps > 2 ) m += w;
				p.a = *m;
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
			    int offset = (height - y - 1) * width;
			    for ( int x = xl; x <= xh; ++x, ++b )
			      {
				CMedia::PixelType& p = pixels[ offset + x ];
				m = b;
				p.r = *m / 255.0f;
				if ( comps > 0 ) m += w;
				p.g = *m / 255.0f;
				if ( comps > 1 ) m += w;
				p.b = *m / 255.0f;
				if ( comps > 2 ) m += w;
				p.a = *m / 255.0f;
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
			    int offset = (height - y - 1) * width;
			    for ( int x = xl; x <= xh; ++x, ++b )
			      {
				CMedia::PixelType& p = pixels[ offset + x ];
				m = b;
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
			    int offset  = (height - y - 1) * width;
			    for ( int x = xl; x <= xh; ++x, ++b )
			      {
				CMedia::PixelType& p = pixels[ offset + x ];
				m = b;
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
			      }
			    b += w * comps;
			  }
			break;
		      }
		    default:
		      LOG_ERROR("Unknown bit depth");
		      stop = true;
		      break;
		    }
		  delete [] bytes;

		  yh = height - 1 - yh;
		  img->refresh( mrv::Recti( xl, yh, w, h ) );
		  break;
		}
	    }
      }
   }


   mr_closesocket(theSocket); 
   img->thread_exit();
   delete d; 
   MR_THREAD_EXIT(0);
}


} // namespace



namespace mrv {



  rmanImage::rmanImage() :  stubImage()
  {
  }


  /*! Test a block of data read from the start of the file to see if it
    looks like the start of an .stub file. This returns true if the 
    data contains STUB's magic number and a "channels" string in the 8th
    position.
  */
  bool rmanImage::test(const boost::uint8_t *data, unsigned num)
  {
    if ( num != 128 ) return false;
    return (strstr((char*) data, "rman") != 0);
  }



  void rmanImage::parse_stub()
  {
     FILE* f = fltk::fltk_fopen( filename(), "rb" );
     if (!f) {
	LOG_ERROR("Could not open '" << filename() << "'" );
	return;
     }

    char data[129];
    size_t num = fread(data, sizeof(char), 129, f);
    if ( num != 128 ) {
      LOG_ERROR("not a renderman stub file anymore. ");
      _host = NULL;
      fclose(f);
      return;
    }

    fclose(f);


    std::string stub( data + 3 );
    stringArray tokens;
    mrv::split_string( tokens, stub, "," );

    _version = (float) atof( tokens[0].c_str() );
    unsigned int W = atoi( tokens[1].c_str() );
    unsigned int H = atoi( tokens[2].c_str() );
    if ( W != _w || H != _h )
      {
	clear_buffers();
	image_size( W, H );
	allocate_image();

	_pixelBuffers.insert( std::make_pair( 0, (PixelType*)_hires->data().get() ) );
      }
    _host  = strdup( (char*) tokens[3].c_str() );
    _portA = atoi( tokens[6].c_str() );
    _portB = atoi( tokens[7].c_str() );
  }



  bool rmanImage::fetch() 
  {
    if ( _thread_id != 0 )
      {
	mr_thread_kill( _thread_id );
	_thread_id = 0;
      }

    image_size(0,0);

    struct stat sbuf;
    int result = stat( filename(), &sbuf );
    if ( result == -1 ) return false;

    // use TCP/IP to connect to machine and port
    // read available data so far
    parse_stub();

    if ( _host == NULL ) return false;


    if ( id != -1 ) mr_closesocket( id );

    id = mr_new_socket_client( _host, _portB );

    if ( ((int)id) < 0 ) 
      {
	postage_stamp();
	LOG_ERROR("Socket error: Could not connect to " << _host 
		  << " at port " << _portB << ".");
	return false;
      }

    // start a thread to read image
    stubData* data = new stubData( id );
    data->stub = this;

    if ( ! mr_thread_new( _thread_id, rman_read, data ) )
      {
	LOG_ERROR( "Could not create new thread for reading stub." );
      }

    return true;
  }

} // namespace mrv
