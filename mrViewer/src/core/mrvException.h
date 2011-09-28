/**
 * @file   mrvException.h
 * @author gga
 * @date   Fri Jul 20 00:48:02 2007
 * 
 * @brief  A simple class used to encapsulate mrv exceptions
 * 
 * 
 */

#ifndef mrvException_h
#define mrvException_h

#include <stdexcept>

namespace mrv
{

  class exception : public std::exception
  {
  public:
    exception( const std::string what,
	       const char* file = 0,
	       const unsigned int line = 0) :
      _what(what),
      _line(line)
    {
      if ( file ) _file = file;
    }

    virtual ~exception() throw() {}

    virtual const char* what()  const throw() { return _what.c_str(); }
    virtual const char* file()  const throw() { return _file.c_str(); }
    virtual unsigned int line() const throw() { return _line; }

//     std::ostream& operator<<( std::ostream& o ) const
//     {
//       o << "mrv::exception " 
// 	<< what() << " at " << file() << " " << line() << endl;
//       return o;
//     }

  protected:
    std::string  _what;
    std::string  _file;
    unsigned int _line;
  };

}  // namespace mrv


#ifdef DEBUG
#define EXCEPTION(x) throw mrv::exception((x), __FILE__, __LINE__);
#else
#define EXCEPTION(x) throw mrv::exception((x));
#endif


#endif // mrvException_h
