#pragma once

//LIBS
#include <Enki/Scenetree.hpp>
#include <Enki/Managers/InputManager.hpp>

struct CustomData
{
	bool window_active = true;
	enki::InputManager* input_manager;
	enki::NetworkManager* network_manager;
	enki::Scenetree* scenetree;
	enki::Window* window;
};