#include "RpgAssetSystem.h"
#include "thirdparty/xxhash/xxhash.h"


RPG_LOG_DEFINE_CATEGORY(RpgLogAsset, VERBOSITY_DEBUG)


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


RpgAssetSystem::~RpgAssetSystem() noexcept
{

}


void RpgAssetSystem::Update() noexcept
{
	// if this is the last reference (SharedRefCount == 1), that means no other referencing it, so remove it
	for (int i = 0; i < LoadedAssetObjects.GetCount();)
	{
		const RpgSharedAsset& check = LoadedAssetObjects[i];

		if (check.GetRefCount() == 1)
		{
			LoadedAssetObjects.RemoveAt(i);
			LoadedAssetHashes.RemoveAt(i);
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

	if (header.Magix != RPG_ASSET_FILE_MAGIX)
	{
		return false;
	}

	if (optOut_AssetInfo)
	{
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


const RpgAssetInfo* RpgAssetSystem::GetAssetInfoByFilePath(const RpgFilePath& filePath) const noexcept
{
	return GetAssetInfoByHash(XXH3_64bits(*filePath, filePath.GetLength()));
}


uint64_t RpgAssetSystem::GetAssetPathHash(const RpgString& path) const noexcept
{
	return XXH3_64bits(*path, path.GetLength());
}


void RpgAssetSystem::RegisterAssetFile(const RpgFilePath& filePath, RpgString* optOut_AssetPath, uint64_t* optOut_AssetHash) noexcept
{
	// get asset path
	const RpgString assetPath = GetAssetPathFromFile(filePath);
	if (optOut_AssetPath)
	{
		*optOut_AssetPath = assetPath;
	}

	const uint64_t hash = XXH3_64bits(*assetPath, assetPath.GetLength());
	if (optOut_AssetHash)
	{
		*optOut_AssetHash = hash;
	}

	// check if already exists
	if (RegisteredAssetHashes.FindIndexByValue(hash) != RPG_INDEX_INVALID)
	{
		return;
	}

	// check if valid
	RpgAssetInfo info;
	if (!IsValidAssetFile(filePath, &info))
	{
		return;
	}

	// Add to registry
	RegisteredAssetHashes.AddValue(hash);
	RegisteredAssetInfos.AddValue(info);

	RPG_CONSOLE_Log(RpgLogAsset, "Added asset to registry (Path: %s, Type: %s, Hash: %llu)", *assetPath, RPG_ASSET_FILE_TYPE_NAMES[static_cast<uint16_t>(info.Type)], hash);
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
