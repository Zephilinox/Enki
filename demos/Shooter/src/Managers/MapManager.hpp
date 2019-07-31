#pragma once

//STD
#include <vector>

//LIB
#include <SFML/Graphics.hpp>
#include <Enki/Scenegraph.hpp>

enum Tile
{
	Unknown = -1,
	Floor,
	Wall,
	HealthPickup,
	Spawnpoint
};

class MapManager
{
public:
	MapManager(enki::Scenegraph* scenegraph, enki::NetworkManager* network_manager);

	void createMap();
	
	Tile getTile(sf::Vector2i mapPos);

	sf::Vector2i worldPosToMapPos(sf::Vector2f);
	sf::Vector2f mapPosToWorldPos(sf::Vector2i);

	float getWidth();
	float getHeight();

private:
	int width;
	int height;
	int tile_size = 64;
	std::vector<std::vector<Tile>> map;
	enki::Scenegraph* scenegraph;
	enki::NetworkManager* network_manager;
};