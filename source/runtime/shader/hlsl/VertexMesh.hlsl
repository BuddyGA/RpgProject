#include "Common.hlsli"


struct VertexShaderInput
{
    // Slot 0
    float4 Position : POSITION;
    
    // Slot 1
    float4 Normal : NORMAL;
    float4 Tangent : TANGENT;
    
    // Slot 2
    float2 TexCoord : TEXCOORD;
};


struct VertexShaderOutput
{
    float4 SvPosition : SV_Position;
    float4 WsFragPosition : WORLD_POSITION;
    float4 WsFragNormal : WORLD_NORMAL;
    float4 WsFragTangent : WORLD_TANGENT;
    float4 WsCameraPosition : CAMERA_WORLD_POSITION;
    float2 TexCoord : TEXCOORD;
};


VertexShaderOutput VS_Main(VertexShaderInput input)
{
    const RpgShaderView camera = WorldData.Views[ObjectParameter.ViewIndex];
    const float4x4 wsMatrix = ObjectTransforms[ObjectParameter.TransformIndex];
    
    VertexShaderOutput output;
    output.WsFragPosition = mul(input.Position, wsMatrix);
    output.WsFragNormal = normalize(mul(input.Normal, wsMatrix));
    output.WsFragTangent = normalize(mul(input.Tangent, wsMatrix));
    output.SvPosition = mul(output.WsFragPosition, camera.ViewProjectionMatrix);
    output.WsCameraPosition = camera.WorldPosition;
    output.TexCoord = input.TexCoord;
    
    return output;
}
