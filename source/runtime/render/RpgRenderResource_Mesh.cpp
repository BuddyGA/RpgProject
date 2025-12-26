#include "RpgRenderResource.h"



RpgMeshResource::RpgMeshResource() noexcept
{
	MeshVertexCount = 0;
	MeshIndexCount = 0;
	TerrainVertexCount = 0;
	TerrainIndexCount = 0;
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


RpgMeshResource::FTerrainID RpgMeshResource::AddTerrain(const RpgVertexMeshPositionArray* vertexPositions, const RpgVertexMeshNormalTangentArray* vertexNormalTangents, const RpgVertexMeshTexCoordArray* vertexTexCoords, const RpgVertexIndexArray* indices, int& out_IndexCount, int& out_IndexStart, int& out_IndexVertexOffset) noexcept
{
	const FTerrainID id = TerrainDatas.GetCount();

	FTerrainData& data = TerrainDatas.Add();
	data.VertexPositions = vertexPositions;
	data.VertexNormalTangents = vertexNormalTangents;
	data.VertexTexCoords = vertexTexCoords;
	data.VertexIndices = indices;
	data.VertexStart = TerrainVertexCount;
	data.VertexCount = vertexPositions->GetCount();
	data.IndexStart = TerrainIndexCount;
	data.IndexCount = indices->GetCount();

	TerrainVertexCount += data.VertexCount;
	TerrainIndexCount += data.IndexCount;

	out_IndexCount = data.IndexCount;
	out_IndexStart = data.IndexStart;
	out_IndexVertexOffset = data.VertexStart;

	return id;
}


void RpgMeshResource::UpdateResources() noexcept
{
	if (!MeshDatas.IsEmpty())
	{
		RpgD3D12::ResizeBuffer(MeshVertexPositionBuffer, sizeof(RpgVertex::FMeshPosition) * MeshVertexCount, false);
		RPG_D3D12_SetDebugNameAllocation(MeshVertexPositionBuffer, "RES_Mesh_VtxPos");

		RpgD3D12::ResizeBuffer(MeshVertexNormalTangentBuffer, sizeof(RpgVertex::FMeshNormalTangent) * MeshVertexCount, false);
		RPG_D3D12_SetDebugNameAllocation(MeshVertexNormalTangentBuffer, "RES_Mesh_VtxNormTan");

		RpgD3D12::ResizeBuffer(MeshVertexTexCoordBuffer, sizeof(RpgVertex::FMeshTexCoord) * MeshVertexCount, false);
		RPG_D3D12_SetDebugNameAllocation(MeshVertexTexCoordBuffer, "RES_Mesh_VtxTexCoord");

		RpgD3D12::ResizeBuffer(MeshIndexBuffer, sizeof(RpgVertex::FIndex) * MeshIndexCount, false);
		RPG_D3D12_SetDebugNameAllocation(MeshIndexBuffer, "RES_Mesh_Idx");
	}

	if (!TerrainDatas.IsEmpty())
	{
		RpgD3D12::ResizeBuffer(TerrainVertexPositionBuffer, sizeof(RpgVertex::FMeshPosition) * TerrainVertexCount, false);
		RPG_D3D12_SetDebugNameAllocation(TerrainVertexPositionBuffer, "RES_Terrain_VtxPos");

		RpgD3D12::ResizeBuffer(TerrainVertexNormalTangentBuffer, sizeof(RpgVertex::FMeshNormalTangent) * TerrainVertexCount, false);
		RPG_D3D12_SetDebugNameAllocation(TerrainVertexNormalTangentBuffer, "RES_Terrain_VtxNormTan");

		RpgD3D12::ResizeBuffer(TerrainVertexTexCoordBuffer, sizeof(RpgVertex::FMeshTexCoord) * TerrainVertexCount, false);
		RPG_D3D12_SetDebugNameAllocation(TerrainVertexTexCoordBuffer, "RES_Terrain_VtxTexCoord");

		RpgD3D12::ResizeBuffer(TerrainIndexBuffer, sizeof(RpgVertex::FIndex) * TerrainIndexCount, false);
		RPG_D3D12_SetDebugNameAllocation(TerrainIndexBuffer, "RES_Terrain_Idx");
	}
}


void RpgMeshResource::CommandCopy(ID3D12GraphicsCommandList* cmdList) noexcept
{
	if (!MeshDatas.IsEmpty())
	{
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


	if (!TerrainDatas.IsEmpty())
	{
		const size_t vertexPositionSizeBytes = sizeof(RpgVertex::FMeshPosition) * TerrainVertexCount;
		const size_t vertexNormalTangentSizeBytes = sizeof(RpgVertex::FMeshNormalTangent) * TerrainVertexCount;
		const size_t vertexTexCoordSizeBytes = sizeof(RpgVertex::FMeshTexCoord) * TerrainVertexCount;
		const size_t indexSizeBytes = sizeof(RpgVertex::FIndex) * TerrainIndexCount;
		const size_t stagingSizeBytes = vertexPositionSizeBytes + vertexNormalTangentSizeBytes + vertexTexCoordSizeBytes + indexSizeBytes;

		RpgD3D12::ResizeBuffer(TerrainStagingBuffer, stagingSizeBytes, true);
		RPG_D3D12_SetDebugNameAllocation(TerrainStagingBuffer, "STG_Terrain");

		uint8_t* stagingMap = RpgD3D12::MapBuffer<uint8_t>(TerrainStagingBuffer.Get());
		{
			ID3D12Resource* stagingResource = TerrainStagingBuffer->GetResource();
			size_t stagingOffset = 0;

			// vertex position
			const size_t srcOffsetVertexPosition = stagingOffset;
			for (int i = 0; i < TerrainDatas.GetCount(); ++i)
			{
				const FTerrainData& data = TerrainDatas[i];
				const size_t sizeBytes = data.VertexPositions->GetMemorySizeBytes_Allocated();
				RpgPlatformMemory::Copy(stagingMap + stagingOffset, data.VertexPositions->GetData(), sizeBytes);
				stagingOffset += sizeBytes;
			}
			cmdList->CopyBufferRegion(TerrainVertexPositionBuffer->GetResource(), 0, stagingResource, srcOffsetVertexPosition, vertexPositionSizeBytes);


			// vertex normal-tangent
			const size_t srcOffsetVertexNormalTangent = stagingOffset;
			for (int i = 0; i < TerrainDatas.GetCount(); ++i)
			{
				const FTerrainData& data = TerrainDatas[i];
				const size_t sizeBytes = data.VertexNormalTangents->GetMemorySizeBytes_Allocated();
				RpgPlatformMemory::Copy(stagingMap + stagingOffset, data.VertexNormalTangents->GetData(), sizeBytes);
				stagingOffset += sizeBytes;
			}
			cmdList->CopyBufferRegion(TerrainVertexNormalTangentBuffer->GetResource(), 0, stagingResource, srcOffsetVertexNormalTangent, vertexNormalTangentSizeBytes);


			// vertex texcoord
			const size_t srcOffsetVertexTexCoord = stagingOffset;
			for (int i = 0; i < TerrainDatas.GetCount(); ++i)
			{
				const FTerrainData& data = TerrainDatas[i];
				const size_t sizeBytes = data.VertexTexCoords->GetMemorySizeBytes_Allocated();
				RpgPlatformMemory::Copy(stagingMap + stagingOffset, data.VertexTexCoords->GetData(), sizeBytes);
				stagingOffset += sizeBytes;
			}
			cmdList->CopyBufferRegion(TerrainVertexTexCoordBuffer->GetResource(), 0, stagingResource, srcOffsetVertexTexCoord, vertexTexCoordSizeBytes);


			// vertex index
			const size_t srcOffsetIndex = stagingOffset;
			for (int i = 0; i < TerrainDatas.GetCount(); ++i)
			{
				const FTerrainData& data = TerrainDatas[i];
				const size_t sizeBytes = data.VertexIndices->GetMemorySizeBytes_Allocated();
				RpgPlatformMemory::Copy(stagingMap + stagingOffset, data.VertexIndices->GetData(), sizeBytes);
				stagingOffset += sizeBytes;
			}
			cmdList->CopyBufferRegion(TerrainIndexBuffer->GetResource(), 0, stagingResource, srcOffsetIndex, indexSizeBytes);


			// Sanity check 
			//RPG_Check(stagingOffset == stagingSizeBytes);
		}
		RpgD3D12::UnmapBuffer(TerrainStagingBuffer.Get());
	}
}
