#pragma once

#include "asset/RpgMesh.h"


#define RPG_TERRAIN_TILE_SIZE			200		// 2m
#define RPG_TERRAIN_WORLD_SIZE			10000	// 100m


typedef RpgArray<class RpgTerrain> RpgTerrainGrids;


class RpgTerrain
{
	RPG_NOCOPY(RpgTerrain)

public:
	RpgTerrain() noexcept;
	void Generate() noexcept;


	inline void SetWorldPosition(const RpgVector3& in_WorldPosition) noexcept
	{
		const float halfWorldSize = static_cast<float>(RPG_TERRAIN_WORLD_SIZE) * 0.5f;
		const RpgVector3 halfWorldBound(halfWorldSize, 100.0f, halfWorldSize);
		WorldPosition = in_WorldPosition;
		WorldBound.Min = WorldPosition - halfWorldBound;
		WorldBound.Max = WorldPosition + halfWorldBound;
	}

	inline const RpgVector3 GetWorldPosition() const noexcept
	{
		return WorldPosition;
	}

	inline const RpgBoundingAABB& GetWorldBound() const noexcept
	{
		return WorldBound;
	}

	inline const RpgSharedMesh& GetMesh() const noexcept
	{
		return Mesh;
	}


private:
	RpgVector3 WorldPosition;
	RpgBoundingAABB WorldBound;
	RpgSharedMesh Mesh;

};
