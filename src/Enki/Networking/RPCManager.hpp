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
		RPCManager(NetworkManager* network_manager);
		
		//Register a global RPC with a function
		template <typename F>
		void registerGlobalRPC(RPCType rpctype, std::string name, F* func);

		//Register a global RPC with any callable
		//used for lambdas, etc
		template <typename Return, typename... Parameters>
		void registerGlobalRPC(RPCType rpctype, std::string name, std::function<Return(Parameters...)>* func);

		//Register a class RPC with a member function
		template <typename R, typename Class, typename... Args>
		void registerClassRPC(RPCType rpctype, std::string name, R(Class::*func)(Args...));

		//Register a derived from Entity RPC with a member function
		template <typename R, typename Class, typename... Args>
		void registerEntityRPC(RPCType rpctype, HashedID type, std::string name, R(Class::*func)(Args...));

		//Handle a global RPC packet
		void receive(Packet p);

		//Handle a member function RPC packet
		template <typename T>
		void receive(Packet p, T* instance);

		template <typename R, typename... Parameters, typename... Args>
		void callGlobalRPC([[maybe_unused]] R(*f)(Parameters...), std::string name, Args... args);

		template <typename R, typename... Parameters, typename... Args>
		void callGlobalRPC([[maybe_unused]] std::function<R(Parameters...)>* f, std::string name, Args... args);

		template <typename R, typename Class, typename... Parameters, typename T, typename... Args>
		void callEntityRPC([[maybe_unused]] R(Class::* f)(Parameters...), std::string name, T* instance, Args... args);

		template <typename R, typename Class, typename... Parameters, typename T, typename... Args>
		void callClassRPC([[maybe_unused]] R(Class::* f)(Parameters...), std::string name, T* instance, Args... args);

		template <typename... Args>
		void callGlobalRPCUnsafe(std::string name, Args... args);

		template <typename T, typename... Args>
		void callEntityRPCUnsafe(std::string name, T* instance, Args... args);

		template <typename T, typename... Args>
		void callClassRPCUnsafe(std::string name, T* instance, Args... args);

		//global rpc
		[[nodiscard]]
		RPCType getGlobalRPCType(const std::string& name) const;

		//entity rpc
		[[nodiscard]]
		RPCType getEntityRPCType(HashedID type, const std::string& name) const;

		//class rpc
		template <typename T>
		[[nodiscard]]
		RPCType getClassRPCType(const std::string& name) const;

	private:
		//Serialize variadic template args to packet in reverse (now correct) order, so as to fix right-to-left ordering
		//not defined in packet header because this stuff is specific to parameter pack expansion and will get misused
		void fillPacket() {};
		void fillPacket([[maybe_unused]] Packet& p) {};

		template <typename T>
		void fillPacket(Packet& p, T x);

		template <typename T, typename... Args>
		void fillPacket(Packet& p, T x, Args... args);

		NetworkManager* network_manager;
		std::tuple<bool, bool> RPCInfo(RPCType type, bool owner);

		std::map<std::string, GlobalRPC> global_rpcs;
		std::map<HashedID, RPCWrapper<Entity>> entity_rpcs;

		std::shared_ptr<spdlog::logger> console;
	};

	template <typename F>
	void RPCManager::registerGlobalRPC(RPCType rpctype, std::string name, F* func)
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

	template <typename Return, typename... Parameters>
	void RPCManager::registerGlobalRPC(RPCType rpctype, std::string name, std::function<Return(Parameters...)>* func)
	{
		/*using call_operator = decltype(&decltype(lambda)::operator());

		static_assert(std::is_void_v<typename RPCUtil<call_operator>::return_t>,
			"You can't register a lambda as an RPC if it doesn't return void");

		if (global_rpcs.count(name))
		{
			return;
		}

		global_rpcs[name].function = RPCUtil<call_operator>::wrap(lambda);
		global_rpcs[name].rpctype = rpctype;*/

		static_assert(std::is_void_v<Return>,
			"You can't register std::function as an RPC if it doesn't return void");

		if (global_rpcs.count(name))
		{
			return;
		}

		global_rpcs[name].function = [func](Packet p)
		{
			(*func)(p.read<Parameters>()...);
		};

		global_rpcs[name].rpctype = rpctype;
	}

	//Register a class RPC with a member function
	template <typename R, typename Class, typename... Args>
	void RPCManager::registerClassRPC(RPCType rpctype, std::string name, R(Class::*func)(Args...))
	{
		static_assert(std::is_void<R>::value,
			"You can't register a function as an RPC if it doesn't return void");

		static_assert(!std::is_base_of_v<Entity, Class>,
			"You can't register a ClassRPC for a class derived from Entity");

		if (RPCWrapper<Class>::class_rpcs.count(name))
		{
			return;
		}

		RPCWrapper<Class>::class_rpcs[name].function = RPCUtil<R(Class::*)(Args...)>::wrap(func);
		RPCWrapper<Class>::class_rpcs[name].rpctype = rpctype;
	}

	//Register a derived from Entity RPC with a member function
	template <typename R, typename Class, typename... Args>
	void RPCManager::registerEntityRPC(RPCType rpctype, HashedID type, std::string name, R(Class::*func)(Args...))
	{
		static_assert(std::is_void<R>::value,
			"You can't register a function as an RPC if it doesn't return void");

		static_assert (std::is_base_of_v<Entity, Class>,
			"You can't register a EntityRPC for a class not derived from Entity");

		if (entity_rpcs.count(type) && entity_rpcs[type].class_rpcs.count(name))
		{
			return;
		}

		entity_rpcs[type].class_rpcs[name].function = RPCUtil<R(Class::*)(Args...)>::template wrapAndCast<Entity*>(func);
		entity_rpcs[type].class_rpcs[name].rpctype = rpctype;
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

			if constexpr (std::is_base_of_v<Entity, T>)
			{
				auto info = p.read<EntityInfo>();
				auto name = p.read<std::string>();

				if (!entity_rpcs.count(info.type) || !entity_rpcs[info.type].class_rpcs.count(name))
				{
					console->error("Invalid RPC packet received due to invalid name {}, ignoring\n{}\n", name, info);
					return;
				}

				entity_rpcs[info.type].class_rpcs[name].function(p, instance);
			}
			else
			{
				auto name = p.read<std::string>();

				if (!RPCWrapper<T>::class_rpcs.count(name))
				{
					console->error("Invalid RPC packet received due to invalid name {}, ignoring\n", name);
					return;
				}

				RPCWrapper<T>::class_rpcs[name].function(p, instance);
			}
		}
		catch (std::exception& e)
		{
			console->error("Invalid RPC packet received that threw an exception ({}), ignoring\n", e.what());
		}
	}

	template <typename R, typename... Parameters, typename... Args>
	void RPCManager::callGlobalRPC([[maybe_unused]] R(*f)(Parameters...), std::string name, Args... args)
	{
		static_assert(std::is_same_v<std::tuple<Parameters...>, std::tuple<Args...>>,
			"You tried to call this rpc with the incorrect number or type of parameters");

		callGlobalRPCUnsafe(std::move(name), std::forward<Args>(args)...);
	}

	template <typename R, typename... Parameters, typename... Args>
	void RPCManager::callGlobalRPC([[maybe_unused]] std::function<R(Parameters...)>* f, std::string name, Args... args)
	{
		static_assert(std::is_same_v<std::tuple<Parameters...>, std::tuple<Args...>>,
			"You tried to call this rpc with the incorrect number or type of parameters");

		callGlobalRPCUnsafe(std::move(name), std::forward<Args>(args)...);
	}

	template <typename R, typename Class, typename... Parameters, typename T, typename... Args>
	void RPCManager::callEntityRPC([[maybe_unused]] R(Class::*f)(Parameters...), std::string name, T* instance, Args... args)
	{
		static_assert(std::is_same_v<std::tuple<Parameters...>, std::tuple<Args...>>,
			"You tried to call this rpc with the incorrect number or type of parameters");
		
		callEntityRPCUnsafe(std::move(name), instance, std::forward<Args>(args)...);
	}

	template <typename R, typename Class, typename... Parameters, typename T, typename... Args>
	void RPCManager::callClassRPC([[maybe_unused]] R(Class::* f)(Parameters...), std::string name, T* instance, Args... args)
	{
		static_assert(std::is_same_v<std::tuple<Parameters...>, std::tuple<Args...>>,
			"You tried to call this rpc with the incorrect number or type of parameters");

		callClassRPCUnsafe(std::move(name), instance, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void RPCManager::callGlobalRPCUnsafe(std::string name, Args... args)
	{
		if (!global_rpcs.count(name))
		{
			return;
		}

		Packet p({PacketType::GLOBAL_RPC});
		p << name;
		fillPacket(p, args...);

		auto [local, remote] = RPCInfo(global_rpcs[name].rpctype, true);

		if (remote &&
			network_manager &&
			network_manager->client &&
			network_manager->client->isConnected())
		{
			network_manager->client->sendPacket(0, &p);
		}

		if (local)
		{
			receive(p);
		}
	}

	template <typename T, typename... Args>
	void RPCManager::callEntityRPCUnsafe(std::string name, T* instance, Args... args)
	{
		static_assert(std::is_base_of_v<Entity, T>,
			"You tried to call a EntityRPC for a class not derived from Entity");

		if (!entity_rpcs.count(instance->info.type) ||
			!entity_rpcs[instance->info.type].class_rpcs.count(name))
		{
			return;
		}

		Packet p({ PacketType::ENTITY_RPC });
		p << instance->info << name;
		fillPacket(p, args...);

		auto [local, remote] = RPCInfo(entity_rpcs[instance->info.type].class_rpcs[name].rpctype, instance->isOwner());

		if (remote &&
			network_manager &&
			network_manager->client &&
			network_manager->client->isConnected())
		{
			network_manager->client->sendPacket(0, &p);
		}

		if (local)
		{
			receive(p, instance);
		}
	}

	template <typename T, typename... Args>
	void RPCManager::callClassRPCUnsafe(std::string name, T* instance, Args... args)
	{
		static_assert(!std::is_base_of_v<Entity, T>,
			"You tried to call a ClassRPC for a class derived from Entity");

		if (!RPCWrapper<T>::class_rpcs.count(name))
		{
			return;
		}

		Packet p({PacketType::CLASS_RPC});
		p << name;
		fillPacket(p, args...);

		auto [local, remote] = RPCInfo(RPCWrapper<T>::class_rpcs[name].rpctype, true);

		if (remote &&
			network_manager &&
			network_manager->client &&
			network_manager->client->isConnected())
		{
			network_manager->client->sendPacket(0, &p);
		}

		if (local)
		{
			receive(p, instance);
		}
	}

	template <typename T>
	RPCType RPCManager::getClassRPCType(const std::string& name) const
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