#pragma once

#include "RpgPlatform.h"



template<typename T>
using TUniqueDeleteFunction = void(*)(T*);

template<typename T>
inline void Rpg_UniquePtrDefaultDelete(T* ref) noexcept
{
	if (ref)
	{
		delete ref;
	}
}


template<typename T, TUniqueDeleteFunction<T> DeleteFunction = Rpg_UniquePtrDefaultDelete<T>>
class RpgUniquePtr
{
	RPG_NOCOPY(RpgUniquePtr)

public:
	RpgUniquePtr(T* in_Ref = nullptr) noexcept
		: Ref(in_Ref)
	{
	}

	RpgUniquePtr(RpgUniquePtr&& other) noexcept
		: Ref(other.Ref)
	{
		other.Ref = nullptr;
	}

	template<typename U>
	RpgUniquePtr(RpgUniquePtr<U>&& other) noexcept
		: Ref(other.Ref)
	{
		static_assert(std::is_base_of<T, U>::value, "Type of <U> must be derived from type of <T>!");
		other.Ref = nullptr;
	}

	~RpgUniquePtr() noexcept
	{
		Release();
	}


public:
	inline RpgUniquePtr& operator=(RpgUniquePtr&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			Ref = rhs.Ref;
			rhs.Ref = nullptr;
		}

		return *this;
	}

	template<typename U>
	inline RpgUniquePtr& operator=(RpgUniquePtr&& rhs) noexcept
	{
		static_assert(std::is_base_of<T, U>::value, "Type of <U> must be derived from type of <T>!");

		Release();
		Ref = rhs.Ref;
		rhs.Ref = nullptr;

		return *this;
	}


	inline RpgUniquePtr& operator=(T* rhs) noexcept
	{
		if (Ref != rhs)
		{
			Release();
			Ref = rhs;
		}

		return *this;
	}

	inline T* operator->() noexcept
	{
		return Ref;
	}

	inline const T* operator->() const noexcept
	{
		return Ref;
	}

	inline bool operator==(const T* rhs) const noexcept
	{
		return Ref == rhs;
	}

	inline operator bool() const noexcept
	{
		return Ref != nullptr;
	}


public:
	inline bool IsValid() const noexcept
	{
		return Ref != nullptr;
	}

	inline void Release() noexcept
	{
		DeleteFunction(Ref);
		Ref = nullptr;
	}

	inline T* Get() noexcept
	{
		return Ref;
	}

	inline const T* Get() const noexcept
	{
		return Ref;
	}


private:
	T* Ref;


	template<typename U, TUniqueDeleteFunction<U>>
	friend class RpgUniquePtr;

};




template<typename T>
struct RpgPointerRefCount
{
	T* Pointer{ nullptr };
	RpgAtomicInt SharedCount;
	RpgAtomicInt WeakCount;
};



template<typename T>
class RpgSharedPtr
{

public:
	explicit RpgSharedPtr(T* in_Obj = nullptr) noexcept
	{
		Ref = nullptr;

		if (in_Obj)
		{
			Ref = new RpgPointerRefCount<T>();
			Ref->Pointer = in_Obj;
			Ref->SharedCount = 1;
			Ref->WeakCount = 0;
		}
	}


	RpgSharedPtr(const RpgSharedPtr& other) noexcept
		: Ref(other.Ref)
	{
		if (Ref)
		{
			InterlockedAdd(&Ref->SharedCount, 1);
		}
	}


	RpgSharedPtr(RpgSharedPtr&& other) noexcept
		: Ref(other.Ref)
	{
		other.Ref = nullptr;
	}


	~RpgSharedPtr() noexcept
	{
		Release();
	}


public:
	inline RpgSharedPtr& operator=(const RpgSharedPtr& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			Ref = rhs.Ref;

			if (Ref)
			{
				InterlockedAdd(&Ref->SharedCount, 1);
			}
		}

		return *this;
	}


	inline RpgSharedPtr& operator=(RpgSharedPtr&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			Ref = rhs.Ref;
			rhs.Ref = nullptr;
		}

		return *this;
	}


	inline bool operator==(const RpgSharedPtr& rhs) const noexcept
	{
		return Ref == rhs.Ref;
	}


	inline bool operator!=(const RpgSharedPtr& rhs) const noexcept
	{
		return !(*this == rhs);
	}


	inline T* operator->() noexcept
	{
		return Ref ? Ref->Pointer : nullptr;
	}

	inline const T* operator->() const noexcept
	{
		return Ref ? Ref->Pointer : nullptr;
	}


	inline operator bool() const noexcept
	{
		return Ref && Ref->Pointer;
	}


