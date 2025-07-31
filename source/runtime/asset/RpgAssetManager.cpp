#include "RpgAssetManager.h"
#include "core/RpgStream.h"



RpgAssetManager* g_AssetManager = nullptr;


RpgAssetManager::RpgAssetManager() noexcept
{
	LoadedModelData = RpgPointer::MakeUnique<RpgAssetLoadedData<RpgModel>>();
	LoadedMaterialData = RpgPointer::MakeUnique<RpgAssetLoadedData<RpgMaterial>>();
	LoadedTextureData = RpgPointer::MakeUnique<RpgAssetLoadedData<RpgTexture2D>>();
}


void RpgAssetManager::Update() noexcept
{
	LoadedModelData->RemoveUnreferenced();
	LoadedMaterialData->RemoveUnreferenced();
	LoadedTextureData->RemoveUnreferenced();
}


bool RpgAssetManager::IsValidAssetFile(const RpgFilePath& filePath, RpgAssetInfo* optOut_AssetInfo) noexcept
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
		optOut_AssetInfo->FilePath = filePath;
		optOut_AssetInfo->Type = static_cast<RpgAssetFileType>(header.Type);
	}

	return true;
}


void RpgAssetManager::ScanAssetFiles() noexcept
{
	RPG_Log(RpgLogAsset, "Scanning asset files...");

	RpgArray<RpgFilePath> assetFilePaths;
	RpgFileSystem::IterateFiles(assetFilePaths, RpgFileSystem::GetAssetDirPath(), true, ".rpga");

	for (int i = 0; i < assetFilePaths.GetCount(); ++i)
	{
		RegisterAssetFile(assetFilePaths[i]);
	}
}


bool RpgAssetManager::RegisterAssetFile(const RpgFilePath& filePath) noexcept
{
	// check if already exists
	const uint64_t hash = XXH3_64bits(*filePath, filePath.GetLength());
	if (RegisteredAssetHashes.FindIndexByValue(hash) != RPG_INDEX_INVALID)
	{
		return false;
	}

	// check if valid
	RpgAssetInfo info;
	if (!IsValidAssetFile(filePath, &info))
	{
		return false;
	}

	// Add to registry
	RegisteredAssetHashes.AddValue(hash);
	RegisteredAssetInfos.AddValue(info);

	RPG_Log(RpgLogAsset, "Added asset (FilePath: %s, Hash: %ull, Type: %s) to registry", *filePath, hash, RPG_ASSET_FILE_TYPE_NAMES[static_cast<uint16_t>(info.Type)]);

	return true;
}


void RpgAssetManager::SaveModel(const RpgSharedModel& model) noexcept
{
}


RpgSharedModel RpgAssetManager::LoadModel(const RpgFilePath& filePath) noexcept
{
	const uint64_t hash = XXH3_64bits(*filePath, filePath.GetLength());

	int index = RPG_INDEX_INVALID;
	if (LoadedModelData->IsLoaded(hash, &index))
	{
		return LoadedModelData->GetShared(index);
	}

	const RpgAssetInfo* info = GetAssetInfoByHash(hash);
	if (info == nullptr)
	{
		RPG_LogError(RpgLogAsset, "Model asset (%s) not found in registry!", *filePath);
		return RpgSharedModel();
	}

	RPG_Check(info->Type == RpgAssetFileType::MODEL);

	return RpgSharedModel();
}
