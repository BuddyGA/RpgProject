#pragma once

#include "../RpgMath.h"
#include "../RpgString.h"
#include "RpgComponent.h"


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
		UpdatePriority = 0;
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
	uint8_t UpdatePriority;


	friend RpgWorld;

};




class RpgWorld
{
	RPG_NOCOPY(RpgWorld)

public:
	RpgWorld(const RpgName& name) noexcept;
	~RpgWorld() noexcept;

	void BeginFrame(int frameIndex) noexcept;
	void EndFrame(int frameIndex) noexcept;

	void DispatchStartPlay() noexcept;
	void DispatchStopPlay() noexcept;
	void DispatchTickUpdate(float deltaTimeSeconds) noexcept;
	void DispatchPostTickUpdate() noexcept;
	void DispatchRender(int frameIndex, RpgRenderer* renderer) noexcept;


	[[nodiscard]] inline const RpgName& GetName() const noexcept
	{
		return Name;
	}

	[[nodiscard]] inline bool HasStartedPlay() const noexcept
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
	inline void Subsystem_Register(uint8_t updatePriority = 0) noexcept
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
		subsystem->UpdatePriority = updatePriority;

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
		TComponent::TYPE_ID = ComponentStorages.GetCount();
		RPG_CheckV(TComponent::TYPE_ID >= 0 && TComponent::TYPE_ID < RPG_COMPONENT_TYPE_MAX_COUNT, "RpgWorld: Exceeds maximum component type count!");
		ComponentStorages.AddValue(new RpgComponentStorage<TComponent>());
	}

	template<typename TComponent>
	[[nodiscard]] inline RpgComponentStorage<TComponent>* Component_GetStorage() noexcept
	{
		return static_cast<RpgComponentStorage<TComponent>*>(ComponentStorages[TComponent::TYPE_ID]);
	}

	template<typename TComponent>
	[[nodiscard]] inline const RpgComponentStorage<TComponent>* Component_GetStorage() const noexcept
	{
		return static_cast<const RpgComponentStorage<TComponent>*>(ComponentStorages[TComponent::TYPE_ID]);
	}

	template<typename TComponent>
	[[nodiscard]] inline RpgFreeList<TComponent>::Iterator Component_CreateIterator() noexcept
	{
		return Component_GetStorage<TComponent>()->GetComponents().CreateIterator();
	}

	template<typename TComponent>
	[[nodiscard]] inline RpgFreeList<TComponent>::ConstIterator Component_CreateConstIterator() const noexcept
	{
		return Component_GetStorage<TComponent>()->GetComponents().CreateConstIterator();
	}


private:
	RpgArrayInline<RpgComponentStorageInterface*, RPG_COMPONENT_TYPE_MAX_COUNT> ComponentStorages;



