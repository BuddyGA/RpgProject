#include "RpgRenderComponent.h"
#include "core/asset/RpgAssetSystem.h"



RPG_COMPONENT_STATIC_StreamWrite(RpgRenderComponent_Camera)
{
	writer.Write(data.RenderTargetDimension);
	writer.Write(data.ProjectionMode);
	writer.Write(data.PerspectiveFoVDegree);
	writer.Write(data.NearClipZ);
	writer.Write(data.FarClipZ);
	writer.Write(data.bActivated);
	writer.Write(data.bFrustumCulling);
}


RPG_COMPONENT_STATIC_StreamRead(RpgRenderComponent_Camera)
{
	reader.Read(data.RenderTargetDimension);
	reader.Read(data.ProjectionMode);
	reader.Read(data.PerspectiveFoVDegree);
	reader.Read(data.NearClipZ);
	reader.Read(data.FarClipZ);
	reader.Read(data.bActivated);
	reader.Read(data.bFrustumCulling);
}


RPG_COMPONENT_STATIC_GetExternalAssetReferences(RpgRenderComponent_Camera)
{
	// no external asset references
}


RPG_COMPONENT_STATIC_IsLoaded(RpgRenderComponent_Camera)
{
	// no external asset references
	return true;
}




RPG_COMPONENT_STATIC_StreamWrite(RpgRenderComponent_Mesh)
{
	writer.Write(data.Mesh ? data.Mesh->GetAssetPath() : RpgString());
	writer.Write(data.Material ? data.Material->GetAssetPath() : RpgString());
	writer.Write(data.bIsVisible);
	writer.Write(data.Bound);
}


RPG_COMPONENT_STATIC_StreamRead(RpgRenderComponent_Mesh)
{
	RpgString tempAssetPath;

	reader.Read(tempAssetPath);
	if (!tempAssetPath.IsEmpty())
	{
		data.Mesh = g_AssetSystem->LoadAssetAsync<RpgMesh>(tempAssetPath);
	}

	reader.Read(tempAssetPath);
	if (!tempAssetPath.IsEmpty())
	{
		data.Material = g_AssetSystem->LoadAssetAsync<RpgMaterial>(tempAssetPath);
	}

	reader.Read(data.bIsVisible);
	reader.Read(data.Bound);
}


RPG_COMPONENT_STATIC_GetExternalAssetReferences(RpgRenderComponent_Mesh)
{
	if (data.Mesh)
	{
		data.Mesh->GetExternalAssetReferences(out_AssetRefs);
		out_AssetRefs.Add(data.Mesh->GetAssetPath());
	}

	if (data.Material)
	{
		data.Material->GetExternalAssetReferences(out_AssetRefs);
		out_AssetRefs.Add(data.Material->GetAssetPath());
	}
}


RPG_COMPONENT_STATIC_IsLoaded(RpgRenderComponent_Mesh)
{
	if (!data.Mesh)
	{
		return false;
	}

	if (!data.Material)
	{
		return false;
	}

	return data.Mesh->IsAssetLoaded() && data.Material->IsAssetLoaded();
}




RPG_COMPONENT_STATIC_StreamWrite(RpgRenderComponent_Light)
{
	writer.Write(data.Type);
	writer.Write(data.ColorIntensity);
	writer.Write(data.AttenuationRadius);
	writer.Write(data.AttenuationFallOffExp);
	writer.Write(data.SpotInnerConeDegree);
	writer.Write(data.SpotOuterConeDegree);
	writer.Write(data.bCastShadow);
	writer.Write(data.bIsVisible);
}


RPG_COMPONENT_STATIC_StreamRead(RpgRenderComponent_Light)
{
	reader.Read(data.Type);
	reader.Read(data.ColorIntensity);
	reader.Read(data.AttenuationRadius);
	reader.Read(data.AttenuationFallOffExp);
	reader.Read(data.SpotInnerConeDegree);
	reader.Read(data.SpotOuterConeDegree);
	reader.Read(data.bCastShadow);
	reader.Read(data.bIsVisible);
}


