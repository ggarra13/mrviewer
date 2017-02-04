
import "ACESlib.Utilities.a1.0.0";

/** 
 * A Sat(uration) node
 * 
 * @param rIn  Red   pixels as input
 * @param gIn  Green pixels as input
 * @param bIn  Blue  pixels as input
 * @param aIn  Alpha pixels as input
 * @param rOut Red   pixels as output
 * @param gOut Green pixels as output
 * @param bOut Blue  pixels as output
 * @param aOut Alpha pixels as output
 * @param slope  saturation value
 */
void main(
    input varying float rIn,
    input varying float gIn,
    input varying float bIn,
    input varying float aIn,
    output varying float rOut,
    output varying float gOut,
    output varying float bOut,
    output varying float aOut,
    input float saturation
)
{
    float luma = 0.2126 * rIn + 0.7152 * gIn + 0.0722 * bIn;

    rOut = clamp( luma + saturation * (rIn - luma), 0.0, 1.0 );
    gOut = clamp( luma + saturation * (gIn - luma), 0.0, 1.0 );
    bOut = clamp( luma + saturation * (bIn - luma), 0.0, 1.0 );

    aOut = aIn;
}

