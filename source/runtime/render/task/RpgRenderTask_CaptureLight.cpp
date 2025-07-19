#include "RpgRenderTask_Capture.h"
#include "core/world/RpgWorld.h"
#include "../world/RpgRenderComponent.h"



RpgRenderTask_CaptureLight::RpgRenderTask_CaptureLight() noexcept
{
	World = nullptr;
	Camera = nullptr;
}


void RpgRenderTask_CaptureLight::Reset() noexcept
{
	RpgThreadTask::Reset();

	World = nullptr;
	Camera = nullptr;
}


void RpgRenderTask_CaptureLight::Execute() noexcept
{
	RPG_Assert(World);
	RPG_Assert(Camera);

	const bool bFrustumCulling = Camera->bFrustumCulling;
	RpgSceneViewport* viewport = Camera->GetSceneViewport();
	const RpgBoundingFrustum frustum = viewport->GetViewFrustum();
	viewport->Lights.Clear();

	for (auto it = World->Component_CreateIterator<RpgRenderComponent_Light>(); it; ++it)
	{
		RpgRenderComponent_Light& comp = it.GetValue();
		if (!comp.bIsVisible)
		{
			continue;
		}

		RpgSceneLight& data = viewport->Lights.Add();
		data.GameObject = comp.GameObject;
		data.WorldTransform = World->GameObject_GetWorldTransform(comp.GameObject);
		data.Type = comp.Type;
		data.ColorIntensity = comp.ColorIntensity;
		data.AttenuationRadius = comp.AttenuationRadius;
		data.AttenuationFallOffExp = comp.AttenuationFallOffExp;
		data.SpotInnerConeDegree = comp.SpotInnerConeDegree;
		data.SpotOuterConeDegree = comp.SpotOuterConeDegree;
		data.ShadowViewport = comp.bCastShadow ? comp.GetShadowViewport() : nullptr;
	}
}
