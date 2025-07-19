#include "Common.hlsli"


#define RPG_POSTPROCESS_GAMMA   1


#define MATERIAL_PARAM_SCALAR_INDEX_gamma   0


// =============================================================================================== //
// VERTEX SHADER
// =============================================================================================== //
struct VertexShaderOutput
{
    float4 SvPosition : SV_Position;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};


VertexShaderOutput VS_Main(uint vertexId : SV_VertexID)
{
    VertexShaderOutput output;
    output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.TexCoord = float2((vertexId << 1) & 2, vertexId & 2);
    output.SvPosition = float4(output.TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
    output.SvPosition.y *= -1.0f;
    
    return output;
}




// =============================================================================================== //
// PIXEL SHADER
// =============================================================================================== //
typedef VertexShaderOutput PixelShaderInput;


float4 PS_Main(PixelShaderInput input) : SV_TARGET
{
    float4 linearColor = Rpg_GetMaterialParameterTextureColor(MaterialParameter.TextureDescriptorIndex_BaseColor, SamplerPoint, input.TexCoord);
    
#if RPG_POSTPROCESS_GAMMA 
    float gamma = Rpg_GetMaterialParameterScalarValue(MATERIAL_PARAM_SCALAR_INDEX_gamma);
    return float4(pow(linearColor.rgb, 1.0f / gamma), linearColor.a);
    
#else
    return linearColor;
    
#endif    
}
