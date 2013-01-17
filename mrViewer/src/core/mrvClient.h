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
     bool stopped() { return stopped_; }
     void start_connect(tcp::resolver::iterator endpoint_iter);
     void handle_connect(const boost::system::error_code& ec,
			 tcp::resolver::iterator endpoint_iter);
     void start_read();
     void handle_read( const boost::system::error_code& ec);
     void start_write( const std::string& s );
     void handle_write( const boost::system::error_code& ec);
     void check_deadline();
     void deliver( std::string m );

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
