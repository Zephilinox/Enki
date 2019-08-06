#include "RPCManager.hpp"

namespace enki
{
	RPCManager::RPCManager()
	{
		console = spdlog::get("Enki");
		if (console == nullptr)
		{
			spdlog::stdout_color_mt("Enki");
			console = spdlog::get("Enki");
		}
	}

	void RPCManager::receive(Packet p)
	{
		try
		{
			//done this way because getBytesRead will be sizeof(PacketHeader), and anything less than that is invalid
			if (p.getBytes().size() <= p.getBytesRead())
			{
				console->error("Invalid RPC packet received due to being empty, ignoring\n");
				return;
			}

			auto name = p.read<std::string>();

			if (!global_rpcs.count(name))
			{
				console->error("Invalid RPC packet received due to invalid name, ignoring\n");
				return;
			}

			global_rpcs[name].function(p);
		}
		catch (std::exception&)
		{
			console->error("Invalid RPC packet received that threw an exception, ignoring\n");
		}
	}

	RPCType RPCManager::getRPCType(const std::string& name) const
	{
		return global_rpcs.at(name).rpctype;
	}

	RPCType RPCManager::getRPCType(HashedID type, const std::string& name) const
	{
		return entity_rpcs.at(type).at(name).rpctype;
	}

	std::tuple<bool, bool> RPCManager::RPCInfo(RPCType type, bool owner)
	{
		bool local = false;
		bool remote = false;

		switch (type)
		{
		case Master:
			if (owner)
			{
				local = true;
			}
			else
			{
				remote = true;
			}
			break;
		case Remote:
			if (owner)
			{
				remote = true;
			}
			break;
		case RemoteAndLocal:
			if (owner)
			{
				remote = true;
				local = true;
			}
			break;
		case MasterAndRemote:
			remote = true;
			break;
		case Local:
			local = true;
			break;
		case All:
			remote = true;
			local = true;
			break;
		default:
			break;
		}

		return std::make_tuple(local, remote);
	}
}