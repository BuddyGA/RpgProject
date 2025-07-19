#pragma once

#include "RpgArray.h"



// ============================================================================================================================================================================================== //
// RpgMap
// Hash map, key-value pair.
// ============================================================================================================================================================================================== //
#define RPG_MAP_CHECK_COLLISION	1

template<typename TUniqueKey, typename TValue>
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
	inline TValue& operator[](const TUniqueKey& key) noexcept
	{
		return Add(key);
	}
	*/


	inline const TValue& operator[](const TUniqueKey& key) const noexcept
	{
		const int index = FindIndex(Rpg_GetHash(key));
		RPG_ValidateV(index != RPG_INDEX_INVALID, "RpgMap key not found!");

		return Values[index];
	}


public:
	inline void Reserve(int newCapacity) noexcept
	{
		Hashes.Reserve(newCapacity);
		Keys.Reserve(newCapacity);
		Values.Reserve(newCapacity);
	}


	inline TValue& Add(const TUniqueKey& key, int* outIndex = nullptr) noexcept
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
			const int keyIndex = Keys.FindFirstIndexOf(key);
			RPG_ValidateV(keyIndex != RPG_INDEX_INVALID, "RpgMap collision!");
		#endif // RPG_MAP_CHECK_COLLISION
		}

		if (outIndex)
		{
			*outIndex = index;
		}

		return Values[index];
	}


	template<typename...TConstructorArgs>
	inline void Add(const TUniqueKey& key, TConstructorArgs&&... args) noexcept
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


	inline void Remove(const TUniqueKey& key, bool bKeepOrder = false) noexcept
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


	inline int FindKeyIndex(const TUniqueKey& key) const noexcept
	{
		return FindIndex(Rpg_GetHash(key));
	}


	inline bool Exists(const TUniqueKey& key, int* optOut_Index = nullptr) const noexcept
	{
		const int index = FindIndex(Rpg_GetHash(key));

		if (optOut_Index)
		{
			*optOut_Index = index;
		}

		return index != RPG_INDEX_INVALID;
	}


	inline TUniqueKey& GetKeyByIndex(int index) noexcept
	{
		return Keys[index];
	}


	inline const TUniqueKey& GetKeyByIndex(int index) const noexcept
	{
		return Keys[index];
	}


	inline const RpgArray<TUniqueKey>& GetKeyArray() const noexcept
	{
		return Keys;
	}


	inline TValue* GetValueByKey(const TUniqueKey& key) noexcept
	{
		const int index = FindIndex(Rpg_GetHash(key));

		if (index == RPG_INDEX_INVALID)
		{
			return nullptr;
		}

		return &Values[index];
	}


	inline const TValue* GetValueByKey(const TUniqueKey& key) const noexcept
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


	inline RpgArray<TValue>& GetValueArray() noexcept
	{
		return Values;
	}


	inline const RpgArray<TValue>& GetValueArray() const noexcept
	{
		return Values;
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
	RpgArray<uint64_t> Hashes;
	RpgArray<TUniqueKey> Keys;
	RpgArray<TValue> Values;

};
