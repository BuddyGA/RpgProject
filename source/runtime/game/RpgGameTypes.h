#pragma once

#include "core/dsa/RpgArray.h"



class RpgAttributeComponent;
class RpgAttributeWorldSubsystem;



namespace RpgAttribute
{
	enum EType : uint8_t
	{
		NONE = 0,

		MHP,				// Maximum health point
		MMP,				// Maximum mana point
		MSP,				// Maximum stamina point
		STR,				// Strength
		VIT,				// Vitality
		INT,				// Intelligence
		CON,				// Concentration
		DEX,				// Dexterity
		AGI,				// Agility

		HP,					// Health point
		MP,					// Mana point
		SP,					// Stamina point
		PATK,				// Physical attack
		PDEF,				// Physical defense
		PCRT,				// Physical critical rate
		MATK,				// Magic attack
		MDEF,				// Magic defense
		MCRT,				// Magic critical rate
		ASPD,				// Attack speed
		CSPD,				// Cast speed
		MSPD,				// Move speed

		EL_RES_FIRE,		// Elemental resistance (fire)
		EL_RES_WATER,		// Elemental resistance (water)
		EL_RES_WIND,		// Elemental resistance (wind)
		EL_RES_EARTH,		// Elemental resistance (earth)
		EL_RES_LIGHT,		// Elemental resistance (light)
		EL_RES_SHADOW,		// Elemental resistance (shadow)

		ST_RES_BURN,		// Status resistance (burn)
		ST_RES_FROZEN,		// Status resistance (frozen)
		ST_RES_SLOW,		// Status resistance (slow)
		ST_RES_STUN,		// Status resistance (stun)
		ST_RES_SHOCK,		// Status resistance (shock)
		ST_RES_BLIND,		// Status resistance (blind)
		ST_RES_POISON,		// Status resistance (poison)
		ST_RES_DIZZY,		// Status resistance (dizzy)
		ST_RES_MANA_BIND,	// Status resistance (mana bind)

		ST_CON_BURN,		// Status condition (burn)
		ST_CON_FROZEN,		// Status condition (frozen)
		ST_CON_SLOW,		// Status condition (slow)
		ST_CON_STUN,		// Status condition (stun)
		ST_CON_SHOCK,		// Status condition (shock)
		ST_CON_BLIND,		// Status condition (blind)
		ST_CON_POISON,		// Status condition (poison)
		ST_CON_DIZZY,		// Status condition (dizzy)
		ST_CON_MANA_BIND,	// Status condition (mana bind)

		MAX_COUNT
	};

};
