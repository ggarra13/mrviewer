/**
 * @file   mrvBarrier.h
 * @author gga
 * @date   Fri Feb 22 22:48:02 2008
 * 
 * @brief  
 * 
 * 
 */


#ifndef mrvBarrier_h
#define mrvBarrier_h

#include <boost/thread/detail/config.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace mrv {

  class barrier
  {
  public:
    barrier(unsigned int count);
    ~barrier();
    
    bool wait();

    inline unsigned int used() { return m_threshold - m_count; }

    void count( unsigned int c );

    void notify_all();
    
  private:
    boost::mutex m_mutex;
    // disable warnings about non dll import
// see: http://www.boost.org/more/separate_compilation.html#dlls
#ifdef BOOST_MSVC
#   pragma warning(push)
#   pragma warning(disable: 4251 4231 4660 4275)
#endif
    boost::condition m_cond;
#ifdef BOOST_MSVC
#   pragma warning(pop)
#endif
    unsigned int m_threshold;
    unsigned int m_count;
    unsigned int m_generation;
  };

}   // namespace mrv

#endif
