#pragma once

#include "RpgArray.h"



template<typename T, int CAPACITY_ALIGNMENT = 1>
class RpgSet
{
public:
	RpgSet() noexcept = default;
	~RpgSet() noexcept = default;


	RpgSet(const RpgSet& other) noexcept
		: Hashes(other.Hashes)
		, Values(other.Values)
	{
	}


	RpgSet(RpgSet&& other) noexcept
		: Hashes(std::move(other.Hashes))
		, Values(std::move(other.Values))
	{
	}


public:
	inline T& operator[](int index) noexcept
	{
		return Values[index];
	}


	inline const T& operator[](int index) const noexcept
	{
		return Values[index];
	}


public:
	inline bool Exists(const T& value) const noexcept
	{
		const uint64_t hash = Rpg::GetHash<T>(value);
		RPG_Assert(hash > 0);

		return Hashes.FindIndexByValue(hash) != RPG_INDEX_INVALID;
	}


	inline void Reserve(int in_Capacity) noexcept
	{
		Hashes.Reserve(in_Capacity);
		Values.Reserve(in_Capacity);
	}


	inline bool Add(const T& in_Value) noexcept
	{
		const uint64_t hash = Rpg::GetHash<T>(in_Value);
		RPG_Assert(hash > 0);

		const int index = Hashes.FindIndexByValue(hash);
		bool bAdded = false;

		if (index == RPG_INDEX_INVALID)
		{
			Hashes.AddValue(hash);
			Values.AddValue(in_Value);
			bAdded = true;
		}
		else
		{
		#ifndef RPG_BUILD_SHIPPING
			const int checkCollisionIndex = Values.FindIndexByValue(in_Value);
			RPG_CheckV(checkCollisionIndex != RPG_INDEX_INVALID, "RpgSet collision!");
		#endif // !RPG_BUILD_SHIPPING
		}

		return bAdded;
	}


	inline bool RemoveAt(int index) noexcept
	{
		const bool bRemoved = Hashes.RemoveAt(index, false);
		Values.RemoveAt(index, false);

		return bRemoved;
	}


	inline void Clear(bool bFreeMemory = false) noexcept
	{
		Hashes.Clear(bFreeMemory);
		Values.Clear(bFreeMemory);
	}


	inline const RpgArray<T, CAPACITY_ALIGNMENT>& GetValueArray() const noexcept
	{
		return Values;
	}


	inline int GetCapacity() const noexcept
	{
		return Hashes.GetCapacity();
	}


	inline int GetCount() const noexcept
	{
		return Hashes.GetCount();
	}


	inline bool IsEmpty() const noexcept
	{
		return Hashes.GetCount() == 0;
	}


	inline T* begin() noexcept
	{
		return Values.begin();
	}

	inline const T* begin() const noexcept
	{
		return Values.begin();
	}

	inline T* end() noexcept
	{
		return Values.end();
	}

	inline const T* end() const noexcept
	{
		return Values.end();
	}


private:
	RpgArray<uint64_t, CAPACITY_ALIGNMENT> Hashes;
	RpgArray<T, CAPACITY_ALIGNMENT> Values;


	friend class RpgStreamWriter;
	friend class RpgStreamReader;

};
