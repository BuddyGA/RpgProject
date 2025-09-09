#include "RpgLevel.h"
#include "../RpgConsoleSystem.h"


RPG_LOG_DECLARE_CATEGORY_STATIC(RpgLogLevel, VERBOSITY_DEBUG)



RpgLevel::RpgLevel(const RpgName& in_Name) noexcept
	: RpgAssetObject(in_Name)
	, FrameDatas()
	, FrameIndex(0)
	, ComponentStorages()
	, State(STATE_NONE)
	, bHasStartedPlay(false)
{
	Bound = RpgBoundingAABB(RpgVector3(-50.0f), RpgVector3(50.0f));

	GameObjectNames.Reserve(64);
	GameObjectStates.Reserve(64);
	GameObjectTransforms.Reserve(64);
	GameObjectComponentScripts.Reserve(64);
}


RpgLevel::~RpgLevel() noexcept
{
	RPG_LogDebug(RpgLogLevel, "Destroy level (%s)", *GetAssetName());

	for (int compType = 0; compType < RPG_COMPONENT_TYPE_MAX_COUNT; ++compType)
	{
		if (ComponentStorages[compType])
		{
			delete ComponentStorages[compType];
			ComponentStorages[compType] = nullptr;
		}
	}
}


void RpgLevel::AssetStreamWrite(RpgStreamWriter& writer) noexcept
{
	RpgArray<int> gameObjectIds;
	gameObjectIds.Reserve(GameObjectStates.GetCount());

	for (auto it = GameObjectStates.CreateConstIterator(); it; ++it)
	{
		const FGameObjectState state = it.GetValue();

		// ignore transient or pending-destroy
		if (state.Flags & (RpgGameObjectFlag::Transient | RpgGameObjectFlag::PendingDestroy))
		{
			continue;
		}

		gameObjectIds.AddValue(it.GetIndex());
	}
	
	// gameobject count
	writer.Write(gameObjectIds.GetCount());

	// for-each gameobject
	for (int i : gameObjectIds)
	{
		const int id = gameObjectIds[i];
		
		const FGameObjectState state = GameObjectStates[id];
		RPG_Check(!(state.Flags & (RpgGameObjectFlag::Transient | RpgGameObjectFlag::PendingDestroy)));
		
		// name
		const RpgName& name = GameObjectNames[id];
		writer.Write(name);

		// transform
		const FGameObjectTransform& transform = GameObjectTransforms[id];
		writer.Write(transform.LocalTransformMatrix);
		writer.Write(transform.WorldTransformMatrix);
		writer.Write(transform.InverseWorldTransformMatrix);

		if (!transform.Parent.IsNull())
		{
			RPG_Check(GameObject_IsValid(transform.Parent));
			writer.Write(transform.Parent.Index);
		}

		// components
		const FGameObjectComponentScript& compScript = GameObjectComponentScripts[id];
		writer.WriteData(compScript.Components, sizeof(int16_t) * RPG_COMPONENT_TYPE_MAX_COUNT);

		// for-each component type
		for (int compType = 0; compType < RPG_COMPONENT_TYPE_MAX_COUNT; ++compType)
		{
			const int compId = compScript.Components[compType];
			if (compId == RPG_INDEX_INVALID)
			{
				continue;
			}

			ComponentStorages[compType]->StreamWrite(compId, RpgGameObject(this, id, state.Gen), writer);
		}
	}
}


void RpgLevel::AssetStreamRead(RpgStreamReader& reader, uint16_t version) noexcept
{

}


bool RpgLevel::IsAssetLoaded() noexcept
{
	if (State == STATE_LOADED)
	{
		return true;
	}

	RPG_Check(State == STATE_LOADING);

	for (auto it = GameObjectStates.CreateIterator(); it; ++it)
	{
		FGameObjectState& state = it.GetValue();
		
		if (state.Flags & RpgGameObjectFlag::Loading)
		{
			
		}
	}

	State = STATE_LOADED;

	return true;
}


