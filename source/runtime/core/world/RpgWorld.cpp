#include "RpgWorld.h"
#include "../RpgConsoleSystem.h"



RPG_LOG_DEFINE_CATEGORY(RpgLogWorld, VERBOSITY_DEBUG)



RpgWorld::RpgWorld(const RpgName& in_Name) noexcept
{
    RPG_CONSOLE_Log(RpgLogWorld, "Create world (%s)", *in_Name);

    Name = in_Name;
    bHasStartedPlay = false;
}


void RpgWorld::Initialize() noexcept
{
    CreateLevel(RpgName::Format("%s/level_main", *Name));
}


void RpgWorld::BeginFrame(int frameIndex) noexcept
{
    for (int i = 0; i < LevelLoadeds.GetCount(); ++i)
    {
        LevelLoadeds[i]->BeginFrame(frameIndex);
    }
}


void RpgWorld::EndFrame(int frameIndex) noexcept
{
    for (int i = 0; i < LevelLoadeds.GetCount(); ++i)
    {
        LevelLoadeds[i]->EndFrame(frameIndex);
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

    for (int i = 0; i < LevelLoadeds.GetCount(); ++i)
    {
        LevelLoadeds[i]->StartPlay();
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

    for (int i = 0; i < LevelLoadeds.GetCount(); ++i)
    {
        LevelLoadeds[i]->StopPlay();
    }

    bHasStartedPlay = false;
}


void RpgWorld::DispatchTickUpdate(float deltaTime) noexcept
{
    for (int i = 0; i < Subsystems.GetCount(); ++i)
    {
        Subsystems[i]->TickUpdate(deltaTime);
    }

    for (int i = 0; i < LevelLoadeds.GetCount(); ++i)
    {
        LevelLoadeds[i]->TickUpdate(deltaTime);
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



void RpgWorld::CreateLevel(const RpgName& name) noexcept
{
    const int index = LevelLoadeds.GetCount();
    LevelLoadeds.AddValue(RpgPointer::MakeUnique<RpgLevel>(name));
    RegisterComponents(LevelLoadeds[index].Get());
}


void RpgWorld::LoadLevelAsync(const RpgString& path) noexcept
{

}


RpgGameObject RpgWorld::CreateGameObject(const RpgName& name, RpgLevel* opt_Level, bool opt_bIsTransient) noexcept
{
    RpgLevel* level = opt_Level ? opt_Level : LevelLoadeds[0].Get();
    return level->GameObject_Create(name, opt_bIsTransient);
}
