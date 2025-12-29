#include "RpgLevel.h"
#include "../RpgConsoleSystem.h"


RPG_LOG_DECLARE_CATEGORY_STATIC(RpgLogLevel, VERBOSITY_DEBUG)



/* Level save format
 
    header
    {
        magix,
        type,
        version,
        asset_ref_count,
        asset_ref_size_bytes,
        asset_data_size_bytes
    }

    asset_ref_begin
    {
        asset_ref_0,
        asset_ref_1,
        ...
    }
    asset_ref_end

    data_begin
    {
		[
			comp_type0_count,
			comp_type1_count,
			...
		],
        gameobject_count,
        gameobject_names
		[
			name_0,
			name_1,
			...
		],
		gameobject_transforms
		[
			{
				local,
				world,
				parentId
			},
			{
				local,
				world,
				parentId
			},
			...
		],
		gameobject_component_scripts
		[
			{
				component_type_ids [ -1, -1, 0, 0, ... ],
				component_datas
				[
					{}, {}, ...
				]
			},
			{
				component_type_ids [ 2, 3, 0, 0, ... ],
				component_datas
				[
					{}, {}, ...
				]
			}
			...
		]
    }
    data_end

    eof

*/


RpgLevel::RpgLevel(const RpgName& in_Name) noexcept
	: RpgAssetObject(in_Name)
	, FrameDatas()
	, FrameIndex(0)
	, ComponentStorages()
	, bHasStartedPlay(false)
{
	Bound = RpgBoundingAABB(RpgVector3(-50.0f), RpgVector3(50.0f));

	Rpg_RegisterComponents(this);

	GameObjectNames.Reserve(64);
	GameObjectStates.Reserve(64);
	GameObjectTransforms.Reserve(64);
	GameObjectComponentScripts.Reserve(64);
}


RpgLevel::~RpgLevel() noexcept
{
	RPG_LogDebug(RpgLogLevel, "Destroy level (%s)", *GetAssetName().ToString());
	
	for (int compId = 0; compId < RPG_COMPONENT_ID_MAX_COUNT; ++compId)
	{
		if (ComponentStorages[compId])
		{
			delete ComponentStorages[compId];
			ComponentStorages[compId] = nullptr;
		}
	}
}


void RpgLevel::AssetStreamWrite(RpgStreamWriter& writer) noexcept
{
	int componentCount[RPG_COMPONENT_ID_MAX_COUNT];
	for (int compId = 0; compId < RPG_COMPONENT_ID_MAX_COUNT; ++compId)
	{
		RpgComponentStorageInterface* compStorage = ComponentStorages[compId];
		componentCount[compId] = compStorage ? compStorage->GetCount() : 0;
	}

	writer.WriteData(componentCount, sizeof(int) * RPG_COMPONENT_ID_MAX_COUNT);

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
	
	// gameobject ids 
	writer.Write(gameObjectIds);

	// foreach gameobject name
	for (int i : gameObjectIds)
	{
		const int id = gameObjectIds[i];
		writer.Write(GameObjectNames[id]);
	}

	// foreach gameobject transform
	for (int i : gameObjectIds)
	{
		const int id = gameObjectIds[i];

		const FGameObjectTransform& transform = GameObjectTransforms[id];
		writer.Write(transform.LocalTransform);
		writer.Write(transform.WorldTransform);

		int parentId = RPG_INDEX_INVALID;
		if (!transform.Parent.IsNull())
		{
			RPG_Check(GameObject_IsValid(transform.Parent));
			parentId = transform.Parent.Index;
		}

		writer.Write(parentId);
	}

	// foreach gameobject componentscripts
	for (int i : gameObjectIds)
	{
		const int gid = gameObjectIds[i];

		const FGameObjectComponentScript& compScript = GameObjectComponentScripts[gid];
		writer.WriteData(compScript.Components, sizeof(int16_t) * RPG_COMPONENT_ID_MAX_COUNT);

		// foreach component type
		for (int compType = 0; compType < RPG_COMPONENT_ID_MAX_COUNT; ++compType)
		{
			const int compId = compScript.Components[compType];

			if (compId != RPG_INDEX_INVALID)
			{
				RpgComponentStorageInterface* compStorage = ComponentStorages[compType];
				RPG_Check(compStorage);
				compStorage->StreamWrite(compId, RpgGameObject(this, gid, GameObjectStates[gid].Gen), writer);
			}
		}
	}
}


