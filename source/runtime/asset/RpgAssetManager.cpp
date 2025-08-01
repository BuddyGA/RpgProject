#include "RpgAssetManager.h"
#include "core/RpgStream.h"



RpgAssetManager* g_AssetManager = nullptr;


RpgAssetManager::RpgAssetManager() noexcept
{
	LoadedMeshData = RpgPointer::MakeUnique<RpgAssetLoadedData<RpgMesh>>();
	LoadedMaterialData = RpgPointer::MakeUnique<RpgAssetLoadedData<RpgMaterial>>();
	LoadedTextureData = RpgPointer::MakeUnique<RpgAssetLoadedData<RpgTexture2D>>();
}


void RpgAssetManager::Update() noexcept
{
	LoadedMeshData->RemoveUnreferenced();
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


void RpgAssetManager::SaveMesh(const RpgSharedMesh& mesh) noexcept
{
	if (!mesh.IsValid())
	{
		RPG_LogError(RpgLogAsset, "Fail to save mesh to asset file. Invalid mesh asset!");
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

	const RpgString assetFilePath = RpgString::Format("%smeshes/%s.rpga", *RpgFileSystem::GetAssetDirPath(), *mesh->GetName());
	
	if (!RpgFileSystem::WriteToFile(assetFilePath, writer.GetByteData(), writer.GetByteSize()))
	{
		RPG_LogError(RpgLogAsset, "Fail to save mesh (%s) to asset file (%s)", *mesh->GetName(), *assetFilePath);
		return;
	}

	RPG_Log(RpgLogAsset, "Saved mesh (%s) to asset file (%s)", *mesh->GetName(), *assetFilePath);
	RegisterAssetFile(assetFilePath);
}


RpgSharedMesh RpgAssetManager::LoadMesh(const RpgFilePath& filePath) noexcept
{
	const uint64_t hash = XXH3_64bits(*filePath, filePath.GetLength());

	int index = RPG_INDEX_INVALID;
	if (LoadedMeshData->IsLoaded(hash, &index))
	{
		return LoadedMeshData->GetSharedAtIndex(index);
	}

	const RpgAssetInfo* info = GetAssetInfoByHash(hash);
	if (info == nullptr)
	{
		RPG_LogError(RpgLogAsset, "Mesh asset (%s) not found in registry!", *filePath);
		return RpgSharedMesh();
	}

	RPG_Check(info->Type == RpgAssetFileType::MESH);

	return RpgSharedMesh();
}
