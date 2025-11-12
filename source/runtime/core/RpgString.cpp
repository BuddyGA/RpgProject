#include "RpgString.h"



RpgStringPool::RpgStringPool() noexcept
{
	Pool.Reserve(1024);
	Hashes.Reserve(32);
	Entries.Reserve(32);
	InitializeSRWLock(&Lock);
}


void RpgStringPool::Allocate(const char* cstr, int& out_Index, bool bIsInstance, int* optOut_Instance) noexcept
{
	const int len = RpgPlatformString::CStringLength(cstr);
	if (len == 0)
	{
		out_Index = RPG_INDEX_INVALID;

		if (optOut_Instance)
		{
			*optOut_Instance = 0;
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
		int entryIndex = Hashes.FindIndexByValue(hash);
		int instance = 0;

		if (entryIndex == RPG_INDEX_INVALID)
		{
			entryIndex = Pool.GetCount();
			instance = 0;

			// add new hash
			Hashes.AddValue(hash);

			// add new entry
			FEntry& newEntry = Entries.Add();
			newEntry.Index = entryIndex;
			newEntry.Count = len + 1;
			newEntry.Instance = 0;

			// add to pool
			Pool.InsertAtRange(cstr, newEntry.Count, RPG_INDEX_LAST);
		}
		else
		{
			FEntry& entry = Entries[entryIndex];
			instance = bIsInstance ? ++entry.Instance : 0;
		}
		
		RPG_Check(entryIndex != RPG_INDEX_INVALID);
		out_Index = entryIndex;

		if (optOut_Instance)
		{
			*optOut_Instance = instance;
		}
	}
	ReleaseSRWLockExclusive(&Lock);
}


RpgString RpgStringPool::ConstructString(int index, int instance) const noexcept
{
	RpgString str;

	AcquireSRWLockShared(&Lock);
	{
		RPG_Check(index >= 0 && index < Entries.GetCount());

		const FEntry& entry = Entries[index];
		const char* src = &Pool[entry.Index];
		str = (instance > 0) ? RpgString::Format("%s_%i", src, instance) : RpgString::Format("%s", src);
	}
	ReleaseSRWLockShared(&Lock);

	return str;
}