void RpgLevel::AssetStreamRead(RpgStreamReader& reader, uint16_t version) noexcept
{
	// reserve component storages
	int componentCount[RPG_COMPONENT_ID_MAX_COUNT];
	reader.ReadData(componentCount, sizeof(int) * RPG_COMPONENT_ID_MAX_COUNT);

	for (int compType = 0; compType < RPG_COMPONENT_ID_MAX_COUNT; ++compType)
	{
		RpgComponentStorageInterface* compStorage = ComponentStorages[compType];
		if (compStorage)
		{
			compStorage->Reserve(componentCount[compType]);
		}
	}

	// gameobject ids
	RpgArray<int> gameObjectIds;
	reader.Read(gameObjectIds);

	const int totalGameObjectCount = gameObjectIds.GetCount();
	LoadingStatus.TotalGameObjectCount = totalGameObjectCount;

	// gameobject names
	GameObjectNames.Resize(totalGameObjectCount);
	for (int i = 0; i < totalGameObjectCount; ++i)
	{
		reader.Read(GameObjectNames[i]);
	}


	// gameobject states
	GameObjectStates.Resize(totalGameObjectCount);
	for (int i = 0; i < totalGameObjectCount; ++i)
	{
		FGameObjectState& state = GameObjectStates[i];
		state.Flags = RpgGameObjectFlag::Allocated | RpgGameObjectFlag::Loading | RpgGameObjectFlag::Visible;
		state.Gen = 0;
	}

	// gameobject transforms
	GameObjectTransforms.Resize(totalGameObjectCount);
	for (int i = 0; i < totalGameObjectCount; ++i)
	{
		FGameObjectTransform& transform = GameObjectTransforms[i];
		reader.Read(transform.LocalTransform);
		reader.Read(transform.WorldTransform);

		int savedParentId = RPG_INDEX_INVALID;
		reader.Read(savedParentId);

		if (savedParentId != RPG_INDEX_INVALID)
		{
			// get the current parent-id based on saved parent-id
			// need to do this because when saving we ignore transient/pending-destroy (which could be a gameobject in the middle of array), meanwhile data loaded linearly
			// therefore, parent-id when saved may not be the same when loaded
			const int parentId = gameObjectIds.FindIndexByValue(savedParentId);
			transform.Parent = RpgGameObject(this, parentId, 0);

			FGameObjectTransform parentTransform = GameObjectTransforms[parentId];
			parentTransform.Children.AddValue(RpgGameObject(this, i, 0));
		}
	}

	// gameobject components
	GameObjectComponentScripts.Resize(totalGameObjectCount);
	for (int i = 0; i < totalGameObjectCount; ++i)
	{
		const RpgGameObject gameObject(this, i, 0);

		FGameObjectComponentScript& compScript = GameObjectComponentScripts[i];
		reader.ReadData(compScript.Components, sizeof(int16_t) * RPG_COMPONENT_ID_MAX_COUNT);

		// foreach component type
		for (int compType = 0; compType < RPG_COMPONENT_ID_MAX_COUNT; ++compType)
		{
			const int compId = compScript.Components[compType];
			if (compId == RPG_INDEX_INVALID)
			{
				continue;
			}

			RpgComponentStorageInterface* compStorage = ComponentStorages[compType];
			compScript.Components[compType] = compStorage->Add(gameObject, GameObjectStates[i].Flags);
			compStorage->StreamRead(compScript.Components[compType], gameObject, reader);
		}
	}
}


