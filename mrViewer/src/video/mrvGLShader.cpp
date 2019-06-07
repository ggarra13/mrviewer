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
 * @file   mrvGLShader.cpp
 * @author gga
 * @date   Fri Jan 18 22:44:59 2008
 *
 * @brief
 *
 *
 */

#include "core/mrvFrame.h"

#include <iostream>
#include <Iex.h>

#include <FL/Fl.H>

#include "core/mrvI8N.h"
#include "mrvGLShader.h"
#include "mrvGLEngine.h"

namespace {
  const char* kModule = "shader";
}

namespace mrv {


  GLShader::GLShader() :
    _frag_target( 0 ),
    _frag_shader( 0 ),
    _program( 0 )
  {
  }

  GLShader::GLShader( const char* filename ) :
    _frag_target( 0 ),
    _frag_shader( 0 ),
    _program( 0 )
  {
    load( filename );
  }

  const char* GLShader::get_error(const char *data, int pos)
  {
    char *s = (char*) data;
    if ( !s ) return N_("");
    while(*s && pos--) s++;
    while(s >= data && *s != '\n') s--;
    char *e = ++s;
    while(*e != '\0' && *e != '\n') e++;
    *e = '\0';
    return s;
  }

  const char* GLShader::get_glsl_error()
  {
    GLint length;
    static char error[4096];
    error[4095] = 0;
    glGetInfoLogARB(_program, 4095, &length, error);
    return error;
  }

  const char* GLShader::get_shader_error(GLuint shader)
  {
    GLint length;
    static char error[4096];
    error[4095] = 0;
    glGetShaderInfoLog(shader, 4095, &length, error);
    return error;
  }

  const char* GLShader::get_program_error(GLuint program)
  {
    GLint length;
    static char error[4096];
    error[4095] = 0;
    glGetProgramInfoLog(program, 4095, &length, error);
    return error;
  }

  void GLShader::store_uniforms( const char* code )
  {
     const char* var = strstr(code, "#var ");
    while( var ) {
      const char* eol = strchr( var + 1, '#');
      const char* c2,* c3,* c4;
      c2 = strchr( var + 6, ' ');
      if( c2 ) {
        c3 = strchr( c2 + 1, ':');
        if( c3 ) {
          c4 = strchr( c3 + 1, ':');
          if( c4 )
            c4 = strchr( c4 + 1, '[');
          if( c4 && c4 < eol) {
            char type[10], name[30];
            strncpy( type, var + 5, c2-var-5 );
            type[c2-var-5] = 0;
            strncpy( name, c2 + 1, c3-c2-2 );
            name[c3-c2-2] = 0;

            GLint location = atoi( c4 + 1);
            _uniforms[ name ] = location;
          }
        }
      }
      var = strstr(var + 1, "#var ");
    }
  }

  GLint GLShader::uniform_location( const char* uniform )
  {
    UniformMap::const_iterator i = _uniforms.find( uniform );
    if ( i == _uniforms.end() ) return -1;
    return i->second;
  }

  void GLShader::load( const char* filename )
  {
     FILE* f = fl_fopen( filename, "rb" );
     if (!f)
     {
        THROW_ERRNO ("Can't load shader file '" << filename <<
                     "' (%T)");
      }

    fseek( f, 0, SEEK_END );
    size_t len = ftell (f);
    fseek( f, 0, SEEK_SET );

    char* code = new char[len + 1];

    size_t read = fread( code, 1, len, f );
    if (read != len)
      {
        fclose(f);
        THROW (Iex::BaseExc, "Expected " << len << " bytes in fragment " <<
               "shader file " << filename << ", only got " << read);
      }

    code[read] = '\0';    // null-terminate
    fclose(f);

    load( filename, code );

    delete [] code;
  }

  void GLShader::load( const char* filename, const char* code )
  {
    GLint len = (GLint) strlen( code );

    if( GLEW_ARB_fragment_program && !strncmp(code,"!!ARBfp1.0",10))
      {
        // ARB fragment program
        _frag_target = GL_FRAGMENT_PROGRAM_ARB;
        glGenProgramsARB( 1, &_frag_shader );
        glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, _frag_shader );
        glProgramStringARB( GL_FRAGMENT_PROGRAM_ARB,
                            GL_PROGRAM_FORMAT_ASCII_ARB,
                           (GLsizei)len, code );
        GLint pos = -1;
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB,&pos);
        if(pos != -1) {
          THROW (Iex::BaseExc, "GLShader::load(): fragment program error in "
                 << filename << " file\n" << get_error(code, pos));
        }