void RpgLevel::GetExternalAssetReferences(RpgAssetReferences& out_AssetRefs) noexcept
{
	for (auto it = GameObjectStates.CreateConstIterator(); it; ++it)
	{
		const int id = it.GetIndex();
		const FGameObjectState state = it.GetValue();

		// ignore transient or pending-destroy
		if (state.Flags & (RpgGameObjectFlag::Transient | RpgGameObjectFlag::PendingDestroy))
		{
			continue;
		}

		GameObject_GetExternalAssetReferences(id, out_AssetRefs);
	}
}


void RpgLevel::SetAssetLoading() noexcept
{
	State = STATE_LOADING;
}


void RpgLevel::BeginFrame(int frameIndex) noexcept
{
	FrameIndex = frameIndex;
	FFrameData& frame = FrameDatas[FrameIndex];
	
	for (int i = 0; i < frame.PendingDestroyGameObjects.GetCount(); ++i)
	{
		const RpgGameObject gameObject = frame.PendingDestroyGameObjects[i];
		const int index = gameObject.Index;

		RpgName& name = GameObjectNames[index];
		RPG_LogDebug(RpgLogTemp, "Destroy gameobject (%s)", *name);
		name = "__empty__";

		FGameObjectState& state = GameObjectStates[index];
		state.Flags = RpgGameObjectFlag::None;
		++state.Gen;

		// remove components
		FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];

		for (int compType = 0; compType < RPG_COMPONENT_TYPE_MAX_COUNT; ++compType)
		{
			const int compId = compScript.Components[compType];

			if (compId != RPG_INDEX_INVALID)
			{
				ComponentStorages[compType]->Remove(compId, gameObject);
				compScript.Components[compType] = RPG_INDEX_INVALID;
			}
		}

		// deallocate gameobject
		GameObjectNames.RemoveAt(index);
		GameObjectStates.RemoveAt(index);
		GameObjectTransforms.RemoveAt(index);
		GameObjectComponentScripts.RemoveAt(index);
	}

	frame.PendingDestroyGameObjects.Clear();
}


void RpgLevel::EndFrame(int frameIndex) noexcept
{
	for (auto it = GameObjectStates.CreateIterator(); it; ++it)
	{
		FGameObjectState& state = it.GetValue();
		state.Flags &= ~RpgGameObjectFlag::TransformUpdated;
	}
}


void RpgLevel::StartPlay() noexcept
{
	RPG_IsMainThread();

	if (bHasStartedPlay)
	{
		return;
	}

	for (int i = 0; i < AttachedScripts.GetCount(); ++i)
	{
		GameObjectScript_StartPlay(i);
	}

	bHasStartedPlay = true;
}


void RpgLevel::StopPlay() noexcept
{
	RPG_IsMainThread();

	if (!bHasStartedPlay)
	{
		return;
	}

	for (int i = 0; i < AttachedScripts.GetCount(); ++i)
	{
		GameObjectScript_StopPlay(i);
	}

	bHasStartedPlay = false;
}


void RpgLevel::TickUpdate(float deltaTime) noexcept
{
	RPG_IsMainThread();

	for (int i = 0; i < AttachedScripts.GetCount(); ++i)
	{
		RpgGameObjectScript* script = AttachedScripts[i];
		RPG_Check(script);

		if (script->TickUpdateOption == RpgTickUpdateOption::NO_UPDATE || (script->TickUpdateOption == RpgTickUpdateOption::STARTED_PLAY && !bHasStartedPlay))
		{
			continue;
		}

		script->TickUpdate(deltaTime);
	}
}


