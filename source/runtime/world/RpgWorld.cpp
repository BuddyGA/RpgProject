#include "RpgWorld.h"



RpgWorld::RpgWorld() noexcept
{
}


void RpgWorld::BeginFrame(int frameIndex) noexcept
{

}


void RpgWorld::EndFrame(int frameIndex) noexcept
{
}


void RpgWorld::DispatchTickUpdate(float deltaTime) noexcept
{
}


void RpgWorld::DispatchPostTickUpdate() noexcept
{
}


RpgEntity RpgWorld::Entity_Create(const RpgName& name, bool bIsTransient = false) noexcept
{
	RPG_Check(EntityNames.GetCount() < RPG_ENTITY_MAX_COUNT);

	const int nameId = EntityNames.Add();
	const int genFlagsId = EntityGenFlags.Add();
	RPG_Check(nameId == genFlagsId);

	// name
	EntityNames[nameId] = RpgStringID(name, true);

	// genflags
	FEntityGenFlags& genFlags = EntityGenFlags[genFlagsId];
	genFlags.Flags = bIsTransient ? FLAG_Entity_Transient : FLAG_Entity_None;

	return RpgEntity(this, nameId, genFlags.Gen);
}


void RpgWorld::Entity_AttachToParent(RpgEntity entity, RpgEntity parent, RpgTransformAttachMode attachMode) noexcept
{

}


void RpgWorld::Entity_Spawn(RpgEntity entity) noexcept
{

}


void RpgWorld::Entity_Destroy(RpgEntity entity) noexcept
{
	if (!Entity_IsValid(entity))
	{
		return;
	}
}
