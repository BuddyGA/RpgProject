#pragma once

#include "core/world/RpgComponent.h"
#include "../RpgGameTypes.h"



class RpgGameComponent_Attribute
{
	RPG_COMPONENT(RpgGameComponent_Attribute, RPG_COMPONENT_ID_GAME_0)

public:
	float Base[RpgAttribute::MAX_COUNT];
	float Modifier[RpgAttribute::MAX_COUNT];
	float Final[RpgAttribute::MAX_COUNT];


public:
	RpgGameComponent_Attribute() noexcept
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
