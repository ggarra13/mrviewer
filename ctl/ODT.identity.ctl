
// <ACEStransformID>ODT.identity</ACEStransformID>
// <ACESuserName>ODT Identity</ACESuserName>

// 
// Output Device Transform - Identity
//

//
// Summary :
//  This ODT is an identity device transform.  It is used to verify the output
//  of the Rendering Transforms.
//


void main 
(
    input varying float rIn, 
    input varying float gIn, 
    input varying float bIn, 
    input varying float aIn, 
    output varying float rOut,
    output varying float gOut,
    output varying float bOut,
    output varying float aOut
)
{  
    rOut = rIn;
    gOut = gIn;
    bOut = bIn;
    aOut = aIn;
}
