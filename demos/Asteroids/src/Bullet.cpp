#include "Bullet.hpp"

//STD
#include <experimental/vector>

//LIBS
#include <Enki/Scenetree.hpp>

//SELF
#include <Asteroid.hpp>

Bullet::Bullet(enki::EntityInfo info, CustomData* custom_data)
	: Entity(info)
	, custom_data(custom_data)
{
	network_tick_rate = 1;
}


sf::Vector2f degToVector(float angle)
{
	return sf::Vector2f(
		std::sin(angle * (3.1415 / 180.0f)),
		-std::cos(angle * 3.1415 / 180.0f));
};

void Bullet::onSpawn([[maybe_unused]]enki::Packet p)
{
	auto console = spdlog::get("console");
	bullet_tex.loadFromFile("resources/bullet.png");
	bullet.setTexture(bullet_tex);
	bullet.setOrigin(
		bullet_tex.getSize().x / 2,
		bullet_tex.getSize().y / 2);

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
	int wrapped = std::abs(b - a) % 360;
	int distance = wrapped > 180 ? 360 - wrapped : wrapped;
	int sign = (a - b >= 0 && a - b <= 180) || (a - b <= -180 && a - b >= -360) ? 1 : -1;
	distance *= sign;
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
	
	if (/*!closest_asteroid || */find_asteroid_timer.getElapsedTime() > find_asteroid_delay)
	{
		find_asteroid_timer.restart();
		
		float shortest_length = 0;
		auto asteroids = custom_data->scenetree->findEntitiesByType<Asteroid>(hash("Asteroid"));
		for (auto asteroid : asteroids)
		{
			sf::Vector2f dist{asteroid->getPosition().x - bullet.getPosition().x, asteroid->getPosition().y - bullet.getPosition().y};
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
		distance_to_asteroid = sf::Vector2f{closest_asteroid->getPosition().x - bullet.getPosition().x, closest_asteroid->getPosition().y - bullet.getPosition().y};
		float angle = std::atan2(distance_to_asteroid.x, distance_to_asteroid.y * -1.0f);
		float angle_degrees = angle * (180.0f / 3.1415f);
		float angle_diff = angle_distance(angle_degrees, bullet.getRotation());
		float max_distance = 100000; //1000 pixels, squared
		float min_distance = 10000; //100 pixels, squared
		float distance = (distance_to_asteroid.x * distance_to_asteroid.x) + (distance_to_asteroid.y * distance_to_asteroid.y);
		float speed = std::max(1.0f, std::min(max_distance / distance, (max_distance / min_distance) * 0.1f));
		angle_diff += (((std::rand() % 1000) / 100.0f) - 1.0f) * 10.0f;
		bullet.setRotation(bullet.getRotation() + (angle_diff * speed * dt));
	}
	
	velocity.x = 1 * degToVector(bullet.getRotation()).x * speed;
	velocity.y = 1 * degToVector(bullet.getRotation()).y * speed;
	bullet.move(velocity.x * dt, velocity.y * dt);

	if (bullet.getPosition().x + (bullet_tex.getSize().x / 2) <= 0)
	{
		warp_count++;
		bullet.setPosition(custom_data->window_sfml->getView().getSize().x, bullet.getPosition().y);
	}
	else if (bullet.getPosition().x - (bullet_tex.getSize().x / 2) >= custom_data->window_sfml->getView().getSize().x)
	{
		warp_count++;
		bullet.setPosition(0, bullet.getPosition().y);
	}
	else if (bullet.getPosition().y + (bullet_tex.getSize().y / 2) <= 0)
	{
		warp_count++;
		bullet.setPosition(bullet.getPosition().x, custom_data->window_sfml->getView().getSize().y);
	}
	else if (bullet.getPosition().y - (bullet_tex.getSize().y / 2) >= custom_data->window_sfml->getView().getSize().y)
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