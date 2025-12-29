#include "RpgGameWorld.h"
#include "physics/world/RpgPhysicsComponent.h"
#include "physics/world/RpgPhysicsWorldSubsystem.h"
#include "render/world/RpgRenderComponent.h"
#include "render/world/RpgRenderWorldSubsystem.h"
#include "animation/world/RpgAnimationComponent.h"
#include "animation/world/RpgAnimationWorldSubsystem.h"



void Rpg_RegisterComponents(RpgLevel* level) noexcept
{
	level->Component_Register<RpgPhysicsComponent_Filter>();
	level->Component_Register<RpgPhysicsComponent_Collision>();
	level->Component_Register<RpgRenderComponent_Camera>();
	level->Component_Register<RpgRenderComponent_Mesh>();
	level->Component_Register<RpgRenderComponent_Light>();
	level->Component_Register<RpgAnimationComponent_AnimSkeletonPose>();
}



RpgGameWorld::RpgGameWorld(const RpgName& in_Name) noexcept
	: RpgWorld(in_Name)
{
}


void RpgGameWorld::Initialize() noexcept
{
	RpgWorld::Initialize();

	Subsystem_Register<RpgPhysicsWorldSubsystem>();
	Subsystem_Register<RpgAnimationWorldSubsystem>();
	Subsystem_Register<RpgRenderWorldSubsystem>();
}
