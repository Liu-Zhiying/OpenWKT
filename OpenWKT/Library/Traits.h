#ifndef TRAITS_H
#define TRAITS_H

template<typename... Types>
using VOID_TRAIT = void;

template<typename Type, typename Enable = void>
class IsClassType
{
public:
	static constexpr bool Value = false;
};

template<typename Type>
class IsClassType<Type, VOID_TRAIT<void(Type::*)()>>
{
public:
	static constexpr bool Value = true;
};

template<typename Type, typename Enable = void>
class IsPointerType
{
public:
	static constexpr bool Value = false;
};

template<typename Type>
class IsPointerType<Type, VOID_TRAIT<decltype(*static_cast<Type>(nullptr))>>
{
public:
	static constexpr bool Value = true;
};

template<typename Type1, typename Type2, typename Enable = void>
class IsExtendFrom
{
public:
	static constexpr bool Value = false;
};

template<typename Type1, typename Type2>
class IsExtendFrom<Type1, Type2, VOID_TRAIT<decltype(static_cast<Type1*>((Type2*)nullptr))>>
{
public:
	static constexpr bool Value = true;
};

#endif
