/**
 * @file   mrvGLShader.h
 * @author gga
 * @date   Fri Jan 18 22:42:38 2008
 * 
 * @brief  
 * 
 * 
 */

#include <GL/glew.h>

#include <map>


namespace mrv {

  class GLShader
  {
  public:
    GLShader();
    GLShader( const char* filename );
    ~GLShader();

    GLhandleARB program()         { return _program;  }
    GLuint      fragment_shader() { return _frag_shader; }
    GLuint      vertex_shader()   { return _vtx_shader; }

    void bind();
    void enable();
    void disable();

    void load( const char* filename );
    void load( const char* filename, const char* code );

    void setTextureUnit( const char* uniform, const int unit );

    void setUniform( const GLint location, const float x );
    void setUniform( const GLint location, const float x, const float y );
    void setUniform( const GLint location, const float x, const float y, 
		     const float z );
    void setUniform( const GLint location, const float x, const float y, 
		     const float z, const float w );

    void setUniform( const char* uniform, const float x );
    void setUniform( const char* uniform, const float x, const float y);
    void setUniform( const char* uniform, const float x, const float y, 
		     const float z );
    void setUniform( const char* uniform, const float x, const float y, 
		     const float z, const float w );

    void setUniform( const GLint location, const int x );
    void setUniform( const GLint location, const int x, const int y );
    void setUniform( const GLint location, const int x, const int y, 
		     const int z );
    void setUniform( const GLint location, const int x, const int y, 
		     const int z, const int w );

    void setUniform( const char* uniform, const int x );
    void setUniform( const char* uniform, const int x, const int y);
    void setUniform( const char* uniform, const int x, const int y, 
		     const int z );
    void setUniform( const char* uniform, const int x, const int y, 
		     const int z, const int w );

  protected:

    void  store_uniforms( const char* arbfp1_code );
    GLint uniform_location( const char* uniform );

    const char* get_error( const char* data, int pos );
    const char* get_glsl_error();
    const char* get_shader_error(GLuint shader);
    const char* get_program_error(GLuint program);

  protected:
    GLuint      _frag_target;
    GLuint      _frag_shader;
    GLuint      _vtx_target;
    GLuint      _vtx_shader;

    GLhandleARB _program;

    typedef std::map< std::string, GLint > UniformMap;
    UniformMap _uniforms;
  };
}
