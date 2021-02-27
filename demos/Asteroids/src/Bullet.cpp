#include "Bullet.hpp"

//SELF
#include <Asteroid.hpp>

//LIBS
#include <Enki/Scenetree.hpp>
#include <Enki/Renderer/RendererSFML.hpp>
#include <Enki/Graphics/Sprite.hpp>

//STD
#include <experimental/vector>

Bullet::Bullet(enki::EntityInfo info, CustomData* custom_data)
	: Entity(std::move(info))
	, m_custom_data(custom_data)
	, m_bullet(custom_data->renderer->createSprite())
{
	custom_data->texture_manager->registerTexture(custom_data->renderer, "resources/bullet.png");
	m_bullet_tex = custom_data->texture_manager->getTexture("resources/bullet.png");
	m_bullet->setTexture(m_bullet_tex);
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

		m_speed = p.read<float>();
		const auto rot = p.read<float>();

		const auto r = p.read<std::uint8_t>();
		const auto g = p.read<std::uint8_t>();
		const auto b = p.read<std::uint8_t>();
		const auto a = p.read<std::uint8_t>();

		const enki::Colour c{r, g, b, a};
		m_bullet->setColour(c);
		m_bullet->setPosition(x, y);
		m_bullet->setRotation(rot);

		m_velocity.x = 1 * degToVector(rot).x * m_speed;
		m_velocity.y = 1 * degToVector(rot).y * m_speed;

		m_find_asteroid_delay = 1.0f + (((std::rand() % 100) / 100.0f) - 0.5f);
	}
	else
	{
		console->error("Couldn't deserialize info for bullet");
	}
}

float angle_distance(const float a, const float b)
{
	const float wrapped = std::fmod(std::abs(b - a), 360.0f);
	const float sign = (a - b >= 0.0f && a - b <= 180.0f) || (a - b <= -180.0f && a - b >= -360.0f) ? 1.0f : -1.0f;
	
	const float distance = (wrapped > 180.0f ? 360.0f - wrapped : wrapped) * sign;
	return distance;
}

void Bullet::update(const float dt)
{
	if (!isOwner(m_custom_data->network_manager))
		return;

	auto* closest_asteroid = static_cast<Asteroid*>(m_custom_data->scenetree->findEntity(m_closest_asteroid_id));
	if (!closest_asteroid)
		m_closest_asteroid_id = 0;
	
	if (/*!closest_asteroid || */m_find_asteroid_timer.getElapsedTime() > m_find_asteroid_delay)
	{
		m_find_asteroid_timer.restart();
		
		float shortest_length = 0;
		auto asteroids = m_custom_data->scenetree->findEntitiesByType<Asteroid>(hash("Asteroid"));
		for (auto* asteroid : asteroids)
		{
			const enki::Vector2 dist{
				asteroid->getPosition().x - m_bullet->getPosition().x,
				asteroid->getPosition().y - m_bullet->getPosition().y};
			
			const float our_length = (dist.x * dist.x) + (dist.y * dist.y);
			if (!closest_asteroid || our_length < shortest_length)
			{
				m_distance_to_asteroid = dist;
				m_closest_asteroid_id = asteroid->info.ID;
				closest_asteroid = asteroid;
				shortest_length = our_length;
			}
		}
	}

	if (closest_asteroid)
	{
		m_distance_to_asteroid = enki::Vector2{
			closest_asteroid->getPosition().x - m_bullet->getPosition().x,
			closest_asteroid->getPosition().y - m_bullet->getPosition().y};

		 debug_line = {
			{
				closest_asteroid->getPosition(),
				m_bullet->getPosition(),
			},
			enki::Colour{255, 0, 0, 100}
		};
		
		const float angle = std::atan2(m_distance_to_asteroid.x, m_distance_to_asteroid.y * -1.0f);
		const float angle_degrees = angle * (180.0f / 3.1415f);
		const float angle_diff = angle_distance(angle_degrees, m_bullet->getRotation());
		const float max_distance = 128*128;
		const float min_distance = 64*64;
		const float distance = (m_distance_to_asteroid.x * m_distance_to_asteroid.x) + (m_distance_to_asteroid.y * m_distance_to_asteroid.y) + 1 /* don't divide by 0*/;
		const float rotation_speed_at_min_distance = 4;
		const float rotation_speed_at_max_distance = 8;
		const float current_distance_percentage = distance / (max_distance - min_distance);
		const float rotation_speed_at_current_distance = rotation_speed_at_max_distance * current_distance_percentage;
		const float rotation_speed = std::clamp(rotation_speed_at_current_distance, rotation_speed_at_min_distance, rotation_speed_at_max_distance);
		m_bullet->setRotation(m_bullet->getRotation() + (angle_diff * rotation_speed * dt));
	}

	if (!closest_asteroid)
		debug_line = {};
	
	m_velocity.x = 1 * degToVector(m_bullet->getRotation()).x * m_speed;
	m_velocity.y = 1 * degToVector(m_bullet->getRotation()).y * m_speed;
	m_bullet->setPosition(
		m_bullet->getPosition().x + m_velocity.x * dt, 
		m_bullet->getPosition().y + m_velocity.y * dt);

	if (m_bullet->getPosition().x + static_cast<float>(m_bullet_tex->getWidth() / 2) <= 0)
	{
		m_warp_count++;
		m_bullet->setPosition(static_cast<float>(m_custom_data->window->getWidth()), m_bullet->getPosition().y);
	}
	else if (m_bullet->getPosition().x - static_cast<float>(m_bullet_tex->getWidth() / 2) >= static_cast<float>(m_custom_data->window->getWidth()))
	{
		m_warp_count++;
		m_bullet->setPosition(0, m_bullet->getPosition().y);
	}
	else if (m_bullet->getPosition().y + static_cast<float>(m_bullet_tex->getHeight() / 2) <= 0)
	{
		m_warp_count++;
		m_bullet->setPosition(m_bullet->getPosition().x, static_cast<float>(m_custom_data->window->getHeight()));
	}
	else if (m_bullet->getPosition().y - static_cast<float>(m_bullet_tex->getHeight() / 2) >= static_cast<float>(m_custom_data->window->getHeight()))
	{
		m_warp_count++;
		m_bullet->setPosition(m_bullet->getPosition().x, 0);
	}

	if (m_warp_count >= 2)
		m_alive = false;

	if (!m_alive)
		m_custom_data->scenetree->deleteEntity(info.ID);
}

