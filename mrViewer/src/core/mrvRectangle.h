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

      inline void x( T x ) { x_ = x; }
      inline void y( T x ) { y_ = x; }
      inline void w( T x ) { w_ = x; }
      inline void h( T x ) { h_ = x; }

    inline T x() const { return x_; }
    inline T y() const { return y_; }

    inline T w() const { return w_; }
    inline T h() const { return h_; }

    inline T t() const { return y_; }
    inline T l() const { return x_; }

    inline T b() const { return y_ + h_ - 1; }
    inline T r() const { return x_ + w_ - 1; }

    /*! Change x() without changing r(), by changing the width. */
    void set_x(T v) {w_ -= v-x_; x_ = v;}
    /*! Change y() without changing b(), by changing the height. */
    void set_y(T v) {h_ -= v-y_; y_ = v;}
    /*! Change r() without changing x(), by changine the width. */
    void set_r(T v) {w_ = v-x_;}
    /*! Change b() without changing y(), by changine the height. */
    void set_b(T v) {h_ = v-y_;}

    inline void merge( const Rectangle< T >& R )
    {
        if (R.w() == 0) return;
        if ( w() == 0 ) return;
        
        if (R.x() < x()) set_x(R.x());
        if (R.r() > r()) set_r(R.r());
        if (R.y() < y()) set_y(R.y());
        if (R.b() > b()) set_b(R.b());
    }

      inline bool operator==( const Rectangle< T >& b ) const
      {
          return ( b.x_ == x_ && b.y_ == y_ && b.w_ == w_ && b.h_ == h_ );
      }

      inline bool operator!=( const Rectangle< T >& b ) const
      {
          return ( b.x_ != x_ || b.y_ != y_ || b.w_ != w_ || b.h_ != h_ );
      }

    inline
    friend std::ostream& operator<<( std::ostream& o, const Rectangle< T >& r )
    {
      return o << '(' << r.l() << ',' << r.t() << " - "
	       << r.r() << ',' << r.b() << ") [" << r.w() << "-" << r.h()
               << ']';
    }

  };


  typedef Rectangle< double > Rectd;
  typedef Rectangle< float  > Rectf;
  typedef Rectangle< int    > Recti;


} // namespace mrv

#endif // mrvRectangle_h
