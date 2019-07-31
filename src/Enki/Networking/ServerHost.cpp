#include "ServerHost.hpp"

//LIBS
#include <spdlog/spdlog.h>

namespace enki
{
	ServerHost::ServerHost(std::uint32_t max_clients, std::uint8_t channel_count, std::uint16_t server_port)
		: client(client)
	{
		auto console = spdlog::get("Enki");
		console->info("Server Initialized");

		auto client_init = [this](ClientInfo& client, const char* ip)
		{
			auto console = spdlog::get("Enki");
			client.id = getNextUID();

			//Tell the client what its ID is
			Packet p({ PacketType::CLIENT_INITIALIZED });
			p << client.id;
			console->info("Client {} initialized. IP = {}", client.id, ip);
			sendPacketToOneClient(client.id, 0, &p);
		};

		mc1 = on_packet_received.connect([this](Packet p)
		{
			if (p.getHeader().type == PacketType::DISCONNECTED)
			{
				free_ids.push(p.info.senderID);
				auto console = spdlog::get("Enki");
				console->info("ServerHost:\tpushed new free id {}", p.info.senderID);
			}
		});

		server.start_listening(enetpp::server_listen_params<ClientInfo>()
			.set_max_client_count(max_clients)
			.set_channel_count(channel_count)
			.set_listen_port(server_port)
			.set_initialize_client_function(std::move(client_init)));
	}

	ServerHost::~ServerHost()
	{
		auto console = spdlog::get("Enki");
		console->info("Server Deinitialized");
		console->info("Server received {} in {} packets", server.enet_host->totalReceivedData, server.enet_host->totalReceivedPackets);
		console->info("Server sent {} in {} packets", server.enet_host->totalSentData, server.enet_host->totalSentPackets);
		server.stop_listening();
	}

	void ServerHost::processPackets()
	{
		auto on_client_connected = [&](ClientInfo& client)
		{
			auto console = spdlog::get("Enki");
			console->info("Client {} Connected.", client.id);
			Packet p({ PacketType::CONNECTED, enet_time_get() });
			p.info.senderID = client.id;
			p.info.timeReceived = enet_time_get();
			pushPacket(std::move(p));
		};

		auto on_client_disconnected = [&](ClientID client_uid)
		{
			auto console = spdlog::get("Enki");
			console->info("Client {} Disconnected", client_uid);
			Packet p({ PacketType::DISCONNECTED, enet_time_get() });
			p.info.senderID = client_uid;
			p.info.timeReceived = enet_time_get();
			pushPacket(std::move(p));
		};

		auto on_client_data_received = [&](ClientInfo& client, const enet_uint8* data, size_t data_size)
		{
			auto console = spdlog::get("Enki");
			//console->info("Data received from Client {}. Size = {}", client.get_id(), data_size);
			Packet p(data, data_size);
			p.info.senderID = client.id;
			p.info.timeReceived = enet_time_get();

			//Sometimes on LAN/localhost a client's time will be a few milliseconds off
			//So if it's before the packet sent time, we make them the same so there's no timetravel
			if (p.getHeader().timeSent > p.info.timeReceived)
			{
				p.info.timeReceived = p.getHeader().timeSent;
			}

			pushPacket(std::move(p));
		};

		server.consume_events(std::move(on_client_connected),
			std::move(on_client_disconnected),
			std::move(on_client_data_received));
	}

	void ServerHost::sendPacketToOneClient(ClientID client_id, enet_uint8 channel_id, Packet* p, enet_uint32 flags)
	{
		auto header = p->getHeader();
		header.timeSent = enet_time_get();
		p->setHeader(header);

		if (client_id != 1)
		{
			auto data = reinterpret_cast<const enet_uint8*>(p->getBytes().data());
			server.send_packet_to(client_id, channel_id, data, p->getBytesWritten(), flags);
		}
		else
		{
			p->resetReadPosition();
			client->on_packet_received.emit(*p);
		}
	}

	void ServerHost::sendPacketToAllClients(enet_uint8 channel_id, Packet* p, enet_uint32 flags)
	{
		auto header = p->getHeader();
		header.timeSent = enet_time_get();
		p->setHeader(header);

		if (!server.get_connected_clients().empty())
		{
			auto data = reinterpret_cast<const enet_uint8*>(p->getBytes().data());
#pragma warning (push)
#pragma warning (disable: 4100)
			server.send_packet_to_all_if(channel_id, data, p->getBytesWritten(), flags, []([[maybe_unused]]const ClientInfo& client) {return true; });
#pragma warning (pop)
		}

		p->resetReadPosition();
		p->info.timeReceived = header.timeSent;
		p->info.senderID = 1;
		client->on_packet_received.emit(*p);
	}

	void ServerHost::sendPacketToSomeClients(enet_uint8 channel_id, Packet* p, enet_uint32 flags, std::function<bool(const ClientInfo& client)> predicate)
	{
		auto header = p->getHeader();
		header.timeSent = enet_time_get();
		p->setHeader(header);

		auto console = spdlog::get("Enki");
		if (!server.get_connected_clients().empty())
		{
			auto data = reinterpret_cast<const enet_uint8*>(p->getBytes().data());
			server.send_packet_to_all_if(channel_id, data, p->getBytesWritten(), flags, predicate);
		}

		if (predicate({ 1 }))
		{
			p->resetReadPosition();
			p->info.timeReceived = header.timeSent;
			p->info.senderID = 1;
			client->on_packet_received.emit(*p);
		}
	}

	void ServerHost::sendPacketToAllExceptOneClient(ClientID client_id_excluded, enet_uint8 channel_id, Packet* p, enet_uint32 flags)
	{
		auto header = p->getHeader();
		header.timeSent = enet_time_get();
		p->setHeader(header);

		if (!server.get_connected_clients().empty())
		{
			auto data = reinterpret_cast<const enet_uint8*>(p->getBytes().data());
			server.send_packet_to_all_if(channel_id, data, p->getBytesWritten(), flags, [client_id_excluded](const ClientInfo& client)
			{
				return client.id != client_id_excluded;
			});
		}

		if (client_id_excluded != 1)
		{
			p->resetReadPosition();
			p->info.timeReceived = header.timeSent;
			p->info.senderID = 1;
			client->on_packet_received.emit(*p);
		}
	}

	ClientID ServerHost::getNextUID()
	{
		auto console = spdlog::get("Enki");
		if (free_ids.empty())
		{
			console->info("ServerHost:\tno free ids, giving {}", next_uid);
			return next_uid++;
		}
		else
		{
			auto id = free_ids.front();
			console->info("ServerHost:\tooh free ids, giving {}", id);
			free_ids.pop();
			return id;
		}
	}
}