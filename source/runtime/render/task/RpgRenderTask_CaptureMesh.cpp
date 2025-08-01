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

	RpgArray<RpgSceneMesh>& sceneMeshes = viewport->GetFrameMeshes(FrameIndex);
	sceneMeshes.Clear();

	for (auto it = World->Component_CreateConstIterator<RpgRenderComponent_Mesh>(); it; ++it)
	{
		const RpgRenderComponent_Mesh& comp = it.GetValue();

		// - check valid model
		// - check visibility
		// - if frustum culling enabled, test bound againts frustum
		if (!comp.Mesh || !comp.bIsVisible || (bFrustumCulling && !frustum.TestIntersectAABB(comp.Bound)) )
		{
			continue;
		}

		const RpgMatrixTransform worldTransformMatrix = World->GameObject_GetWorldTransformMatrix(comp.GameObject);

		RpgSceneMesh& data = sceneMeshes.Add();
		data.GameObject = comp.GameObject;
		data.WorldTransformMatrix = worldTransformMatrix;
		data.Material = comp.Material;
		data.Mesh = comp.Mesh;

		// TODO: Determine LOD level based on distance from the camera

		data.Lod = 0;
	}
}
