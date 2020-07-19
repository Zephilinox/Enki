#include "Game.hpp"

//LIBS
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <SFML/Graphics.hpp>
#include <Enki/Networking/ServerHost.hpp>
#include <Enki/Window/WindowSFML.hpp>
#include <Enki/Messages/MessageFunction.hpp>

//SELF
#include "Player.hpp"
#include "Asteroid.hpp"
#include "Bullet.hpp"
#include "CollisionManager.hpp"
#include "PlayerText.hpp"
#include "CustomData.hpp"

Game::Game()
	: window(std::make_unique<enki::WindowSFML>(enki::Window::Properties{1280, 720, "Enki Asteroids Demo", false}))
	, scenetree(&network_manager)
	, renderer(window->as<enki::WindowSFML>()->getRawWindow())
{
	spdlog::stdout_color_mt("console");
	auto console = spdlog::get("console");

	input_manager.window = window.get();
	
	custom_data.input_manager = &input_manager;

	auto enki_logger = spdlog::get("Enki");
	enki_logger->set_level(spdlog::level::critical);

	custom_data.scenetree = &scenetree;
	custom_data.network_manager = &network_manager;
	custom_data.window = window.get();
	custom_data.font_manager = &font_manager;
	custom_data.texture_manager = &texture_manager;

	auto player_children = std::vector<enki::EntityChildCreationInfo>{
		{hash("PlayerText"), "PlayerText"},
	};
	
	custom_data.scenetree->registerEntity<Player>(hash("Player"), std::move(player_children), &custom_data);
	custom_data.scenetree->registerEntity<Asteroid>(hash("Asteroid"), {}, &custom_data);
	custom_data.scenetree->registerEntity<Bullet>(hash("Bullet"), {}, &custom_data);
	custom_data.scenetree->registerEntity<CollisionManager>(hash("CollisionManager"), {}, &custom_data);
	custom_data.scenetree->registerEntity<PlayerText>(hash("PlayerText"), {}, &custom_data);

	custom_data.scenetree->rpc_man.registerEntityRPC(enki::RPCType::REMOTE_AND_LOCAL, hash("Player"), "startInvincible", &Player::startInvincible);
	custom_data.scenetree->rpc_man.registerEntityRPC(enki::RPCType::REMOTE_AND_LOCAL, hash("Player"), "stopInvincible", &Player::stopInvincible);

	message_queue.addListener<enki::MessageFunction&>([](enki::MessageFunction& msg) {
		msg.execute();
	});
	
	run();
}

void Game::run()
{
	float dt = 1.0f / 60.0f;
	
	enki::Timer display_fps_timer;
	auto console = spdlog::get("console");

	enki::Timer draw_timer;
	
	while (window->isOpen())
	{
		input_manager.update();
		input();
		//todo: could this be part of the scene tree update? does it ever need to be run at a different stage?
		scenetree.processMessages();
		update(dt);

		if (draw_timer.getElapsedTime() > 1.0f / 60.0f)
		{
			draw();
			draw_timer.restart();
		}

		if (display_fps_timer.getElapsedTime() > 5.0f)
		{
			message_queue.sendPriorityMessage<enki::MessageFunction>([dt]() {
				spdlog::info("FPS {}", 1.0f / dt);
				spdlog::info("MS {}", dt * 1000.0f);
			});
			
			display_fps_timer.restart();
		}
		
		dt = timer.getElapsedTime();
		timer.restart();
	}
}

void Game::input()
{
	enki::Event e;
	while (window->poll(e))
	{
		auto visitor = enki::overload{
			[&](enki::EventQuit) {
				window->close();
			},
			[&](enki::EventFocus e) {
				custom_data.window_active = e.focused;
			},
			[](auto&) {

			},
		};

		std::visit(visitor, e);

		input_manager.input(e);
		scenetree.input(e);
	}
}

void Game::update(float dt)
{
	message_queue.processMessages(2ms);
	network_manager.update();
	input_manager.update();

	static bool networking = false;

	if (network_manager.server &&
		(input_manager.isKeyPressed(enki::Keyboard::Key::F2) ||
			asteroid_spawn_timer.getElapsedTime() > 1.0f))
	{
		enki::Packet p;
		p << (std::rand() % 8) + 5
			<< static_cast<float>(std::rand() % 1280)
			<< static_cast<float>(std::rand() % 720)
			<< static_cast<float>((std::rand() % 200) + 50);
		scenetree.createEntityNetworkedRequest(hash("Asteroid"), "Asteroid", 0, p, {});
		asteroid_spawn_timer.restart();
	}

	if (!networking && custom_data.window_active)
	{
		if (input_manager.isKeyPressed(enki::Keyboard::Key::S))
		{
			networking = true;
			network_manager.startHost();
			scenetree.enableNetworking();
			scenetree.createEntityNetworkedRequest(hash("Player"), "Player 1");
			scenetree.createEntityLocal(hash("CollisionManager"), "CollisionManager");

			for (int i = 0; i < 20; ++i)
			{
				enki::Packet p;
				p << (std::rand() % 8) + 5
					<< static_cast<float>(std::rand() % 1280)
					<< static_cast<float>(std::rand() % 720)
					<< static_cast<float>((std::rand() % 200) + 50);
				scenetree.createEntityNetworkedRequest(hash("Asteroid"), "Asteroid", 0, p);
			}

			mc1 = network_manager.server->on_packet_received.connect([this](enki::Packet p)
			{
				if (p.getHeader().type == enki::PacketType::CONNECTED)
				{
					scenetree.createEntityNetworkedFromRequest(
						{hash("Player"), "Player " + std::to_string(p.info.senderID), 0, p.info.senderID},
						{},
						{});
				}

				if (p.getHeader().type == enki::PacketType::DISCONNECTED)
				{
					auto ent = scenetree.findEntityByName("Player " + std::to_string(p.info.senderID));
					if (ent)
					{
						scenetree.deleteEntity(ent->info.ID);
					}
				}
			});
		}

		if (input_manager.isKeyPressed(enki::Keyboard::Key::C))
		{
			networking = true;
			network_manager.startClient();
			scenetree.enableNetworking();
			scenetree.createEntityLocal(hash("CollisionManager"), "CollisionManager");
		}
	}

	scenetree.update(dt);
	scenetree.processMessages(true);
	scenetree.finishUpdate();
}

void Game::draw()
{
	window->clear(40, 40, 40);
	scenetree.draw(&renderer);
	window->display();
}