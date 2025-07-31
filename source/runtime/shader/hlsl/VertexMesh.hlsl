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
    float4 WsVertexPosition : WORLD_POSITION;
    float4 WsVertexNormal : WORLD_NORMAL;
    float4 WsVertexTangent : WORLD_TANGENT;
    float4 WsCameraPosition : CAMERA_WORLD_POSITION;
    float2 TexCoord : TEXCOORD;
};


VertexShaderOutput VS_Main(VertexShaderInput input)
{
    const RpgShaderView camera = WorldData.Views[ObjectParameter.ViewIndex];
    const float4x4 wsMatrix = ObjectTransforms[ObjectParameter.TransformIndex];
    
    VertexShaderOutput output;
    output.WsVertexPosition = mul(input.Position, wsMatrix);
    output.WsVertexNormal = normalize(mul(input.Normal, wsMatrix));
    output.WsVertexTangent = normalize(mul(input.Tangent, wsMatrix));
    output.SvPosition = mul(output.WsVertexPosition, camera.ViewProjectionMatrix);
    output.WsCameraPosition = camera.WorldPosition;
    output.TexCoord = input.TexCoord;
    
    return output;
}
