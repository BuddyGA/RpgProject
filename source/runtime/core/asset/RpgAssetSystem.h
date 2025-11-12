#pragma once

#include "RpgAssetTypes.h"
#include "../RpgMap.h"
#include "../RpgConsoleSystem.h"



class RpgAssetSystem
{
	RPG_NOCOPYMOVE(RpgAssetSystem)

public:
	RpgAssetSystem() noexcept;
	~RpgAssetSystem() noexcept;


	// Register asset type into registry
	template<typename TAsset>
	inline void RegisterAssetClass() noexcept
	{
		const TAsset* defaultObject = TAsset::GetDefault();
		const RpgName className = defaultObject->GetAssetClassName();
		Registry.DefaultObjectTable.AddValue(className, defaultObject);
	}


	// Update asset loading and try to unload if no other referencing it
	void Update() noexcept;


	// Check if file is a valid asset file
	// @param filePath - Absolute path to a file
	// @param optOut_AssetInfo - (Optional) output asset info if file is valid
	// @return TRUE if file is valid
	bool IsValidAssetFile(const RpgFilePath& filePath, RpgAssetInfo* optOut_AssetInfo = nullptr) noexcept;


	// Scan all asset files in asset directory and try register them
	void ScanAssetFiles() noexcept;


	// Save asset
	// @param asset - Asset to save
	// @param dstFolder - Path relative to asset folder
	void SaveAsset(RpgSharedAsset asset, const char* dstFolder) noexcept;

	// Save asset
	// @param asset - Asset to save
	// @param directory - Path relative to asset folder
	template<typename TAsset>
	inline void SaveAsset(RpgSharedPtr<TAsset>& asset, const char* dstFolder) noexcept
	{
		SaveAsset(asset.CastStatic<RpgAssetObject>(), dstFolder);
	}


	// Load asset synchronously
	// @param assetPath - Asset path relative to asset directory
	// @return SharedPtr to asset
	RpgSharedAsset LoadAsset(const RpgString& assetPath) noexcept;

	// Load asset synchronously
	// @param path - Asset path relative to asset directory
	// @return SharedPtr to asset if path is valid and asset is loaded
	template<typename TAsset>
	inline RpgSharedPtr<TAsset> LoadAsset(const RpgString& assetPath) noexcept
	{
		static_assert(std::is_base_of<RpgAssetObject, TAsset>::value, "RpgAssetSystem::LoadAsset type of <TAsset> must be derived from type <RpgAssetObject>!");

		RpgSharedAsset asset = LoadAsset(assetPath);
		if (asset)
		{
			RPG_Check(asset->GetAssetPath().Equals(assetPath, true));
			RPG_Check(asset->GetAssetFileType() == TAsset::FILE_TYPE);
			return asset.CastDynamic<TAsset>();
		}

		return RpgSharedPtr<TAsset>();
	}


	// Load asset asynchronously. 
	// The return value is empty/unloaded, call IsAssetLoaded() to check if asset has fully loaded before using it
	// @param assetPath - Asset path relative to asset directory
	// @param out_Asset - Asset object result
	RpgSharedAsset LoadAssetAsync(const RpgString& assetPath) noexcept;

	// Load asset asynchronously. 
	// The return value is empty/unloaded, call IsAssetLoaded() to check if asset has fully loaded before using it
	// @param assetPath - Asset path relative to asset directory
	// @return Empty (unloaded) SharedPtr to asset
	template<typename TAsset>
	inline RpgSharedPtr<TAsset> LoadAssetAsync(const RpgString& assetPath)
	{
		static_assert(std::is_base_of<RpgAssetObject, TAsset>::value, "RpgAssetSystem::LoadAssetAsync type of <TAsset> must be derived from type <RpgAssetObject>!");

		RpgSharedAsset asset = LoadAssetAsync(assetPath);
		RPG_Check(asset);
		if (asset)
		{
			RPG_Check(asset->GetAssetPath().Equals(assetPath, true));
			RPG_Check(asset->GetAssetFileType() == TAsset::FILE_TYPE);
			return asset.CastDynamic<TAsset>();
		}

		return RpgSharedPtr<TAsset>();
	}


