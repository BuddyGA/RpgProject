#pragma once

#include "core/RpgThreadPool.h"


class RpgWorld;
class RpgRenderComponent_Camera;



class RpgRenderTask_CaptureMesh : public RpgThreadTask
{
public:
	RpgWorld* World;
	RpgRenderComponent_Camera* Camera;


public:
	RpgRenderTask_CaptureMesh() noexcept;
	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgRenderTask_CaptureMesh";
	}

};



class RpgRenderTask_CaptureLight : public RpgThreadTask
{
public:
	RpgWorld* World;
	RpgRenderComponent_Camera* Camera;


public:
	RpgRenderTask_CaptureLight() noexcept;
	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgRenderTask_CaptureLight";
	}

};