bool RpgLevel::IsAssetLoaded() noexcept
{
	return LoadingStatus.State == STATE_LOADED;
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
	Bound = RpgBoundingAABB(RpgVector3(-50.0f), RpgVector3(50.0f));

	for (int f = 0; f < RPG_FRAME_BUFFERING; ++f)
	{
		FrameDatas[f].PendingDestroyGameObjects.Clear();
	}

	FrameIndex = 0;

	LoadingStatus.TotalGameObjectCount = 0;
	LoadingStatus.LoadedGameObjectCount = 0;
	LoadingStatus.State = STATE_LOADING;

	bHasStartedPlay = false;

	for (int compType = 0; compType < RPG_COMPONENT_ID_MAX_COUNT; ++compType)
	{
		if (RpgComponentStorageInterface* compStorage = ComponentStorages[compType])
		{
			compStorage->Clear();
		}
	}

	AttachedScripts.Clear();

	GameObjectNames.Clear();
	GameObjectStates.Clear();
	GameObjectTransforms.Clear();
	GameObjectComponentScripts.Clear();
}


void RpgLevel::BeginFrame(int frameIndex) noexcept
{
	FrameIndex = frameIndex;

	if (LoadingStatus.State == STATE_LOADING)
	{
		for (auto it = GameObjectStates.CreateIterator(); it; ++it)
		{
			const int index = it.GetIndex();

			FGameObjectState& state = it.GetValue();

			if (!(state.Flags & RpgGameObjectFlag::Loading))
			{
				continue;
			}

			bool bAllComponentsLoaded = true;

			const FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];
			for (int compType = 0; compType < RPG_COMPONENT_ID_MAX_COUNT; ++compType)
			{
				const int compId = compScript.Components[compType];
				if (compId == RPG_INDEX_INVALID)
				{
					continue;
				}

				RpgComponentStorageInterface* compStorage = ComponentStorages[compType];
				if (!compStorage->IsValid(compId) || !compStorage->IsLoaded(compId, RpgGameObject(this, index, state.Gen)))
				{
					bAllComponentsLoaded = false;
					break;
				}
			}

			if (bAllComponentsLoaded)
			{
				state.Flags = (state.Flags & ~RpgGameObjectFlag::Loading) | RpgGameObjectFlag::Loaded;
				++LoadingStatus.LoadedGameObjectCount;
			}
		}

		if (LoadingStatus.LoadedGameObjectCount >= LoadingStatus.TotalGameObjectCount)
		{
			RPG_Check(LoadingStatus.LoadedGameObjectCount == LoadingStatus.TotalGameObjectCount);
			LoadingStatus.State = STATE_LOADED;
			RPG_Log(RpgLogLevel, "Level (%s) loaded", *GetAssetName().ToString());

			for (auto it = GameObjectStates.CreateIterator(); it; ++it)
			{
				FGameObjectState& state = it.GetValue();
				state.Flags |= RpgGameObjectFlag::Spawned;
				GameObject_UpdateComponentFlags(RpgGameObject(this, it.GetIndex(), state.Gen));
			}
		}

		return;
	}


	FFrameData& frame = FrameDatas[FrameIndex];

	for (int i = 0; i < frame.PendingDestroyGameObjects.GetCount(); ++i)
	{
		const RpgGameObject gameObject = frame.PendingDestroyGameObjects[i];
		const int index = gameObject.Index;

		RpgStringID& name = GameObjectNames[index];
		RPG_LogDebug(RpgLogTemp, "Destroy gameobject (%s)", *name.ToString());
		name = RpgStringID("__empty__");

		FGameObjectState& state = GameObjectStates[index];
		state.Flags = RpgGameObjectFlag::None;
		++state.Gen;

		// remove components
		FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];

		for (int compType = 0; compType < RPG_COMPONENT_ID_MAX_COUNT; ++compType)
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
	if (LoadingStatus.State == STATE_LOADING)
	{
		return;
	}

	for (auto it = GameObjectStates.CreateIterator(); it; ++it)
	{
		FGameObjectState& state = it.GetValue();
		state.Flags &= ~RpgGameObjectFlag::TransformUpdated;
	}
}


