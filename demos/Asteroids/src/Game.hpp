#pragma once

//LIBS
#include <Enki/Scenetree.hpp>
#include <Enki/Timer.hpp>
#include <Enki/Networking/RPC.hpp>

//SELF
#include "CustomData.hpp"
#include "Enki/Managers/InputManager.hpp"
#include "Enki/Messages/MessageQueue.hpp"

class Game
{
public:
	Game();

private:
	void run();
	void input();
	void update(float dt);
	void draw();

	std::unique_ptr<enki::Window> window;
	enki::Timer timer;
	enki::NetworkManager network_manager;
	enki::Scenetree scenetree;
	enki::InputManager input_manager;
	enki::Renderer renderer;
	enki::MessageQueue message_queue;
	enki::FontManager font_manager;
	CustomData custom_data;

	enki::ManagedConnection mc1;
	enki::Timer asteroid_spawn_timer;
};
