#pragma once

#include "../RpgMath.h"
#include "../RpgStream.h"
#include "../dsa/RpgFreeList.h"


class RpgWorld;
class RpgLevel;
class RpgGameObject;
class RpgGameObjectScript;



namespace RpgGameObjectFlag
{
	enum : uint32_t
	{
		None				= (0),
		Allocated			= (1 << 0),
		Transient			= (1 << 1),
		Loading				= (1 << 2),
		Loaded				= (1 << 3),
		Spawned				= (1 << 4),
		Visible				= (1 << 5),
		TransformUpdated	= (1 << 6),
		PendingDestroy		= (1 << 7),
	};
};



enum class RpgGameObjectAttachMode : uint8_t
{
	RESET_TRANSFORM = 0,
	KEEP_LOCAL_TRANSFORM,
	KEEP_WORLD_TRANSFORM
};



class RpgGameObject final
{
public:
	RpgGameObject() noexcept;

private:
	RpgGameObject(RpgLevel* in_Level, int in_Index, uint16_t in_Gen) noexcept;


public:
	void Destroy() noexcept;
	bool IsPendingDestroy() const noexcept;
	const RpgName& GetName() const noexcept;
	void AttachToParent(RpgGameObject parent, RpgGameObjectAttachMode mode) noexcept;
	void DetachFromParent() noexcept;
	void SpawnAtTransform(const RpgTransform& worldTransform) noexcept;
	void SetVisibility(bool bIsVisible) noexcept;
	void AttachScript(RpgGameObjectScript* script) noexcept;
	void DetachScript(RpgGameObjectScript* script) noexcept;
	void SetLocalTransform(const RpgTransform& transform) noexcept;
	RpgTransform GetLocalTransform() const noexcept;
	void SetWorldTransform(const RpgTransform& transform) noexcept;
	RpgTransform GetWorldTransform() const noexcept;
	const RpgMatrixTransform& GetWorldTransformMatrix() const noexcept;
	


	inline bool IsNull() const noexcept
	{
		return Level == nullptr || Index == RPG_INDEX_INVALID || Gen == UINT16_MAX;
	}


	template<typename TComponent>
	inline TComponent* GetComponent() const noexcept
	{
		return static_cast<TComponent*>(InternalGetComponent(TComponent::TYPE_ID));
	}


	template<typename TComponent>
	inline TComponent& AddComponent() noexcept
	{
		return *static_cast<TComponent*>(InternalAddComponent(TComponent::TYPE_ID));
	}


	template<typename TComponent>
	inline bool RemoveComponent() noexcept
	{
		return InternalRemoveComponent(TComponent::TYPE_ID);
	}


public:
	inline bool operator==(const RpgGameObject& rhs) const noexcept
	{
		return Level == rhs.Level && Index == rhs.Index && Gen == rhs.Gen;
	}

	inline bool operator!=(const RpgGameObject& rhs) const noexcept
	{
		return !(*this == rhs);
	}


private:
	void* InternalGetComponent(int compType) const noexcept;
	void* InternalAddComponent(int compType) const noexcept;
	bool InternalRemoveComponent(int compType) const noexcept;


private:
	RpgLevel* Level;
	int Index;
	uint16_t Gen;


	friend RpgLevel;

};



class RpgGameObjectScript
{
	RPG_NOCOPY(RpgGameObjectScript)

protected:
	RpgGameObjectScript() noexcept
	{
		AttachedScriptIndex = RPG_INDEX_INVALID;
		bHasStartedPlay = false;
	}

public:
	virtual ~RpgGameObjectScript() noexcept = default;

	virtual void AttachedToGameObject() noexcept {}
	virtual void DetachedFromGameObject() noexcept {}
	virtual void StartPlay() noexcept {}
	virtual void StopPlay() noexcept {}
	virtual void TickUpdate(float deltaTime) noexcept {}
	virtual const char* GetTypeName() const noexcept { return "RpgScript"; }


protected:
	RpgGameObject GameObject;

private:
	int16_t AttachedScriptIndex;
	bool bHasStartedPlay;


	friend RpgLevel;

};



#define RPG_GAMEOBJECT_SCRIPT(name)										\
public:																	\
static constexpr const char* TYPE_NAME = name;							\
virtual const char* GetTypeName() const noexcept { return TYPE_NAME; }	
