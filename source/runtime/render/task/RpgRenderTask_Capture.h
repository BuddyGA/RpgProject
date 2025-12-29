#pragma once

#include "core/RpgThreadPool.h"


class RpgSceneViewport;
class RpgShadowViewport;



class RpgRenderTask_Capture : public RpgThreadTask
{
public:
	RpgSceneViewport* SceneViewport;
	RpgShadowViewport* ShadowViewport;
	int FrameIndex;


public:
	RpgRenderTask_Capture() noexcept;
	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgRenderTask_Capture";
	}

};
