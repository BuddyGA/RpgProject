#include "RpgAssetManager.h"
#include "core/RpgConsoleSystem.h"
#include "core/RpgStream.h"



RpgAssetManager* g_AssetManager = nullptr;


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
	RPG_CONSOLE_Log(RpgLogAsset, "Scanning asset files...");

	RpgArray<RpgFilePath> filePaths;
	RpgFileSystem::IterateFiles(filePaths, RpgFileSystem::GetAssetDirPath(), true, ".rpga");

	for (int i = 0; i < filePaths.GetCount(); ++i)
	{
		RegisterAssetFile(filePaths[i]);
	}
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


void RpgAssetManager::SaveMesh(const RpgSharedMesh& mesh) noexcept
{
	if (!mesh.IsValid())
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to save mesh to asset file. Invalid mesh asset!");
		return;
	}

	RpgAssetFileHeader fileHeader;
	fileHeader.Magix = RPG_ASSET_FILE_MAGIX;
	fileHeader.Type = static_cast<uint16_t>(RpgAssetFileType::MESH);
	fileHeader.Version = RPG_ASSET_FILE_VERSION_MESH;
	fileHeader.OffsetBytes = 0;
	fileHeader.SizeBytes = sizeof(RpgAssetFileHeader) +						// header
		static_cast<uint32_t>(RpgMesh::s_CalculateAssetSizeBytes(mesh)) +	// data
		sizeof(int);														// eof

	RpgBinaryStreamWriter writer;
	writer.Write(fileHeader);
	mesh->StreamWrite(writer);
	writer.Write(RPG_ASSET_FILE_MAGIX);

	const RpgString filePath = RpgString::Format("%smesh/%s.rpga", *RpgFileSystem::GetAssetDirPath(), *mesh->GetName());
	
	if (!RpgFileSystem::WriteToFile(filePath, writer.GetByteData(), writer.GetByteSize()))
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to save mesh (%s) to asset file (%s)", *mesh->GetName(), *filePath);
		return;
	}

	RPG_CONSOLE_Log(RpgLogAsset, "Saved mesh (%s) to asset file (%s)", *mesh->GetName(), *filePath);

	uint64_t assetHash = 0;
	RegisterAssetFile(filePath, &assetHash);

	LoadedMeshData.Add(assetHash, mesh);
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


RpgSharedMesh RpgAssetManager::LoadMesh(const RpgString& path) noexcept
{
	const uint64_t hash = XXH3_64bits(*path, path.GetLength());

	int index = RPG_INDEX_INVALID;
	if (LoadedMeshData.IsLoaded(hash, &index))
	{
		return LoadedMeshData.GetSharedAtIndex(index);
	}

	const RpgAssetInfo* info = GetAssetInfoByHash(hash);
	if (info == nullptr)
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Mesh asset (%s) not found in registry!", *path);
		return RpgSharedMesh();
	}

	RPG_Check(info->Type == RpgAssetFileType::MESH);

	RPG_CONSOLE_Log(RpgLogAsset, "Load mesh asset (%s)", *path);
	
	const RpgString filePath = RpgString::Format("%s/%s.rpga", *RpgFileSystem::GetAssetDirPath(), *path);

	RpgArray<uint8_t> fileData;
	if (!RpgFileSystem::ReadFromFile(filePath, fileData))
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to load mesh (%s). Reading file data failed! (FilePath: %s)", *path, *filePath);
		return RpgSharedMesh();
	}

	RpgBinaryStreamReader reader(fileData);

	RpgAssetFileHeader header;
	reader.Read(header);
	RPG_Check(header.Magix == RPG_ASSET_FILE_MAGIX);
	RPG_Check(header.Type == static_cast<uint16_t>(RpgAssetFileType::MESH));
	RPG_Check(header.Version == RPG_ASSET_FILE_VERSION_MESH);

	RpgSharedMesh mesh = RpgMesh::s_CreateShared("");
	mesh->StreamRead(reader);

	int eofMagix = 0;
	reader.Read(eofMagix);
	RPG_Check(eofMagix == RPG_ASSET_FILE_MAGIX);

	LoadedMeshData.Add(hash, mesh);

	return mesh;
}
