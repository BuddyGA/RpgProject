#pragma once

#include "RpgPointer.h"
#include "RpgStream.h"



class RpgProperty;
class RpgClass;
class RpgObject;



class RpgProperty
{
public:
	RpgProperty() noexcept
	{
		Name = nullptr;
		Offset = UINT64_MAX;
		Type = nullptr;
		bIsTransient = false;
	}


	RpgProperty(const char* in_Name, uint64_t in_Offset, const RpgType* in_Type, bool in_bIsTransient) noexcept
	{
		Name = in_Name;
		Offset = in_Offset;
		Type = in_Type;
		bIsTransient = in_bIsTransient;
	}


	virtual ~RpgProperty() noexcept = default;
	virtual void StreamWrite(const RpgObject* obj, RpgStreamWriter& writer) const noexcept = 0;
	virtual void StreamRead(RpgObject* obj, RpgStreamReader& reader) noexcept = 0;


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

	inline void* GetValuePointer(RpgObject* obj) noexcept
	{
		RPG_Assert(obj);
		return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(obj) + Offset);
	}

	inline const void* GetValuePointer(const RpgObject* obj) const noexcept
	{
		RPG_Assert(obj);
		return reinterpret_cast<const void*>(reinterpret_cast<const uint8_t*>(obj) + Offset);
	}

	template<typename T>
	inline T* GetValue(RpgObject* obj) noexcept
	{
		return static_cast<T*>(GetValuePointer(obj));
	}

	template<typename T>
	inline const T* GetValue(const RpgObject* obj) const noexcept
	{
		return static_cast<const T*>(GetValuePointer(obj));
	}

	inline RpgString ToString() const noexcept
	{
		return RpgString::Format("PropertyName: %s, PropertyType: %s, ArrayType: %s, PointerType: %s",
			Name, Type->GetName(), 
			Type->IsArray() ? Type->GetArrayType()->GetName() : "null",
			Type->IsPointer() ? Type->GetPointerType()->GetName() : "null"
		);
	}


protected:
	const char* Name;
	uint64_t Offset;
	const RpgType* Type;
	bool bIsTransient;

};



template<typename T>
class RpgPropertyType : public RpgProperty
{
	static_assert(!std::is_pointer<T>::value, "Type must not a pointer!");

public:
	RpgPropertyType(const char* in_Name, uint32_t in_Offset, bool in_bIsTransient) noexcept
		: RpgProperty(in_Name, in_Offset, &Rpg::Type<T>::Value, in_bIsTransient)
	{
	}

	
	virtual void StreamWrite(const RpgObject* obj, RpgStreamWriter& writer) const noexcept
	{
		RPG_Assert(!Type->IsPointer());

		const T& value = *GetValue<T>(obj);
		writer.Write(value);
	}


	virtual void StreamRead(RpgObject* obj, RpgStreamReader& reader) noexcept
	{
		RPG_Assert(!Type->IsPointer());

		T& value = *GetValue<T>(obj);
		reader.Read(value);
	}

};




class RpgClass
{
	RPG_NOCOPY(RpgClass)

public:
	RpgClass(const char* in_Name, const RpgClass* in_Parent, const RpgObject* in_DefaultObject, RpgArray<RpgProperty*> in_Properties) noexcept;
	virtual ~RpgClass() noexcept = default;

	void GetProperties(RpgArray<RpgProperty*>& out_Properties) const noexcept;
	bool IsParentOf(const RpgClass* childClass) const noexcept;

	virtual RpgObject* CreateObject(const RpgName& in_Name) const noexcept = 0;
	virtual void DestroyObject(RpgObject* obj) const noexcept = 0;


	inline const char* GetName() const noexcept
	{
		return Name;
	}


	inline const RpgClass* GetParent() const noexcept
	{
		return Parent;
	}


	inline const RpgObject* GetDefaultObject() const noexcept
	{
		return DefaultObject;
	}


private:
	const char* Name;
	const RpgClass* Parent;
	const RpgObject* DefaultObject;
	RpgArray<RpgProperty*> Properties;


private:
	static void Register(const RpgClass* in_Class) noexcept;
	static const RpgClass* Find(const RpgName& name) noexcept;

};


