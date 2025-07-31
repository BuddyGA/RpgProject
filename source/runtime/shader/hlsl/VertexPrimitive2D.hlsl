#include "Common.hlsli"



struct VertexShaderInput
{
    float2 Position : POSITION;
    float4 Color : COLOR;
};


struct VertexShaderOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};


VertexShaderOutput VS_Main(VertexShaderInput input)
{
    const float2 scaledPosition = float2(input.Position.x * (2.0f / ViewportParameter.Width), input.Position.y * (2.0f / ViewportParameter.Height));
    
    VertexShaderOutput output;
    output.Position = float4(float2(scaledPosition.x - 1.0f, 1.0f - scaledPosition.y), 0.0f, 1.0f);
    output.Color = input.Color;
    
    return output;
}
