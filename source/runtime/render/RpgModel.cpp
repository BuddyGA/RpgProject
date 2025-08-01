#include "RpgModel.h"



RpgModel::RpgModel(const RpgName& name) noexcept
{
	RPG_Log(RpgLogTemp, "Create model (%s)", *name);

	Name = name;
	MeshCount = 0;
	LodCount = 0;
}


RpgModel::~RpgModel() noexcept
{
	//RPG_PLATFORM_Log(RpgLogTemp, "Destroy model (%s)", *Name);
}


int RpgModel::AddMeshEmpty() noexcept
{
	RPG_Check(MeshCount < RPG_MODEL_MAX_MESH);

	const int meshIndex = MeshCount++;

	for (int l = 0; l < LodCount; ++l)
	{
		Meshes[meshIndex][l] = RpgMesh::s_CreateShared(RpgName::Format("%s_mesh%i_lod%i", *Name, meshIndex, l));
	}
	
	return meshIndex;
}


int RpgModel::AddMesh(const RpgSharedMesh& mesh) noexcept
{
	RPG_Check(MeshCount < RPG_MODEL_MAX_MESH);

	const int meshIndex = MeshCount++;
	
	for (int l = 0; l < LodCount; ++l)
	{
		Meshes[meshIndex][l] = mesh;
	}

	UpdateBound();

	return meshIndex;
}


void RpgModel::UpdateBound() noexcept
{
	Bound.Min = FLT_MAX;
	Bound.Max = -FLT_MAX;

	for (int m = 0; m < MeshCount; ++m)
	{
		const RpgBoundingAABB meshBound = Meshes[m][0]->GetBound();
		Bound.Min = RpgVector3::Min(Bound.Min, meshBound.Min);
		Bound.Max = RpgVector3::Max(Bound.Max, meshBound.Max);
	}
}



RpgSharedModel RpgModel::s_CreateShared(const RpgName& name) noexcept
{
	return RpgSharedModel(new RpgModel(name));
}
