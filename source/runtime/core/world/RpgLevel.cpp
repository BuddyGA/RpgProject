#include "RpgLevel.h"
#include "RpgWorld.h"



RpgLevel::RpgLevel(const RpgName& in_Name) noexcept
	: Name(in_Name)
	, World(nullptr)
{
	RPG_Log(RpgLogWorld, "Create level (%s)", *Name);

}


RpgLevel::~RpgLevel() noexcept
{
	RPG_Log(RpgLogWorld, "Destroy level (%s)", *Name);
}


void RpgLevel::StreamWrite(RpgStreamWriter& writer) const noexcept
{
	RPG_Check(World);

	writer.Write(Name);
	writer.Write(Bound);
	writer.Write(WorldMatrix);
	writer.Write(InverseWorldMatrix);

	const int count = GameObjects.GetCount();
	writer.Write(count);

	for (int i = 0; i < count; ++i)
	{
		World->GameObject_StreamWrite(GameObjects[i], writer);
	}
}


void RpgLevel::StreamRead(RpgStreamReader& reader) noexcept
{
	RPG_Check(World);

	reader.Read(Name);
	reader.Read(Bound);
	reader.Read(WorldMatrix);
	reader.Read(InverseWorldMatrix);

	GameObjects.Clear(true);

	int count = 0;
	reader.Read(count);

	for (int i = 0; i < count; ++i)
	{
		const RpgGameObjectID gameObject = World->GameObject_Create("");
		World->GameObject_StreamRead(gameObject, reader);
		World->GameObject_Spawn(gameObject, this);
	}
}


void RpgLevel::AddGameObject(RpgGameObjectID gameObject) noexcept
{
#ifndef RPG_BUILD_SHIPPING
	const bool bAdded = GameObjects.AddUnique(gameObject);
	RPG_Check(bAdded);
#else
	GameObjects.AddValue(gameObject);
#endif // !RPG_BUILD_SHIPPING


	UpdateBound();
}


void RpgLevel::RemoveGameObject(RpgGameObjectID gameObject) noexcept
{
	if (GameObjects.RemoveByValue(gameObject, false))
	{
		UpdateBound();
	}
}


void RpgLevel::UpdateBound() noexcept
{

}
