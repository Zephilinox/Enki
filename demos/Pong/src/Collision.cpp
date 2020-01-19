#include "Collision.hpp"

//LIBS
#include <spdlog/fmt/fmt.h>
#include <Enki/Scenetree.hpp>

//SELF
#include "Ball.hpp"
#include "Paddle.hpp"

Collision::Collision(enki::EntityInfo info, enki::GameData* game_data)
	: Entity(info, game_data)
{
}

void Collision::onSpawn([[maybe_unused]] enki::Packet p)
{
	ball_collider.height = 32;
	ball_collider.width = 32;

	paddle1_collider.height = 128;
	paddle1_collider.width = 32;

	paddle2_collider.height = 128;
	paddle2_collider.width = 32;
}

void Collision::update([[maybe_unused]]float dt)
{
	auto console = spdlog::get("console");
	auto scenetree = game_data->scenetree;

	Ball* ball = scenetree->findEntityByType<Ball>(hash("Ball"));
	Paddle* paddle1 = scenetree->findEntityByName<Paddle>("Paddle 1");
	Paddle* paddle2 = scenetree->findEntityByName<Paddle>("Paddle 2");

	if (ball && paddle1 && paddle2)
	{
		ball_collider.left = ball->sprite.getPosition().x;
		ball_collider.top = ball->sprite.getPosition().y;
		
		bool intersects = false;

		paddle1_collider.left = paddle1->sprite.getPosition().x;
		paddle1_collider.top = paddle1->sprite.getPosition().y;

		if (ball_collider.intersects(paddle1_collider))
		{
			ball->moving_left = false;
			intersects = true;
		}

		paddle2_collider.left = paddle2->sprite.getPosition().x;
		paddle2_collider.top = paddle2->sprite.getPosition().y;

		if (ball_collider.intersects(paddle2_collider))
		{
			ball->moving_left = true;
			intersects = true;
		}

		if (intersects)
		{
			if (ball->y_dir == 0)
			{
				if (std::rand() % 2)
				{
					ball->y_dir = 1;
				}
				else
				{
					ball->y_dir = -1;
				}
			}
		}
	}	
}