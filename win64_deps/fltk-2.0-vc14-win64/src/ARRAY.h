/*! \macro ARRAY(type, name, n)
  Makes a variable-sized local array. In gcc this turns into
  "type name[size]". In other systems this turns into calls to alloca() or
  into C++ hacks with destructors to get the same result.

  Warning: most systems that support multithreading make the stack very
  small for all threads except the "main" one, and the alloca solutions
  can crash these threads.
*/

#ifndef fl_ALLOC_h
#define fl_ALLOC_h
#undef ARRAY

#ifndef _COMPILER_VERSION
#define _COMPILER_VERSION 0
#endif

// Systems with compiler support for dynamic local arrays:
#if defined(__GNUC__) || defined(__sgi) && _COMPILER_VERSION<730
# define ARRAY(type, name, size) type name[size]

// Win32 named alloca "_alloca" but it does not work when multithreaded:
//#elif defined(_WIN32)
//# include <malloc.h>
//# define ARRAY(type, name, size) type*const name = (type*)_alloca(size*sizeof(type))

// Systems with alloca():
#elif defined(__DECCXX) || defined(__sgi)
# include <alloca.h>
# define ARRAY(type, name, size) type*const name = (type*)alloca(size*sizeof(type))

// Use C++ to make the array:
#else
template <class T> class Alloca {
  T* p;
public:
  Alloca(int size) {p = new T[size];}
  operator T*() {return p;}
  ~Alloca() {delete [] p;}
};
#define ARRAY(type, name, size) Alloca<type> name(size)

#endif
#endif

