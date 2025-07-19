#include "RpgRenderResource.h"



RpgMeshResource::RpgMeshResource() noexcept
{
	TotalVertexCount = 0;
	TotalIndexCount = 0;
}


RpgMeshResource::FMeshID RpgMeshResource::AddMesh(const RpgSharedMesh& mesh, int& out_IndexCount, int& out_IndexStart, int& out_IndexVertexOffset) noexcept
{
	FMeshID id = MeshDatas.FindIndexByPredicate([&](const FMeshData& check) { return check.Mesh == mesh; } );

	if (id == RPG_INDEX_INVALID)
	{
		id = MeshDatas.GetCount();

		FMeshData& data = MeshDatas.Add();
		data.Mesh = mesh;
		data.VertexStart = TotalVertexCount;
		data.VertexCount = mesh->GetVertexCount();
		data.IndexStart = TotalIndexCount;
		data.IndexCount = mesh->GetIndexCount();

		TotalVertexCount += data.VertexCount;	
		TotalIndexCount += data.IndexCount;
	}

	const FMeshData& data = MeshDatas[id];
	out_IndexCount = data.IndexCount;
	out_IndexStart = data.IndexStart;
	out_IndexVertexOffset = data.VertexStart;

	return id;
}


void RpgMeshResource::UpdateResources() noexcept
{
	if (MeshDatas.IsEmpty())
	{
		return;
	}

	RpgD3D12::ResizeBuffer(VertexPositionBuffer, sizeof(RpgVertex::FMeshPosition) * TotalVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(VertexPositionBuffer, "RES_Mesh_VtxPos");

	RpgD3D12::ResizeBuffer(VertexNormalTangentBuffer, sizeof(RpgVertex::FMeshNormalTangent) * TotalVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(VertexNormalTangentBuffer, "RES_Mesh_VtxNormTan");

	RpgD3D12::ResizeBuffer(VertexTexCoordBuffer, sizeof(RpgVertex::FMeshTexCoord) * TotalVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(VertexTexCoordBuffer, "RES_Mesh_VtxTexCoord");

	RpgD3D12::ResizeBuffer(IndexBuffer, sizeof(RpgVertex::FIndex) * TotalIndexCount, false);
	RPG_D3D12_SetDebugNameAllocation(IndexBuffer, "RES_Mesh_Idx");
}


void RpgMeshResource::CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept
{
	if (MeshDatas.IsEmpty())
	{
		return;
	}

	const size_t vertexPositionSizeBytes = sizeof(RpgVertex::FMeshPosition) * TotalVertexCount;
	const size_t vertexNormalTangentSizeBytes = sizeof(RpgVertex::FMeshNormalTangent) * TotalVertexCount;
	const size_t vertexTexCoordSizeBytes = sizeof(RpgVertex::FMeshTexCoord) * TotalVertexCount;
	const size_t indexSizeBytes = sizeof(RpgVertex::FIndex) * TotalIndexCount;
	const size_t stagingSizeBytes = vertexPositionSizeBytes + vertexNormalTangentSizeBytes + vertexTexCoordSizeBytes + indexSizeBytes;

	RpgD3D12::ResizeBuffer(StagingBuffer, stagingSizeBytes, true);
	RPG_D3D12_SetDebugNameAllocation(StagingBuffer, "STG_Mesh");

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


		// vertex index
		const size_t srcOffsetIndex = stagingOffset;
		for (int i = 0; i < MeshDatas.GetCount(); ++i)
		{
			MeshDatas[i].Mesh->CopyIndexData(stagingMap, stagingOffset);
		}
		cmdList->CopyBufferRegion(IndexBuffer->GetResource(), 0, stagingResource, srcOffsetIndex, indexSizeBytes);


		// Sanity check 
		RPG_Check(stagingOffset == stagingSizeBytes);
	}
	RpgD3D12::UnmapBuffer(StagingBuffer.Get());
}
