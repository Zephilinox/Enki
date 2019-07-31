#include "Wall.hpp"

Wall::Wall(enki::EntityInfo info, enki::GameData* game_data)
	: Entity(info, game_data)
{
	if (!texture.loadFromFile("resources/Wall.png"))
	{
		auto console = spdlog::get("console");
		console->error("Failed to load wall");
	}

	sprite.setTexture(texture);
}

void Wall::onSpawn(enki::Packet& p)
{
	if (p.canDeserialize<float, float>())
	{
		deserializeOnConnection(p);
	}
}

void Wall::draw(sf::RenderWindow& window) const
{
	window.draw(sprite);
}

void Wall::serializeOnConnection(enki::Packet &p)
{
	p << sprite.getPosition().x << sprite.getPosition().y;
}

void Wall::deserializeOnConnection(enki::Packet& p)
{
	float x = p.read<float>();
	float y = p.read<float>();
	sprite.setPosition(x, y);
}