#pragma once

#include "core/RpgFilePath.h"
#include "render/RpgModel.h"
#include "thirdparty/xxhash/xxhash.h"
#include "RpgAssetTypes.h"



template<typename T>
class RpgAssetLoadedData
{
public:
	RpgAssetLoadedData() noexcept = default;


	void RemoveUnreferenced() noexcept
	{
		// if this is the last reference (SharedRefCount == 1), that means no other referencing it, so remove it
		for (int i = 0; i < Shareds.GetCount();)
		{
			const RpgSharedPtr<T>& check = Shareds[i];

			if (check.GetRefCount() == 1)
			{
				Shareds.RemoveAt(i);
				Hashes.RemoveAt(i);
			}
			else
			{
				++i;
			}
		}
	}


	inline int Add(uint64_t in_Hash, const RpgSharedPtr<T>& in_Shared) noexcept
	{
		RPG_Check(Hashes.FindIndexByValue(in_Hash) == RPG_INDEX_INVALID);
		const int index = Hashes.GetCount();
		Hashes.AddValue(in_Hash);
		Shareds.AddValue(in_Shared);

		return index;
	}


	inline bool IsLoaded(uint64_t hash, int* optOut_Index = nullptr) const noexcept
	{
		const int index = Hashes.FindIndexByValue(hash);

		if (optOut_Index)
		{
			*optOut_Index = index;
		}

		return index != RPG_INDEX_INVALID;
	}


	inline uint64_t GetHashByShared(const RpgSharedPtr<T>& ref) const noexcept
	{
		const int index = Shareds.FindIndexByValue(ref);
		return Hashes[index];
	}

	inline const RpgSharedPtr<T>& GetSharedAtIndex(int index) const noexcept
	{
		return Shareds[index];
	}


private:
	RpgArray<uint64_t, 8> Hashes;
	RpgArray<RpgSharedPtr<T>, 8> Shareds;

};




extern class RpgAssetManager* g_AssetManager;

class RpgAssetManager
{
	RPG_NOCOPYMOVE(RpgAssetManager)

public:
	RpgAssetManager() noexcept;
	void Update() noexcept;

	// Check if file is a valid asset file
	// @param filePath - Path to a file
	// @param optOut_AssetInfo - (Optional) output asset info if file is valid
	// @return TRUE if file is valid
	bool IsValidAssetFile(const RpgFilePath& filePath, RpgAssetInfo* optOut_AssetInfo = nullptr) noexcept;

	// Scan all asset files in filesystem asset directory and try register them
	void ScanAssetFiles() noexcept;

	// Try register file as asset file
	// @param filePath - Path to a file
	// @return TRUE if file is valid asset file and added to registry
	bool RegisterAssetFile(const RpgFilePath& filePath) noexcept;

	// Save mesh to asset file
	// @param mesh - Shared ptr to a mesh asset
	void SaveMesh(const RpgSharedMesh& mesh) noexcept;

	// Load mesh from asset file
	// @param filePath - Path to a mesh asset file
	// @return SharedPtr to a mesh asset, NULL SharedPtr if file is not a valid mesh asset file
	RpgSharedMesh LoadMesh(const RpgFilePath& filePath) noexcept;

	// Get asset info from registry
	// @param filePath - Path to a file
	// @return Pointer to asset info, nullptr if file not found in registry
	inline const RpgAssetInfo* GetAssetInfoByFilePath(const RpgFilePath& filePath) const noexcept
	{
		return GetAssetInfoByHash(XXH3_64bits(*filePath, filePath.GetLength()));
	}


private:
	inline const RpgAssetInfo* GetAssetInfoByHash(uint64_t hash) const noexcept
	{
		const int index = RegisteredAssetHashes.FindIndexByValue(hash);
		return index == RPG_INDEX_INVALID ? nullptr : &RegisteredAssetInfos[index];
	}


private:
	RpgArray<uint64_t> RegisteredAssetHashes;
	RpgArray<RpgAssetInfo> RegisteredAssetInfos;
	
	// Loaded mesh data
	RpgUniquePtr<RpgAssetLoadedData<RpgMesh>> LoadedMeshData;

	// Loaded material data
	RpgUniquePtr<RpgAssetLoadedData<RpgMaterial>> LoadedMaterialData;

	// Loaded texture data (not part of texture streaming system, ex: UI)
	RpgUniquePtr<RpgAssetLoadedData<RpgTexture2D>> LoadedTextureData;

};
