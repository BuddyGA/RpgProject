#pragma once

#include "../RpgMath.h"
#include "RpgComponent.h"



class RpgLevel
{
	RPG_NOCOPY(RpgLevel)

public:
	RpgLevel(const RpgName& in_Name) noexcept;
	~RpgLevel() noexcept;

	void StreamWrite(RpgStreamWriter& writer) const noexcept;
	void StreamRead(RpgStreamReader& reader) noexcept;
	void AddGameObject(RpgGameObjectID gameObject) noexcept;
	void RemoveGameObject(RpgGameObjectID gameObject) noexcept;


	inline const RpgName& GetName() const noexcept
	{
		return Name;
	}


	inline const RpgBoundingAABB& GetBound() const noexcept
	{
		return Bound;
	}


	inline const RpgArray<RpgGameObjectID>& GetGameObjects() const noexcept
	{
		return GameObjects;
	}


private:
	void UpdateBound() noexcept;


private:
	RpgName Name;
	RpgBoundingAABB Bound;
	RpgMatrixTransform WorldMatrix;
	RpgMatrixTransform InverseWorldMatrix;
	RpgArray<RpgGameObjectID> GameObjects;
	RpgWorld* World;


	friend RpgWorld;

};
