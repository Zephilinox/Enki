#pragma once

//SELF
#include "Managers/InputManager.hpp"

//LIBS
#include <Enki/Scenetree.hpp>
#include <Enki/Window/Window.hpp>
#include "Enki/Managers/InputManager.hpp"
#include "Enki/Managers/FontManager.hpp"
#include "Enki/Managers/TextureManager.hpp"

struct CustomData
{
	bool window_active = true;
	enki::Window* window = nullptr;
	sf::RenderWindow* window_sfml = nullptr;
	enki::Scenetree* scenetree = nullptr;
	enki::NetworkManager* network_manager = nullptr;
	enki::InputManager* input_manager = nullptr;
	enki::FontManager* font_manager = nullptr;
	enki::TextureManager* texture_manager = nullptr;
};
