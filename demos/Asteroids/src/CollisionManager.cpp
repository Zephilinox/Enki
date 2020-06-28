#include "CollisionManager.hpp"

//STD
#include <experimental/vector>

//LIBS
#include <Enki/Scenetree.hpp>
#include <Enki/Window/WindowSFML.hpp>
#include <Enki/Messages/MessageFunction.hpp>

//SELF
#include "Bullet.hpp"
#include "Asteroid.hpp"
#include "Player.hpp"

CollisionManager::CollisionManager(enki::EntityInfo info, CustomData* custom_data)
	: Entity(info)
	, custom_data(custom_data)
	, window(custom_data->window->as<enki::WindowSFML>()->getRawWindow())
{

}

void CollisionManager::onSpawn([[maybe_unused]]enki::Packet p)
{
	auto console = spdlog::get("console");
}

std::unique_ptr<enki::Entity> CollisionManager::clone()
{
	return std::make_unique<CollisionManager>(*this);
}

void CollisionManager::update(float dt)
{
	auto bullets = custom_data->scenetree->findEntitiesByType(hash("Bullet"));
	auto asteroids = custom_data->scenetree->findEntitiesByType(hash("Asteroid"));
	auto player1 = custom_data->scenetree->findEntityByName<Player>("Player 1");
	auto player2 = custom_data->scenetree->findEntityByName<Player>("Player 2");
	auto player3 = custom_data->scenetree->findEntityByName<Player>("Player 3");
	auto player4 = custom_data->scenetree->findEntityByName<Player>("Player 4");

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
		if (!asteroid->isAlive())
			continue;
		
		asteroidCS.setRadius(asteroid->getRadius());
		asteroidCS.setOrigin(asteroid->getRadius() / 2.0f, asteroid->getRadius() / 2.0f);
		asteroidCS.setPosition(asteroid->getPosition());

		sf::CircleShape bulletCS(5);
		bulletCS.setOrigin(2.5f, 2.5f);
		for (auto& b : bullets)
		{
			auto bullet = static_cast<Bullet*>(b);
			if (!bullet->isAlive())
				continue;
			
			bulletCS.setPosition(bullet->getPosition());

			if (circlesColliding(asteroidCS, bulletCS))
			{
				custom_data->scenetree->sendMessage(asteroid->info.ID, std::make_unique<enki::MessageID<hash_constexpr("Collision")>>());
				custom_data->scenetree->sendMessage(bullet->info.ID, std::make_unique<enki::MessageID<hash_constexpr("Collision")>>());
			}
		}

		sf::CircleShape playerCS(16);
		playerCS.setOrigin(8.0f, 8.0f);

		const auto playerCheck = [this, asteroid, &asteroidCS, &playerCS, &circlesColliding](Player* player)
		{
			if (player)
			{
				playerCS.setPosition(player->getPosition());
				if (circlesColliding(asteroidCS, playerCS))
				{
					if (!player->isInvincible())
					{
						custom_data->scenetree->sendMessage(asteroid->info.ID, std::make_unique<enki::MessageID<hash_constexpr("Collision")>>());
					}
					custom_data->scenetree->sendMessage(player->info.ID, std::make_unique<enki::MessageID<hash_constexpr("Collision")>>());
				}
			}
		};

		playerCheck(player1);
		playerCheck(player2);
		playerCheck(player3);
		playerCheck(player4);
	}
}

void CollisionManager::draw(enki::Renderer* renderer)
{

}

void CollisionManager::serializeOnTick(enki::Packet& p)
{

}

void CollisionManager::deserializeOnTick(enki::Packet& p)
{

}