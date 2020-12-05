#include "Bullet.hpp"

#undef max
#undef min

//STD
#include <experimental/vector>

//LIBS
#include <Enki/Scenetree.hpp>
#include <Enki/Renderer/RendererSFML.hpp>
#include <Enki/Graphics/Sprite.hpp>

//SELF
#include <Asteroid.hpp>

Bullet::Bullet(enki::EntityInfo info, CustomData* custom_data)
	: Entity(info)
	, custom_data(custom_data)
	, bullet(custom_data->renderer->createSprite())
{
	custom_data->texture_manager->registerTexture(custom_data->renderer, "resources/bullet.png");
	bullet_tex = custom_data->texture_manager->getTexture("resources/bullet.png");
	bullet->setTexture(bullet_tex);
	network_tick_rate = 1;
}

sf::Vector2f degToVector(float angle)
{
	const float radians = angle * (3.1415f / 180.0f);
	return sf::Vector2f(std::sin(radians), -1 * std::cos(radians));
};

void Bullet::onSpawn([[maybe_unused]]enki::Packet p)
{
	auto console = spdlog::get("console");

	if (p.canDeserialize<float, float, float, float, sf::Uint8, sf::Uint8, sf::Uint8>())
	{
		const auto x = p.read<float>();
		const auto y = p.read<float>();

		speed = p.read<float>();
		const auto rot = p.read<float>();

		const auto r = p.read<std::uint8_t>();
		const auto g = p.read<std::uint8_t>();
		const auto b = p.read<std::uint8_t>();
		const auto a = p.read<std::uint8_t>();

		const enki::Colour c{r, g, b, a};
		bullet->setColour(c);
		bullet->setPosition(x, y);
		bullet->setRotation(rot);

		velocity.x = 1 * degToVector(rot).x * speed;
		velocity.y = 1 * degToVector(rot).y * speed;

		find_asteroid_delay = 1.0f + (((std::rand() % 100) / 100.0f) - 0.5f);
	}
	else
	{
		console->error("Couldn't deserialize info for bullet");
	}
}

int angle_distance(int a, int b)
{
	const int wrapped = std::abs(b - a) % 360;
	const int sign = (a - b >= 0 && a - b <= 180) || (a - b <= -180 && a - b >= -360) ? 1 : -1;
	
	const int distance = (wrapped > 180 ? 360 - wrapped : wrapped) * sign;
	return distance;
}

