#pragma once

#include "core/world/RpgWorld.h"



class RpgRenderWorldSubsystem : public RpgWorldSubsystem
{
public:
	RpgRenderWorldSubsystem() noexcept;

protected:
	virtual void PostTickUpdate() noexcept override;
	virtual void Render(int frameIndex, RpgRenderer* renderer) noexcept override;


#ifndef RPG_BUILD_SHIPPING
public:
	bool bDebugDrawMeshBound;
	bool bDebugDrawLightBound;
#endif // !RPG_BUILD_SHIPPING

};
