#include "RpgRenderComponent.h"
#include "core/asset/RpgAssetSystem.h"



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
		data.Mesh = g_AssetSystem->LoadAsset<RpgMesh>(tempAssetPath);
	}

	reader.Read(tempAssetPath);
	if (!tempAssetPath.IsEmpty())
	{
		data.Material = g_AssetSystem->LoadAsset<RpgMaterial>(tempAssetPath);
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
