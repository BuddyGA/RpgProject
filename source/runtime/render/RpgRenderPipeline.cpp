#include "RpgRenderPipeline.h"
#include "task/RpgRenderTask_CompilePSO.h"
#include "shader/RpgShaderManager.h"
#include "core/dsa/RpgArray.h"



namespace RpgRenderPipeline
{
    static ComPtr<ID3D12RootSignature> RootSignatureGraphics;
    static ComPtr<ID3D12RootSignature> RootSignatureCompute;
    static bool bInitialized;

    static RpgArray<RpgSharedMaterial> Materials;
    static RpgArray<ComPtr<ID3D12PipelineState>> MaterialPipelineStates;
    static RpgArray<RpgRenderTask_CompilePSO> TaskCompilePSOs;

    static ComPtr<ID3D12PipelineState> GraphicsPSO_ShadowMapDirectional;
    static ComPtr<ID3D12PipelineState> GraphicsPSO_ShadowMapCube;
    static ComPtr<ID3D12PipelineState> ComputePSO_Skinning;



    static void ValidateRootParameters(const D3D12_ROOT_PARAMETER1* rootParamData, int rootParamCount) noexcept
    {
        RPG_Assert(rootParamData);
        RPG_Assert(rootParamCount > 0);

        uint32_t dwordCount = 0;

        for (int i = 0; i < rootParamCount; ++i)
        {
            const D3D12_ROOT_PARAMETER1& rp = rootParamData[i];

            if (rp.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            {
                ++dwordCount;
            }
            else if (rp.ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
            {
                dwordCount += rp.Constants.Num32BitValues;
            }
            else
            {
                dwordCount += 2;
            }
        }

        RPG_Assert(dwordCount <= 64);
    }


    static void CreateRootSignatureGraphics() noexcept
    {
        RpgArrayInline<D3D12_STATIC_SAMPLER_DESC, 3> staticSamplers;
        {
            D3D12_STATIC_SAMPLER_DESC& samplerMipMapLinear = staticSamplers.Add();
            samplerMipMapLinear.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerMipMapLinear.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerMipMapLinear.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            samplerMipMapLinear.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            samplerMipMapLinear.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            samplerMipMapLinear.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            samplerMipMapLinear.MaxAnisotropy = 0;
            samplerMipMapLinear.MinLOD = 0.0f;
            samplerMipMapLinear.MaxLOD = 16.0f;
            samplerMipMapLinear.MipLODBias = 0.0f;
            samplerMipMapLinear.ShaderRegister = 0;	// s0
            samplerMipMapLinear.RegisterSpace = 0;	// space0
            samplerMipMapLinear.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_STATIC_SAMPLER_DESC& samplerPoint = staticSamplers.Add();
            samplerPoint.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            samplerPoint.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            samplerPoint.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            samplerPoint.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            samplerPoint.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            samplerPoint.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            samplerPoint.MaxAnisotropy = 0;
            samplerPoint.MinLOD = 0.0f;
            samplerPoint.MaxLOD = 0.0f;
            samplerPoint.MipLODBias = 0.0f;
            samplerPoint.ShaderRegister = 1;	// s1
            samplerPoint.RegisterSpace = 0;		// space0
            samplerPoint.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_STATIC_SAMPLER_DESC& samplerShadow = staticSamplers.Add();
            samplerShadow.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            samplerShadow.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            samplerShadow.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            samplerShadow.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
            samplerShadow.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            samplerShadow.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            samplerShadow.MaxAnisotropy = 1;
            samplerShadow.MinLOD = 0.0f;
            samplerShadow.MaxLOD = 0.0f;
            samplerShadow.MipLODBias = 0.0f;
            samplerShadow.ShaderRegister = 2;	// s2
            samplerShadow.RegisterSpace = 0;	// space0
            samplerShadow.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        }


        RpgArrayInline<D3D12_ROOT_PARAMETER1, GRPI_MAX_COUNT> rootParameters(GRPI_MAX_COUNT);

        RpgArrayInline<D3D12_DESCRIPTOR_RANGE1, 2> descriptorRangeTextures;
        {
            D3D12_DESCRIPTOR_RANGE1& descriptorRangeTexture2d = descriptorRangeTextures.Add();
            descriptorRangeTexture2d.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
            descriptorRangeTexture2d.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            descriptorRangeTexture2d.OffsetInDescriptorsFromTableStart = 0;
            descriptorRangeTexture2d.NumDescriptors = UINT_MAX;
            descriptorRangeTexture2d.BaseShaderRegister = 0;	// t0
            descriptorRangeTexture2d.RegisterSpace = 0;			// space0

            D3D12_DESCRIPTOR_RANGE1& descriptorRangeTextureCube = descriptorRangeTextures.Add();
            descriptorRangeTextureCube.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
            descriptorRangeTextureCube.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            descriptorRangeTextureCube.OffsetInDescriptorsFromTableStart = 0;
            descriptorRangeTextureCube.NumDescriptors = UINT_MAX;
            descriptorRangeTextureCube.BaseShaderRegister = 0;	// t0
            descriptorRangeTextureCube.RegisterSpace = 1;		// space1
        }

        D3D12_ROOT_PARAMETER1& dynamicIndexingTextures = rootParameters[GRPI_TEXTURES];
        dynamicIndexingTextures.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        dynamicIndexingTextures.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        dynamicIndexingTextures.DescriptorTable.pDescriptorRanges = descriptorRangeTextures.GetData();
        dynamicIndexingTextures.DescriptorTable.NumDescriptorRanges = descriptorRangeTextures.GetCount();

        D3D12_ROOT_PARAMETER1& srvMaterialVectorScalarData = rootParameters[GRPI_MATERIAL_VECTOR_SCALAR_DATA];
        srvMaterialVectorScalarData.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        srvMaterialVectorScalarData.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        srvMaterialVectorScalarData.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        srvMaterialVectorScalarData.Descriptor.ShaderRegister = 0;	// t0
        srvMaterialVectorScalarData.Descriptor.RegisterSpace = 2;	// space2

        D3D12_ROOT_PARAMETER1& srvWorldTransformData = rootParameters[GRPI_TRANSFORM_DATA];
        srvWorldTransformData.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        srvWorldTransformData.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        srvWorldTransformData.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        srvWorldTransformData.Descriptor.ShaderRegister = 1;	// t1
        srvWorldTransformData.Descriptor.RegisterSpace = 2;		// space2

        D3D12_ROOT_PARAMETER1& cbvWorldData = rootParameters[GRPI_WORLD_DATA];
        cbvWorldData.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        cbvWorldData.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        cbvWorldData.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        cbvWorldData.Descriptor.ShaderRegister = 0;	// b0
        cbvWorldData.Descriptor.RegisterSpace = 0;	// space0

        D3D12_ROOT_PARAMETER1& rcViewportParameter = rootParameters[GRPI_VIEWPORT_PARAM];
        rcViewportParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rcViewportParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rcViewportParameter.Constants.Num32BitValues = sizeof(RpgShaderViewportParameter) / 4;
        rcViewportParameter.Constants.ShaderRegister = 1;	// b1
        rcViewportParameter.Constants.RegisterSpace = 0;	// space0

        D3D12_ROOT_PARAMETER1& rcMaterialParameter = rootParameters[GRPI_MATERIAL_PARAM];
        rcMaterialParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rcMaterialParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rcMaterialParameter.Constants.Num32BitValues = sizeof(RpgShaderMaterialParameter) / 4;
        rcMaterialParameter.Constants.ShaderRegister = 2;	// b2
        rcMaterialParameter.Constants.RegisterSpace = 0;	// space0

        D3D12_ROOT_PARAMETER1& rcObjectParameter = rootParameters[GRPI_OBJECT_PARAM];
        rcObjectParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rcObjectParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rcObjectParameter.Constants.Num32BitValues = sizeof(RpgShaderObjectParameter) / 4;
        rcObjectParameter.Constants.ShaderRegister = 3;	// b3
        rcObjectParameter.Constants.RegisterSpace = 0;	// space0

        // Validate root parameters word count
        ValidateRootParameters(rootParameters.GetData(), rootParameters.GetCount());

        // Create root signature
        D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;

        rootSignatureDesc.Desc_1_1.Flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

        rootSignatureDesc.Desc_1_1.pStaticSamplers = staticSamplers.GetData();
        rootSignatureDesc.Desc_1_1.NumStaticSamplers = static_cast<UINT>(staticSamplers.GetCount());
        rootSignatureDesc.Desc_1_1.pParameters = rootParameters.GetData();
        rootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(rootParameters.GetCount());

        ComPtr<ID3DBlob> signatureBlob;
        ComPtr<ID3DBlob> errorBlob;
        D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signatureBlob, &errorBlob);

        if (errorBlob)
        {
            RPG_RuntimeErrorCheck("Fail to serialize root signature!\n\tMessage: %s", (const char*)errorBlob->GetBufferPointer());
        }

        RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignatureGraphics)));
        RPG_D3D12_SetDebugName(RootSignatureGraphics, "RootSig_Graphics");
    }


    static void CreateRootSignatureCompute() noexcept
    {
        RpgArrayInline<D3D12_ROOT_PARAMETER1, CRPI_MAX_COUNT> rootParameters(CRPI_MAX_COUNT);

        D3D12_ROOT_PARAMETER1& uavVertexOutputPosition = rootParameters[CRPI_SKINNED_VERTEX_POSITION];
        uavVertexOutputPosition.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
        uavVertexOutputPosition.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        uavVertexOutputPosition.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        uavVertexOutputPosition.Descriptor.ShaderRegister = 0;	// u0
        uavVertexOutputPosition.Descriptor.RegisterSpace = 0;	// space0

        D3D12_ROOT_PARAMETER1& uavVertexOutputNormalTangent = rootParameters[CRPI_SKINNED_VERTEX_NORMAL_TANGENT];
        uavVertexOutputNormalTangent.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
        uavVertexOutputNormalTangent.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        uavVertexOutputNormalTangent.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        uavVertexOutputNormalTangent.Descriptor.ShaderRegister = 1;	// u1
        uavVertexOutputNormalTangent.Descriptor.RegisterSpace = 0;	// space0

        D3D12_ROOT_PARAMETER1& srvVertexInputPosition = rootParameters[CRPI_VERTEX_POSITION];
        srvVertexInputPosition.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        srvVertexInputPosition.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        srvVertexInputPosition.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        srvVertexInputPosition.Descriptor.ShaderRegister = 8;	// t8
        srvVertexInputPosition.Descriptor.RegisterSpace = 0;	// space0

        D3D12_ROOT_PARAMETER1& srvVertexInputNormalTangent = rootParameters[CRPI_VERTEX_NORMAL_TANGENT];
        srvVertexInputNormalTangent.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        srvVertexInputNormalTangent.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        srvVertexInputNormalTangent.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        srvVertexInputNormalTangent.Descriptor.ShaderRegister = 9;	// t9
        srvVertexInputNormalTangent.Descriptor.RegisterSpace = 0;	// space0

        D3D12_ROOT_PARAMETER1& srvVertexInputSkin = rootParameters[CRPI_VERTEX_SKIN];
        srvVertexInputSkin.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        srvVertexInputSkin.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        srvVertexInputSkin.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        srvVertexInputSkin.Descriptor.ShaderRegister = 10;	// t10
        srvVertexInputSkin.Descriptor.RegisterSpace = 0;	// space0

        D3D12_ROOT_PARAMETER1& srvSkeletonBone = rootParameters[CRPI_SKELETON_BONE_DATA];
        srvSkeletonBone.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        srvSkeletonBone.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        srvSkeletonBone.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        srvSkeletonBone.Descriptor.ShaderRegister = 11;	// t11
        srvSkeletonBone.Descriptor.RegisterSpace = 0;	// space0

        D3D12_ROOT_PARAMETER1& rcObjectParameter = rootParameters[CRPI_SKINNED_OBJECT_PARAM];
        rcObjectParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rcObjectParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        rcObjectParameter.Constants.Num32BitValues = sizeof(RpgShaderSkinnedObjectParameter) / sizeof(UINT);
        rcObjectParameter.Constants.ShaderRegister = 0;	// b0
        rcObjectParameter.Constants.RegisterSpace = 0;	// space0

        // Validate root parameters word count
        ValidateRootParameters(rootParameters.GetData(), rootParameters.GetCount());

        // Create root signature
        D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
        rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        rootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
        rootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
        rootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
        rootSignatureDesc.Desc_1_1.pParameters = rootParameters.GetData();
        rootSignatureDesc.Desc_1_1.NumParameters = static_cast<UINT>(rootParameters.GetCount());

        ComPtr<ID3DBlob> signatureBlob;
        ComPtr<ID3DBlob> errorBlob;
        D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signatureBlob, &errorBlob);

        if (errorBlob)
        {
            RPG_RuntimeErrorCheck("Fail to serialize root signature!\n\tMessage: %s", (const char*)errorBlob->GetBufferPointer());
        }

        RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignatureCompute)));
        RPG_D3D12_SetDebugName(RootSignatureCompute, "RootSig_Compute");
    }

};



