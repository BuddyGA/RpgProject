#include "RpgWorld.h"
#include "../RpgConsoleSystem.h"


RPG_LOG_DEFINE_CATEGORY(RpgLogWorld, VERBOSITY_DEBUG)



RpgWorld::RpgWorld(const RpgName& name) noexcept
{
    RPG_LogDebug(RpgLogWorld, "Create world (%s)", *name);

    Name = name;
    bHasStartedPlay = false;
    FrameIndex = 0;
    ComponentStorages.Resize(RPG_COMPONENT_TYPE_MAX_COUNT);

    Level_Create(RpgName::Format("%s/level_main", *name));
}


RpgWorld::~RpgWorld() noexcept
{
    RPG_LogDebug(RpgLogWorld, "Destroy world (%s)", *Name);

    for (int i = 0; i < Levels.GetCount(); ++i)
    {
        delete Levels[i];
        Levels[i] = nullptr;
    }

    for (int i = 0; i < ComponentStorages.GetCount(); ++i)
    {
        if (ComponentStorages[i])
        {
            delete ComponentStorages[i];
            ComponentStorages[i] = nullptr;
        }
    }

    for (int i = 0; i < Subsystems.GetCount(); ++i)
    {
        RPG_LogDebug(RpgLogWorld, "Destroy subsystem (%s)", *Subsystems[i]->Name);
        delete Subsystems[i];
    }
}


void RpgWorld::StreamWrite(RpgStreamWriter& writer) const noexcept
{
    writer.Write(Name);

    // counting non transient and alive object
    int objectCount = 0;
    for (auto it = GameObjectInfos.CreateConstIterator(); it; ++it)
    {
        const FGameObjectInfo& info = it.GetValue();

        if (info.Flags & (FLAG_Transient | FLAG_PendingDestroy))
        {
            continue;
        }

        ++objectCount;
    }

    writer.Write(objectCount);

    const int levelCount = Levels.GetCount();
    writer.Write(levelCount);

    for (int i = 0; i < levelCount; ++i)
    {
        Levels[i]->StreamWrite(writer);
    }
}


void RpgWorld::StreamRead(RpgStreamReader& reader) noexcept
{
    ClearFast();

    reader.Read(Name);

    int objectCount = 0;
    reader.Read(objectCount);

    GameObjectNames.Reserve(objectCount);
    GameObjectInfos.Reserve(objectCount);
    GameObjectTransforms.Reserve(objectCount);
    
    int levelCount = 0;
    reader.Read(levelCount);

    for (int i = 0; i < levelCount; ++i)
    {
        Level_Create("")->StreamRead(reader);
    }
}


void RpgWorld::ClearFast() noexcept
{
    for (uint16_t compType = 0; compType < RPG_COMPONENT_TYPE_MAX_COUNT; ++compType)
    {
        if (ComponentStorages[compType])
        {
            ComponentStorages[compType]->Clear();
        }
    }

    for (int i = 0; i < Levels.GetCount(); ++i)
    {
        delete Levels[i];
        Levels[i] = nullptr;
    }

    Levels.Clear();

    GameObjectNames.Clear();
    GameObjectInfos.Clear();
    GameObjectTransforms.Clear();
}


void RpgWorld::BeginFrame(int frameIndex) noexcept
{
    FrameIndex = frameIndex;
    FFrameData& frame = FrameDatas[FrameIndex];

    for (int i = 0; i < frame.PendingDestroyObjects.GetCount(); ++i)
    {
        const int index = frame.PendingDestroyObjects[i];
        FGameObjectInfo& info = GameObjectInfos[index];

        for (int c = 0; c < RPG_COMPONENT_TYPE_MAX_COUNT; ++c)
        {
            if (info.ComponentIndices[c] != RPG_COMPONENT_ID_INVALID)
            {
                ComponentStorages[c]->Remove(info.ComponentIndices[c]);
                info.ComponentIndices[c] = RPG_COMPONENT_ID_INVALID;
            }
        }

        GameObjectNames.RemoveAt(index);
        GameObjectInfos.RemoveAt(index);
        GameObjectTransforms.RemoveAt(index);
    }
}


