#pragma once

#include "core/RpgThreadPool.h"
#include "asset/RpgMesh.h"



class RpgWorldTask_GenerateTerrain : public RpgThreadTask
{
public:
	RpgSharedMesh TerrainMesh;


public:
	RpgWorldTask_GenerateTerrain() noexcept;

	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;

	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgWorldTask_GenerateTerrain";
	}

};
