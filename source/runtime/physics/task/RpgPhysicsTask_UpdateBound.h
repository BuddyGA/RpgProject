#pragma once

#include "core/RpgThreadPool.h"
#include "../world/RpgPhysicsComponent.h"



class RpgPhysicsTask_UpdateBound : public RpgThreadTask
{
public:
	RpgWorld* World;


public:
	RpgPhysicsTask_UpdateBound() noexcept;
	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgPhysicsTask_UpdateBound";
	}

};