        store_uniforms( code );
      }
    else
      {
        _program = glCreateProgramObjectARB();

        GLhandleARB shader = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );

        glShaderSource( shader, 1, (const GLcharARB**)&code, &len );
        glCompileShaderARB( shader );

        GLint bDidCompile;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &bDidCompile);

        if ( !bDidCompile )
        {
           delete [] code;
           THROW(Iex::BaseExc, _("Shader did not compile. Error: ") <<
                 get_shader_error( shader ) );
        }

        glAttachObjectARB( _program, shader );

        glDeleteObjectARB( shader );
        glLinkProgramARB( _program );

        GLint bDidLink;
        glGetProgramiv( _program, GL_LINK_STATUS, &bDidLink);
        if ( !bDidLink )
        {
           glDeleteObjectARB( _program );
           delete [] code;
           THROW( Iex::BaseExc, _("Program did not link. Error: ")
                  << get_program_error( _program ) );
        }

        GLint ok;
        glGetObjectParameterivARB( _program, GL_OBJECT_LINK_STATUS_ARB, &ok );
        if (!ok) {
          delete [] code;
          glDeleteObjectARB( _program );

          THROW (Iex::BaseExc, _("GLSL error in file ")
                 << filename << ": " << get_glsl_error() );

        }

      }
  }


  void GLShader::setUniform( const GLint location, const float x  )
  {
    glUniform1f( location, x );
    CHECK_GL;
  }

  void GLShader::setUniform( const GLint location,
                             const float x, const float y )
  {
    glUniform2f( location, x, y );
    CHECK_GL;
  }

  void GLShader::setUniform( const GLint location,
                             const float x, const float y, const float z )
  {
    glUniform3f( location, x, y, z );
    CHECK_GL;
  }


  void GLShader::setUniform( const GLint location,
                             const float x, const float y,
                             const float z, const float w  )
  {
    glUniform4f( location, x, y, z, w );
    CHECK_GL;
  }

  void GLShader::setUniform( const char* uniform,
                             const float x )
  {
    if ( _program )
      {
        GLint location = glGetUniformLocationARB( _program, uniform );
        CHECK_GL;
        setUniform( location, x );
      }
    else if ( _frag_target == GL_FRAGMENT_PROGRAM_ARB )
      {
        GLint location = uniform_location( uniform );
        CHECK_GL;
        glProgramLocalParameter4fARB( _frag_target, location, x, 0, 0, 0);
        CHECK_GL;
      }
  }

  void GLShader::setUniform( const char* uniform,
                             const float x, const float y )
  {
    if ( _program )
      {
        GLint location = glGetUniformLocationARB( _program, uniform );
        CHECK_GL;
        setUniform( location, x, y );
      }
    else if ( _frag_target == GL_FRAGMENT_PROGRAM_ARB )
      {
        GLint location = uniform_location( uniform );
        CHECK_GL;
        glProgramLocalParameter4fARB( _frag_target, location, x, y, 0, 0);
        CHECK_GL;
      }
  }

  void GLShader::setUniform( const char* uniform,
                             const float x, const float y,
                             const float z )
  {
    if ( _program )
      {
        GLint location = glGetUniformLocationARB( _program, uniform );
        CHECK_GL;
        setUniform( location, x, y, z );
      }
    else if ( _frag_target == GL_FRAGMENT_PROGRAM_ARB )
      {
        GLint location = uniform_location( uniform );
        CHECK_GL;
        glProgramLocalParameter4fARB( _frag_target, location, x, y, z, 0);
        CHECK_GL;
      }
  }

  void GLShader::setUniform( const char* uniform,
                             const float x, const float y,
                             const float z, const float w )
  {
    if ( _program )
      {
        GLint location = glGetUniformLocationARB( _program, uniform );
        CHECK_GL;
        setUniform( location, x, y, z, w );
      }
    else if ( _frag_target == GL_FRAGMENT_PROGRAM_ARB )
      {
        GLint location = uniform_location( uniform );
        CHECK_GL;
        glProgramLocalParameter4fARB( _frag_target, location, x, y, z, w);
        CHECK_GL;
      }
  }



  void GLShader::setUniform( const GLint location, const int x  )
  {
    glUniform1i( location, x );
    CHECK_GL;
  }

  void GLShader::setUniform( const GLint location,
                             const int x, const int y )
  {
    glUniform2i( location, x, y );
    CHECK_GL;
  }

  void GLShader::setUniform( const GLint location,
                             const int x, const int y, const int z )
  {
    glUniform3i( location, x, y, z );
    CHECK_GL;
  }


  void GLShader::setUniform( const GLint location,
                             const int x, const int y,
                             const int z, const int w  )
  {
    glUniform4i( location, x, y, z, w );
    CHECK_GL;
  }

  void GLShader::setUniform( const char* uniform,
                             const int x )
  {
    if ( _program )
      {
        GLint location = glGetUniformLocationARB( _program, uniform );
        CHECK_GL;
        setUniform( location, x );
      }
    else if ( _frag_target == GL_FRAGMENT_PROGRAM_ARB )
      {
        GLint location = uniform_location( uniform );
        CHECK_GL;
        glProgramLocalParameter4fARB( _frag_target, location,
                                      float(x), 0.f, 0.f, 0.f);
        CHECK_GL;
      }
  }

  void GLShader::setUniform( const char* uniform,
                             const int x, const int y )
  {
    if ( _program )
      {
        GLint location = glGetUniformLocationARB( _program, uniform );
        CHECK_GL;
        setUniform( location, x, y );
      }
    else if ( _frag_target == GL_FRAGMENT_PROGRAM_ARB )
      {
        GLint location = uniform_location( uniform );
        CHECK_GL;
        glProgramLocalParameter4fARB( _frag_target, location,
                                      float(x), float(y), 0.f, 0.f);
        CHECK_GL;
      }
  }

  void GLShader::setUniform( const char* uniform,
                             const int x, const int y,
                             const int z )
  {
    if ( _program )
      {
        GLint location = glGetUniformLocationARB( _program, uniform );
        CHECK_GL;
        setUniform( location, x, y, z );
      }
    else if ( _frag_target == GL_FRAGMENT_PROGRAM_ARB )
      {
        GLint location = uniform_location( uniform );
        CHECK_GL;
        glProgramLocalParameter4fARB( _frag_target, location,
                                      float(x), float(y), float(z), 0);
        CHECK_GL;
      }
  }

  void GLShader::setUniform( const char* uniform,
                             const int x, const int y,
                             const int z, const int w )
  {
    if ( _program )
      {
        GLint location = glGetUniformLocationARB( _program, uniform );
        CHECK_GL;
        setUniform( location, x, y, z, w );
      }
    else if ( _frag_target == GL_FRAGMENT_PROGRAM_ARB )
      {
        GLint location = uniform_location( uniform );
        CHECK_GL;
        glProgramLocalParameter4fARB( _frag_target, location,
                                      float(x), float(y), float(z), float(w) );
        CHECK_GL;
      }
  }

  void GLShader::setTextureUnit( const char* uniform, const GLint unit )
  {
    if ( !_program ) return;

    GLint location = glGetUniformLocationARB( _program, uniform );
    if ( location < 0 ) return;

    glUniform1iARB(location, unit);
  }


  void GLShader::enable()
  {
    if ( _frag_shader ) glEnable( _frag_target );
  }

  void GLShader::disable()
  {
    if ( _frag_shader ) glDisable( _frag_target );
    if ( _program  ) glUseProgramObjectARB(0);
  }

  void GLShader::bind()
  {
    CHECK_GL;
    if ( _frag_shader )    glBindProgramARB( _frag_target, _frag_shader );
    else if ( _program  )  glUseProgramObjectARB( _program );
    CHECK_GL;
  }

  GLShader::~GLShader()
  {
    glDeleteObjectARB( _program );
  }

}
