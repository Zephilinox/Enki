#pragma once

//STD
#include <tuple>

//LIBS
#include <spdlog/spdlog.h>

//SELF
#include "Enki/Networking/RPC.hpp"
	
namespace enki
{
	class RPCManager
	{
	public:
		RPCManager();

		//Register a global RPC with a callable
		template <typename F>
		void add(RPCType rpctype, std::string name, F* func);

		//Register a class RPC with a member function
		template <typename R, typename Class, typename... Args>
		void add(RPCType rpctype, std::string name, R(Class::*func)(Args...));

		//Register a derived from Entity RPC with a member function
		template <typename R, typename Class, typename... Args>
		void add(RPCType rpctype, std::string type, std::string name, R(Class::*func)(Args...));

		//Handle a global RPC packet
		void receive(Packet p);

		//Handle a member function RPC packet
		template <typename T>
		void receive(Packet p, T* instance);

		//call local global RPC
		template <typename F, typename... Args>
		void call([[maybe_unused]] F* f, std::string name, Args... args);

		//call local class RPC
		template <typename R, typename Class, typename T, typename... Args>
		void call([[maybe_unused]] R(Class::*f)(Args...), std::string name, T* instance, Args... args);

		//call local and remote RPC
		template <typename R, typename Class, typename T, typename... Args>
		void call([[maybe_unused]] R(Class::*f)(Args...), std::string name, NetworkManager* net_man, T* instance, Args... args);

		//call local global RPC unsafe
		template <typename... Args>
		void callUnsafe(std::string name, Args... args);

		//call local entity RPC unsafe
		template <typename T, typename... Args>
		void callUnsafe(std::string name, T* instance, Args... args);

		//global rpc
		[[nodiscard]]
		RPCType getRPCType(const std::string& name) const;

		//entity rpc
		[[nodiscard]]
		RPCType getRPCType(const std::string& type, const std::string& name) const;

		//class rpc
		template <typename T>
		[[nodiscard]]
		RPCType getRPCType(const std::string& name) const;

	private:
		//Serialize variadic template args to packet in reverse (now correct) order, so as to fix right-to-left ordering
		//not defined in packet header because this stuff is specific to parameter pack expansion and will get misused
		void fillPacket() {};
		void fillPacket([[maybe_unused]] Packet& p) {};

		template <typename T>
		void fillPacket(Packet& p, T x);

		template <typename T, typename... Args>
		void fillPacket(Packet& p, T x, Args... args);

		std::tuple<bool, bool> RPCInfo(RPCType type, bool owner);

		std::map<std::string, GlobalRPC> global_rpcs;
		std::map<std::string, std::map<std::string, EntityRPC>> entity_rpcs;

		std::shared_ptr<spdlog::logger> console;
	};

	template <typename F>
	void RPCManager::add(RPCType rpctype, std::string name, F* func)
	{
		static_assert(std::is_void<typename RPCUtil<F>::return_t>::value,
			"You can't register a function as an RPC if it doesn't return void");

		if (global_rpcs.count(name))
		{
			return;
		}

		global_rpcs[name].function = RPCUtil<F>::wrap(func);
		global_rpcs[name].rpctype = rpctype;
	}

	//Register a class RPC with a member function
	template <typename R, typename Class, typename... Args>
	void RPCManager::add(RPCType rpctype, std::string name, R(Class::*func)(Args...))
	{
		static_assert(std::is_void<R>::value,
			"You can't register a function as an RPC if it doesn't return void");
		if constexpr (std::is_base_of_v<Entity, Class>)
		{
			static_assert(false,
				"You can't call add(rpctype, name, function pointer) for a derived class of Entity. Use add(rpctype, type, name, function pointer) instead");
		}
		else
		{
			if (RPCWrapper<Class>::class_rpcs.count(name))
			{
				return;
			}

			RPCWrapper<Class>::class_rpcs[name] = RPCUtil<R(Class::*)(Args...)>::wrap(func);
			RPCWrapper<Class>::rpctypes[name] = rpctype;
		}
	}

	//Register a derived from Entity RPC with a member function
	template <typename R, typename Class, typename... Args>
	void RPCManager::add(RPCType rpctype, std::string type, std::string name, R(Class::*func)(Args...))
	{
		static_assert(std::is_void<R>::value,
			"You can't register a function as an RPC if it doesn't return void");

		if (entity_rpcs.count(type) && entity_rpcs[type].count(name))
		{
			return;
		}

		entity_rpcs[type][name].function = RPCUtil<R(Class::*)(Args...)>::wrapEntity(func);
		entity_rpcs[type][name].rpctype = rpctype;
	}

