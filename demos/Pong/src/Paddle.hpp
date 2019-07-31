#pragma once

//LIBS
#include <SFML/Graphics.hpp>
#include <Enki/Entity.hpp>
#include <Enki/Timer.hpp>

class Paddle : public enki::Entity
{
public:
	Paddle(enki::EntityInfo info, enki::GameData* game_data);

	void onSpawn(enki::Packet& p) final;
	void input(sf::Event& e) final;
	void update(float dt) final;
	void draw(sf::RenderWindow& window) const final;

	void serializeOnConnection(enki::Packet& p) final;
	void deserializeOnConnection(enki::Packet& p) final;

	void serializeOnTick(enki::Packet& p) final;
	void deserializeOnTick(enki::Packet& p) final;

	sf::Sprite sprite;
	sf::Texture texture;

	void setColour(int r, int g, int b);

private:
	bool interpolation_enabled = false;
	sf::Sprite latest_sprite;
	float latest_network_y = 0;
	float last_interpolation_y = 0;
	float interpolation_y = 0;
	enki::Timer interpolation_timer;
};