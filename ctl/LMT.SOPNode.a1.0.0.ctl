
import "ACESlib.Utilities.a1.0.0";

/** 
 * A SOP node
 * 
 * @param rIn  Red   pixels as input
 * @param gIn  Green pixels as input
 * @param bIn  Blue  pixels as input
 * @param aIn  Alpha pixels as input
 * @param rOut Red   pixels as output
 * @param gOut Green pixels as output
 * @param bOut Blue  pixels as output
 * @param aOut Alpha pixels as output
 * @param slope  Slope values in x,y,z
 * @param offset Offset values in x,y,z
 * @param power  Power  values in x,y,z
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
    input float slope[3] = { 1, 1, 1 },
    input float offset[3] = { 0, 0, 0 },
    input float power[3] = { 1, 1, 1 }
)
{
    rOut = pow(clamp(rIn * slope[0] + offset[0], 0., 1.), power[0] );
    gOut = pow(clamp(gIn * slope[1] + offset[1], 0., 1.), power[1] );
    bOut = pow(clamp(bIn * slope[2] + offset[2], 0., 1.), power[2] );
    aOut = aIn;
}
