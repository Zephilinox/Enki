#include "Paddle.hpp"

//LIBS
#include <spdlog/fmt/fmt.h>
#include <Enki/Scenetree.hpp>

Paddle::Paddle(enki::EntityInfo info, enki::GameData* game_data)
	: enki::Entity(info, game_data)
{
	network_tick_rate = 1;
}

void Paddle::onSpawn([[maybe_unused]]enki::Packet p)
{
	texture.loadFromFile("resources/Paddle.png");
	sprite.setTexture(texture);

	if (info.name == "Paddle 1")
	{
		sprite.setPosition(32, 180);
		sprite.setColor(sf::Color(0, 150, 255));
	}
	else
	{
		sprite.setPosition(640 - 64, 180);
		sprite.setColor(sf::Color(220, 25, 25));
	}

	latest_sprite = sprite;
	latest_sprite.setColor(sf::Color(50, 50, 50, 100));
}

void Paddle::input(sf::Event& e)
{
	if (e.type == sf::Event::KeyPressed)
	{
		if (!isOwner())
		{
			if (e.key.code == sf::Keyboard::F2)
			{
				if (interpolation_enabled)
				{
					sprite.setPosition(latest_sprite.getPosition());
				}
				else
				{
					latest_sprite.setPosition(sprite.getPosition());
				}

				interpolation_enabled = !interpolation_enabled;
				latest_network_y = sprite.getPosition().y;
				last_interpolation_y = sprite.getPosition().y;
			}
		}

		if (e.key.code == sf::Keyboard::Num3)
		{
			game_data->scenetree->rpc_man.callEntityRPC(
				&Paddle::setColour, "setColour", this, std::rand() % 255, std::rand() % 255, std::rand() % 255);
		}
	}
}

void Paddle::update(float dt)
{
	if (info.name == "Paddle 1" && isOwner())
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			sprite.move(0, -200.0f * dt);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			sprite.move(0, 200.0f * dt);
		}
	}
	else if (info.name == "Paddle 2" && isOwner())
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			sprite.move(0, -200.0f * dt);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			sprite.move(0, 200.0f * dt);
		}
	}

	if (isOwner())
	{
		if (sprite.getPosition().y < 0)
		{
			sprite.setPosition(sprite.getPosition().x, 0);
		}

		if (sprite.getPosition().y > 360 - 128)
		{
			sprite.setPosition(sprite.getPosition().x, 360 - 128);
		}
	}
	else if (interpolation_enabled)
	{
		if (std::abs(latest_network_y - last_interpolation_y) > 64)
		{
			sprite.setPosition(sprite.getPosition().x, latest_network_y);
			last_interpolation_y = latest_network_y;
		}
		else
		{
			float network_delay = 1.0f / (game_data->network_manager->network_send_rate / network_tick_rate);
			float interpolation_time = interpolation_timer.getElapsedTime();
			float interpolation_percent = float(1.0 - ((network_delay - interpolation_time) / network_delay));

			float interpolated_y = last_interpolation_y + interpolation_percent * (latest_network_y - last_interpolation_y);
			sprite.setPosition(sprite.getPosition().x, interpolated_y);
		}
	}
}

void Paddle::draw(enki::Renderer* renderer)
{
	renderer->draw({ &sprite, 0, 0 });

	if (!isOwner() && interpolation_enabled)
	{
		renderer->draw({ &latest_sprite, 0, 0});
	}
}

void Paddle::serializeOnConnection(enki::Packet& p)
{
	p << sprite.getColor().r
		<< sprite.getColor().g 
		<< sprite.getColor().b;
	serializeOnTick(p);
}

void Paddle::deserializeOnConnection(enki::Packet& p)
{
	sf::Color c;
	p >> c.r >> c.g >> c.b;
	c.a = 255;
	sprite.setColor(c);

	deserializeOnTick(p);
}

void Paddle::serializeOnTick(enki::Packet& p)
{
	p << sprite.getPosition().x << sprite.getPosition().y;
}

void Paddle::deserializeOnTick(enki::Packet& p)
{
	float x = p.read<float>();

	last_interpolation_y = sprite.getPosition().y;
	p >> latest_network_y;

	if (interpolation_enabled)
	{
		latest_sprite.setPosition(x, latest_network_y);
		interpolation_timer.restart();
	}
	else
	{
		sprite.setPosition(x, latest_network_y);
	}
}

void Paddle::setColour(int r, int g, int b)
{
	sprite.setColor(sf::Color(uint8_t(r), uint8_t(g), uint8_t(b), uint8_t(255)));
}
