#pragma once

//LIBS
#include <Enki/Entity.hpp>

class HealthSpawner : public enki::Entity
{
public:
	HealthSpawner(enki::EntityInfo info, enki::GameData* game_data);

	void onSpawn(enki::Packet& p) final;
	void draw(sf::RenderWindow& window) const final;
	void update(float dt) final;

	void serializeOnConnection(enki::Packet& p) final;
	void deserializeOnConnection(enki::Packet& p) final;

private:
	void spawn();
	void pickedUp(enki::EntityID playerID);

	bool pickupAvailable = false;

	sf::Texture spawner_texture;
	sf::Texture pickup_texture;
	sf::Sprite spawner;
	sf::Sprite pickup;
	sf::Rect<float> collider;

	float elapsed_spawn_time = 0;
	float spawn_delay = 1;
};