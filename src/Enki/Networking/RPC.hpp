#pragma once

//SELF
#include "Enki/Entity.hpp"
#include "Enki/Networking/ClientStandard.hpp"
#include "Enki/Networking/Packet.hpp"
#include "Enki/Networking/ServerHost.hpp"

namespace enki
{
enum RPCType
{
	//For non-entity RPC's, ownership is always true.

	//RPC only runs for master
	//If you own the entity and call this RPC, it will execute only for you
	//If you don't own the entity and call this RPC, only the owner will execute the RPC
	MASTER,

	//RPC runs for all non-owners
	//If you own the entity and call this RPC, all remote entities will execute the RPC, but not yourself.
	//If you don't own the entity and call this RPC, nothing will happen
	REMOTE,

	//if you own the entity it will also execute it for you
	//Note that this might be slower than just using Remote and then calling the function directly as normal
	//This is provided mainly for ease-of-use, as well as when you might not be able to call it directly for some reason
	REMOTE_AND_LOCAL,

	//RPC runs for masters and non-owners
	//Calling this RPC will execute it for everyone, except yourself
	MASTER_AND_REMOTE,

	//Calling this RPC will execute it only for yourself
	LOCAL,

	//Calling this RPC will execute it for everyone, including yourself
	//Note that this might be slower than just using MasterAndRemote and then calling the function directly as normal
	//This is provided mainly for ease-of-use, as well as when you might not be able to call it directly for some reason
	ALL,
};

struct GlobalRPC
{
	using FunctionType = std::function<void(Packet)>;
	RPCType rpctype;
	FunctionType function;
};

inline Packet& operator<<(Packet& p, RPCType r)
{
	p << static_cast<std::uint8_t>(r);
	return p;
}

inline Packet& operator>>(Packet& p, RPCType& r)
{
	std::uint8_t k;
	p >> k;
	r = static_cast<RPCType>(k);
	return p;
}

//Used for storing member function RPC's for classes not derived from Entity*
//Might also be able to use variadic variables instead
template <class Wrapee>
class RPCWrapper
{
public:
	struct ClassRPC
	{
		using FunctionType = std::function<void(Packet, Wrapee*)>;
		RPCType rpctype = RPCType::ALL; //todo: create a default None?
		FunctionType function;
	};

	inline static std::map<std::string, ClassRPC> class_rpcs;
};

//Used for getting type info from functions
//and having that info available in the wrapped RPC functions
template <typename NotImportant>
struct RPCUtil;

//For free-standing functions
template <typename Return, typename... Parameters>
struct RPCUtil<Return(Parameters ...)>
{
	using return_t = Return;

	template <typename F>
	static std::function<void(Packet)> wrap(F f)
	{
		//Wrapping the function for later when we have the parameters available
		//This is an alternative method to std::bind with a nicer interface
		return [f](Packet p)
		{
			//Using parameter pack expansion within the call site
			//Remember lambdas can use template types
			//that are available when defined
			(*f)(p.read<Parameters>()...);
		};
	}
};

//For member functions
template <typename Return, typename Class, typename... Parameters>
struct RPCUtil<Return((Class::*)(Parameters ...))>
{
	using return_t = Return;

	template <typename F>
	static std::function<void(Packet, Class*)> wrap(F f)
	{
		return [f](Packet p, Class* instance)
		{
			(instance->*f)(p.read<Parameters>()...);
		};
	}

	//Special case, used for classes derived from Entity's for Entity RPC's
	template <typename Base, typename F>
	static std::function<void(Packet, Base*)> wrapAndCast(F f)
	{
		static_assert(!std::is_same_v<Class, Base>);
		static_assert(std::is_base_of_v<Base, Class>);

		return [f](Packet p, Base* instance)
		{
			auto derived = static_cast<Class*>(instance);
			(derived->*f)(p.read<Parameters>()...);
		};
	}
};
} // namespace enki
