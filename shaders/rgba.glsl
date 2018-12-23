/**
 * @file   rgba.glsl
 * @author gga
 * @date   Thu Jul  5 22:50:08 2007
 * 
 * @brief    simple rgba texture with 3D lut shader
 * 
 */

#version 130

// Images
uniform sampler2D fgImage;
uniform sampler3D lut;

// Interlaced/Checkerboard controls
uniform int mask;
uniform int mask_value;
uniform int height;
uniform int width;

// Standard controls
uniform float gain;
uniform float gamma;
uniform int   channel;

// Normalization variables
uniform bool  unpremult;
uniform bool  premult;
uniform bool  enableNormalization;
uniform float normMin;
uniform float normSpan;

// Lut variables
uniform bool  enableLut;
uniform bool  lutF;
uniform float lutMin;
uniform float lutMax;
uniform float lutM;
uniform float lutT;
uniform float scale;
uniform float offset;

void main()
{ 
  //
  // Sample RGBA texture. 
  //
  vec2 tc = gl_TexCoord[0].st;
  vec4 c = texture2D(fgImage, tc );
  int x = 1000;

  //
  // Apply normalization
  //
  if (enableNormalization)
    {
      c.rgb = (c.rgb - normMin) / normSpan;
    }
    
  //
  // Apply gain 
  //
  c.rgb *= gain;

  //
  // Apply 3D color lookup table (in log space).
  //
  if (enableLut)
    {
      c.rgb = lutT + lutM * log( clamp(c.rgb, lutMin, lutMax) );
      c.rgb = exp( texture3D( lut, c.rgb * scale + offset ).rgb ); 
    }

  if ( unpremult && c.a > 0.00001 )
  {
      c.rgb /= c.a;
  }


  //
  // Apply video gamma correction.
  // 
  c.r = pow( c.r, gamma );
  c.g = pow( c.g, gamma );
  c.b = pow( c.b, gamma );

  //
  // Apply channel selection
  // 
  if ( channel == 1 )
    {
      c.rgb = c.rrr;
    }
  else if ( channel == 2 )
    {
      c.rgb = c.ggg;
    }
  else if ( channel == 3 )
    {
      c.rgb = c.bbb;
    }
  else if ( channel == 4 )
    {
      c.rgb = c.aaa;
    }
  else if ( channel == 5 )
    {
      c.r *= 0.5;
      c.r += c.a * 0.5;
    }
  else if ( channel == 6 )
    {
      c.rgb = vec3( (c.r + c.g + c.b) / 3.0 );
    }

  if ( mask == 1 )  // even odd rows
  {
      float f = tc.y * height;
      x = int( mod( f, 2.0 ) );
  }
  else if ( mask == 2 )  // even-odd columns
  {
      float f2 = tc.x * width;
      x = int( mod( f2, 2.0 ) );
  }
  else if ( mask == 3 ) // checkerboard
  {
      float f = tc.y * height;
      float f2 = tc.x * width;
      x = int( mod( floor( f2 ) + floor( f ), 2.0 ) < 1 );
  }


  if ( x == mask_value )
  { 
      c.r = c.g = c.b = c.a = 0.0;
  }

  if ( premult )
  {
      c.rgb *= c.a;
  }

  gl_FragColor = c;

} 
