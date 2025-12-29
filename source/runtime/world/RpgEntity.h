#pragma once

#include "core/RpgMath.h"


#define RPG_ENTITY_MAX_COUNT	65536


class RpgWorld;



class RpgEntity
{
public:
	RpgEntity() noexcept
	{
		World = nullptr;
		Id = RPG_INDEX_INVALID;
		Gen = 0;
	}

private:
	RpgEntity(RpgWorld* in_World, int in_Id, uint16_t in_Gen) noexcept
	{
		World = in_World;
		Id = in_Id;
		Gen = in_Gen;
	}


public:
	inline int GetId() const noexcept
	{
		return Id;
	}

	inline bool IsValid() const noexcept
	{
		return Id != RPG_INDEX_INVALID;
	}

	inline bool operator==(const RpgEntity& rhs) const noexcept
	{
		return World == rhs.World && Id == rhs.Id && Gen == rhs.Gen;
	}

	inline bool operator!=(const RpgEntity& rhs) const noexcept
	{
		return !(*this == rhs);
	}

	inline operator bool() noexcept
	{
		return IsValid();
	}


private:
	RpgWorld* World;
	int Id;
	uint16_t Gen;


	friend class RpgWorld;

};
