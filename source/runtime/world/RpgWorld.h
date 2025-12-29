#pragma once

#include "core/RpgString.h"
#include "core/RpgFreeList.h"
#include "RpgEntity.h"
#include "RpgComponent.h"
#include "RpgTerrain.h"



enum class RpgTransformAttachMode : uint8_t
{
	RESET_TRANSFORM = 0,
	KEEP_TRANSFORM_LOCAL,
	KEEP_TRANSFORM_WORLD
};



class RpgWorld 
{
	RPG_NOCOPY(RpgWorld)

public:
	RpgWorld() noexcept;
	void BeginFrame(int frameIndex) noexcept;
	void EndFrame(int frameIndex) noexcept;
	void DispatchTickUpdate(float deltaTime) noexcept;
	void DispatchPostTickUpdate() noexcept;

	RpgEntity Entity_Create(const RpgName& name, bool bIsTransient = false) noexcept;
	void Entity_AttachToParent(RpgEntity entity, RpgEntity parent, RpgTransformAttachMode attachMode) noexcept;
	void Entity_Spawn(RpgEntity entity) noexcept;
	void Entity_Destroy(RpgEntity entity) noexcept;


	inline bool Entity_IsValid(RpgEntity entity) const noexcept
	{
		if (!entity)
		{
			return false;
		}

		const FEntityGenFlags genFlags = EntityGenFlags[entity.Id];
		return entity.Gen == entity.Gen && !(genFlags.Flags & FLAG_Entity_PendingDestroy);
	}

	inline bool Entity_IsSpawned(RpgEntity entity) const noexcept
	{
		return Entity_IsValid(entity) && EntityGenFlags[entity.Id].Flags & FLAG_Entity_Spawned;
	}


	inline const RpgTerrainGrids& GetTerrainGrids() const noexcept
	{
		return TerrainGrids;
	}


private:
	enum EFlags : uint16_t
	{
		FLAG_Entity_None			= (0),
		FLAG_Entity_Transient		= (1 << 0),
		FLAG_Entity_Spawned			= (1 << 1),
		FLAG_Entity_PendingDestroy	= (1 << 2),
	};


	struct FEntityGenFlags
	{
		uint16_t Gen{ 0 };
		uint16_t Flags{ FLAG_Entity_None };
	};


	struct FEntityTransform
	{
		RpgTransform LocalTransform;
		RpgTransform WorldTransform;
		RpgEntity Parent;
		uint8_t Dirty;
	};


private:
	RpgFreeList<RpgStringID> EntityNames;
	RpgFreeList<FEntityGenFlags> EntityGenFlags;
	RpgFreeList<FEntityTransform> EntityTransforms;

	RpgComponentStorageType<RpgComponent_Mesh> MeshComponents;
	
	RpgTerrainGrids TerrainGrids;

};
