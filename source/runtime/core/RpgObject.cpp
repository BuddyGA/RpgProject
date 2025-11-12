#include "RpgObject.h"
#include "RpgMap.h"



namespace Rpg
{
	static RpgMap<RpgName, const RpgClass*> RegisteredClassTable;

};


RpgClass::RpgClass(const char* in_Name, const RpgClass* in_Parent, const RpgObject* in_DefaultObject, RpgArray<RpgProperty*> in_Properties) noexcept
{
	Name = in_Name;
	Parent = in_Parent;
	DefaultObject = in_DefaultObject;
	Properties = in_Properties;

	Register(this);
}


void RpgClass::GetProperties(RpgArray<RpgProperty*>& out_Properties) const noexcept
{
	if (Parent)
	{
		Parent->GetProperties(out_Properties);
	}

	out_Properties.Reserve(out_Properties.GetCount() + Properties.GetCount());

	for (RpgProperty* p : Properties)
	{
		out_Properties.AddValue(p);
	}
}


bool RpgClass::IsParentOf(const RpgClass* childClass) const noexcept
{
	if (childClass == nullptr)
	{
		return false;
	}

	const RpgClass* checkClass = childClass;

	do
	{
		if (this == checkClass)
		{
			return true;
		}

		checkClass = checkClass->Parent;
	}
	while (checkClass);

	return false;
}


void RpgClass::Register(const RpgClass* in_Class) noexcept
{
	RPG_Check(in_Class);

	const RpgName name(in_Class->GetName());
	Rpg::RegisteredClassTable.AddValue(name, in_Class);
}


const RpgClass* RpgClass::Find(const RpgName& name) noexcept
{
	const RpgClass** classPtr = Rpg::RegisteredClassTable.FindValue(name);
	RPG_Check(classPtr);

	return *classPtr;
}



void RpgObject::StreamWrite(RpgStreamWriter& writer) const
{
	RpgArray<RpgProperty*> properties;
	GetClass()->GetProperties(properties);

	for (RpgProperty* p : properties)
	{
		if (p->IsTransient())
		{
			continue;
		}

		p->StreamWrite(this, writer);
	}
}


void RpgObject::StreamRead(RpgStreamReader& reader)
{
	RpgArray<RpgProperty*> properties;
	GetClass()->GetProperties(properties);

	for (RpgProperty* p : properties)
	{
		if (p->IsTransient())
		{
			continue;
		}

		p->StreamRead(this, reader);
	}
}
