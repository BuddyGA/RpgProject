#pragma once

#ifndef RPG_SHADER_HLSL

#include "core/RpgMath.h"


typedef DirectX::XMVECTOR   RpgShaderFloat4;
typedef DirectX::XMMATRIX   RpgShaderMatrix;
typedef DirectX::XMINT4     RpgShaderInt4;


#define RPG_SHADER_CONSTANT_STATIC_ASSERT_LIMIT(type, alignmentSizeBytes, limitSizeBytes)   \
static_assert(sizeof(type) % alignmentSizeBytes == 0, "Shader constant type of <" ## #type ## "> must be multiple of " ## #alignmentSizeBytes ## " bytes!"); \
static_assert(sizeof(type) <= limitSizeBytes, "Shader constant type of <" ## #type ## "> exceeds limit size bytes!");


#else

typedef float4      RpgShaderFloat4;
typedef float4x4    RpgShaderMatrix;
typedef int4        RpgShaderInt4;

#define RPG_SHADER_CONSTANT_STATIC_ASSERT_LIMIT(type, alignmentSizeBytes, limitSizeBytes)   

#endif // !RPG_SHADER_HLSL


// Maximum material param vector
#define RPG_SHADER_MATERIAL_PARAM_VECTOR_COUNT      12

// Maximum material param scalar
#define RPG_SHADER_MATERIAL_PARAM_SCALAR_COUNT      16

// Maximum camera per world in single frame rendering
#define RPG_SHADER_CAMERA_MAX_COUNT                 4

// First index of point light in shader constant
#define RPG_SHADER_LIGHT_POINT_INDEX				0

// Maximum point light in single frame rendering
#define RPG_SHADER_LIGHT_POINT_MAX_COUNT			200

// First index of spot light in shader constant
#define RPG_SHADER_LIGHT_SPOT_INDEX				    (RPG_SHADER_LIGHT_POINT_INDEX + RPG_SHADER_LIGHT_POINT_MAX_COUNT)

// Maximum spot light in single frame rendering
#define RPG_SHADER_LIGHT_SPOT_MAX_COUNT			    64

// First index of directional light in shader constant
#define RPG_SHADER_LIGHT_DIRECTIONAL_INDEX			(RPG_SHADER_LIGHT_SPOT_INDEX + RPG_SHADER_LIGHT_SPOT_MAX_COUNT)

// Maximum directional light in single frame rendering
#define RPG_SHADER_LIGHT_DIRECTIONAL_MAX_COUNT		4

// Maximum light all types in single frame rendering
#define RPG_SHADER_MAX_LIGHT                        (RPG_SHADER_LIGHT_DIRECTIONAL_MAX_COUNT + RPG_SHADER_LIGHT_POINT_MAX_COUNT + RPG_SHADER_LIGHT_SPOT_MAX_COUNT)

// Maximum view per world in single frame rendering
#define RPG_SHADER_MAX_VIEW							(RPG_SHADER_CAMERA_MAX_COUNT + RPG_SHADER_MAX_LIGHT)



struct RpgShaderMaterialVectorScalarData
{
    RpgShaderFloat4 Vectors[RPG_SHADER_MATERIAL_PARAM_VECTOR_COUNT];
    float Scalars[RPG_SHADER_MATERIAL_PARAM_SCALAR_COUNT];
};
RPG_SHADER_CONSTANT_STATIC_ASSERT_LIMIT(RpgShaderMaterialVectorScalarData, 16, 256)



struct RpgShaderView
{
    // Used in world renderer and shadow map renderer
    RpgShaderMatrix ViewMatrix;

    // Used in world renderer and shadow map renderer
    RpgShaderMatrix ViewProjectionMatrix;

    // [XYZ]: World position, [W]: 1.0f
    RpgShaderFloat4 WorldPosition;

    // Near clip range
    float NearClipZ;

    // Far clip range
    float FarClipZ;
};


struct RpgShaderLight
{
    // [XYZ]: World position, [W]: 1.0f
    RpgShaderFloat4 WorldPosition;

    // [XYZ]: Unit vector, [W]: 0.0f
    RpgShaderFloat4 WorldDirection;

    // [XYZ]: RGB color, [W]: Intensity
    RpgShaderFloat4 ColorIntensity;

    // Attenuation radius factor
    float AttenuationRadius;

    // Attenuation falloff exponential factor
    float AttenuationFallOffExp;

    // For spotlight only. Inner cone angle in radian
    float SpotLightInnerConeRadian;

    // For spotlight only. Outer cone angle in radian
    float SpotLightOuterConeRadian;

    // Shadow view index used in shadow map renderer. (-1) if light does not cast shadow
    int ShadowViewIndex;

    // Shadow texture descriptor index. (-1) if light does not cast shadow
    int ShadowTextureDescriptorIndex;
};



struct RpgShaderWorldData
{
    // All views (camera + light)
    RpgShaderView Views[RPG_SHADER_MAX_VIEW];

    // All lights
    RpgShaderLight Lights[RPG_SHADER_MAX_LIGHT];

    // [XYZ]: RGB color, [W]: Strength multiplier
    RpgShaderFloat4 AmbientColorStrength;

    float DeltaTime;
    int ViewCount;
    int DirectionalLightCount;
    int PointLightCount;
    int SpotLightCount;
};
RPG_SHADER_CONSTANT_STATIC_ASSERT_LIMIT(RpgShaderWorldData, 16, RPG_MEMORY_SIZE_KiB(64))



struct RpgShaderViewportParameter
{
    float Width;
    float Height;
};
RPG_SHADER_CONSTANT_STATIC_ASSERT_LIMIT(RpgShaderViewportParameter, 4, 8)



struct RpgShaderMaterialParameter
{
    int TextureDescriptorIndex_BaseColor;
    int TextureDescriptorIndex_Normal;
    int TextureDescriptorIndex_Metalness;
    int TextureDescriptorIndex_Specular;
    int TextureDescriptorIndex_Roughness;
    int TextureDescriptorIndex_AmbOcc;
    int TextureDescriptorIndex_OpacityMask;
    int TextureDescriptorIndex_Custom0;
    int TextureDescriptorIndex_Custom1;
    int TextureDescriptorIndex_Custom2;
    int TextureDescriptorIndex_Custom3;
    int TextureDescriptorIndex_Custom4;
    int TextureDescriptorIndex_Custom5;
    int TextureDescriptorIndex_Custom6;
    int TextureDescriptorIndex_Custom7;
    int VectorScalarValueIndex;
};
RPG_SHADER_CONSTANT_STATIC_ASSERT_LIMIT(RpgShaderMaterialParameter, 4, 64)



struct RpgShaderObjectParameter
{
    int ViewIndex;
    int TransformIndex;
};
RPG_SHADER_CONSTANT_STATIC_ASSERT_LIMIT(RpgShaderObjectParameter, 4, 16)



struct RpgShaderSkinnedObjectParameter
{
    int SkeletonIndex;
    int VertexStart;
    int VertexCount;
    int IndexStart;
    int IndexCount;
    int SkinnedVertexStart;
    int SkinnedIndexStart;
};



#define RPG_SHADER_SHADOW_MAP_CUBE_FACE_R   0x1
#define RPG_SHADER_SHADOW_MAP_CUBE_FACE_L   0x2
#define RPG_SHADER_SHADOW_MAP_CUBE_FACE_U   0x4
#define RPG_SHADER_SHADOW_MAP_CUBE_FACE_D   0x8
#define RPG_SHADER_SHADOW_MAP_CUBE_FACE_F   0x10
#define RPG_SHADER_SHADOW_MAP_CUBE_FACE_B   0x20


#define RPG_SHADER_NAME_VertexPrimitive             "VertexPrimitive"
#define RPG_SHADER_NAME_VertexPrimitive2D           "VertexPrimitive2D"
#define RPG_SHADER_NAME_VertexMesh                  "VertexMesh"

#define RPG_SHADER_NAME_PixelColor                  "PixelColor"
#define RPG_SHADER_NAME_PixelForwardPhong           "PixelForwardPhong"
#define RPG_SHADER_NAME_PixelForwardPhong_Mask      "PixelForwardPhong_Mask"

#define RPG_SHADER_NAME_ComputeFrustum              "ComputeFrustum"
#define RPG_SHADER_NAME_ComputeSkinning				"ComputeSkinning"

#define RPG_SHADER_NAME_ShadowMapDirectional		"ShadowMapDirectional"

#define RPG_SHADER_NAME_ShadowMapCube_VS			"ShadowMapCube_VS"
#define RPG_SHADER_NAME_ShadowMapCube_GS			"ShadowMapCube_GS"

#define RPG_SHADER_NAME_PostProcessFullscreen_VS    "PostProcessFullScreen_VS"
#define RPG_SHADER_NAME_PostProcessFullscreen_PS    "PostProcessFullScreen_PS"

#define RPG_SHADER_NAME_GUI_VS                      "GUI_VS"
#define RPG_SHADER_NAME_GUI_PS                      "GUI_PS"
#define RPG_SHADER_NAME_GUI_Font_PS                 "GUI_Font_PS"
