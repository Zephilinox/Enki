#pragma once

//LIBS
#include <Enki/Entity.hpp>
#include <Enki/Signals/Signal.hpp>
#include <SFML/Graphics.hpp>

//SELF
#include "CustomData.hpp"

class Player : public enki::Entity
{
public:
	Player(enki::EntityInfo info, CustomData* custom_data);

	void onSpawn(enki::Packet p) final;

	void update(float dt) final;
	void draw(enki::Renderer* renderer) final;

	void serializeOnTick(enki::Packet& p) final;
	void deserializeOnTick(enki::Packet& p) final;
	void serializeOnConnection(enki::Packet& p) final;
	void deserializeOnConnection(enki::Packet& p) final;
	std::vector<std::pair<std::string, std::string>> serializeToStrings() const final;
	
	void handleCollision();

	sf::Vector2f getPosition() const;
	bool isInvincible() const;
	int getLives() const;
	sf::Color getColour() const;

	void startInvincible();
	void stopInvincible();

private:
	enki::Keyboard::Key up;
	enki::Keyboard::Key down;
	enki::Keyboard::Key left;
	enki::Keyboard::Key right;
	enki::Keyboard::Key shoot;
	enki::Keyboard::Key slow;

	CustomData* const custom_data;
	sf::View view;

	sf::Texture ship_tex;
	sf::Sprite ship;

	sf::Vector2f velocity;
	float speed = 300;
	float max_velocity_length = 600;
	int lives = 10;
	bool was_damaged = false;

	enki::Timer flashing_timer;
	float flashing_duration = 1.0f;

	enki::Timer shoot_timer;
	float shoot_delay = 0.4f;
};