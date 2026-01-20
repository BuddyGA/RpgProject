#include "Common.hlsli"


Texture2DArray TextureColors : register(t2, space2);
Texture2DArray TextureNormals : register(t3, space2);
Texture2DArray TextureSplats : register(t4, space2);


struct PixelShaderInput
{
    float4 SvPosition : SV_Position;
    float4 WsFragPosition : WORLD_POSITION;
    float4 WsFragNormal : WORLD_NORMAL;
    float4 WsFragTangent : WORLD_TANGENT;
    float4 WsCameraPosition : CAMERA_WORLD_POSITION;
    float2 TexCoord : TEXCOORD;
};


float4 PS_Main(PixelShaderInput input) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}