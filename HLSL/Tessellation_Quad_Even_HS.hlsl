#include "Tessellation.hlsli"

// 0  1
// .__.
//   /
// ./_.
// 2  3 
[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("QuadConstantHS")]
[maxtessfactor(64.0f)]
float3 HS(InputPatch<VertexOut, 3> patch, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID) : POSITION
{
    return patch[i].posL;
}