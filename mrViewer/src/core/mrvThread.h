/**
 * @file   mrvThread.h
 * @author gga
 * @date   Sun Aug  5 07:07:23 2007
 * 
 * @brief  Some auxiliary macros for dealing and debugging locks in threads
 * 
 * 
 */

#ifndef mrvThread_h
#define mrvThread_h

// #define DEBUG_MUTEX

#ifdef DEBUG_MUTEX


#define SCOPED_LOCK(x) \
       cerr << "Lock   " << #x << " " << __FILE__ << " " << __LINE__ << endl; \
       Mutex::scoped_lock lk_##x(x);				\
       cerr << "Locked " << #x << " " << __FILE__ << " " << __LINE__ << endl;

#define CONDITION_WAIT( cond, x )  \
       cerr << "Wait   " << #x << " " << __FILE__ << " " << __LINE__ << endl; \
       cond.wait(lk_##x); \
       cerr << "Got    " << #x << " " << __FILE__ << " " << __LINE__ << endl;


#else


#define SCOPED_LOCK(mutex) boost::recursive_mutex::scoped_lock lk_##mutex(mutex)
#define CONDITION_WAIT( cond, mutex )   cond.wait( lk_##mutex );

#endif



#endif // mrvThread_h
