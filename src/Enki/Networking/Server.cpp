#include "Server.hpp"

namespace enki
{
	void Server::update()
	{
		packetsReceived += packets.size();

		if (packetsTimer.getElapsedTime() > 10)
		{
			auto console = spdlog::get("Enki");
			packetsTimer.restart();
			console->info("server received {} packets in the last 10 seconds", packetsReceived);
			packetsReceived = 0;
		}

		std::lock_guard<std::mutex> guard(mutex);
		while (!packets.empty())
		{
			on_packet_received.emit(packets.front());
			packets.pop();
		}
	}

	void Server::pushPacket(Packet&& p)
	{
		std::lock_guard<std::mutex> lock(mutex);
		packets.push(std::move(p));
	}
}