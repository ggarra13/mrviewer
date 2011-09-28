/**
 * @file   rgba.glsl
 * @author gga
 * @date   Thu Jul  5 22:50:08 2007
 * 
 * @brief    simple rgba texture with 3D lut shader
 * 
 */


// Images
uniform sampler2D YImage;
uniform sampler2D UImage;
uniform sampler2D VImage;
uniform sampler3D lut;

// Standard controls
uniform float gain;
uniform float gamma;
uniform int   channel;

// Normalization variables
uniform bool  enableNormalization;
uniform float normMin;
uniform float normSpan;

// YCbCr variables
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


void main()
{ 
  //
  // Sample luminance and chroma, convert to RGB. 
  //
  vec3 pre;
  pre.r = texture2D(YImage, gl_TexCoord[0].st).r;  // Y
  pre.g = texture2D(UImage, gl_TexCoord[0].st).r;  // U
  pre.b = texture2D(VImage, gl_TexCoord[0].st).r;  // V

  pre += Koff;

  vec4 c;
  c.r = dot(Kr, pre);
  c.g = dot(Kg, pre);
  c.b = dot(Kb, pre);
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
      c.rgb = exp( texture3D(lut, c.rgb).rgb ); 
    }

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

  gl_FragColor = c;
} 
