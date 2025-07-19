#pragma once

#include "core/world/RpgWorld.h"
#include "../task/RpgPhysicsTask_UpdateBound.h"
#include "../task/RpgPhysicsTask_UpdateShape.h"



class RpgPhysicsWorldSubsystem : public RpgWorldSubsystem
{
public:
	RpgPhysicsWorldSubsystem() noexcept;

protected:
	virtual void StartPlay() noexcept override;
	virtual void StopPlay() noexcept override;
	virtual void TickUpdate(float deltaTime) noexcept override;
	virtual void Render(int frameIndex, RpgRenderer* renderer) noexcept override;


private:
	RpgPhysicsTask_UpdateBound TaskUpdateBound;
	RpgPhysicsTask_UpdateShape TaskUpdateShape;
	RpgArray<RpgPhysicsCollision::FPairTest> BroadphaseCollisionPairs;
	RpgArray<RpgPhysicsCollision::FPairTest> NarrowphaseCollisionPairs;
	bool bTickUpdateCollision;


#ifndef RPG_BUILD_SHIPPING
public:
	bool bDebugDrawCollisionBound;
	bool bDebugDrawCollisionShape;
#endif // !RPG_BUILD_SHIPPING

};
