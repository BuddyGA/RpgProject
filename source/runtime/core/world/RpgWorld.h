#pragma once

#include "../RpgPointer.h"
#include "RpgLevel.h"



#define RPG_WORLD_MAX_GAMEOBJECT	65536


RPG_LOG_DECLARE_CATEGORY_EXTERN(RpgLogWorld)


class RpgWorld;
class RpgRenderer;



class RpgWorldSubsystem
{
	RPG_NOCOPY(RpgWorldSubsystem)

public:
	RpgWorldSubsystem() noexcept
	{
		World = nullptr;
	}

	virtual ~RpgWorldSubsystem() noexcept = default;

protected:
	virtual void StartPlay() noexcept {}
	virtual void StopPlay() noexcept {}
	virtual void PreTickUpdate() noexcept {}
	virtual void TickUpdate(float deltaTime) noexcept {}
	virtual void PostTickUpdate() noexcept {}
	virtual void Render(int frameIndex, RpgRenderer* renderer) noexcept {}


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}

	inline RpgWorld* GetWorld() const noexcept
	{
		return World;
	}


protected:
	RpgName Name;

private:
	RpgWorld* World;


	friend RpgWorld;

};




class RpgWorld
{
	RPG_NOCOPY(RpgWorld)

public:
	RpgWorld(const RpgName& name) noexcept;
	~RpgWorld() noexcept;

	void StreamWrite(RpgStreamWriter& writer) const noexcept;
	void StreamRead(RpgStreamReader& reader) noexcept;

	void ClearFast() noexcept;
	void BeginFrame(int frameIndex) noexcept;
	void EndFrame(int frameIndex) noexcept;

	void DispatchStartPlay() noexcept;
	void DispatchStopPlay() noexcept;
	void DispatchTickUpdate(float deltaTimeSeconds) noexcept;
	void DispatchPostTickUpdate() noexcept;
	void DispatchRender(int frameIndex, RpgRenderer* renderer) noexcept;


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}

	inline bool HasStartedPlay() const noexcept
	{
		return bHasStartedPlay;
	}


private:
	RpgName Name;
	bool bHasStartedPlay;


	struct FFrameData
	{
		RpgArray<int> PendingDestroyObjects;
	};

	FFrameData FrameDatas[RPG_FRAME_BUFFERING];
	int FrameIndex;



// --------------------------------------------------------------------------------------------------------------------------------------------- //
// 	Subsystem interface
// --------------------------------------------------------------------------------------------------------------------------------------------- //
public:
	template<typename TWorldSubsystem>
	inline void Subsystem_Register() noexcept
	{
		static_assert(std::is_base_of<RpgWorldSubsystem, TWorldSubsystem>::value, "RpgWorld: Add subsystem type of <TWorldSubsystem> must be derived from type <RpgWorldSubsystem>!");
		
		for (int i = 0; i < Subsystems.GetCount(); ++i)
		{
			if (TWorldSubsystem* check = dynamic_cast<TWorldSubsystem*>(Subsystems[i]))
			{
				RPG_LogWarn(RpgLogWorld, "World subsystem type (%s) already exists!", *check->GetName());
				return;
			}
		}

		RpgWorldSubsystem* subsystem = new TWorldSubsystem();
		subsystem->World = this;

		Subsystems.AddValue(subsystem);
	}


	template<typename TWorldSubsystem>
	inline TWorldSubsystem* Subsystem_Get() const noexcept
	{
		static_assert(std::is_base_of<RpgWorldSubsystem, TWorldSubsystem>::value, "RpgWorld: Get subsystem type of <TWorldSubsystem> must be derived from type <RpgWorldSubsystem>!");

		for (int i = 0; i < Subsystems.GetCount(); ++i)
		{
			if (TWorldSubsystem* check = dynamic_cast<TWorldSubsystem*>(Subsystems[i]))
			{
				return check;
			}
		}

		return nullptr;
	}


private:
	RpgArrayInline<RpgWorldSubsystem*, 16> Subsystems;



// --------------------------------------------------------------------------------------------------------------------------------------------- //
// 	Component interface
// --------------------------------------------------------------------------------------------------------------------------------------------- //
public:
	template<typename TComponent>
	inline void Component_Register() noexcept
	{
		RPG_CheckV(TComponent::TYPE_ID >= 0 && TComponent::TYPE_ID < RPG_COMPONENT_TYPE_MAX_COUNT, "RpgWorld: Exceeds maximum component type count!");
		RPG_Check(ComponentStorages[TComponent::TYPE_ID] == nullptr);
		ComponentStorages[TComponent::TYPE_ID] = new RpgComponentStorage<TComponent>();

		RPG_Log(RpgLogWorld, "Registered component of type (%s)", TComponent::TYPE_NAME);
	}

	template<typename TComponent>
	inline RpgFreeList<TComponent>::Iterator Component_CreateIterator() noexcept
	{
		return Component_GetStorage<TComponent>()->GetComponents().CreateIterator();
	}

	template<typename TComponent>
	inline RpgFreeList<TComponent>::ConstIterator Component_CreateConstIterator() const noexcept
	{
		return Component_GetStorage<TComponent>()->GetComponents().CreateConstIterator();
	}


