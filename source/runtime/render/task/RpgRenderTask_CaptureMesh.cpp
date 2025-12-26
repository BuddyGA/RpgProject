#include "RpgRenderTask_Capture.h"
#include "core/world/RpgWorld.h"
#include "../world/RpgRenderComponent.h"



RpgRenderTask_CaptureMesh::RpgRenderTask_CaptureMesh() noexcept
{
	World = nullptr;
	Camera = nullptr;
	FrameIndex = 0;
}


void RpgRenderTask_CaptureMesh::Reset() noexcept
{
	RpgThreadTask::Reset();

	World = nullptr;
	Camera = nullptr;
	FrameIndex = 0;
}


void RpgRenderTask_CaptureMesh::Execute() noexcept
{
	RPG_Assert(World);
	RPG_Assert(Camera);

	const bool bFrustumCulling = Camera->bFrustumCulling;
	RpgSceneViewport* viewport = Camera->GetSceneViewport();
	const RpgBoundingFrustum frustum = viewport->GetViewFrustum();

	// capture meshes
	RpgArray<RpgSceneMesh>& sceneMeshes = viewport->GetFrameMeshes(FrameIndex);
	sceneMeshes.Clear();

	for (auto it = World->ComponentIterator<RpgRenderComponent_Mesh>(); it; ++it)
	{
		const RpgRenderComponent_Mesh& comp = it.GetValue();

		// - check if spawned
		// - check valid model
		// - check visibility
		// - if frustum culling enabled, test bound againts frustum
		if (!comp.IsGameObjectSpawned() || !comp.Mesh || !comp.bIsVisible || (bFrustumCulling && !frustum.TestIntersectAABB(comp.GetBound())) )
		{
			continue;
		}

		const RpgGameObject gameObject = comp.GetGameObject();

		RpgSceneMesh& data = sceneMeshes.Add();
		data.GameObject = gameObject;
		data.WorldTransformMatrix = gameObject.GetWorldTransformMatrix();
		data.Material = comp.Material;
		data.Mesh = comp.Mesh;

		// TODO: Determine LOD level based on distance from the camera

		data.Lod = 0;
	}

	
	// capture terrains
	RpgArray<RpgSceneTerrain>& sceneTerrains = viewport->GetFrameTerrains(FrameIndex);
	sceneTerrains.Clear();

	for (auto it = World->ComponentIterator<RpgRenderComponent_Terrain>(); it; ++it)
	{
		RpgRenderComponent_Terrain& comp = it.GetValue();

		// - check if spawned
		// - check if visible
		// - check if vertex generated
		if (!comp.IsGameObjectSpawned() || !comp.bIsVisible || !comp.HasGenerated())
		{
			continue;
		}

		const RpgGameObject gameObject = comp.GetGameObject();

		RpgSceneTerrain& data = sceneTerrains.Add();
		data.GameObject = gameObject;
		data.WorldTransformMatrix = gameObject.GetWorldTransformMatrix();
		data.Material = comp.Material;
		data.VertexPositions = &comp.GetVertexPositions();
		data.VertexNormalTangents = &comp.GetVertexNormalTangents();
		data.VertexTexCoords = &comp.GetVertexTexCoords();
		data.VertexIndices = &comp.GetVertexIndices(&frustum);
	}
}
