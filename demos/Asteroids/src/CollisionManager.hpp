#pragma once

//LIBS
#include <Enki/Entity.hpp>
#include <Enki/Signals/Signal.hpp>
#include <SFML/Graphics.hpp>

//SELF
#include "CustomData.hpp"

class CollisionManager final : public enki::Entity
{
public:
	CollisionManager(enki::EntityInfo info, CustomData* custom_data);

	std::unique_ptr<enki::Entity> clone() final;
	
	void onSpawn(enki::Packet p) final;

	void update(float dt) final;
	void draw(enki::Renderer* renderer) final;

	void serializeOnTick(enki::Packet& p) final;
	void deserializeOnTick(enki::Packet& p) final;

private:
	CustomData* custom_data;
	sf::RenderWindow* window;
};