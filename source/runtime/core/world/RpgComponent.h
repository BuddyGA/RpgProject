#pragma once

#include "../RpgStream.h"
#include "../dsa/RpgFreeList.h"
#include "RpgGameObject.h"


class RpgWorld;


#define RPG_COMPONENT_ID_INVALID		UINT16_MAX
#define RPG_COMPONENT_TYPE_MAX_COUNT	16



#define RPG_COMPONENT_CLASS_BEGIN(type, id, name)										\
class type																				\
{																						\
public:																					\
	static constexpr uint16_t TYPE_ID = id;												\
	static constexpr const char* TYPE_NAME = name;										\
	static void StreamWrite(RpgStreamWriter& writer, const type& data) noexcept;		\
	static void StreamRead(RpgStreamReader& reader, type& data) noexcept;				\
public:																					\
	RpgGameObjectID GameObject;


#define RPG_COMPONENT_CLASS_END()	\
	friend RpgWorld;				\
};


#define RPG_COMPONENT_DEFINITION_STATIC_StreamWrite(type)	void type::StreamWrite(RpgStreamWriter& writer, const type& data) noexcept
#define RPG_COMPONENT_DEFINITION_STATIC_StreamRead(type)	void type::StreamRead(RpgStreamReader& reader, type& data) noexcept




class RpgComponentStorageInterface
{
	RPG_NOCOPY(RpgComponentStorageInterface)

public:
	RpgComponentStorageInterface() noexcept = default;
	virtual ~RpgComponentStorageInterface() noexcept = default;

	virtual void StreamWrite(RpgStreamWriter& writer, int id) const noexcept = 0;
	virtual void StreamRead(RpgStreamReader& reader, int id, RpgGameObjectID gameObject) noexcept = 0;
	virtual int Add() noexcept = 0;
	virtual void Remove(int index) noexcept = 0;
	virtual void Clear(bool bFreeMemory = false) noexcept = 0;

};


template<typename TComponent>
class RpgComponentStorage : public RpgComponentStorageInterface
{
public:
	RpgComponentStorage() noexcept = default;
	
	~RpgComponentStorage() noexcept
	{
		RPG_LogDebug(RpgLogTemp, "Destroy component storage (%s)", TComponent::TYPE_NAME);
	}

	virtual void StreamWrite(RpgStreamWriter& writer, int id) const noexcept override
	{
		TComponent::StreamWrite(writer, Components[id]);
	}

	virtual void StreamRead(RpgStreamReader& reader, int id, RpgGameObjectID gameObject) noexcept override
	{
		TComponent& data = Components[id];
		data.GameObject = gameObject;
		TComponent::StreamRead(reader, data);
	}

	virtual int Add() noexcept override
	{
		return Components.Add();
	}

	virtual void Remove(int id) noexcept override
	{
		Components[id].Destroy();
		Components.RemoveAt(id);
	}

	virtual void Clear(bool bFreeMemory = false) noexcept override
	{
		Components.Clear(bFreeMemory);
	}


	inline bool IsValid(int id) const noexcept 
	{
		return Components.IsValid(id);
	}
	

	inline TComponent& Get(int id) noexcept
	{
		return Components.GetAt(id);
	}

	inline const TComponent& Get(int id) const noexcept
	{
		return Components.GetAt(id);
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
