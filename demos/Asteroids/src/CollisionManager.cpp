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
	struct Circle
	{
		sf::Vector2f pos;
		const float radius = 0;
	};

	static std::vector<Entity*> bullets;
	static std::vector<Entity*> asteroids;
	static std::vector<Entity*> players;
	bullets.clear();
	asteroids.clear();
	players.clear();
	
	custom_data->scenetree->fillEntitiesByType(hash("Bullet"), bullets);
	custom_data->scenetree->fillEntitiesByType(hash("Asteroid"), asteroids);
	custom_data->scenetree->fillEntitiesByType(hash("Player"), players);

	const auto circlesColliding = [](Circle shape_one, Circle shape_two) -> bool
	{
		const float dist_x = shape_one.pos.x - shape_two.pos.x;
		const float dist_y = shape_one.pos.y - shape_two.pos.y;
		const float length_squared = (dist_x * dist_x) + (dist_y * dist_y);
		return length_squared < (shape_one.radius + shape_two.radius) * (shape_one.radius + shape_two.radius);
	};

	for (auto a : asteroids)
	{
		const auto asteroid = static_cast<Asteroid*>(a);
		const Circle asteroidCS{asteroid->getPosition(), asteroid->getRadius()};
		
		for (auto b : bullets)
		{
			const auto bullet = static_cast<Bullet*>(b);
			static Circle bulletCS{{}, 5};
			bulletCS.pos = bullet->getPosition();

			if (circlesColliding(asteroidCS, bulletCS))
			{
				custom_data->scenetree->sendMessage(asteroid->info.ID, std::make_unique<enki::MessageID<hash_constexpr("Collision")>>());
				custom_data->scenetree->sendMessage(bullet->info.ID, std::make_unique<enki::MessageID<hash_constexpr("Collision")>>());
			}
		}

		for (auto p : players)
		{
			const auto ply = static_cast<Player*>(p);
			static Circle playerCS{{}, 16};
			playerCS.pos = ply->getPosition();
			
			if (circlesColliding(asteroidCS, playerCS))
			{
				if (!ply->isInvincible())
				{
					custom_data->scenetree->sendMessage(asteroid->info.ID, std::make_unique<enki::MessageID<hash_constexpr("Collision")>>());
				}
				custom_data->scenetree->sendMessage(ply->info.ID, std::make_unique<enki::MessageID<hash_constexpr("Collision")>>());
			}
		}
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