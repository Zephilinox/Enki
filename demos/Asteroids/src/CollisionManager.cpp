#include "CollisionManager.hpp"

//SELF
#include "Bullet.hpp"
#include "Asteroid.hpp"
#include "Player.hpp"

//LIBS
#include <Enki/Scenetree.hpp>

//STD
#include <experimental/vector>

CollisionManager::CollisionManager(enki::EntityInfo info, CustomData* custom_data)
	: Entity(std::move(info))
	, custom_data(custom_data)
{

}

void CollisionManager::onSpawn([[maybe_unused]]enki::Packet p)
{
	auto console = spdlog::get("console");
}

void CollisionManager::update(float)
{
	auto bullets = custom_data->scenetree->findEntitiesByType(hash("Bullet"));
	auto asteroids = custom_data->scenetree->findEntitiesByType(hash("Asteroid"));
	auto* player1 = custom_data->scenetree->findEntityByName<Player>("Player 1");
	auto* player2 = custom_data->scenetree->findEntityByName<Player>("Player 2");
	auto* player3 = custom_data->scenetree->findEntityByName<Player>("Player 3");
	auto* player4 = custom_data->scenetree->findEntityByName<Player>("Player 4");

	struct CircleShape
	{
		enki::Vector2 position{};
		float radius{};
	};
	
	const auto circlesColliding = [](const CircleShape& shape_one, const CircleShape& shape_two) -> bool
	{
		const enki::Vector2 distance = {
			shape_one.position.x - shape_two.position.x,
			shape_one.position.y - shape_two.position.y,
		};
		const float length = (distance.x * distance.x) + (distance.y * distance.y);
		return length < (shape_one.radius * shape_one.radius) + (shape_two.radius * shape_two.radius);
	};

	CircleShape asteroidCS;
	for (auto* a : asteroids)
	{
		auto* asteroid = static_cast<Asteroid*>(a);
		asteroidCS.radius = asteroid->getRadius();
		asteroidCS.position = asteroid->getPosition();

		CircleShape bulletCS{{}, 5};
		if (asteroid->isAlive())
		{
			for (auto* b : bullets)
			{
				auto* bullet = static_cast<Bullet*>(b);
				bulletCS.position = bullet->getPosition();

				if (circlesColliding(asteroidCS, bulletCS))
				{
					asteroid->handleCollision();
					bullet->handleCollision();
				}
			}
		}

		CircleShape playerCS{{}, 16};

		const auto playerCheck = [asteroid, &asteroidCS, &playerCS, &circlesColliding](Player* player)
		{
			if (player)
			{
				playerCS.position = player->getPosition();
				if (circlesColliding(asteroidCS, playerCS))
				{
					if (!player->isInvincible())
					{
						asteroid->handleCollision();
					}
					player->handleCollision();
				}
			}
		};

		playerCheck(player1);
		playerCheck(player2);
		playerCheck(player3);
		playerCheck(player4);
	}
}

void CollisionManager::draw(enki::Renderer*)
{

}

void CollisionManager::serializeOnTick(enki::Packet&)
{

}

void CollisionManager::deserializeOnTick(enki::Packet&)
{

}