void RpgRenderPipeline::Initialize() noexcept
{
    if (bInitialized)
    {
        return;
    }

    CreateRootSignatureGraphics();
    CreateRootSignatureCompute();

    TaskCompilePSOs.Reserve(32);

    // Graphics PSO
    {
        // ShadowDepth
        {
            RpgRenderPipelineState state{};
            state.VertexShaderName = RPG_SHADER_NAME_ShadowMapDirectional;
            state.VertexMode = RpgRenderVertexMode::MESH;
            state.RasterMode = RpgRenderRasterMode::SOLID;
            state.BlendMode = RpgRenderColorBlendMode::NONE;
            state.DepthStencilFormat = RpgRender::DEFAULT_FORMAT_SHADOW_DEPTH;
            state.bDepthTest = true;
            state.bDepthWrite = true;
            state.DepthBias = 0;
            state.DepthBiasSlope = 0.0f;
            state.DepthBiasClamp = 0.0f;

            RpgRenderTask_CompilePSO task;
            task.Reset();
            task.RootSignature = RootSignatureGraphics.Get();
            task.Name = "ShadowMapDirectional";
            task.PipelineState = state;
            task.Execute();

            GraphicsPSO_ShadowMapDirectional = task.GetCompiledPSO();
        }

        // ShadowDepthCube
        {
            RpgRenderPipelineState state{};
            state.VertexShaderName = RPG_SHADER_NAME_ShadowMapCube_VS;
            state.GeometryShaderName = RPG_SHADER_NAME_ShadowMapCube_GS;
            //state.PixelShaderName = RPG_SHADER_DEFAULT_NAME_ShadowMapCube_PS;
            state.VertexMode = RpgRenderVertexMode::MESH;
            state.RasterMode = RpgRenderRasterMode::SOLID;
            state.BlendMode = RpgRenderColorBlendMode::NONE;
            state.DepthStencilFormat = RpgRender::DEFAULT_FORMAT_SHADOW_DEPTH;
            state.RenderTargetCount = 0;
            state.bDepthTest = true;
            state.bDepthWrite = true;
            state.DepthBias = 1000;
            state.DepthBiasSlope = 2.0f;
            //state.DepthBiasClamp = 4.0f;

            RpgRenderTask_CompilePSO task;
            task.Reset();
            task.RootSignature = RootSignatureGraphics.Get();
            task.Name = "ShadowMapCube";
            task.PipelineState = state;
            task.Execute();

            GraphicsPSO_ShadowMapCube = task.GetCompiledPSO();
        }
    }


    // Compute PSO
    {
        // Skinning
        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc{};
        psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        psoDesc.NodeMask = 0;
        psoDesc.CachedPSO.pCachedBlob = nullptr;
        psoDesc.CachedPSO.CachedBlobSizeInBytes = 0;
        psoDesc.pRootSignature = RootSignatureCompute.Get();

        IDxcBlob* shaderCodeBlob = RpgShaderManager::GetShaderCodeBlob(RPG_SHADER_NAME_ComputeSkinning);
        psoDesc.CS.pShaderBytecode = shaderCodeBlob->GetBufferPointer();
        psoDesc.CS.BytecodeLength = shaderCodeBlob->GetBufferSize();

        RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&ComputePSO_Skinning)));
        RPG_D3D12_SetDebugName(ComputePSO_Skinning, "PSO_ComputeSkinning");

        RPG_LogDebug(RpgLogD3D12, "Compiled compute PSO for (Skinning)");
    }

    bInitialized = true;
}


