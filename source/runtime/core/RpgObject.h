#pragma once

#include "RpgString.h"
#include "RpgMath.h"
#include "RpgPointer.h"



class RpgObjectProperty;
class RpgObjectClass;
class RpgObject;



class RpgObjectProperty
{
public:
	RpgObjectProperty() noexcept
	{
		Name = nullptr;
		Offset = UINT64_MAX;
		Type = nullptr;
		bIsTransient = false;
	}


	RpgObjectProperty(const char* in_Name, uint64_t in_Offset, const RpgType* in_Type, bool in_bIsTransient) noexcept
	{
		Name = in_Name;
		Offset = in_Offset;
		Type = in_Type;
		bIsTransient = in_bIsTransient;
	}


	virtual ~RpgObjectProperty() noexcept = default;


	inline const char* GetName() const noexcept
	{
		return Name;
	}

	inline uint64_t GetOffset() const noexcept
	{
		return Offset;
	}

	inline const RpgType* GetType() const noexcept
	{
		return Type;
	}

	inline bool IsTransient() const noexcept
	{
		return bIsTransient;
	}


private:
	const char* Name;
	uint64_t Offset;
	const RpgType* Type;
	bool bIsTransient;

};




class RpgObjectClass
{
	RPG_NOCOPY(RpgObjectClass)

private:
	static void Register(const RpgObjectClass* in_Class) noexcept;
	static const RpgObjectClass* Find(const RpgName& name) noexcept;


public:
	RpgObjectClass(const char* in_Name, const RpgObjectClass* in_Parent, const RpgObject* in_DefaultObject, RpgArray<RpgObjectProperty> in_Properties) noexcept
	{
		Name = in_Name;
		Parent = in_Parent;
		DefaultObject = in_DefaultObject;
		Properties = in_Properties;

		Register(this);
	}

	virtual ~RpgObjectClass() noexcept = default;


	void GetProperties(RpgArray<const RpgObjectProperty*>& out_Properties) const noexcept
	{
		if (Parent)
		{
			Parent->GetProperties(out_Properties);
		}

		out_Properties.Reserve(out_Properties.GetCount() + Properties.GetCount());

		for (const RpgObjectProperty& p : Properties)
		{
			out_Properties.AddValue(&p);
		}
	}


	inline const char* GetName() const noexcept
	{
		return Name;
	}


	inline const RpgObjectClass* GetParent() const noexcept
	{
		return Parent;
	}


	inline const RpgObject* GetDefaultObject() const noexcept
	{
		return DefaultObject;
	}


	bool IsParentOf(const RpgObjectClass* childClass) const noexcept
	{
		if (childClass == nullptr)
		{
			return false;
		}

		const RpgObjectClass* checkClass = childClass;

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


	virtual RpgObject* CreateObject(const RpgName& in_Name) noexcept = 0;
	virtual void DestroyObject(RpgObject* obj) noexcept = 0;


private:
	const char* Name;
	const RpgObjectClass* Parent;
	const RpgObject* DefaultObject;
	RpgArray<RpgObjectProperty> Properties;

};


template<typename T>
class RpgClassType : public RpgObjectClass
{
public:
	RpgClassType(const char* in_Name, const RpgObjectClass* in_Parent, const RpgObject* in_DefaultObject, RpgArray<RpgObjectProperty> in_Properties) noexcept
		: RpgObjectClass(in_Name, in_Parent, in_DefaultObject, in_Properties)
	{
	}


	virtual RpgObject* CreateObject(const RpgName& in_Name) noexcept override;
	virtual void DestroyObject(RpgObject* obj) noexcept override;

};




class RpgObject
{
	RPG_NOCOPY(RpgObject)

public:
	static const RpgObjectClass* Class() noexcept
	{
		static RpgObject __default("__default");

		static RpgClassType<RpgObject> __class("RpgObject", nullptr, &__default,
			{
				RpgObjectProperty("Name", offsetof(RpgObject, Name), &Rpg::Type<RpgName>::Value, false)
			}
		);

		return &__class;
	}


private:
	RpgObject(const RpgName& in_Name) noexcept
		: RpgObject()
	{
		Name = in_Name;
	}


public:
	RpgObject() noexcept = default;
	virtual ~RpgObject() noexcept = default;


