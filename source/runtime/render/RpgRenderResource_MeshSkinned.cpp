#include "RpgRenderResource.h"



RpgMeshSkinnedResource::RpgMeshSkinnedResource() noexcept
{
	VertexCount = 0;
	IndexCount = 0;
	SkinnedVertexCount = 0;
	SkinnedIndexCount = 0;
}


RpgMeshSkinnedResource::FMeshID RpgMeshSkinnedResource::AddMesh(const RpgSharedMesh& mesh, int& out_IndexCount, int& out_IndexStart, int& out_IndexVertexOffset) noexcept
{
	FMeshID id = MeshDatas.FindIndexByPredicate([&](const FMeshData& check) { return check.Mesh == mesh; } );

	if (id == RPG_INDEX_INVALID)
	{
		id = MeshDatas.GetCount();

		FMeshData& data = MeshDatas.Add();
		data.Mesh = mesh;
		data.VertexStart = VertexCount;
		data.VertexCount = mesh->GetVertexCount();
		data.IndexStart = IndexCount;
		data.IndexCount = mesh->GetIndexCount();

		VertexCount += data.VertexCount;
		IndexCount += data.IndexCount;
	}

	const FMeshData& data = MeshDatas[id];
	out_IndexCount = data.IndexCount;
	out_IndexStart = data.IndexStart;
	out_IndexVertexOffset = data.VertexStart;

	return id;
}


RpgMeshSkinnedResource::FSkeletonID RpgMeshSkinnedResource::AddObjectBoneSkinningTransforms(FMeshID meshId, const RpgArray<RpgMatrixTransform>& boneSkinningTransforms) noexcept
{
	const FSkeletonID id = SkeletonBoneSkinningTransforms.GetCount();
	
	FMeshData& meshData = MeshDatas[meshId];
	meshData.InstanceCount++;

	RpgShaderSkinnedObjectParameter& param = ObjectParameters.Add();
	param.VertexStart = meshData.VertexStart;
	param.VertexCount = meshData.VertexCount;
	param.IndexStart = meshData.IndexStart;
	param.IndexCount = meshData.IndexCount;
	param.SkeletonIndex = id;

	for (int b = 0; b < boneSkinningTransforms.GetCount(); ++b)
	{
		SkeletonBoneSkinningTransforms.AddValue(boneSkinningTransforms[b].Xmm);
	}

	return id;
}


