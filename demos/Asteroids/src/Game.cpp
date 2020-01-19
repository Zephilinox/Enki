#include "Game.hpp"

//STD
#include <iostream>

//LIBS
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <Enki/Networking/ServerHost.hpp>

//SELF
#include "Player.hpp"
#include "Asteroid.hpp"
#include "Bullet.hpp"
#include "CollisionManager.hpp"
#include "PlayerText.hpp"
#include "CustomData.hpp"

Game::Game()
{
	spdlog::stdout_color_mt("console");
	auto console = spdlog::get("console");

	window = std::make_unique<sf::RenderWindow>(sf::VideoMode(1280, 720), "Enki");
	input_manager.window = window.get();

	game_data = std::make_unique<enki::GameData>();
	custom_data = std::make_unique<CustomData>();

	custom_data->input_manager = &input_manager;

	scenetree = std::make_unique<enki::Scenetree>(game_data.get());
	auto enki_logger = spdlog::get("Enki");
	enki_logger->set_level(spdlog::level::err);

	network_manager = std::make_unique<enki::NetworkManager>();

	game_data->scenetree = scenetree.get();
	game_data->network_manager = network_manager.get();

	scenetree->registerEntity<Player>("Player", custom_data.get(), window.get());
	scenetree->registerEntity<Asteroid>("Asteroid", custom_data.get(), window.get());
	scenetree->registerEntity<Bullet>("Bullet", custom_data.get(), window.get());
	scenetree->registerEntity<CollisionManager>("CollisionManager", custom_data.get(), window.get());
	scenetree->registerEntity<PlayerText>("PlayerText");

	scenetree->rpc_man.add(enki::RPCType::REMOTE_AND_LOCAL, "Player", "startInvincible", &Player::startInvincible);
	scenetree->rpc_man.add(enki::RPCType::REMOTE_AND_LOCAL, "Player", "stopInvincible", &Player::stopInvincible);

	scenetree->registerEntityChildren("Player", enki::ChildEntityCreationInfo{"PlayerText", "PlayerText"});

	run();
}

void Game::run()
{
	//window->setFramerateLimit(120);

	enki::Timer fpsTimer;
	auto console = spdlog::get("console");
	while (window->isOpen())
	{
		input();
		update();
		draw();

		dt = timer.getElapsedTime();
		if (fpsTimer.getElapsedTime() > 0.5f)
		{
			console->info("FPS: {}", 1.0f / dt);
			fpsTimer.restart();
		}
		timer.restart();
	}
}

void Game::input()
{
	sf::Event e;
	while (window->pollEvent(e))
	{
		if (e.type == sf::Event::Closed)
		{
			window->close();
		}
		else if (e.type == sf::Event::GainedFocus)
		{
			custom_data->window_active = true;
		}
		else if (e.type == sf::Event::LostFocus)
		{
			custom_data->window_active = false;
		}

		scenetree->input(e);
	}
}

void Game::update()
{
	network_manager->update();
	input_manager.update();

	static bool networking = false;

	if (network_manager->server &&
		(input_manager.isKeyPressed(sf::Keyboard::Key::F2) ||
			asteroid_spawn_timer.getElapsedTime() > 1.0f))
	{
		enki::Packet p;
		p << (std::rand() % 8) + 5
			<< static_cast<float>(std::rand() % 1280)
			<< static_cast<float>(std::rand() % 720)
			<< static_cast<float>((std::rand() % 200) + 50);
		scenetree->createNetworkedEntity({ "Asteroid", "Asteroid" }, p);
		asteroid_spawn_timer.restart();
	}

	if (!networking && custom_data->window_active)
	{
		if (input_manager.isKeyPressed(sf::Keyboard::Key::S))
		{
			networking = true;
			network_manager->startHost();
			scenetree->enableNetworking();
			scenetree->createNetworkedEntity({ "Player", "Player 1" });
			scenetree->createEntity({ "CollisionManager", "CollisionManager" });

			for (int i = 0; i < 20; ++i)
			{
				enki::Packet p;
				p << (std::rand() % 8) + 5
					<< static_cast<float>(std::rand() % 1280)
					<< static_cast<float>(std::rand() % 720)
					<< static_cast<float>((std::rand() % 200) + 50);
				scenetree->createNetworkedEntity({ "Asteroid", "Asteroid" }, p);
			}

			mc1 = network_manager->server->on_packet_received.connect([this](enki::Packet p)
			{
				if (p.getHeader().type == enki::PacketType::CONNECTED)
				{
					scenetree->createNetworkedEntity({ "Player", "Player " + std::to_string(p.info.senderID), 0, p.info.senderID });
				}

				if (p.getHeader().type == enki::PacketType::DISCONNECTED)
				{
					auto ent = scenetree->findEntityByName("Player " + std::to_string(p.info.senderID));
					if (ent)
					{
						scenetree->deleteEntity(ent->info.ID);
					}
				}
			});
		}

		if (input_manager.isKeyPressed(sf::Keyboard::Key::C))
		{
			networking = true;
			network_manager->startClient();
			scenetree->enableNetworking();
			scenetree->createEntity({ "CollisionManager", "CollisionManager" });
		}
	}

	scenetree->update(dt);
}

void Game::draw() const
{
	window->clear({ 40, 40, 40, 255 });
	scenetree->draw(*window.get());
	window->display();
}