template<typename T>
class RpgClassType : public RpgClass
{
public:
	RpgClassType(const char* in_Name, const RpgClass* in_Parent, const RpgObject* in_DefaultObject, RpgArray<RpgProperty*> in_Properties) noexcept
		: RpgClass(in_Name, in_Parent, in_DefaultObject, in_Properties)
	{
	}


	virtual RpgObject* CreateObject(const RpgName& in_Name) const noexcept override;
	virtual void DestroyObject(RpgObject* obj) const noexcept override;

};



class RpgObject
{
	RPG_NOCOPY(RpgObject)

public:
	static const RpgClass* Class() noexcept
	{
		static RpgObject __default("__default");

		static RpgClassType<RpgObject> __class("RpgObject", nullptr, &__default,
			{
				new RpgPropertyType<RpgName>("Name", offsetof(RpgObject, Name), false)
			}
		);

		return &__class;
	}


private:
	RpgObject(const RpgName& in_Name) noexcept
	{
		Name = in_Name;
	}


public:
	RpgObject() noexcept = default;
	virtual ~RpgObject() noexcept = default;

	virtual void StreamWrite(RpgStreamWriter& writer) const;
	virtual void StreamRead(RpgStreamReader& reader);


	virtual const RpgClass* GetClass() const noexcept
	{
		return Class();
	}


	template<typename T>
	inline bool IsClass() const noexcept
	{
		const RpgClass* objClass = GetClass();
		const RpgClass* checkClass = T::Class();

		// try actual class
		// try up-cast
		return (checkClass == objClass) || (checkClass->IsParentOf(objClass));
	}


public:
	RpgName Name;

};



template<typename T>
inline RpgObject* RpgClassType<T>::CreateObject(const RpgName& in_Name) const noexcept
{
	T* obj = reinterpret_cast<T*>(RpgPlatformMemory::Malloc(sizeof(T)));
	obj->Name = in_Name;
	new (obj)T();

	return obj;
}


template<typename T>
inline void RpgClassType<T>::DestroyObject(RpgObject* obj) const noexcept
{
	RPG_Assert(obj && obj->GetClass() == this);
	delete static_cast<T*>(obj);
}




#define RPG_CLASS_BEGIN(classType, parentClassType)										\
	using ThisClass = classType;														\
	using Super = parentClassType;														\
public:																					\
	static const RpgClass* Class() noexcept												\
	{																					\
		static classType __default("__default");										\
		static RpgClassType<classType> __class(#classType, Super::Class(), &__default,	\
			{

#define RPG_PROPERTY(type, name)			new RpgPropertyType<type>(#name, offsetof(ThisClass, name), false),
#define RPG_PROPERTY_Transient(type, name)	new RpgPropertyType<type>(#name, offsetof(ThisClass, name), true),

#define RPG_CLASS_END(classType) 													\
			}																		\
		);																			\
		return &__class;															\
	}																				\
public:																				\
	virtual const RpgClass* GetClass() const noexcept override						\
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

		RpgClass* objClass = const_cast<RpgClass*>(obj->GetClass());
		objClass->DestroyObject(obj);
	}
	

	template<typename T>
	inline T* ObjectCast(RpgObject* obj) noexcept
	{
		static_assert(std::is_base_of<RpgObject, T>::value, "Rpg::ObjectCast type of <T> must be derived from type <RpgObject>!");

		return (obj && obj->IsClass<T>()) ? static_cast<T*>(obj) : nullptr;
	}

};



class RpgTestObject : public RpgObject
{
	RPG_CLASS_BEGIN(RpgTestObject, RpgObject)
		RPG_PROPERTY(RpgArray<RpgObject*>, Objects)
	RPG_CLASS_END(RpgTestObject)

public:
	RpgTestObject() noexcept = default;


private:
	RpgArray<RpgObject*> Objects;

};
