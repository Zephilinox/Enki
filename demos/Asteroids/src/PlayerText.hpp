#pragma once

//LIBS
#include <SFML/Graphics.hpp>

//LIBS
#include <Enki/Entity.hpp>

//SELF
#include "CustomData.hpp"

class PlayerText : public enki::Entity
{
public:
	PlayerText(enki::EntityInfo info, CustomData* custom_data);

	void onSpawn(enki::Packet p) final;
	void update(float dt) final;
	void draw(enki::Renderer* renderer) final;
	std::vector<std::pair<std::string, std::string>> serializeToStrings() const final;

private:
	CustomData* const custom_data;
	sf::Text label;
};