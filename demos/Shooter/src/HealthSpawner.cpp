#include "HealthSpawner.hpp"

//LIBS
#include <Enki/GameData.hpp>
#include <Enki/Scenetree.hpp>

//SELF
#include "Player.hpp"

HealthSpawner::HealthSpawner(enki::EntityInfo info, enki::GameData* game_data)
	: Entity(info, game_data)
{
	if (!spawner_texture.loadFromFile("resources/Spawner.png"))
	{
		auto console = spdlog::get("console");
		console->error("Failed to load spawner");
	}

	spawner.setTexture(spawner_texture);

	if (!pickup_texture.loadFromFile("resources/Health.png"))
	{
		auto console = spdlog::get("console");
		console->error("Failed to load health");
	}

	pickup.setTexture(pickup_texture);

	game_data->scenetree->rpc_man.add(enki::RPCType::REMOTE_AND_LOCAL, "HealthSpawner", "spawn", &HealthSpawner::spawn);
	game_data->scenetree->rpc_man.add(enki::RPCType::REMOTE_AND_LOCAL, "HealthSpawner", "pickedUp", &HealthSpawner::pickedUp);
}

void HealthSpawner::onSpawn(enki::Packet & p)
{
	if (p.canDeserialize<float, float>())
	{
		deserializeOnConnection(p);
	}
}

void HealthSpawner::draw(sf::RenderWindow & window) const
{
	window.draw(spawner);
	if (pickupAvailable)
	{
		window.draw(pickup);
	}
}

void HealthSpawner::update(float dt)
{
	if (isOwner())
	{
		elapsed_spawn_time += dt;
		if (elapsed_spawn_time > spawn_delay)
		{
			game_data->scenetree->rpc_man.call(&HealthSpawner::spawn, "spawn", game_data->network_manager, this);
			elapsed_spawn_time = 0;
		}
	}
}

void HealthSpawner::serializeOnConnection(enki::Packet& p)
{
	p << spawner.getPosition().x
		<< spawner.getPosition().y;
}

void HealthSpawner::deserializeOnConnection(enki::Packet& p)
{
	float x = p.read<float>();
	float y = p.read<float>();
	spawner.setPosition(x, y);
	pickup.setPosition(x, y);
}

void HealthSpawner::spawn()
{
	auto console = spdlog::get("console");
	console->info("Spawned HP");
	pickupAvailable = true;
}

void HealthSpawner::pickedUp(enki::EntityID playerID)
{
	pickupAvailable = false;
	auto player = static_cast<Player*>(game_data->scenetree->getEntity(playerID));
	player->increaseHP();
}