void RpgLevel::StartPlay() noexcept
{
	RPG_IsMainThread();

	if (LoadingStatus.State == STATE_LOADING || bHasStartedPlay)
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

	if (LoadingStatus.State == STATE_LOADING || !bHasStartedPlay)
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

	if (LoadingStatus.State == STATE_LOADING)
	{
		return;
	}

	for (int i = 0; i < AttachedScripts.GetCount(); ++i)
	{
		RpgGameObjectScript* script = AttachedScripts[i];
		RPG_Check(script);

		if (script->TickUpdateOption == RpgGameObjectScript::TICK_UPDATE_NONE || (script->TickUpdateOption == RpgGameObjectScript::TICK_UPDATE_START_PLAY && !bHasStartedPlay))
		{
			continue;
		}

		script->TickUpdate(deltaTime);
	}
}


float RpgLevel::GetLoadingProgress() const noexcept
{
	if (LoadingStatus.State != STATE_LOADING)
	{
		return 1.0f;
	}

	return static_cast<float>(LoadingStatus.LoadedGameObjectCount) / static_cast<float>(LoadingStatus.TotalGameObjectCount);
}


RpgGameObject RpgLevel::GameObject_Create(const RpgName& name, bool bIsTransient) noexcept
{
	RPG_IsMainThread();

	RPG_Check(LoadingStatus.State == STATE_LOADED);

	const int nameId = GameObjectNames.Add();
	const int stateId = GameObjectStates.Add();
	const int trxId = GameObjectTransforms.Add();
	const int compId = GameObjectComponentScripts.Add();
	RPG_Check(nameId < UINT16_MAX && nameId == stateId && stateId == trxId && trxId == compId);

	GameObjectNames[nameId] = RpgStringID(name, true);

	FGameObjectState& state = GameObjectStates[stateId];
	state.Flags = RpgGameObjectFlag::Allocated | RpgGameObjectFlag::Loaded | RpgGameObjectFlag::Visible;

	if (bIsTransient)
	{
		state.Flags |= RpgGameObjectFlag::Transient;
	}

	FGameObjectTransform& transform = GameObjectTransforms[trxId];
	//transform.LocalTransformMatrix = RpgMatrixTransform();
	//transform.WorldTransformMatrix = RpgMatrixTransform();
	//transform.InverseWorldTransformMatrix = RpgMatrixTransform();
	transform.LocalTransform = RpgTransform();
	transform.WorldTransform = RpgTransform();
	transform.Parent = RpgGameObject();
	transform.Children.Clear();
	transform.Dirty = EDirty::DIRTY_NONE;

	FGameObjectComponentScript& compScript = GameObjectComponentScripts[compId];
	RpgPlatformMemory::Set(compScript.Components, RPG_INDEX_INVALID, sizeof(int16_t) * RPG_COMPONENT_ID_MAX_COUNT);
	compScript.ScriptIndex = RPG_INDEX_INVALID;

	return RpgGameObject(this, nameId, state.Gen);
}


void RpgLevel::GameObject_Destroy(RpgGameObject gameObject) noexcept
{
	RPG_IsMainThread();

	RPG_Check(LoadingStatus.State == STATE_LOADED);

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

	RPG_LogDebug(RpgLogLevel, "Mark gameobject (%s) as pending destroy", *GameObjectNames[index].ToString());
}


void RpgLevel::GameObject_AttachToParent(RpgGameObject gameObject, RpgGameObject parent, RpgGameObjectAttachMode mode) noexcept
{
	RPG_Check(LoadingStatus.State == STATE_LOADED);
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
		//transform.LocalTransformMatrix = RpgMatrixTransform();
		//transform.WorldTransformMatrix = RpgMatrixTransform();
		//transform.InverseWorldTransformMatrix = RpgMatrixTransform();
		transform.LocalTransform = RpgTransform();
		transform.WorldTransform = GameObjectTransforms[parent.Index].WorldTransform;
	}
	else if (mode == RpgGameObjectAttachMode::KEEP_LOCAL_TRANSFORM)
	{
		//transform.WorldTransformMatrix = RpgMatrixTransform();
		//transform.InverseWorldTransformMatrix = RpgMatrixTransform();
		transform.WorldTransform = RpgTransform();
		transform.Dirty = DIRTY_WORLD;
	}
	else // mode == RpgGameObjectAttachMode::KEEP_WORLD_TRANSFORM
	{
		//transform.LocalTransformMatrix = RpgMatrixTransform();
		transform.LocalTransform = RpgTransform();
		transform.Dirty = DIRTY_LOCAL;
	}

	// add child to parent
	GameObjectTransforms[parent.Index].Children.AddValue(gameObject);
}


