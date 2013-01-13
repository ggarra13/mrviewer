//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// 
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <fstream>
#include <set>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>  // for PRId64


// #define BOOST_ASIO_ENABLE_HANDLER_TRACKING 1

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio.hpp>

#include "mrvClient.h"
#include "mrvServer.h"
#include "mrViewer.h"
#include "gui/mrvLogDisplay.h"
#include "gui/mrvIO.h"
#include "gui/mrvReel.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageBrowser.h"

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

//----------------------------------------------------------------------

namespace {
const char* const kModule = "server";
}


//----------------------------------------------------------------------

namespace mrv {


Parser::Parser( boost::asio::io_service& io_service, mrv::ViewerUI* v ) :
connected( false ),
socket_( io_service ),
ui( v )
{
}

Parser::~Parser()
{
   if ( !ui || !ui->uiView ) return;

   ParserList::iterator i = ui->uiView->_clients.begin();
   ParserList::iterator e = ui->uiView->_clients.end();
   for ( ; i != e; ++i  )
   {
      if ( *i == this )
      {
	 ui->uiView->_clients.erase( i );
	 break;
      }
   }
   ui = NULL;
}

void Parser::write( std::string s )
{
   if ( !ui || !ui->uiView || !connected ) return;

   ParserList::iterator i = ui->uiView->_clients.begin();
   ParserList::iterator e = ui->uiView->_clients.end();

   for ( ; i != e; ++i )
      (*i)->deliver( s );
}

bool Parser::parse( const std::string& m )
{
   if ( !connected ) return false;

   std::istringstream is( m );
   std::string cmd;
   is >> cmd;

   if ( !ui ) return false;

   static mrv::Reel r;

   if ( cmd == "Reel" )
   {
      std::string name;
      is >> name;
      r = ui->uiReelWindow->uiBrowser->reel( name.c_str() );
      if (!r) {
	 r = ui->uiReelWindow->uiBrowser->new_reel( name.c_str() );
      }
      return true;
   }
   else if ( cmd == "CurrentReel" )
   {
      std::string name;
      is >> name;
      r = ui->uiReelWindow->uiBrowser->reel( name.c_str() );
      if (!r) {
	 r = ui->uiReelWindow->uiBrowser->new_reel( name.c_str() );
      }
      return true;
   }
   else if ( cmd == "Image" )
   {
      std::string imgname;
      is >> imgname;
      imgname = imgname.substr( 1, imgname.size() - 2 );

      bool found = false;
      if ( r )
      {
	 mrv::MediaList::iterator j = r->images.begin();
	 mrv::MediaList::iterator e = r->images.end();
	 for ( ; j != e; ++j )
	 {
	    std::string fileroot = (*j)->image()->directory();
	    fileroot += "/";
	    fileroot += (*j)->image()->name();
	    if ( (*j)->image() && fileroot == imgname )
	    {
	       found = true;
	    }
	 }
      
	 if (!found)
	 {
	    stringArray files;
	    files.push_back( imgname );
	   
	    ui->uiReelWindow->uiBrowser->load( files, false );
	 }
      }

      return true;
   }
   else if ( cmd == "CurrentImage" )
   {
      std::string imgname;
      is >> imgname;
      imgname = imgname.substr( 1, imgname.size() - 2 );

      if ( r )
      {
	 mrv::MediaList::iterator j = r->images.begin();
	 mrv::MediaList::iterator e = r->images.end();
	 int idx = 0;
	 bool found = false;
	 for ( ; j != e; ++j, ++idx )
	 {
	    std::string fileroot = (*j)->image()->directory();
	    fileroot += "/";
	    fileroot += (*j)->image()->name();
	    if ( (*j)->image() && fileroot == imgname )
	    {
	       ParserList c = ui->uiView->_clients;
	       ui->uiView->_clients.clear();
	       ui->uiReelWindow->uiBrowser->change_image( idx );
	       ui->uiView->_clients = c;
	       found = true;
	       break;
	    }
	 }

	 if (! found )
	 {
	    stringArray files;
	    files.push_back( imgname );
	   
	    ui->uiReelWindow->uiBrowser->load( files, false );
	 }
      }

      return true;
   }
   if ( cmd == "sync_image" )
   {
      std::string cmd;
      unsigned num = ui->uiReelWindow->uiBrowser->number_of_reels();
      for (unsigned i = 0; i < num; ++i )
      {
	 mrv::Reel r = ui->uiReelWindow->uiBrowser->reel( i );
	 cmd = "Reel ";
	 cmd += r->name;
	 deliver( cmd );

	 mrv::MediaList::iterator j = r->images.begin();
	 mrv::MediaList::iterator e = r->images.end();
	 for ( ; j != e; ++j )
	 {
	    cmd = "Image \"";
	    cmd += (*j)->image()->directory();
	    cmd += "/";
	    cmd += (*j)->image()->name();
	    cmd += "\"";
	    deliver( cmd );

	    char buf[128];
	    boost::int64_t frame = (*j)->image()->frame();
	    sprintf( buf, "seek %" PRId64, frame );
	    cmd = buf;
	    deliver( cmd );
	 }
      }

      mrv::Reel r = ui->uiReelWindow->uiBrowser->current_reel();
      if (r)
      {
	 cmd = "CurrentReel ";
	 cmd += r->name;
	 deliver( cmd );
      }

      mrv::media img = ui->uiView->foreground();
      if ( img )
      {
	 cmd = "CurrentImage \"";
	 cmd += img->image()->directory();
	 cmd += "/";
	 cmd += img->image()->name();
	 cmd += "\"";
	 deliver( cmd );
      }

      return true;
   }
   else if ( cmd == "stop" )
   {
      ParserList c = ui->uiView->_clients;
      ui->uiView->_clients.clear();

      boost::int64_t f;
      is >> f;
      ui->uiView->stop();

      ui->uiView->_clients = c;
      return true;
   }
   else if ( cmd == "playfwd" )
   {
      ParserList c = ui->uiView->_clients;
      ui->uiView->_clients.clear();

      ui->uiView->play_forwards();

      ui->uiView->_clients = c;
      return true;
   }
   else if ( cmd == "playback" )
   {
      ParserList c = ui->uiView->_clients;
      ui->uiView->_clients.clear();

      ui->uiView->play_backwards();

      ui->uiView->_clients = c;
      return true;
   }
   else if ( cmd == "seek" )
   {
      ParserList c = ui->uiView->_clients;
      ui->uiView->_clients.clear();

      boost::int64_t f;
      is >> f;
      ui->uiView->seek( f );

      ui->uiView->_clients = c;

      return true;
   }
   return false;
}


//------------------------ --------------------------

//----------------------------------------------------------------------


tcp_session::tcp_session(boost::asio::io_service& io_service,
			 mrv::ViewerUI* const v) :
input_deadline_(io_service),
non_empty_output_queue_(io_service),
output_deadline_(io_service),
Parser(io_service, v)
{
 
   // boost::asio::socket_base::debug option(true);
   // socket_.set_option( option );

   input_deadline_.expires_at(boost::posix_time::pos_infin);
   output_deadline_.expires_at(boost::posix_time::pos_infin);
   

   // The non_empty_output_queue_ deadline_timer is set to pos_infin 
   // whenever the output queue is empty. This ensures that the output 
   // actor stays asleep until a message is put into the queue.
   non_empty_output_queue_.expires_at(boost::posix_time::pos_infin);
}

tcp_session::~tcp_session()
{
}

tcp::socket& tcp_session::socket()
{
   return socket_;
}

// Called by the server object to initiate the four actors.
void tcp_session::start()
{
   connected = true;

   ui->uiView->_clients.push_back( this );

   start_read();
   
   //	std::cerr << "start1: " << socket_.native_handle() << std::endl;
   // input_deadline_.async_wait(
   //     boost::bind(&tcp_session::check_deadline,
   //     shared_from_this(), &input_deadline_));
   
   await_output();
   
   // std::cerr << "start2: " << socket_.native_handle() << std::endl;
   // output_deadline_.async_wait(
   //     boost::bind(&tcp_session::check_deadline,
   //     shared_from_this(), &output_deadline_));
   
}

bool tcp_session::stopped()
{
   return !socket_.is_open();
}

void tcp_session::deliver(const std::string msg)
{
   
   output_queue_.push_back(msg + "\n");

   
   // Signal that the output queue contains messages. Modifying the expiry
   // will wake the output actor, if it is waiting on the timer.
   non_empty_output_queue_.expires_at(boost::posix_time::neg_infin);  
   //non_empty_output_queue_.expires_from_now(boost::posix_time::seconds(0));  
}


void tcp_session::stop()
{
   connected = false;
   boost::system::error_code ignored_ec;
   socket_.close(ignored_ec);
   input_deadline_.cancel();
   non_empty_output_queue_.cancel();
   output_deadline_.cancel();
}


void tcp_session::start_read()
{
   // Set a deadline for the read operation.
   
   // input_deadline_.expires_from_now(boost::posix_time::seconds(30));
   // input_deadline_.expires_at(boost::posix_time::pos_infin);
   
   // Start an asynchronous operation to read a newline-delimited message.
   boost::asio::async_read_until(socket(), input_buffer_, '\n',
				 boost::bind(&tcp_session::handle_read, 
					     shared_from_this(), 
					     boost::asio::placeholders::error));
}

void tcp_session::handle_read(const boost::system::error_code& ec)
{
   if (stopped())
      return;
   
   if (!ec)
   {
      // Extract the newline-delimited message from the buffer.
      std::string msg;
      std::istream is(&input_buffer_);
      is.exceptions( std::ifstream::eofbit );
      
      
      try {
	 while ( std::getline(is, msg) )
	 {
	    if ( msg != "" && msg != "OK" && msg != "Not OK")
	    {
	       if ( parse( msg ) )
	       {
		  write( msg );  // send message to all clients
		  deliver( "OK" );
	       }
	       else
	       {
		  deliver( "Not OK" );
	       }
	    }
	 }
      } 
      catch ( std::ios_base::failure e )
      {
      }
      
      
      start_read();
   }
   else
   {
      LOG_ERROR( "ERROR handle_read " << ec );
      stop();
   }
}

void tcp_session::await_output()
{

   if (stopped())
      return;

   
   if (output_queue_.empty())
   {
      // There are no messages that are ready to be sent. The actor goes to
      // sleep by waiting on the non_empty_output_queue_ timer. When a new
      // message is added, the timer will be modified and the actor will
      // wake.
      

      non_empty_output_queue_.async_wait(
					 boost::bind(&tcp_session::await_output,
						     shared_from_this())
					 ); 
      non_empty_output_queue_.expires_at(boost::posix_time::pos_infin);
   }
   else
   {
      start_write();
   }
}

void tcp_session::start_write()
{
   // Start an asynchronous operation to send a message.
   boost::asio::async_write(socket(),
			    boost::asio::buffer(output_queue_.front()),
			    boost::bind(&tcp_session::handle_write, 
					shared_from_this(), _1));
   
}

void tcp_session::handle_write(const boost::system::error_code& ec)
{
   if (stopped())
      return;

   if (!ec)
   {
      output_queue_.pop_front();

      await_output();
   }
   else
   {
      stop();
   }
}

void tcp_session::check_deadline(deadline_timer* deadline)
{
   if (stopped())
      return;
   
   // Check whether the deadline has passed. We compare the deadline against
   // the current time since a new asynchronous operation may have moved the
   // deadline before this actor had a chance to run.
   if (deadline->expires_at() <= deadline_timer::traits_type::now())
   {
      // The deadline has passed. Stop the session. The other actors will
      // terminate as soon as possible.
      stop();
   }
   else
   {
      // Put the actor back to sleep.
      deadline->async_wait(
			   boost::bind(&tcp_session::check_deadline,
				       shared_from_this(), deadline));
   }
}


//----------------------------------------------------------------------

//
// This class manages socket timeouts by applying the concept of a deadline.
// Some asynchronous operations are given deadlines by which they must complete.
// Deadlines are enforced by two "actors" that persist for the lifetime of the
// session object, one for input and one for output:
//
//  +----------------+                     +----------------+
//  |                |                     |                |
//  | check_deadline |<---+                | check_deadline |<---+
//  |                |    | async_wait()   |                |    | async_wait()
//  +----------------+    |  on input      +----------------+    |  on output
//              |         |  deadline                  |         |  deadline
//              +---------+                            +---------+
//
// If either deadline actor determines that the corresponding deadline has
// expired, the socket is closed and any outstanding operations are cancelled.
//
// The input actor reads messages from the socket, where messages are delimited
// by the newline character:
//
//  +------------+
//  |            |
//  | start_read |<---+
//  |            |    |
//  +------------+    |
//          |         |
//  async_- |    +-------------+
//   read_- |    |             |
//  until() +--->| handle_read |
//               |             |
//               +-------------+
//
// The deadline for receiving a complete message is 30 seconds. If a non-empty
// message is received, it is delivered to all subscribers. If a heartbeat (a
// message that consists of a single newline character) is received, a heartbeat
// is enqueued for the client, provided there are no other messages waiting to
// be sent.
//
// The output actor is responsible for sending messages to the client:
//
//  +--------------+
//  |              |<---------------------+
//  | await_output |                      |
//  |              |<---+                 |
//  +--------------+    |                 |
//      |      |        | async_wait()    |
//      |      +--------+                 |
//      V                                 |
//  +-------------+               +--------------+
//  |             | async_write() |              |
//  | start_write |-------------->| handle_write |
//  |             |               |              |
//  +-------------+               +--------------+
//
// The output actor first waits for an output message to be enqueued. It does
// this by using a deadline_timer as an asynchronous condition variable. The
// deadline_timer will be signalled whenever the output queue is non-empty.
//
// Once a message is available, it is sent to the client. The deadline for
// sending a complete message is 30 seconds. After the message is successfully
// sent, the output actor again waits for the output queue to become non-empty.
//


server::server(boost::asio::io_service& io_service,
	       const tcp::endpoint& listen_endpoint,
	       mrv::ViewerUI* v)
: io_service_(io_service),
  acceptor_(io_service, listen_endpoint),
  ui_( v )
{
   start_accept();
}

void server::start_accept()
{
   //    tcp_session_ptr new_session(new tcp_session(io_service_, ui_));
   tcp_session_ptr new_session(
			       boost::make_shared<tcp_session>(
							       boost::ref(
									  io_service_
									  ),
							       boost::ref(ui_)
							       )
			       );
   

   acceptor_.async_accept(new_session->socket(),
			  boost::bind(&server::handle_accept, this, 
				      new_session, _1));
}

void server::handle_accept(tcp_session_ptr session,
			   const boost::system::error_code& ec)
{
   if (!ec)
   {
      session->start();
   }
   
   start_accept();
}

ConnectionUI* ViewerUI::uiConnection = NULL;

void server::create(mrv::ViewerUI* ui)
{
   unsigned port = ui->uiConnection->uiServerPort->value();
   ServerData* data = new ServerData;
   data->port = port;
   data->ui = ui;

   boost::thread t( boost::bind( mrv::server_thread, 
				 data ) );
}



//----------------------------------------------------------------------

void server_thread( const ServerData* s )
{
   try
   {

      boost::asio::io_service io_service;

      tcp::endpoint listen_endpoint(tcp::v4(), s->port);

      server rp(io_service, listen_endpoint, s->ui);

      s->ui->uiConnection->uiServerGroup->deactivate();
      s->ui->uiConnection->uiClientGroup->deactivate();

      LOG_CONN( "Created server at port " << s->port );

      delete s;


      io_service.run();
   }
   catch (std::exception& e)
   {
      LOG_ERROR( "Exception: " << e.what() );
   }
}

} // namespace mrv


