#pragma once

#include "core/dsa/RpgArray.h"



class RpgAttributeComponent;
class RpgAttributeWorldSubsystem;



namespace RpgAttribute
{
	enum EType : uint8_t
	{
		NONE = 0,

		STR,
		VIT,
		INT,
		CON,
		DEX,
		AGI,

		PHYS_ATK,
		PHYS_DEF,
		PHYS_CRIT_RATE,
		MAG_ATK,
		MAG_DEF,
		MAG_CRIT_RATE,
		ASPD,
		CSPD,
		MSPD,

		MAX_COUNT
	};

};