private:
	template<typename TComponent>
	inline RpgComponentStorage<TComponent>* Component_GetStorage() noexcept
	{
		return static_cast<RpgComponentStorage<TComponent>*>(ComponentStorages[TComponent::TYPE_ID]);
	}

	template<typename TComponent>
	inline const RpgComponentStorage<TComponent>* Component_GetStorage() const noexcept
	{
		return static_cast<const RpgComponentStorage<TComponent>*>(ComponentStorages[TComponent::TYPE_ID]);
	}


private:
	RpgArrayInline<RpgComponentStorageInterface*, RPG_COMPONENT_TYPE_MAX_COUNT> ComponentStorages;



// --------------------------------------------------------------------------------------------------------------------------------------------- //
// 	Level interface
// --------------------------------------------------------------------------------------------------------------------------------------------- //
public:
	RpgLevel* Level_Create(const RpgName& name) noexcept;


	inline RpgLevel* Level_GetMain() noexcept
	{
		return Levels[0];
	}

	inline const RpgLevel* Level_GetMain() const noexcept
	{
		return Levels[0];
	}


private:
	RpgArray<RpgLevel*> Levels;



// --------------------------------------------------------------------------------------------------------------------------------------------- //
// 	GameObject interface
// --------------------------------------------------------------------------------------------------------------------------------------------- //
public:
	[[nodiscard]] RpgGameObjectID GameObject_Create(const RpgName& name, const RpgTransform& worldTransform = RpgTransform()) noexcept;

	[[nodiscard]] inline RpgGameObjectID GameObject_CreateTransient(const RpgName& name, const RpgTransform& worldTransform = RpgTransform()) noexcept
	{
		const RpgGameObjectID gameObject = GameObject_Create(name, worldTransform);
		GameObjectInfos[gameObject.Index].Flags |= FLAG_Transient;

		return gameObject;
	}


	void GameObject_Destroy(RpgGameObjectID& gameObject) noexcept;
	void GameObject_StreamWrite(RpgGameObjectID gameObject, RpgStreamWriter& writer) const noexcept;
	void GameObject_StreamRead(RpgGameObjectID gameObject, RpgStreamReader& reader) noexcept;
	void GameObject_AttachScript(RpgGameObjectID gameObject, RpgGameObjectScript* script) noexcept;
	void GameObject_DetachScript(RpgGameObjectID gameObject, RpgGameObjectScript* script) noexcept;
	void GameObject_Spawn(RpgGameObjectID gameObject, RpgLevel* opt_Level = nullptr) noexcept;


	inline bool GameObject_IsValid(RpgGameObjectID gameObject) const noexcept
	{
		if (!gameObject.IsValid() || gameObject.World != this || !GameObjectInfos.IsValid(gameObject.Index))
		{
			return false;
		}

		const FGameObjectInfo& info = GameObjectInfos[gameObject.Index];
		return info.Gen == gameObject.Gen && !(info.Flags & FLAG_PendingDestroy);
	}


	inline void GameObject_SetWorldTransform(RpgGameObjectID gameObject, const RpgTransform& worldTransform) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		FGameObjectTransform& transform = GameObjectTransforms[gameObject.Index];
		transform.WorldMatrix = worldTransform.ToMatrixTransform();

		GameObjectInfos[gameObject.Index].Flags |= FLAG_TransformUpdated;
	}


	inline RpgTransform GameObject_GetWorldTransform(RpgGameObjectID gameObject) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		RpgTransform transform;
		GameObjectTransforms[gameObject.Index].WorldMatrix.Decompose(transform.Position, transform.Rotation, transform.Scale);

		return transform;
	}


	inline const RpgName& GameObject_GetName(RpgGameObjectID gameObject) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		return GameObjectNames[gameObject.Index];
	}


	inline const RpgMatrixTransform& GameObject_GetWorldTransformMatrix(RpgGameObjectID gameObject) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		return GameObjectTransforms[gameObject.Index].WorldMatrix;
	}


	template<typename TComponent>
	inline TComponent* GameObject_AddComponent(RpgGameObjectID gameObject) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		FGameObjectInfo& info = GameObjectInfos[gameObject.Index];
		int index = info.ComponentIndices[TComponent::TYPE_ID];

		if (index != RPG_COMPONENT_ID_INVALID)
		{
			TComponent& check = Component_GetStorage<TComponent>()->Get(index);
			RPG_Check(check.GameObject == gameObject);

			return &check;
		}

		auto storage = Component_GetStorage<TComponent>();
		index = storage->Add();
		info.ComponentIndices[TComponent::TYPE_ID] = index;

		TComponent& data = storage->Get(index);
		data.GameObject = gameObject;

		return &data;
	}


	template<typename TComponent>
	inline void GameObject_RemoveComponent(RpgGameObjectID gameObject) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		FGameObjectInfo& info = GameObjectInfos[gameObject.Index];
		const int index = info.ComponentIndices[TComponent::TYPE_ID];

		auto storage = Component_GetStorage<TComponent>();
		{
			TComponent& data = storage->Get(index);
			RPG_Check(data.GameObject == gameObject);
		}
		storage->Remove(index);

		info.ComponentIndices[TComponent::TYPE_ID] = RPG_COMPONENT_ID_INVALID;
	}


	template<typename TComponent>
	inline TComponent* GameObject_GetComponent(RpgGameObjectID gameObject) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		const FGameObjectInfo& info = GameObjectInfos[gameObject.Index];
		const int index = info.ComponentIndices[TComponent::TYPE_ID];

		if (index == RPG_COMPONENT_ID_INVALID)
		{
			return nullptr;
		}

		TComponent& data = Component_GetStorage<TComponent>()->Get(index);
		RPG_Check(data.GameObject == gameObject);

		return &data;
	}


	template<typename TComponent>
	inline const TComponent* GameObject_GetComponent(RpgGameObjectID gameObject) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		const FGameObjectInfo& info = GameObjectInfos[gameObject.Index];
		const int index = info.ComponentIndices[TComponent::TYPE_ID];

		if (index == RPG_COMPONENT_ID_INVALID)
		{
			return nullptr;
		}

		const TComponent& data = Component_GetStorage<TComponent>()->Get(index);
		RPG_Check(data.GameObject == gameObject);

		return &data;
	}


	inline int GameObject_GetCount() const noexcept
	{
		return GameObjectInfos.GetCount();
	}

	inline bool GameObject_IsTransient(RpgGameObjectID gameObject) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		return GameObjectInfos[gameObject.Index].Flags & FLAG_Transient;
	}


	// Check if gameobject is spawned
	// @param gameObject - Gameobject to check
	// @return TRUE if spawned
	inline bool GameObject_IsSpawned(RpgGameObjectID gameObject) const noexcept
	{
		return GameObject_IsValid(gameObject) && (GameObjectInfos[gameObject.Index].Flags & FLAG_Spawned);
	}


	inline bool GameObject_IsTransformUpdated(RpgGameObjectID gameObject) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		return GameObjectInfos[gameObject.Index].Flags & FLAG_TransformUpdated;
	}


