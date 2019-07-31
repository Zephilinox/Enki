#pragma once

//STD
#include <set>

//LIBS
#include <Enki/Scenegraph.hpp>
#include <Enki/Timer.hpp>
#include <Enki/Networking/RPC.hpp>
#include <Enki/GameData.hpp>

//SELF
#include "CustomData.hpp"

class Game
{
public:
	Game();

private:
	void run();
	void input();
	void update();
	void draw() const;

	float dt = 1.0f / 60.0f;

	enki::Timer timer;
	std::unique_ptr<sf::RenderWindow> window;
	std::unique_ptr<enki::Scenegraph> scenegraph;
	std::unique_ptr<enki::GameData> game_data;
	std::unique_ptr<enki::NetworkManager> network_manager;
	std::unique_ptr<CustomData> custom_data;
	InputManager input_manager;
	std::unique_ptr<MapManager> map_manager;

	std::set<enki::ClientID> players;

	enki::ManagedConnection mc1;
};