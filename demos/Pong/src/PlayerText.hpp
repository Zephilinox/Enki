#pragma once

//LIBS
#include <SFML/Graphics.hpp>

//SELF
#include <Enki/Entity.hpp>

class PlayerText : public enki::Entity
{
public:
	PlayerText(enki::EntityInfo info, enki::GameData* game_data);

	void onSpawn(enki::Packet& p) final;
	void update(float dt) final;
	void draw(sf::RenderWindow& window) const final;

private:
	sf::Font font;
	sf::Text label;
};