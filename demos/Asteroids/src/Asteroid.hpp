#pragma once

//LIBS
#include <Enki/Entity.hpp>
#include <Enki/Signals/Signal.hpp>
#include <SFML/Graphics.hpp>

//SELF
#include "CustomData.hpp"

class Asteroid : public enki::Entity
{
public:
	Asteroid(enki::EntityInfo info, CustomData* custom_data);

	void onSpawn(enki::Packet p) final;

	void update(float dt) final;
	void draw(enki::Renderer* renderer) final;

	void serializeOnConnection(enki::Packet& p) final;
	void deserializeOnConnection(enki::Packet& p) final;
	void serializeOnTick(enki::Packet& p) final;
	void deserializeOnTick(enki::Packet& p) final;
	std::vector<std::pair<std::string, std::string>> serializeToStrings() const final;
	
	void handleCollision();
	void split();

	bool isAlive() const;
	bool canSplit() const;
	sf::Vector2f getPosition() const;
	float getRadius() const;
	float getRotation() const;

private:
	void constructAsteroid(unsigned sides, float x, float y);
	void createShape(unsigned sides);

	sf::ConvexShape shape;

	CustomData* const custom_data;

	float speed = 300;
	float radius = 10;
	float rotation_speed = 200;
	sf::Vector2f velocity = {1, 1};
	bool alive = true;
};