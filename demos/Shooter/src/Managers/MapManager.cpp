#include "MapManager.hpp"

//LIBS
#include <SFML/Graphics.hpp>
#include <spdlog/spdlog.h>
#include <Enki/Entity.hpp>

//SELF
#include "Floor.hpp"
#include "Wall.hpp"

MapManager::MapManager(enki::Scenegraph* scenegraph, enki::NetworkManager* network_manager)
	: scenegraph(scenegraph)
	, network_manager(network_manager)
{
	auto console = spdlog::get("console");
	sf::Image image;
	if (!image.loadFromFile("resources/map.png"))
	{
		console->error("failed to load map");
	}

	width = image.getSize().x;
	height = image.getSize().y;

	const sf::Uint8* pixels = image.getPixelsPtr();

	map.resize(image.getSize().y);

	for (auto& row : map)
	{
		row.resize(image.getSize().x);
	}

	for (unsigned int pixel = 0; pixel < image.getSize().x * image.getSize().y * 4; pixel += 4)
	{
		sf::Color c(pixels[pixel], pixels[pixel + 1], pixels[pixel + 2], pixels[pixel + 3]);

		int x = (pixel / 4) % image.getSize().x;
		int y = (pixel / 4) / image.getSize().x;

		if (c.r == 0 &&
			c.g == 0 &&
			c.b == 0)
		{
			map[y][x] = Tile::Wall;

		}
		else if (c.r == 255 &&
			c.g == 174 &&
			c.b == 201)
		{
			map[y][x] = Tile::Spawnpoint;
		}
		else if (c.r == 237 &&
			c.g == 28 &&
			c.b == 36)
		{
			map[y][x] = Tile::HealthPickup;
		}
		else if (c.r == 255 &&
			c.g == 255 &&
			c.b == 255)
		{
			map[y][x] = Tile::Floor;
		}
		else
		{
			map[y][x] = Tile::Unknown;
		}
	}

	/*for (auto& row : map)
	{
		std::string tiles;
		for (auto& tile : row)
		{
			tiles += std::to_string(tile) + " ";
		}
		tiles += ";";
		console->info(tiles);
	}*/
}

void MapManager::createMap()
{
	if (network_manager->client->getID() == 1)
	{
		int y = 0;
		for (auto& row : map)
		{
			int x = 0;
			for (auto& tile : row)
			{
				enki::Packet p;
				sf::Vector2f pos = mapPosToWorldPos(sf::Vector2i(x, y));
				p << pos.x << pos.y;

				switch (tile)
				{
				case Tile::Floor:
				{
					std::string name = std::string("Floor ") + std::to_string(x) + ", " + std::to_string(y);
					scenegraph->createNetworkedEntity({ "Floor", name }, p);
					break;
				}

				case Tile::Wall:
				{
					std::string name = std::string("Wall ") + std::to_string(x) + ", " + std::to_string(y);
					scenegraph->createNetworkedEntity({ "Wall", name }, p);
					break;
				}

				case Tile::HealthPickup:
				{
					std::string name = std::string("HealthSpawner ") + std::to_string(x) + ", " + std::to_string(y);
					scenegraph->createNetworkedEntity({ "HealthSpawner", name }, p);
					break;
				}
				}

				x++;
			}

			y++;
		}
	}
}

Tile MapManager::getTile(sf::Vector2i mapPos)
{
	if (mapPos.x >= 0 && mapPos.x < width &&
		mapPos.y >= 0 && mapPos.y < height)
	{
		return map[mapPos.y][mapPos.x];
	}

	return Tile::Unknown;
}

sf::Vector2i MapManager::worldPosToMapPos(sf::Vector2f worldPos)
{
	return sf::Vector2i(static_cast<int>(worldPos.x) / tile_size, static_cast<int>(worldPos.y) / tile_size);
}

sf::Vector2f MapManager::mapPosToWorldPos(sf::Vector2i mapPos)
{
	return sf::Vector2f(static_cast<float>(mapPos.x * tile_size), static_cast<float>(mapPos.y * tile_size));
}

float MapManager::getWidth()
{
	return width * static_cast<float>(tile_size);
}

float MapManager::getHeight()
{
	return height * static_cast<float>(tile_size);
}