void Bullet::update(float dt)
{
	if (!isOwner(custom_data->network_manager))
	{
		return;
	}

	auto input_manager = custom_data->input_manager;

	Asteroid* closest_asteroid = static_cast<Asteroid*>(custom_data->scenetree->findEntity(closest_asteroid_id));
	if (!closest_asteroid)
		closest_asteroid_id = 0;
	
	if (/*!closest_asteroid || */find_asteroid_timer.getElapsedTime() > find_asteroid_delay)
	{
		find_asteroid_timer.restart();
		
		float shortest_length = 0;
		auto asteroids = custom_data->scenetree->findEntitiesByType<Asteroid>(hash("Asteroid"));
		for (auto asteroid : asteroids)
		{
			enki::Vector2 dist{
				asteroid->getPosition().x - bullet->getPosition().x,
				asteroid->getPosition().y - bullet->getPosition().y};
			
			float our_length = (dist.x * dist.x) + (dist.y * dist.y);
			if (!closest_asteroid || our_length < shortest_length)
			{
				distance_to_asteroid = dist;
				closest_asteroid_id = asteroid->info.ID;
				closest_asteroid = asteroid;
				shortest_length = our_length;
			}
		}
	}

	if (closest_asteroid)
	{
		distance_to_asteroid = enki::Vector2{
			closest_asteroid->getPosition().x - bullet->getPosition().x,
			closest_asteroid->getPosition().y - bullet->getPosition().y};
		
		float angle = std::atan2(distance_to_asteroid.x, distance_to_asteroid.y * -1.0f);
		float angle_degrees = angle * (180.0f / 3.1415f);
		float angle_diff = angle_distance(angle_degrees, bullet->getRotation());
		float max_distance = 100000; //1000 pixels, squared
		float min_distance = 10000; //100 pixels, squared
		float distance = (distance_to_asteroid.x * distance_to_asteroid.x) + (distance_to_asteroid.y * distance_to_asteroid.y);
		float speed = std::max(1.0f, std::min(max_distance / distance, (max_distance / min_distance) * 0.1f));
		angle_diff += (((std::rand() % 1000) / 100.0f) - 1.0f) * 10.0f;
		bullet->setRotation(bullet->getRotation() + (angle_diff * speed * dt));
	}
	
	velocity.x = 1 * degToVector(bullet->getRotation()).x * speed;
	velocity.y = 1 * degToVector(bullet->getRotation()).y * speed;
	bullet->setPosition(
		bullet->getPosition().x + velocity.x * dt, 
		bullet->getPosition().y + velocity.y * dt);

	if (bullet->getPosition().x + (bullet_tex->getWidth() / 2) <= 0)
	{
		warp_count++;
		bullet->setPosition(custom_data->window->getWidth(), bullet->getPosition().y);
	}
	else if (bullet->getPosition().x - (bullet_tex->getWidth() / 2) >= custom_data->window->getWidth())
	{
		warp_count++;
		bullet->setPosition(0, bullet->getPosition().y);
	}
	else if (bullet->getPosition().y + (bullet_tex->getHeight() / 2) <= 0)
	{
		warp_count++;
		bullet->setPosition(bullet->getPosition().x, custom_data->window->getHeight());
	}
	else if (bullet->getPosition().y - (bullet_tex->getHeight() / 2) >= custom_data->window->getHeight())
	{
		warp_count++;
		bullet->setPosition(bullet->getPosition().x, 0);
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
	renderer->queue(bullet.get());
}

void Bullet::serializeOnConnection(enki::Packet& p)
{
	serializeOnTick(p);
	p << bullet->getColour().r << bullet->getColour().g << bullet->getColour().b << bullet->getColour().a;
}

void Bullet::deserializeOnConnection(enki::Packet& p)
{
	deserializeOnTick(p);
	enki::Colour c;
	p >> c.r >> c.g >> c.b >> c.a;
	bullet->setColour(c);
}

void Bullet::serializeOnTick(enki::Packet& p)
{
	p.writeCompressedFloat(bullet->getPosition().x, 0, 1280, 0.01f);
	p.writeCompressedFloat(bullet->getPosition().y, 0, 720, 0.01f);
	p.writeCompressedFloat(bullet->getRotation(), 0, 360, 0.01f);
}

void Bullet::deserializeOnTick(enki::Packet& p)
{
	float x = p.readCompressedFloat(0, 1280, 0.01f);
	float y = p.readCompressedFloat(0, 720, 0.01f);
	bullet->setPosition(x, y);
	bullet->setRotation(p.readCompressedFloat(0, 360, 0.01f));
}

std::vector<std::pair<std::string, std::string>> Bullet::serializeToStrings() const
{
	return {
		{"Position", fmt::format("{{{}, {}}}", bullet->getPosition().x, bullet->getPosition().y)},
		{"Velocity", fmt::format("{{{}, {}}}", velocity.x, velocity.y)},
		{"Rotation", std::to_string(bullet->getRotation())},
		{"Speed", std::to_string(speed)},
		{"Alive", alive ? "true" : "false"},
		{"Warp Count", std::to_string(warp_count)},
		{"Find Asteroid Timer", std::to_string(find_asteroid_timer.getElapsedTime())},
		{"Find Asteroid Delay", std::to_string(find_asteroid_delay)},
		{"Closest Asteroid", enki::prettyID(closest_asteroid_id)},
		{"Closest Asteroid Distance", fmt::format("{{{}, {}}}", distance_to_asteroid.x, distance_to_asteroid.y)},
	};
}

bool Bullet::isAlive() const
{
	return alive;
}

enki::Vector2 Bullet::getPosition() const
{
	return bullet->getPosition();
}

float Bullet::getRotation() const
{
	return bullet->getRotation();
}

enki::Colour Bullet::getColour() const
{
	return bullet->getColour();
}

unsigned int Bullet::getWarpCount() const
{
	return warp_count;
}

void Bullet::handleCollision()
{
	if (!custom_data->network_manager->isServer())
	{
		return;
	}

	if (alive)
	{
		alive = false;
		custom_data->scenetree->deleteEntity(info.ID);
	}
}