#include "RpgMesh.h"



RpgMesh::RpgMesh(const RpgName& name) noexcept
{
	Name = name;
	Flags = FLAG_None;
	InitializeSRWLock(&LockPosition);
	InitializeSRWLock(&LockNormalTangent);
	InitializeSRWLock(&LockTexCoord);
	InitializeSRWLock(&LockSkin);
	InitializeSRWLock(&LockIndex);
}


void RpgMesh::UpdateVertexData(int vertexCount, const RpgVertex::FMeshPosition* positionData, const RpgVertex::FMeshNormalTangent* normalTangentData, const RpgVertex::FMeshTexCoord* texCoordData, const RpgVertex::FMeshSkin* skinData, int indexCount, const RpgVertex::FIndex* indexData) noexcept
{
	RPG_Assert(vertexCount > 0);
	RPG_Assert(positionData);
	RPG_Assert(indexCount > 0);
	RPG_Assert(indexData);

	Flags = FLAG_None;

	WriteLockAll();
	{
		Positions.Clear(true);
		Positions.InsertAtRange(positionData, vertexCount, RPG_INDEX_LAST);
		Flags |= FLAG_Attribute_Position;

		NormalTangents.Clear(true);
		if (normalTangentData)
		{
			NormalTangents.InsertAtRange(normalTangentData, vertexCount, RPG_INDEX_LAST);
			Flags |= FLAG_Attribute_NormalTangent;
		}

		TexCoords.Clear(true);
		if (texCoordData)
		{
			TexCoords.InsertAtRange(texCoordData, vertexCount, RPG_INDEX_LAST);
			Flags |= FLAG_Attribute_TexCoord;
		}
		
		Skins.Clear(true);
		if (skinData)
		{
			Skins.InsertAtRange(skinData, vertexCount, RPG_INDEX_LAST);
			Flags |= FLAG_Attribute_Skin;
		}

		Indices.Clear(true);
		Indices.InsertAtRange(indexData, indexCount, RPG_INDEX_LAST);
		Flags |= FLAG_Attribute_Index;
	}
	WriteUnlockAll();

	UpdateBound();
}


void RpgMesh::AddBatchVertexData(int vertexCount, const RpgVertex::FMeshPosition* positionData, const RpgVertex::FMeshNormalTangent* normalTangentData, const RpgVertex::FMeshTexCoord* texCoordData, const RpgVertex::FMeshSkin* skinData, int indexCount, const RpgVertex::FIndex* indexData) noexcept
{
	RPG_Assert(vertexCount > 0);
	RPG_Assert(positionData);
	RPG_Assert(indexCount > 0);
	RPG_Assert(indexData);

	const uint32_t baseVertex = static_cast<uint32_t>(Positions.GetCount());
	if (baseVertex == 0)
	{
		UpdateVertexData(vertexCount, positionData, normalTangentData, texCoordData, skinData, indexCount, indexData);
		return;
	}

	if (baseVertex > 0)
	{
		if (Flags & FLAG_Attribute_NormalTangent)
		{
			RPG_CheckV(normalTangentData, "Vertex data added to batch must have normal-tangent data if original mesh contains normal-tangent data!");
		}
		else
		{
			RPG_CheckV(skinData == nullptr, "Vertex data added to batch must not contain normal-tangent data if original mesh does not have normal-tangent data!");
		}

		if (Flags & FLAG_Attribute_TexCoord)
		{
			RPG_CheckV(skinData, "Vertex data added to batch must have texcoord data if original mesh contains texcoord data!");
		}
		else
		{
			RPG_CheckV(skinData == nullptr, "Vertex data added to batch must not contain texcoord data if original mesh does not have texcoord data!");
		}

		if (Flags & FLAG_Attribute_Skin)
		{
			RPG_CheckV(skinData, "Vertex data added to batch must have skin data if original mesh contains skin data!");
		}
		else
		{
			RPG_CheckV(skinData == nullptr, "Vertex data added to batch must not contain skin data if original mesh does not have skin data!");
		}
	}

	WriteLockAll();
	{
		Positions.InsertAtRange(positionData, vertexCount, RPG_INDEX_LAST);
		NormalTangents.InsertAtRange(normalTangentData, vertexCount, RPG_INDEX_LAST);
		TexCoords.InsertAtRange(texCoordData, vertexCount, RPG_INDEX_LAST);

		if (skinData)
		{
			Skins.InsertAtRange(skinData, vertexCount, RPG_INDEX_LAST);
		}

		const int baseIndex = Indices.GetCount();
		Indices.InsertAtRange(indexData, indexCount, RPG_INDEX_LAST);

		if (baseVertex > 0)
		{
			RpgVertexGeometryFactory::UpdateBatchIndices(Indices, baseVertex, baseIndex, indexCount);
		}
	}
	WriteUnlockAll();

	UpdateBound();
}


void RpgMesh::UpdateBound() noexcept
{
	RPG_Check(!Positions.IsEmpty());

	Bound.Min = FLT_MAX;
	Bound.Max = -FLT_MAX;
	const int vertexCount = Positions.GetCount();

	for (int v = 0; v < vertexCount; ++v)
	{
		const RpgVector3 vec(Positions[v].ToVector3());
		Bound.Min = RpgVector3::Min(Bound.Min, vec);
		Bound.Max = RpgVector3::Max(Bound.Max, vec);
	}
}


RpgSharedMesh RpgMesh::s_CreateShared(const RpgName& name) noexcept
{
	return RpgSharedMesh(new RpgMesh(name));
}


size_t RpgMesh::s_CalculateAssetSizeBytes(const RpgSharedMesh& mesh) noexcept
{
	size_t totalSizeBytes = 0;

	// name
	totalSizeBytes += sizeof(RpgName);

	// vertex position
	totalSizeBytes += mesh->Positions.GetMemorySizeBytes_Allocated();

	// vertex normal-tangent
	totalSizeBytes += mesh->NormalTangents.GetMemorySizeBytes_Allocated();

	// vertex texcoord
	totalSizeBytes += mesh->TexCoords.GetMemorySizeBytes_Allocated();

	// vertex skin
	totalSizeBytes += mesh->Skins.GetMemorySizeBytes_Allocated();

	// vertex index
	totalSizeBytes += mesh->Indices.GetMemorySizeBytes_Allocated();

	// flags
	totalSizeBytes += sizeof(uint8_t);

	// bound
	totalSizeBytes += sizeof(RpgBoundingAABB);

	return totalSizeBytes;
}