// --------------------------------------------------------------------------------------------------------------------------------------------- //
// 	GameObject interface
// --------------------------------------------------------------------------------------------------------------------------------------------- //
public:
	[[nodiscard]] RpgGameObjectID GameObject_Create(const RpgName& name, const RpgTransform& worldTransform = RpgTransform()) noexcept;
	void GameObject_Destroy(RpgGameObjectID& gameObject) noexcept;


	[[nodiscard]] inline bool GameObject_IsValid(RpgGameObjectID gameObject) const noexcept
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


	[[nodiscard]] inline RpgTransform GameObject_GetWorldTransform(RpgGameObjectID gameObject) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		RpgTransform transform;
		GameObjectTransforms[gameObject.Index].WorldMatrix.Decompose(transform.Position, transform.Rotation, transform.Scale);

		return transform;
	}


	[[nodiscard]] inline const RpgName& GameObject_GetName(RpgGameObjectID gameObject) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		return GameObjectNames[gameObject.Index];
	}


	[[nodiscard]] inline const RpgMatrixTransform& GameObject_GetWorldTransformMatrix(RpgGameObjectID gameObject) const noexcept
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
			data.Destroy();
		}
		storage->Remove(index);

		info.ComponentIndices[TComponent::TYPE_ID] = RPG_COMPONENT_ID_INVALID;
	}


	template<typename TComponent>
	[[nodiscard]] inline TComponent* GameObject_GetComponent(RpgGameObjectID gameObject) noexcept
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
	[[nodiscard]] inline const TComponent* GameObject_GetComponent(RpgGameObjectID gameObject) const noexcept
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


	inline void GameObject_AttachScript(RpgGameObjectID gameObject, RpgGameObjectScript* script) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		
		RPG_Check(script);
		const char* scriptTypeName = script->GetTypeName();

		FGameObjectInfo& info = GameObjectInfos[gameObject.Index];

		int emptyIndex = RPG_INDEX_INVALID;

		for (int i = 0; i < RPG_GAMEOBJECT_MAX_SCRIPT; ++i)
		{
			const int scriptIndex = info.ScriptIndices[i];

			if (scriptIndex != RPG_INDEX_INVALID)
			{
				if (GameObjectScripts[scriptIndex]->GetTypeName() == scriptTypeName)
				{
					RPG_LogWarn(RpgLogWorld, "Script of type (%s) already exists! Ignore attach script to game object (%s)", scriptTypeName, *GameObjectNames[gameObject.Index]);
				}
			}
			else
			{
				emptyIndex = i;
			}
		}

		RPG_CheckV(emptyIndex != RPG_INDEX_INVALID, "Cannot add script into game object (%s). Exceeds maximum limit (%i) of scripts per game object!", 
			scriptTypeName, RPG_GAMEOBJECT_MAX_SCRIPT
		);

		info.ScriptIndices[emptyIndex] = GameObjectScripts.GetCount();
		GameObjectScripts.AddValue(script);

		script->World = this;
		script->GameObject = gameObject;
		script->AttachedToGameObject();

		RPG_Log(RpgLogWorld, "Attached script of type (%s) to game object (%s)", scriptTypeName, *GameObjectNames[gameObject.Index]);

		if (bHasStartedPlay && !script->bStartedPlay)
		{
			script->StartPlay();
			script->bStartedPlay;
		}
	}


	inline void GameObject_DetachScript(RpgGameObjectID gameObject, RpgGameObjectScript* script) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		RPG_Check(script);
		const char* scriptTypeName = script->GetTypeName();

		RPG_Check(GameObject_IsValid(gameObject));
		FGameObjectInfo& info = GameObjectInfos[gameObject.Index];

		for (int i = 0; i < RPG_GAMEOBJECT_MAX_SCRIPT; ++i)
		{
			const int scriptIndex = info.ScriptIndices[i];
			if (scriptIndex == RPG_INDEX_INVALID)
			{
				continue;
			}

			if (GameObjectScripts[scriptIndex] == script)
			{
				RPG_Check(GameObjectScripts[scriptIndex]->GetTypeName() == scriptTypeName);
				GameObject_RemoveScriptAtIndex(scriptIndex);
				info.ScriptIndices[i] = RPG_INDEX_INVALID;
				RPG_Log(RpgLogWorld, "Detached script of type (%s) from game object (%s)", scriptTypeName, *GameObjectNames[gameObject.Index]);

				return;
			}
		}

		RPG_LogWarn(RpgLogWorld, "Script of type (%s) not found! Ignore remove script from game object (%s)", scriptTypeName, *GameObjectNames[gameObject.Index]);
	}


	[[nodiscard]] inline int GameObject_GetCount() const noexcept
	{
		return GameObjectInfos.GetCount();
	}


	[[nodiscard]] inline bool GameObject_IsTransformUpdated(RpgGameObjectID gameObject) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		return GameObjectInfos[gameObject.Index].Flags & FLAG_TransformUpdated;
	}


private:
	inline void GameObject_RemoveScriptAtIndex(int index) noexcept
	{
		RpgGameObjectScript* script = GameObjectScripts[index];
		RPG_Check(script);

		script->DetachedFromGameObject();
		script->World = nullptr;
		script->GameObject = RpgGameObjectID();

		if (script->bStartedPlay)
		{
			script->StopPlay();
		}

		GameObjectScripts.RemoveAt(index);
	}


private:
	enum EGameObjectFlag : uint16_t
	{
		FLAG_None				= (0),
		FLAG_Allocated			= (1 << 0),
		FLAG_Loading			= (1 << 1),
		FLAG_Loaded				= (1 << 2),
		FLAG_PendingDestroy		= (1 << 3),
		FLAG_TransformUpdated	= (1 << 4),
	};

	struct FGameObjectInfo
	{
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
		RpgGameObjectID Parent;
	};

	RpgFreeList<RpgName> GameObjectNames;
	RpgFreeList<FGameObjectInfo> GameObjectInfos;
	RpgFreeList<FGameObjectTransform> GameObjectTransforms;

	RpgArray<RpgGameObjectScript*> GameObjectScripts;

};
