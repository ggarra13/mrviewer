/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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

class ViewerUI;

namespace mrv {


class client : public Parser,
               public boost::enable_shared_from_this< client >
{
  public:
    client(boost::asio::io_service& io_service,
           ViewerUI* v); 
    virtual ~client() { stop(); };

    void start(tcp::resolver::iterator endpoint_iter);

    virtual void stop();
    virtual void deliver( const std::string& msg );

    bool stopped() { return stopped_; }

    void start_connect(tcp::resolver::iterator endpoint_iter);
    void handle_connect(const boost::system::error_code& ec,
                        tcp::resolver::iterator endpoint_iter);

    void start_read();
    void handle_read( const boost::system::error_code& ec );

    void await_output();
    void start_write();
    void handle_write( const boost::system::error_code& ec );

    void check_deadline();

    static void create( ViewerUI* main );
    static void remove( ViewerUI* main );

  private:
    bool stopped_;
    deadline_timer non_empty_output_queue_;
    std::deque< std::string > output_queue_;
    boost::asio::streambuf input_buffer_;
    deadline_timer deadline_;
};

typedef boost::shared_ptr< client > client_ptr;
typedef std::vector< client* >      ClientList;

void client_thread( const ServerData* s );

} // namespace mrv

#endif
