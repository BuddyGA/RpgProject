#include "RpgPhysicsWorldSubsystem.h"
#include "RpgPhysicsComponent.h"



RpgPhysicsWorldSubsystem::RpgPhysicsWorldSubsystem() noexcept
{
	Name = "PhysicsWorldSubsystem";
	bTickUpdateCollision = false;

#ifndef RPG_BUILD_SHIPPING
	bDebugDrawCollisionBound = false;
	bDebugDrawCollisionShape = false;
#endif // !RPG_BUILD_SHIPPING

}


void RpgPhysicsWorldSubsystem::StartPlay() noexcept
{
	bTickUpdateCollision = true;
}


void RpgPhysicsWorldSubsystem::StopPlay() noexcept
{
	bTickUpdateCollision = false;
}


void RpgPhysicsWorldSubsystem::TickUpdate(float deltaTime) noexcept
{
	if (!bTickUpdateCollision)
	{
		return;
	}

	BroadphaseCollisionPairs.Clear();
	NarrowphaseCollisionPairs.Clear();

	RpgWorld* world = GetWorld();


	RpgArrayInline<RpgThreadTask*, 4> submitTasks;
	// update bounds
	{
		TaskUpdateBound.Reset();
		TaskUpdateBound.World = world;
		submitTasks.AddValue(&TaskUpdateBound);
	}
	// update shapes
	{
		TaskUpdateShape.Reset();
		TaskUpdateShape.World = world;
		submitTasks.AddValue(&TaskUpdateShape);
	}
	RpgThreadPool::SubmitTasks(submitTasks.GetData(), submitTasks.GetCount());


	// generate pairs for broadphase
	RpgPhysicsCollision::Filter::GeneratePairs(BroadphaseCollisionPairs, world);

	if (BroadphaseCollisionPairs.IsEmpty())
	{
		return;
	}


	// wait update bound finished
	TaskUpdateBound.Wait();

	// generate pairs for narrowphase
	RpgPhysicsCollision::Broadphase::GeneratePairs(NarrowphaseCollisionPairs, BroadphaseCollisionPairs);


	// wait update shape finished
	TaskUpdateShape.Wait();

	// test overlaps
	for (int i = 0; i < NarrowphaseCollisionPairs.GetCount(); ++i)
	{

	}
}


void RpgPhysicsWorldSubsystem::Render(int frameIndex, RpgRenderer* renderer) noexcept
{

#ifndef RPG_BUILD_SHIPPING
	const RpgWorld* world = GetWorld();

	if (bDebugDrawCollisionBound)
	{

	}


	if (bDebugDrawCollisionShape)
	{

	}
#endif // !RPG_BUILD_SHIPPING

}