RpgGameObject RpgLevel::GameObject_Create(const RpgName& name, bool bIsTransient) noexcept
{
	RPG_IsMainThread();

	const int nameId = GameObjectNames.Add();
	const int stateId = GameObjectStates.Add();
	const int trxId = GameObjectTransforms.Add();
	const int compId = GameObjectComponentScripts.Add();
	RPG_Check(nameId < UINT16_MAX && nameId == stateId && stateId == trxId && trxId == compId);

	GameObjectNames[nameId] = name;

	FGameObjectState& state = GameObjectStates[stateId];
	state.Flags = RpgGameObjectFlag::Allocated | RpgGameObjectFlag::Loaded | RpgGameObjectFlag::Visible;

	if (bIsTransient)
	{
		state.Flags |= RpgGameObjectFlag::Transient;
	}

	FGameObjectTransform& transform = GameObjectTransforms[trxId];
	transform.LocalTransformMatrix = RpgMatrixTransform();
	transform.WorldTransformMatrix = RpgMatrixTransform();
	transform.InverseWorldTransformMatrix = RpgMatrixTransform();
	transform.Parent = RpgGameObject();
	transform.Children.Clear();
	transform.Dirty = EDirty::DIRTY_NONE;

	FGameObjectComponentScript& compScript = GameObjectComponentScripts[compId];
	RpgPlatformMemory::MemSet(compScript.Components, RPG_INDEX_INVALID, sizeof(int16_t) * RPG_COMPONENT_TYPE_MAX_COUNT);
	compScript.ScriptIndex = RPG_INDEX_INVALID;

	return RpgGameObject(this, nameId, state.Gen);
}


void RpgLevel::GameObject_Destroy(RpgGameObject gameObject) noexcept
{
	RPG_IsMainThread();

	if (!GameObject_IsValid(gameObject))
	{
		return;
	}

	const int index = gameObject.Index;

	// Spawned OFF, PendingDestroy ON
	FGameObjectState& state = GameObjectStates[index];
	state.Flags = (state.Flags & ~RpgGameObjectFlag::Spawned) | RpgGameObjectFlag::PendingDestroy;
	GameObject_UpdateComponentFlags(gameObject);

	FGameObjectTransform& transform = GameObjectTransforms[index];

	// detach from parent
	GameObject_InternalDetachFromParent(gameObject, true);

	// destroy children
	for (int i = 0; i < transform.Children.GetCount(); ++i)
	{
		GameObject_Destroy(transform.Children[i]);
	}

	// remove scripts
	FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];
	if (compScript.ScriptIndex != RPG_INDEX_INVALID)
	{
		GameObjectScript_StopPlay(compScript.ScriptIndex);
		GameObjectScript_Remove(compScript.ScriptIndex);
		compScript.ScriptIndex = RPG_INDEX_INVALID;
	}

	// add to pending destroy
	FrameDatas[FrameIndex].PendingDestroyGameObjects.AddValue(gameObject);

	RPG_LogDebug(RpgLogLevel, "Mark gameobject (%s) as pending destroy", *GameObjectNames[index]);
}


void RpgLevel::GameObject_AttachToParent(RpgGameObject gameObject, RpgGameObject parent, RpgGameObjectAttachMode mode) noexcept
{
	RPG_Check(GameObject_IsValid(gameObject));
	RPG_Check(GameObject_IsValid(parent));

	// ignore attach to self
	if (gameObject == parent)
	{
		return;
	}

	FGameObjectTransform& transform = GameObjectTransforms[gameObject.Index];

	// Check if parent is one of children
	for (int i = 0; i < transform.Children.GetCount(); ++i)
	{
		const RpgGameObject child = transform.Children[i];
		RPG_Check(GameObject_IsValid(child));

		if (child == parent)
		{
			const FGameObjectTransform& childTransform = GameObjectTransforms[child.Index];
			RPG_Check(childTransform.Parent == gameObject);
			GameObject_InternalDetachFromParent(child, false);

			break;
		}
	}

	// detach from prev parent
	GameObject_InternalDetachFromParent(gameObject, false);

	// set parent
	transform.Parent = parent;

	// adjust transform
	if (mode == RpgGameObjectAttachMode::RESET_TRANSFORM)
	{
		transform.LocalTransformMatrix = RpgMatrixTransform();
		transform.WorldTransformMatrix = RpgMatrixTransform();
		transform.InverseWorldTransformMatrix = RpgMatrixTransform();
	}
	else if (mode == RpgGameObjectAttachMode::KEEP_LOCAL_TRANSFORM)
	{
		transform.WorldTransformMatrix = RpgMatrixTransform();
		transform.InverseWorldTransformMatrix = RpgMatrixTransform();
		transform.Dirty = DIRTY_WORLD;
	}
	else // mode == RpgGameObjectAttachMode::KEEP_WORLD_TRANSFORM
	{
		transform.LocalTransformMatrix = RpgMatrixTransform();
		transform.Dirty = DIRTY_LOCAL;
	}

	// add child to parent
	GameObjectTransforms[parent.Index].Children.AddValue(gameObject);
}


