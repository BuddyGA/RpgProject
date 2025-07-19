#pragma once

#include "RpgMaterial.h"



namespace RpgRenderPipeline
{
    enum EGraphicsRootParamIndex : uint8_t
    {
        GRPI_OBJECT_PARAM = 0,
        GRPI_MATERIAL_PARAM,
        GRPI_VIEWPORT_PARAM,
        GRPI_WORLD_DATA,
        GRPI_TRANSFORM_DATA,
        GRPI_MATERIAL_VECTOR_SCALAR_DATA,
        GRPI_TEXTURES,

        GRPI_MAX_COUNT
    };


    enum EComputeRootParamIndex : uint8_t
    {
        CRPI_SKINNED_OBJECT_PARAM = 0,
        CRPI_SKELETON_BONE_DATA,
        CRPI_VERTEX_POSITION,
        CRPI_VERTEX_NORMAL_TANGENT,
        CRPI_VERTEX_SKIN,
        CRPI_SKINNED_VERTEX_POSITION,
        CRPI_SKINNED_VERTEX_NORMAL_TANGENT,

        CRPI_MAX_COUNT
    };


    extern void Initialize() noexcept;
    extern void Shutdown() noexcept;


    extern ID3D12RootSignature* GetRootSignatureGraphics() noexcept;
    extern ID3D12RootSignature* GetRootSignatureCompute() noexcept;

    extern void AddMaterials(RpgSharedMaterial* materialArray, int materialCount) noexcept;
    extern void CompileMaterialPSOs(bool bWaitAll) noexcept;
    extern ID3D12PipelineState* GetMaterialPSO(const RpgSharedMaterial& material) noexcept;


    // Graphics
    extern ID3D12PipelineState* GetGraphicsPSO_ShadowMapDirectional() noexcept;
    extern ID3D12PipelineState* GetGraphicsPSO_ShadowMapCube() noexcept;

    // Compute
    extern ID3D12PipelineState* GetComputePSO_Skinning() noexcept;

};