void Bullet::draw(enki::Renderer* renderer)
{
	renderer->queue(m_bullet.get());
	if (debug_line)
		m_custom_data->renderer->queue(debug_line.value());
}

void Bullet::serializeOnConnection(enki::Packet& p)
{
	serializeOnTick(p);
	p << m_bullet->getColour().r << m_bullet->getColour().g << m_bullet->getColour().b << m_bullet->getColour().a;
}

void Bullet::deserializeOnConnection(enki::Packet& p)
{
	deserializeOnTick(p);
	enki::Colour c;
	p >> c.r >> c.g >> c.b >> c.a;
	m_bullet->setColour(c);
}

void Bullet::serializeOnTick(enki::Packet& p)
{
	p.writeCompressedFloat(m_bullet->getPosition().x, 0, 1280, 0.01f);
	p.writeCompressedFloat(m_bullet->getPosition().y, 0, 720, 0.01f);
	p.writeCompressedFloat(m_bullet->getRotation(), 0, 360, 0.01f);
}

void Bullet::deserializeOnTick(enki::Packet& p)
{
	float x = p.readCompressedFloat(0, 1280, 0.01f);
	float y = p.readCompressedFloat(0, 720, 0.01f);
	m_bullet->setPosition(x, y);
	m_bullet->setRotation(p.readCompressedFloat(0, 360, 0.01f));
}

std::vector<std::pair<std::string, std::string>> Bullet::serializeToStrings() const
{
	return {
		{"Position", fmt::format("{{{}, {}}}", m_bullet->getPosition().x, m_bullet->getPosition().y)},
		{"Velocity", fmt::format("{{{}, {}}}", m_velocity.x, m_velocity.y)},
		{"Rotation", std::to_string(m_bullet->getRotation())},
		{"Speed", std::to_string(m_speed)},
		{"Alive", m_alive ? "true" : "false"},
		{"Warp Count", std::to_string(m_warp_count)},
		{"Find Asteroid Timer", std::to_string(m_find_asteroid_timer.getElapsedTime())},
		{"Find Asteroid Delay", std::to_string(m_find_asteroid_delay)},
		{"Closest Asteroid", enki::prettyID(m_closest_asteroid_id)},
		{"Closest Asteroid Distance", fmt::format("{{{}, {}}}", m_distance_to_asteroid.x, m_distance_to_asteroid.y)},
	};
}

bool Bullet::isAlive() const
{
	return m_alive;
}

enki::Vector2 Bullet::getPosition() const
{
	return m_bullet->getPosition();
}

float Bullet::getRotation() const
{
	return m_bullet->getRotation();
}

enki::Colour Bullet::getColour() const
{
	return m_bullet->getColour();
}

unsigned int Bullet::getWarpCount() const
{
	return m_warp_count;
}

void Bullet::handleCollision()
{
	if (!m_custom_data->network_manager->isServer())
	{
		return;
	}

	if (m_alive)
	{
		m_alive = false;
		m_custom_data->scenetree->deleteEntity(info.ID);
	}
}