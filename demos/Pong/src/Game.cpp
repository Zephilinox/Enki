#include "Game.hpp"

//STD
#include <iostream>

//LIBS
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <Enki/Networking/ServerHost.hpp>

//SELF
#include "Paddle.hpp"
#include "Ball.hpp"
#include "Collision.hpp"
#include "PlayerText.hpp"
#include "Score.hpp"

Game::Game()
{
	spdlog::stdout_color_mt("console");
	auto console = spdlog::get("console");
	game_data = std::make_unique<enki::GameData>();
	window = std::make_unique<sf::RenderWindow>(sf::VideoMode(640, 360), "Enki");
	renderer = std::make_unique<enki::Renderer>(window.get());

	network_manager = std::make_unique<enki::NetworkManager>();
	game_data->network_manager = network_manager.get();

	scenegraph = std::make_unique<enki::Scenegraph>(game_data.get());
	game_data->scenegraph = scenegraph.get();

	scenegraph->registerEntity<PlayerText>(hash("PlayerText3"));

	scenegraph->registerEntity<PlayerText>(hash("PlayerText2"));
	scenegraph->registerEntityChildren(hash("PlayerText2"),
		enki::ChildEntityCreationInfo{hash("PlayerText3"), "my child :O" });

	scenegraph->registerEntity<PlayerText>(hash("PlayerText"));
	scenegraph->registerEntityChildren(hash("PlayerText"),
		enki::ChildEntityCreationInfo{hash("PlayerText2"), "ayyy" });

	scenegraph->registerEntity<Paddle>(hash("Paddle"));
	scenegraph->registerEntityChildren(hash("Paddle"),
		enki::ChildEntityCreationInfo{hash("PlayerText"), "yeet" },
		enki::ChildEntityCreationInfo{hash("PlayerText"), "yeet" });

	//if the master calls it, every remote calls it
	//if a remote tries to call it, nothing will happen
	game_data->scenegraph->rpc_man.registerEntityRPC(enki::RPCType::REMOTE_AND_LOCAL, hash("Paddle"), "setColour", &Paddle::setColour);

	scenegraph->registerEntity<Ball>(hash("Ball"));

	scenegraph->registerEntity<Collision>(hash("Collision"));
	scenegraph->createEntity({hash("Collision"), "Collision"});

	scenegraph->registerEntity<Score>(hash("Score"));
	game_data->scenegraph->rpc_man.registerEntityRPC(enki::RPCType::REMOTE_AND_LOCAL, hash("Score"), "increaseScore1", &Score::increaseScore1);
	game_data->scenegraph->rpc_man.registerEntityRPC(enki::RPCType::REMOTE_AND_LOCAL, hash("Score"), "increaseScore2", &Score::increaseScore2);

	run();
}

void Game::run()
{
	window->setFramerateLimit(120);
	while (window->isOpen())
	{
		game_data->network_manager->update();

		input();
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

		scenegraph->input(e);
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
			scenegraph->enableNetworking();
			scenegraph->createNetworkedEntity(enki::EntityInfo{ hash("Paddle"), "Paddle 1" });
			scenegraph->createNetworkedEntity(enki::EntityInfo{hash("Ball"), "Ball" });
			scenegraph->createNetworkedEntity({hash("Score"), "Score" });

			mc2 = game_data->network_manager->client->on_packet_received.connect([](enki::Packet p)
			{
				auto console = spdlog::get("console");
				if (p.info.timeReceived - p.getHeader().timeSent > 2000)
				{
					console->error("local client received {}. sent at {} and received at {}, delta of {}", p.getHeader().type, p.getHeader().timeSent, p.info.timeReceived, p.info.timeReceived - p.getHeader().timeSent);
				}
			});

			mc3 = game_data->network_manager->server->on_packet_received.connect([scenegraph = scenegraph.get()](enki::Packet p)
			{
				auto console = spdlog::get("console");
				if (p.info.timeReceived - p.getHeader().timeSent > 2000)
				{
					console->error("server received {} from {}. sent at {} and received at {}, delta of {}", p.getHeader().type, p.info.senderID, p.getHeader().timeSent, p.info.timeReceived, p.info.timeReceived - p.getHeader().timeSent);
				}
				
				if (p.getHeader().type == enki::PacketType::CONNECTED)
				{
					if (scenegraph->findEntityByName("Paddle 2") == nullptr)
					{
						scenegraph->createNetworkedEntity(enki::EntityInfo{hash("Paddle"), "Paddle 2", 0, p.info.senderID });
					}
				}
			});
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C))
		{
			game_data->network_manager->startClient();
			scenegraph->enableNetworking();
			mc2 = game_data->network_manager->client->on_packet_received.connect([](enki::Packet p)
			{
				auto console = spdlog::get("console");
				if (p.info.timeReceived - p.getHeader().timeSent > 2000)
				{
					console->error("client received {}. sent at {} and received at {}, delta of {}", p.getHeader().type, p.getHeader().timeSent, p.info.timeReceived, p.info.timeReceived - p.getHeader().timeSent);
				}
			});
		}
	}

	scenegraph->update(dt);
}

void Game::draw() const
{
	window->clear({ 230, 230, 230, 255 });
	scenegraph->draw(renderer.get());
	renderer->end();
	window->display();
}