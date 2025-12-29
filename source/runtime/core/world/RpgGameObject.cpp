#include "RpgGameObject.h"
#include "RpgLevel.h"



RpgGameObject::RpgGameObject() noexcept
{
	Level = nullptr;
	Index = RPG_INDEX_INVALID;
	Gen = UINT16_MAX;
}


RpgGameObject::RpgGameObject(RpgLevel* in_Level, int in_Index, uint16_t in_Gen) noexcept
{
	Level = in_Level;
	Index = in_Index;
	Gen = in_Gen;
}


void RpgGameObject::Destroy() noexcept
{
	RPG_Check(!IsNull());
	Level->GameObject_Destroy(*this);
}


bool RpgGameObject::IsPendingDestroy() const noexcept
{
	return IsNull() || !Level->GameObject_IsValid(*this);
}


RpgStringID RpgGameObject::GetName() const noexcept
{
	RPG_Check(!IsNull());
	return Level->GameObject_GetName(*this);
}


void RpgGameObject::AttachToParent(RpgGameObject parent, RpgGameObjectAttachMode mode) noexcept
{
	RPG_Check(!IsNull());
	Level->GameObject_AttachToParent(*this, parent, mode);
}


void RpgGameObject::DetachFromParent() noexcept
{
	RPG_Check(!IsNull());
	Level->GameObject_DetachFromParent(*this);
}


void RpgGameObject::SpawnAtTransform(const RpgTransform& worldTransform) noexcept
{
	RPG_Check(!IsNull());
	Level->GameObject_Spawn(*this, worldTransform);
}


void RpgGameObject::SetVisibility(bool bIsVisible) noexcept
{
	RPG_Check(!IsNull());
	Level->GameObject_SetVisibility(*this, bIsVisible);
}


void RpgGameObject::SetLocalTransform(const RpgTransform& transform) noexcept
{
	RPG_Check(!IsNull());
	Level->GameObject_SetLocalTransform(*this, transform);
}


const RpgTransform& RpgGameObject::GetLocalTransform() const noexcept
{
	RPG_Check(!IsNull());
	return Level->GameObject_GetLocalTransform(*this);
}


void RpgGameObject::SetWorldTransform(const RpgTransform& transform) noexcept
{
	RPG_Check(!IsNull());
	Level->GameObject_SetWorldTransform(*this, transform);
}


const RpgTransform& RpgGameObject::GetWorldTransform() const noexcept
{
	RPG_Check(!IsNull());
	return Level->GameObject_GetWorldTransform(*this);
}


RpgMatrixTransform RpgGameObject::GetWorldTransformMatrix() const noexcept
{
	RPG_Check(!IsNull());
	return Level->GameObject_GetWorldTransformMatrix(*this);
}


void RpgGameObject::AttachScript(RpgGameObjectScript* script) noexcept
{
	RPG_Check(!IsNull());
	Level->GameObject_AttachScript(*this, script);
}


void RpgGameObject::DetachScript(RpgGameObjectScript* script) noexcept
{
	RPG_Check(!IsNull());
	Level->GameObject_DetachScript(*this, script);
}


void* RpgGameObject::InternalGetComponent(int compType) const noexcept
{
	RPG_Check(!IsNull());
	return Level->GameObject_GetComponent(*this, compType);
}


void* RpgGameObject::InternalAddComponent(int compType) const noexcept
{
	RPG_Check(!IsNull());
	return Level->GameObject_AddComponent(*this, compType);
}


bool RpgGameObject::InternalRemoveComponent(int compType) const noexcept
{
	RPG_Check(!IsNull());
	return Level->GameObject_RemoveComponent(*this, compType);
}
