#pragma once

//LIBS
#include <Enki/Entity.hpp>

class Floor : public enki::Entity
{
public:
	Floor(enki::EntityInfo info, enki::GameData* game_data);
	
	void onSpawn(enki::Packet& p) final;
	void draw(sf::RenderWindow& window) const final;

	void serializeOnConnection(enki::Packet& p) final;
	void deserializeOnConnection(enki::Packet& p) final;

private:
	sf::Texture texture;
	sf::Sprite sprite;
};