void RpgRenderPipeline::Shutdown() noexcept
{
    if (!bInitialized)
    {
        return;
    }

    Materials.Clear(true);
    MaterialPipelineStates.Clear(true);
    TaskCompilePSOs.Clear(true);
    ComputePSO_Skinning.Reset();
    GraphicsPSO_ShadowMapDirectional.Reset();
    GraphicsPSO_ShadowMapCube.Reset();
    RootSignatureGraphics.Reset();
    RootSignatureCompute.Reset();

    bInitialized = false;
}


ID3D12RootSignature* RpgRenderPipeline::GetRootSignatureGraphics() noexcept
{
    return RootSignatureGraphics.Get();
}


ID3D12RootSignature* RpgRenderPipeline::GetRootSignatureCompute() noexcept
{
    return RootSignatureCompute.Get();
}


void RpgRenderPipeline::AddMaterials(RpgSharedMaterial* materialArray, int materialCount) noexcept
{
    RPG_Assert(materialArray);
    RPG_Assert(materialCount > 0 && materialCount <= RPG_MAX_COUNT);

    for (int i = 0; i < materialCount; ++i)
    {
        RpgSharedMaterial& mat = materialArray[i];
        RPG_Assert(!mat->IsInstance());
        const int foundAtIndex = Materials.FindIndexByValue(materialArray[i]);

        if (foundAtIndex != RPG_INDEX_INVALID)
        {
            RPG_LogWarn(RpgLogD3D12, "Ignore add material. Material [%s] has been added!", *mat->GetName());
            continue;
        }

        mat->MarkPipelinePending();

        Materials.AddValue(mat);
        MaterialPipelineStates.AddValue(nullptr);
        TaskCompilePSOs.Add();
    }
}


