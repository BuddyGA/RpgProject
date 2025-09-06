/*
#include "RpgAssetManager.h"
#include "core/RpgConsoleSystem.h"
#include "core/world/RpgWorld.h"



RpgAssetManager* g_AssetManager = nullptr;



namespace RpgAssetStream
{
	template<typename TAsset>
	static RpgString SaveToFile(const RpgSharedPtr<TAsset>& asset, const char* directory) noexcept
	{
		if (!asset.IsValid())
		{
			RPG_CONSOLE_Error(RpgLogAsset, "Fail to save asset to file. Invalid asset!");
			return RpgString();
		}

		// Serialize
		RpgBinaryStreamWriter writer;
		asset->StreamWrite(writer);

		// create folder
		const RpgString dirPath = RpgString::Format("%s%s/", *RpgFileSystem::GetAssetDirPath(), directory);
		RpgFileSystem::CreateFolder(dirPath);

		const char* name = *asset->GetName();
		const RpgString filePath = RpgString::Format("%s%s.rpga", *dirPath, name);

		// open file
		HANDLE fileHandle = RpgPlatformFile::FileOpen(*filePath, RpgPlatformFile::OPEN_MODE_WRITE_OVERWRITE);
		RPG_Check(fileHandle && fileHandle != INVALID_HANDLE_VALUE);
		{
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

		return filePath;
	}


	template<typename TAsset>
	static RpgSharedPtr<TAsset> LoadFromFile(const RpgString& path, RpgAssetLoadedData<TAsset>& out_LoadedData, const RpgArray<uint64_t>& registryHashes, const RpgArray<RpgAssetInfo>& registryInfos)
	{
		const uint64_t hash = XXH3_64bits(*path, path.GetLength());

		// check if loaded
		int loadedIndex = RPG_INDEX_INVALID;
		if (out_LoadedData.IsLoaded(hash, &loadedIndex))
		{
			return out_LoadedData.GetSharedAtIndex(loadedIndex);
		}

		// check in registry
		const int regIndex = registryHashes.FindIndexByValue(hash);
		if (regIndex == RPG_INDEX_INVALID)
		{
			RPG_CONSOLE_Error(RpgLogAsset, "Asset (%s) not found in registry!", *path);
			return RpgSharedPtr<TAsset>();
		}

		RPG_Check(registryInfos[regIndex].Type == TAsset::FILE_TYPE);


		// load asset
		RPG_CONSOLE_Log(RpgLogAsset, "Load asset (%s)", *path);

		const RpgFilePath filePath = RpgString::Format("%s/%s.rpga", *RpgFileSystem::GetAssetDirPath(), *path);

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

		// data
		RpgSharedPtr<TAsset> asset = RpgPointer::MakeShared<TAsset>(filePath.GetFileName());
		asset->StreamRead(reader);
		RPG_Check(header.DataSizeBytes == reader.GetOffset() - sizeof(RpgAssetFileHeader));

		// eof
		int eof = 0;
		reader.Read(eof);
		RPG_Check(eof == RPG_ASSET_FILE_MAGIX);

		// add to loaded data
		out_LoadedData.Add(hash, asset);

		return asset;
	}

};




RpgAssetManager::RpgAssetManager() noexcept
{
}


void RpgAssetManager::Initialize() noexcept
{
	RPG_CONSOLE_Log(RpgLogAsset, "Initialize asset manager");


#ifndef RPG_BUILD_SHIPPING

	RpgFilePath assetDirPath = RpgFileSystem::GetAssetDirPath();
	RpgFileSystem::CreateFolder(assetDirPath);
	RpgFileSystem::CreateFolder(assetDirPath + "engine/");
	RpgFileSystem::CreateFolder(assetDirPath + "game/");

	SaveTexture(RpgTexture2D::s_GetDefault_White(), "engine/texture");

	for (int i = 0; i < RpgMaterialDefault::MAX_COUNT; ++i)
	{
		const RpgSharedMaterial& material = RpgMaterial::s_GetDefault(static_cast<RpgMaterialDefault::EType>(i));
		SaveMaterial(material, "engine/material");
	}

	ScanAssetFiles();

#endif // !RPG_BUILD_SHIPPING

}


void RpgAssetManager::Update() noexcept
{
	LoadedMeshData.RemoveUnreferenced();
	LoadedTextureData.RemoveUnreferenced();
	LoadedMaterialData.RemoveUnreferenced();
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
		optOut_AssetInfo->Path = GetAssetPath(filePath);
		optOut_AssetInfo->Type = static_cast<RpgAssetFileType>(header.Type);
	}

	return true;
}


void RpgAssetManager::ScanAssetFiles() noexcept
{
	RPG_CONSOLE_Log(RpgLogAsset, "Scanning asset files...");

	RpgArray<RpgFilePath> filePaths;
	RpgFileSystem::IterateFiles(filePaths, RpgFileSystem::GetAssetDirPath(), true, ".rpga");

	for (int i = 0; i < filePaths.GetCount(); ++i)
	{
		RegisterAssetFile(filePaths[i]);
	}
}




void RpgAssetManager::SaveMesh(const RpgSharedMesh& mesh, const char* directory) noexcept
{
	const RpgString filePath = RpgAssetStream::SaveToFile<RpgMesh>(mesh, directory);
	RPG_Check(!filePath.IsEmpty());

	uint64_t assetHash = 0;
	RegisterAssetFile(filePath, &assetHash);

	LoadedMeshData.Add(assetHash, mesh);
}


RpgSharedMesh RpgAssetManager::LoadMesh(const RpgString& path) noexcept
{
	if (path.IsEmpty())
	{
		return RpgSharedMesh();
	}

	return RpgAssetStream::LoadFromFile<RpgMesh>(path, LoadedMeshData, RegisteredAssetHashes, RegisteredAssetInfos);
}


void RpgAssetManager::SaveTexture(const RpgSharedTexture2D& texture, const char* directory) noexcept
{
	const RpgString filePath = RpgAssetStream::SaveToFile<RpgTexture2D>(texture, directory);
	RPG_Check(!filePath.IsEmpty());

	uint64_t assetHash = 0;
	RegisterAssetFile(filePath, &assetHash);

	LoadedTextureData.Add(assetHash, texture);
}


RpgSharedTexture2D RpgAssetManager::LoadTexture(const RpgString& path) noexcept
{
	if (path.IsEmpty())
	{
		return RpgSharedTexture2D();
	}

	return RpgAssetStream::LoadFromFile<RpgTexture2D>(path, LoadedTextureData, RegisteredAssetHashes, RegisteredAssetInfos);
}


void RpgAssetManager::SaveMaterial(const RpgSharedMaterial& material, const char* directory) noexcept
{
	const RpgString filePath = RpgAssetStream::SaveToFile<RpgMaterial>(material, directory);
	RPG_Check(!filePath.IsEmpty());

	uint64_t assetHash = 0;
	RegisterAssetFile(filePath, &assetHash);

	LoadedMaterialData.Add(assetHash, material);
}


RpgSharedMaterial RpgAssetManager::LoadMaterial(const RpgString& path) noexcept
{
	if (path.IsEmpty())
	{
		return RpgSharedMaterial();
	}

	return RpgAssetStream::LoadFromFile<RpgMaterial>(path, LoadedMaterialData, RegisteredAssetHashes, RegisteredAssetInfos);
}


void RpgAssetManager::SaveLevel(const RpgWorld* world, const char* directory) noexcept
{
	RPG_Check(world);

	RpgBinaryStreamWriter writer;
	world->StreamWrite(writer);

	// create folder
	const RpgString dirPath = RpgString::Format("%s%s/", *RpgFileSystem::GetAssetDirPath(), directory);
	RpgFileSystem::CreateFolder(dirPath);

	const RpgName& name = world->GetName();
	const RpgString filePath = RpgString::Format("%s%s.rpga", *dirPath, *name);

	// open file
	HANDLE fileHandle = RpgPlatformFile::FileOpen(*filePath, RpgPlatformFile::OPEN_MODE_WRITE_OVERWRITE);
	RPG_Check(fileHandle && fileHandle != INVALID_HANDLE_VALUE);
	{
		// header
		RpgAssetFileHeader header;
		header.Magix = RPG_ASSET_FILE_MAGIX;
		header.Type = static_cast<uint16_t>(RpgAssetFileType::LEVEL);
		header.Version = 1;
		header.DataSizeBytes = static_cast<uint32_t>(writer.GetByteArraySize());
		RpgPlatformFile::FileWrite(fileHandle, &header, sizeof(RpgAssetFileHeader));

		// data
		RpgPlatformFile::FileWrite(fileHandle, writer.GetByteArrayData(), writer.GetByteArraySize());

		// eof
		const int eof = RPG_ASSET_FILE_MAGIX;
		RpgPlatformFile::FileWrite(fileHandle, &eof, sizeof(int));
	}
	RpgPlatformFile::FileClose(fileHandle);

	RPG_CONSOLE_Log(RpgLogAsset, "Saved level (%s) to file (%s)", name, *filePath);

	RegisterAssetFile(filePath);
}


void RpgAssetManager::LoadLevel(const RpgString& path, RpgWorld* out_World) noexcept
{
	const uint64_t hash = XXH3_64bits(*path, path.GetLength());

	const int regIndex = RegisteredAssetHashes.FindIndexByValue(hash);
	if (regIndex == RPG_INDEX_INVALID)
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to load level (%s). Level not found in registry!", *path);
		return;
	}

	const RpgAssetInfo& info = RegisteredAssetInfos[regIndex];
	RPG_Check(info.Type == RpgAssetFileType::LEVEL);

	const RpgString filePath = RpgString::Format("%s/%s.rpga", *RpgFileSystem::GetAssetDirPath(), *path);

	RpgArray<uint8_t> data;
	if (!RpgFileSystem::ReadFromFile(filePath, data))
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to load level (%s). Reading file data failed! (FilePath: %s)", *path, *filePath);
		return;
	}

	RpgBinaryStreamReader reader(data);

	// header
	RpgAssetFileHeader header;
	reader.Read(header);
	RPG_Check(header.Magix == RPG_ASSET_FILE_MAGIX);
	RPG_Check(header.Type == static_cast<uint16_t>(RpgAssetFileType::LEVEL));
	RPG_Check(header.Version == 1);

	// data
	out_World->StreamRead(reader);
	RPG_Check(header.DataSizeBytes == reader.GetOffset() - sizeof(RpgAssetFileHeader));

	// eof
	int eof = 0;
	reader.Read(eof);
	RPG_Check(eof == RPG_ASSET_FILE_MAGIX);

	RPG_CONSOLE_Log(RpgLogAsset, "Loaded level (%s)", *path);
}


bool RpgAssetManager::RegisterAssetFile(const RpgFilePath& filePath, uint64_t* optOut_Hash) noexcept
{
	// get asset path
	const RpgString assetPath = GetAssetPath(filePath);

	// check if already exists
	const uint64_t hash = XXH3_64bits(*assetPath, assetPath.GetLength());

	if (optOut_Hash)
	{
		*optOut_Hash = hash;
	}

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

	RPG_CONSOLE_Log(RpgLogAsset, "Added asset to registry (FilePath: %s, Hash: %llu, Type: %s)", *filePath, hash, RPG_ASSET_FILE_TYPE_NAMES[static_cast<uint16_t>(info.Type)]);

	return true;
}


RpgString RpgAssetManager::GetAssetPath(const RpgFilePath& filePath) const noexcept
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
*/
