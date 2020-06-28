#pragma once

//LIBS
#include <Enki/Scenetree.hpp>
#include <Enki/Managers/InputManager.hpp>
#include <Enki/Managers/FontManager.hpp>
#include <Enki/Managers/SoundManager.hpp>
#include <Enki/Managers/TextureManager.hpp>

struct CustomData
{
	bool window_active = true;
	enki::InputManager* input_manager = nullptr;
	enki::NetworkManager* network_manager = nullptr;
	enki::FontManager* font_manager = nullptr;
	enki::SoundManager* sound_manager = nullptr;
	enki::TextureManager* texture_manager = nullptr;
	enki::Scenetree* scenetree = nullptr;
	enki::Window* window = nullptr;
};
