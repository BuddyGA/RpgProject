#include "RpgRenderResource.h"


RpgMeshResource::RpgMeshResource() noexcept
{
	MeshVertexCount = 0;
	MeshIndexCount = 0;
}


RpgMeshResource::FMeshID RpgMeshResource::AddMesh(const RpgSharedMesh& mesh, int& out_IndexCount, int& out_IndexStart, int& out_IndexVertexOffset) noexcept
{
	FMeshID id = MeshDatas.FindIndexByPredicate([&](const FMeshData& check) { return check.Mesh == mesh; } );

	if (id == RPG_INDEX_INVALID)
	{
		id = MeshDatas.GetCount();

		FMeshData& data = MeshDatas.Add();
		data.Mesh = mesh;
		data.VertexStart = MeshVertexCount;
		data.VertexCount = mesh->GetVertexCount();
		data.IndexStart = MeshIndexCount;
		data.IndexCount = mesh->GetIndexCount();

		MeshVertexCount += data.VertexCount;	
		MeshIndexCount += data.IndexCount;
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

	RpgD3D12::ResizeBuffer(MeshVertexPositionBuffer, sizeof(RpgVertex::FMeshPosition) * MeshVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(MeshVertexPositionBuffer, "RES_Mesh_VtxPos");

	RpgD3D12::ResizeBuffer(MeshVertexNormalTangentBuffer, sizeof(RpgVertex::FMeshNormalTangent) * MeshVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(MeshVertexNormalTangentBuffer, "RES_Mesh_VtxNormTan");

	RpgD3D12::ResizeBuffer(MeshVertexTexCoordBuffer, sizeof(RpgVertex::FMeshTexCoord) * MeshVertexCount, false);
	RPG_D3D12_SetDebugNameAllocation(MeshVertexTexCoordBuffer, "RES_Mesh_VtxTexCoord");

	RpgD3D12::ResizeBuffer(MeshIndexBuffer, sizeof(RpgVertex::FIndex) * MeshIndexCount, false);
	RPG_D3D12_SetDebugNameAllocation(MeshIndexBuffer, "RES_Mesh_Idx");
}


void RpgMeshResource::CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept
{
	if (MeshDatas.IsEmpty())
	{
		return;
	}

	const size_t vertexPositionSizeBytes = sizeof(RpgVertex::FMeshPosition) * MeshVertexCount;
	const size_t vertexNormalTangentSizeBytes = sizeof(RpgVertex::FMeshNormalTangent) * MeshVertexCount;
	const size_t vertexTexCoordSizeBytes = sizeof(RpgVertex::FMeshTexCoord) * MeshVertexCount;
	const size_t indexSizeBytes = sizeof(RpgVertex::FIndex) * MeshIndexCount;
	const size_t stagingSizeBytes = vertexPositionSizeBytes + vertexNormalTangentSizeBytes + vertexTexCoordSizeBytes + indexSizeBytes;

	RpgD3D12::ResizeBuffer(MeshStagingBuffer, stagingSizeBytes, true);
	RPG_D3D12_SetDebugNameAllocation(MeshStagingBuffer, "STG_Mesh");

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


		// vertex index
		const size_t srcOffsetIndex = stagingOffset;
		for (int i = 0; i < MeshDatas.GetCount(); ++i)
		{
			MeshDatas[i].Mesh->CopyIndexData(stagingMap, stagingOffset);
		}
		cmdList->CopyBufferRegion(MeshIndexBuffer->GetResource(), 0, stagingResource, srcOffsetIndex, indexSizeBytes);


		// Sanity check 
		RPG_Check(stagingOffset == stagingSizeBytes);
	}
	RpgD3D12::UnmapBuffer(MeshStagingBuffer.Get());
}
