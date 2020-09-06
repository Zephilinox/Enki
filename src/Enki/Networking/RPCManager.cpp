#include "RPCManager.hpp"

namespace enki
{
RPCManager::RPCManager(NetworkManager* network_manager)
	: network_manager(network_manager)
{
	//if (!network_manager)
	//	throw;

	console = spdlog::get("Enki");
	if (!console)
		console = spdlog::stdout_color_mt("Enki");
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

		//todo: change to string_view in C++20
		const auto name = p.read<std::string>();
		const auto found_it = global_rpcs.find(name);
		if (found_it != global_rpcs.end())
			found_it->second.function(std::move(p));
		else
			console->error("Invalid RPC packet received with name '{}', ignoring\n", name);
	}
	catch (std::exception&)
	{
		console->error("Invalid RPC packet received that threw an exception, ignoring\n");
	}
}

RPCType RPCManager::getGlobalRPCType(const std::string& name) const
{
	return global_rpcs.at(name).rpctype;
}

RPCType RPCManager::getEntityRPCType(HashedID type, const std::string& name) const
{
	return RPCWrapper<Entity>::class_rpcs.at(name).rpctype;
}

std::tuple<bool, bool> RPCManager::RPCInfo(RPCType type, bool owner)
{
	bool local = false;
	bool remote = false;

	switch (type)
	{
		case MASTER:
		{
			if (owner)
				local = true;
			else
				remote = true;
			break;
		}
		case REMOTE:
		{
			if (owner)
				remote = true;
			break;
		}
		case REMOTE_AND_LOCAL:
		{
			if (owner)
			{
				remote = true;
				local = true;
			}
			break;
		}
		case MASTER_AND_REMOTE:
		{
			remote = true;
			break;
		}
		case LOCAL:
		{
			local = true;
			break;
		}
		case ALL:
		{
			remote = true;
			local = true;
			break;
		}
		default:
		{
			throw;
			break;
		}
	}

	return {local, remote};
}
} // namespace enki
