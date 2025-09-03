#pragma once

#include "core/world/RpgComponent.h"
#include "../RpgGameTypes.h"



class RpgAttributeComponent
{
	RPG_COMPONENT_TYPE("RpgAttributeComponent");

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