RPG_COMPONENT_STATIC_GetExternalAssetReferences(RpgRenderComponent_Light)
{
	// no external asset references
}


RPG_COMPONENT_STATIC_IsLoaded(RpgRenderComponent_Light)
{
	// no external asset references
	return true;
}




RPG_COMPONENT_STATIC_StreamWrite(RpgRenderComponent_Terrain)
{
	/*
	for (int i = 0; i < 4; ++i)
	{
		writer.Write(data.SplatTextures[i] ? data.SplatTextures[i]->GetAssetPath() : RpgString());
	}
	*/

	writer.Write(data.Material ? data.Material->GetAssetPath() : RpgString());
	writer.Write(data.bIsVisible);

	writer.Write(data.WorldSize);

	const int tileCount = data.Tiles.GetCount();
	writer.Write(tileCount);

	for (int i = 0; i < tileCount; ++i)
	{
		const FTile& tt = data.Tiles[i];
		writer.Write(tt.VertexIndices);
		writer.Write(tt.Bound);
	}

	writer.Write(data.VertexPositions);
	writer.Write(data.VertexNormalTangents);
}


RPG_COMPONENT_STATIC_StreamRead(RpgRenderComponent_Terrain)
{
	RpgString tempAssetPath;

	/*
	for (int i = 0; i < 4; ++i)
	{
		reader.Read(tempAssetPath);
		if (!tempAssetPath.IsEmpty())
		{
			data.SplatTextures[i] = g_AssetSystem->LoadAssetAsync<RpgTexture2D>(tempAssetPath);
		}
	}
	*/
	reader.Read(tempAssetPath);
	if (!tempAssetPath.IsEmpty())
	{
		data.Material = g_AssetSystem->LoadAssetAsync<RpgMaterial>(tempAssetPath);
	}

	reader.Read(data.bIsVisible);

	reader.Read(data.WorldSize);

	int tileCount = 0;
	reader.Read(tileCount);
	data.Tiles.Resize(tileCount);

	for (int i = 0; i < tileCount; ++i)
	{
		FTile& tt = data.Tiles[i];
		reader.Read(tt.VertexIndices);
		reader.Read(tt.Bound);
	}

	reader.Read(data.VertexPositions);
	reader.Read(data.VertexNormalTangents);
}


RPG_COMPONENT_STATIC_GetExternalAssetReferences(RpgRenderComponent_Terrain)
{
	/*
	for (int i = 0; i < 4; ++i)
	{
		if (data.SplatTextures[i])
		{
			data.SplatTextures[i]->GetExternalAssetReferences(out_AssetRefs);
			out_AssetRefs.Add(data.SplatTextures[i]->GetAssetPath());
		}
	}
	*/

	if (data.Material)
	{
		data.Material->GetExternalAssetReferences(out_AssetRefs);
		out_AssetRefs.Add(data.Material->GetAssetPath());
	}
}


RPG_COMPONENT_STATIC_IsLoaded(RpgRenderComponent_Terrain)
{
	/*
	bool bAllTexturesLoaded = true;

	for (int i = 0; i < 4; ++i)
	{
		if (data.SplatTextures[i] && !data.SplatTextures[i]->IsAssetLoaded())
		{
			bAllTexturesLoaded = false;
			break;
		}
	}

	return bAllTexturesLoaded;
	*/

	if (!data.Material)
	{
		return false;
	}

	return data.Material->IsAssetLoaded();
}


