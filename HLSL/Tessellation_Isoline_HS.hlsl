#include "Tessellation.hlsli"

[domain("isoline")]
[partitioning("integer")]
[outputtopology("line")]
[outputcontrolpoints(2)]
[patchconstantfunc("IsolineConstantHS")]
[maxtessfactor(64.0f)]
float3 HS(InputPatch<VertexOut, 3> patch, uint i : SV_OutputControlPointID, uint patchID : SV_PrimitiveID) : POSITION
{
    return patch[i].posL;
}