#pragma once

#include "core/RpgThreadPool.h"
#include "../asset/RpgMesh.h"



class RpgRenderTask_GenerateTerrain : public RpgThreadTask
{
public:
	RpgSharedMesh TerrainMesh;


public:
	RpgRenderTask_GenerateTerrain() noexcept;

	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;

	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgRenderTask_GenerateTerrain";
	}

};
