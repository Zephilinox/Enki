#pragma once

//LIB
#include <enetpp/client.h>
#include <spdlog/spdlog.h>

//SELF
#include "Enki/Networking/Packet.hpp"
#include "Enki/Signals/Signal.hpp"
#include "Enki/Timer.hpp"

namespace enki
{
class ClientInfo
{
public:
	ClientID id;

	//required by enetpp
	[[nodiscard]] ClientID get_id() const noexcept
	{
		return id;
	}
};

class Server
{
public:
	virtual ~Server() = default;

	virtual void processPackets() = 0;

	virtual void sendPacketToOneClient(ClientID client_id, enet_uint8 channel_id, Packet* p, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) = 0;
	virtual void sendPacketToAllClients(enet_uint8 channel_id, Packet* p, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) = 0;
	virtual void sendPacketToSomeClients(enet_uint8 channel_id, Packet* p, enet_uint32 flags, std::function<bool(const ClientInfo& client)> predicate) = 0;
	virtual void sendPacketToAllExceptOneClient(ClientID client_id_excluded, enet_uint8 channel_id, Packet* p, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) = 0;

	[[nodiscard]] virtual bool isListening() const = 0;

	[[nodiscard]] virtual const std::vector<ClientInfo*>& getConnectedClients() const = 0;

	//Called by the Network Manager
	void update();

	//Push a packet to the queue for later emission
	//thread-safe
	void pushPacket(Packet&& p);

	//Connect to this signal to be notified when a packet is received
	Signal<Packet> on_packet_received;

protected:
	Server() = default;

private:
	std::mutex mutex;
	std::queue<Packet> packets;

	Timer packetsTimer;
	uint32_t packetsReceived = 0;
};
}	// namespace enki