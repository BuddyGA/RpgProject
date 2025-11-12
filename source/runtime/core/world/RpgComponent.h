#pragma once

#include "RpgGameObject.h"
#include "../asset/RpgAssetTypes.h"



class RpgComponentStorageInterface
{
	RPG_NOCOPY(RpgComponentStorageInterface)

public:
	RpgComponentStorageInterface() noexcept = default;
	virtual ~RpgComponentStorageInterface() noexcept = default;

	virtual bool IsValid(int id) const noexcept = 0;
	virtual void Clear(bool bFreeMemory = false) noexcept = 0;
	virtual void Reserve(int capacity) noexcept = 0;
	virtual int GetCount() const noexcept = 0;
	virtual int Add(RpgGameObject gameObject, uint32_t flags) noexcept = 0;
	virtual void* Get(int id, RpgGameObject gameObject) noexcept = 0;
	virtual void Remove(int id, RpgGameObject gameObject) noexcept = 0;
	virtual void SetFlags(int id, RpgGameObject gameObject, uint32_t flags) noexcept = 0;
	virtual void StreamWrite(int id, RpgGameObject gameObject, RpgStreamWriter& writer) const noexcept = 0;
	virtual void StreamRead(int id, RpgGameObject gameObject, RpgStreamReader& reader) noexcept = 0;
	virtual void GetExternalAssetReferences(int id, RpgAssetReferences& out_AssetRefs) noexcept = 0;
	virtual bool IsLoaded(int id, RpgGameObject gameObject) noexcept = 0;

};



template<typename TComponent>
class RpgComponentStorage : public RpgComponentStorageInterface
{
public:
	RpgComponentStorage() noexcept = default;


	virtual bool IsValid(int id) const noexcept override
	{
		return Components.IsValid(id);
	}

	virtual void Clear(bool bFreeMemory = false) noexcept override
	{
		Components.Clear(bFreeMemory);
	}

	virtual void Reserve(int capacity) noexcept override
	{
		Components.Reserve(capacity);
	}

	virtual int GetCount() const noexcept override
	{
		return Components.GetCount();
	}

	virtual int Add(RpgGameObject gameObject, uint32_t flags) noexcept override
	{
		const int id = Components.Add();

		TComponent& data = Components[id];
		data.GameObject = gameObject;
		data.Flags = flags;

		return id;
	}

	virtual void* Get(int id, RpgGameObject gameObject) noexcept override
	{
		TComponent& data = Components[id];
		RPG_Check(data.GameObject == gameObject);

		return &data;
	}

	virtual void Remove(int id, RpgGameObject gameObject) noexcept override
	{
		TComponent& data = Components[id];
		RPG_Check(data.GameObject == gameObject);
		data.Destroy();
		data.GameObject = RpgGameObject();
		data.Flags = RpgGameObjectFlag::None;

		Components.RemoveAt(id);
	}

	virtual void SetFlags(int id, RpgGameObject gameObject, uint32_t flags) noexcept override
	{
		TComponent& data = Components[id];
		RPG_Check(data.GameObject == gameObject);
		data.Flags = flags;
	}

	virtual void StreamWrite(int id, RpgGameObject gameObject, RpgStreamWriter& writer) const noexcept override
	{
		const TComponent& data = Components[id];
		RPG_Check(data.GameObject == gameObject);
		TComponent::StreamWrite(writer, data);
	}

	virtual void StreamRead(int id, RpgGameObject gameObject, RpgStreamReader& reader) noexcept override
	{
		TComponent& data = Components[id];
		RPG_Check(data.GameObject == gameObject);
		TComponent::StreamRead(reader, data);
	}

	virtual void GetExternalAssetReferences(int id, RpgAssetReferences& out_AssetRefs) noexcept override
	{
		TComponent& data = Components[id];
		TComponent::GetExternalAssetReferences(data, out_AssetRefs);
	}

	virtual bool IsLoaded(int id, RpgGameObject gameObject) noexcept override
	{
		TComponent& data = Components[id];
		RPG_Check(data.GameObject == gameObject);
		return TComponent::IsLoaded(data);
	}


	inline RpgFreeList<TComponent>& GetComponents() noexcept
	{
		return Components;
	}

	inline const RpgFreeList<TComponent>& GetComponents() const noexcept
	{
		return Components;
	}


private:
	RpgFreeList<TComponent> Components;

};



#define RPG_COMPONENT_TYPE_MAX_COUNT		32


#define RPG_COMPONENT(type, id)																											\
	friend RpgComponentStorage<type>;																									\
public:																																	\
	static constexpr const char* TYPE_NAME = #type;																						\
	static constexpr int TYPE_ID = id;																									\
	static void StreamWrite(RpgStreamWriter& writer, const type& data) noexcept;														\
	static void StreamRead(RpgStreamReader& reader, type& data) noexcept;																\
	static void GetExternalAssetReferences(type& data, RpgAssetReferences& out_AssetRefs) noexcept;										\
	static bool IsLoaded(type& data) noexcept;																							\
public:																																	\
inline RpgGameObject GetGameObject() const noexcept																						\
{																																		\
	return GameObject;																													\
}																																		\
inline bool IsGameObjectSpawned() const noexcept																						\
{																																		\
	RPG_Check(!GameObject.IsNull());																									\
	return (Flags & RpgGameObjectFlag::Loaded) && (Flags & RpgGameObjectFlag::Spawned) && !(Flags & RpgGameObjectFlag::PendingDestroy);	\
}																																		\
inline bool IsGameObjectVisible() const noexcept																						\
{																																		\
	RPG_Check(!GameObject.IsNull());																									\
	return (Flags & RpgGameObjectFlag::Visible);																						\
}																																		\
inline bool IsGameObjectTransformUpdated() const noexcept																				\
{																																		\
	RPG_Check(!GameObject.IsNull());																									\
	return (Flags & RpgGameObjectFlag::TransformUpdated);																				\
}																																		\
private:																																\
	RpgGameObject GameObject;																											\
	uint32_t Flags{ RpgGameObjectFlag::None };																							


#define RPG_COMPONENT_STATIC_StreamWrite(type)					void type::StreamWrite(RpgStreamWriter& writer, const type& data) noexcept
#define RPG_COMPONENT_STATIC_StreamRead(type)					void type::StreamRead(RpgStreamReader& reader, type& data) noexcept
#define RPG_COMPONENT_STATIC_GetExternalAssetReferences(type)	void type::GetExternalAssetReferences(type& data, RpgAssetReferences& out_AssetRefs) noexcept
#define RPG_COMPONENT_STATIC_IsLoaded(type)						bool type::IsLoaded(type& data) noexcept