void RpgLevel::GameObject_Spawn(RpgGameObject gameObject, const RpgTransform& worldTransform) noexcept
{
	RPG_IsMainThread();

	RPG_Check(LoadingStatus.State == STATE_LOADED);
	RPG_Check(GameObject_IsValid(gameObject));

	const int index = gameObject.Index;
	
	FGameObjectTransform& transform = GameObjectTransforms[index];
	//transform.WorldTransformMatrix = worldTransform.ToMatrixTransform();
	//transform.InverseWorldTransformMatrix = transform.WorldTransformMatrix.GetInverse();
	transform.WorldTransform = worldTransform;
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
	RPG_Check(LoadingStatus.State == STATE_LOADED);
	RPG_Check(GameObject_IsValid(gameObject));
	RPG_Check(script);

	const int index = gameObject.Index;
	const char* scriptTypeName = script->GetTypeName();

	// check if script already attached to any gameobject
	const int checkScriptIndex = AttachedScripts.FindIndexByValue(script);
	if (checkScriptIndex != RPG_INDEX_INVALID)
	{
		RPG_Check(GameObject_IsValid(script->GameObject));
		RPG_CONSOLE_Warn(RpgLogLevel, "Script (%s) has been attached to gameobject (%s). Ignore attach script!", scriptTypeName, *GameObjectNames[script->GameObject.Index].ToString());
		return;
	}

	const int attachedScriptIndex = AttachedScripts.GetCount();
	AttachedScripts.AddValue(script);

	GameObjectComponentScripts[index].ScriptIndex = attachedScriptIndex;

	script->GameObject = gameObject;
	script->AttachedScriptIndex = attachedScriptIndex;
	script->AttachedToGameObject();

	RPG_CONSOLE_Log(RpgLogLevel, "Attached script (%s) to gameobject (%s)", scriptTypeName, *GameObjectNames[index].ToString());
}


void RpgLevel::GameObject_DetachScript(RpgGameObject gameObject, RpgGameObjectScript* script) noexcept
{
	RPG_IsMainThread();
	RPG_Check(LoadingStatus.State == STATE_LOADED);

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
			*GameObjectNames[index].ToString(),
			*GameObjectNames[script->GameObject.Index].ToString()
		);

		return;
	}

	const int attachedScriptIndex = script->AttachedScriptIndex;
	FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];
	RPG_Check(attachedScriptIndex != RPG_INDEX_INVALID && attachedScriptIndex == compScript.ScriptIndex && script == AttachedScripts[attachedScriptIndex]);

	GameObjectScript_StopPlay(attachedScriptIndex);
	GameObjectScript_Remove(attachedScriptIndex);
	compScript.ScriptIndex = RPG_INDEX_INVALID;

	RPG_CONSOLE_Log(RpgLogLevel, "Detached script (%s) from gameobject (%s)", script->GetTypeName(), *GameObjectNames[index].ToString());
}


void RpgLevel::GameObject_UpdateComponentFlags(RpgGameObject gameObject) noexcept
{
	RPG_IsMainThread();
	RPG_Check(LoadingStatus.State == STATE_LOADED);

	const int index = gameObject.Index;
	const uint32_t flags = GameObjectStates[index].Flags;
	const FGameObjectComponentScript& compScript = GameObjectComponentScripts[index];

	for (int compType = 0; compType < RPG_COMPONENT_ID_MAX_COUNT; ++compType)
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
	RPG_Check(LoadingStatus.State == STATE_LOADED);

	FGameObjectTransform& transform = GameObjectTransforms[gameObject.Index];
	if (transform.Parent.IsNull())
	{
		return;
	}

	RPG_Check(GameObject_IsValid(transform.Parent));
	FGameObjectTransform& parentTransform = GameObjectTransforms[transform.Parent.Index];
	parentTransform.Children.RemoveByValue(gameObject, false);

	transform.Parent = RpgGameObject();
	//transform.LocalTransformMatrix = transform.WorldTransformMatrix;
	transform.LocalTransform = transform.WorldTransform;
	transform.WorldTransform = RpgTransform();
	transform.Dirty = DIRTY_WORLD;

	if (!bIsDestroying)
	{
		GameObject_UpdateTransform(gameObject);
	}
}


