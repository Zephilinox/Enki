#pragma once

//LIBS
#include <SFML/Graphics.hpp>

//SELF
#include <Enki/Entity.hpp>
#include "CustomData.hpp"

class PlayerText final : public enki::Entity
{
public:
	PlayerText(enki::EntityInfo info, CustomData* custom_data);
	std::unique_ptr<Entity> clone() final;
	void onSpawn(enki::Packet p) final;
	void update(float dt) final;
	void draw(enki::Renderer* renderer) final;

private:
	CustomData* custom_data;
	sf::Font font;
	sf::Text label;
};