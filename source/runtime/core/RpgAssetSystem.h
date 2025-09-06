#pragma once

#include "RpgAssetTypes.h"
#include "RpgConsoleSystem.h"



class RpgAssetSystem
{
	RPG_NOCOPYMOVE(RpgAssetSystem)

public:
	RpgAssetSystem() noexcept;
	virtual ~RpgAssetSystem() noexcept;


	// Update asset loading and try to unload if no other referencing it
	void Update() noexcept;


	// Check if file is a valid asset file
	// @param filePath - Absolute path to a file
	// @param optOut_AssetInfo - (Optional) output asset info if file is valid
	// @return TRUE if file is valid
	bool IsValidAssetFile(const RpgFilePath& filePath, RpgAssetInfo* optOut_AssetInfo = nullptr) noexcept;


	// Scan all asset files in asset directory and try register them
	void ScanAssetFiles() noexcept;


	// Get asset info from registry
	// @param filePath - Absolute path to a file
	// @return Pointer to asset info, nullptr if file not found in registry
	const RpgAssetInfo* GetAssetInfoByFilePath(const RpgFilePath& filePath) const noexcept;


	// Save asset to file
	// @param asset - Asset to save
	// @param directory - Path relative to asset directory
	template<typename TAsset>
	void SaveAsset(RpgSharedPtr<TAsset>& asset, const char* directory) noexcept;


	// Load asset
	// @param path - Asset path relative to asset directory
	// @return SharedPtr to asset if path is valid and asset is loaded
	template<typename TAsset>
	RpgSharedPtr<TAsset> LoadAsset(const RpgString& path) noexcept;


private:
	uint64_t GetAssetPathHash(const RpgString& path) const noexcept;

	// Try register file as asset file
	// @param filePath - Absolute path to an asset file
	// @param optOut_Hash - (optional) Asset hash
	void RegisterAssetFile(const RpgFilePath& filePath, RpgString* optOut_AssetPath = nullptr, uint64_t* optOut_AssetHash = nullptr) noexcept;


	// Get asset path 
	// @param filePath - Absolute path to an asset file
	// @return Relative asset path
	RpgString GetAssetPathFromFile(const RpgFilePath& filePath) const noexcept;


	// Get asset info by asset hash from registry
	// @param hash - Asset path hash
	// @return Pointer to RpgAssetInfo if found in registry or NULL if not found
	inline const RpgAssetInfo* GetAssetInfoByHash(uint64_t hash) const noexcept
	{
		const int index = RegisteredAssetHashes.FindIndexByValue(hash);
		return index == RPG_INDEX_INVALID ? nullptr : &RegisteredAssetInfos[index];
	}


private:
	// Asset registry
	RpgArray<uint64_t, 8> RegisteredAssetHashes;
	RpgArray<RpgAssetInfo, 8> RegisteredAssetInfos;

	// Loaded data
	RpgArray<uint64_t, 8> LoadedAssetHashes;
	RpgArray<RpgSharedAsset, 8> LoadedAssetObjects;

};



