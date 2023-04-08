#ifndef _DATA_ORIENTED_HELPERS_H_
#define _DATA_ORIENTED_HELPERS_H_

#include <cstdio>

template <std::size_t N, typename T0, typename ... Ts>
struct typeN
{
	using type = typename typeN<N - 1U, Ts...>::type;
};

template <typename T0, typename ... Ts>
struct typeN<0U, T0, Ts...>
{
	using type = T0;
};

template <std::size_t, typename>
struct argN;

template <std::size_t N, typename R, typename ... As>
struct argN<N, R(As...)>
{
	using type = typename typeN<N, As...>::type;
};

template <std::size_t N, typename R, typename CLASS, typename ... As>
struct argN<N, R(CLASS::*)(As...)>
{
	using type = typename typeN<N, As...>::type;
};

template <typename>
struct returnType;

template <typename R, typename ... As>
struct returnType<R(As...)>
{
	using type = R;
};

template <typename R, typename CLASS, typename ... As>
struct returnType<R(CLASS::*)(As...)>
{
	using type = R;
};


template <std::size_t, typename>
struct templateElem;

template <std::size_t N, template<typename> typename T, typename ... As>
struct templateElem<N, T<As...>>
{
	using type = typename typeN<N, As...>::type;
};

#define IMPLEMENT_KEY(ClassName, keyName, additional)			\
	class ClassName												\
	{															\
	private:													\
		unsigned int keyName = 0;								\
		inline unsigned int GetID() const { return keyName; }	\
																\
	public:														\
		inline bool operator==(const ClassName& rhs) const		\
		{														\
			return keyName == rhs.keyName;						\
		}														\
		inline bool operator!=(const ClassName& rhs) const		\
		{														\
			return !(*this == rhs);								\
		}														\
																\
		additional												\
	};

#define GENERATE_MAKE(FunctionName, Type)						\
	inline Type FunctionName(unsigned int index)				\
	{															\
		return Type(*this, index);								\
	}

#define GENERATE_FOREACH_SIGNATURE(FunctionName, arrayName, MakeFunction)	\
	template<typename FUNCTOR/*(decltype(MadeFunct()))*/>					\
	void FunctionName(FUNCTOR&& functor)									\
	{																		\
		for (unsigned int i = 0; i < arrayName.size(); i++)					\
		{																	\
			auto madeObject = MakeFunction(i);								\
			functor(madeObject);											\
		}																	\
	}	

#define GENERATE_REVERSED_FOREACH_SIGNATURE(FunctionName, arrayName, MakeFunction)	\
	template<typename FUNCTOR/*(decltype(MadeFunct()))*/>							\
	void FunctionName(FUNCTOR&& functor)											\
	{																				\
		for (unsigned int i = arrayName.size(); i > 0;)								\
		{																			\
			i--;																	\
			auto madeObject = MakeFunction(i);										\
			functor(madeObject);													\
		}																			\
	}	

#define GENERATE_FIND(FunctionName, arrayName, MakeFunction)			\
	bool FunctionName(templateElem<0, decltype(arrayName)>::type& signature, returnType<decltype(&MakeFunction)>::type& obj)		\
	{																				\
		for (unsigned int index = 0; index < arrayName.size(); index++)				\
		{																			\
			if (arrayName[index].GetID() == signature.GetID())						\
			{																		\
				obj = MakeFunction(index);											\
				return true;														\
			}																		\
		}																			\
		return false;																\
	}

#define GENERATED_FIND_OR_ADD(FunctionName, FindFunctionName, AddFunctionName)														\
	returnType<decltype(&AddFunctionName)>::type FunctionName(typename argN<0U, decltype(&FindFunctionName)>::type signature)		\
	{																																\
		decltype(AddFunctionName()) obj;																							\
		if (FindFunctionName(signature, obj))																						\
		{																															\
			return obj;																												\
		}																															\
		else																														\
		{																															\
			obj = AddFunctionName();																								\
			signature = obj.GetSignature();																							\
			return obj;																												\
		}																															\
	}																				

