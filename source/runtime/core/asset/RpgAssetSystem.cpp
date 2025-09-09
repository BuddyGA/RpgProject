#include "RpgAssetSystem.h"
#include "../RpgThreadPool.h"



class RpgAssetTask_Loader : public RpgThreadTask
{
public:
	RpgAssetObject* Asset;


public:
	RpgAssetTask_Loader() noexcept
	{
		Asset = nullptr;
	}


	virtual void Reset() noexcept override
	{
		RpgThreadTask::Reset();

		Asset = nullptr;
	}


	virtual void Execute() noexcept override
	{
		RPG_Check(Asset);
		
		RpgAssetStream::Read(Asset);
	}


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgAssetTask_Loader";
	}

};



RpgAssetSystem* g_AssetSystem = nullptr;



RpgAssetSystem::RpgAssetSystem() noexcept
{
	RPG_CONSOLE_Log(RpgLogAsset, "Initialize asset system");

#ifndef RPG_BUILD_SHIPPING
	RpgFilePath assetDirPath = RpgFileSystem::GetAssetDirPath();
	RpgFileSystem::CreateFolder(assetDirPath);
	RpgFileSystem::CreateFolder(assetDirPath + "engine/");
	RpgFileSystem::CreateFolder(assetDirPath + "game/");
#endif // !RPG_BUILD_SHIPPING

}


void RpgAssetSystem::Update() noexcept
{
	// if this is the last reference (SharedRefCount == 1), that means no other referencing it, so remove it
	for (int i = 0; i < LoadedAssetTable.GetCount();)
	{
		const RpgSharedAsset& check = LoadedAssetTable.GetValueByIndex(i);

		if (check.GetRefCount() == 1)
		{
			LoadedAssetTable.RemoveAt(i);
		}
		else
		{
			++i;
		}
	}

	// update loading assets
	for (int i = 0; i < LoadingAssetTable.GetCount(); )
	{
		RpgSharedAsset& asset = LoadingAssetTable.GetValueByIndex(i);
		RPG_Check(asset);

		if (asset->IsAssetLoaded())
		{
			LoadedAssetTable.AddValue(LoadingAssetTable.GetKeyByIndex(i), asset);
			LoadingAssetTable.RemoveAt(i);
		}
		else
		{
			++i;
		}
	}
}


bool RpgAssetSystem::IsValidAssetFile(const RpgFilePath& filePath, RpgAssetInfo* optOut_AssetInfo) noexcept
{
	if (!filePath.IsFilePath())
	{
		return false;
	}

	// check extension
	const RpgName fileExt = filePath.GetFileExtension();
	if (fileExt != RPG_ASSET_FILE_EXT)
	{
		return false;
	}

	// read file
	RpgArray<uint8_t> fileData;
	if (!RpgFileSystem::ReadFromFile(filePath.ToString(), fileData))
	{
		return false;
	}

	RpgBinaryStreamReader reader(fileData);

	// validate header
	RpgAssetFileHeader header;
	reader.Read(header);

	if (header.Magix != RPG_ASSET_FILE_HEADER || header.AssetClassName.IsEmpty())
	{
		return false;
	}

	if (optOut_AssetInfo)
	{
		optOut_AssetInfo->ClassName = header.AssetClassName;
		optOut_AssetInfo->Path = GetAssetPathFromFile(filePath);
		optOut_AssetInfo->Type = static_cast<RpgAssetFileType>(header.Type);
	}

	return true;
}


void RpgAssetSystem::ScanAssetFiles() noexcept
{
	RPG_CONSOLE_Log(RpgLogAsset, "Scanning asset files...");

	RpgArray<RpgFilePath> filePaths;
	RpgFileSystem::IterateFiles(filePaths, RpgFileSystem::GetAssetDirPath(), true, ".rpga");

	for (int i = 0; i < filePaths.GetCount(); ++i)
	{
		RegisterAssetFile(filePaths[i]);
	}
}


void RpgAssetSystem::SaveAsset(RpgSharedAsset asset, const char* dstFolder) noexcept
{
	RPG_IsMainThread();

	if (!asset.IsValid())
	{
		RPG_CONSOLE_Warn(RpgLogAsset, "Fail to save asset. <asset> is NULL!");
		return;
	}

	const RpgFilePath filePath = RpgAssetStream::Write(asset.Get(), dstFolder);

	RPG_CONSOLE_Log(RpgLogAsset, "Saved asset file (%s)", *filePath);

	// Register asset
	RpgString assetPath;
	RegisterAssetFile(filePath, &assetPath);

	// set asset path
	asset->Path = assetPath;

	// Add to loaded asset
	LoadedAssetTable.AddValue(assetPath, asset);
}


