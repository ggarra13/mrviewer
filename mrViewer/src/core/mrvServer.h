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
#ifndef mrvServer_h
#define mrvServer_h

#include <deque>

#include <boost/thread/recursive_mutex.hpp>
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

#include "gui/mrvReel.h"

namespace mrv {

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

class ViewerUI;
class Parser;
class ImageView;
class ImageBrowser;
class EDLGroup;


class Parser
{
public:
    typedef boost::recursive_mutex Mutex;

public:
    Parser( boost::asio::io_service& io_service, mrv::ViewerUI* v );
    virtual ~Parser();

    bool parse( const std::string& m );
    void write( const std::string& s, const std::string& id );

    mrv::ImageView* view() const;
    mrv::ImageBrowser* browser() const;
    mrv::EDLGroup*     edl_group() const;

    virtual void deliver( const std::string& m ) = 0;
    virtual void stop() {};


public:
    bool connected;
    tcp::socket socket_;
    Mutex mtx;
    mrv::ViewerUI* ui;
};


class tcp_session : public Parser,
    public boost::enable_shared_from_this< tcp_session >
{
public:
    tcp_session(boost::asio::io_service& io_service,
                mrv::ViewerUI* const v);
    virtual ~tcp_session();

    tcp::socket& socket();
    void start();

    bool stopped();

    void start_read();
    void handle_read(const boost::system::error_code& ec);
    void await_output();

    virtual void deliver( const std::string& m );

    virtual void stop();

    void start_write();
    void handle_write(const boost::system::error_code& ec);
    void check_deadline(deadline_timer* deadline);

protected:
    boost::asio::streambuf input_buffer_;
    deadline_timer non_empty_output_queue_;
    std::deque< std::string > output_queue_;
};

typedef boost::shared_ptr<tcp_session> tcp_session_ptr;

class server
{
public:
    server(boost::asio::io_service& io_service,
           const tcp::endpoint& listen_endpoint,
           mrv::ViewerUI* v);

    ~server();

    void start_accept();

    void handle_accept(tcp_session_ptr session,
                       const boost::system::error_code& ec);

    static void create(mrv::ViewerUI* ui);
    static void remove(mrv::ViewerUI* ui);

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    mrv::ViewerUI* ui_;
};

typedef std::vector< Parser* > ParserList;
typedef boost::shared_ptr<server> tcp_server_ptr;

struct ServerData
{
    std::string host;
    std::string group;
    unsigned short port;
    mrv::ViewerUI* ui;
};

void server_thread( const ServerData* s );


}  // namespace mrv

#endif

