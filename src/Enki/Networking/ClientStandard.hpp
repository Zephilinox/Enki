#pragma once

//LIB
#include <enetpp/client.h>

//SELF
#include "Enki/Networking/Client.hpp"

namespace enki
{
class ClientStandard : public Client
{
public:
	ClientStandard(std::uint8_t channel_count, const std::string& server_ip, std::uint16_t server_port);
	~ClientStandard() final;

	//Called by the network manager in a different thread
	void processPackets() final;

	//Send a packet to the server
	void sendPacket(enet_uint8 channel_id, Packet* p, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) final;

	inline bool isConnected() const final
	{
		return isConnecting() && connected_to_server;
	}

	inline bool isConnecting() const final
	{
		return client.is_connecting_or_connected();
	}

private:
	enetpp::client client;
	bool connected_to_server = false;
};
}	// namespace enki