void RpgLevel::GameObject_Spawn(RpgGameObject gameObject, const RpgTransform& worldTransform) noexcept
{
	RPG_IsMainThread();
	RPG_Check(GameObject_IsValid(gameObject));

	const int index = gameObject.Index;
	
	FGameObjectTransform& transform = GameObjectTransforms[index];
	transform.WorldTransformMatrix = worldTransform.ToMatrixTransform();
	transform.InverseWorldTransformMatrix = transform.WorldTransformMatrix.GetInverse();
	transform.Dirty = DIRTY_LOCAL;

	GameObjectStates[index].Flags |= RpgGameObjectFlag::Spawned;
	GameObject_UpdateComponentFlags(gameObject);

	FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];
	if (compScript.ScriptIndex != RPG_INDEX_INVALID)
	{
		GameObjectScript_StartPlay(compScript.ScriptIndex);
	}
}


void RpgLevel::GameObject_AttachScript(RpgGameObject gameObject, RpgGameObjectScript* script) noexcept
{
	RPG_IsMainThread();
	RPG_Check(GameObject_IsValid(gameObject));
	RPG_Check(script);

	const int index = gameObject.Index;
	const char* scriptTypeName = script->GetTypeName();

	// check if script already attached to any gameobject
	const int checkScriptIndex = AttachedScripts.FindIndexByValue(script);
	if (checkScriptIndex != RPG_INDEX_INVALID)
	{
		RPG_Check(GameObject_IsValid(script->GameObject));
		RPG_CONSOLE_Warn(RpgLogLevel, "Script (%s) has been attached to gameobject (%s). Ignore attach script!", scriptTypeName, *GameObjectNames[script->GameObject.Index]);
		return;
	}

	const int attachedScriptIndex = AttachedScripts.GetCount();
	AttachedScripts.AddValue(script);

	GameObjectComponentScripts[index].ScriptIndex = attachedScriptIndex;

	script->GameObject = gameObject;
	script->AttachedScriptIndex = attachedScriptIndex;
	script->AttachedToGameObject();

	RPG_CONSOLE_Log(RpgLogLevel, "Attached script (%s) to gameobject (%s)", scriptTypeName, *GameObjectNames[index]);
}


void RpgLevel::GameObject_DetachScript(RpgGameObject gameObject, RpgGameObjectScript* script) noexcept
{
	RPG_IsMainThread();

	if (script == nullptr)
	{
		return;
	}

	RPG_Check(GameObject_IsValid(gameObject));

	const int index = gameObject.Index;
	const char* scriptTypeName = script->GetTypeName();

	if (gameObject != script->GameObject)
	{
		RPG_Check(GameObject_IsValid(script->GameObject));

		RPG_CONSOLE_Warn(RpgLogLevel, "Cannot detach script (%s) from gameobject (%s) because script is attached to gameobject (%s)",
			scriptTypeName,
			*GameObjectNames[index],
			*GameObjectNames[script->GameObject.Index]
		);

		return;
	}

	const int attachedScriptIndex = script->AttachedScriptIndex;
	FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];
	RPG_Check(attachedScriptIndex != RPG_INDEX_INVALID && attachedScriptIndex == compScript.ScriptIndex && script == AttachedScripts[attachedScriptIndex]);

	GameObjectScript_StopPlay(attachedScriptIndex);
	GameObjectScript_Remove(attachedScriptIndex);
	compScript.ScriptIndex = RPG_INDEX_INVALID;

	RPG_CONSOLE_Log(RpgLogLevel, "Detached script (%s) from gameobject (%s)", script->GetTypeName(), *GameObjectNames[index]);
}


