#pragma once

//LIBS
#include <Enki/Entity.hpp>
#include <Enki/Signals/Signal.hpp>
#include <SFML/Graphics.hpp>

//SELF
#include "CustomData.hpp"

class Bullet final : public enki::Entity
{
public:
	Bullet(enki::EntityInfo info, CustomData* custom_data);
	std::unique_ptr<Entity> clone() final;
	
	void onSpawn(enki::Packet p) final;

	void update(float dt) final;
	void draw(enki::Renderer* renderer) final;

	void serializeOnConnection(enki::Packet& p) final;
	void deserializeOnConnection(enki::Packet& p) final;
	void serializeOnTick(enki::Packet& p) final;
	void deserializeOnTick(enki::Packet& p) final;
	
	void handleCollision();
	sf::Vector2f getPosition() const;
	float getRotation() const;
	sf::Color getColour() const;
	unsigned int getWarpCount() const;

	bool isAlive() const;

private:
	CustomData* custom_data;
	sf::RenderWindow* window;

	sf::Texture bullet_tex;
	sf::Sprite bullet;

	float speed = 300;
	sf::Vector2f velocity = { 1, 1 };

	unsigned int warp_count = 0;
	bool alive = true;
};