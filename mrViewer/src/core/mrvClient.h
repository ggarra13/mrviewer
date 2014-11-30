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
#ifndef mrvClient_h
#define mrvClient_h


#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include <mrvServer.h>

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

namespace mrv {

class ViewerUI;

class client : public Parser
{
   public:
     client(boost::asio::io_service& io_service,
	    mrv::ViewerUI* v); 

     void start(tcp::resolver::iterator endpoint_iter);

     virtual void stop();
     virtual void deliver( std::string m );

     bool stopped() { return stopped_; }
     void start_connect(tcp::resolver::iterator endpoint_iter);
     void handle_connect(const boost::system::error_code& ec,
			 tcp::resolver::iterator endpoint_iter);
     void start_read();
     void handle_read( const boost::system::error_code& ec );
     void start_write( const std::string& s );
     void handle_write( const boost::system::error_code& ec );
     void check_deadline();

     static void create( mrv::ViewerUI* main );
     static void remove( mrv::ViewerUI* main );

private:
  bool stopped_;
  boost::asio::streambuf input_buffer_;
  deadline_timer deadline_;
};

void client_thread( const ServerData* s );

} // namespace mrv

#endif
