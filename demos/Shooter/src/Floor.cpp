#include "Floor.hpp"

Floor::Floor(enki::EntityInfo info, enki::GameData* game_data)
	: Entity(info, game_data)
{
	if (!texture.loadFromFile("resources/Floor.png"))
	{
		auto console = spdlog::get("console");
		console->error("Failed to load floor");
	}

	sprite.setTexture(texture);
}

void Floor::onSpawn(enki::Packet& p)
{
	if (p.canDeserialize<float, float>())
	{
		deserializeOnConnection(p);
	}
}

void Floor::draw(sf::RenderWindow& window) const
{
	window.draw(sprite);
}

void Floor::serializeOnConnection(enki::Packet& p)
{
	p << sprite.getPosition().x << sprite.getPosition().y;
}

void Floor::deserializeOnConnection(enki::Packet& p)
{
	float x = p.read<float>();
	float y = p.read<float>();
	sprite.setPosition(x, y);
}
