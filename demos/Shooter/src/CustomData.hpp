#pragma once

//SELF
#include "Managers/InputManager.hpp"
#include "Managers/MapManager.hpp"

struct CustomData
{
	bool window_active = true;
	InputManager* input_manager;
	MapManager* map_manager;
};