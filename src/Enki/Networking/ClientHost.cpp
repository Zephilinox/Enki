#include "ClientHost.hpp"

//SELF
#include "Enki/Entity.hpp"

namespace enki
{
ClientHost::ClientHost()
{
	id = 1;
	auto console = spdlog::get("Enki");
	console->info("Client Initialized");
}

ClientHost::~ClientHost()
{
	auto console = spdlog::get("Enki");
	console->info("Client Deinitialized");
}

void ClientHost::sendPacket([[maybe_unused]] enet_uint8 channel_id, Packet* p, [[maybe_unused]] enet_uint32 flags)
{
	auto header = p->getHeader();
	header.timeSent = enet_time_get();
	p->setHeader(header);

	auto console = spdlog::get("Enki");

	p->info.senderID = id;
	p->info.timeReceived = enet_time_get();
	server->on_packet_received.emit(*p);
}
}	// namespace enki