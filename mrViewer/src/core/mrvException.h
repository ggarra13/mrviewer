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
