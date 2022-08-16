#include "Tessellation.hlsli"

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("QuadPatchConstantHS")]
[maxtessfactor(64.0f)]
float3 HS(InputPatch<VertexOut, 3> patch, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID) : POSITION
{
    return patch[i].posL;
}