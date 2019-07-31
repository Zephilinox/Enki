#pragma once

//LIBS
#include <Enki/Entity.hpp>
#include <Enki/Signals/Signal.hpp>
#include <SFML/Graphics.hpp>

//SELF
#include "CustomData.hpp"

class CollisionManager : public enki::Entity
{
public:
	CollisionManager(enki::EntityInfo info, enki::GameData* data, CustomData* custom_data, sf::RenderWindow* window);

	void onSpawn(enki::Packet& p) final;

	void update(float dt) final;
	void draw(sf::RenderWindow& window) const final;

	void serializeOnTick(enki::Packet& p) final;
	void deserializeOnTick(enki::Packet& p) final;

private:
	CustomData* custom_data;
	sf::RenderWindow* window;
};