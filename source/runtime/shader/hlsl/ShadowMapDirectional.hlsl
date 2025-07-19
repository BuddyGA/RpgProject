#include "Common.hlsli"


// =============================================================================================== //
// VERTEX SHADER
// =============================================================================================== //
float4 VS_Main(float4 VertexPosition : POSITION) : SV_Position
{
    const RpgShaderView view = WorldData.Views[ObjectParameter.ViewIndex];
    const float4 wsVertexPosition = mul(VertexPosition, ObjectTransforms[ObjectParameter.TransformIndex]);
    
    float4 svPosition = mul(wsVertexPosition, view.ViewProjectionMatrix);
    svPosition.z = (length(wsVertexPosition.xyz - view.WorldPosition.xyz) * svPosition.w) / view.FarClipZ;
    
    return svPosition;
}
