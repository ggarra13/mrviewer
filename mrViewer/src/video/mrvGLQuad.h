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
 * @file   mrvGLQuad.h
 * @author gga
 * @date   Fri Feb  8 10:12:06 2008
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvGLQuad_h
#define mrvGLQuad_h

namespace mrv {

  class GLLut3d;
  class GLShader;
  class ImageView;


  class GLQuad
  {
  protected:
    struct uvCoords {
      float u;
      float v;
    };

  public:
    GLQuad( const ImageView* view );
    ~GLQuad();

       float gamma() const { return _gamma; }
       void gamma( const float f ) { _gamma = f; }

    void bind( const image_type_ptr& pic );

    void draw( const unsigned dw, const unsigned dh ) const;

    inline GLLut3d* lut() const { return _lut; }
    void lut( const CMedia* img );
    inline void clear_lut() { 
        _lut = NULL; // lut is not deleted here
    } 

       inline void minmax( float fmin, float fmax ) {
	  _normMin = fmin;
	  _normMax = fmax;
       }

  protected:
    void init_texture();

    /// Calculate char step from GL format and pixel type
    int calculate_gl_step( const GLenum format,
			   const GLenum pixel_type ) const;

    void bind_texture_quad( const image_type_ptr& pic,
			    const unsigned poww, const unsigned int powh );

    void bind_texture_yuv( const image_type_ptr& pic,
			   const unsigned int poww, 
			   const unsigned int powh );

    void bind_texture_pixels( const image_type_ptr& pic );

    void draw_pixels( const unsigned dw, const unsigned dh )        const;
    void draw_quad( const unsigned dw, const unsigned dh )          const;
    void draw_frame( const unsigned int dw, const unsigned int dh ) const;
    void draw_field( const unsigned int dw, const unsigned int dh ) const;

  protected:
    static GLenum       gl_pixel_type( const image_type::PixelType type );
    static GLenum       gl_format( const image_type::Format format );
    static unsigned int calculate_pow2( unsigned int w );

    static const char* debug_internal_format( const GLenum format );
    static const char* debug_format( const GLenum format );
    static const char* debug_pixel_type( const GLenum pixel_type );

  protected:
    void update_texsub( unsigned int idx,
			unsigned int rx, unsigned int ry, 
			unsigned int rw, unsigned int rh,
			unsigned int tw, unsigned int th, 
			GLenum format, GLenum pixel_type, 
			unsigned short  channels, 
			unsigned short  pixel_size, 
			boost::uint8_t* pixels );

  protected:
    const ImageView*   _view;
    GLShader*    _shader;
    GLLut3d*     _lut;
    const CMedia*   _image;

    float        _gamma;
    bool         _blend;
    GLenum       _blend_mode;
    unsigned     _num_textures;
    GLuint       _pbo[4];
    GLuint       _texId[4];

    GLenum       _glformat;
    GLenum       _internalFormat;
    GLenum       _pixel_type;

    image_type::Format _format; 
    unsigned int _width;
    unsigned int _height;
    unsigned int _channels;
    uvCoords     _uvMax; 

    float        _yw[3];
    float        _normMin;
    float        _normMax;

    image_type::PixelData _pixels;
  };


} // namespace mrv


#endif // mrvGLQuad_h
