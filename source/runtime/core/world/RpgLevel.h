#pragma once

#include "RpgComponent.h"
#include "../asset/RpgAssetTypes.h"
#include "../RpgPointer.h"


extern void Rpg_RegisterComponents(RpgLevel* level) noexcept;



typedef RpgSharedPtr<RpgLevel> RpgSharedLevel;

class RpgLevel : public RpgAssetObject
{
	RPG_ASSET_CLASS(RpgLevel, RpgAssetFileType::LEVEL, 1);

public:
	RpgLevel(const RpgName& in_Name) noexcept;
	virtual ~RpgLevel() noexcept;


// Begin RpgAssetObject interfaces //
public:
	virtual void AssetStreamWrite(RpgStreamWriter& writer) noexcept override;
	virtual void AssetStreamRead(RpgStreamReader& reader, uint16_t version) noexcept override;
	virtual bool IsAssetLoaded() noexcept override;
	virtual void GetExternalAssetReferences(RpgAssetReferences& out_AssetRefs) noexcept override;

protected:
	virtual void SetAssetLoading() noexcept override;
// End RpgAssetObject interfaces //


public:
	void BeginFrame(int frameIndex) noexcept;
	void EndFrame(int frameIndex) noexcept;
	void StartPlay() noexcept;
	void StopPlay() noexcept;
	void TickUpdate(float deltaTime) noexcept;
	float GetLoadingProgress() const noexcept;


	inline const RpgBoundingAABB& GetBound() const noexcept
	{
		return Bound;
	}

	inline bool IsLoading() const noexcept
	{
		return LoadingStatus.State == STATE_LOADING;
	}

	inline bool IsLoaded() const noexcept
	{
		return LoadingStatus.State == STATE_LOADED;
	}


private:
	RpgBoundingAABB Bound;


	struct FFrameData
	{
		RpgArray<RpgGameObject> PendingDestroyGameObjects;
	};

	FFrameData FrameDatas[RPG_FRAME_BUFFERING];
	int FrameIndex;


	enum EState : uint8_t
	{
		STATE_NONE = 0,
		STATE_LOADING,
		STATE_LOADED
	};


	struct FLoadingStatus
	{
		int TotalGameObjectCount{ 0 };
		int LoadedGameObjectCount{ 0 };
		uint8_t State = STATE_NONE;
	};
	FLoadingStatus LoadingStatus;


	bool bHasStartedPlay;


public:
	template<typename TComponent>
	inline void Component_Register() noexcept
	{
		RPG_Check(ComponentStorages[TComponent::TYPE_ID] == nullptr);
		ComponentStorages[TComponent::TYPE_ID] = new RpgComponentStorage<TComponent>();
	}

	template<typename TComponent>
	inline RpgFreeList<TComponent>::Iterator Component_Iterator() noexcept
	{
		return Component_GetStorage<TComponent>()->GetComponents().CreateIterator();
	}

	template<typename TComponent>
	inline RpgFreeList<TComponent>::ConstIterator Component_ConstIterator() const noexcept
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
	RpgComponentStorageInterface* ComponentStorages[RPG_COMPONENT_TYPE_MAX_COUNT];



public:
	RpgGameObject GameObject_Create(const RpgName& name, bool bIsTransient = false) noexcept;
	void GameObject_Destroy(RpgGameObject gameObject) noexcept;
	void GameObject_AttachToParent(RpgGameObject gameObject, RpgGameObject parent, RpgGameObjectAttachMode mode) noexcept;
	void GameObject_Spawn(RpgGameObject gameObject, const RpgTransform& worldTransform) noexcept;


	inline bool GameObject_IsValid(RpgGameObject gameObject) const noexcept
	{
		if (gameObject.IsNull() || !GameObjectStates.IsValid(gameObject.Index))
		{
			return false;
		}

		const FGameObjectState state = GameObjectStates[gameObject.Index];

		return (gameObject.Level == this) && (state.Gen == gameObject.Gen) && !(state.Flags & RpgGameObjectFlag::PendingDestroy);
	}


