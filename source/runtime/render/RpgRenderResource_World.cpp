#include "RpgRenderResource.h"
#include "RpgRenderPipeline.h"



RpgWorldResource::RpgWorldResource() noexcept
{
}


void RpgWorldResource::Reset() noexcept
{
	CachedTagLights.Clear();

	WorldData.DeltaTime = 0.0f;
	WorldData.ViewCount = 0;
	WorldData.DirectionalLightCount = 0;
	WorldData.PointLightCount = 0;
	WorldData.SpotLightCount = 0;
	WorldData.AmbientColorStrength = RpgVector4(1.0f, 1.0f, 1.0f, 0.05f).Xmm;

	CachedTagTransforms.Clear();
	TransformDatas.Clear();


#ifndef RPG_BUILD_SHIPPING
	DebugLineVertexSizeBytes = 0;
	DebugLineIndexSizeBytes = 0;
	DebugLine.Clear();
	DebugLineNoDepth.Clear();
#endif // !RPG_BUILD_SHIPPING

}


void RpgWorldResource::UpdateResources() noexcept
{
	RpgD3D12::ResizeBuffer(WorldConstantBuffer, sizeof(RpgShaderWorldData), false);
	RPG_D3D12_SetDebugNameAllocation(WorldConstantBuffer, "RES_WorldConstantBuffer");

	if (!TransformDatas.IsEmpty())
	{
		RpgD3D12::ResizeBuffer(TransformStructBuffer, TransformDatas.GetMemorySizeBytes_Allocated(), false);
		RPG_D3D12_SetDebugNameAllocation(TransformStructBuffer, "RES_TransformStructBuffer");
	}


#ifndef RPG_BUILD_SHIPPING
	DebugLineVertexSizeBytes = 0;
	DebugLineIndexSizeBytes = 0;

	if (!DebugLine.IsEmpty())
	{
		DebugLineVertexSizeBytes += DebugLine.GetVertexSizeBytes();
		DebugLineIndexSizeBytes += DebugLine.GetIndexSizeBytes();
	}

	if (!DebugLineNoDepth.IsEmpty())
	{
		DebugLineVertexSizeBytes += DebugLineNoDepth.GetVertexSizeBytes();
		DebugLineIndexSizeBytes += DebugLineNoDepth.GetIndexSizeBytes();
	}

	if (DebugLineVertexSizeBytes > 0)
	{
		RpgD3D12::ResizeBuffer(DebugLineVertexBuffer, DebugLineVertexSizeBytes, false);
		RPG_D3D12_SetDebugNameAllocation(DebugLineVertexBuffer, "RES_DebugLineVertexBuffer");

		RpgD3D12::ResizeBuffer(DebugLineIndexBuffer, DebugLineIndexSizeBytes, false);
		RPG_D3D12_SetDebugNameAllocation(DebugLineIndexBuffer, "RES_DebugLineIndexBuffer");
	}
#endif // !RPG_BUILD_SHIPPING

}


RpgWorldResource::FTransformID RpgWorldResource::AddTransform(int uniqueTagId, const RpgMatrixTransform& worldTransformMatrix) noexcept
{
	const int tagIndex = CachedTagTransforms.FindIndexByCompare(uniqueTagId);
	if (tagIndex != RPG_INDEX_INVALID)
	{
		return CachedTagTransforms[tagIndex].TransformId;
	}

	FTagTransformID tag;
	tag.TagId = uniqueTagId;
	tag.TransformId = TransformDatas.GetCount();

	CachedTagTransforms.AddValue(tag);
	TransformDatas.AddValue(worldTransformMatrix.Xmm);

	return tag.TransformId;
}


RpgWorldResource::FLightID RpgWorldResource::AddLight_Point(int uniqueTagId, RpgVector3 worldPosition, RpgColorLinear colorIntensity, float attRadius, float attFallOffExp) noexcept
{
	const int tagIndex = CachedTagLights.FindIndexByCompare(uniqueTagId);
	if (tagIndex != RPG_INDEX_INVALID)
	{
		return CachedTagLights[tagIndex].LightId;
	}

	FTagLightID tag;
	tag.TagId = uniqueTagId;
	tag.LightId = RPG_SHADER_LIGHT_POINT_INDEX + WorldData.PointLightCount++;
	CachedTagLights.AddValue(tag);

	RpgShaderLight& data = WorldData.Lights[tag.LightId];
	data.WorldPosition = worldPosition.Xmm;
	data.ColorIntensity = DirectX::XMVectorSet(colorIntensity.R, colorIntensity.G, colorIntensity.B, colorIntensity.A);
	data.AttenuationRadius = attRadius;
	data.AttenuationFallOffExp = attFallOffExp;
	data.ShadowViewIndex = RPG_INDEX_INVALID;
	data.ShadowTextureDescriptorIndex = RPG_INDEX_INVALID;

	return tag.LightId;
}


