#pragma once

//LIBS
#include <Enki/Scenetree.hpp>
#include <Enki/Window/Window.hpp>
#include "Enki/Managers/InputManager.hpp"
#include "Enki/Managers/FontManager.hpp"
#include "Enki/Managers/TextureManager.hpp"
#include "Enki/Renderer/Renderer.hpp"

struct CustomData
{
	bool window_active = true;
	enki::Window* window = nullptr;
	enki::Renderer* renderer = nullptr;
	enki::Scenetree* scenetree = nullptr;
	enki::NetworkManager* network_manager = nullptr;
	enki::InputManager* input_manager = nullptr;
	enki::FontManager* font_manager = nullptr;
	enki::TextureManager* texture_manager = nullptr;
};
