#include "Game.hpp"

//LIBS
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <Enki/Networking/ServerHost.hpp>
#include <Enki/Window/Window.hpp>
#include <Enki/Window/WindowSFML.hpp>
#include <Enki/Messages/MessageFunction.hpp>
#include <Enki/GUI/IMGUI/imgui_SFML.h> //todo: rename to .hpp so clang format can sort it properly
#include <Enki/GUI/Console.hpp>
#include <Enki/Entity.hpp>

//SELF
#include "Player.hpp"
#include "Asteroid.hpp"
#include "Bullet.hpp"
#include "CollisionManager.hpp"
#include "PlayerText.hpp"
#include "CustomData.hpp"

Game::Game()
	: window(std::make_unique<enki::WindowSFML>(enki::Window::Properties{1280, 720, "Enki Asteroids Demo", true}))
	, scenetree(&network_manager)
	, renderer(window->as<enki::WindowSFML>()->getRawWindow())
{
	ImGui::SFML::Init(*static_cast<enki::WindowSFML*>(window.get())->getRawWindow());
	
	spdlog::stdout_color_mt("console");
	auto console = spdlog::get("console");

	input_manager.window = window.get();

	custom_data.input_manager = &input_manager;

	auto enki_logger = spdlog::get("Enki");
	enki_logger->set_level(spdlog::level::warn);

	custom_data.scenetree = &scenetree;
	custom_data.network_manager = &network_manager;
	custom_data.window = window.get();
	custom_data.window_sfml = window->as<enki::WindowSFML>()->getRawWindow();
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
	scenetree.registerEntity<enki::Console>(hash("Console"), {}, &scenetree);

	custom_data.scenetree->rpc_man.registerEntityRPC(enki::RPCType::REMOTE_AND_LOCAL, hash("Player"), "startInvincible", &Player::startInvincible);
	custom_data.scenetree->rpc_man.registerEntityRPC(enki::RPCType::REMOTE_AND_LOCAL, hash("Player"), "stopInvincible", &Player::stopInvincible);

	message_queue.addListener<enki::MessageFunction&>([](enki::MessageFunction& msg) {
		msg.execute();
	});

	scenetree.createEntityLocal(hash("Console"), "Console");
	
	run();
}

void Game::run()
{
	//custom_data.window_sfml->setFramerateLimit(120);

	float dt = 1.0f / 60.0f;
	enki::Timer display_fps_timer;
	auto console = spdlog::get("console");
	
	while (window->isOpen())
	{
		input();
		update(dt);
		draw();

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
		ImGui::SFML::ProcessEvent(e);
		
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


sf::Vector2f asteroidPos()
{
	bool vertical_horizontal = std::rand() % 2;
	bool side = std::rand() % 2;
	float x = 0;
	float y = 0;
	if (vertical_horizontal)
	{
		x = std::rand() % 1280;
		if (side)
		{
			y = 0 - 100;
		}
		else
		{
			y = 720 + 100;
		}
	}
	else
	{
		y = std::rand() % 720;
		if (side)
		{
			x = 0 - 100;
		}
		else
		{
			x = 1280 + 100;
		}
	}

	return {x, y};
}
void Game::update(float dt)
{
	ImGui::SFML::Update(*static_cast<enki::WindowSFML*>(window.get())->getRawWindow(), dt);
	
	network_manager.update();
	input_manager.update();

	static bool networking = false;

	if (network_manager.server)
	{
		ImGui::Begin(" ", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::SetWindowPos({300, 50}, ImGuiCond_Once);

		const auto asteroidCount = custom_data.scenetree->findEntitiesByType(hash("Asteroid")).size();
		const bool spawnAsteroids = asteroid_spawn_timer.getElapsedTime() > 1.0f && asteroidCount < 50;
		const auto str = fmt::format("Spawn Asteroid {}", asteroidCount);
		if (ImGui::Button(str.c_str(), {160, 30}) || spawnAsteroids)
		{
			enki::Packet p;

			auto pos = asteroidPos();
			p << (std::rand() % 8) + 5
			  << pos.x
			  << pos.y
			  << static_cast<float>((std::rand() % 200) + 50);
			scenetree.createEntityNetworkedRequest(hash("Asteroid"), "Asteroid", 0, p, {});
			asteroid_spawn_timer.restart();
		}
		ImGui::End();
	}

	if (!networking && custom_data.window_active)
	{
		const auto startServer = [&]() {
			networking = true;
			network_manager.startHost();
			scenetree.enableNetworking();
			scenetree.createEntityNetworkedRequest(hash("Player"), "Player 1");
			scenetree.createEntityLocal(hash("CollisionManager"), "CollisionManager");

			for (int i = 0; i < 20; ++i)
			{
				enki::Packet p;
				auto pos = asteroidPos();
				p << (std::rand() % 8) + 5
				  << pos.x
				  << pos.y
				  << static_cast<float>((std::rand() % 200) + 50);
				scenetree.createEntityNetworkedRequest(hash("Asteroid"), "Asteroid", 0, p);
			}

			mc1 = network_manager.server->on_packet_received.connect([this](enki::Packet p) {
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
		};

		const auto startClient = [&]() {
			networking = true;
			network_manager.startClient();
			scenetree.enableNetworking();
			scenetree.createEntityLocal(hash("CollisionManager"), "CollisionManager");
		};
		
		ImGui::Begin("Pick", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::SetWindowSize({400, 300}, ImGuiCond_Always);
		ImGui::SetWindowPos({100, 100}, ImGuiCond_Always);
		
		if (ImGui::Button("Server", {60, 30}))
		{
			startServer();
		}
		
		if (ImGui::Button("Client", {60, 30}))
		{
			startClient();
		}

		std::string buffer(custom_data.network_manager->server_ip);
		buffer.resize(256, '\0');
		ImGui::InputText("IP", buffer.data(), buffer.size(), ImGuiInputTextFlags_None, nullptr, nullptr);
		custom_data.network_manager->server_ip = buffer;

		int port = custom_data.network_manager->server_port;
		ImGui::InputInt("Port", &port);
		custom_data.network_manager->server_port = port;
		
		ImGui::End();
	}

	scenetree.update(dt);
	
	ImGui::Begin("Framerate", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	ImGui::SetWindowSize({200, 30}, ImGuiCond_FirstUseEver);
	ImGui::SetWindowPos({2, 2}, ImGuiCond_FirstUseEver);
	ImGui::Text("%s", fmt::format("{:.3f}ms/frame ({:.1f} FPS)", dt * 1000.0f, 1.0f / dt).c_str());
	ImGui::End();
}

void Game::draw()
{
	window->clear(40, 40, 40);
	scenetree.draw(&renderer);
	renderer.end();
	ImGui::SFML::Render(*static_cast<enki::WindowSFML*>(window.get())->getRawWindow());
	window->display();
	FrameMark
}