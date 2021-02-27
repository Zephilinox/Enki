#pragma once

//SELF
#include "Enki/Networking/Packet.hpp"
#include "Enki/Signals/Signal.hpp"
#include "Enki/Timer.hpp"

//LIBS
#include <enetpp/client.h>
#include <spdlog/spdlog.h>

//STD
#include <iostream>

namespace enki
{
class Client
{
public:
	virtual ~Client() = default;

	virtual void processPackets() = 0;

	virtual void sendPacket(enet_uint8 channel_id, Packet* p, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) = 0;

	[[nodiscard]] virtual bool isConnected() const = 0;

	[[nodiscard]] virtual bool isConnecting() const = 0;

	void update();
	//Push a packet on to the queue for later emission
	//thread safe
	void pushPacket(Packet&& p);

	[[nodiscard]] inline ClientID getID() const
	{
		return id;
	}

	//Connect to this signal to be notified when a packet is received
	Signal<Packet> on_packet_received;

protected:
	Client() = default;
	ClientID id = 0;

private:
	std::mutex mutex;
	std::queue<Packet> packets;

	Timer packetsTimer;
	uint32_t packetsReceived = 0;
};
}	// namespace enki