void RpgLevel::GameObject_UpdateTransform(RpgGameObject gameObject) noexcept
{
	RPG_Check(LoadingStatus.State == STATE_LOADED);

	FGameObjectTransform& transform = GameObjectTransforms[gameObject.Index];
	if (transform.Dirty == DIRTY_NONE)
	{
		return;
	}

	/*
	const FGameObjectTransform* parentTransform = nullptr;
	if (!transform.Parent.IsNull())
	{
		RPG_Check(GameObject_IsValid(transform.Parent));
		parentTransform = &GameObjectTransforms[transform.Parent.Index];
		RPG_Check(parentTransform->Dirty == DIRTY_NONE);
	}
	*/

	const RpgTransform* parentTransform = nullptr;

	if (!transform.Parent.IsNull())
	{
		RPG_Check(GameObject_IsValid(transform.Parent));
		RPG_Check(GameObjectTransforms[transform.Parent.Index].Dirty == DIRTY_NONE);
		parentTransform = &GameObjectTransforms[transform.Parent.Index].WorldTransform;
	}

	if (transform.Dirty == DIRTY_LOCAL)
	{
		//transform.LocalTransformMatrix = parentTransform ? transform.WorldTransformMatrix * parentTransform->InverseWorldTransformMatrix : transform.WorldTransformMatrix;
		const RpgTransform& worldTransform = transform.WorldTransform;
		const RpgQuaternion parentInverseRotation = (parentTransform) ? parentTransform->Rotation.GetInverse() : RpgQuaternion();

		RpgTransform& localTransform = transform.LocalTransform;
		localTransform.Scale = (parentTransform) ? worldTransform.Scale / parentTransform->Scale : worldTransform.Scale;
		localTransform.Rotation = (parentTransform) ? parentInverseRotation * worldTransform.Rotation : worldTransform.Rotation;
		localTransform.Position = (parentTransform) ? (parentInverseRotation * (worldTransform.Position - parentTransform->Position)) / parentTransform->Scale : worldTransform.Position;
	}
	else // Dirty == DIRTY_WORLD
	{
		//const RpgMatrixTransform& localTransformMatrix = transform.LocalTransformMatrix;
		//transform.WorldTransformMatrix = parentTransform ? localTransformMatrix * parentTransform->WorldTransformMatrix : localTransformMatrix;
		//transform.InverseWorldTransformMatrix = transform.WorldTransformMatrix.GetInverse();
		const RpgTransform& localTransform = transform.LocalTransform;

		RpgTransform& worldTransform = transform.WorldTransform;
		worldTransform.Scale = (parentTransform) ? parentTransform->Scale * localTransform.Scale : localTransform.Scale;
		worldTransform.Rotation = (parentTransform) ? parentTransform->Rotation * localTransform.Rotation : localTransform.Rotation;
		worldTransform.Position = (parentTransform) ? parentTransform->Position + (parentTransform->Rotation * (parentTransform->Scale * localTransform.Position)) : localTransform.Position;
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
	RPG_Check(LoadingStatus.State == STATE_LOADED);

	const FGameObjectComponentScript& compScript = GameObjectComponentScripts[id];

	for (int compType = 0; compType < RPG_COMPONENT_ID_MAX_COUNT; ++compType)
	{
		const int compId = compScript.Components[compType];
		if (compId == RPG_INDEX_INVALID)
		{
			continue;
		}

		ComponentStorages[compType]->GetExternalAssetReferences(compId, out_AssetRefs);
	}
}
