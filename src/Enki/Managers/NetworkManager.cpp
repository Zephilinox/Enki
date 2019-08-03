#include "NetworkManager.hpp"

//STD
#include <chrono>

//LIBS

//SELF
#include "Enki/Networking/ServerHost.hpp"
#include "Enki/Networking/ServerStandard.hpp"
#include "Enki/Networking/ClientHost.hpp"
#include "Enki/Networking/ClientStandard.hpp"

namespace enki
{
	using namespace std::chrono_literals;

	NetworkManager::NetworkManager()
	{
		console = spdlog::get("Enki");
		if (console == nullptr)
		{
			spdlog::stdout_color_mt("Enki");
			console = spdlog::get("Enki");
		}

		console->info("Initializing enet global state");
		enetpp::global_state::get().initialize();

		network_thread = std::thread(&NetworkManager::runThreadedNetwork, this);
	}

	NetworkManager::~NetworkManager()
	{
		console->info("Deinitializing global state");
		exit_thread = true;
		network_thread.join();

		stopServer();
		stopClient();

		enetpp::global_state::get().deinitialize();
	}

	void NetworkManager::startHost()
	{
		assert(!server);
		assert(!client);

		console->info("Starting Listen Server Hosting");
		server = std::make_unique<ServerHost>(max_clients, channel_count, server_port);
		client = std::make_unique<ClientHost>();
		static_cast<ServerHost*>(server.get())->client = client.get();
		static_cast<ClientHost*>(client.get())->server = server.get();
	}

	void NetworkManager::startServer()
	{
		assert(!server);
		console->info("Starting Server");
		server = std::make_unique<ServerStandard>(max_clients, channel_count, server_port);
	}

	void NetworkManager::startClient()
	{
		assert(!server);
		assert(!client);

		console->info("Starting Client");
		client = std::make_unique<ClientStandard>(channel_count, server_ip, server_port);
	}

	void NetworkManager::stopServer()
	{
		console->info("Stopping Server");
		if (server)
		{
			console->info("Server exists");
			server.reset(nullptr);
			console->info("Server destroyed");
		}
	}

	void NetworkManager::stopClient()
	{
		console->info("Stopping Client");
		if (client)
		{
			console->info("Client exists");
			client.reset(nullptr);
			console->info("Client destroyed");
		}
	}

	void NetworkManager::stopHost()
	{
		stopClient();
		stopServer();
	}

	void NetworkManager::update()
	{
		if (server)
		{
			server->update();
		}

		if (client)
		{
			client->update();
		}

		if (network_process_timer.getElapsedTime() > 1.0f / float(network_send_rate))
		{
			//Only emit the signal when there's someone else connected
			if ((server && (!server->getConnectedClients().empty() || client)) ||
				!server && client && client->isConnected())
			{
				network_process_timer.restart();
				on_network_tick.emit();
			}
		}
	}

	void NetworkManager::runThreadedNetwork()
	{
		console->info("Network thread started");
		while (!exit_thread)
		{
			if (server)
			{
				server->processPackets();
			}

			if (client)
			{
				client->processPackets();
			}

			std::this_thread::sleep_for(1s / float(network_process_rate));
		}

		console->info("Network thread stopped");
	}
}