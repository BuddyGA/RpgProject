#include "RpgWorld.h"


RPG_LOG_DEFINE_CATEGORY(RpgLogWorld, VERBOSITY_DEBUG)



RpgWorld::RpgWorld(const RpgName& name) noexcept
{
    RPG_LogDebug(RpgLogWorld, "Create world (%s)", *name);

    Name = name;
    bHasStartedPlay = false;
    FrameIndex = 0;
}


RpgWorld::~RpgWorld() noexcept
{
    RPG_LogDebug(RpgLogWorld, "Destroy world (%s)", *Name);

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