template<typename TAsset>
void RpgAssetSystem::SaveAsset(RpgSharedPtr<TAsset>& asset, const char* directory) noexcept
{
	static_assert(std::is_base_of<RpgAssetObject, TAsset>::value, "RpgAssetSystem::SaveAsset type of <TAsset> must be derived from type <RpgAssetObject>!");

	if (!asset)
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to save asset to file. Invalid asset!");
		return;
	}

	// create folder
	const RpgString dirPath = RpgString::Format("%s%s/", *RpgFileSystem::GetAssetDirPath(), directory);
	RpgFileSystem::CreateFolder(dirPath);

	const char* name = *asset->GetAssetName();
	const RpgFilePath filePath = RpgString::Format("%s%s.rpga", *dirPath, name);

	// open file
	HANDLE fileHandle = RpgPlatformFile::FileOpen(*filePath, RpgPlatformFile::OPEN_MODE_WRITE_OVERWRITE);
	RPG_Check(fileHandle && fileHandle != INVALID_HANDLE_VALUE);
	{
		// Serialize
		RpgBinaryStreamWriter writer;
		asset->StreamWrite(writer);

		// header
		RpgAssetFileHeader header;
		header.Magix = RPG_ASSET_FILE_MAGIX;
		header.Type = static_cast<uint16_t>(TAsset::FILE_TYPE);
		header.Version = TAsset::FILE_VERSION;
		header.DataSizeBytes = static_cast<uint32_t>(writer.GetByteArraySize());
		RpgPlatformFile::FileWrite(fileHandle, &header, sizeof(RpgAssetFileHeader));

		// data
		RpgPlatformFile::FileWrite(fileHandle, writer.GetByteArrayData(), writer.GetByteArraySize());

		// eof
		const int eof = RPG_ASSET_FILE_MAGIX;
		RpgPlatformFile::FileWrite(fileHandle, &eof, sizeof(int));
	}
	RpgPlatformFile::FileClose(fileHandle);

	RPG_CONSOLE_Log(RpgLogAsset, "Saved asset (%s) to file (%s)", name, *filePath);

	RpgString assetPath;
	uint64_t assetHash = 0;
	RegisterAssetFile(filePath, &assetPath, &assetHash);

	// set asset path
	static_cast<RpgAssetObject*>(asset.Get())->Path = assetPath;

	if (LoadedAssetHashes.FindIndexByValue(assetHash) == RPG_INDEX_INVALID)
	{
		LoadedAssetHashes.AddValue(assetHash);
		LoadedAssetObjects.AddValue(asset.CastStatic<RpgAssetObject>());
	}
}



template<typename TAsset>
RpgSharedPtr<TAsset> RpgAssetSystem::LoadAsset(const RpgString& path) noexcept
{
	static_assert(std::is_base_of<RpgAssetObject, TAsset>::value, "RpgAssetSystem::LoadAsset type of <TAsset> must be derived from type <RpgAssetObject>!");

	if (path.IsEmpty())
	{
		return RpgSharedPtr<TAsset>();
	}

	const uint64_t hash = GetAssetPathHash(path);

	// check if loaded
	const int loadedIndex = LoadedAssetHashes.FindIndexByValue(hash);
	if (loadedIndex != RPG_INDEX_INVALID)
	{
		const RpgSharedAsset& checkAsset = LoadedAssetObjects[loadedIndex];
		RPG_Check(checkAsset->GetAssetPath().Equals(path, true));

		return checkAsset.CastDynamic<TAsset>();
	}

	// check if exists in registry
	const int regIndex = RegisteredAssetHashes.FindIndexByValue(hash);
	if (regIndex == RPG_INDEX_INVALID)
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to load asset (%s). Asset not found in registry!", *path);
		return RpgSharedPtr<TAsset>();
	}

	// load from file
	const RpgFilePath filePath = RpgString::Format("%s%s.rpga", *RpgFileSystem::GetAssetDirPath(), *path);

	RpgArray<uint8_t> data;
	if (!RpgFileSystem::ReadFromFile(filePath.ToString(), data))
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to load asset (%s). Reading file data failed! (FilePath: %s)", *path, *filePath);
		return RpgSharedPtr<TAsset>();
	}

	RpgBinaryStreamReader reader(data);

	// header
	RpgAssetFileHeader header;
	reader.Read(header);
	RPG_Check(header.Magix == RPG_ASSET_FILE_MAGIX);
	RPG_Check(header.Type == static_cast<uint16_t>(TAsset::FILE_TYPE));
	RPG_Check(header.Version == TAsset::FILE_VERSION);

	// create asset
	RpgSharedPtr<TAsset> asset = RpgPointer::MakeShared<TAsset>(filePath.GetFileName());

	// set asset path
	static_cast<RpgAssetObject*>(asset.Get())->Path = path;

	// deserialize
	asset->StreamRead(reader);
	RPG_Check(header.DataSizeBytes == reader.GetOffset() - sizeof(RpgAssetFileHeader));

	// check eof
	int eof = 0;
	reader.Read(eof);
	RPG_Check(eof == RPG_ASSET_FILE_MAGIX);

	// add to loaded data
	LoadedAssetHashes.AddValue(hash);
	LoadedAssetObjects.AddValue(asset.CastStatic<RpgAssetObject>());

	RPG_CONSOLE_Log(RpgLogAsset, "Loaded asset (%s)", *path);

	return asset;
}
