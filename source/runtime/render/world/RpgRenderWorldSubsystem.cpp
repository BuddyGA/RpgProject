#include "RpgRenderWorldSubsystem.h"
#include "RpgRenderComponent.h"
#include "../RpgRenderer.h"
#include "../task/RpgRenderTask_Capture.h"



RpgRenderWorldSubsystem::RpgRenderWorldSubsystem() noexcept
{
	Name = "RenderWorldSubsystem";


#ifndef RPG_BUILD_SHIPPING
	bDebugDrawMeshBound = false;
	bDebugDrawLightBound = false;
#endif // !RPG_BUILD_SHIPPING

}


void RpgRenderWorldSubsystem::PostTickUpdate() noexcept
{
    RpgWorld* world = GetWorld();

    for (auto it = world->Component_CreateIterator<RpgRenderComponent_Mesh>(); it; ++it)
    {
        RpgRenderComponent_Mesh& comp = it.GetValue();

        if (!world->GameObject_IsTransformUpdated(comp.GameObject))
        {
            continue;
        }

        comp.Bound = comp.Model ? comp.Model->GetBound() : RpgBoundingAABB(RpgVector3(-32.0f), RpgVector3(32.0f));

        // transform bound into world space
        comp.Bound = RpgBoundingBox(comp.Bound, world->GameObject_GetWorldTransformMatrix(comp.GameObject)).ToAABB();
    }


	for (auto it = world->Component_CreateIterator<RpgRenderComponent_Light>(); it; ++it)
	{
		RpgRenderComponent_Light& comp = it.GetValue();
		if (comp.Type == RpgRenderLight::TYPE_NONE || !comp.bIsVisible)
		{
			continue;
		}

		RpgShadowViewport* viewport = comp.ShadowViewport.Get();

		if (viewport == nullptr)
		{
			switch (comp.Type)
			{
				case RpgRenderLight::TYPE_POINT_LIGHT:
				{
					comp.ShadowViewport = RpgPointer::MakeUnique<RpgShadowViewport_PointLight>();
					viewport = comp.ShadowViewport.Get();

					break;
				}

				case RpgRenderLight::TYPE_SPOT_LIGHT:
				{
					comp.ShadowViewport = RpgPointer::MakeUnique<RpgShadowViewport_SpotLight>();
					viewport = comp.ShadowViewport.Get();

					break;
				}

				case RpgRenderLight::TYPE_DIRECTIONAL_LIGHT:
				{
					break;
				}

				default:
					break;
			}
		}

		RPG_Check(viewport);

		viewport->GameObject = comp.GameObject;
		viewport->AttenuationRadius = comp.AttenuationRadius;
		viewport->SpotInnerConeDegree = comp.SpotInnerConeDegree;
		viewport->SpotInnerConeDegree = comp.SpotOuterConeDegree;
	}


	for (auto it = world->Component_CreateIterator<RpgRenderComponent_Camera>(); it; ++it)
	{
		RpgRenderComponent_Camera& comp = it.GetValue();
		if (!comp.bActivated)
		{
			continue;
		}

		RpgSceneViewport* sceneViewport = comp.GetSceneViewport();
		sceneViewport->RenderTargetDimension = comp.RenderTargetDimension;

		const RpgTransform worldTransform = world->GameObject_GetWorldTransform(comp.GameObject);
		sceneViewport->SetViewRotationAndPosition(worldTransform.Rotation, worldTransform.Position);

		if (comp.ProjectionMode == RpgRenderProjectionMode::PERSPECTIVE)
		{
			sceneViewport->SetProjectionPerspective(comp.PerspectiveFoVDegree, comp.NearClipZ, comp.FarClipZ);
		}
		else
		{
			sceneViewport->SetProjectionOrthographic(comp.NearClipZ, comp.FarClipZ);
		}

		sceneViewport->UpdateViewProjection();

		RpgRenderTask_CaptureMesh taskCaptureMesh;
		taskCaptureMesh.World = world;
		taskCaptureMesh.Camera = &comp;
		taskCaptureMesh.Execute();

		RpgRenderTask_CaptureLight taskCaptureLight;
		taskCaptureLight.World = world;
		taskCaptureLight.Camera = &comp;
		taskCaptureLight.Execute();
	}
}


void RpgRenderWorldSubsystem::Render(int frameIndex, RpgRenderer* renderer) noexcept
{
	RpgWorld* world = GetWorld();

	for (auto it = world->Component_CreateIterator<RpgRenderComponent_Camera>(); it; ++it)
	{
		RpgRenderComponent_Camera& comp = it.GetValue();
		if (!comp.bActivated)
		{
			continue;
		}

		renderer->AddWorldSceneViewport(frameIndex, world, comp.GetSceneViewport());
	}


#ifndef RPG_BUILD_SHIPPING
	if (bDebugDrawMeshBound)
	{
		RpgVertexPrimitiveBatchLine* debugLine = renderer->Debug_GetPrimitiveBatchLine(frameIndex, world, false);

		for (auto it = world->Component_CreateConstIterator<RpgRenderComponent_Mesh>(); it; ++it)
		{
			const RpgRenderComponent_Mesh& comp = it.GetValue();
			if (!comp.bIsVisible)
			{
				continue;
			}

			debugLine->AddAABB(comp.Bound, RpgColorRGBA(255, 255, 255));
		}
	}

	if (bDebugDrawLightBound)
	{

	}
#endif // RPG_BUILD_SHIPPING

}
