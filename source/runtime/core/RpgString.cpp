#include "RpgString.h"



RpgStringID::FHashTable RpgStringID::HashTable;


RpgStringID::FHashTable::FHashTable() noexcept
{
	Hashes.Reserve(128);
	Entries.Reserve(128);
	StringPool.Reserve(1024);
	InitializeSRWLock(&Lock);
}


void RpgStringID::FHashTable::Allocate(const char* cstr, int& out_Index, bool bIsUnique, int* optOut_UniqueId) noexcept
{
	const int len = RpgPlatformString::CStringLength(cstr);
	if (len == 0)
	{
		out_Index = RPG_INDEX_INVALID;

		if (optOut_UniqueId)
		{
			*optOut_UniqueId = 0;
		}

		return;
	}

	RPG_Check(len < 256);
	char lowerStr[256];
	RpgPlatformString::CStringCopy(lowerStr, cstr);
	RpgPlatformString::CStringToLower(lowerStr, len);

	const uint64_t hash = RpgPlatformString::CStringHash(lowerStr);

	AcquireSRWLockExclusive(&Lock);
	{
		int index = Hashes.FindIndexByValue(hash);
		int uniqueId = 0;

		if (index == RPG_INDEX_INVALID)
		{
			index = Hashes.GetCount();
			uniqueId = 0;

			// add new hash
			Hashes.AddValue(hash);

			// add new entry
			FEntry& newEntry = Entries.Add();
			newEntry.StringIndex = StringPool.GetCount();
			newEntry.UniqueId = 0;

			// add to pool (+1 for null terminator)
			StringPool.InsertAtRange(cstr, len + 1, RPG_INDEX_LAST);
		}
		else
		{
			FEntry& entry = Entries[index];
			uniqueId = bIsUnique ? ++entry.UniqueId : 0;
		}
		
		RPG_Check(index != RPG_INDEX_INVALID);
		out_Index = index;

		if (optOut_UniqueId)
		{
			*optOut_UniqueId = uniqueId;
		}
	}
	ReleaseSRWLockExclusive(&Lock);
}


RpgString RpgStringID::FHashTable::ConstructString(int index, int uniqueId) const noexcept
{
	RpgString str;

	AcquireSRWLockShared(&Lock);
	{
		RPG_Check(index >= 0 && index < Entries.GetCount());

		const FEntry& entry = Entries[index];
		const char* src = &StringPool[entry.StringIndex];
		str = (uniqueId > 0) ? RpgString::Format("%s_%i", src, uniqueId) : RpgString::Format("%s", src);
	}
	ReleaseSRWLockShared(&Lock);

	return str;
}
