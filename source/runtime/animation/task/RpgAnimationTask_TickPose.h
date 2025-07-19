#pragma once

#include "core/RpgThreadPool.h"
#include "core/dsa/RpgArray.h"


class RpgWorld;
class RpgAnimationComponent_AnimSkeletonPose;



class RpgAnimationTask_TickPose : public RpgThreadTask
{
public:
	const RpgWorld* World;
	float DeltaTime;
	float GlobalPlayRate;
	RpgArray<RpgAnimationComponent_AnimSkeletonPose*> AnimationComponents;


public:
	RpgAnimationTask_TickPose() noexcept;
	virtual void Reset() noexcept override;
	virtual void Execute() noexcept override;


	virtual const char* GetTaskName() const noexcept override
	{
		return "RpgAnimationTask_TickPose";
	}

};
