#pragma once

#include "core/world/RpgWorld.h"
#include "../task/RpgAnimationTask_TickPose.h"



class RpgAnimationWorldSubsystem : public RpgWorldSubsystem
{
public:
	float GlobalPlayRate;
	bool bDebugDrawSkeletonBones;


public:
	RpgAnimationWorldSubsystem() noexcept;

protected:
	virtual void StartPlay() noexcept override;
	virtual void StopPlay() noexcept override;
	virtual void TickUpdate(float deltaTime) noexcept override;
	virtual void Render(int frameIndex, RpgRenderer* renderer) noexcept override;


private:
	static constexpr int TASK_COUNT = 4;
	RpgAnimationTask_TickPose TaskTickPoses[TASK_COUNT];
	bool bTickAnimationPose;

};
