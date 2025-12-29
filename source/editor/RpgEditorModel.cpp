#include "RpgEditorModel.h"



RpgEditorModel::RpgEditorModel(const RpgName& name) noexcept
{
	RPG_Log(RpgLogTemp, "Create model (%s)", *name);

	Name = name;
	MeshCount = 0;
	LodCount = 0;
}


RpgEditorModel::~RpgEditorModel() noexcept
{
	//RPG_PLATFORM_Log(RpgLogTemp, "Destroy model (%s)", *Name);
}


int RpgEditorModel::AddMeshEmpty() noexcept
{
	RPG_Check(MeshCount < RPG_EDITOR_MODEL_MAX_MESH);

	const int meshIndex = MeshCount++;

	for (int l = 0; l < LodCount; ++l)
	{
		Meshes[meshIndex][l] = RpgPointer::MakeShared<RpgMesh>(RpgName::Format("%s_mesh%i_lod%i", *Name, meshIndex, l));
	}

	return meshIndex;
}


int RpgEditorModel::AddMesh(const RpgSharedMesh& mesh) noexcept
{
	RPG_Check(MeshCount < RPG_EDITOR_MODEL_MAX_MESH);

	const int meshIndex = MeshCount++;

	for (int l = 0; l < LodCount; ++l)
	{
		Meshes[meshIndex][l] = mesh;
	}

	UpdateBound();

	return meshIndex;
}


void RpgEditorModel::UpdateBound() noexcept
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
