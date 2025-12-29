#pragma once

#include "render/asset/RpgMesh.h"
#include "render/asset/RpgMaterial.h"


// Maximum model meshes/materials 
#define RPG_EDITOR_MODEL_MAX_MESH	8

// Maximum model LOD count
#define RPG_EDITOR_MODEL_MAX_LOD	4



typedef RpgSharedPtr<class RpgEditorModel> RpgSharedEditorModel;

class RpgEditorModel
{
	RPG_NOCOPY(RpgEditorModel)

public:
	RpgEditorModel(const RpgName& name) noexcept;
	~RpgEditorModel() noexcept;

	int AddMeshEmpty() noexcept;
	int AddMesh(const RpgSharedMesh& mesh) noexcept;
	void UpdateBound() noexcept;


	inline int AddLod() noexcept
	{
		const int lodIndex = LodCount;
		RPG_Check(lodIndex >= 0 && lodIndex < RPG_EDITOR_MODEL_MAX_LOD);
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

	RpgSharedMesh Meshes[RPG_EDITOR_MODEL_MAX_MESH][RPG_EDITOR_MODEL_MAX_LOD];
	RpgSharedMaterial Materials[RPG_EDITOR_MODEL_MAX_MESH];

};
