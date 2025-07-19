#pragma once

#include "dsa/RpgArray.h"



template<typename...TArgs>
class RpgFunction
{
public:
	RpgFunction() noexcept = default;
	virtual void Execute(TArgs...args) noexcept = 0;

};



template<typename T, typename...TArgs>
class RpgObjectFunction : public RpgFunction<TArgs...>
{
public:
	typedef void(T::*FFunction)(TArgs...);


public:
	RpgObjectFunction(T* inObject, FFunction inFunction) noexcept
		: Object(inObject)
		, Function(inFunction)
	{
	}

	virtual void Execute(TArgs... args) noexcept override
	{
		(Object->*Function)(args...);
	}

	inline T* GetObject() const noexcept
	{
		return Object;
	}

	inline FFunction GetFunction() const noexcept
	{
		return Function;
	}


private:
	T* Object;
	FFunction Function;

};



template<typename...TArgs>
class RpgDelegateInterface
{
public:
	typedef void(*FFunction)(TArgs...);


public:
	RpgDelegateInterface() noexcept = default;

	virtual ~RpgDelegateInterface() noexcept
	{
		for (int i = 0; i < ObjectFunctionInvokeList.GetCount(); ++i)
		{
			delete ObjectFunctionInvokeList[i];
		}
	}


	inline void AddStaticFunction(FFunction function) noexcept
	{
		FreeFunctionInvokeList.AddUnique(function);
	}

	inline void RemoveStaticFunction(FFunction function) noexcept
	{
		FreeFunctionInvokeList.Remove(function);
	}


	template<typename T>
	inline void AddObjectFunction(T* obj, typename RpgObjectFunction<T, TArgs...>::FFunction function) noexcept
	{
		if (FindObjectFunction(obj, function) != RPG_INDEX_INVALID)
		{
			return;
		}

		RpgFunction<TArgs...>* callback = new RpgObjectFunction<T, TArgs...>(obj, function);
		ObjectFunctionInvokeList.Add(callback);
	}


	template<typename T>
	inline void RemoveObjectFunction(T* obj, typename RpgObjectFunction<T, TArgs...>::FFunction function) noexcept
	{
		const int index = FindObjectFunction(obj, function);
		
		if (index != RPG_INDEX_INVALID)
		{
			delete ObjectFunctionInvokeList[index];
			ObjectFunctionInvokeList.RemoveAt(index);
		}
	}


protected:
	template<typename...TArgs>
	inline void Broadcast_Implementation(TArgs... args) noexcept
	{
		for (int i = 0; i < FreeFunctionInvokeList.GetCount(); ++i)
		{
			FreeFunctionInvokeList[i](args...);
		}

		for (int i = 0; i < ObjectFunctionInvokeList.GetCount(); ++i)
		{
			ObjectFunctionInvokeList[i]->Execute(args...);
		}
	}


private:
	template<typename T>
	int FindObjectFunction(T* obj, typename RpgObjectFunction<T, TArgs...>::FFunction function) const noexcept
	{
		for (int i = 0; i < ObjectFunctionInvokeList.GetCount(); ++i)
		{
			if (RpgObjectFunction<T, TArgs...>* check = dynamic_cast<RpgObjectFunction<T, TArgs...>*>(ObjectFunctionInvokeList[i]))
			{
				if (check->GetObject() == obj && check->GetFunction() == function)
				{
					return i;
				}
			}
		}

		return RPG_INDEX_INVALID;
	}


protected:
	RpgArrayInline<FFunction, 8> FreeFunctionInvokeList;
	RpgArrayInline<RpgFunction<TArgs...>*, 16> ObjectFunctionInvokeList;

};



class RpgDelegate : public RpgDelegateInterface<>
{
public:
	RpgDelegate() noexcept = default;

	inline void Broadcast() noexcept
	{
		Broadcast_Implementation();
	}

};



#define RPG_DELEGATE_DECLARE_OneParam(delegateType, paramType0, paramName0)	\
class delegateType : public RpgDelegateInterface<paramType0>				\
{																			\
public:																		\
	delegateType() noexcept = default;										\
	inline void Broadcast(paramType0 paramName0) noexcept					\
	{																		\
		Broadcast_Implementation(paramName0);								\
	}																		\
};


#define RPG_DELEGATE_DECLARE_TwoParams(delegateType, paramType0, paramName0, paramType1, paramName1)	\
class delegateType : public RpgDelegateInterface<paramType0, paramType1>								\
{																										\
public:																									\
	delegateType() noexcept = default;																	\
	inline void Broadcast(paramType0 paramName0, paramType1 paramName1) noexcept						\
	{																									\
		Broadcast_Implementation(paramName0, paramName1);												\
	}																									\
};


#define RPG_DELEGATE_DECLARE_ThreeParams(delegateType, paramType0, paramName0, paramType1, paramName1, paramType2, paramName2)	\
class delegateType : public RpgDelegateInterface<paramType0, paramType1, paramType2>											\
{																																\
public:																															\
	delegateType() noexcept = default;																							\
	inline void Broadcast(paramType0 paramName0, paramType1 paramName1, paramType2 paramName2) noexcept							\
	{																															\
		Broadcast_Implementation(paramName0, paramName1, paramName2);															\
	}																															\
};