public:
	inline bool IsValid() const noexcept
	{
		return Ref && Ref->Pointer;
	}


	inline void Release() noexcept
	{
		if (Ref)
		{
			if (InterlockedAdd(&Ref->SharedCount, -1) == 0)
			{
				delete Ref->Pointer;
				Ref->Pointer = nullptr;

				if (Ref->WeakCount == 0)
				{
					delete Ref;
				}
			}
		}

		Ref = nullptr;
	}


	inline T* Get() noexcept
	{
		return Ref ? Ref->Pointer : nullptr;
	}

	inline const T* Get() const noexcept
	{
		return Ref ? Ref->Pointer : nullptr;
	}

	inline int GetRefCount() const noexcept
	{
		return Ref ? Ref->SharedCount : 0;
	}


	template<typename U>
	[[nodiscard]] inline RpgSharedPtr<U> Cast() const noexcept
	{
		static_assert(std::is_convertible<T*, U*>::value, "RpgSharedPtr: Cast type <T> must be convertible to type <U>!");
		
		if (Ref)
		{
			InterlockedAdd(&Ref->SharedCount, 1);
		}

		RpgSharedPtr<U> parent;
		parent.Ref->Pointer = Ref->Pointer;
		parent.Ref->SharedCount = Ref->SharedCount;
		parent.Ref->WeakCount = Ref->WeakCount;

		return parent;
	}


private:
	RpgPointerRefCount<T>* Ref;


	template<typename U>
	friend class RpgSharedPtr;

	template<typename>
	friend class RpgWeakPtr;

};




template<typename T>
class RpgWeakPtr
{

public:
	RpgWeakPtr() noexcept
	{
		Ref = nullptr;
	}


	explicit RpgWeakPtr(const RpgSharedPtr<T>& shared) noexcept
	{
		Ref = shared.Ref;

		if (Ref)
		{
			InterlockedAdd(&Ref->WeakCount, 1);
		}
	}


	RpgWeakPtr(const RpgWeakPtr& other) noexcept
	{
		Ref = other.Ref;

		if (Ref)
		{
			InterlockedAdd(&Ref->WeakCount, 1);
		}
	}


	RpgWeakPtr(RpgWeakPtr&& other) noexcept
	{
		Ref = other.Ref;
		other.Ref = nullptr;
	}


	~RpgWeakPtr() noexcept
	{
		Release();
	}


public:
	inline RpgWeakPtr& operator=(const RpgSharedPtr<T>& rhs) noexcept
	{
		Release();
		Ref = rhs.Ref;

		if (Ref)
		{
			InterlockedAdd(&Ref->WeakCount, 1);
		}

		return *this;
	}


	inline RpgWeakPtr& operator=(const RpgWeakPtr& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			Ref = rhs.Ref;

			if (Ref)
			{
				InterlockedAdd(&Ref->WeakCount, 1);
			}
		}

		return *this;
	}


	inline RpgWeakPtr& operator=(RpgWeakPtr&& rhs) noexcept
	{
		if (this != &rhs)
		{
			Release();
			Ref = rhs.Ref;
			rhs.Ref = nullptr;
		}

		return *this;
	}


	inline bool operator==(const RpgWeakPtr& rhs) const noexcept
	{
		return Ref == rhs.Ref;
	}

	inline bool operator!=(const RpgWeakPtr& rhs) const noexcept
	{
		return Ref != rhs.Ref;
	}

	inline bool operator==(const RpgSharedPtr<T>& rhs) noexcept
	{
		return Ref == rhs.Ref;
	}


public:
	inline void Release() noexcept
	{
		if (Ref)
		{
			if ( (InterlockedAdd(&Ref->WeakCount, -1) == 0) && (Ref->SharedCount == 0) )
			{
				RPG_Check(Ref->Pointer == nullptr);
				delete Ref;
			}
		}

		Ref = nullptr;
	}


	[[nodiscard]] inline RpgSharedPtr<T> AsShared() const noexcept
	{
		RpgSharedPtr<T> shared;

		if (Ref == nullptr)
		{
			return shared;
		}

		int expected = 0;
		int exchange = 0;

		do
		{
			expected = Ref->SharedCount;
			if (expected == 0)
			{
				return shared;
			}

			exchange = expected + 1;
		}
		while (InterlockedCompareExchange(&Ref->SharedCount, exchange, expected) != expected);

		shared.Ref = Ref;
		RPG_Check(shared.Ref && shared.Ref->Pointer);

		return shared;
	}


private:
	RpgPointerRefCount<T>* Ref;

};




namespace RpgPointer
{
	template<typename T, typename...TConstructorArgs>
	[[nodiscard]] inline RpgUniquePtr<T> MakeUnique(TConstructorArgs&&... args) noexcept
	{
		return RpgUniquePtr<T>(new T(std::forward<TConstructorArgs>(args)...));
	}


	template<typename T, typename...TConstructorArgs>
	[[nodiscard]] inline RpgSharedPtr<T> MakeShared(TConstructorArgs&&... args) noexcept
	{
		return RpgSharedPtr<T>(new T(std::forward<TConstructorArgs>(args)...));
	}

};