void RpgRenderPipeline::CompileMaterialPSOs(bool bWaitAll) noexcept
{
    RpgArrayInline<RpgThreadTask*, 16> taskToSubmits;

    for (int i = 0; i < Materials.GetCount(); ++i)
    {
        RpgSharedMaterial& mat = Materials[i];
        if (!mat->IsPipelinePending())
        {
            continue;
        }

        RpgRenderTask_CompilePSO& task = TaskCompilePSOs[i];
        task.Reset();
        task.RootSignature = RootSignatureGraphics.Get();
        task.Name = mat->GetName();
        task.PipelineState = mat->GetRenderState();

        mat->MarkPipelineCompiling();

        taskToSubmits.AddValue(&task);
    }

    if (!taskToSubmits.IsEmpty())
    {
        RpgThreadPool::SubmitTasks(taskToSubmits.GetData(), taskToSubmits.GetCount());
    }

    for (int i = 0; i < Materials.GetCount(); ++i)
    {
        RpgSharedMaterial& mat = Materials[i];
        if (!mat->IsPipelineCompiling())
        {
            continue;
        }

        RpgRenderTask_CompilePSO& task = TaskCompilePSOs[i];
        bool bCompileFinished = task.IsDone();

        if (!bCompileFinished && bWaitAll)
        {
            task.Wait();
            bCompileFinished = true;
        }

        if (bCompileFinished)
        {
            MaterialPipelineStates[i] = task.GetCompiledPSO();
            task.Reset();
            mat->MarkPipelineCompiled();
        }
    }
}


ID3D12PipelineState* RpgRenderPipeline::GetMaterialPSO(const RpgSharedMaterial& material) noexcept
{
    const RpgSharedMaterial& checkMaterial = material->IsInstance() ? material->GetParentMaterial() : material;
    const int index = Materials.FindIndexByValue(checkMaterial);
    RPG_Assert(index != RPG_INDEX_INVALID);

    return MaterialPipelineStates[index].Get();
}


ID3D12PipelineState* RpgRenderPipeline::GetGraphicsPSO_ShadowMapDirectional() noexcept
{
    return GraphicsPSO_ShadowMapDirectional.Get();
}


ID3D12PipelineState* RpgRenderPipeline::GetGraphicsPSO_ShadowMapCube() noexcept
{
    return GraphicsPSO_ShadowMapCube.Get();
}


ID3D12PipelineState* RpgRenderPipeline::GetComputePSO_Skinning() noexcept
{
    return ComputePSO_Skinning.Get();
}
