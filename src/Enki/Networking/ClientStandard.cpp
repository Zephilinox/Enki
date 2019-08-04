#include "ClientStandard.hpp"

//STD
#include <iostream>

//SELF
#include "Enki/Entity.hpp"

namespace enki
{
	ClientStandard::ClientStandard(std::uint8_t channel_count, const std::string& server_ip, std::uint16_t server_port)
	{
		auto console = spdlog::get("Enki");
		console->info("Client Initialized");
		client.connect(enetpp::client_connect_params()
			.set_channel_count(channel_count)
			.set_server_host_name_and_port(server_ip.c_str(), server_port));
	}

	ClientStandard::~ClientStandard()
	{
		auto console = spdlog::get("Enki");
		console->info("Client Deinitialized");
		client.disconnect();
	}

	void ClientStandard::processPackets()
	{
		auto console = spdlog::get("Enki");

		auto on_connected = [this]()
		{
			auto console = spdlog::get("Enki");
			connected_to_server = true;
			console->info("Connected");
			Packet p({ PacketType::CONNECTED, enet_time_get() });
			p.info.senderID = 1;
			p.info.timeReceived = enet_time_get();
			pushPacket(std::move(p));
		};

		auto on_disconnected = [this]()
		{
			auto console = spdlog::get("Enki");
			connected_to_server = false;
			console->info("Disconnected");
			Packet p({ PacketType::DISCONNECTED, enet_time_get() });
			p.info.senderID = 1;
			p.info.timeReceived = enet_time_get();
			pushPacket(std::move(p));
		};

		auto on_data_received = [this](const enet_uint8* data, size_t data_size)
		{
			auto console = spdlog::get("Enki");
			Packet p(data, data_size);

			if (p.getHeader().type == PacketType::CLIENT_INITIALIZED)
			{
				p >> id;
				console->info("Our ID is {} ", id);
			}

			p.resetReadPosition();
			p.info.senderID = 1;
			p.info.timeReceived = enet_time_get();

			//Sometimes on LAN/localhost a client's time will be a few milliseconds off
			//So if it's before the packet sent time, we make them the same so there's no timetravel shenanigans
			if (p.getHeader().timeSent > p.info.timeReceived)
			{
				p.info.timeReceived = p.getHeader().timeSent;
			}

			pushPacket(std::move(p));
		};

		client.consume_events(std::move(on_connected),
			std::move(on_disconnected),
			std::move(on_data_received));
	}

	void ClientStandard::sendPacket(enet_uint8 channel_id, Packet* p, enet_uint32 flags)
	{
		auto header = p->getHeader();
		header.timeSent = enet_time_get();
		p->setHeader(header);

		auto console = spdlog::get("Enki");
		//console->info("Client sending packet");
		auto data = reinterpret_cast<const enet_uint8*>(p->getBytes().data());
		client.send_packet(channel_id, data, p->getBytesWritten(), flags);
	}
}