#include "Common.hlsli"


// =============================================================================================== //
// VERTEX SHADER
// =============================================================================================== //
struct VertexShaderInput
{
    float2 Position : POSITION0;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};


struct VertexShaderOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};


VertexShaderOutput VS_Main(VertexShaderInput input)
{
    const float2 scaledPosition = float2(input.Position.x * (2.0f / ViewportParameter.Width), input.Position.y * (2.0f / ViewportParameter.Height));
    
    VertexShaderOutput output;
    output.Position = float4(float2(scaledPosition.x - 1.0f, 1.0f - scaledPosition.y), 0.0f, 1.0f);
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;
    
    return output;
}



// =============================================================================================== //
// PIXEL SHADER
// =============================================================================================== //
typedef VertexShaderOutput PixelShaderInput;


float4 PS_Main(PixelShaderInput input) : SV_TARGET
{
#if FONT
    return float4(input.Color.rgb, Rpg_GetMaterialParameterTextureColor(MaterialParameter.TextureDescriptorIndex_OpacityMask, SamplerPoint, input.TexCoord).r * input.Color.a);
    
#else
    return input.Color * Rpg_GetMaterialParameterTextureColor(MaterialParameter.TextureDescriptorIndex_BaseColor, SamplerPoint, input.TexCoord);
    
#endif // FONT
}
