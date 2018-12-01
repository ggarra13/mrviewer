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
//
// async_tcp_client.cpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

//#define BOOST_ASIO_ENABLE_HANDLER_TRACKING 1
//#define BOOST_ASIO_ENABLE_BUFFER_DEBUGGING 1

#include <boost/lexical_cast.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <fstream>
#include "mrvServer.h"
#include "mrvClient.h"
#include "mrViewer.h"
#include "gui/mrvIO.h"
#include "mrvImageView.h"

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

namespace {
const char* kModule = "client";
}

namespace mrv {

//
// This class manages socket timeouts by applying the concept of a deadline.
// Some asynchronous operations are given deadlines by which they must complete.
// Deadlines are enforced by an "actor" that persists for the lifetime of the
// client object:
//
//  +----------------+
//  |                |
//  | check_deadline |<---+
//  |                |    |
//  +----------------+    | async_wait()
//              |         |
//              +---------+
//
// If the deadline actor determines that the deadline has expired, the socket
// is closed and any outstanding operations are consequently cancelled.
//
// Connection establishment involves trying each endpoint in turn until a
// connection is successful, or the available endpoints are exhausted. If the
// deadline actor closes the socket, the connect actor is woken up and moves to
// the next endpoint.
//
//  +---------------+
//  |               |
//  | start_connect |<---+
//  |               |    |
//  +---------------+    |
//           |           |
//  async_-  |    +----------------+
// connect() |    |                |
//           +--->| handle_connect |
//                |                |
//                +----------------+
//                          :
// Once a connection is     :
// made, the connect        :
// actor forks in two -     :
//                          :
// an actor for reading     :       and an actor for
// inbound messages:        :       sending heartbeats:
//                          :
//  +------------+          :          +-------------+
//  |            |<- - - - -+- - - - ->|             |
//  | start_read |                     | start_write |<---+
//  |            |<---+                |             |    |
//  +------------+    |                +-------------+    | async_wait()
//          |         |                        |          |
//  async_- |    +-------------+       async_- |    +--------------+
//   read_- |    |             |       write() |    |              |
//  until() +--->| handle_read |               +--->| handle_write |
//               |             |                    |              |
//               +-------------+                    +--------------+
//
// The input actor reads messages from the socket, where messages are delimited
// by the newline character. The deadline for a complete message is 30 seconds.
//
// The heartbeat actor sends a heartbeat (a message that consists of a single
// newline character) every 10 seconds. In this example, no deadline is applied
// message sending.
//
client::client(boost::asio::io_service& io_service,
               mrv::ViewerUI* v) :
stopped_(false),
deadline_(io_service),
non_empty_output_queue_(io_service),
Parser( io_service, v )
{
   // The non_empty_output_queue_ deadline_timer is set to pos_infin
   // whenever the output queue is empty. This ensures that the output
   // actor stays asleep until a message is put into the queue.
   non_empty_output_queue_.expires_at(boost::posix_time::pos_infin);

}

// Called by the user of the client class to initiate the connection process.
// The endpoint iterator will have been obtained using a tcp::resolver.
void client::start(tcp::resolver::iterator endpoint_iter)
{
   // Start the connect actor.
   start_connect(endpoint_iter);

   //connected = true;

   // Start the deadline actor. You will note that we're not setting any
   // particular deadline here. Instead, the connect and input actors will
   // update the deadline prior to each asynchronous operation.
   deadline_.async_wait(boost::bind(&client::check_deadline, this));
}

// This function terminates all the actors to shut down the connection. It
// may be called by the user of the client class, or by the class itself in
// response to graceful termination or an unrecoverable error.
void client::stop()
{
   connected = false;
   stopped_ = true;
   LOG_CONN( _("Disconnected from ") << socket_.remote_endpoint() );
   boost::system::error_code ignored_ec;
   socket_.close(ignored_ec);
   deadline_.cancel();
   non_empty_output_queue_.cancel();

   if ( ui && ui->uiView )
   {
       Mutex& m = ui->uiView->_clients_mtx;
       SCOPED_LOCK( m );

       ParserList& v = ui->uiView->_clients;
       ParserList::iterator i = v.begin();
       ParserList::iterator e = v.end();

       for ( ; i != e; ++i )
       {
           if ( *i == this )
           {
               LOG_CONN( _("Removed client ") << this );
               v.erase( i );
               break;
           }
       }

       if ( ui->uiConnection )
       {
           ui->uiConnection->uiServerGroup->activate();
           ui->uiConnection->uiConnect->copy_label( _("Connect") );
       }
   }
}


void client::start_connect(tcp::resolver::iterator endpoint_iter)
{
   if (endpoint_iter != tcp::resolver::iterator())
   {
       LOG_CONN( _("Trying ") << endpoint_iter->endpoint() << "..." );

      // Set a deadline for the connect operation.
      deadline_.expires_from_now(boost::posix_time::seconds(60));

      // Start the asynchronous connect operation.
      socket_.async_connect(endpoint_iter->endpoint(),
                            boost::bind(&client::handle_connect,
                                        shared_from_this(), _1, endpoint_iter));
   }
   else
   {
      // There are no more endpoints to try. Shut down the client.
      stop();
   }
}

void client::handle_connect(const boost::system::error_code& ec,
                            tcp::resolver::iterator endpoint_iter)
{
   if (stopped_)
      return;

   // The async_connect() function automatically opens the socket at the start
   // of the asynchronous operation. If the socket is closed at this time then
   // the timeout handler must have run first.
   if (!socket_.is_open())
   {
       // LOG_CONN( "Connect timed out." );

      connected = false;

      // Try the next available endpoint.
      start_connect(++endpoint_iter);
   }

   // Check if the connect operation failed before the deadline expired.
   else if (ec)
   {
       LOG_CONN( "Connect error: " << ec.message() );

      connected = false;

      // We need to close the socket used in the previous connection attempt
      // before starting a new one.
      socket_.close();

      // Try the next available endpoint.
      start_connect(++endpoint_iter);
   }
   // Otherwise we have successfully established a connection.
   else
   {
       LOG_CONN( _("Connected to ") << endpoint_iter->endpoint() );

      connected = true;

      {
          mrv::ImageView* v = ui->uiView;

          Mutex& m = v->_clients_mtx;
          SCOPED_LOCK( m );

          v->_clients.push_back( this );
      }

      ui->uiConnection->uiServerGroup->deactivate();
      ui->uiConnection->uiConnect->copy_label( _("Disconnect") );

      await_output();

      write( N_("sync_image"), "" );

      // Start the input actor.
      start_read();

   }
}


void client::deliver( const std::string& msg )
{
    SCOPED_LOCK( mtx );

    
   output_queue_.push_back(msg + "\n");

   non_empty_output_queue_.expires_from_now(boost::posix_time::seconds(0));
}

void client::start_read()
{
   // DON'T Set a deadline for the read operation.
   // deadline_.expires_from_now(boost::posix_time::seconds(30));
   deadline_.expires_at(boost::posix_time::pos_infin);

   // Start an asynchronous operation to read a newline-delimited message.
   boost::asio::async_read_until(socket_, input_buffer_, '\n',
                                 boost::bind(&client::handle_read,
                                             shared_from_this(),
                                             boost::asio::placeholders::error));
}

void client::handle_read(const boost::system::error_code& ec)
{
    if (stopped_)
        return;

    if (!ec)
    {
        // Extract the newline-delimited message from the buffer.
        std::istream is(&input_buffer_);
        // is.exceptions( std::ifstream::failbit | std::ifstream::badbit |
        //                std::ifstream::eofbit );


        std::string id = boost::lexical_cast<std::string>(socket_.remote_endpoint() );
        try {
            std::string msg;
            is.clear();
            if ( std::getline(is, msg) )
            {
                if ( msg == N_("OK") || msg.empty() )
                {
                    // LOG_CONN("OK");
                }
                else if ( msg == N_("Not OK") )
                {
                    LOG_CONN( N_("Not OK") );
                }
                else if ( parse( msg ) )
                {
                    write( N_("OK"), "" );
                }
                else
                {
                    write( N_("Not OK"), id );
                }
            }
        }
        catch ( const std::ios_base::failure& e )
        {
            LOG_ERROR( _("Parse failure ") << e.what() );
        }
        catch ( const std::exception& e )
        {
            LOG_ERROR( _("Parse std failure ") << e.what() );
        }
        catch ( ... )
        {
            LOG_ERROR( _("Unknown exception thrown") );
        }


        start_read();
    }
    else
    {
        // LOG_CONN( ">>>>>>>>>>>> Error on receive: " << ec.message() );

        stop();
    }
}

void client::await_output()
{

   if (stopped_)
   {
      return;
   }

   if (output_queue_.empty())
   {
      // There are no messages that are ready to be sent. The actor goes to
      // sleep by waiting on the non_empty_output_queue_ timer. When a new
      // message is added, the timer will be modified and the actor will
      // wake.

      non_empty_output_queue_.expires_at(boost::posix_time::pos_infin);
      non_empty_output_queue_.async_wait(
                                         boost::bind(&mrv::client::await_output,
                                                     shared_from_this() )
                                         );
   }
   else
   {
      start_write();
   }
}

void client::start_write()
{

    SCOPED_LOCK( mtx );

   // Start an asynchronous operation to send a message.
   boost::asio::async_write(socket_,
                            boost::asio::buffer(output_queue_.front()),
                            boost::bind(&client::handle_write,
                                        shared_from_this(),
                                        boost::asio::placeholders::error));
}


void client::handle_write(const boost::system::error_code& ec)
{
   if (stopped_)
      return;

   if (!ec)
   {
       std::cerr << "Send " << output_queue_.front();
       output_queue_.pop_front();
       await_output();
   }
   else
   {
       LOG_CONN( _("Error on send: ") << ec.message() );

      stop();
   }
}

void client::check_deadline()
{

   if (stopped_)
      return;


    // Check whether the deadline has passed. We compare the deadline against
    // the current time since a new asynchronous operation may have moved the
    // deadline before this actor had a chance to run.
   if (deadline_.expires_at() <= deadline_timer::traits_type::now())
   {

      // The deadline has passed. The socket is closed so that any outstanding
      // asynchronous operations are cancelled.
      socket_.close();

      // There is no longer an active deadline. The expiry is set to positive
      // infinity so that the actor takes no action until a new deadline is set.
      deadline_.expires_at(boost::posix_time::pos_infin);
   }


   // Put the actor back to sleep.
   deadline_.async_wait(boost::bind(&client::check_deadline, this));
}


void client::create(mrv::ViewerUI* ui)
{
   unsigned short port = (unsigned short) ui->uiConnection->uiClientPort->value();
   ServerData* data = new ServerData;
   data->port = port;
   char buf[128];
   sprintf( buf, "%d", port );
   data->host = ui->uiConnection->uiClientServer->text();
   data->group = buf;
   data->ui = ui;

   if ( ui->uiConnection )
   {
       ui->uiConnection->uiServerGroup->activate();
       ui->uiConnection->uiConnect->copy_label( _("Connect") );
   }
   
   boost::thread t( boost::bind( mrv::client_thread,
                                 data ) );
   t.detach();
}

void client::remove( mrv::ViewerUI* ui )
{
    if ( !ui || !ui->uiView ) return;

    mrv::ImageView* v = ui->uiView;

    Mutex& m = v->_clients_mtx;
    SCOPED_LOCK( m );

   ParserList::iterator i = v->_clients.begin();
   ParserList::iterator e = v->_clients.end();

   for ( ; i != e; ++i )
   {
      (*i)->stop();
   }


   v->_clients.clear();
}


void client_thread( const ServerData* s )
{
   try
   {
    boost::asio::io_service io_service;
    tcp::resolver r(io_service);

    client_ptr c( boost::make_shared<client>( boost::ref(io_service),
                                              boost::ref(s->ui) ) );

    c->start(r.resolve(tcp::resolver::query(s->host, s->group)));

    delete s;

    size_t runs = io_service.run();


   }
   catch (const std::exception& e)
   {
       LOG_ERROR( _("Client Exception: ") << e.what() );
   }
}


} // namespace mrv
