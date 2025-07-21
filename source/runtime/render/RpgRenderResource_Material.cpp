#include "RpgRenderResource.h"
#include "RpgRenderPipeline.h"



RpgMaterialResource::RpgMaterialResource() noexcept
{

}


void RpgMaterialResource::UpdateResources(int frameIndex) noexcept
{
	if (Materials.IsEmpty())
	{
		return;
	}

	const int count = Materials.GetCount();
	VectorScalarData.Resize(count);
	ParameterRootConstants.Resize(count);

	for (int m = 0; m < count; ++m)
	{
		RpgSharedMaterial& material = Materials[m];

		RpgShaderMaterialParameter& parameterRootConstant = ParameterRootConstants[m];
		RpgPlatformMemory::MemSet(&parameterRootConstant, RPG_INDEX_INVALID, sizeof(RpgShaderMaterialParameter));
		int* parameterRootConstantTextureIndexes = reinterpret_cast<int*>(&parameterRootConstant);


		RpgMaterialParameterTextureArray& materialParameterTextures = material->ParameterTexturesReadLock();
		for (int t = 0; t < materialParameterTextures.GetCount(); ++t)
		{
			RPG_Check(parameterRootConstantTextureIndexes[t] == RPG_INDEX_INVALID);

			RpgSharedTexture2D& texture = materialParameterTextures[t];
			if (!texture)
			{
				continue;
			}

			int texIndex = RPG_INDEX_INVALID;
			if (TextureDescriptors.AddUnique(FTextureDescriptor(texture), &texIndex))
			{
				FTextureDescriptor& td = TextureDescriptors[texIndex];
				RpgSharedTexture2D sharedTexture = td.WeakTexture.AsShared();
				RPG_Check(sharedTexture == texture);

				sharedTexture->GPU_UpdateResource();

				if (sharedTexture->IsDirty())
				{
					sharedTexture->GPU_SetLoading();
				}

				td.Descriptor = RpgD3D12::AllocateDescriptor_TDI(frameIndex, sharedTexture->GPU_GetResource());
			}

			RPG_Check(texIndex != RPG_INDEX_INVALID);
			RPG_Check(texture == TextureDescriptors[texIndex].WeakTexture);
			parameterRootConstantTextureIndexes[t] = TextureDescriptors[texIndex].Descriptor.Index;
		}
		material->ParameterTexturesReadUnlock();


		RpgShaderMaterialVectorScalarData& vectorScalar = VectorScalarData[m];

		const RpgMaterialParameterVectorArray& materialVectors = material->ParameterVectorsReadLock();
		for (int v = 0; v < materialVectors.GetCount(); ++v)
		{
			RpgPlatformMemory::MemCopy(vectorScalar.Vectors + v, &materialVectors[v].Value, sizeof(float) * 4);
		}
		material->ParameterVectorsReadUnlock();


		const RpgMaterialParameterScalarArray& materialScalars = material->ParameterScalarsReadLock();
		for (int s = 0; s < materialScalars.GetCount(); ++s)
		{
			vectorScalar.Scalars[s] = materialScalars[s].Value;
		}
		material->ParameterScalarsReadUnlock();


		parameterRootConstant.VectorScalarValueIndex = m;
	}

	RpgD3D12::ResizeBuffer(VectorScalarStructBuffer, VectorScalarData.GetMemorySizeBytes_Allocated(), false);
	RPG_D3D12_SetDebugNameAllocation(VectorScalarStructBuffer, "RES_MaterialVectorScalarStructBuffer");
}


void RpgMaterialResource::CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept
{
	if (Materials.IsEmpty())
	{
		return;
	}


	// Copy material vector-scalar data
	const size_t materialVectorScalarSizeBytes = VectorScalarData.GetMemorySizeBytes_Allocated();
	if (materialVectorScalarSizeBytes > 0)
	{
		RpgD3D12::ResizeBuffer(MaterialStagingBuffer, materialVectorScalarSizeBytes, true);
		RPG_D3D12_SetDebugNameAllocation(MaterialStagingBuffer, "STG_MaterialVectorScalarData");

		void* stagingMap = RpgD3D12::MapBuffer(MaterialStagingBuffer.Get());
		RpgPlatformMemory::MemCopy(stagingMap, VectorScalarData.GetData(), materialVectorScalarSizeBytes);
		cmdList->CopyBufferRegion(VectorScalarStructBuffer->GetResource(), 0, MaterialStagingBuffer->GetResource(), 0, materialVectorScalarSizeBytes);
		RpgD3D12::UnmapBuffer(MaterialStagingBuffer.Get());
	}


	// Calculate texture staging size bytes
	for (int t = 0; t < TextureDescriptors.GetCount(); ++t)
	{
		RpgSharedTexture2D texture = TextureDescriptors[t].WeakTexture.AsShared();
		RPG_Check(texture.IsValid());

		if (!texture->GPU_IsLoading())
		{
			continue;
		}

		UploadTextureIndices.AddValue(t);
	}

	for (int i = 0; i < UploadTextureIndices.GetCount(); ++i)
	{
		const int index = UploadTextureIndices[i];
		RpgSharedTexture2D texture = TextureDescriptors[index].WeakTexture.AsShared();
		RPG_CheckV(texture.IsValid() && texture->IsDirty(), "Texture: %s", *texture->GetName());

		texture->GPU_CommandCopy(cmdList);
		texture->GPU_SetLoaded();

		RPG_LogDebug(RpgLogTemp, "Texture loaded to GPU (%s)", *texture->GetName());
	}
}


void RpgMaterialResource::CommandBindShaderResources(ID3D12GraphicsCommandList* cmdList) const noexcept
{
	RPG_Check(!Materials.IsEmpty());

	if (VectorScalarData.GetCount() > 0)
	{
		cmdList->SetGraphicsRootShaderResourceView(RpgRenderPipeline::GRPI_MATERIAL_VECTOR_SCALAR_DATA, VectorScalarStructBuffer->GetResource()->GetGPUVirtualAddress());
	}
}


void RpgMaterialResource::CommandBindMaterial(ID3D12GraphicsCommandList* cmdList, FMaterialID materialId) const noexcept
{
	RPG_Check(!Materials.IsEmpty());
	RPG_Check(materialId >= 0 && materialId < Materials.GetCount());

	const RpgSharedMaterial& material = Materials[materialId];

	ID3D12PipelineState* pipelineState = RpgRenderPipeline::GetMaterialPSO(material);
	cmdList->SetPipelineState(pipelineState);

	const RpgShaderMaterialParameter& param = ParameterRootConstants[materialId];
	cmdList->SetGraphicsRoot32BitConstants(RpgRenderPipeline::GRPI_MATERIAL_PARAM, sizeof(RpgShaderMaterialParameter) / 4, &param, 0);
}
