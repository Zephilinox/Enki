#include "CollisionManager.hpp"

//STD
#include <experimental/vector>

//LIBS
#include <Enki/Scenegraph.hpp>

//SELF
#include "Bullet.hpp"
#include "Asteroid.hpp"
#include "Player.hpp"

CollisionManager::CollisionManager(enki::EntityInfo info, enki::GameData* data, CustomData* custom_data, sf::RenderWindow* window)
	: Entity(info, data)
	, custom_data(custom_data)
	, window(window)
{

}

void CollisionManager::onSpawn([[maybe_unused]]enki::Packet& p)
{
	auto console = spdlog::get("console");
}

void CollisionManager::update(float dt)
{
	auto bullets = game_data->scenegraph->findEntitiesByType("Bullet");
	auto asteroids = game_data->scenegraph->findEntitiesByType("Asteroid");
	auto player1 = game_data->scenegraph->findEntityByName<Player>("Player 1");
	auto player2 = game_data->scenegraph->findEntityByName<Player>("Player 2");
	auto player3 = game_data->scenegraph->findEntityByName<Player>("Player 3");
	auto player4 = game_data->scenegraph->findEntityByName<Player>("Player 4");

	const auto circlesColliding = [](sf::CircleShape& shape_one, sf::CircleShape& shape_two) -> bool
	{
		sf::Vector2f distance = shape_one.getPosition() - shape_two.getPosition();
		float length = std::sqrtf((distance.x * distance.x) + (distance.y * distance.y));
		return length < shape_one.getRadius() + shape_two.getRadius();
	};

	sf::CircleShape asteroidCS;
	for (auto& a : asteroids)
	{
		auto asteroid = static_cast<Asteroid*>(a);
		asteroidCS.setRadius(asteroid->getRadius());
		asteroidCS.setOrigin(asteroid->getRadius() / 2.0f, asteroid->getRadius() / 2.0f);
		asteroidCS.setPosition(asteroid->getPosition());

		sf::CircleShape bulletCS(5);
		bulletCS.setOrigin(2.5f, 2.5f);
		for (auto& b : bullets)
		{
			auto bullet = static_cast<Bullet*>(b);
			bulletCS.setPosition(bullet->getPosition());

			if (circlesColliding(asteroidCS, bulletCS))
			{
				asteroid->handleCollision();
				bullet->handleCollision();
			}
		}

		sf::CircleShape playerCS(16);
		playerCS.setOrigin(8.0f, 8.0f);

		const auto playerCheck = [asteroid, &asteroidCS, &playerCS, &circlesColliding](Player* player)
		{
			if (player)
			{
				playerCS.setPosition(player->getPosition());
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

void CollisionManager::draw(sf::RenderWindow& window_) const
{

}

void CollisionManager::serializeOnTick(enki::Packet& p)
{

}

void CollisionManager::deserializeOnTick(enki::Packet& p)
{

}