#include "NetworkManager.hpp"

//STD
#include <chrono>

//LIBS
#include <Tracy.hpp>

//SELF
#include "Enki/Networking/ClientHost.hpp"
#include "Enki/Networking/ClientStandard.hpp"
#include "Enki/Networking/ServerHost.hpp"
#include "Enki/Networking/ServerStandard.hpp"

namespace enki
{
using namespace std::chrono_literals;

NetworkManager::NetworkManager()
{
	console = spdlog::get("Enki");
	if (!console)
		console = spdlog::stdout_color_mt("Enki");

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
	assert(!isServer());
	assert(!isClient());

	console->info("Starting Listen Server Hosting");
	server = std::make_unique<ServerHost>(max_clients, channel_count, server_port);
	client = std::make_unique<ClientHost>();

	//the hackyness ermahgawd todo
	static_cast<ServerHost*>(server.get())->client = client.get();
	static_cast<ClientHost*>(client.get())->server = server.get();
}

void NetworkManager::startServer()
{
	assert(!isServer());
	console->info("Starting Server");
	server = std::make_unique<ServerStandard>(max_clients, channel_count, server_port);
}

void NetworkManager::startClient()
{
	assert(!isServer());
	assert(!isClient());

	console->info("Starting Client");
	client = std::make_unique<ClientStandard>(channel_count, server_ip, server_port);
}

void NetworkManager::stopServer()
{
	console->info("Stopping Server");
	if (isServer())
	{
		console->info("Server exists");
		server.reset(nullptr);
		console->info("Server destroyed");
	}
}

void NetworkManager::stopClient()
{
	console->info("Stopping Client");
	if (isClient())
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
	ZoneScopedN("update network manager")
	if (isServer())
		server->update();

	if (isClient())
		client->update();

	if (network_process_timer.getElapsedTime() > 1.0f / float(network_send_rate))
	{
		//Only emit the signal when there's someone connected
		const bool serverAndConnectedClients = isServer() && (!server->getConnectedClients().empty() || isClient());
		const bool clientAndConnected = isClientOnly() && client->isConnected();
		if (serverAndConnectedClients || clientAndConnected)
		{
			network_process_timer.restart();
			on_network_tick.emit();
		}
	}
}

bool NetworkManager::isHost() const
{
	return server != nullptr && client != nullptr;
}

bool NetworkManager::isServer() const
{
	return server != nullptr;
}

bool NetworkManager::isClient() const
{
	return client != nullptr;
}

bool NetworkManager::isClientOnly() const
{
	return client != nullptr && server == nullptr;
}

bool NetworkManager::isServerOnly() const
{
	return server != nullptr && client == nullptr;
}

bool NetworkManager::isNetworked() const
{
	return server != nullptr || client != nullptr;
}

void NetworkManager::runThreadedNetwork()
{
	console->info("Network thread started");
	while (!exit_thread)
	{
		if (isServer())
			server->processPackets();

		if (isClient())
				client->processPackets();

		std::this_thread::sleep_for(1s / float(network_process_rate));
	}

	console->info("Network thread stopped");
}
}	// namespace enki