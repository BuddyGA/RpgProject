#include "RpgRenderTask_GenerateTerrain.h"
#include "core/RpgConsoleSystem.h"


#define RPG_TERRAIN_TILE_SIZE		2 // 2m
#define RPG_TERRAIN_WORLD_SIZE		1000 // 1km



RpgRenderTask_GenerateTerrain::RpgRenderTask_GenerateTerrain() noexcept
{
}


void RpgRenderTask_GenerateTerrain::Reset() noexcept
{
	RpgThreadTask::Reset();

	TerrainMesh.Release();
}


void RpgRenderTask_GenerateTerrain::Execute() noexcept
{
	RPG_Assert(TerrainMesh);

	constexpr int HALF_WORLD_SIZE = RPG_TERRAIN_WORLD_SIZE / 2;
	constexpr int TILE_COUNT_AXIS = RPG_TERRAIN_WORLD_SIZE / RPG_TERRAIN_TILE_SIZE;
	constexpr int TILE_COUNT_TOTAL = TILE_COUNT_AXIS * TILE_COUNT_AXIS;
	constexpr int VERTEX_COUNT_TOTAL = TILE_COUNT_TOTAL * 4;
	constexpr int INDEX_COUNT_TOTAL = TILE_COUNT_TOTAL * 6;
	
	RpgVertexMeshPositionArray& vertexPositions = TerrainMesh->VertexWritePositionLock();
	vertexPositions.Clear(true);
	vertexPositions.Reserve(VERTEX_COUNT_TOTAL);

	RpgVertexMeshNormalTangentArray& vertexNormalTangents = TerrainMesh->VertexWriteNormalTangentLock();
	vertexNormalTangents.Clear(true);
	vertexNormalTangents.Reserve(VERTEX_COUNT_TOTAL);

	RpgVertexMeshTexCoordArray& vertexTexCoords = TerrainMesh->VertexWriteTexCoordLock();
	vertexTexCoords.Clear(true);
	vertexTexCoords.Reserve(VERTEX_COUNT_TOTAL);

	RpgVertexIndexArray& vertexIndices = TerrainMesh->VertexWriteIndexLock();
	vertexIndices.Clear(true);
	vertexIndices.Reserve(INDEX_COUNT_TOTAL);

	RpgVector3 vertexPos(-static_cast<float>(HALF_WORLD_SIZE), 0.0f, static_cast<float>(HALF_WORLD_SIZE));

	for (int tz = 0; tz < TILE_COUNT_AXIS; ++tz)
	{
		for (int tx = 0; tx < TILE_COUNT_AXIS; ++tx)
		{
			const uint32_t vid = static_cast<uint32_t>(vertexPositions.GetCount());

			// vertex position
			vertexPositions.AddValue(vertexPos);
			vertexPositions.AddValue(vertexPos + RpgVector3(RPG_TERRAIN_TILE_SIZE, 0.0f, 0.0f));
			vertexPositions.AddValue(vertexPos + RpgVector3(RPG_TERRAIN_TILE_SIZE, 0.0f, -RPG_TERRAIN_TILE_SIZE));
			vertexPositions.AddValue(vertexPos + RpgVector3(0.0f, 0.0f, -RPG_TERRAIN_TILE_SIZE));

			// vertex normal-tangent
			for (int i = 0; i < 4; ++i)
			{
				RpgVertex::FMeshNormalTangent& nt = vertexNormalTangents.Add();
				nt.Normal = RpgVector4(0.0f, 1.0f, 0.0f, 0.0f);
				nt.Tangent = RpgVector4(0.0f, 1.0f, 0.0f, 0.0f);
			}

			// vertex tex-coord
			vertexTexCoords.AddValue({ 0.0f, 0.0f });
			vertexTexCoords.AddValue({ 1.0f, 0.0f });
			vertexTexCoords.AddValue({ 1.0f, 1.0f });
			vertexTexCoords.AddValue({ 0.0f, 1.0f });

			// vertex indices
			vertexIndices.InsertAtRange({ vid, vid + 1, vid + 2, vid + 2, vid + 3, vid }, RPG_INDEX_LAST);

			vertexPos.X += RPG_TERRAIN_TILE_SIZE;
		}

		vertexPos.X = -HALF_WORLD_SIZE;
		vertexPos.Z -= RPG_TERRAIN_TILE_SIZE;
	}

	RPG_CONSOLE_Log(RpgLogTemp, "Generated terrain vertex (Tiles: %i, Vertices: %i, SizeBytes: %u KiB)", TILE_COUNT_TOTAL, VERTEX_COUNT_TOTAL,
		(vertexPositions.GetMemorySizeBytes_Allocated() + vertexNormalTangents.GetMemorySizeBytes_Allocated() + vertexTexCoords.GetMemorySizeBytes_Allocated() + vertexIndices.GetMemorySizeBytes_Allocated()) / RPG_MEMORY_SIZE_KiB(1)
	);

	TerrainMesh->VertexWritePositionUnlock();
	TerrainMesh->VertexWriteNormalTangentUnlock();
	TerrainMesh->VertexWriteTexCoordUnlock();
	TerrainMesh->VertexWriteIndexUnlock();

	TerrainMesh.Release();
}
