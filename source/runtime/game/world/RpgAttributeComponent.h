#pragma once

#include "core/world/RpgComponent.h"
#include "../RpgGameTypes.h"



class RpgAttributeComponent
{
	RPG_COMPONENT(RpgAttributeComponent, 16);

public:
	float Base[RpgAttribute::MAX_COUNT];
	float Modifier[RpgAttribute::MAX_COUNT];
	float Final[RpgAttribute::MAX_COUNT];


public:
	RpgAttributeComponent() noexcept
		: Base()
		, Modifier()
		, Final()
	{
	}


	inline void Destroy() noexcept
	{
		// Nothing to do
	}

};
