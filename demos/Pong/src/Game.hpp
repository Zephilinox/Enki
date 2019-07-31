#pragma once

//SELF
#include <Enki/Scenegraph.hpp>
#include <Enki/Timer.hpp>
#include <Enki/Networking/RPC.hpp>
#include <Enki/Managers/NetworkManager.hpp>
#include <Enki/GameData.hpp>

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

	std::unique_ptr<sf::RenderWindow> window;
	std::unique_ptr<enki::Scenegraph> scenegraph;
	std::unique_ptr<enki::GameData> game_data;
	std::unique_ptr<enki::NetworkManager> network_manager;

	enki::Timer timer;

	enki::ManagedConnection mc1;
	enki::ManagedConnection mc2;
	enki::ManagedConnection mc3;
};