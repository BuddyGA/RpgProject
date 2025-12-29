#include "RpgPhysicsTask_UpdateShape.h"
#include "core/world/RpgWorld.h"



RpgPhysicsTask_UpdateShape::RpgPhysicsTask_UpdateShape() noexcept
{
}


void RpgPhysicsTask_UpdateShape::Reset() noexcept
{
	RpgThreadTask::Reset();

	World = nullptr;
}


void RpgPhysicsTask_UpdateShape::Execute() noexcept
{

}