private:
	inline void GameObjectScript_StartPlay(int index) noexcept
	{
		RpgGameObjectScript* script = GameObjectScripts[index];
		RPG_Check(script);

		if (bHasStartedPlay && !script->bStartedPlay)
		{
			script->StartPlay();
			script->bStartedPlay = true;
		}
	}


	inline void GameObjectScript_StopPlay(int index) noexcept
	{
		RpgGameObjectScript* script = GameObjectScripts[index];
		RPG_Check(script);

		if (script->bStartedPlay)
		{
			script->StopPlay();
			script->bStartedPlay = false;
		}
	}


	inline void GameObjectScript_Remove(int index) noexcept
	{
		RpgGameObjectScript* script = GameObjectScripts[index];
		RPG_Check(script);

		script->DetachedFromGameObject();
		script->World = nullptr;
		script->GameObject = RpgGameObjectID();
		script->CachedWorldScriptIndex = RPG_INDEX_INVALID;
		script->CachedObjectScriptIndex = RPG_INDEX_INVALID;

		GameObjectScripts.RemoveAt(index);
	}


private:
	enum EGameObjectFlag : uint16_t
	{
		FLAG_None				= (0),
		FLAG_Allocated			= (1 << 0),
		FLAG_Transient			= (1 << 1),
		FLAG_Loading			= (1 << 2),
		FLAG_Loaded				= (1 << 3),
		FLAG_Spawned			= (1 << 4),
		FLAG_PendingDestroy		= (1 << 5),
		FLAG_TransformUpdated	= (1 << 6),
	};

	struct FGameObjectInfo
	{
		// Level where the gameobject spawned on
		RpgLevel* Level{ nullptr };

		// Component index for each type
		uint16_t ComponentIndices[RPG_COMPONENT_TYPE_MAX_COUNT]{};

		// Generation number
		uint16_t Gen{ 0 };

		// Flags
		uint16_t Flags{ 0 };

		// Script indices
		int16_t ScriptIndices[RPG_GAMEOBJECT_MAX_SCRIPT]{};
	};

	struct FGameObjectTransform
	{
		RpgMatrixTransform LocalMatrix;
		RpgMatrixTransform WorldMatrix;
		RpgMatrixTransform InverseWorldMatrix;
	};

	RpgFreeList<RpgName> GameObjectNames;
	RpgFreeList<FGameObjectInfo> GameObjectInfos;
	RpgFreeList<FGameObjectTransform> GameObjectTransforms;

	RpgArray<RpgGameObjectScript*> GameObjectScripts;

};
