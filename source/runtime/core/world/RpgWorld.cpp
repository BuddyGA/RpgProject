#include "RpgWorld.h"
#include "../RpgConsoleSystem.h"
#include "../RpgThreadPool.h"
#include "../asset/RpgAssetSystem.h"



RPG_LOG_DEFINE_CATEGORY(RpgLogWorld, VERBOSITY_DEBUG)



class RpgWorldTask_LoadLevel : public RpgThreadTask
{
public:
    RpgString AssetPath;


public:
    RpgWorldTask_LoadLevel() noexcept
    {
    }


    virtual void Reset() noexcept override
    {
        RpgThreadTask::Reset();

        AssetPath = RpgString();
    }


    virtual void Execute() noexcept override
    {
        RPG_LogDebug(RpgLogWorld, "[ThreadId-%u] Execute task load level (%s)", GetCurrentThreadId(), *AssetPath);


    }


    virtual const char* GetTaskName() const noexcept override
    {
        return "RpgWorldTask_LoadLevel";
    }

};



RpgWorld::RpgWorld(const RpgName& in_Name) noexcept
{
    RPG_CONSOLE_Log(RpgLogWorld, "Create world (%s)", *in_Name);

    Name = RpgStringID(in_Name, true);
    bHasStartedPlay = false;
}


RpgWorld::~RpgWorld() noexcept
{
    RPG_CONSOLE_Log(RpgLogWorld, "Destroy world (%s)", *Name.ToString());

    ClearLevels();
}


void RpgWorld::Initialize() noexcept
{
    // create level_main
    Levels.AddValue(RpgPointer::MakeShared<RpgLevel>("level_main"));
    RegisterComponents(Levels[0].Get());
    Levels[0]->LoadingStatus.State = RpgLevel::STATE_LOADED;
}


void RpgWorld::BeginFrame(int frameIndex) noexcept
{
    for (int i = 0; i < Levels.GetCount(); ++i)
    {
        Levels[i]->BeginFrame(frameIndex);
    }
}


void RpgWorld::EndFrame(int frameIndex) noexcept
{
    for (int i = 0; i < Levels.GetCount(); ++i)
    {
        Levels[i]->EndFrame(frameIndex);
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

    for (int i = 0; i < Levels.GetCount(); ++i)
    {
        Levels[i]->StartPlay();
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

    for (int i = 0; i < Levels.GetCount(); ++i)
    {
        Levels[i]->StopPlay();
    }

    bHasStartedPlay = false;
}


void RpgWorld::DispatchTickUpdate(float deltaTime) noexcept
{
    for (int i = 0; i < Subsystems.GetCount(); ++i)
    {
        Subsystems[i]->TickUpdate(deltaTime);
    }

    for (int i = 0; i < Levels.GetCount(); ++i)
    {
        Levels[i]->TickUpdate(deltaTime);
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


void RpgWorld::ClearLevels() noexcept
{
    Levels.Clear();
}


void RpgWorld::SaveLevel(const RpgName& name) noexcept
{
    RPG_IsMainThread();

    RPG_CONSOLE_Log(RpgLogWorld, "Save level (%s)", *name);

    g_AssetSystem->SaveAsset(Levels[0], "game");
}


RpgLevel* RpgWorld::LoadLevelAsync(const RpgStringID& levelAssetPath) noexcept
{
    RPG_IsMainThread();

    RPG_CONSOLE_Log(RpgLogWorld, "Loading level (%s)", *levelAssetPath.ToString());

    RpgSharedLevel level = g_AssetSystem->LoadAssetAsync<RpgLevel>(levelAssetPath);
    if (!level)
    {
        RPG_CONSOLE_Error(RpgLogWorld, "Fail to load level (%s)!", *levelAssetPath.ToString());
        return nullptr;
    }

    const int index = Levels.FindIndexByValue(level);
    if (index == RPG_INDEX_INVALID)
    {
        Levels.AddValue(level);
    }

    return level.Get();
}


RpgGameObject RpgWorld::CreateGameObject(const RpgName& name, RpgLevel* opt_Level, bool opt_bIsTransient) noexcept
{
    RpgLevel* level = opt_Level ? opt_Level : Levels[0].Get();
    RPG_Check(level->IsAssetLoaded());

    return level->GameObject_Create(name, opt_bIsTransient);
}


int RpgWorld::GetGameObjectCount() const noexcept
{
    int count = 0;

    for (const RpgSharedLevel& level : Levels)
    {
        count += level->GameObject_GetCount();
    }

    return count;
}
