/**
 * @file   rgba.glsl
 * @author gga
 * @date   Thu Jul  5 22:50:08 2007
 *
 * @brief    simple YByRy texture with 3D lut shader
 *
 */
#version 120

// Images
uniform sampler2D YImage;
uniform sampler2D UImage;
uniform sampler2D VImage;
uniform sampler3D lut;

// Interlaced/Checkerboard controls
uniform int mask;
uniform int mask_value;
uniform int height;
uniform int width;

// Standard controls
uniform float dissolve;
uniform float gain;
uniform float gamma;
uniform int   channel;

// Normalization variables
uniform bool  premult;
uniform bool  unpremult;
uniform bool  enableNormalization;
uniform float normMin;
uniform float normSpan;

// Y B-Y R-Y variables
uniform vec3  yw;

// Lut variables
uniform bool  enableLut;
uniform bool  lutF;
uniform float lutMin;
uniform float lutMax;
uniform float lutM;
uniform float lutT;
uniform float scale;
uniform float offset;

uniform bool   enableColorMatrix;
uniform mat4x4 colorMatrix;

void main()
{
  //
  // Sample luminance and chroma, convert to RGB.
  //
  vec3 pre;
  vec2 tc = gl_TexCoord[0].st;
  pre.r = texture2D(YImage, tc).r;  // Y
  pre.g = texture2D(UImage, tc).r;  // U
  pre.b = texture2D(VImage, tc).r;  // V

  vec4 c;
  c.r = (pre.g + 1.0) * pre.r;
  c.b = (pre.b + 1.0) * pre.r;
  c.g = (pre.r - c.r * yw.x - c.b * yw.z) / yw.y;
  c.rgb = clamp( c.rgb, 0.0, 1.0 );
  c.a = 1.0;

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
      c.rgb = exp( texture3D(lut, scale * c.rgb + offset ).rgb );
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


  int x = 1000;

  if ( mask == 1 )  // even odd rows
  {
      float f = float( tc.y * height );
      x = int( mod( f, 2.0 ) );
  }
  else if ( mask == 2 ) // even odd columns
  {
      float f2 = float( tc.x * width );
      x = int( mod( f2, 2.0 ) );
  }
  else if ( mask == 3 ) // checkerboard
  {
      float f = float( tc.y * height );
      float f2 = tc.x * width;
      x = int( mod( floor( f2 ) + floor( f ), 2.0 ) < 1 );
  }


  if ( x == mask_value )
  {
      c.r = c.g = c.b = c.a = 0.0f;
  }

  if ( enableColorMatrix )
    c.rgba *= colorMatrix;


  if ( premult )
  {
      c.rgb *= c.a;
  }

  c.rgba *= dissolve;

  gl_FragColor = c;
}