void RpgLevel::GameObject_UpdateComponentFlags(RpgGameObject gameObject) noexcept
{
	RPG_IsMainThread();

	const int index = gameObject.Index;
	const uint32_t flags = GameObjectStates[index].Flags;
	const FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];

	for (int compType = 0; compType < RPG_COMPONENT_TYPE_MAX_COUNT; ++compType)
	{
		const int compId = compScript.Components[compType];

		if (compId != RPG_INDEX_INVALID)
		{
			ComponentStorages[compType]->SetFlags(compId, gameObject, flags);
		}
	}
}


void RpgLevel::GameObject_InternalDetachFromParent(RpgGameObject gameObject, bool bIsDestroying) noexcept
{
	FGameObjectTransform& transform = GameObjectTransforms[gameObject.Index];
	if (transform.Parent.IsNull())
	{
		return;
	}

	RPG_Check(GameObject_IsValid(transform.Parent));
	FGameObjectTransform& parentTransform = GameObjectTransforms[transform.Parent.Index];
	parentTransform.Children.RemoveByValue(gameObject, false);

	transform.Parent = RpgGameObject();
	transform.LocalTransformMatrix = transform.WorldTransformMatrix;
	transform.Dirty = DIRTY_WORLD;

	if (!bIsDestroying)
	{
		GameObject_UpdateTransform(gameObject);
	}
}


void RpgLevel::GameObject_UpdateTransform(RpgGameObject gameObject) noexcept
{
	FGameObjectTransform& transform = GameObjectTransforms[gameObject.Index];
	if (transform.Dirty == DIRTY_NONE)
	{
		return;
	}

	const FGameObjectTransform* parentTransform = nullptr;
	if (!transform.Parent.IsNull())
	{
		RPG_Check(GameObject_IsValid(transform.Parent));
		parentTransform = &GameObjectTransforms[transform.Parent.Index];
		RPG_Check(parentTransform->Dirty == DIRTY_NONE);
	}

	if (transform.Dirty == DIRTY_LOCAL)
	{
		transform.LocalTransformMatrix = parentTransform ? transform.WorldTransformMatrix * parentTransform->InverseWorldTransformMatrix : transform.WorldTransformMatrix;
	}
	else // Dirty == DIRTY_WORLD
	{
		const RpgMatrixTransform& localTransformMatrix = transform.LocalTransformMatrix;
		transform.WorldTransformMatrix = parentTransform ? localTransformMatrix * parentTransform->WorldTransformMatrix : localTransformMatrix;
		transform.InverseWorldTransformMatrix = transform.WorldTransformMatrix.GetInverse();
	}

	transform.Dirty = DIRTY_NONE;

	for (int i = 0; i < transform.Children.GetCount(); ++i)
	{
		FGameObjectTransform& childTransform = GameObjectTransforms[i];
		childTransform.Dirty = DIRTY_WORLD;
	}

	GameObjectStates[gameObject.Index].Flags |= RpgGameObjectFlag::TransformUpdated;
	GameObject_UpdateComponentFlags(gameObject);
}


void RpgLevel::GameObject_GetExternalAssetReferences(int id, RpgAssetReferences& out_AssetRefs) const noexcept
{
	const FGameObjectComponentScript& compScript = GameObjectComponentScripts[id];

	for (int compType = 0; compType < RPG_COMPONENT_TYPE_MAX_COUNT; ++compType)
	{
		const int compId = compScript.Components[compType];
		if (compId == RPG_INDEX_INVALID)
		{
			continue;
		}

		ComponentStorages[compType]->GetExternalAssetReferences(compId, out_AssetRefs);
	}
}


bool RpgLevel::GameObject_IsLoaded(int id) noexcept
{
	FGameObjectState& state = GameObjectStates[id];

	if (state.Flags & RpgGameObjectFlag::Loaded)
	{
		return true;
	}

	RPG_Check(state.Flags & RpgGameObjectFlag::Loading);

	state.Flags = (state.Flags & ~RpgGameObjectFlag::Loading) | RpgGameObjectFlag::Loaded;
	
	return true;
}
