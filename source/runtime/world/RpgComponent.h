#pragma once

#include "core/RpgFreeList.h"
#include "asset/RpgMesh.h"
#include "asset/RpgMaterial.h"
#include "RpgEntity.h"



class RpgComponentStorage
{
	RPG_NOCOPY(RpgComponentStorage)

public:
	RpgComponentStorage() noexcept
	{
		for (int i = 0; i < RPG_ENTITY_MAX_COUNT; ++i)
		{
			ComponentIndices[i] = RPG_INDEX_INVALID;
		}
	}


	virtual void* AddComponent(RpgEntity entity) noexcept = 0;
	virtual bool RemoveComponent(RpgEntity entity) noexcept = 0;
	virtual void* GetComponent(RpgEntity entity) noexcept = 0;


	inline bool HasComponent(RpgEntity entity) const noexcept
	{
		return ComponentIndices[entity.GetId()] != RPG_INDEX_INVALID;
	}


protected:
	RpgFreeList<RpgEntity> Owners;
	int ComponentIndices[RPG_ENTITY_MAX_COUNT];

};



template<typename TComponent>
class RpgComponentStorageType : public RpgComponentStorage
{
public:
	RpgComponentStorageType() noexcept = default;


	virtual void* AddComponent(RpgEntity entity) noexcept override
	{
		RPG_Check(entity);

		if (void* checkComponent = GetComponent(entity))
		{
			return checkComponent;
		}

		const int ownerIndex = Owners.Add();
		const int compIndex = Components.Add();
		RPG_Check(ownerIndex == compIndex);
	
		const int entityId = entity.GetId();
		RPG_Check(ComponentIndices[entityId] == RPG_INDEX_INVALID);
		ComponentIndices[entityId] = compIndex;

		return &Components[compIndex];
	}


	virtual bool RemoveComponent(RpgEntity entity) noexcept override
	{
		RPG_Check(entity);

		const int entityId = entity.GetId();

		const int compIndex = ComponentIndices[entityId];
		if (compIndex == RPG_INDEX_INVALID)
		{
			return false;
		}

		RPG_Check(Owners[compIndex] == entity);
		Owners.RemoveAt(compIndex);
		Components.RemoveAt(compIndex);
		ComponentIndices[entityId] = RPG_INDEX_INVALID;

		return true;
	}


	virtual void* GetComponent(RpgEntity entity) noexcept override
	{
		RPG_Check(entity);

		const int compIndex = ComponentIndices[entity.Id];

		if (compIndex != RPG_INDEX_INVALID)
		{
			RPG_Check(Owners[compIndex] == entity);
			return &Components[compIndex];
		}

		return nullptr;
	}


private:
	RpgFreeList<TComponent> Components;

};



#define RPG_COMPONENT_ID_MAX_COUNT		32

#define RPG_COMPONENT_ID_PHYSICS_0		0
#define RPG_COMPONENT_ID_PHYSICS_1		(RPG_COMPONENT_ID_PHYSICS_0 + 1)
#define RPG_COMPONENT_ID_PHYSICS_2		(RPG_COMPONENT_ID_PHYSICS_0 + 2)
#define RPG_COMPONENT_ID_PHYSICS_3		(RPG_COMPONENT_ID_PHYSICS_0 + 3)
#define RPG_COMPONENT_ID_PHYSICS_4		(RPG_COMPONENT_ID_PHYSICS_0 + 4)
#define RPG_COMPONENT_ID_PHYSICS_5		(RPG_COMPONENT_ID_PHYSICS_0 + 5)
#define RPG_COMPONENT_ID_PHYSICS_6		(RPG_COMPONENT_ID_PHYSICS_0 + 6)
#define RPG_COMPONENT_ID_PHYSICS_7		(RPG_COMPONENT_ID_PHYSICS_0 + 7)

#define RPG_COMPONENT_ID_RENDER_0		8
#define RPG_COMPONENT_ID_RENDER_1		(RPG_COMPONENT_ID_RENDER_0 + 1)
#define RPG_COMPONENT_ID_RENDER_2		(RPG_COMPONENT_ID_RENDER_0 + 2)
#define RPG_COMPONENT_ID_RENDER_3		(RPG_COMPONENT_ID_RENDER_0 + 3)
#define RPG_COMPONENT_ID_RENDER_4		(RPG_COMPONENT_ID_RENDER_0 + 4)
#define RPG_COMPONENT_ID_RENDER_5		(RPG_COMPONENT_ID_RENDER_0 + 5)
#define RPG_COMPONENT_ID_RENDER_6		(RPG_COMPONENT_ID_RENDER_0 + 6)
#define RPG_COMPONENT_ID_RENDER_7		(RPG_COMPONENT_ID_RENDER_0 + 7)

#define RPG_COMPONENT_ID_ANIMATION_0	16
#define RPG_COMPONENT_ID_ANIMATION_1	(RPG_COMPONENT_ID_ANIMATION_0 + 1)
#define RPG_COMPONENT_ID_ANIMATION_2	(RPG_COMPONENT_ID_ANIMATION_0 + 2)
#define RPG_COMPONENT_ID_ANIMATION_3	(RPG_COMPONENT_ID_ANIMATION_0 + 3)
#define RPG_COMPONENT_ID_ANIMATION_4	(RPG_COMPONENT_ID_ANIMATION_0 + 4)
#define RPG_COMPONENT_ID_ANIMATION_5	(RPG_COMPONENT_ID_ANIMATION_0 + 5)
#define RPG_COMPONENT_ID_ANIMATION_6	(RPG_COMPONENT_ID_ANIMATION_0 + 6)
#define RPG_COMPONENT_ID_ANIMATION_7	(RPG_COMPONENT_ID_ANIMATION_0 + 7)


#define RPG_COMPONENT(id)			\
public:								\
static constexpr TYPE_ID = id;		\



struct RpgComponent_Mesh
{
	RpgBoundingAABB WorldBound;
	RpgSharedMesh Mesh;
	RpgSharedMaterial Material;
	bool bVisible;
};
