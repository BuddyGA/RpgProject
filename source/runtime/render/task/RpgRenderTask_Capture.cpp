#include "RpgRenderTask_Capture.h"
#include "world/RpgWorld.h"
#include "../RpgSceneViewport.h"



RpgRenderTask_Capture::RpgRenderTask_Capture() noexcept
{
	SceneViewport = nullptr;
	ShadowViewport = nullptr;
	FrameIndex = 0;
}


void RpgRenderTask_Capture::Reset() noexcept
{
	RpgThreadTask::Reset();

	SceneViewport = nullptr;
	ShadowViewport = nullptr;
	FrameIndex = 0;
}


void RpgRenderTask_Capture::Execute() noexcept
{
	RPG_Assert(SceneViewport || ShadowViewport);

	if (SceneViewport)
	{
		RpgWorld* world = SceneViewport->World;
		RPG_Assert(world);

		const bool bFrustumCulling = SceneViewport->bFrustumCulling;
		const RpgBoundingFrustum frustum = SceneViewport->GetFrameViewFrustum(FrameIndex);

		// capture meshes

	}
}
