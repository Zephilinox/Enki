#pragma once

//STD
#include <set>

//LIBS
#include <Enki/Scenetree.hpp>
#include <Enki/Timer.hpp>
#include <Enki/Networking/RPC.hpp>
#include <Enki/Managers/InputManager.hpp>
#include <Enki/Messages/MessageQueue.hpp>

//SELF
#include "CustomData.hpp"

class Game
{
public:
	Game();

private:
	void run();
	void input();
	void update(float dt);
	void draw();

	enki::Timer timer;
	CustomData custom_data;
	std::unique_ptr<enki::Window> window;
	enki::NetworkManager network_manager;
	enki::Scenetree scenetree;
	enki::InputManager input_manager;
	enki::Renderer renderer;
	enki::MessageQueue message_queue;

	enki::ManagedConnection mc1;
	enki::Timer asteroid_spawn_timer;
};