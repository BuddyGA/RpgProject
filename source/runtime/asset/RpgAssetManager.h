#pragma once

#include "core/RpgFilePath.h"
#include "render/RpgMesh.h"
#include "render/RpgMaterial.h"
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
		RPG_Check(Shareds.FindIndexByValue(in_Shared) == RPG_INDEX_INVALID);

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
		return index == RPG_INDEX_INVALID ? 0 : Hashes[index];
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

	// Initialize asset manager
	void Initialize() noexcept;

	// Update asset loading and try to unload if no other referencing it
	void Update() noexcept;

	// Check if file is a valid asset file
	// @param filePath - Absolute path to a file
	// @param optOut_AssetInfo - (Optional) output asset info if file is valid
	// @return TRUE if file is valid
	bool IsValidAssetFile(const RpgFilePath& filePath, RpgAssetInfo* optOut_AssetInfo = nullptr) noexcept;

	// Scan all asset files in filesystem asset directory and try register them
	void ScanAssetFiles() noexcept;


	// Save mesh to asset file
	// @param mesh - Mesh shared ptr
	void SaveMesh(const RpgSharedMesh& mesh, const char* directory) noexcept;

	// Load mesh from asset registry
	// @param path - Path to mesh asset (relative to asset directory)
	// @return SharedPtr to mesh, NULL SharedPtr if asset not found
	RpgSharedMesh LoadMesh(const RpgString& path) noexcept;

	// Get mesh asset path
	// @param mesh - Mesh shared ptr
	// @return Mesh asset path in registry
	inline RpgString GetMeshAssetPath(const RpgSharedMesh& mesh) const noexcept
	{
		const uint64_t hash = LoadedMeshData.GetHashByShared(mesh);
		if (hash == 0)
		{
			return RpgString();
		}

		const RpgAssetInfo* info = GetAssetInfoByHash(hash);
		RPG_Check(info && info->Type == RpgAssetFileType::MESH);

		return info->Path;
	}


	// Save texture to asset file
	// @param texture - Texture shared ptr
	void SaveTexture(const RpgSharedTexture2D& texture, const char* directory) noexcept;

	// Load texture from asset registry
	// @param path - Path to texture asset (relative to asset directory)
	// @return SharedPtr to texture, NULL SharedPtr if asset not found
	RpgSharedTexture2D LoadTexture(const RpgString& path) noexcept;

	// Get texture asset path
	// @param texture - Texture shared ptr
	// @return Texture asset path in registry
	inline RpgString GetTextureAssetPath(const RpgSharedTexture2D& texture) const noexcept
	{
		const uint64_t hash = LoadedTextureData.GetHashByShared(texture);
		if (hash == 0)
		{
			return RpgString();
		}

		const RpgAssetInfo* info = GetAssetInfoByHash(hash);
		RPG_Check(info && info->Type == RpgAssetFileType::TEXTURE);

		return info->Path;
	}


	// Save material to asset file
	// @param material - Material shared ptr
	void SaveMaterial(const RpgSharedMaterial& material, const char* directory) noexcept;

	// Load material from asset registry
	// @param path - Path to material asset (relative to asset directory)
	// @return SharedPtr to material, NULL SharedPtr if asset not found
	RpgSharedMaterial LoadMaterial(const RpgString& path) noexcept;

	// Get material asset path
	// @param material - Material shared ptr
	// @return Material asset path in registry
	inline RpgString GetMaterialAssetPath(const RpgSharedMaterial& material) const noexcept
	{
		const uint64_t hash = LoadedMaterialData.GetHashByShared(material);
		if (hash == 0)
		{
			return RpgString();
		}

		const RpgAssetInfo* info = GetAssetInfoByHash(hash);
		RPG_Check(info && info->Type == RpgAssetFileType::MATERIAL);

		return info->Path;
	}


	// Get asset info from registry
	// @param filePath - Absolute path to a file
	// @return Pointer to asset info, nullptr if file not found in registry
	inline const RpgAssetInfo* GetAssetInfoByFilePath(const RpgFilePath& filePath) const noexcept
	{
		return GetAssetInfoByHash(XXH3_64bits(*filePath, filePath.GetLength()));
	}


private:
	// Try register file as asset file
	// @param filePath - Absolute path to an asset file
	// @param optOut_Hash - (optional) Asset hash
	// @return TRUE if file is valid asset file and added to registry
	bool RegisterAssetFile(const RpgFilePath& filePath, uint64_t* optOut_Hash = nullptr) noexcept;


	// Get asset path 
	// @param filePath - Absolute path to an asset file
	// @return Relative asset path
	RpgString GetAssetPath(const RpgFilePath& filePath) const noexcept;


	// Get asset info by asset hash from registry
	// @param hash - Asset path hash
	// @return Pointer to RpgAssetInfo if found in registry or NULL if not found
	inline const RpgAssetInfo* GetAssetInfoByHash(uint64_t hash) const noexcept
	{
		const int index = RegisteredAssetHashes.FindIndexByValue(hash);
		return index == RPG_INDEX_INVALID ? nullptr : &RegisteredAssetInfos[index];
	}


private:
	// Asset hash registry
	RpgArray<uint64_t> RegisteredAssetHashes;

	// Asset info registry
	RpgArray<RpgAssetInfo> RegisteredAssetInfos;
	
	// Loaded mesh data
	RpgAssetLoadedData<RpgMesh> LoadedMeshData;

	// Loaded texture data (not part of texture streaming system, ex: UI)
	RpgAssetLoadedData<RpgTexture2D> LoadedTextureData;

	// Loaded material data
	RpgAssetLoadedData<RpgMaterial> LoadedMaterialData;

};
