#include "RpgObject.h"
#include "RpgMap.h"


static RpgMap<RpgName, const RpgObjectClass*> s_RegisteredClassTable;


void RpgObjectClass::Register(const RpgObjectClass* in_Class) noexcept
{
	RPG_Check(in_Class);

	const RpgName name(in_Class->GetName());
	s_RegisteredClassTable.AddValue(name, in_Class);
}


const RpgObjectClass* RpgObjectClass::Find(const RpgName& name) noexcept
{
	const RpgObjectClass** classPtr = s_RegisteredClassTable.FindValue(name);
	RPG_Check(classPtr);

	return *classPtr;
}
