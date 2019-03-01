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
/**
 * @file   mrvPacketQueue.h
 * @author gga
 * @date   Sun Jul 15 08:41:02 2007
 *
 * @brief
 *
 *
 */

#ifndef mrvPacketQueue_h
#define mrvPacketQueue_h

#include <cassert>
#include <iostream>
#include <deque>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/cstdint.hpp>


extern "C" {
#include <libavformat/avformat.h>
#define MRV_NOPTS_VALUE (int64_t) AV_NOPTS_VALUE
}



/**
 * assert0() equivalent, that is always enabled.
 */
#define assert0(cond) do {                                            \
    if (!(cond)) {                                                       \
        std::cerr << "Assertion " << AV_STRINGIFY(cond) << " failed at " \
                  << __FILE__ << ":" << __LINE__ << std::endl;           \
        ::abort();                                                      \
    }                                                                    \
} while (0)

//#define DEBUG_PACKET_QUEUE

namespace mrv {

class CMedia;

class PacketQueue
{
private:
    PacketQueue( const PacketQueue& b );

public:
    typedef std::deque< AVPacket >    Packets_t;
    typedef Packets_t::iterator       iterator;
    typedef Packets_t::const_iterator const_iterator;
    typedef Packets_t::reverse_iterator reverse_iterator;
    typedef Packets_t::const_reverse_iterator const_reverse_iterator;
    typedef boost::recursive_mutex    Mutex;
    typedef boost::condition_variable_any Condition;

    static const char* kModule;

    static bool inited;

    inline PacketQueue() : _bytes(0)
    {
    }

    inline Condition& cond()
    {
        return _cond;
    }

    inline Mutex& mutex()
    {
        return _mutex;
    }

    inline Packets_t& queue()
    {
        return _packets;
    }

    inline uint64_t bytes()
    {
        Mutex::scoped_lock lk( _mutex );
        return _bytes;
    }

    inline iterator begin()
    {
        Mutex::scoped_lock lk( _mutex );
        return _packets.begin();
    }

    inline const_iterator begin() const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        return _packets.begin();
    }

    inline reverse_iterator rbegin()
    {
        Mutex::scoped_lock lk( _mutex );
        return _packets.rbegin();
    }

