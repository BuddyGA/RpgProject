#define RPG_SHADER_HLSL
#include "../RpgShaderTypes.h"


// ================================================================================================================================= //
// RESOURCE BINDINGS
// ================================================================================================================================= //

// Static samplers
SamplerState SamplerMipMapLinear : register(s0, space0);
SamplerState SamplerPoint : register(s1, space0);
SamplerComparisonState SamplerShadow : register(s2, space0);

// Texture2D (Dynamic indexing)
Texture2D DynamicIndexingTexture2Ds[] : register(t0, space0);

// TextureCube (Dynamic indexing) 
TextureCube DynamicIndexingTextureCubes[] : register(t0, space1);

// Material vector/scalar values
StructuredBuffer<RpgShaderMaterialVectorScalarData> MaterialVectorScalars : register(t0, space2);

// Transform data
StructuredBuffer<float4x4> ObjectTransforms : register(t1, space2);

// World data
ConstantBuffer<RpgShaderWorldData> WorldData : register(b0, space0);

// Viewport parameter
ConstantBuffer<RpgShaderViewportParameter> ViewportParameter : register(b1, space0);

// Material parameter
ConstantBuffer<RpgShaderMaterialParameter> MaterialParameter : register(b2, space0);

// Object parameter
ConstantBuffer<RpgShaderObjectParameter> ObjectParameter : register(b3, space0);




// ================================================================================================================================= //
// FUNCTIONS
// ================================================================================================================================= //
#define RPG_SQR(x) ((x) * (x))


inline float4 Rpg_GetMaterialParameterTextureColor(int descriptorIndex, SamplerState samplerState, float2 texCoord)
{
    return DynamicIndexingTexture2Ds[NonUniformResourceIndex(descriptorIndex)].Sample(samplerState, texCoord);
}


inline float4 Rpg_GetMaterialParameterVectorValue(int vectorIndex)
{
    return MaterialVectorScalars.Load(MaterialParameter.VectorScalarValueIndex).Vectors[vectorIndex];
}


inline float Rpg_GetMaterialParameterScalarValue(int scalarIndex)
{
    return MaterialVectorScalars.Load(MaterialParameter.VectorScalarValueIndex).Scalars[scalarIndex];
}
