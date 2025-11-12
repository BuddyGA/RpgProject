#include "RpgAssetSystem.h"
#include "../RpgThreadPool.h"


// Asset file extension
#define RPG_ASSET_FILE_EXT_0						".rpga"
#define RPG_ASSET_FILE_EXT_1						".rpgm"


// Magic number for asset file header
#define RPG_ASSET_FILE_HEADER					0x41475052U // (RPGA)

// Magic number for marker asset ref
#define RPG_ASSET_FILE_ASSET_REF				0x52475052U // (RPGR)

// Magic number for marker data asset
#define RPG_ASSET_FILE_ASSET_DATA				0x44475052U // (RPGD)

// Magic number for marker end-of-file
#define RPG_ASSET_FILE_EOF						0x41475052U // (RPGA)



RPG_LOG_DECLARE_CATEGORY_STATIC(RpgLogAsset, VERBOSITY_DEBUG)



namespace RpgAssetStream
{
	static RpgFilePath Write(RpgAssetObject* asset, const char* dstFolder) noexcept
	{
		RPG_Check(asset);
		RPG_Check(dstFolder);

		// create folder
		const RpgString folderPath = RpgString::Format("%s%s/", *RpgFileSystem::GetAssetDirPath(), dstFolder);
		RpgFileSystem::CreateFolder(folderPath);

		// file extension
		const char* fileExt = asset->GetAssetFileType() != RpgAssetFileType::LEVEL ? RPG_ASSET_FILE_EXT_0 : RPG_ASSET_FILE_EXT_1;

		// output file path
		const RpgFilePath filePath = RpgString::Format("%s%s%s", *folderPath, *asset->GetAssetName(), fileExt);

		// open file
		HANDLE fileHandle = RpgPlatformFile::FileOpen(*filePath, RpgPlatformFile::OPEN_MODE_WRITE_OVERWRITE);
		RPG_ValidateV(fileHandle && fileHandle != INVALID_HANDLE_VALUE, "Open file failed! (FilePath: %s)", *filePath);


		// asset references
		RpgAssetReferences assetRefs;
		RpgBinaryStreamWriter refWriter;
		{
			asset->GetExternalAssetReferences(assetRefs);

			refWriter.Write(RPG_ASSET_FILE_ASSET_REF);
			refWriter.Write(assetRefs);
			refWriter.Write(RPG_ASSET_FILE_ASSET_REF);
		}
		RPG_Check(assetRefs.GetCount() < UINT16_MAX);


		// data asset
		RpgBinaryStreamWriter dataWriter;
		{
			dataWriter.Write(RPG_ASSET_FILE_ASSET_DATA);
			asset->AssetStreamWrite(dataWriter);
			dataWriter.Write(RPG_ASSET_FILE_ASSET_DATA);

			RPG_Check(dataWriter.GetByteArraySize() < UINT32_MAX);
		}


		// header
		RpgAssetFileHeader fileHeader;
		{
			fileHeader.Magix = RPG_ASSET_FILE_HEADER;
			fileHeader.Type = static_cast<uint16_t>(asset->GetAssetFileType());
			fileHeader.Version = asset->GetAssetFileVersion();
			fileHeader.AssetReferenceCount = static_cast<uint16_t>(assetRefs.GetCount());
			fileHeader.AssetReferenceSizeBytes = static_cast<uint32_t>(refWriter.GetByteArraySize());
			fileHeader.AssetDataSizeBytes = static_cast<uint32_t>(dataWriter.GetByteArraySize());
			fileHeader.AssetClassName = asset->GetAssetClassName();
		}


		// write to file (header)
		RpgPlatformFile::FileWrite(fileHandle, &fileHeader, sizeof(RpgAssetFileHeader));

		// write to file (asset refs)
		RpgPlatformFile::FileWrite(fileHandle, refWriter.GetByteArrayData(), refWriter.GetByteArraySize());

		// write to file (data)
		RpgPlatformFile::FileWrite(fileHandle, dataWriter.GetByteArrayData(), dataWriter.GetByteArraySize());

		// write to file (end-of-file)
		const uint32_t eof = RPG_ASSET_FILE_EOF;
		RpgPlatformFile::FileWrite(fileHandle, &eof, sizeof(uint32_t));

		// close
		RpgPlatformFile::FileClose(fileHandle);

		return filePath;
	}