	template <typename T>
	void RPCManager::receive(Packet p, T* instance)
	{
		try
		{
			//done this way because getBytesRead will be sizeof(PacketHeader), and anything less than that is invalid
			if (p.getBytes().size() <= p.getBytesRead())
			{
				console->error("Invalid RPC packet received due to being empty, ignoring\n");
				return;
			}

			auto info = p.read<EntityInfo>();
			auto name = p.read<std::string>();

			if constexpr (std::is_base_of_v<Entity, T>)
			{
				if (!entity_rpcs.count(info.type) || !entity_rpcs[info.type].count(name))
				{
					console->error("Invalid RPC packet received due to invalid name, ignoring\n");
					return;
				}

				entity_rpcs[info.type][name].function(p, instance);
			}
			else
			{
				if (!RPCWrapper<T>::class_rpcs.count(name))
				{
					console->error("Invalid RPC packet received due to invalid name, ignoring\n");
					return;
				}

				RPCWrapper<T>::class_rpcs[name](p, instance);
			}
		}
		catch (std::exception&)
		{
			console->error("Invalid RPC packet received that threw an exception, ignoring\n");
		}
	}

	template <typename F, typename... Args>
	void RPCManager::call([[maybe_unused]] F* f, std::string name, Args... args)
	{
		static_assert(RPCUtil<F>::template matchesArgs<Args...>(),
			"You tried to call this rpc with the incorrect number or type of parameters");

		if (!global_rpcs.count(name))
		{
			return;
		}

		Packet p;
		p << name;
		fillPacket(p, args...);

		receive(p);
	}

	template <typename R, typename Class, typename T, typename... Args>
	void RPCManager::call([[maybe_unused]] R(Class::*f)(Args...), std::string name, T* instance, Args... args)
	{
		static_assert(RPCUtil<R(Class::*)(Args...)>::template matchesArgs<Args...>(),
			"You tried to call this rpc with the incorrect number or type of parameters");

		if (!RPCWrapper<T>::class_rpcs.count(name))
		{
			return;
		}

		Packet p({ PacketType::ENTITY_RPC });
		p << EntityInfo{} << name;
		fillPacket(p, args...);

		receive(p, instance);
	}

	template <typename R, typename Class, typename T, typename... Args>
	void RPCManager::call([[maybe_unused]] R(Class::*f)(Args...), std::string name, NetworkManager* net_man, T* instance, Args... args)
	{
		static_assert(RPCUtil<R(Class::*)(Args...)>::template matchesArgs<Args...>(),
			"You tried to call this rpc with the incorrect number or type of parameters");

		if (instance == nullptr ||
			net_man == nullptr ||
			net_man->client == nullptr ||
			!net_man->client->isConnected())
		{
			return;
		}

		if constexpr (std::is_base_of_v<Entity, T>)
		{
			//We know it's derived from Entity, so it must have an info member variable, no need to cast it.
			if (!entity_rpcs.count(instance->info.type) ||
				!entity_rpcs[instance->info.type].count(name))
			{
				return;
			}

			auto [local, remote] = RPCInfo(entity_rpcs[instance->info.type][name].rpctype, instance->isOwner());

			Packet p({ PacketType::ENTITY_RPC });
			p << instance->info << name;
			fillPacket(p, args...);

			if (remote)
			{
				net_man->client->sendPacket(0, &p);
			}

			if (local)
			{
				receive(p, instance);
			}
		}
		else
		{
			if (!RPCWrapper<T>::class_rpcs.count(name))
			{
				return;
			}

			Packet p({ PacketType::CLASS_RPC });
			p << name;
			fillPacket(p, args...);

			net_man->client->sendPacket(0, &p);
			receive(p, instance);
		}
	}

	template <typename... Args>
	void RPCManager::callUnsafe(std::string name, Args... args)
	{
		if (!global_rpcs.count(name))
		{
			return;
		}

		Packet p;
		p << name;
		fillPacket(p, args...);
		receive(p);
	}

	template <typename T, typename... Args>
	void RPCManager::callUnsafe(std::string name, T* instance, Args... args)
	{
		static_assert(std::is_base_of_v<Entity, T>);

		if (!entity_rpcs.count(instance->info.type) ||
			!entity_rpcs[instance->info.type].count(name))
		{
			return;
		}

		Packet p({ PacketType::ENTITY_RPC });
		p << instance->info;
		p << name;
		fillPacket(p, args...);
		receive(p, instance);
	}

	template <typename T>
	RPCType RPCManager::getRPCType(const std::string& name) const
	{
		return RPCWrapper<T>::class_rpcs.at(name).rpctype;
	}

	template <typename T>
	void RPCManager::fillPacket(Packet& p, T x)
	{
		p << x;
	}

	template <typename T, typename... Args>
	void RPCManager::fillPacket(Packet& p, T x, Args... args)
	{
		fillPacket(p, args...);
		p << x;
	}
}