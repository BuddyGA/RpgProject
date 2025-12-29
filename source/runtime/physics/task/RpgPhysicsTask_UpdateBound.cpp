#include "RpgPhysicsTask_UpdateBound.h"
#include "core/world/RpgWorld.h"



RpgPhysicsTask_UpdateBound::RpgPhysicsTask_UpdateBound() noexcept
{
}


void RpgPhysicsTask_UpdateBound::Reset() noexcept
{
	RpgThreadTask::Reset();

	World = nullptr;
}


void RpgPhysicsTask_UpdateBound::Execute() noexcept
{

}
