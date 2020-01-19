#pragma once

//LIBS
#include <SFML/Graphics.hpp>

//SELF
#include <Enki/Entity.hpp>

class Collision : public enki::Entity
{
public:
	Collision(enki::EntityInfo info, enki::GameData* game_data);

	void onSpawn(enki::Packet p) final;
	void update(float dt) final;

private:
	sf::Rect<float> ball_collider;
	sf::Rect<float> paddle1_collider;
	sf::Rect<float> paddle2_collider;
};