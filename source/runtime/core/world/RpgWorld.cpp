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
    // create level_main
    LevelLoadeds.AddValue(RpgPointer::MakeUnique<RpgLevel>(RpgName::Format("%s/level_main", *Name)));
    RegisterComponents(LevelLoadeds[0].Get());
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


void RpgWorld::SaveLevel(const RpgName& name) noexcept
{
    /*
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

    level_main_begin
    {
        level_object_count,
        level_objects
        {
            object_0
            {
                name,
                transform,
                component_types,
                component_datas
                {
                    component_type_0,
                    component_type_1,
                    ...
                }
            },
            object_1
            {
                name,
                transform,
                component_types,
                component_datas
                {
                    component_type_0,
                    null,
                    component_type_2,
                    ...
                }
            },
            ...
        }
    }
    level_main_end

    level_streaming_ref_count,
    level_streaming_ref_begin
    {
        level_asset_ref_0,
        level_asset_ref_1,
        ...
    }
    level_streaming_ref_end

    eof
    */

    RpgLevel* levelMain = LevelLoadeds[0].Get();


    // asset references
    RpgAssetReferences assetReferences;
    RpgBinaryStreamWriter assetRefWriter;
    {
        levelMain->GetExternalAssetReferences(assetReferences);

        assetRefWriter.Write(RPG_ASSET_FILE_ASSET_EXT);
        assetRefWriter.Write(assetReferences);
        assetRefWriter.Write(RPG_ASSET_FILE_ASSET_EXT);
    }
    RPG_Check(assetReferences.GetCount() < UINT16_MAX);


    // level main data
    RpgBinaryStreamWriter levelWriter;
    {
        levelWriter.Write(RPG_ASSET_FILE_ASSET_DATA);
        levelMain->AssetStreamWrite(levelWriter);
        levelWriter.Write(RPG_ASSET_FILE_ASSET_DATA);
    }


    const RpgFilePath filePath = RpgString::Format("%sgame/%s.rpgm", *RpgFileSystem::GetAssetDirPath(), *name);
    HANDLE fileHandle = RpgPlatformFile::FileOpen(*filePath, RpgPlatformFile::OPEN_MODE_WRITE_OVERWRITE);
    {
        RPG_ValidateV(fileHandle && fileHandle != INVALID_HANDLE_VALUE, "Open file failed! (FilePath: %s)", *filePath);

        // header
        RpgAssetFileHeader fileHeader{};
        fileHeader.Magix = RPG_ASSET_FILE_HEADER;
        fileHeader.Type = static_cast<uint16_t>(RpgAssetFileType::LEVEL);
        fileHeader.Version = 1;
        fileHeader.AssetReferenceCount = static_cast<uint16_t>(assetReferences.GetCount());
        fileHeader.AssetReferenceSizeBytes = static_cast<uint32_t>(assetRefWriter.GetByteArraySize());
        fileHeader.AssetDataSizeBytes = static_cast<uint32_t>(levelWriter.GetByteArraySize());
        fileHeader.AssetClassName = levelMain->GetAssetClassName();
        RpgPlatformFile::FileWrite(fileHandle, &fileHeader, sizeof(RpgAssetFileHeader));

        // asset references
        RpgPlatformFile::FileWrite(fileHandle, assetRefWriter.GetByteArrayData(), assetRefWriter.GetByteArraySize());

        // level main data
        RpgPlatformFile::FileWrite(fileHandle, levelWriter.GetByteArrayData(), levelWriter.GetByteArraySize());

        // TODO: Level streaming refs

        // end-of-file
        const uint32_t eof = RPG_ASSET_FILE_EOF;
        RpgPlatformFile::FileWrite(fileHandle, &eof, sizeof(uint32_t));
    }
    RpgPlatformFile::FileClose(fileHandle);

    RPG_CONSOLE_Log(RpgLogWorld, "Saved level (File: %s)", *filePath);
}


void RpgWorld::LoadLevelAsync(const RpgString& path) noexcept
{

}


RpgGameObject RpgWorld::CreateGameObject(const RpgName& name, RpgLevel* opt_Level, bool opt_bIsTransient) noexcept
{
    RpgLevel* level = opt_Level ? opt_Level : LevelLoadeds[0].Get();
    return level->GameObject_Create(name, opt_bIsTransient);
}