void RpgWorld::EndFrame(int frameIndex) noexcept
{
    for (auto it = GameObjectInfos.CreateIterator(); it; ++it)
    {
        FGameObjectInfo& gameObjectInfo = it.GetValue();
        gameObjectInfo.Flags &= ~FLAG_TransformUpdated;
    }
}


void RpgWorld::DispatchStartPlay() noexcept
{
    if (bHasStartedPlay)
    {
        return;
    }

    for (int i = 0; i < Subsystems.GetCount(); ++i)
    {
        Subsystems[i]->StartPlay();
    }

    for (int i = 0; i < GameObjectScripts.GetCount(); ++i)
    {
        GameObjectScript_StartPlay(i);
    }

    bHasStartedPlay = true;
}


void RpgWorld::DispatchStopPlay() noexcept
{
    if (!bHasStartedPlay)
    {
        return;
    }

    for (int i = 0; i < Subsystems.GetCount(); ++i)
    {
        Subsystems[i]->StopPlay();
    }

    for (int i = 0; i < GameObjectScripts.GetCount(); ++i)
    {
        GameObjectScript_StopPlay(i);
    }

    bHasStartedPlay = false;
}


void RpgWorld::DispatchTickUpdate(float deltaTime) noexcept
{
    for (int i = 0; i < Subsystems.GetCount(); ++i)
    {
        Subsystems[i]->TickUpdate(deltaTime);
    }

    for (int i = 0; i < GameObjectScripts.GetCount(); ++i)
    {
        GameObjectScripts[i]->TickUpdate(deltaTime);
    }
}


void RpgWorld::DispatchPostTickUpdate() noexcept
{
    for (int i = 0; i < Subsystems.GetCount(); ++i)
    {
        Subsystems[i]->PostTickUpdate();
    }
}


void RpgWorld::DispatchRender(int frameIndex, RpgRenderer* renderer) noexcept
{
    for (int i = 0; i < Subsystems.GetCount(); ++i)
    {
        Subsystems[i]->Render(frameIndex, renderer);
    }
}



// --------------------------------------------------------------------------------------------------------------------------------------------- //
// 	Level interface
// --------------------------------------------------------------------------------------------------------------------------------------------- //
RpgLevel* RpgWorld::Level_Create(const RpgName& name) noexcept
{
    for (int i = 0; i < Levels.GetCount(); ++i)
    {
        if (Levels[i]->GetName() == name)
        {
            RPG_LogWarn(RpgLogWorld, "Ignore create level. Level with name (%s) already exists!", *name);
            return Levels[i];
        }
    }

    RpgLevel* newLevel = new RpgLevel(name);
    newLevel->World = this;
    
    Levels.AddValue(newLevel);

    return newLevel;
}




// --------------------------------------------------------------------------------------------------------------------------------------------- //
// 	GameObject interface
// --------------------------------------------------------------------------------------------------------------------------------------------- //
RpgGameObjectID RpgWorld::GameObject_Create(const RpgName& name, const RpgTransform& worldTransform) noexcept
{
    RPG_IsMainThread();

    RPG_Assert(!name.IsEmpty());
    RPG_Check(GameObjectNames.GetCount() < RPG_WORLD_MAX_GAMEOBJECT);

    RPG_LogDebug(RpgLogWorld, "Create game object (%s)", *name);

    const int nameId = GameObjectNames.Add();
    const int infoId = GameObjectInfos.Add();
    const int transformId = GameObjectTransforms.Add();
    RPG_Check(nameId == infoId && infoId == transformId);

    GameObjectNames[nameId] = name;
    
    FGameObjectInfo& info = GameObjectInfos[infoId];
    RpgPlatformMemory::MemSet(info.ComponentIndices, RPG_COMPONENT_ID_INVALID, sizeof(uint16_t) * RPG_COMPONENT_TYPE_MAX_COUNT);

    ++info.Gen;
    RPG_Check(info.Gen < UINT16_MAX);
    info.Flags = FLAG_Allocated | FLAG_TransformUpdated;

    RpgPlatformMemory::MemSet(info.ScriptIndices, RPG_INDEX_INVALID, sizeof(int16_t) * RPG_GAMEOBJECT_MAX_SCRIPT);

    FGameObjectTransform& transform = GameObjectTransforms[transformId];
    transform.LocalMatrix = RpgMatrixTransform();
    transform.WorldMatrix = worldTransform.ToMatrixTransform();
    transform.InverseWorldMatrix = transform.WorldMatrix.GetInverse();

    return RpgGameObjectID(this, nameId, info.Gen);
}