RpgSharedAsset RpgAssetSystem::LoadAsset(const RpgString& assetPath) noexcept
{
	RPG_IsMainThread();

	if (assetPath.IsEmpty())
	{
		RPG_CONSOLE_Warn(RpgLogAsset, "Fail to load asset. Invalid asset path (%s)!", *assetPath);
		return RpgSharedAsset();
	}

	// check if loaded
	RpgSharedAsset asset = FindLoadedAsset(assetPath);
	if (asset)
	{
		return asset;
	}

	// check if exists in registry
	RpgAssetInfo assetInfo;
	if (!DoesAssetExists(assetPath, &assetInfo))
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to load asset (%s). Asset not found in registry!", *assetPath);
		return RpgSharedAsset();
	}

	// create asset
	CreateAsset(asset, assetInfo.ClassName, assetInfo.Path);

	// stream
	RpgAssetStream::Read(asset.Get());

	// add to loaded data
	LoadedAssetTable.AddValue(assetPath, asset);
	RPG_CONSOLE_Log(RpgLogAsset, "Loaded asset (%s)", *assetPath);

	return asset;
}


RpgSharedAsset RpgAssetSystem::LoadAssetAsync(const RpgString& assetPath) noexcept
{
	RPG_IsMainThread();

	if (assetPath.IsEmpty())
	{
		RPG_CONSOLE_Warn(RpgLogAsset, "Fail to load asset. Invalid asset path (%s)!", *assetPath);
		return RpgSharedAsset();
	}

	// check if loaded
	RpgSharedAsset asset = FindLoadedAsset(assetPath);
	if (asset)
	{
		return asset;
	}

	// check if exists in registry
	RpgAssetInfo assetInfo;
	if (!DoesAssetExists(assetPath, &assetInfo))
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to load asset (%s). Asset not found in registry!", *assetPath);
		return RpgSharedAsset();
	}

	// create asset
	CreateAsset(asset, assetInfo.ClassName, assetInfo.Path);

	// TODO: Add to loading asset
	RPG_NotImplementedYet();

	return RpgSharedAsset();
}


RpgSharedAsset RpgAssetSystem::FindLoadedAsset(const RpgString& assetPath) const noexcept
{
	if (assetPath.IsEmpty())
	{
		return RpgSharedAsset();
	}

	int index = RPG_INDEX_INVALID;
	if (LoadedAssetTable.Exists(assetPath, &index))
	{
		const RpgSharedAsset& asset = LoadedAssetTable.GetValueByIndex(index);
		RPG_Check(asset->GetAssetPath().Equals(assetPath, true));
		return asset;
	}

	return RpgSharedAsset();
}


void RpgAssetSystem::AddLoadingAsset(const RpgSharedAsset& asset) noexcept
{
	RPG_Check(asset.IsValid());

	const RpgString& assetPath = asset->GetAssetPath();
	RPG_Check(!assetPath.IsEmpty());

	RPG_CONSOLE_Log(RpgLogAsset, "Add loading asset (%s)", *assetPath);
	LoadingAssetTable.AddValue(assetPath, asset);
}


void RpgAssetSystem::RegisterAssetFile(const RpgFilePath& filePath, RpgString* optOut_AssetPath) noexcept
{
	// get asset path
	const RpgString assetPath = GetAssetPathFromFile(filePath);

	if (optOut_AssetPath)
	{
		*optOut_AssetPath = assetPath;
	}

	// check if already exists in registry
	if (Registry.AssetInfoTable.Exists(assetPath))
	{
		return;
	}

	// check if valid
	RpgAssetInfo assetInfo;
	if (!IsValidAssetFile(filePath, &assetInfo))
	{
		return;
	}

	// Add to registry
	Registry.AssetInfoTable.AddValue(assetPath, assetInfo);

	RPG_CONSOLE_Log(RpgLogAsset, "Added asset to registry (Path: %s, Type: %s)", *assetPath, RPG_ASSET_FILE_TYPE_NAMES[static_cast<uint16_t>(assetInfo.Type)]);
}


RpgString RpgAssetSystem::GetAssetPathFromFile(const RpgFilePath& filePath) const noexcept
{
	const int dirPathLength = RpgFileSystem::GetAssetDirPath().GetLength();
	RpgString assetPath = filePath.ToString().Substring(dirPathLength);

	// try remove extension
	const int tokenExtIndex = assetPath.FindLastIndexOf('.');
	if (tokenExtIndex != RPG_INDEX_INVALID)
	{
		const int assetPathLength = assetPath.GetLength();
		const int extLength = assetPathLength - tokenExtIndex;
		assetPath = assetPath.Substring(0, assetPathLength - extLength);
	}

	return assetPath;
}
