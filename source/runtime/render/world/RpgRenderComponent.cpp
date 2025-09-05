#include "RpgRenderComponent.h"
#include "asset/RpgAssetManager.h"



RPG_COMPONENT_DEFINITION_STATIC_StreamWrite(RpgRenderComponent_Mesh)
{
	writer.Write(g_AssetManager->GetMeshAssetPath(data.Mesh));
	writer.Write(g_AssetManager->GetMaterialAssetPath(data.Material));
	writer.Write(data.bIsVisible);
	writer.Write(data.Bound);
}


RPG_COMPONENT_DEFINITION_STATIC_StreamRead(RpgRenderComponent_Mesh)
{
	RpgString meshAssetPath;
	reader.Read(meshAssetPath);
	data.Mesh = g_AssetManager->LoadMesh(meshAssetPath);

	RpgString materialAssetPath;
	reader.Read(materialAssetPath);
	data.Material = g_AssetManager->LoadMaterial(materialAssetPath);

	reader.Read(data.bIsVisible);
	reader.Read(data.Bound);
}



RPG_COMPONENT_DEFINITION_STATIC_StreamWrite(RpgRenderComponent_Light)
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


RPG_COMPONENT_DEFINITION_STATIC_StreamRead(RpgRenderComponent_Light)
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



RPG_COMPONENT_DEFINITION_STATIC_StreamWrite(RpgRenderComponent_Camera)
{
	writer.Write(data.RenderTargetDimension);
	writer.Write(data.ProjectionMode);
	writer.Write(data.PerspectiveFoVDegree);
	writer.Write(data.NearClipZ);
	writer.Write(data.FarClipZ);
	writer.Write(data.bActivated);
	writer.Write(data.bFrustumCulling);
}


RPG_COMPONENT_DEFINITION_STATIC_StreamRead(RpgRenderComponent_Camera)
{
	reader.Read(data.RenderTargetDimension);
	reader.Read(data.ProjectionMode);
	reader.Read(data.PerspectiveFoVDegree);
	reader.Read(data.NearClipZ);
	reader.Read(data.FarClipZ);
	reader.Read(data.bActivated);
	reader.Read(data.bFrustumCulling);
}
