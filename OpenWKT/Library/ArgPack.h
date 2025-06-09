#ifndef ARGPACK_H
#define ARGPACK_H

class IArgPack {
private:
	virtual void unused() = 0;
};

template<typename T1, typename T2>
class ArgPack : public IArgPack
{
private:
	virtual void unused() override {}

	//ǿ������T1ΪIArgPack������
	using ParentType = decltype(*static_cast<IArgPack*>(reinterpret_cast<T1*>(NULL)));

public:

	static constexpr unsigned int argCount = 2;

	//��һ��������
	T1 nextArgPack;
	//����
	T2 arg;

	ArgPack(T2 _arg, T1&& _nextArgPack) : arg(_arg), nextArgPack(static_cast<T1&&>(_nextArgPack)) {}
	ArgPack(T2&& _arg, T1&& _nextArgPack) : arg(static_cast<T2&&>(_arg)), nextArgPack(static_cast<T1&&>(_nextArgPack)) {}
};

template<typename T>
class ArgPack<void, T> : public IArgPack
{
private:
	virtual void unused() override {}

	using ParentType = IArgPack;

public:
	T arg;

	static constexpr unsigned int argCount = 1;

	ArgPack(T _arg) : arg(_arg) {}
	ArgPack(T&& _arg) : arg(static_cast<T&&>(_arg)) {}
};

template<>
class ArgPack<void, void> : public IArgPack
{
private:
	virtual void unused() override {}

	using ParentType = IArgPack;

public:

	static constexpr unsigned int argCount = 0;
};

class PackArgs
{
	class PackArgInternal
	{
	public:

		template<typename Arg, typename ArgPackT>
		static constexpr ArgPack<ArgPackT, Arg> pack(ArgPackT _argPack, Arg arg)
		{
			ArgPack<ArgPackT, Arg> argPack(arg, static_cast<ArgPackT&&>(_argPack));

			return argPack;
		}

		template <typename Arg, typename ArgPackT, typename... Args>
		static constexpr auto pack(ArgPackT&& _argPack, Arg arg, Args ...args)
		{
			ArgPack<ArgPackT, Arg> argPack(arg, static_cast<ArgPackT&&>(_argPack));

			return pack(static_cast<ArgPack<ArgPackT, Arg>&&>(argPack), args...);
		}
	};

public:

	template <typename Arg, typename... Args>
	static constexpr auto pack(Arg arg, Args ...args)
	{
		ArgPack<void, Arg> argPack(arg);

		return PackArgInternal::pack(static_cast<ArgPack<void, Arg>&&>(argPack), args...);
	}

	template<typename Arg>
	static constexpr ArgPack<void, Arg> pack(Arg arg)
	{
		return ArgPack<void, Arg>(arg);
	}

	static constexpr ArgPack<void, void> pack()
	{
		return ArgPack<void, void>();
	}
};

class UnpackArgsAndCall
{

	template<typename Func, typename ArgPack, typename ...Args>
	static constexpr void UnpackAndCall(Func func, ArgPack argPack, Args ...arg)
	{
		//ǿ������ArgPackΪIArgPack������
		using ParentType = decltype(*static_cast<IArgPack*>(reinterpret_cast<ArgPack*>(NULL)));

		if constexpr (argPack.argCount == 2)
			UnpackAndCall(func, argPack.nextArgPack, argPack.arg, arg...);
		else if constexpr (argPack.argCount == 1)
			func(argPack.arg, arg...);
	}

	template<typename Ret, typename Func, typename ArgPack, typename ...Args>
	static constexpr Ret UnpackAndCall(Func func, ArgPack argPack, Args ...arg)
	{
		//ǿ������ArgPackΪIArgPack������
		using ParentType = decltype(*static_cast<IArgPack*>(reinterpret_cast<ArgPack*>(NULL)));

		if constexpr (argPack.argCount == 2)
			return UnpackAndCall<Ret>(func, argPack.nextArgPack, argPack.arg, arg...);
		else if constexpr (argPack.argCount == 1)
			return func(argPack.arg, arg...);
	}

public:

	template<typename Func, typename ArgPack>
	static constexpr void UnpackAndCall(Func func, ArgPack argPack)
	{
		//ǿ������ArgPackΪIArgPack������
		using ParentType = decltype(*static_cast<IArgPack*>(reinterpret_cast<ArgPack*>(NULL)));

		if constexpr (argPack.argCount == 2)
			UnpackAndCall(func, argPack.nextArgPack, argPack.arg);
		else if constexpr (argPack.argCount == 1)
			func(argPack.arg);
		else
			func();
	}

	template<typename Ret, typename Func, typename ArgPack>
	static constexpr Ret UnpackAndCall(Func func, ArgPack argPack)
	{
		//ǿ������ArgPackΪIArgPack������
		using ParentType = decltype(*static_cast<IArgPack*>(reinterpret_cast<ArgPack*>(NULL)));

		if constexpr (argPack.argCount == 2)
			return UnpackAndCall<Ret>(func, argPack.nextArgPack, argPack.arg);
		else if constexpr (argPack.argCount == 1)
			return func(argPack.arg);
		else
			return func();
	}
};

#endif
