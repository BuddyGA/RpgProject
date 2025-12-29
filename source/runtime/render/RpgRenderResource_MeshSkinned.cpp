#include "RpgRenderResource.h"



RpgMeshSkinnedResource::RpgMeshSkinnedResource() noexcept
{
	MeshVertexCount = 0;
	MeshIndexCount = 0;
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
		data.VertexStart = MeshVertexCount;
		data.MeshVertexCount = mesh->GetVertexCount();
		data.IndexStart = MeshIndexCount;
		data.MeshIndexCount = mesh->GetIndexCount();

		MeshVertexCount += data.MeshVertexCount;
		MeshIndexCount += data.MeshIndexCount;
	}

	const FMeshData& data = MeshDatas[id];
	out_IndexCount = data.MeshIndexCount;
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
	param.VertexCount = meshData.MeshVertexCount;
	param.IndexStart = meshData.IndexStart;
	param.IndexCount = meshData.MeshIndexCount;
	param.SkeletonIndex = id;

	SkeletonBoneSkinningTransforms.InsertAtRange(boneSkinningTransforms, RPG_INDEX_LAST);

	return id;
}


void RpgMeshSkinnedResource::UpdateResources() noexcept
{
	if (MeshDatas.IsEmpty())
	{
		return;
	}

	RpgD3D12::ResizeBuffer(MeshVertexPositionBuffer, sizeof(RpgVertex::FMeshPosition) * MeshVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(MeshVertexPositionBuffer, "RES_MeshSkin_VtxPos");

	RpgD3D12::ResizeBuffer(MeshVertexNormalTangentBuffer, sizeof(RpgVertex::FMeshNormalTangent) * MeshVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(MeshVertexNormalTangentBuffer, "RES_MeshSkin_VtxNormTan");

	RpgD3D12::ResizeBuffer(MeshVertexTexCoordBuffer, sizeof(RpgVertex::FMeshTexCoord) * MeshVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(MeshVertexTexCoordBuffer, "RES_MeshSkin_VtxTexCoord");

	RpgD3D12::ResizeBuffer(VertexSkinBuffer, sizeof(RpgVertex::FMeshSkin) * MeshVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(VertexSkinBuffer, "RES_MeshSkin_VtxSkin");

	RpgD3D12::ResizeBuffer(MeshIndexBuffer, sizeof(RpgVertex::FIndex) * MeshIndexCount, false);
	RPG_D3D12_SetDebugNameAllocation(MeshIndexBuffer, "RES_MeshSkin_Idx");

	RpgD3D12::ResizeBuffer(SkeletonBoneSkinningBuffer, SkeletonBoneSkinningTransforms.GetMemorySizeBytes_Allocated(), false);
	RPG_D3D12_SetDebugNameAllocation(SkeletonBoneSkinningBuffer, "RES_MeshSkin_SkelBone");

	for (int i = 0; i < MeshDatas.GetCount(); ++i)
	{
		const FMeshData& data = MeshDatas[i];
		SkinnedVertexCount += data.MeshVertexCount * data.InstanceCount;
		SkinnedIndexCount += data.MeshIndexCount * data.InstanceCount;
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

	const size_t vertexPositionSizeBytes = sizeof(RpgVertex::FMeshPosition) * MeshVertexCount;
	const size_t vertexNormalTangentSizeBytes = sizeof(RpgVertex::FMeshNormalTangent) * MeshVertexCount;
	const size_t vertexTexCoordSizeBytes = sizeof(RpgVertex::FMeshTexCoord) * MeshVertexCount;
	const size_t vertexSkinSizeBytes = sizeof(RpgVertex::FMeshSkin) * MeshVertexCount;
	const size_t indexSizeBytes = sizeof(RpgVertex::FIndex) * MeshIndexCount;
	const size_t skeletonBoneSkinningSizeBytes = SkeletonBoneSkinningTransforms.GetMemorySizeBytes_Allocated();
	const size_t stagingSizeBytes = vertexPositionSizeBytes + vertexNormalTangentSizeBytes + vertexTexCoordSizeBytes + vertexSkinSizeBytes + indexSizeBytes + skeletonBoneSkinningSizeBytes;

	RpgD3D12::ResizeBuffer(MeshStagingBuffer, stagingSizeBytes, true);
	RPG_D3D12_SetDebugNameAllocation(MeshStagingBuffer, "STG_MeshSkinning");

	uint8_t* stagingMap = RpgD3D12::MapBuffer<uint8_t>(MeshStagingBuffer.Get());
	{
		ID3D12Resource* stagingResource = MeshStagingBuffer->GetResource();
		size_t stagingOffset = 0;

		// vertex position
		const size_t srcOffsetVertexPosition = stagingOffset;
		for (int i = 0; i < MeshDatas.GetCount(); ++i)
		{
			MeshDatas[i].Mesh->CopyVertexData_Position(stagingMap, stagingOffset);
		}
		cmdList->CopyBufferRegion(MeshVertexPositionBuffer->GetResource(), 0, stagingResource, srcOffsetVertexPosition, vertexPositionSizeBytes);


		// vertex normal-tangent
		const size_t srcOffsetVertexNormalTangent = stagingOffset;
		for (int i = 0; i < MeshDatas.GetCount(); ++i)
		{
			MeshDatas[i].Mesh->CopyVertexData_NormalTangent(stagingMap, stagingOffset);
		}
		cmdList->CopyBufferRegion(MeshVertexNormalTangentBuffer->GetResource(), 0, stagingResource, srcOffsetVertexNormalTangent, vertexNormalTangentSizeBytes);


		// vertex texcoord
		const size_t srcOffsetVertexTexCoord = stagingOffset;
		for (int i = 0; i < MeshDatas.GetCount(); ++i)
		{
			MeshDatas[i].Mesh->CopyVertexData_TexCoord(stagingMap, stagingOffset);
		}
		cmdList->CopyBufferRegion(MeshVertexTexCoordBuffer->GetResource(), 0, stagingResource, srcOffsetVertexTexCoord, vertexTexCoordSizeBytes);


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
		cmdList->CopyBufferRegion(MeshIndexBuffer->GetResource(), 0, stagingResource, srcOffsetIndex, indexSizeBytes);


		// skeleton bone skinning
		const size_t srcOffsetSkeletonBoneSkinning = stagingOffset;
		RpgPlatformMemory::Copy(stagingMap + stagingOffset, SkeletonBoneSkinningTransforms.GetData(), skeletonBoneSkinningSizeBytes);
		cmdList->CopyBufferRegion(SkeletonBoneSkinningBuffer->GetResource(), 0, stagingResource, srcOffsetSkeletonBoneSkinning, skeletonBoneSkinningSizeBytes);
		stagingOffset += skeletonBoneSkinningSizeBytes;

		// Sanity check 
		RPG_Check(stagingOffset == stagingSizeBytes);	
	}
	RpgD3D12::UnmapBuffer(MeshStagingBuffer.Get());


	// transition original vertex (texcoord, index) to COPY_SOURCE
	D3D12_RESOURCE_BARRIER copySourceTransitionBarriers[2] =
	{
		RpgD3D12::CreateResourceBarrier_Transition(MeshVertexTexCoordBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE),
		RpgD3D12::CreateResourceBarrier_Transition(MeshIndexBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_SOURCE),
	};
	cmdList->ResourceBarrier(2, copySourceTransitionBarriers);

	// copy original vertex texcoord to skinned
	for (int i = 0; i < ObjectParameters.GetCount(); ++i)
	{
		const RpgShaderSkinnedObjectParameter& param = ObjectParameters[i];

		const size_t dstTexCoordOffset = sizeof(RpgVertex::FMeshTexCoord) * param.SkinnedVertexStart;
		const size_t srcTexCoordOffset = sizeof(RpgVertex::FMeshTexCoord) * param.VertexStart;
		const size_t texCoordSizeBytes = sizeof(RpgVertex::FMeshTexCoord) * param.VertexCount;
		cmdList->CopyBufferRegion(SkinnedVertexTexCoordBuffer->GetResource(), dstTexCoordOffset, MeshVertexTexCoordBuffer->GetResource(), srcTexCoordOffset, texCoordSizeBytes);
	}

	// copy original vertex index to skinned
	for (int i = 0; i < ObjectParameters.GetCount(); ++i)
	{
		const RpgShaderSkinnedObjectParameter& param = ObjectParameters[i];

		const size_t dstIndexOffset = sizeof(RpgVertex::FIndex) * param.SkinnedIndexStart;
		const size_t srcIndexOffset = sizeof(RpgVertex::FIndex) * param.IndexStart;
		const size_t indexSizeBytes = sizeof(RpgVertex::FIndex) * param.IndexCount;
		cmdList->CopyBufferRegion(SkinnedIndexBuffer->GetResource(), dstIndexOffset, MeshIndexBuffer->GetResource(), srcIndexOffset, indexSizeBytes);
	}
}
