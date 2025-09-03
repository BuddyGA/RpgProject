#include "RpgWorld.h"


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

    const int levelCount = Levels.GetCount();
    writer.Write(levelCount);

    const int objectCount = GameObjectNames.GetCount();

    for (int i = 0; i < ComponentStorages.GetCount(); ++i)
    {
        ComponentStorages[i]->StreamWrite(writer);
    }

    for (int i = 0; i < levelCount; ++i)
    {
        Levels[i]->StreamWrite(writer);
    }
}


void RpgWorld::StreamRead(RpgStreamReader& reader) noexcept
{
    reader.Read(Name);

    int levelCount = 0;
    reader.Read(levelCount);

    int objectCount = 0;
    reader.Read(objectCount);

    GameObjectNames.Clear(true);
    GameObjectNames.Reserve(objectCount);

    GameObjectInfos.Clear(true);
    GameObjectInfos.Reserve(objectCount);

    GameObjectTransforms.Clear(true);
    GameObjectTransforms.Reserve(objectCount);
    

    for (int i = 0; i < ComponentStorages.GetCount(); ++i)
    {
        ComponentStorages[i]->StreamRead(reader);
    }
    

    Levels.Clear(true);

    for (int i = 0; i < levelCount; ++i)
    {
        Level_Create("")->StreamRead(reader);
    }
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

        info.Flags = 0;
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
        RpgGameObjectScript* script = GameObjectScripts[i];
        RPG_Check(script);

        if (!script->bStartedPlay)
        {
            script->StartPlay();
            script->bStartedPlay = true;
        }
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
        RpgGameObjectScript* script = GameObjectScripts[i];
        RPG_Check(script);

        if (script->bStartedPlay)
        {
            script->StopPlay();
            script->bStartedPlay = false;
        }
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
        RpgGameObjectScript* script = GameObjectScripts[i];
        RPG_Check(script);

        script->TickUpdate(deltaTime);
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
    info.Flags = FLAG_Allocated | FLAG_TransformUpdated;
    RPG_Check(info.Gen < UINT16_MAX);

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
        info.Flags |= FLAG_PendingDestroy;

        // remove scripts
        for (int i = 0; i < RPG_GAMEOBJECT_MAX_SCRIPT; ++i)
        {
            const int scriptIndex = info.ScriptIndices[i];
            if (scriptIndex != RPG_INDEX_INVALID)
            {
                GameObject_RemoveScriptAtIndex(scriptIndex);
                info.ScriptIndices[i] = RPG_INDEX_INVALID;
            }
        }

        FrameDatas[FrameIndex].PendingDestroyObjects.AddValue(gameObject.Index);

        RPG_LogDebug(RpgLogWorld, "Mark game object (%s) as pending destroy", *GameObjectNames[gameObject.Index]);
    }

    gameObject = RpgGameObjectID();
}


void RpgWorld::GameObject_Spawn(RpgGameObjectID gameObject, RpgLevel* opt_Level) noexcept
{
    RPG_Check(GameObject_IsValid(gameObject));

    RpgLevel* level = opt_Level == nullptr ? Levels[0] : opt_Level;
    RPG_Check(level);
    level->AddGameObject(gameObject);

    FGameObjectInfo& info = GameObjectInfos[gameObject.Index];
    info.Flags |= FLAG_Spawned;
}


void RpgWorld::GameObject_StreamWrite(RpgGameObjectID gameObject, RpgStreamWriter& writer) const noexcept
{
    RPG_Check(GameObject_IsValid(gameObject));

    const int id = gameObject.Index;
    writer.Write(GameObjectNames[id]);
    writer.Write(GameObjectInfos[id]);
    writer.Write(GameObjectTransforms[id]);
}


void RpgWorld::GameObject_StreamRead(RpgGameObjectID gameObject, RpgStreamReader& reader) noexcept
{
    RPG_Check(GameObject_IsValid(gameObject));

    const int id = gameObject.Index;
    reader.Read(GameObjectNames[id]);
    reader.Read(GameObjectInfos[id]);
    reader.Read(GameObjectTransforms[id]);
}
