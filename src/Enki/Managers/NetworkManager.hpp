#pragma once

//STD
#include <functional>
#include <thread>

//LIB
#include <enetpp/client.h>
#include <enetpp/server.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

//SELF
#include "../Signals/Signal.hpp"
#include "../Timer.hpp"

namespace enki
{
	class Server;
	class Client;

	class NetworkManager
	{
	public:
		NetworkManager();
		~NetworkManager();

		//Start a server and accept new connections
		void startServer();

		//Start a client and connect to the server
		void startClient();

		//Start a combined server + client and accept new connections
		//Set max clients to 0 for single player
		void startHost();

		//Stop the server and disconnect all clients
		void stopServer();

		//Stop the client and disconnect from the server
		void stopClient();

		//Stop both the server and client
		void stopHost();

		//Must be called once per game loop to ensure packets are handled
		void update();

		std::unique_ptr<Server> server;
		std::unique_ptr<Client> client;

		//Connect to this to be notified of when a network tick occurs
		Signal<> on_network_tick;

		//How many network ticks there are in a second
		int network_send_rate = 60;

		//How many times new packets will be checked every second
		//Increase for reduced delays but with increased CPU usage
		int network_process_rate = 120;

		//Used by the server
		//note: doesn't include the client when hosting
		uint32_t max_clients = 7;
		
		//Used by the client and server
		//How many channels to use, useful for having a reliable and unreliable option
		uint8_t channel_count = 1;

		//Used by the client to connect to a server
		std::string server_ip = "localhost";
		//Used by the client and server
		uint16_t server_port = 22222;

	private:
		void runThreadedNetwork();

		std::thread network_thread;
		bool exit_thread = false;
		Timer network_process_timer;
		std::shared_ptr<spdlog::logger> console = nullptr;
	};
}