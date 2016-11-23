

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

  protected:
    void draw_sphere( const unsigned dw, const unsigned dh ) const;
  protected:
    GLUquadric* qObj;
    double _rotX;
    double _rotY;
};

} // namespace mrv

#endif