	static RpgBinaryStreamReader CreateReaderForAsset(RpgAssetObject* asset) noexcept
	{
		RPG_Check(asset);

		const RpgString& path = asset->GetAssetPath();
		RPG_Check(!path.IsEmpty());

		const RpgAssetFileType fileType = asset->GetAssetFileType();

		// load from file
		const RpgFilePath filePath = RpgString::Format("%s%s%s",
			*RpgFileSystem::GetAssetDirPath(),
			*path,
			fileType != RpgAssetFileType::LEVEL ? RPG_ASSET_FILE_EXT_0 : RPG_ASSET_FILE_EXT_1
		);

		RpgArray<uint8_t> data;
		RPG_ValidateV(RpgFileSystem::ReadFromFile(filePath.ToString(), data), "Read file data failed! (FilePath: %s)", *filePath);

		return RpgBinaryStreamReader(data);
	}


	static void ReadHeaderAndAssetReferences(RpgBinaryStreamReader& reader, RpgAssetObject* asset, RpgAssetFileHeader& out_Header, RpgAssetReferences& out_AssetReferences) noexcept
	{
		RPG_Check(asset);

		// header
		{
			reader.Read(out_Header);
			RPG_Check(out_Header.Magix == RPG_ASSET_FILE_HEADER);
			RPG_Check(out_Header.Type == static_cast<uint16_t>(asset->GetAssetFileType()));
		}

		// asset references
		{
			uint32_t begin = 0;
			reader.Read(begin);
			RPG_Check(begin == RPG_ASSET_FILE_ASSET_REF);

			reader.Read(out_AssetReferences);

			uint32_t end = 0;
			reader.Read(end);
			RPG_Check(end == RPG_ASSET_FILE_ASSET_REF);

			RPG_Check(out_Header.AssetReferenceSizeBytes == reader.GetOffset() - sizeof(RpgAssetFileHeader));
		}
	}


	static void ReadAsset(RpgBinaryStreamReader& reader, RpgAssetObject* asset, const RpgAssetFileHeader& header) noexcept
	{
		RPG_Check(asset);

		// data
		{
			uint32_t begin = 0;
			reader.Read(begin);
			RPG_Check(begin == RPG_ASSET_FILE_ASSET_DATA);

			asset->AssetStreamRead(reader, header.Version);

			uint32_t end = 0;
			reader.Read(end);
			RPG_Check(end == RPG_ASSET_FILE_ASSET_DATA);

			RPG_Check(header.AssetDataSizeBytes == reader.GetOffset() - sizeof(RpgAssetFileHeader) - header.AssetReferenceSizeBytes);
		}
	}


	static inline void ReadEndFile(RpgBinaryStreamReader& reader) noexcept
	{
		// end-of-file
		uint32_t eof = 0;
		reader.Read(eof);
		RPG_Check(eof == RPG_ASSET_FILE_EOF);
	}


	static void ReadAssetAll(RpgAssetObject* asset) noexcept
	{
		RPG_Check(asset);

		RpgBinaryStreamReader reader = CreateReaderForAsset(asset);

		RpgAssetFileHeader header;
		RpgAssetReferences assetRefs;
		ReadHeaderAndAssetReferences(reader, asset, header, assetRefs);

		ReadAsset(reader, asset, header);

		ReadEndFile(reader);
	}

};



class RpgAssetTask_Loader : public RpgThreadTask
{
public:
	RpgSharedAsset Asset;


public:
	RpgAssetTask_Loader() noexcept = default;


	virtual void Reset() noexcept override
	{
		RpgThreadTask::Reset();

		Asset.Release();
	}


