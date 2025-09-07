#include "RpgRenderTask_Capture.h"
#include "core/world/RpgWorld.h"
#include "../world/RpgRenderComponent.h"



RpgRenderTask_CaptureLight::RpgRenderTask_CaptureLight() noexcept
{
	World = nullptr;
	Camera = nullptr;
	FrameIndex = 0;
}


void RpgRenderTask_CaptureLight::Reset() noexcept
{
	RpgThreadTask::Reset();

	World = nullptr;
	Camera = nullptr;
	FrameIndex = 0;
}


void RpgRenderTask_CaptureLight::Execute() noexcept
{
	RPG_Assert(World);
	RPG_Assert(Camera);

	const bool bFrustumCulling = Camera->bFrustumCulling;
	RpgSceneViewport* viewport = Camera->GetSceneViewport();
	const RpgBoundingFrustum frustum = viewport->GetViewFrustum();
	
	RpgArray<RpgSceneLight>& sceneLights = viewport->GetFrameLights(FrameIndex);
	sceneLights.Clear();

	for (auto it = World->ComponentIterator<RpgRenderComponent_Light>(); it; ++it)
	{
		RpgRenderComponent_Light& comp = it.GetValue();

		// check if visible
		// check if spawned
		if (!comp.IsGameObjectSpawned() || !comp.bIsVisible)
		{
			continue;
		}

		RpgSceneLight& data = sceneLights.Add();
		data.GameObject = comp.GetGameObject();
		data.WorldTransform = data.GameObject.GetWorldTransform();
		data.Type = comp.Type;
		data.ColorIntensity = comp.ColorIntensity;
		data.AttenuationRadius = comp.AttenuationRadius;
		data.AttenuationFallOffExp = comp.AttenuationFallOffExp;
		data.SpotInnerConeDegree = comp.SpotInnerConeDegree;
		data.SpotOuterConeDegree = comp.SpotOuterConeDegree;
		data.ShadowViewport = comp.bCastShadow ? comp.GetShadowViewport() : nullptr;
	}
}
