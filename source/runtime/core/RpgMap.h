#pragma once

#include "RpgArray.h"


#ifndef RPG_BUILD_SHIPPING
	#define RPG_MAP_CHECK_COLLISION	1
#else
	#define RPG_MAP_CHECK_COLLISION 0
#endif // !RPG_BUILD_SHIPPING



// ============================================================================================================================================================================================== //
// RpgMap
// Unordered hash map, key-value pair array.
// ============================================================================================================================================================================================== //
template<typename TKey, typename TValue, int CAPACITY_ALIGNMENT = 1>
class RpgMap
{
public:
	RpgMap() noexcept
	{
		Hashes.Reserve(8);
		Keys.Reserve(8);
		Values.Reserve(8);
	}


	RpgMap(const RpgMap& other) noexcept
		: Hashes(other.Hashes)
		, Keys(other.Keys)
		, Values(other.Values)
	{
	}


	RpgMap(RpgMap&& other) noexcept
		: Hashes(std::move(other.Hashes))
		, Keys(std::move(other.Keys))
		, Values(std::move(other.Values))
	{
	}


	~RpgMap() noexcept = default;


public:
	inline RpgMap& operator=(const RpgMap& rhs) noexcept
	{
		if (this != &rhs)
		{
			Hashes = rhs.Hashes;
			Keys = rhs.Keys;
			Values = rhs.Values;
		}

		return *this;
	}


	inline RpgMap& operator=(RpgMap&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Hashes = std::move(rhs.Hashes);
			Keys = std::move(rhs.Keys);
			Values = std::move(rhs.Values);
		}

		return *this;
	}


	/*
	inline TValue& operator[](const TKey& key) noexcept
	{
		return Add(key);
	}
	*/


	inline const TValue& operator[](const TKey& key) const noexcept
	{
		const int index = FindIndex(Rpg_GetHash(key));
		RPG_ValidateV(index != RPG_INDEX_INVALID, "RpgMap key not found!");

		return Values[index];
	}


public:
	inline void Reserve(int in_Capacity) noexcept
	{
		Hashes.Reserve(in_Capacity);
		Keys.Reserve(in_Capacity);
		Values.Reserve(in_Capacity);
	}


	inline TValue& Add(const TKey& key, int* optOut_Index = nullptr) noexcept
	{
		const uint64_t hash = Rpg_GetHash(key);
		int index = FindIndex(hash);

		if (index == RPG_INDEX_INVALID)
		{
			index = Hashes.GetCount();
			Hashes.AddValue(hash);
			Keys.AddValue(key);
			Values.Add();
		}
		else
		{
		#if RPG_MAP_CHECK_COLLISION
			const int keyIndex = Keys.FindIndexByValue(key);
			RPG_ValidateV(keyIndex != RPG_INDEX_INVALID, "RpgMap collision!");
		#endif // RPG_MAP_CHECK_COLLISION
		}

		if (optOut_Index)
		{
			*optOut_Index = index;
		}

		return Values[index];
	}


	inline int AddValue(const TKey& key, const TValue& in_Value) noexcept
	{
		const uint64_t hash = Rpg_GetHash(key);
		int index = FindIndex(hash);

		if (index == RPG_INDEX_INVALID)
		{
			index = Hashes.GetCount();
			Hashes.AddValue(hash);
			Keys.AddValue(key);
			Values.AddValue(in_Value);
		}
		else
		{
		#if RPG_MAP_CHECK_COLLISION
			const int keyIndex = Keys.FindIndexByValue(key);
			RPG_ValidateV(keyIndex != RPG_INDEX_INVALID, "RpgMap collision!");
		#endif // RPG_MAP_CHECK_COLLISION

			Values[index] = in_Value;
		}

		return index;
	}


	template<typename...TConstructorArgs>
	inline void AddConstruct(const TKey& key, TConstructorArgs&&... args) noexcept
	{
		const uint64_t hash = Rpg_GetHash(key);
		int index = FindIndex(hash);

		if (index == RPG_INDEX_INVALID)
		{
			index = Hashes.GetCount();
			Hashes.AddValue(hash);
			Keys.AddValue(key);
			Values.AddConstruct(std::forward<TConstructorArgs>(args)...);
		}
		else
		{
		#if RPG_MAP_CHECK_COLLISION
			const int keyIndex = Keys.FindIndexByValue(key);
			RPG_ValidateV(keyIndex != RPG_INDEX_INVALID, "RpgMap collision!");
		#endif // RPG_MAP_CHECK_COLLISION

			Values[index] = TValue(std::forward<TConstructorArgs>(args)...);
		}
	}


	inline void Remove(const TKey& key, bool bKeepOrder = false) noexcept
	{
		const int index = FindIndex(Rpg_GetHash(key));

		if (index != RPG_INDEX_INVALID)
		{
			Hashes.RemoveAt(index, bKeepOrder);
			Keys.RemoveAt(index, bKeepOrder);
			Values.RemoveAt(index, bKeepOrder);
		}
	}


	inline void RemoveAt(int index, bool bKeepOrder = false) noexcept
	{
		Hashes.RemoveAt(index, bKeepOrder);
		Keys.RemoveAt(index, bKeepOrder);
		Values.RemoveAt(index, bKeepOrder);
	}


	inline bool Exists(const TKey& key, int* optOut_Index = nullptr) const noexcept
	{
		const int index = FindIndex(Rpg_GetHash(key));

		if (optOut_Index)
		{
			*optOut_Index = index;
		}

		return index != RPG_INDEX_INVALID;
	}


	inline const TKey& GetKeyByIndex(int index) const noexcept
	{
		return Keys[index];
	}


	inline TValue* FindValue(const TKey& key) noexcept
	{
		const int index = FindIndex(Rpg_GetHash(key));

		if (index == RPG_INDEX_INVALID)
		{
			return nullptr;
		}

		return &Values[index];
	}


	inline const TValue* FindValue(const TKey& key) const noexcept
	{
		const int index = FindIndex(Rpg_GetHash(key));

		if (index == RPG_INDEX_INVALID)
		{
			return nullptr;
		}

		return &Values[index];
	}


	inline TValue& GetValueByIndex(int index) noexcept
	{
		return Values[index];
	}


	inline const TValue& GetValueByIndex(int index) const noexcept
	{
		return Values[index];
	}


	inline void Clear(bool bFree = false) noexcept
	{
		Hashes.Clear(bFree);
		Keys.Clear(bFree);
		Values.Clear(bFree);
	}


	inline int GetCount() const noexcept
	{
		return Hashes.GetCount();
	}


	inline bool IsEmpty() const noexcept
	{
		return Hashes.IsEmpty();
	}


private:
	inline int FindIndex(uint64_t hashValue) const noexcept
	{
		for (int i = 0; i < Hashes.GetCount(); ++i)
		{
			if (Hashes[i] == hashValue)
			{
				return i;
			}
		}

		return RPG_INDEX_INVALID;
	}


private:
	RpgArray<uint64_t, CAPACITY_ALIGNMENT> Hashes;
	RpgArray<TKey, CAPACITY_ALIGNMENT> Keys;
	RpgArray<TValue, CAPACITY_ALIGNMENT> Values;

};
