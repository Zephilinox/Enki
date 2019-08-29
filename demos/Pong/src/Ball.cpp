#include "Ball.hpp"

//SELF
#include <Enki/Scenegraph.hpp>
#include "Score.hpp"

Ball::Ball(enki::EntityInfo info, enki::GameData* game_data)
	: Entity(info, game_data)
{
	network_tick_rate = 1;
}

void Ball::onSpawn([[maybe_unused]]enki::Packet& p)
{
	texture.loadFromFile("resources/Ball.png");
	sprite.setTexture(texture);
	sprite.setPosition(320, 180);
}

void Ball::update(float dt)
{
	if (game_data->scenegraph->findEntitiesByType(hash("Paddle")).size() != 2 ||
		!isOwner())
	{
		return;
	}

	if (moving_left)
	{
		sprite.move(-300 * dt, 0);
		if (sprite.getPosition().x < -64)
		{
			sprite.setPosition(320 - 16, 180 - 16);
			y_speed = float(std::rand() % 300 + 100);

			Score* score = game_data->scenegraph->findEntityByType<Score>(hash("Score"));
			if (score)
			{
				game_data->scenegraph->rpc_man.callEntityRPC(&Score::increaseScore2, "increaseScore2", score);
			}

			moving_left = false;
		}
	}
	else
	{
		sprite.move(300 * dt, 0);
		if (sprite.getPosition().x > 640 + 32)
		{
			sprite.setPosition(320 - 16, 180 - 16);
			y_speed = float(std::rand() % 300 + 100);

			Score* score = game_data->scenegraph->findEntityByType<Score>(hash("Score"));
			if (score)
			{
				game_data->scenegraph->rpc_man.callEntityRPC(&Score::increaseScore1, "increaseScore1", score);
			}

			moving_left = true;
		}
	}

	if (y_dir == 1)
	{
		sprite.move(0, -y_speed * dt);
		if (sprite.getPosition().y < 0)
		{
			y_dir = -1;
			y_speed = float(std::rand() % 300 + 100);
		}
	}
	else if (y_dir == -1)
	{
		sprite.move(0, y_speed * dt);
		if (sprite.getPosition().y > 360 - 32)
		{
			y_dir = 1;
			y_speed = float(std::rand() % 300 + 100);
		}
	}
}

void Ball::draw(enki::Renderer* renderer)
{
	renderer->draw({ &sprite , 0, 0});
}

void Ball::serializeOnConnection([[maybe_unused]]enki::Packet& p)
{
	sprite.setPosition(320, 180);
}

void Ball::deserializeOnConnection([[maybe_unused]]enki::Packet& p)
{
	sprite.setPosition(320, 180);
}

void Ball::serializeOnTick(enki::Packet& p)
{
	p << sprite.getPosition().x << sprite.getPosition().y;
}

void Ball::deserializeOnTick(enki::Packet& p)
{
	float x = p.read<float>();
	float y = p.read<float>();
	sprite.setPosition(x, y);
}