    inline const_reverse_iterator rbegin() const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        return _packets.rbegin();
    }

    inline void clear()
    {
        Mutex::scoped_lock lk( _mutex );
        while ( !_packets.empty() )
        {
            pop_front();
        }

        assert0( _bytes == 0 );
        _bytes = 0;
    }

    inline iterator end()
    {
        Mutex::scoped_lock lk( _mutex );
        return _packets.end();
    }

    inline const_iterator end() const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        return _packets.end();
    }

    inline reverse_iterator rend()
    {
        Mutex::scoped_lock lk( _mutex );
        return _packets.rend();
    }

    inline const_reverse_iterator rend() const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        return _packets.rend();
    }


    inline void push_back( const AVPacket& pkt )
    {
        Mutex::scoped_lock lk( _mutex );

        assert0( pkt.size >= 0 );

        _packets.push_back( pkt );

        if ( pkt.data != _flush.data &&
                pkt.data != _seek.data  &&
                pkt.data != _seek_end.data  &&
                pkt.data != _jump.data &&
                pkt.data != _preroll.data &&
                pkt.data != _loop_start.data &&
                pkt.data != _loop_end.data &&
                pkt.data != NULL )
        {
            // std::cerr << this << " #" << _packets.size()
            // 		<< " push back " << &pkt << " at "
            // 		<< &(_packets.back())
            //                << " start size: " << pkt.size
            // 		<< " bytes " << _bytes
            // 		<< std::endl;
            assert0( pkt.size > 0 );

            _bytes += pkt.size;
            assert0( _bytes > 0 );

            // std::cerr << this << " #" << _packets.size()
            // 		<< " push back end " << std::dec
            // 		<< pkt.dts << " size: " << pkt.size
            // 		<< " bytes " << _bytes
            // 		<< std::endl;
        }

#ifdef DEBUG_PACKET_QUEUE
        std::cerr << "PUSH BACK " << pkt.stream_index
                  << " PTS: " << pkt.pts
                  << " DTS: " << pkt.dts
                  << " DATA: " << (void*)pkt.data
                  << " SIZE: " << pkt.size
                  << " AT: " << &(_packets.back())
                  << " TOTAL " << _bytes
                  << std::endl;
#endif

        _cond.notify_one();

    }

    inline size_t size()
    {
        Mutex::scoped_lock lk( _mutex );
        return _packets.size();
    }

    inline bool empty()
    {
        Mutex::scoped_lock lk( _mutex );
        return _packets.empty();
    }

    inline const AVPacket& front() const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        assert( ! _packets.empty() );
        return _packets.front();
    }

    inline AVPacket& front()
    {
        Mutex::scoped_lock lk( _mutex );
        assert0( ! _packets.empty() );
        return _packets.front();
    }

    inline const AVPacket& back() const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        assert( ! _packets.empty() );
        return _packets.back();
    }

    inline AVPacket& back()
    {
        Mutex::scoped_lock lk( _mutex );
        assert0( ! _packets.empty() );
        return _packets.back();
    }

    inline void erase( iterator it )
    {
        Mutex::scoped_lock lk( _mutex );
        assert0( ! _packets.empty() );

        AVPacket& pkt = *it;

        if ( pkt.data != _flush.data &&
                pkt.data != _seek.data  &&
                pkt.data != _seek_end.data  &&
                pkt.data != _jump.data &&
                pkt.data != _preroll.data &&
                pkt.data != _loop_start.data &&
                pkt.data != _loop_end.data &&
                pkt.data != NULL )
        {
            // std::cerr << this << " #" << _packets.size()
            // 		<< " erase " << &pkt << std::endl;

            assert0( pkt.size >= 0 );
            assert0( _bytes >= pkt.size );

            // std::cerr << std::hex << this << " #"
            // 		<< std::dec << _packets.size()
            // 		<< " erase " << std::dec << pkt.dts
            // 		<< " bytes " << std::dec << _bytes
            // 		<< " - " << std::dec << pkt.size << " = ";

            _bytes -= pkt.size;

            // std::cerr << std::dec << _bytes << std::endl;

            av_packet_unref( &pkt );
        }

        _packets.erase( it );
    }

    inline void pop_front()
    {
        Mutex::scoped_lock lk( _mutex );
        // this is neededd to handle packets in network connections where
        // they may not match
        if ( _packets.empty() ) return;

        AVPacket& pkt = _packets.front();

        if ( pkt.data != _flush.data &&
                pkt.data != _seek.data  &&
                pkt.data != _seek_end.data  &&
                pkt.data != _jump.data &&
                pkt.data != _preroll.data &&
                pkt.data != _loop_start.data &&
                pkt.data != _loop_end.data &&
                pkt.data != NULL && pkt.size != 0 )
        {
#ifdef DEBUG_PACKET_QUEUE
            std::cerr << "POP FRONT " << std::dec << pkt.stream_index
                      << " #: " << _packets.size()
                      << " PTS: " << pkt.pts
                      << " DTS: " << pkt.dts
                      << " SIZE: " << pkt.size
                      << " DATA: " << (void*)pkt.data
                      << " AT: " << std::hex << &(_packets.front())
                      << " TOTAL " << std::dec << _bytes
                      << std::endl;
#endif
            // if ( pkt.size > _bytes )
            //     _bytes = 0;
            // else
            assert0( pkt.size > 0 || pkt.data != NULL );
            assert0( _bytes >= pkt.size );

            // std::cerr << this  << " #"
            // 		<< std::dec << _packets.size()
            // 		<< " pop front " << &pkt << std::endl;

            // std::cerr << std::hex << this << " #"
            // 		<< std::dec << _packets.size()
            // 		<< " pop front " << std::dec << pkt.dts
            // 		<< " bytes " << std::dec << _bytes
            // 		<< " - " << std::dec << pkt.size << " = ";

            _bytes -= pkt.size;

            // std::cerr << std::dec << _bytes << std::endl;

            av_packet_unref( &pkt );
        }

        _packets.pop_front();
    }


    inline
    friend std::ostream& operator<<( std::ostream& o, PacketQueue& b )
    {
        Mutex::scoped_lock lk( b.mutex() );
        PacketQueue::const_iterator i = b.begin();
        PacketQueue::const_iterator e = b.end();
        o << " (size " << b._packets.size() << " ) ";
        for ( ; i != e; ++i )
        {
            o << " " << (*i).dts;
        }
        return o;
    }

    bool is_flush()
    {
        Mutex::scoped_lock lk( _mutex );
        if ( _packets.empty() ) return false;
        if ( _packets.front().data == _flush.data ) return true;
        return false;
    }

    bool is_flush(const AVPacket& pkt) const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        if ( pkt.data == _flush.data ) return true;
        return false;
    }

    bool is_seek()
    {
        Mutex::scoped_lock lk( _mutex );
        if ( _packets.empty() ) return false;
        if ( _packets.front().data == _seek.data ) return true;
        return false;
    }

    bool is_jump()
    {
        Mutex::scoped_lock lk( _mutex );
        if ( _packets.empty() ) return false;
        if ( _packets.front().data == _jump.data ) return true;
        return false;
    }

    bool is_preroll()
    {
        Mutex::scoped_lock lk( _mutex );
        if ( _packets.empty() ) return false;
        if ( _packets.front().data == _preroll.data ) return true;
        return false;
    }

    bool is_seek_end()
    {
        Mutex::scoped_lock lk( _mutex );
        if ( _packets.empty() ) return false;
        if ( _packets.front().data == _seek_end.data ) return true;
        return false;
    }

    bool is_seek(const AVPacket& pkt) const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        if ( pkt.data == _seek.data ) return true;
        return false;
    }

    bool is_jump(const AVPacket& pkt) const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        if ( pkt.data == _jump.data ) return true;
        return false;
    }

    bool is_preroll(const AVPacket& pkt) const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        if ( pkt.data == _preroll.data ) return true;
        return false;
    }

    bool is_seek_end(const AVPacket& pkt) const
    {
        Mutex& m = const_cast< Mutex& >( _mutex );
        Mutex::scoped_lock lk( m );
        if ( pkt.data == _seek_end.data ) return true;
        return false;
    }

    bool is_loop_start()
    {
        Mutex::scoped_lock lk( _mutex );
        if ( _packets.empty() ) return false;
        if ( _packets.front().data == _loop_start.data ) return true;
        return false;
    }

    bool is_loop_start(const AVPacket& pkt) const
    {
        if ( pkt.data == _loop_start.data ) return true;
        return false;
    }


    bool is_loop_end()
    {
        Mutex::scoped_lock lk( _mutex );
        if ( _packets.empty() ) return false;
        if ( _packets.front().data == _loop_end.data ) return true;
        return false;
    }

    bool is_loop_end(const AVPacket& pkt) const
    {
        if ( pkt.data == _loop_end.data ) return true;
        return false;
    }

    void flush(const int64_t pts)
    {
        Mutex::scoped_lock lk( _mutex );
        _packets.push_back( _flush );
        AVPacket& pkt = _packets.back();
        pkt.dts = pkt.pts = pts;
        _cond.notify_one();
    }

    void jump(const int64_t pts)
    {
        Mutex::scoped_lock lk( _mutex );
        _packets.push_back( _jump );
        AVPacket& pkt = _packets.back();
        pkt.dts = pkt.pts = pts;
        _cond.notify_one();
    }

    void preroll(const int64_t pts)
    {
        Mutex::scoped_lock lk( _mutex );
        flush(pts);
        _packets.push_back( _preroll );
        AVPacket& pkt = _packets.back();
        pkt.dts = pkt.pts = pts;
        _cond.notify_one();
    }

    void loop_at_start(const int64_t frame)
    {
        Mutex::scoped_lock lk( _mutex );
        _packets.push_back( _loop_start );
        AVPacket& pkt = _packets.back();
        pkt.dts = pkt.pts = frame;
        _cond.notify_one();
    }

    void loop_at_end(const int64_t frame)
    {
        Mutex::scoped_lock lk( _mutex );
        push_back( _loop_end );
        AVPacket& pkt = _packets.back();
        pkt.dts = pkt.pts = frame;
        _cond.notify_one();
    }

    void seek_begin(const int64_t pts)
    {
        Mutex::scoped_lock lk( _mutex );
        flush(pts);
        _packets.push_back( _seek );
        AVPacket& pkt = _packets.back();
        pkt.dts = pkt.pts = pts;
        _cond.notify_one();
    }

    void seek_end(const int64_t pts)
    {
        Mutex::scoped_lock lk( _mutex );
        _packets.push_back( _seek_end );
        AVPacket& pkt = _packets.back();
        pkt.dts = pkt.pts = pts;
        _cond.notify_one();
    }

    static void initialize()
    {
        av_init_packet( &_flush );
        _flush.data = (uint8_t*)strdup("FLUSH");
        _flush.size = 0;
        av_init_packet( &_seek );
        _seek.data  = (uint8_t*)strdup("SEEK");
        _seek.size  = 0;
        av_init_packet( &_jump );
        _jump.data = (uint8_t*)strdup("JUMP");
        _jump.size = 0;
        av_init_packet( &_preroll );
        _preroll.data = (uint8_t*)strdup("PREROLL");
        _preroll.size = 0;
        av_init_packet( &_seek_end );
        _seek_end.data = (uint8_t*)strdup("SEEK END");
        _seek_end.size = 0;
        av_init_packet( &_loop_start );
        _loop_start.data = (uint8_t*)strdup("LOOP START");
        _loop_start.size = 0;
        av_init_packet( &_loop_end );
        _loop_end.data = (uint8_t*)strdup("LOOP END");
        _loop_end.size = 0;

        inited = true;
    }

    static void release()
    {
        free( _loop_end.data );
        free( _loop_start.data );
        free( _seek_end.data );
        free( _preroll.data );
        free( _jump.data );
        free( _seek.data );
        free( _flush.data );
    }


protected:
    uint64_t      _bytes;
    Packets_t    _packets;
    mutable Mutex  _mutex;
    Condition       _cond;

    static AVPacket _flush;      // special packet used to flush buffers
    static AVPacket _seek;       // special packet used to mark seeks to skip
    static AVPacket _jump;       // special packet used to mark jumps
    static AVPacket _preroll;    // special packet used to mark preroll seeks
    static AVPacket _seek_end;   // special packet used to mark seek/preroll 
                                 // endings (used when playing backwards)
    static AVPacket _loop_end;   // special packet to mark loops in playback
    static AVPacket _loop_start; // special packet to mark loops in reverse playback
};


} // namespace mrv


#endif