#define GENERATE_TRY_REMOVE(FunctionName, FindFunctionName, RemoveFunctionName)											\
	bool FunctionName(typename argN<0U, decltype(&FindFunctionName)>::type signature)									\
	{																													\
		typename argN<0U, decltype(&std::remove_reference<decltype(*this)>::type::RemoveFunctionName)>::type obj;		\
		if (FindFunctionName(signature, obj))																			\
		{																												\
			RemoveFunctionName(obj);																					\
			return true;																								\
		}																												\
		return false;																									\
	}

#define GENERATE_ALL_UTILITY_MACROS(Type, signatureContainer)								\
	GENERATE_MAKE(Make##Type, Type);														\
	GENERATE_FOREACH_SIGNATURE(ForEach##Type, signatureContainer, Make##Type);				\
	GENERATE_REVERSED_FOREACH_SIGNATURE(RForEach##Type, signatureContainer, Make##Type);	\
	GENERATE_FIND(Find##Type, signatureContainer, Make##Type);								\
	GENERATED_FIND_OR_ADD(FindOrAdd##Type, Find##Type, Add##Type);							\
	GENERATE_TRY_REMOVE(TryRemove##Type, Find##Type, Remove##Type);

#define GENERATE_REDIRECTOR_CHILD_GETSYSTEM(SystemType)						\
	SystemType& GetSystemRef()												\
	{																		\
		return static_cast<SystemType&>(Super::GetSystemRef());				\
	}

#define GENERATE_REDIRECTOR_CHILD_START(Type, ParentType, SystemType)			\
	class Type : public ParentType												\
	{																			\
	public:																		\
		using System = SystemType;												\
		using Super = ParentType;												\
																				\
		inline Type(Super obj) : Super(obj) {}									\
																				\
		GENERATE_REDIRECTOR_CHILD_GETSYSTEM(System);

#define GENERATE_REDIRECTOR_CHILD_END() };

#define GENERATE_REDIRECTOR_GETTER(FunctionKeyword, containerName)			\
	inline templateElem<0, decltype(System::containerName)>::type& Get ##FunctionKeyword ##Ref()			\
	{												\
		assert(IsValid());													\
		return GetSystemRef().containerName[index];							\
	}

#define GENERATE_REDIRECTOR(Type, SystemType, SignatureType, signatureContainer, MakeFunction)										\
	protected:																														\
		using System = SystemType;																									\
		System* system = nullptr;																									\
		unsigned int index = 0;																										\
																																	\
	public:																															\
		inline Type(System& newSystem, unsigned int newIndex) : system(&newSystem), index(newIndex) {}								\
		inline Type() = default;																									\
																																	\
		inline bool IsValid() const																									\
		{																															\
			return system != nullptr;																								\
		}																															\
																																	\
		inline unsigned int GetIndex() const																						\
		{																															\
			assert(IsValid());																										\
			return index;																											\
		}																															\
																																	\
		inline System& GetSystemRef()																								\
		{																															\
			assert(IsValid());																										\
			return *system;																											\
		}																															\
																																	\
		inline SignatureType GetSignature() const																					\
		{																															\
			assert(IsValid());																										\
			return system->signatureContainer[index];																				\
		}																															\
																																	\
		inline operator SignatureType() const																						\
		{																															\
			return GetSignature();																									\
		}																															\
																																	\
		friend Type System::MakeFunction(unsigned int index);

template<typename SYSTEM>
class DataOrientedObjectRedirector
{
protected:
	using System = SYSTEM;
	System* system = nullptr;
	unsigned int index = 0;

public:
	inline DataOrientedObjectRedirector(System& newSystem, unsigned int newIndex) : system(&newSystem), index(newIndex) {}
	inline DataOrientedObjectRedirector() = default;

	inline bool IsValid() const
	{
		return system != nullptr;
	}

	inline unsigned int GetIndex() const
	{
		assert(IsValid());
		return index;
	}

	inline System& GetSystemRef()
	{
		assert(IsValid());
		return *system;
	}
};

template<typename T>
void QuickRemove(std::vector<T>& container, size_t index)
{
	container[index] = std::move(container[container.size() - 1]);
	container.pop_back();
}

#endif