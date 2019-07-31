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
	Player(enki::EntityInfo info, enki::GameData* data, CustomData* custom_data, sf::RenderWindow* window);

	void onSpawn(enki::Packet& p) final;

	void update(float dt) final;
	void draw(sf::RenderWindow& window) const final;

	void serializeOnTick(enki::Packet& p) final;
	void deserializeOnTick(enki::Packet& p) final;
	
	void handleCollision();

	sf::Vector2f getPosition() const;
	bool isInvincible() const;
	int getLives() const;
	sf::Color getColour() const;

	void startInvincible();
	void stopInvincible();

private:
	sf::Keyboard::Key up;
	sf::Keyboard::Key down;
	sf::Keyboard::Key left;
	sf::Keyboard::Key right;
	sf::Keyboard::Key shoot;
	sf::Keyboard::Key slow;

	CustomData* custom_data;
	sf::RenderWindow* window;
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
	float shoot_delay = 0.1f;
};