	inline const RpgName& GameObject_GetName(RpgGameObject gameObject) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		return GameObjectNames[gameObject.Index];
	}


	inline void GameObject_DetachFromParent(RpgGameObject gameObject) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		GameObject_InternalDetachFromParent(gameObject, false);
	}


	inline void GameObject_SetLocalTransform(RpgGameObject gameObject, const RpgTransform& localTransform) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		GameObject_UpdateTransform(gameObject);

		FGameObjectTransform& transform = GameObjectTransforms[gameObject.Index];
		//transform.LocalTransformMatrix = localTransform.ToMatrixTransform();
		transform.LocalTransform = localTransform;
		transform.Dirty = DIRTY_WORLD;
	}


	inline const RpgTransform& GameObject_GetLocalTransform(RpgGameObject gameObject) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		GameObject_UpdateTransform(gameObject);
		
		/*
		RpgTransform transform;
		GameObjectTransforms[gameObject.Index].LocalTransformMatrix.Decompose(transform.Position, transform.Rotation, transform.Scale);

		return transform;
		*/
		return GameObjectTransforms[gameObject.Index].LocalTransform;
	}


	inline void GameObject_SetWorldTransform(RpgGameObject gameObject, const RpgTransform& worldTransform) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		GameObject_UpdateTransform(gameObject);

		FGameObjectTransform& transform = GameObjectTransforms[gameObject.Index];
		//transform.WorldTransformMatrix = worldTransform.ToMatrixTransform();
		//transform.InverseWorldTransformMatrix = transform.WorldTransformMatrix.GetInverse();
		transform.WorldTransform = worldTransform;
		transform.Dirty = DIRTY_LOCAL;
	}


	inline const RpgTransform& GameObject_GetWorldTransform(RpgGameObject gameObject) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		GameObject_UpdateTransform(gameObject);

		//RpgTransform transform;
		//GameObjectTransforms[gameObject.Index].WorldTransformMatrix.Decompose(transform.Position, transform.Rotation, transform.Scale);

		//return transform;
		return GameObjectTransforms[gameObject.Index].WorldTransform;
	}


	inline RpgMatrixTransform GameObject_GetWorldTransformMatrix(RpgGameObject gameObject) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		GameObject_UpdateTransform(gameObject);
		return GameObjectTransforms[gameObject.Index].WorldTransform.ToMatrixTransform();
	}


	inline void GameObject_SetVisibility(RpgGameObject gameObject, bool bIsVisible) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));

		FGameObjectState& state = GameObjectStates[gameObject.Index];
		const bool bWasVisible = (state.Flags & RpgGameObjectFlag::Visible);

		if (bWasVisible == bIsVisible)
		{
			return;
		}

		state.Flags = bIsVisible ? (state.Flags | RpgGameObjectFlag::Visible) : (state.Flags & ~RpgGameObjectFlag::Visible);
		GameObject_UpdateComponentFlags(gameObject);
	}


	inline void* GameObject_GetComponent(RpgGameObject gameObject, int compType) const noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		RPG_Check(compType >= 0 && compType < RPG_COMPONENT_TYPE_MAX_COUNT);

		const int index = gameObject.Index;
		const int compId = GameObjectComponentScripts[index].Components[compType];

		return (compId != RPG_INDEX_INVALID) ? ComponentStorages[compType]->Get(compId, gameObject) : nullptr;
	}


	inline void* GameObject_AddComponent(RpgGameObject gameObject, int compType) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		RPG_Check(compType >= 0 && compType < RPG_COMPONENT_TYPE_MAX_COUNT);

		const int index = gameObject.Index;
		FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];
		RpgComponentStorageInterface* storage = ComponentStorages[compType];
		
		int compId = compScript.Components[compType];
		if (compId != RPG_INDEX_INVALID)
		{
			return storage->Get(compId, gameObject);
		}

		const FGameObjectState state = GameObjectStates[index];
		compId = storage->Add(gameObject, state.Flags);
		compScript.Components[compType] = compId;

		return storage->Get(compId, gameObject);
	}


	inline bool GameObject_RemoveComponent(RpgGameObject gameObject, int compType) noexcept
	{
		RPG_Check(GameObject_IsValid(gameObject));
		RPG_Check(compType >= 0 && compType < RPG_COMPONENT_TYPE_MAX_COUNT);

		const int index = gameObject.Index;
		FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];

		const int compId = compScript.Components[compType];
		if (compId == RPG_INDEX_INVALID)
		{
			return false;
		}

		ComponentStorages[compType]->Remove(compId, gameObject);
		compScript.Components[compType] = RPG_INDEX_INVALID;

		return true;
	}


	inline int GameObject_GetCount() const noexcept
	{
		return GameObjectStates.GetCount();
	}


	void GameObject_AttachScript(RpgGameObject gameObject, RpgGameObjectScript* script) noexcept;
	void GameObject_DetachScript(RpgGameObject gameObject, RpgGameObjectScript* script) noexcept;


private:
	void GameObject_UpdateComponentFlags(RpgGameObject gameObject) noexcept;
	void GameObject_InternalDetachFromParent(RpgGameObject gameObject, bool bIsDestroying) noexcept;
	void GameObject_UpdateTransform(RpgGameObject gameObject) noexcept;
	void GameObject_GetExternalAssetReferences(int id, RpgAssetReferences& out_AssetRefs) const noexcept;
	

	inline void GameObjectScript_StartPlay(int index) noexcept
	{
		RpgGameObjectScript* script = AttachedScripts[index];
		RPG_Check(script);

		if (bHasStartedPlay && !script->bHasStartedPlay)
		{
			script->StartPlay();
			script->bHasStartedPlay = true;
		}
	}


	inline void GameObjectScript_StopPlay(int index) noexcept
	{
		RpgGameObjectScript* script = AttachedScripts[index];
		RPG_Check(script);

		if (script->bHasStartedPlay)
		{
			script->StopPlay();
			script->bHasStartedPlay = false;
		}
	}


	inline void GameObjectScript_Remove(int index) noexcept
	{
		RpgGameObjectScript* script = AttachedScripts[index];
		RPG_Check(script);

		script->DetachedFromGameObject();
		script->AttachedScriptIndex = RPG_INDEX_INVALID;

		AttachedScripts.RemoveAt(index);
	}


private:
	RpgTransform WorldTransform;
	RpgArray<RpgGameObjectScript*> AttachedScripts;


	RpgFreeList<RpgName> GameObjectNames;


	struct FGameObjectState
	{
		uint32_t Flags{ RpgGameObjectFlag::None };
		uint16_t Gen{ 0 };
	};
	RpgFreeList<FGameObjectState> GameObjectStates;


	enum EDirty : uint8_t
	{
		DIRTY_NONE = 0,
		DIRTY_LOCAL,
		DIRTY_WORLD
	};

	struct FGameObjectTransform
	{
		RpgTransform LocalTransform;
		RpgTransform WorldTransform;
		RpgGameObject Parent;
		RpgGameObjectChildrenArray Children;
		uint8_t Dirty;
	};
	RpgFreeList<FGameObjectTransform> GameObjectTransforms;


	struct FGameObjectComponentScript
	{
		int16_t Components[RPG_COMPONENT_TYPE_MAX_COUNT];
		int16_t ScriptIndex;
	};
	RpgFreeList<FGameObjectComponentScript> GameObjectComponentScripts;


	friend RpgWorld;

};
