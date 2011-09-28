/**
 * @file   mrvRectangle.h
 * @author gga
 * @date   Mon Oct 23 13:36:30 2006
 * 
 * @brief  A simple rectangle containing double coordinates
 * 
 * 
 */

#ifndef mrvRectangle_h
#define mrvRectangle_h

#include <iostream>


namespace mrv
{
  template< typename T >
  class Rectangle
  {
    T x_, y_, w_, h_;

  public:
    inline Rectangle() : x_(0), y_(0), w_(0), h_(0) {}

    inline Rectangle( T x, T y, T w, T h ) :
      x_(x), y_(y), w_(w), h_(h) {}

    inline Rectangle( T w, T h ) :
      x_(0), y_(0), w_(w), h_(h) {}

    inline Rectangle( const Rectangle< T >& r ) :
      x_(r.x()), y_(r.y()), w_(r.w()), h_(r.h()) {}

    inline T x() const { return x_; }
    inline T y() const { return y_; }

    inline T w() const { return w_; }
    inline T h() const { return h_; }

    inline T t() const { return y_; }
    inline T l() const { return x_; }

    inline T b() const { return y_ + h_; }
    inline T r() const { return x_ + w_; }

    inline void merge( const Rectangle< T >& b )
    {
      if ( b.x_ < x_ ) x_ = b.x_;
      if ( b.y_ < y_ ) y_ = b.y_;

      if ( b.w_ > w_ ) w_ = b.w_;
      if ( b.h_ > h_ ) h_ = b.h_;
    }


    inline
    friend std::ostream& operator<<( std::ostream& o, const Rectangle< T >& r )
    {
      return o << '(' << r.x() << ", " << r.y() << '-'
	       << r.r() << ", " << r.b() << ')';
    }

  };


  typedef Rectangle< double > Rectd;
  typedef Rectangle< float  > Rectf;
  typedef Rectangle< int    > Recti;


} // namespace mrv

#endif // mrvRectangle_h
