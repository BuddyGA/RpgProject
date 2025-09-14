#pragma once

#include "core/world/RpgWorld.h"



class RpgGameWorld : public RpgWorld
{
public:
	RpgGameWorld(const RpgName& in_Name) noexcept;
	virtual void Initialize() noexcept override;

};
