#pragma once

//LIBS
#include <Enki/Entity.hpp>
#include <Enki/Signals/Signal.hpp>

//SELF
#include "CustomData.hpp"

class Asteroid;

class Bullet : public enki::Entity
{
public:
	Bullet(enki::EntityInfo info, CustomData* custom_data);

	void onSpawn(enki::Packet p) final;

	void update(float dt) final;
	void draw(enki::Renderer* renderer) final;

	void serializeOnConnection(enki::Packet& p) final;
	void deserializeOnConnection(enki::Packet& p) final;
	void serializeOnTick(enki::Packet& p) final;
	void deserializeOnTick(enki::Packet& p) final;
	std::vector<std::pair<std::string, std::string>> serializeToStrings() const final;
	
	void handleCollision();
	enki::Vector2 getPosition() const;
	float getRotation() const;
	enki::Colour getColour() const;
	unsigned int getWarpCount() const;

	bool isAlive() const;

private:
	CustomData* const custom_data;

	enki::Texture* bullet_tex;
	std::unique_ptr<enki::Sprite> bullet;

	float speed = 300;
	enki::Vector2 velocity = { 1, 1 };

	unsigned int warp_count = 0;
	bool alive = true;

	enki::Timer find_asteroid_timer;
	float find_asteroid_delay = 1.0f;
	enki::EntityID closest_asteroid_id = 0;
	enki::Vector2 distance_to_asteroid = {0, 0};
};