	virtual void Execute() noexcept override
	{
		RpgAssetStream::ReadAssetAll(Asset.Get());
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


RpgAssetSystem::~RpgAssetSystem() noexcept
{
	RPG_Check(LoadingAssetTable.IsEmpty() && LoadingAssetTasks.IsEmpty());
}


void RpgAssetSystem::Update() noexcept
{
	// Trying to remove unreferenced assets
	{
		RPG_PLATFORM_ScopedLock(LoadedMutex);

		for (int i = 0; i < LoadedAssetTable.GetCount();)
		{
			const RpgSharedAsset& check = LoadedAssetTable.GetValueByIndex(i);

			// if this is the last reference (SharedRefCount == 1), that means no other referencing it, so remove it
			if (check.GetRefCount() == 1)
			{
				LoadedAssetTable.RemoveAt(i);
			}
			else
			{
				++i;
			}
		}
	}

	// update loading assets
	{
		RPG_PLATFORM_ScopedLock(LoadingMutex);
		RPG_Check(LoadingAssetTable.GetCount() == LoadingAssetTasks.GetCount());

		for (int i = 0; i < LoadingAssetTasks.GetCount(); )
		{
			RpgAssetTask_Loader* task = LoadingAssetTasks[i];

			if (task->IsDone())
			{
				const RpgSharedAsset& asset = task->Asset;
				const RpgString& path = asset->GetAssetPath();

				AddLoadedAsset(path, asset);

				LoadingAssetTable.RemoveAt(i);

				delete LoadingAssetTasks[i];
				LoadingAssetTasks.RemoveAt(i);
			}
			else
			{
				++i;
			}
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
	if (fileExt != RPG_ASSET_FILE_EXT_0 && fileExt != RPG_ASSET_FILE_EXT_1)
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

	if (header.Magix != RPG_ASSET_FILE_HEADER || 
		header.Type == static_cast<uint16_t>(RpgAssetFileType::NONE) || 
		header.AssetDataSizeBytes == 0 ||
		header.AssetClassName.IsEmpty()
		)
	{
		return false;
	}

	if (optOut_AssetInfo)
	{
		optOut_AssetInfo->Class = header.AssetClassName;
		optOut_AssetInfo->Path = GetAssetPathFromFile(filePath);
		optOut_AssetInfo->Type = static_cast<RpgAssetFileType>(header.Type);
	}

	return true;
}


void RpgAssetSystem::ScanAssetFiles() noexcept
{
	RPG_CONSOLE_Log(RpgLogAsset, "Scanning asset files...");

	RpgArray<RpgFilePath> filePaths;
	RpgFileSystem::IterateFiles(filePaths, RpgFileSystem::GetAssetDirPath(), true);

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
	AddLoadedAsset(assetPath, asset);
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
	if (!IsAssetRegistered(assetPath, &assetInfo))
	{
		RPG_CONSOLE_Error(RpgLogAsset, "Fail to load asset (%s). Asset not found in registry!", *assetPath);
		return RpgSharedAsset();
	}

	// create asset
	CreateAsset(asset, assetInfo.Class, assetInfo.Path);
	asset->SetAssetLoading();

	// stream
	RpgAssetStream::ReadAssetAll(asset.Get());

	// add to loaded data
	AddLoadedAsset(assetPath, asset);
	RPG_CONSOLE_Log(RpgLogAsset, "Loaded asset (%s)", *assetPath);

	return asset;
}


RpgSharedAsset RpgAssetSystem::LoadAssetAsync(const RpgString& assetPath) noexcept
{
	//RPG_IsMainThread();

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

	// check if loading
	{
		RPG_PLATFORM_ScopedLock(LoadingMutex);

		int loadingIndex = RPG_INDEX_INVALID;
		if (LoadingAssetTable.Exists(assetPath, &loadingIndex))
		{
			return LoadingAssetTable.GetValueByIndex(loadingIndex);
		}

		// check if exists in registry
		RpgAssetInfo assetInfo;
		if (!IsAssetRegistered(assetPath, &assetInfo))
		{
			RPG_CONSOLE_Error(RpgLogAsset, "Fail to load asset (%s). Asset not found in registry!", *assetPath);
			return RpgSharedAsset();
		}
		RPG_Check(assetPath == assetInfo.Path);

		RPG_Log(RpgLogAsset, "Loading asset (%s)", *assetPath);

		// create asset
		CreateAsset(asset, assetInfo.Class, assetInfo.Path);
		asset->SetAssetLoading();

		LoadingAssetTable.AddValue(assetPath, asset);

		RpgAssetTask_Loader* task = new RpgAssetTask_Loader();
		task->Asset = asset;
		LoadingAssetTasks.AddValue(task);

		RpgThreadPool::SubmitTasks(reinterpret_cast<RpgThreadTask**>(&task), 1);
	}

	// read external asset references
	RpgBinaryStreamReader reader = RpgAssetStream::CreateReaderForAsset(asset.Get());

	RpgAssetFileHeader header;
	RpgAssetReferences assetRefs;
	RpgAssetStream::ReadHeaderAndAssetReferences(reader, asset.Get(), header, assetRefs);

	for (const RpgString& ref : assetRefs)
	{
		LoadAssetAsync(ref);
	}

	return asset;
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
