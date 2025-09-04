#include "RpgAssetManager.h"
#include "core/RpgConsoleSystem.h"
#include "core/RpgStream.h"



RpgAssetManager* g_AssetManager = nullptr;



namespace RpgAssetStream
{
	template<typename TAsset>
	static RpgString SaveToFile(const RpgSharedPtr<TAsset>& asset, RpgAssetFileType type, uint16_t version, const char* directory) noexcept
	{
		if (!asset.IsValid())
		{
			RPG_CONSOLE_Error(RpgLogAsset, "Fail to save asset to file. Invalid asset!");
			return RpgString();
		}

		RpgAssetFileHeader fileHeader;
		fileHeader.Magix = RPG_ASSET_FILE_MAGIX;
		fileHeader.Type = static_cast<uint16_t>(type);
		fileHeader.Version = version;
		fileHeader.OffsetBytes = 0;
		fileHeader.SizeBytes = sizeof(RpgAssetFileHeader) +		// header
			asset->CalculateDataSizeBytes() +					// data
			sizeof(int);										// eof

		RpgBinaryStreamWriter writer;
		writer.Write(fileHeader);
		asset->StreamWrite(writer);
		writer.Write(RPG_ASSET_FILE_MAGIX);

		const char* name = *asset->GetName();
		const RpgString filePath = RpgString::Format("%s%s/%s.rpga", *RpgFileSystem::GetAssetDirPath(), directory, name);

		if (!RpgFileSystem::WriteToFile(filePath, writer.GetByteData(), writer.GetByteSize()))
		{
			RPG_CONSOLE_Error(RpgLogAsset, "Fail to save asset (%s) to file (%s)", name, *filePath);
			return RpgString();
		}

		RPG_CONSOLE_Log(RpgLogAsset, "Saved asset (%s) to file (%s)", name, *filePath);

		return filePath;
	}

};




RpgAssetManager::RpgAssetManager() noexcept
{
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
	// Add engine asset files
#ifndef RPG_BUILD_SHIPPING
	SaveTexture(RpgTexture2D::s_GetDefault_White());
#endif // !RPG_BUILD_SHIPPING


	RPG_CONSOLE_Log(RpgLogAsset, "Scanning asset files...");

	RpgArray<RpgFilePath> filePaths;
	RpgFileSystem::IterateFiles(filePaths, RpgFileSystem::GetAssetDirPath(), true, ".rpga");

	for (int i = 0; i < filePaths.GetCount(); ++i)
	{
		RegisterAssetFile(filePaths[i]);
	}
}




void RpgAssetManager::SaveMesh(const RpgSharedMesh& mesh) noexcept
{
	const RpgString filePath = RpgAssetStream::SaveToFile(mesh, RpgAssetFileType::MESH, RPG_ASSET_FILE_VERSION_MESH, "mesh");
	RPG_Check(!filePath.IsEmpty());

	uint64_t assetHash = 0;
	RegisterAssetFile(filePath, &assetHash);

	LoadedMeshData.Add(assetHash, mesh);
}


RpgSharedMesh RpgAssetManager::LoadMesh(const RpgString& path) noexcept
{
	const uint64_t hash = XXH3_64bits(*path, path.GetLength());

	// check if loaded
	int index = RPG_INDEX_INVALID;
	if (LoadedMeshData.IsLoaded(hash, &index))
	{
		return LoadedMeshData.GetSharedAtIndex(index);
	}

	// check in registry
	const RpgAssetInfo* info = GetAssetInfoByHash(hash);
	if (info == nullptr)
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Mesh asset (%s) not found in registry!", *path);
		return RpgSharedMesh();
	}

	RPG_Check(info->Type == RpgAssetFileType::MESH);


	// load mesh
	RPG_CONSOLE_Log(RpgLogAsset, "Load mesh asset (%s)", *path);

	const RpgString filePath = RpgString::Format("%s/%s.rpga", *RpgFileSystem::GetAssetDirPath(), *path);

	RpgArray<uint8_t> data;
	if (!RpgFileSystem::ReadFromFile(filePath, data))
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to load mesh (%s). Reading file data failed! (FilePath: %s)", *path, *filePath);
		return RpgSharedMesh();
	}

	RpgBinaryStreamReader reader(data);

	RpgAssetFileHeader header;
	reader.Read(header);
	RPG_Check(header.Magix == RPG_ASSET_FILE_MAGIX);
	RPG_Check(header.Type == static_cast<uint16_t>(RpgAssetFileType::MESH));
	RPG_Check(header.Version == RPG_ASSET_FILE_VERSION_MESH);

	RpgSharedMesh mesh = RpgMesh::s_CreateShared("");
	mesh->StreamRead(reader);

	int eof = 0;
	reader.Read(eof);
	RPG_Check(eof == RPG_ASSET_FILE_MAGIX);

	// add to loaded mesh
	LoadedMeshData.Add(hash, mesh);

	return mesh;
}


void RpgAssetManager::SaveTexture(const RpgSharedTexture2D& texture) noexcept
{
	const RpgString filePath = RpgAssetStream::SaveToFile(texture, RpgAssetFileType::TEXTURE, RPG_ASSET_FILE_VERSION_TEXTURE, "texture");
	RPG_Check(!filePath.IsEmpty());

	uint64_t assetHash = 0;
	RegisterAssetFile(filePath, &assetHash);

	LoadedTextureData.Add(assetHash, texture);
}


RpgSharedTexture2D RpgAssetManager::LoadTexture(const RpgString& path) noexcept
{
	RPG_NotImplementedYet();
	return RpgSharedTexture2D();
}


void RpgAssetManager::SaveMaterial(const RpgSharedMaterial& material) noexcept
{
	const RpgString filePath = RpgAssetStream::SaveToFile(material, RpgAssetFileType::MATERIAL, RPG_ASSET_FILE_VERSION_MATERIAL, "material");
	RPG_Check(!filePath.IsEmpty());

	uint64_t assetHash = 0;
	RegisterAssetFile(filePath, &assetHash);

	LoadedMaterialData.Add(assetHash, material);
}


RpgSharedMaterial RpgAssetManager::LoadMaterial(const RpgString& path) noexcept
{
	RPG_NotImplementedYet();
	return RpgSharedMaterial();
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