void RpgRenderComponent_Terrain::Generate(float in_WorldSize) noexcept
{
	VertexPositions.Clear(true);
	VertexNormalTangents.Clear(true);
	VertexTexCoords.Clear(true);
	Tiles.Clear(true);
	WorldSize = RpgMath::Clamp(in_WorldSize, RPG_RENDER_TERRAIN_WORLD_SIZE_MIN, RPG_RENDER_TERRAIN_WORLD_SIZE_MAX);
	
	const int tileCountPerAxis = static_cast<int>(WorldSize / RPG_RENDER_TERRAIN_TILE_SIZE);
	const int tileCountTotal = tileCountPerAxis * tileCountPerAxis;
	const int vertexCountPerAxis = tileCountPerAxis * 2;
	const int vertexCountTotal = vertexCountPerAxis * vertexCountPerAxis;

	VertexPositions.Reserve(vertexCountTotal);
	VertexNormalTangents.Reserve(vertexCountTotal);
	VertexTexCoords.Reserve(vertexCountTotal);
	Tiles.Reserve(tileCountTotal);

	// Start at left-top with terrain center at coord (0, y, 0)
	const float terrainHalfSize = WorldSize * 0.5f;
	RpgVector3 vertexPos(-terrainHalfSize, 0.0f, terrainHalfSize);

	for (int tz = 0; tz < tileCountPerAxis; ++tz)
	{
		for (int tx = 0; tx < tileCountPerAxis; ++tx)
		{
			const uint32_t vid = static_cast<uint32_t>(VertexPositions.GetCount());

			// vertex position
			VertexPositions.AddValue(vertexPos);
			VertexPositions.AddValue(vertexPos + RpgVector3(RPG_RENDER_TERRAIN_TILE_SIZE, 0.0f, 0.0f));
			VertexPositions.AddValue(vertexPos + RpgVector3(RPG_RENDER_TERRAIN_TILE_SIZE, 0.0f, -RPG_RENDER_TERRAIN_TILE_SIZE));
			VertexPositions.AddValue(vertexPos + RpgVector3(0.0f, 0.0f, -RPG_RENDER_TERRAIN_TILE_SIZE));

			// vertex normal-tangent
			for (int i = 0; i < 4; ++i)
			{
				RpgVertex::FMeshNormalTangent& nt = VertexNormalTangents.Add();
				nt.Normal = RpgVector4(0.0f, 1.0f, 0.0f, 0.0f);
				nt.Tangent = RpgVector4(0.0f, 1.0f, 0.0f, 0.0f);
			}
			
			// vertex tex-coord
			VertexTexCoords.AddValue({ 0.0f, 0.0f });
			VertexTexCoords.AddValue({ 1.0f, 0.0f });
			VertexTexCoords.AddValue({ 1.0f, 1.0f });
			VertexTexCoords.AddValue({ 0.0f, 1.0f });

			const uint32_t tid = Tiles.GetCount();
			FTile& tt = Tiles.Add();
			tt.VertexIndices = { vid, vid + 1, vid + 2, vid + 2, vid + 3, vid };
			UpdateTileBound(tid);

			vertexPos.X += RPG_RENDER_TERRAIN_TILE_SIZE;
		}

		vertexPos.X = -terrainHalfSize;
		vertexPos.Z -= RPG_RENDER_TERRAIN_TILE_SIZE;
	}

	RPG_Assert(VertexPositions.GetCount() == vertexCountTotal);
	RPG_Assert(VertexNormalTangents.GetCount() == vertexCountTotal);
	RPG_Assert(VertexTexCoords.GetCount() == vertexCountTotal);
	RPG_Assert(Tiles.GetCount() == tileCountTotal);
}


const RpgVertexIndexArray& RpgRenderComponent_Terrain::GetVertexIndices(const RpgBoundingFrustum* frustum) noexcept
{
	VertexIndices.Clear();
	
	for (int i = 0; i < Tiles.GetCount(); ++i)
	{
		const FTile& tt = Tiles[i];

		if (frustum == nullptr || frustum->TestIntersectAABB(tt.Bound))
		{
			VertexIndices.InsertAtRange(tt.VertexIndices.GetData(), tt.VertexIndices.GetCount(), RPG_INDEX_LAST);
		}
	}

	return VertexIndices;
}


void RpgRenderComponent_Terrain::UpdateTileBound(int index) noexcept
{
	FTile& tt = Tiles[index];
	tt.Bound.Min = FLT_MAX;
	tt.Bound.Max = -FLT_MAX;

	for (int i = 0; i < 6; ++i)
	{
		const RpgVector3 vp = VertexPositions[tt.VertexIndices[i]].ToVector3();
		tt.Bound.Min = RpgVector3::Min(tt.Bound.Min, vp);
		tt.Bound.Max = RpgVector3::Max(tt.Bound.Max, vp);
	}
}