RpgWorldResource::FLightID RpgWorldResource::AddLight_Spot(int uniqueTagId, RpgVector3 worldPosition, RpgVector3 worldDirection, RpgColorLinear colorIntensity, float attRadius, float attFallOffExp, float innerConeDegree, float outerConeDegree) noexcept
{
	const int tagIndex = CachedTagLights.FindIndexByCompare(uniqueTagId);
	if (tagIndex != RPG_INDEX_INVALID)
	{
		return CachedTagLights[tagIndex].LightId;
	}

	FTagLightID tag;
	tag.TagId = uniqueTagId;
	tag.LightId = RPG_SHADER_LIGHT_SPOT_INDEX + WorldData.SpotLightCount++;
	CachedTagLights.AddValue(tag);

	RpgShaderLight& data = WorldData.Lights[tag.LightId];
	data.WorldPosition = worldPosition.Xmm;
	data.WorldDirection = worldDirection.Xmm;
	data.ColorIntensity = DirectX::XMVectorSet(colorIntensity.R, colorIntensity.G, colorIntensity.B, colorIntensity.A);
	data.AttenuationRadius = attRadius;
	data.AttenuationFallOffExp = attFallOffExp;
	data.SpotLightInnerConeRadian = RpgMath::DegToRad(innerConeDegree);
	data.SpotLightOuterConeRadian = RpgMath::DegToRad(outerConeDegree);
	data.ShadowViewIndex = RPG_INDEX_INVALID;
	data.ShadowTextureDescriptorIndex = RPG_INDEX_INVALID;

	return tag.LightId;
}


void RpgWorldResource::CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept
{
	const size_t worldDataSizeBytes = sizeof(RpgShaderWorldData);
	const size_t transformDataSizeBytes = TransformDatas.GetMemorySizeBytes_Allocated();


#ifndef RPG_BUILD_SHIPPING
	const size_t stagingSizeBytes = worldDataSizeBytes + transformDataSizeBytes + DebugLineVertexSizeBytes + DebugLineIndexSizeBytes;

#else
	const size_t stagingSizeBytes = worldDataSizeBytes + transformDataSizeBytes;

#endif // !RPG_BUILD_SHIPPING


	RpgD3D12::ResizeBuffer(StagingBuffer, stagingSizeBytes, true);
	RPG_D3D12_SetDebugNameAllocation(StagingBuffer, "STG_WorldResource");

	ID3D12Resource* stagingResource = StagingBuffer->GetResource();
	uint8_t* stagingMap = RpgD3D12::MapBuffer<uint8_t>(StagingBuffer.Get());
	size_t stagingOffset = 0;


	// Local function helper to copy data to staging buffer and command copy buffer
	auto LocalFunc_CopyStaging_CopyBuffer = [&](const void* data, size_t sizeBytes, ID3D12Resource* dstResource, size_t dstOffset)
	{
		RpgPlatformMemory::MemCopy(stagingMap + stagingOffset, data, sizeBytes);
		cmdList->CopyBufferRegion(dstResource, dstOffset, stagingResource, stagingOffset, sizeBytes);
		stagingOffset += sizeBytes;
	};


	LocalFunc_CopyStaging_CopyBuffer(&WorldData, worldDataSizeBytes, WorldConstantBuffer->GetResource(), 0);

	if (transformDataSizeBytes > 0)
	{
		LocalFunc_CopyStaging_CopyBuffer(TransformDatas.GetData(), transformDataSizeBytes, TransformStructBuffer->GetResource(), 0);
	}


#ifndef RPG_BUILD_SHIPPING
	size_t debugDstVertexOffset = 0;
	size_t debugDstIndexOffset = 0;

	if (!DebugLine.IsEmpty())
	{
		const size_t vertexSizeBytes = DebugLine.GetVertexSizeBytes();
		LocalFunc_CopyStaging_CopyBuffer(DebugLine.GetVertexData(), vertexSizeBytes, DebugLineVertexBuffer->GetResource(), debugDstVertexOffset);
		debugDstVertexOffset += vertexSizeBytes;

		const size_t indexSizeBytes = DebugLine.GetIndexSizeBytes();
		LocalFunc_CopyStaging_CopyBuffer(DebugLine.GetIndexData(), indexSizeBytes, DebugLineIndexBuffer->GetResource(), debugDstIndexOffset);
		debugDstIndexOffset += indexSizeBytes;
	}

	if (!DebugLineNoDepth.IsEmpty())
	{
		const size_t vertexSizeBytes = DebugLineNoDepth.GetVertexSizeBytes();
		LocalFunc_CopyStaging_CopyBuffer(DebugLineNoDepth.GetVertexData(), vertexSizeBytes, DebugLineVertexBuffer->GetResource(), debugDstVertexOffset);
		debugDstVertexOffset += vertexSizeBytes;

		const size_t indexSizeBytes = DebugLineNoDepth.GetIndexSizeBytes();
		LocalFunc_CopyStaging_CopyBuffer(DebugLineNoDepth.GetIndexData(), indexSizeBytes, DebugLineIndexBuffer->GetResource(), debugDstIndexOffset);
		debugDstIndexOffset += indexSizeBytes;
	}
#endif // !RPG_BUILD_SHIPPING


	RPG_Check(stagingSizeBytes == stagingOffset);
	RpgD3D12::UnmapBuffer(StagingBuffer.Get());
}


