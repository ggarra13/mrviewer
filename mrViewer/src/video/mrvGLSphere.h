

#ifndef mrvGLSphere_h
#define mrvGLSphere_h

#include "mrvGLQuad.h"

namespace mrv {

class GLSphere : public GLQuad
{
  public:
    GLSphere( const ImageView* view );
    ~GLSphere();

    virtual void bind( const image_type_ptr& pic );
    
    virtual void draw( const unsigned dw, const unsigned dh ) const;

    void rot_x( double t ) { _rotX = t; }
    void rot_y( double t ) { _rotY = t; }
    
    double rot_x() const { return _rotX; }
    double rot_y() const { return _rotY; }
    
  protected:
    void draw_sphere( const unsigned dw, const unsigned dh ) const;
  protected:
    GLUquadric* qObj;
    double _rotX;
    double _rotY;
};

} // namespace mrv

#endif
