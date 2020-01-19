#include "Asteroid.hpp"

//STD
#include <experimental/vector>

//LIBS
#include <Enki/Scenetree.hpp>

Asteroid::Asteroid(enki::EntityInfo info, enki::GameData* data, CustomData* custom_data, sf::RenderWindow* window)
	: Entity(info, data)
	, custom_data(custom_data)
	, window(window)
{
	network_tick_rate = 1;
}

void Asteroid::onSpawn([[maybe_unused]]enki::Packet& p)
{
	auto console = spdlog::get("console");

	if (p.canDeserialize<int, float, float, float>())
	{
		int sides = p.read<int>();
		float x = p.read<float>();
		float y = p.read<float>();
		speed = p.read<float>();
		constructAsteroid(sides, x, y);
	}
}

void Asteroid::update(float dt)
{
	if (!isOwner())
	{
		return;
	}

	auto input_manager = custom_data->input_manager;

	shape.move(velocity.x * dt, velocity.y * dt);

	if (shape.getPosition().x + radius <= 0)
	{
		shape.setPosition(window->getView().getSize().x, shape.getPosition().y);
	}
	else if (shape.getPosition().x - radius >= window->getView().getSize().x)
	{
		shape.setPosition(0, shape.getPosition().y);
	}
	else if (shape.getPosition().y + radius <= 0)
	{
		shape.setPosition(shape.getPosition().x, window->getView().getSize().y);
	}
	else if (shape.getPosition().y - radius >= window->getView().getSize().y)
	{
		shape.setPosition(shape.getPosition().x, 0);
	}

	if (!alive)
	{
		game_data->scenetree->deleteEntity(info.ID);
	}
}

void Asteroid::draw(sf::RenderWindow& window_) const
{
	window_.draw(shape);
}

void Asteroid::serializeOnConnection(enki::Packet& p)
{
	serializeOnTick(p);
	p << shape.getPointCount() << speed;
}

void Asteroid::deserializeOnConnection(enki::Packet& p)
{
	deserializeOnTick(p);

	unsigned sides;
	p >> sides >> speed;

	constructAsteroid(sides, shape.getPosition().x, shape.getPosition().y);
}

void Asteroid::serializeOnTick(enki::Packet& p)
{
	p.writeCompressedFloat(shape.getPosition().x, 0, 1280, 0.01f);
	p.writeCompressedFloat(shape.getPosition().y, 0, 720, 0.01f);
	p.writeCompressedFloat(shape.getRotation(), 0, 360, 0.01f);
}

void Asteroid::deserializeOnTick(enki::Packet& p)
{
	float x = p.readCompressedFloat(0, 1280, 0.01f);
	float y = p.readCompressedFloat(0, 720, 0.01f);
	shape.setPosition(x, y);
	shape.setRotation(p.readCompressedFloat(0, 360, 0.01f));
}

bool Asteroid::isAlive() const
{
	return alive;
}

bool Asteroid::canSplit() const
{
	return shape.getPointCount() - 2 >= 5;
}

sf::Vector2f Asteroid::getPosition() const
{
	return shape.getPosition();
}

float Asteroid::getRadius() const
{
	return radius;
}

float Asteroid::getRotation() const
{
	return shape.getRotation();
}

void Asteroid::constructAsteroid(unsigned sides, float x, float y)
{
	radius = (sides - 4) * 8;
	shape.setPosition(x, y);
	shape.setOutlineThickness(1.5f);
	shape.setOutlineColor(sf::Color::Black);
	createShape(sides);

	sf::Vector2f target_pos(
		std::rand() % 1280,
		std::rand() % 720);

	sf::Vector2f target_dir = target_pos - shape.getPosition();
	float length = std::sqrtf((target_dir.x * target_dir.x) + (target_dir.y * target_dir.y));
	sf::Vector2f target_dir_norm;
	if (length != 0)
	{
		target_dir_norm.x = target_dir.x / length;
		target_dir_norm.y = target_dir.y / length;
	}
	velocity.x *= target_dir_norm.x * speed / sides;
	velocity.y *= target_dir_norm.y * speed / sides;
	rotation_speed = (std::rand() % int((speed * 2) / sides)) - speed / sides;
}

void Asteroid::createShape(unsigned sides)
{
	float total_angle = (sides - 2) * 180;
	float interior_angle = total_angle / sides;
	float exterior_angle = 180 - interior_angle;
	shape.setPointCount(sides);

	const auto degToVector = [](float angle) -> sf::Vector2f
	{
		return sf::Vector2f(
			std::sin(angle * (3.1415 / 180.0f)),
			-std::cos(angle * 3.1415 / 180.0f));
	};

	for (unsigned i = 0; i < sides; ++i)
	{
		sf::Vector2f angleDir = degToVector(exterior_angle * i);
		shape.setPoint(i, sf::Vector2f(angleDir.x, angleDir.y) * radius);
	}
}

void Asteroid::handleCollision()
{
	if (!isOwner())
	{
		return;
	}

	if (alive)
	{
		alive = false;
		split();
	}
}

void Asteroid::split()
{
	if (canSplit())
	{
		const auto newAsteroid = [&]()
		{
			enki::Packet p;
			p << shape.getPointCount() - 2
				<< shape.getPosition().x + (std::rand() % 20 - 10)
				<< shape.getPosition().y + (std::rand() % 20 - 10)
				<< speed + (std::rand() % 200 + 50);
			game_data->scenetree->createNetworkedEntity({ "Asteroid", "Asteroid" }, p);
		};

		newAsteroid();
		newAsteroid();
	}

	alive = false;
	game_data->scenetree->deleteEntity(info.ID);
}
