#pragma once

//LIB
#include <enetpp/server.h>

//SELF
#include "Enki/Networking/Client.hpp"
#include "Enki/Networking/Server.hpp"

namespace enki
{
	class ServerHost : public Server
	{
	public:
		ServerHost(std::uint32_t max_clients, std::uint8_t channel_count, std::uint16_t server_port);
		~ServerHost() final;

		void processPackets() final;

		void sendPacketToOneClient(ClientID client_id, enet_uint8 channel_id, Packet* p, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) final;
		void sendPacketToAllClients(enet_uint8 channel_id, Packet* p, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) final;
		void sendPacketToSomeClients(enet_uint8 channel_id, Packet* p, enet_uint32 flags, std::function<bool(const ClientInfo& client)> predicate) final;
		void sendPacketToAllExceptOneClient(ClientID client_id_excluded, enet_uint8 channel_id, Packet* p, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) final;

		[[nodiscard]]
		inline bool isListening() const final
		{
			return server.is_listening();
		}

		[[nodiscard]]
		inline const std::vector<ClientInfo*>& getConnectedClients() const final
		{
			return server.get_connected_clients();
		}

		//This must be assigned after creating a ServerHost
		//Used to send packets to the local client directly
		//Cannot be passed through in constructor due to ClientHost requiring access to the server
		Client* client;

	private:
		ClientID getNextUID();

		enetpp::server<ClientInfo> server;
		ClientID next_uid = 2;
		std::queue<ClientID> free_ids;

		ManagedConnection mc1;
	};
}