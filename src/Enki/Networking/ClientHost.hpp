#pragma once

//SELF
#include "Enki/Networking/Client.hpp"
#include "Enki/Networking/ServerHost.hpp"

//LIBS
#include <enetpp/client.h>

namespace enki
{
class ClientHost : public Client
{
public:
	ClientHost();
	~ClientHost() final;

	//Called by the network manager in a different thread
	void processPackets() final{};

	//Send a packet to the server directly
	void sendPacket(enet_uint8 channel_id, Packet* p, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) final;

	[[nodiscard]] inline bool isConnected() const final
	{
		return true;
	}

	[[nodiscard]] inline bool isConnecting() const final
	{
		return true;
	}

	//This must be assigned after creating a ClientHost
	//Used to send packets to the local server directly
	//Cannot be passed through in constructor due to ServerHost requiring access to the client
	Server* server = nullptr;
};
}	// namespace enki