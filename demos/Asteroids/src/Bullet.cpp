#include "Bullet.hpp"

//STD
#include <experimental/vector>

//LIBS
#include <Enki/Scenetree.hpp>

#include "Enki/Window/WindowSFML.hpp"

Bullet::Bullet(enki::EntityInfo info, CustomData* custom_data)
	: Entity(info)
	, custom_data(custom_data)
	, window(custom_data->window->as<enki::WindowSFML>()->getRawWindow())
	, bullet_tex(custom_data->texture_manager->getTextureOrRegister("resources/bullet.png"))
{
	network_tick_rate = 1;
}

std::unique_ptr<enki::Entity> Bullet::clone()
{
	return std::make_unique<Bullet>(*this);
}

void Bullet::onSpawn([[maybe_unused]] enki::Packet p)
{
	auto console = spdlog::get("console");
	bullet.setTexture(*bullet_tex);
	bullet.setOrigin(
		bullet_tex->getSize().x / 2,
		bullet_tex->getSize().y / 2);

	if (p.canDeserialize<float, float, float, float, sf::Uint8, sf::Uint8, sf::Uint8>())
	{
		float x = p.read<float>();
		float y = p.read<float>();

		speed = p.read<float>();
		float rot = p.read<float>();

		sf::Uint8 r = p.read<sf::Uint8>();
		sf::Uint8 g = p.read<sf::Uint8>();
		sf::Uint8 b = p.read<sf::Uint8>();

		sf::Color c(r, g, b);
		bullet.setColor(c);
		bullet.setPosition(x, y);
		bullet.setRotation(rot);

		const auto degToVector = [](float angle) -> sf::Vector2f
		{
			return sf::Vector2f(
				std::sin(angle * (3.1415 / 180.0f)),
				-std::cos(angle * 3.1415 / 180.0f));
		};

		velocity.x *= degToVector(rot).x * speed;
		velocity.y *= degToVector(rot).y * speed;
	}
	else
	{
		console->error("Couldn't deserialize info for bullet");
	}
}

void Bullet::update(float dt)
{
	if (!isOwner(custom_data->network_manager))
	{
		return;
	}

	auto input_manager = custom_data->input_manager;

	bullet.move(velocity.x * dt, velocity.y * dt);

	if (bullet.getPosition().x + (bullet_tex->getSize().x / 2) <= 0)
	{
		warp_count++;
		bullet.setPosition(window->getView().getSize().x, bullet.getPosition().y);
	}
	else if (bullet.getPosition().x - (bullet_tex->getSize().x / 2) >= window->getView().getSize().x)
	{
		warp_count++;
		bullet.setPosition(0, bullet.getPosition().y);
	}
	else if (bullet.getPosition().y + (bullet_tex->getSize().y / 2) <= 0)
	{
		warp_count++;
		bullet.setPosition(bullet.getPosition().x, window->getView().getSize().y);
	}
	else if (bullet.getPosition().y - (bullet_tex->getSize().y / 2) >= window->getView().getSize().y)
	{
		warp_count++;
		bullet.setPosition(bullet.getPosition().x, 0);
	}

	if (warp_count >= 2)
	{
		alive = false;
	}

	if (!alive)
	{
		custom_data->scenetree->deleteEntity(info.ID);
	}
}

void Bullet::draw(enki::Renderer* renderer)
{
	renderer->draw(&bullet);
}

void Bullet::receive(enki::Message* msg)
{
	if (msg->id == hash_constexpr("Collision"))
	{
		handleCollision();
	}
}

void Bullet::serializeOnConnection(enki::Packet& p)
{
	serializeOnTick(p);
	p << bullet.getColor().r << bullet.getColor().g << bullet.getColor().b;
}

void Bullet::deserializeOnConnection(enki::Packet& p)
{
	deserializeOnTick(p);
	sf::Color c;
	p >> c.r >> c.g >> c.b;
	bullet.setColor(c);
}

void Bullet::serializeOnTick(enki::Packet& p)
{
	p.writeCompressedFloat(bullet.getPosition().x, 0, 1280, 0.01f);
	p.writeCompressedFloat(bullet.getPosition().y, 0, 720, 0.01f);
	p.writeCompressedFloat(bullet.getRotation(), 0, 360, 0.01f);
}

void Bullet::deserializeOnTick(enki::Packet& p)
{
	float x = p.readCompressedFloat(0, 1280, 0.01f);
	float y = p.readCompressedFloat(0, 720, 0.01f);
	bullet.setPosition(x, y);
	bullet.setRotation(p.readCompressedFloat(0, 360, 0.01f));
}

bool Bullet::isAlive() const
{
	return alive;
}

sf::Vector2f Bullet::getPosition() const
{
	return bullet.getPosition();
}

float Bullet::getRotation() const
{
	return bullet.getRotation();
}

sf::Color Bullet::getColour() const
{
	return bullet.getColor();
}

unsigned int Bullet::getWarpCount() const
{
	return warp_count;
}

void Bullet::handleCollision()
{
	if (!isOwner(custom_data->network_manager))
	{
		return;
	}

	if (alive)
	{
		alive = false;
		custom_data->scenetree->deleteEntity(info.ID);
	}
}