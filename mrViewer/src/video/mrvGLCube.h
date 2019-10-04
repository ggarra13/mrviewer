

#ifndef mrvGLCube_h
#define mrvGLCube_h

#include "mrvGLQuad.h"

namespace mrv {

class GLCube : public GLQuad
{
public:
    GLCube( const ImageView* view );
    ~GLCube();

    virtual void bind( const image_type_ptr pic );

    virtual void draw( const unsigned dw, const unsigned dh ) const;

    void rot_x( double t ) {
        _rotX = t;
    }
    void rot_y( double t ) {
        _rotY = t;
    }

    double rot_x() const {
        return _rotX;
    }
    double rot_y() const {
        return _rotY;
    }

protected:
    void draw_cube( const unsigned dw, const unsigned dh ) const;
protected:
    double _rotX;
    double _rotY;
};

} // namespace mrv

#endif
