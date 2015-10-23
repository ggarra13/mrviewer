/**
 * @file   rgba.glsl
 * @author gga
 * @date   Thu Jul  5 22:50:08 2007
 * 
 * @brief    simple rgba texture with 3D lut shader
 * 
 */

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


void main()
{ 
  //
  // Sample RGBA texture. 
  //
  vec4 c = texture2D(fgImage, gl_TexCoord[0].st);

  //
  // Apply normalization
  //
  if (enableNormalization)
    {
      c.rgb = (c.rgb - normMin) / normSpan;
    }

  //
  // Apply 3D color lookup table (in log space).
  //
  if (enableLut)
    {
      c.rgb = lutT + lutM * log( clamp(c.rgb, lutMin, lutMax) );
      c.rgb = exp( texture3D(lut, c.rgb).rgb ); 
    }

  if ( unpremult && c.a != 0.0 )
  {
      c.rgb /= c.a;
  }

  //
  // Apply gain 
  //
  c.rgb *= gain;

  //
  // Apply video gamma correction.
  // 
  c.rgb = pow( c.rgb, gamma );

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

  int x = -1;
  if ( mask == 1 )  // even odd rows
  {
      x = mod( gl_TexCoord[0].t * height, 2 );
      if ( c.a == 0.0 ) c.a = 1.0;
  }
  else if ( mask == 2 )  // even-odd columns
  {
      x = mod( gl_TexCoord[0].s * width, 2 );
      if ( c.a == 0.0 ) c.a = 1.0;
  }
  else if ( mask == 3 ) // checkerboard
  {
      x = mod( floor( gl_TexCoord[0].s * width ) + floor( gl_TexCoord[0].t * height ), 2 ) < 1;
      if ( c.a == 0.0 ) c.a = 1.0;
  }


  if ( x == mask_value )
  { 
      c.rgba = 0.0;
  }

  if ( premult )
  {
      c.rgb *= c.a;
  }

  gl_FragColor = c;

} 
