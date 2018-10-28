/**
 * @file   YCbCr.glsl
 * @author gga
 * @date   Thu Jul  5 22:50:08 2007
 * 
 * @brief    simple YCbCrA texture with 3D lut shader
 * 
 */

#version 130

// Images
uniform sampler2D YImage;
uniform sampler2D UImage;
uniform sampler2D VImage;
uniform sampler3D lut;
uniform sampler2D AImage;

// Interlaced/Checkerboard controls (don't work)
uniform int mask;
uniform int mask_value;
uniform int height;
uniform int width;

// Standard controls
uniform float gain;
uniform float gamma;
uniform int   channel;

// Normalization variables
uniform bool  premult;
uniform bool  unpremult;
uniform bool  enableNormalization;
uniform float normMin;
uniform float normSpan;

// YCbCr variables
uniform bool  coeffs;  // Use fed coefficients instead of builtin ones
uniform vec3  Koff;
uniform vec3  Kr;
uniform vec3  Kg;
uniform vec3  Kb;

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
  // Sample luminance and chroma, convert to RGB. 
  //
  vec3 yuv;	
  vec4 c;	
  vec4 pre;
  vec2 tc = gl_TexCoord[0].st;
  
  pre.r = texture2D(YImage, tc.st).r;  // Y
  pre.g = texture2D(UImage, tc.st).r;  // U
  pre.b = texture2D(VImage, tc.st).r;  // V
  pre.a = texture2D(AImage, tc.st).r;  // A

  if ( coeffs )
  {
	pre += vec4( Koff.r, Koff.g, Koff.b, 0.0 );

  	c.r = dot(Kr, pre.xyz);
  	c.g = dot(Kg, pre.xyz);
  	c.b = dot(Kb, pre.xyz);
  }
  else
  {

      yuv.r = 1.1643 * ( pre.r - 0.0625 );
      yuv.g = pre.g - 0.5;
      yuv.b = pre.b - 0.5;

      c.r = yuv.r + 1.5958 * yuv.b;
      c.g = yuv.r - 0.39173 * yuv.g - 0.81290 * yuv.b;
      c.b = yuv.r + 2.017 * yuv.g;
  }

  c.rgb = clamp( c.rgb, 0.0, 1.0 );
  int x = 1000;
  
  if ( mask == 1 )  // even odd rows
  {
      float f = tc.y * height;
      x = int( mod( f, 2.0 ) );
  }
  else if ( mask == 2 ) // even odd columns
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

  if ( premult )
  {
      c.rgb *= c.a;
  }

  gl_FragColor = c;
} 
