#include "Common.hlsli"



struct VertexShaderInput
{
    float4 Position : POSITION;
    float4 Color : COLOR;
};


struct VertexShaderOutput
{
    float4 SvPosition : SV_POSITION;
    float4 Color : COLOR0;
};


VertexShaderOutput VS_Main(VertexShaderInput input)
{
    const float4x4 vpMatrix = WorldData.Views[ObjectParameter.ViewIndex].ViewProjectionMatrix;
    
    VertexShaderOutput output;
    output.SvPosition = mul(input.Position, vpMatrix);
    output.Color = input.Color;
    
    return output;
}