void RpgWorld::GameObject_Destroy(RpgGameObjectID& gameObject) noexcept
{
    RPG_IsMainThread();

    if (GameObject_IsValid(gameObject))
    {
        FGameObjectInfo& info = GameObjectInfos[gameObject.Index];

        // Remove from level
        info.Level->RemoveGameObject(gameObject);

        // FLAG_Spawned off, FLAG_PendingDestroy ON
        info.Flags = (info.Flags & ~FLAG_Spawned) | FLAG_PendingDestroy;

        // remove scripts
        for (int i = 0; i < RPG_GAMEOBJECT_MAX_SCRIPT; ++i)
        {
            const int scriptIndex = info.ScriptIndices[i];

            if (scriptIndex != RPG_INDEX_INVALID)
            {
                GameObjectScript_StopPlay(scriptIndex);
                GameObjectScript_Remove(scriptIndex);
                info.ScriptIndices[i] = RPG_INDEX_INVALID;
            }
        }

        FrameDatas[FrameIndex].PendingDestroyObjects.AddValue(gameObject.Index);

        RPG_LogDebug(RpgLogWorld, "Mark game object (%s) as pending destroy", *GameObjectNames[gameObject.Index]);
    }

    gameObject = RpgGameObjectID();
}


void RpgWorld::GameObject_StreamWrite(RpgGameObjectID gameObject, RpgStreamWriter& writer) const noexcept
{
    RPG_Check(GameObject_IsValid(gameObject));

    const int id = gameObject.Index;
    const FGameObjectInfo& info = GameObjectInfos[id];
    RPG_Check(!(info.Flags & (FLAG_Transient | FLAG_PendingDestroy)));

    writer.Write(GameObjectNames[id]);
    writer.Write(GameObjectTransforms[id]);

    // components
    writer.Write(RPG_COMPONENT_TYPE_MAX_COUNT);
    writer.WriteData(info.ComponentIndices, sizeof(uint16_t) * RPG_COMPONENT_TYPE_MAX_COUNT);

    for (uint16_t compType = 0; compType < RPG_COMPONENT_TYPE_MAX_COUNT; ++compType)
    {
        const int compId = info.ComponentIndices[compType];
        if (compId == RPG_COMPONENT_ID_INVALID)
        {
            continue;
        }

        ComponentStorages[compType]->StreamWrite(writer, compId);
    }
}


void RpgWorld::GameObject_StreamRead(RpgGameObjectID gameObject, RpgStreamReader& reader) noexcept
{
    RPG_Check(GameObject_IsValid(gameObject));

    const int id = gameObject.Index;
    FGameObjectInfo& info = GameObjectInfos[id];

    reader.Read(GameObjectNames[id]);
    reader.Read(GameObjectTransforms[id]);

    // components
    int compTypeCount = 0;
    reader.Read(compTypeCount);
    reader.ReadData(info.ComponentIndices, sizeof(uint16_t) * compTypeCount);

    for (uint16_t compType = 0; compType < compTypeCount; ++compType)
    {
        if (info.ComponentIndices[compType] == RPG_COMPONENT_ID_INVALID)
        {
            continue;
        }

        const int compId = ComponentStorages[compType]->Add();
        ComponentStorages[compType]->StreamRead(reader, compId, gameObject);
    }
}


