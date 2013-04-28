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

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/cstdint.hpp>


extern "C" {
#include <libavformat/avformat.h>
#define MRV_NOPTS_VALUE (int64_t) AV_NOPTS_VALUE
}

#include <deque>


// #define DEBUG_PACKET_QUEUE

namespace mrv {

  class CMedia;

  class PacketQueue
  {
  public:
    typedef std::deque< AVPacket >    Packets_t;
    typedef Packets_t::iterator       iterator;
    typedef Packets_t::const_iterator const_iterator;
    typedef boost::recursive_mutex    Mutex;
    typedef boost::condition          Condition;

    static const char* kModule;
 

    PacketQueue() : _bytes(0)
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

    inline unsigned int bytes() const
    {
      return _bytes;
    }

    inline iterator begin()
    {
      Mutex::scoped_lock lk( _mutex );
      return _packets.begin();
    }

    inline const_iterator begin() const
    {
      return _packets.begin();
    }

    inline void clear()
    {
      Mutex::scoped_lock lk( _mutex );
      while ( !_packets.empty() )
	{
	  pop_front();
	}

      assert( _bytes == 0 );
      _bytes = 0;
    }

    inline iterator end()
    {
      Mutex::scoped_lock lk( _mutex );
      return _packets.end();
    }

    inline const_iterator end() const
    {
      return _packets.end();
    }

    inline void push_back( AVPacket& pkt )
    {
      Mutex::scoped_lock lk( _mutex );
      assert( pkt.dts != MRV_NOPTS_VALUE );

      if ( av_dup_packet(&pkt) < 0 )
      {
	 std::cerr << "Could not duplicate packet - not added" << std::endl;
	 return;
      }

      _packets.push_back( pkt );
      _bytes += pkt.size;

      _cond.notify_one();

#ifdef DEBUG_PACKET_QUEUE
      std::cerr << "PUSH BACK " << pkt.stream_index 
		<< " PTS: " << pkt.pts
		<< " DTS: " << pkt.dts
		<< " DATA: " << (void*)pkt.data
		<< " SIZE: " << pkt.size
#if 0
		<< " AT: " << &(_packets.back()) 
		<< " TOTAL " << _bytes
#endif	
		<< std::endl;
#endif

    }

    inline size_t size() const
    {
      return _packets.size();
    }

    inline bool empty() const
    {
      return _packets.empty();
    }

    inline const AVPacket& front() const
    {
      assert( ! _packets.empty() );
      return _packets.front();
    }

    inline AVPacket& front()
    {
      Mutex::scoped_lock lk( _mutex );
      assert( ! _packets.empty() );
      return _packets.front();
    }

    inline const AVPacket& back() const
    {
      assert( ! _packets.empty() );
      return _packets.back();
    }

    inline AVPacket& back()
    {
      Mutex::scoped_lock lk( _mutex );
      assert( ! _packets.empty() );
      return _packets.back();
    }

    inline void pop_front()
    {
      Mutex::scoped_lock lk( _mutex );
      assert( ! _packets.empty() );

      AVPacket& pkt = _packets.front();

      if ( pkt.data != _flush.data &&
	   pkt.data != _seek.data  &&
	   pkt.data != _loop_start.data &&
	   pkt.data != _preroll.data &&
	   pkt.data != _loop_end.data )
	{
	  assert( _bytes >= (unsigned)pkt.size );
	  _bytes -= pkt.size;

#ifdef DEBUG_PACKET_QUEUE
	  std::cerr << "POP FRONT " << pkt.stream_index
		    << " PTS: " << pkt.pts
		    << " DTS: " << pkt.dts  
		    << " SIZE: " << pkt.size
		    << " DATA: " << (void*)pkt.data
#if 0
		    << " AT: " << &(_packets.front()) 
		    << " TOTAL " << _bytes
#endif 
		    << std::endl;
	
#endif
	  av_free_packet( &pkt );
	  pkt.data = NULL;
	  pkt.size = 0;
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

    bool is_preroll()
    {
      Mutex::scoped_lock lk( _mutex );
      if ( _packets.empty() ) return false;
      if ( _packets.front().data == _preroll.data ) return true;
      return false;
    }

    bool is_seek(const AVPacket& pkt) const
    {
      if ( pkt.data == _seek.data ) return true;
      return false;
    }

    bool is_preroll(const AVPacket& pkt) const
    {
      if ( pkt.data == _preroll.data ) return true;
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
    }

    void preroll(const int64_t pts)
    {
      Mutex::scoped_lock lk( _mutex );
      flush(pts);
      _packets.push_back( _preroll );
      AVPacket& pkt = _packets.back();
      pkt.dts = pkt.pts = pts;
    }

    void loop_at_start(const int64_t frame)
    {
      Mutex::scoped_lock lk( _mutex );
      _packets.push_back( _loop_start );
      AVPacket& pkt = _packets.back();
      pkt.dts = pkt.pts = frame;
    }

    void loop_at_end(const int64_t frame)
    {
      Mutex::scoped_lock lk( _mutex );
      _packets.push_back( _loop_end );
      AVPacket& pkt = _packets.back();
      pkt.dts = pkt.pts = frame;
    }

    void seek_begin(const int64_t pts)
    {
      Mutex::scoped_lock lk( _mutex );
      flush(pts);
      _packets.push_back( _seek );
      AVPacket& pkt = _packets.back();
      pkt.dts = pkt.pts = pts;
    }

    void seek_end(const int64_t pts)
    {
      Mutex::scoped_lock lk( _mutex );
      _packets.push_back( _seek );
      AVPacket& pkt = _packets.back();
      pkt.dts = pkt.pts = pts;
    }

    static void initialize()
    {
      av_init_packet( &_flush );
      _flush.data = (uint8_t*)"FLUSH";
      _flush.size = 0;
      av_init_packet( &_seek );
      _seek.data  = (uint8_t*)"SEEK";
      _seek.size  = 0;
      av_init_packet( &_preroll );
      _preroll.data  = (uint8_t*)"PREROLL";
      _preroll.size  = 0;
      av_init_packet( &_loop_start );
      _loop_start.data  = (uint8_t*)"LOOP START";
      _loop_start.size  = 0;
      av_init_packet( &_loop_end );
      _loop_end.data  = (uint8_t*)"LOOP END";
      _loop_end.size  = 0;
    }

    static void release()
    {
    }


  protected:
    unsigned int _bytes;
    Packets_t    _packets;
    Mutex        _mutex;
    Condition    _cond;

    static AVPacket _flush;      // special packet used to flush buffers
    static AVPacket _seek;       // special packet used to mark seeks to skip
                                 // intermediate I/B/P frames
    static AVPacket _preroll;    // special packet used to mark preroll seeks
                                 // (used when playing backwards)
    static AVPacket _loop_end;   // special packet to mark loops in playback
    static AVPacket _loop_start; // special packet to mark loops in reverse playback
  };


} // namespace mrv


#endif