void RpgWorldResource::CommandBindShaderResources(ID3D12GraphicsCommandList* cmdList) const noexcept
{
	cmdList->SetGraphicsRootConstantBufferView(RpgRenderPipeline::GRPI_WORLD_DATA, WorldConstantBuffer->GetResource()->GetGPUVirtualAddress());

	if (TransformStructBuffer)
	{
		cmdList->SetGraphicsRootShaderResourceView(RpgRenderPipeline::GRPI_TRANSFORM_DATA, TransformStructBuffer->GetResource()->GetGPUVirtualAddress());
	}
}



#ifndef RPG_BUILD_SHIPPING

void RpgWorldResource::Debug_CommandDrawIndexed_Line(ID3D12GraphicsCommandList* cmdList, const RpgMaterialResource* materialResource, RpgMaterialResource::FMaterialID materialId, RpgMaterialResource::FMaterialID noDepthMaterialId, FViewID cameraId) const noexcept
{
	if (DebugLine.IsEmpty() && DebugLineNoDepth.IsEmpty())
	{
		return;
	}

	// Bind vertex buffer
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = DebugLineVertexBuffer->GetResource()->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(RpgVertex::FPrimitive);
	vertexBufferView.SizeInBytes = static_cast<UINT>(DebugLineVertexSizeBytes);
	cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);

	// Bind index buffer
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	indexBufferView.BufferLocation = DebugLineIndexBuffer->GetResource()->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = static_cast<UINT>(DebugLineIndexSizeBytes);
	cmdList->IASetIndexBuffer(&indexBufferView);

	// Set topology linelist
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	// Bind root constant object param
	RpgShaderObjectParameter param;
	param.ViewIndex = cameraId;
	param.TransformIndex = RPG_INDEX_INVALID;
	cmdList->SetGraphicsRoot32BitConstants(RpgRenderPipeline::GRPI_OBJECT_PARAM, sizeof(RpgShaderObjectParameter) / sizeof(uint32_t), &param, 0);

	if (!DebugLine.IsEmpty())
	{
		materialResource->CommandBindMaterial(cmdList, materialId);
		cmdList->DrawIndexedInstanced(DebugLine.GetIndexCount(), 1, 0, 0, 0);
	}

	if (!DebugLineNoDepth.IsEmpty())
	{
		materialResource->CommandBindMaterial(cmdList, noDepthMaterialId);
		cmdList->DrawIndexedInstanced(DebugLineNoDepth.GetIndexCount(), 1, DebugLine.GetIndexCount(), DebugLine.GetVertexCount(), 0);
	}
}

#endif // !RPG_BUILD_SHIPPING