	virtual const RpgObjectClass* GetClass() const noexcept
	{
		return Class();
	}


	template<typename T>
	inline bool IsClass() const noexcept
	{
		const RpgObjectClass* objClass = GetClass();
		const RpgObjectClass* checkClass = T::Class();

		// try actual class
		// try up-cast
		return (checkClass == objClass) || (checkClass->IsParentOf(objClass));
	}


public:
	RpgName Name;

};



template<typename T>
inline RpgObject* RpgClassType<T>::CreateObject(const RpgName& in_Name) noexcept
{
	T* obj = reinterpret_cast<T*>(RpgPlatformMemory::MemMalloc(sizeof(T)));
	obj->Name = in_Name;
	new (obj)T();

	return obj;
}


template<typename T>
inline void RpgClassType<T>::DestroyObject(RpgObject* obj) noexcept
{
	RPG_Assert(obj && obj->GetClass() == this);
	delete static_cast<T*>(obj);
}



#define RPG_CLASS_BEGIN(classType, parentClassType)										\
	using ThisClass = classType;														\
	using Super = parentClassType;														\
public:																					\
	static const RpgObjectClass* Class() noexcept												\
	{																					\
		static classType __default("__default");										\
		static RpgClassType<classType> __class(#classType, Super::Class(), &__default,	\
			{

#define RPG_PROPERTY(propertyType, propertyName)					RpgObjectProperty(#propertyName, offsetof(ThisClass, propertyName), &Rpg::Type<propertyType>::Value, false)
#define RPG_PROPERTY_Transient(propertyType, propertyName)			RpgObjectProperty(#propertyName, offsetof(ThisClass, propertyName), &Rpg::Type<propertyType>::Value, true)

#define RPG_CLASS_END(classType) 													\
			}																		\
		);																			\
		return &__class;															\
	}																				\
public:																				\
	classType() noexcept;															\
	virtual const RpgObjectClass* GetClass() const noexcept override				\
	{																				\
		return Class();																\
	}																				\
private:																			\
	classType(const RpgName& in_Name) noexcept										\
		: classType()																\
	{																				\
		Name = in_Name;																\
	}




namespace Rpg
{
	template<typename T>
	inline T* ObjectCreate(const RpgName& in_Name) noexcept
	{
		return static_cast<T*>(T::Class()->CreateObject(in_Name));
	}


	inline void ObjectDestroy(RpgObject* obj) noexcept
	{
		if (obj == nullptr)
		{
			return;
		}

		RpgObjectClass* objClass = const_cast<RpgObjectClass*>(obj->GetClass());
		objClass->DestroyObject(obj);
	}
	

	template<typename T>
	inline T* ObjectCast(RpgObject* obj) noexcept
	{
		static_assert(std::is_base_of<RpgObject, T>::value, "Rpg::ObjectCast type of <T> must be derived from type <RpgObject>!");

		return (obj && obj->IsClass<T>()) ? static_cast<T*>(obj) : nullptr;
	}


	template<typename T>
	inline T* ObjectCastCheck(RpgObject* obj) noexcept
	{
		static_assert(std::is_base_of<RpgObject, T>::value, "Rpg::ObjectCast type of <T> must be derived from type <RpgObject>!");
		
		RPG_Check(obj && obj->IsClass<T>());

		return static_cast<T*>(obj);
	}

};



class RpgAsset : public RpgObject
{
	RPG_CLASS_BEGIN(RpgAsset, RpgObject)
		RPG_PROPERTY(RpgArray<RpgObject*>, Objects)
	RPG_CLASS_END(RpgAsset)

private:
	RpgArray<RpgObject*> Objects;

};