	// Check if asset path exists in registry
	// @param path - Asset path relative to asset directory
	// @param optOut_AssetInfo - (Optional) Asset info result if found in registry
	// @return TRUE if asset exists in registry, FALSE otherwise
	inline bool IsAssetRegistered(const RpgString& assetPath, RpgAssetInfo* optOut_AssetInfo = nullptr) const noexcept
	{
		const RpgAssetInfo* assetInfo = Registry.AssetInfoTable.FindValue(assetPath);
		if (assetInfo == nullptr)
		{
			return false;
		}

		RPG_Check(assetInfo->Path.Equals(assetPath, true));

		if (optOut_AssetInfo)
		{
			*optOut_AssetInfo = *assetInfo;
		}

		return true;
	}


	// Check if asset loaded
	// @param path - Asset path relative to asset directory
	// @return TRUE if asset loaded, FALSE otherwise
	inline bool IsAssetLoaded(const RpgString& assetPath) const noexcept
	{
		RPG_PLATFORM_ScopedLock(LoadedMutex);
		return LoadedAssetTable.Exists(assetPath);
	}


private:
	// Try register file as asset file
	// @param filePath - Absolute path to an asset file
	// @param optOut_Hash - (optional) Asset hash
	void RegisterAssetFile(const RpgFilePath& filePath, RpgString* optOut_AssetPath = nullptr) noexcept;


	// Get asset path 
	// @param filePath - Absolute path to an asset file
	// @return Relative asset path
	RpgString GetAssetPathFromFile(const RpgFilePath& filePath) const noexcept;


	inline void CreateAsset(RpgSharedAsset& out_Asset, const RpgName& className, const RpgString& assetPath) const noexcept
	{
		const RpgAssetObject* defaultObject = Registry.DefaultObjectTable[className];
		out_Asset = RpgSharedAsset(defaultObject->CreateAsset());
		out_Asset->Path = assetPath;
	}


	inline RpgSharedAsset FindLoadedAsset(const RpgString& assetPath) const noexcept
	{
		if (assetPath.IsEmpty())
		{
			return RpgSharedAsset();
		}

		RpgSharedAsset asset;

		RPG_PLATFORM_ScopedLock(LoadedMutex);

		int index = RPG_INDEX_INVALID;
		if (LoadedAssetTable.Exists(assetPath, &index))
		{
			asset = LoadedAssetTable.GetValueByIndex(index);
			RPG_Check(asset->GetAssetPath().Equals(assetPath, true));
		}

		return asset;
	}


	inline void AddLoadedAsset(const RpgString& assetPath, const RpgSharedAsset& asset) noexcept
	{
		RPG_PLATFORM_ScopedLock(LoadedMutex);
		LoadedAssetTable.AddValue(assetPath, asset);
	}


	inline void RemoveLoadedAsset(int index) noexcept
	{
		RPG_PLATFORM_ScopedLock(LoadedMutex);
		LoadedAssetTable.RemoveAt(index);
	}


private:
	// Asset registry
	struct FRegistry
	{
		// Class default objects
		RpgMap<RpgName, const RpgAssetObject*, 8> DefaultObjectTable;

		// Registered assets
		RpgMap<RpgString, RpgAssetInfo, 8> AssetInfoTable;
	};
	FRegistry Registry;


	// Loading assets
	RpgMap<RpgString, RpgSharedAsset, 8> LoadingAssetTable;
	RpgArray<RpgAssetTask_Loader*> LoadingAssetTasks;
	mutable RpgPlatformMutex LoadingMutex;

	// Loaded assets
	RpgMap<RpgString, RpgSharedAsset, 8> LoadedAssetTable;
	mutable RpgPlatformMutex LoadedMutex;

};
