#pragma once

#include "../RpgPlatform.h"


class RpgWorld;



struct RpgGameObjectID
{
public:
	RpgGameObjectID() noexcept
	{
		World = nullptr;
		Index = -1;
		Gen = UINT16_MAX;
	}

private:
	RpgGameObjectID(RpgWorld* in_World, int in_Index, uint16_t in_Gen) noexcept
	{
		World = in_World;
		Index = in_Index;
		Gen = in_Gen;
	}


public:
	inline bool IsValid() const noexcept
	{
		return World && Index != -1 && Gen != UINT16_MAX;
	}

	inline int GetIndex() const noexcept
	{
		return Index;
	}

	inline bool operator==(const RpgGameObjectID& rhs) const noexcept
	{
		return World == rhs.World && Index == rhs.Index && Gen == rhs.Gen;
	}

	inline bool operator!=(const RpgGameObjectID& rhs) const noexcept
	{
		return !(*this == rhs);
	}


private:
	class RpgWorld* World;
	int Index;
	uint16_t Gen;


	friend RpgWorld;

};



// Maximum script per game object
#define RPG_GAMEOBJECT_MAX_SCRIPT		4


#define RPG_GAMEOBJECT_SCRIPT(name)										\
public:																	\
static constexpr const char* TYPE_NAME = name;							\
virtual const char* GetTypeName() const noexcept { return TYPE_NAME; }	\
friend RpgWorld;



class RpgGameObjectScript
{
	RPG_NOCOPY(RpgGameObjectScript)

protected:
	RpgGameObjectScript() noexcept
	{
		World = nullptr;
		bInitialized = false;
		bStartedPlay = false;
	}

public:
	virtual ~RpgGameObjectScript() noexcept = default;

	virtual void AttachedToGameObject() noexcept {}
	virtual void DetachedFromGameObject() noexcept {}
	virtual void StartPlay() noexcept {}
	virtual void StopPlay() noexcept {}
	virtual void TickUpdate(float deltaTime) noexcept {}
	virtual const char* GetTypeName() const noexcept { return "RpgScript"; }


protected:
	RpgGameObjectID GameObject;
	RpgWorld* World;

private:
	bool bInitialized;
	bool bStartedPlay;


	friend RpgWorld;

};
