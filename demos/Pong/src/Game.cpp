#include "Game.hpp"

//STD
#include <iostream>

//LIBS
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <Enki/Networking/ServerHost.hpp>
#include <SFML/Graphics.hpp>
#include <Enki/GUI/IMGUI/imgui_SFML.h>
#include <Enki/GUI/Console.hpp>

//SELF
#include "Ball.hpp"
#include "Collision.hpp"
#include "Paddle.hpp"
#include "PlayerText.hpp"
#include "Score.hpp"

Game::Game()
{/*
	enki::EntityInfo info;
	info.childIDs = {10};
	enki::Packet p;
	p << info;
	auto info2 = p.read<enki::EntityInfo>();*/
	spdlog::info("{}", sizeof(enki::EntityInfo));
	spdlog::stdout_color_mt("console");
	auto console = spdlog::get("console");
	game_data = std::make_unique<enki::GameData>();
	window = std::make_unique<sf::RenderWindow>(sf::VideoMode(640, 360), "Enki");
	ImGui::SFML::Init(*window);
	renderer = std::make_unique<enki::Renderer>(window.get());

	network_manager = std::make_unique<enki::NetworkManager>();
	game_data->network_manager = network_manager.get();

	scenetree = std::make_unique<enki::Scenetree>(game_data.get());
	game_data->scenetree = scenetree.get();
	scenetree->registerEntity<enki::Console>(hash("Console"), {});
	scenetree->createEntityLocal(hash("Console"), "Console");

	scenetree->registerEntity<PlayerText>(hash("PlayerText3"), {});

	scenetree->registerEntity<PlayerText>(hash("PlayerText2"), {
		{hash("PlayerText3"), "my child :O"}
	});

	scenetree->registerEntity<PlayerText>(hash("PlayerText"), {
		{hash("PlayerText2"), "ayyy"}
	});

	scenetree->registerEntity<Paddle>(hash("Paddle"), {
		{hash("PlayerText"), "yeet"},
		{hash("PlayerText"), "yeet"}
	});

	//if the master calls it, every remote calls it
	//if a remote tries to call it, nothing will happen
	game_data->scenetree->rpc_man.registerEntityRPC(enki::RPCType::REMOTE_AND_LOCAL, hash("Paddle"), "setColour", &Paddle::setColour);

	scenetree->registerEntity<Ball>(hash("Ball"), {});

	scenetree->registerEntity<Collision>(hash("Collision"), {});
	scenetree->createEntityLocal(hash("Collision"), "Collision");

	scenetree->registerEntity<Score>(hash("Score"), {});
	game_data->scenetree->rpc_man.registerEntityRPC(enki::RPCType::REMOTE_AND_LOCAL, hash("Score"), "increaseScore1", &Score::increaseScore1);
	game_data->scenetree->rpc_man.registerEntityRPC(enki::RPCType::REMOTE_AND_LOCAL, hash("Score"), "increaseScore2", &Score::increaseScore2);

	run();
}

void Game::run()
{
	window->setFramerateLimit(120);
	while (window->isOpen())
	{
		game_data->network_manager->update();

		input();
		ImGui::SFML::Update(*window, dt);
		update();
		draw();

		dt = timer.getElapsedTime();
		timer.restart();
	}
}

void Game::input()
{
	sf::Event e;
	while (window->pollEvent(e))
	{
		ImGui::SFML::ProcessEvent(e);

		if (e.type == sf::Event::Closed)
		{
			window->close();
		}

		if (e.type == sf::Event::GainedFocus)
		{
			//game_data->window_active = true;
		}

		if (e.type == sf::Event::LostFocus)
		{
			//game_data->window_active = false;
		}

		if (e.type == sf::Event::KeyPressed)
		{
			if (e.key.code == sf::Keyboard::F1)
			{
				if (game_data->network_manager->network_send_rate == 10)
				{
					game_data->network_manager->network_send_rate = 60;
				}
				else
				{
					game_data->network_manager->network_send_rate = 10;
				}
			}
		}

		scenetree->input(e);
	}
}

void Game::update()
{
	if (/*game_data->window_active &&*/
		!game_data->network_manager->server &&
		!game_data->network_manager->client)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		{
			game_data->network_manager->startHost();
			scenetree->enableNetworking();
			//todo: switch to doing it as a demand, not request
			scenetree->createEntityNetworkedRequest(hash("Paddle"), "Paddle 1");
			scenetree->createEntityNetworkedRequest(hash("Ball"), "Ball");
			scenetree->createEntityNetworkedRequest(hash("Score"), "Score");

			mc2 = game_data->network_manager->client->on_packet_received.connect([](enki::Packet p) {
				auto console = spdlog::get("console");
				if (p.info.timeReceived - p.getHeader().timeSent > 2000)
				{
					console->error("local client received {}. sent at {} and received at {}, delta of {}", p.getHeader().type, p.getHeader().timeSent, p.info.timeReceived, p.info.timeReceived - p.getHeader().timeSent);
				}
			});

			mc3 = game_data->network_manager->server->on_packet_received.connect([scenetree = scenetree.get()](enki::Packet p) {
				auto console = spdlog::get("console");
				if (p.info.timeReceived - p.getHeader().timeSent > 2000)
				{
					console->error("server received {} from {}. sent at {} and received at {}, delta of {}", p.getHeader().type, p.info.senderID, p.getHeader().timeSent, p.info.timeReceived, p.info.timeReceived - p.getHeader().timeSent);
				}

				if (p.getHeader().type == enki::PacketType::CONNECTED)
				{
					if (scenetree->findEntityByName("Paddle 2") == nullptr)
					{
						//todo: need a public way of creating a networked entity that can be given a specific owner ID
						//scenetree->createEntityNetworkedRequest(hash("Paddle"), "Paddle 2", 0, p.info.senderID);
					}
				}
			});
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C))
		{
			game_data->network_manager->startClient();
			scenetree->enableNetworking();
			mc2 = game_data->network_manager->client->on_packet_received.connect([](enki::Packet p) {
				auto console = spdlog::get("console");
				if (p.info.timeReceived - p.getHeader().timeSent > 2000)
				{
					console->error("client received {}. sent at {} and received at {}, delta of {}", p.getHeader().type, p.getHeader().timeSent, p.info.timeReceived, p.info.timeReceived - p.getHeader().timeSent);
				}
			});
		}
	}

	scenetree->update(dt);

	ImGui::Begin("Framerate", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize({200, 30}, ImGuiCond_FirstUseEver);
	ImGui::SetWindowPos({2, 2}, ImGuiCond_FirstUseEver);
	ImGui::Text(fmt::format("{:.3f}ms/frame ({:.1f} FPS)", dt * 1.00f, 1.0f / dt).c_str());
	ImGui::End();
}

void Game::draw() const
{
	window->clear({230, 230, 230, 255});
	scenetree->draw(renderer.get());
	renderer->end();
	ImGui::SFML::Render(*window);
	window->display();
}