void RpgMeshSkinnedResource::UpdateResources() noexcept
{
	if (MeshDatas.IsEmpty())
	{
		return;
	}

	RpgD3D12::ResizeBuffer(VertexPositionBuffer, sizeof(RpgVertex::FMeshPosition) * VertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(VertexPositionBuffer, "RES_MeshSkin_VtxPos");

	RpgD3D12::ResizeBuffer(VertexNormalTangentBuffer, sizeof(RpgVertex::FMeshNormalTangent) * VertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(VertexNormalTangentBuffer, "RES_MeshSkin_VtxNormTan");

	RpgD3D12::ResizeBuffer(VertexTexCoordBuffer, sizeof(RpgVertex::FMeshTexCoord) * VertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(VertexTexCoordBuffer, "RES_MeshSkin_VtxTexCoord");

	RpgD3D12::ResizeBuffer(VertexSkinBuffer, sizeof(RpgVertex::FMeshSkin) * VertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(VertexSkinBuffer, "RES_MeshSkin_VtxSkin");

	RpgD3D12::ResizeBuffer(IndexBuffer, sizeof(RpgVertex::FIndex) * IndexCount, false);
	RPG_D3D12_SetDebugNameAllocation(IndexBuffer, "RES_MeshSkin_Idx");

	RpgD3D12::ResizeBuffer(SkeletonBoneSkinningBuffer, SkeletonBoneSkinningTransforms.GetMemorySizeBytes_Allocated(), false);
	RPG_D3D12_SetDebugNameAllocation(SkeletonBoneSkinningBuffer, "RES_MeshSkin_SkelBone");

	for (int i = 0; i < MeshDatas.GetCount(); ++i)
	{
		const FMeshData& data = MeshDatas[i];
		SkinnedVertexCount += data.VertexCount * data.InstanceCount;
		SkinnedIndexCount += data.IndexCount * data.InstanceCount;
	}

	int skinnedVtxOffset = 0;
	int skinnedIdxOffset = 0;

	for (int i = 0; i < ObjectParameters.GetCount(); ++i)
	{
		RpgShaderSkinnedObjectParameter& param = ObjectParameters[i];
		param.SkinnedVertexStart = skinnedVtxOffset;
		param.SkinnedIndexStart = skinnedIdxOffset;

		skinnedVtxOffset += param.VertexCount;
		skinnedIdxOffset += param.IndexCount;
	}

	RPG_Check(skinnedVtxOffset == SkinnedVertexCount);
	RPG_Check(skinnedIdxOffset == SkinnedIndexCount);

	RpgD3D12::ResizeBuffer(SkinnedVertexPositionBuffer, sizeof(RpgVertex::FMeshPosition) * SkinnedVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(SkinnedVertexPositionBuffer, "RES_MeshSkin_SkinnedVtxPos");

	RpgD3D12::ResizeBuffer(SkinnedVertexNormalTangentBuffer, sizeof(RpgVertex::FMeshNormalTangent) * SkinnedVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(SkinnedVertexNormalTangentBuffer, "RES_MeshSkin_SkinnnedVtxNormTan");

	RpgD3D12::ResizeBuffer(SkinnedVertexTexCoordBuffer, sizeof(RpgVertex::FMeshTexCoord) * SkinnedVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(SkinnedVertexTexCoordBuffer, "RES_MeshSkin_SkinnedVtxTexCoord");

	RpgD3D12::ResizeBuffer(SkinnedIndexBuffer, sizeof(RpgVertex::FIndex) * SkinnedIndexCount, false);
	RPG_D3D12_SetDebugNameAllocation(SkinnedIndexBuffer, "RES_MeshSkin_SkinnedIdx");
}


void RpgMeshSkinnedResource::CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept
{
	if (MeshDatas.IsEmpty())
	{
		return;
	}

	const size_t vertexPositionSizeBytes = sizeof(RpgVertex::FMeshPosition) * VertexCount;
	const size_t vertexNormalTangentSizeBytes = sizeof(RpgVertex::FMeshNormalTangent) * VertexCount;
	const size_t vertexTexCoordSizeBytes = sizeof(RpgVertex::FMeshTexCoord) * VertexCount;
	const size_t vertexSkinSizeBytes = sizeof(RpgVertex::FMeshSkin) * VertexCount;
	const size_t indexSizeBytes = sizeof(RpgVertex::FIndex) * IndexCount;
	const size_t skeletonBoneSkinningSizeBytes = SkeletonBoneSkinningTransforms.GetMemorySizeBytes_Allocated();
	const size_t stagingSizeBytes = vertexPositionSizeBytes + vertexNormalTangentSizeBytes + vertexTexCoordSizeBytes + vertexSkinSizeBytes + indexSizeBytes + skeletonBoneSkinningSizeBytes;

	RpgD3D12::ResizeBuffer(StagingBuffer, stagingSizeBytes, true);
	RPG_D3D12_SetDebugNameAllocation(StagingBuffer, "STG_MeshSkinning");

	uint8_t* stagingMap = RpgD3D12::MapBuffer<uint8_t>(StagingBuffer.Get());
	{
		ID3D12Resource* stagingResource = StagingBuffer->GetResource();
		size_t stagingOffset = 0;

		// vertex position
		const size_t srcOffsetVertexPosition = stagingOffset;
		for (int i = 0; i < MeshDatas.GetCount(); ++i)
		{
			MeshDatas[i].Mesh->CopyVertexData_Position(stagingMap, stagingOffset);
		}
		cmdList->CopyBufferRegion(VertexPositionBuffer->GetResource(), 0, stagingResource, srcOffsetVertexPosition, vertexPositionSizeBytes);


		// vertex normal-tangent
		const size_t srcOffsetVertexNormalTangent = stagingOffset;
		for (int i = 0; i < MeshDatas.GetCount(); ++i)
		{
			MeshDatas[i].Mesh->CopyVertexData_NormalTangent(stagingMap, stagingOffset);
		}
		cmdList->CopyBufferRegion(VertexNormalTangentBuffer->GetResource(), 0, stagingResource, srcOffsetVertexNormalTangent, vertexNormalTangentSizeBytes);


		// vertex texcoord
		const size_t srcOffsetVertexTexCoord = stagingOffset;
		for (int i = 0; i < MeshDatas.GetCount(); ++i)
		{
			MeshDatas[i].Mesh->CopyVertexData_TexCoord(stagingMap, stagingOffset);
		}
		cmdList->CopyBufferRegion(VertexTexCoordBuffer->GetResource(), 0, stagingResource, srcOffsetVertexTexCoord, vertexTexCoordSizeBytes);


		// vertex skin
		const size_t srcOffsetVertexSkin = stagingOffset;
		for (int i = 0; i < MeshDatas.GetCount(); ++i)
		{
			MeshDatas[i].Mesh->CopyVertexData_Skin(stagingMap, stagingOffset);
		}
		cmdList->CopyBufferRegion(VertexSkinBuffer->GetResource(), 0, stagingResource, srcOffsetVertexSkin, vertexSkinSizeBytes);


		// vertex index
		const size_t srcOffsetIndex = stagingOffset;
		for (int i = 0; i < MeshDatas.GetCount(); ++i)
		{
			MeshDatas[i].Mesh->CopyIndexData(stagingMap, stagingOffset);
		}
		cmdList->CopyBufferRegion(IndexBuffer->GetResource(), 0, stagingResource, srcOffsetIndex, indexSizeBytes);


		// skeleton bone skinning
		const size_t srcOffsetSkeletonBoneSkinning = stagingOffset;
		RpgPlatformMemory::MemCopy(stagingMap + stagingOffset, SkeletonBoneSkinningTransforms.GetData(), skeletonBoneSkinningSizeBytes);
		cmdList->CopyBufferRegion(SkeletonBoneSkinningBuffer->GetResource(), 0, stagingResource, srcOffsetSkeletonBoneSkinning, skeletonBoneSkinningSizeBytes);
		stagingOffset += skeletonBoneSkinningSizeBytes;

		// Sanity check 
		RPG_Check(stagingOffset == stagingSizeBytes);	
	}
	RpgD3D12::UnmapBuffer(StagingBuffer.Get());


	// transition original vertex (texcoord, index) to COPY_SOURCE
	D3D12_RESOURCE_BARRIER copySourceTransitionBarriers[2] =
	{
		RpgD3D12::CreateResourceBarrier_Transition(VertexTexCoordBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE),
		RpgD3D12::CreateResourceBarrier_Transition(IndexBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE),
	};
	cmdList->ResourceBarrier(2, copySourceTransitionBarriers);

	// copy original vertex texcoord to skinned
	for (int i = 0; i < ObjectParameters.GetCount(); ++i)
	{
		const RpgShaderSkinnedObjectParameter& param = ObjectParameters[i];

		const size_t dstTexCoordOffset = sizeof(RpgVertex::FMeshTexCoord) * param.SkinnedVertexStart;
		const size_t srcTexCoordOffset = sizeof(RpgVertex::FMeshTexCoord) * param.VertexStart;
		const size_t texCoordSizeBytes = sizeof(RpgVertex::FMeshTexCoord) * param.VertexCount;
		cmdList->CopyBufferRegion(SkinnedVertexTexCoordBuffer->GetResource(), dstTexCoordOffset, VertexTexCoordBuffer->GetResource(), srcTexCoordOffset, texCoordSizeBytes);
	}

	// copy original vertex index to skinned
	for (int i = 0; i < ObjectParameters.GetCount(); ++i)
	{
		const RpgShaderSkinnedObjectParameter& param = ObjectParameters[i];

		const size_t dstIndexOffset = sizeof(RpgVertex::FIndex) * param.SkinnedIndexStart;
		const size_t srcIndexOffset = sizeof(RpgVertex::FIndex) * param.IndexStart;
		const size_t indexSizeBytes = sizeof(RpgVertex::FIndex) * param.IndexCount;
		cmdList->CopyBufferRegion(SkinnedIndexBuffer->GetResource(), dstIndexOffset, IndexBuffer->GetResource(), srcIndexOffset, indexSizeBytes);
	}
}
