#include "RpgRenderTask_Capture.h"
#include "core/world/RpgWorld.h"
#include "../world/RpgRenderComponent.h"



RpgRenderTask_CaptureMesh::RpgRenderTask_CaptureMesh() noexcept
{
	World = nullptr;
	Camera = nullptr;
}


void RpgRenderTask_CaptureMesh::Reset() noexcept
{
	RpgThreadTask::Reset();

	World = nullptr;
	Camera = nullptr;
}


void RpgRenderTask_CaptureMesh::Execute() noexcept
{
	RPG_Assert(World);
	RPG_Assert(Camera);

	const bool bFrustumCulling = Camera->bFrustumCulling;
	RpgSceneViewport* viewport = Camera->GetSceneViewport();
	const RpgBoundingFrustum frustum = viewport->GetViewFrustum();
	viewport->Meshes.Clear();

	for (auto it = World->Component_CreateConstIterator<RpgRenderComponent_Mesh>(); it; ++it)
	{
		const RpgRenderComponent_Mesh& comp = it.GetValue();

		// - check valid model
		// - check visibility
		// - if frustum culling enabled, test bound againts frustum
		if (!comp.Model || !comp.bIsVisible || (bFrustumCulling && !frustum.TestIntersectAABB(comp.Bound)) )
		{
			continue;
		}

		const RpgMatrixTransform worldTransformMatrix = World->GameObject_GetWorldTransformMatrix(comp.GameObject);

		for (int m = 0; m < comp.Model->GetMeshCount(); ++m)
		{
			RpgSceneMesh& data = viewport->Meshes.Add();
			data.GameObject = comp.GameObject;
			data.WorldTransformMatrix = worldTransformMatrix;
			data.Material = comp.Model->GetMaterial(m);
			data.Mesh = comp.Model->GetMeshLod(m, 0);

			// TODO: Determine LOD level based on distance from the camera

			data.Lod = 0;
		}
	}
}
