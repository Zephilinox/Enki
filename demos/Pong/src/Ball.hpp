#pragma once

//LIBS
#include <SFML/Graphics.hpp>
#include <Enki/Entity.hpp>

class Ball : public enki::Entity
{
public:
	Ball(enki::EntityInfo info, enki::GameData* game_data);

	void onSpawn(enki::Packet& p) final;
	void update(float dt) final;
	void draw(sf::RenderWindow& window) const final;

	void serializeOnConnection(enki::Packet& p) final;
	void deserializeOnConnection(enki::Packet& p) final;

	void serializeOnTick(enki::Packet& p) final;
	void deserializeOnTick(enki::Packet& p) final;

	sf::Sprite sprite;
	sf::Texture texture;

	bool moving_left = true;
	int y_dir = 1;
	float y_speed = 300;

private:
	enki::ManagedConnection mc1;
};