void RpgWorld::GameObject_AttachScript(RpgGameObjectID gameObject, RpgGameObjectScript* script) noexcept
{
    RPG_Check(GameObject_IsValid(gameObject));
    RPG_Check(script);

    const char* scriptTypeName = script->GetTypeName();

    // check if script already attached to any gameobject
    const int checkIndex = GameObjectScripts.FindIndexByValue(script);
    if (checkIndex != RPG_INDEX_INVALID)
    {
        RPG_CONSOLE_Warn(RpgLogWorld, "Script (%s) has been attached to gameobject (%s). Ignore attach script!", scriptTypeName, *GameObjectNames[script->GameObject.Index]);
        return;
    }


    FGameObjectInfo& info = GameObjectInfos[gameObject.Index];

    int objectScriptIndex = RPG_INDEX_INVALID;

    for (int i = 0; i < RPG_GAMEOBJECT_MAX_SCRIPT; ++i)
    {
        const int checkWorldScriptIndex = info.ScriptIndices[i];

        if (checkWorldScriptIndex != RPG_INDEX_INVALID)
        {
            if (GameObjectScripts[checkWorldScriptIndex]->GetTypeName() == scriptTypeName)
            {
                RPG_CONSOLE_Warn(RpgLogWorld, "Script (%s) has been attached to gameobject (%s). Ignore attach script!", scriptTypeName, *GameObjectNames[gameObject.Index]);
            }
        }
        else
        {
            objectScriptIndex = i;
        }
    }

    RPG_CheckV(objectScriptIndex != RPG_INDEX_INVALID, "Cannot add script into game object (%s). Exceeds maximum limit (%i) of scripts per game object!", scriptTypeName, RPG_GAMEOBJECT_MAX_SCRIPT);

    const int worldScriptIndex = GameObjectScripts.GetCount();
    info.ScriptIndices[objectScriptIndex] = worldScriptIndex;
    GameObjectScripts.AddValue(script);

    script->World = this;
    script->GameObject = gameObject;
    script->CachedWorldScriptIndex = worldScriptIndex;
    script->CachedObjectScriptIndex = objectScriptIndex;
    script->AttachedToGameObject();

    RPG_CONSOLE_Log(RpgLogWorld, "Attached script (%s) to game object (%s)", scriptTypeName, *GameObjectNames[gameObject.Index]);
}


void RpgWorld::GameObject_DetachScript(RpgGameObjectID gameObject, RpgGameObjectScript* script) noexcept
{
    if (script == nullptr)
    {
        return;
    }

    RPG_Check(GameObject_IsValid(gameObject));

    const char* scriptTypeName = script->GetTypeName();

    const RpgGameObjectID scriptGameObject = script->GameObject;
    if (gameObject != scriptGameObject)
    {
        RPG_Check(GameObject_IsValid(scriptGameObject));

        RPG_CONSOLE_Warn(RpgLogWorld, "Cannot detach script (%s) from gameobject (%s) because script is attached to gameobject (%s)", 
            scriptTypeName, 
            *GameObjectNames[gameObject.Index],
            *GameObjectNames[scriptGameObject.Index]
        );

        return;
    }

    const int worldScriptIndex = script->CachedWorldScriptIndex;
    const int objectScriptIndex = script->CachedObjectScriptIndex;
    FGameObjectInfo& info = GameObjectInfos[gameObject.Index];

    RPG_Check(script && script->GameObject == gameObject);
    RPG_Check(worldScriptIndex != RPG_INDEX_INVALID && script == GameObjectScripts[worldScriptIndex]);
    RPG_Check(objectScriptIndex != RPG_INDEX_INVALID && worldScriptIndex == info.ScriptIndices[objectScriptIndex]);

    GameObjectScript_StopPlay(worldScriptIndex);
    GameObjectScript_Remove(worldScriptIndex);
    info.ScriptIndices[objectScriptIndex] = RPG_INDEX_INVALID;

    RPG_CONSOLE_Log(RpgLogWorld, "Detached script (%s) from game object (%s)", script->GetTypeName(), *GameObjectNames[gameObject.Index]);
}


void RpgWorld::GameObject_Spawn(RpgGameObjectID gameObject, RpgLevel* opt_Level) noexcept
{
    RPG_Check(GameObject_IsValid(gameObject));

    FGameObjectInfo& info = GameObjectInfos[gameObject.Index];
    info.Level = opt_Level == nullptr ? Levels[0] : opt_Level;
    RPG_Check(info.Level);
    info.Level->AddGameObject(gameObject);
    info.Flags |= FLAG_Spawned;

    for (int i = 0; i < RPG_GAMEOBJECT_MAX_SCRIPT; ++i)
    {
        const int scriptIndex = info.ScriptIndices[i];

        if (scriptIndex != RPG_INDEX_INVALID)
        {
            GameObjectScript_StartPlay(scriptIndex);
        }
    }
}
