#pragma once

#include "RpgMesh.h"
#include "RpgMaterial.h"


// Maximum model meshes/materials 
#define RPG_MODEL_MAX_MESH	8

// Maximum model LOD count
#define RPG_MODEL_MAX_LOD	4



typedef RpgSharedPtr<class RpgModel> RpgSharedModel;

class RpgModel
{
	RPG_NOCOPY(RpgModel)

public:
	RpgModel(const RpgName& name) noexcept;
	~RpgModel() noexcept;

	int AddMeshEmpty() noexcept;
	int AddMesh(const RpgSharedMesh& mesh) noexcept;
	void UpdateBound() noexcept;


	inline int AddLod() noexcept
	{
		const int lodIndex = LodCount;
		RPG_Check(lodIndex >= 0 && lodIndex < RPG_MODEL_MAX_LOD);
		++LodCount;

		return lodIndex;
	}


	inline void SetMaterial(int meshIndex, const RpgSharedMaterial& material) noexcept
	{
		RPG_Check(meshIndex >= 0 && meshIndex < MeshCount);
		Materials[meshIndex] = material;
	}


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}

	inline int GetLodCount() const noexcept
	{
		return LodCount;
	}

	inline int GetMeshCount() const noexcept
	{
		return MeshCount;
	}

	inline bool HasSkin() const noexcept
	{
		return Meshes[0][0]->HasSkin();
	}

	inline RpgBoundingAABB GetBound() const noexcept
	{
		return Bound;
	}


	inline RpgSharedMesh& GetMeshLod(int meshIndex, int lodIndex) noexcept
	{
		RPG_Check(meshIndex >= 0 && meshIndex < MeshCount);
		RPG_Check(lodIndex >= 0 && lodIndex < LodCount);

		return Meshes[meshIndex][lodIndex];
	}

	inline const RpgSharedMesh& GetMeshLod(int meshIndex, int lodIndex) const noexcept
	{
		RPG_Check(meshIndex >= 0 && meshIndex < MeshCount);
		RPG_Check(lodIndex >= 0 && lodIndex < LodCount);

		return Meshes[meshIndex][lodIndex];
	}


	inline const RpgSharedMaterial& GetMaterial(int meshIndex) const noexcept
	{
		RPG_Check(meshIndex >= 0 && meshIndex < MeshCount);

		return Materials[meshIndex];
	}


private:
	RpgName Name;
	int MeshCount;
	int LodCount;
	RpgBoundingAABB Bound;

	RpgSharedMesh Meshes[RPG_MODEL_MAX_MESH][RPG_MODEL_MAX_LOD];
	RpgSharedMaterial Materials[RPG_MODEL_MAX_MESH];


public:
	[[nodiscard]] static RpgSharedModel s_CreateShared(const RpgName& name) noexcept;

};
