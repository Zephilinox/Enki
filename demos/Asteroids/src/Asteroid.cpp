#include "Asteroid.hpp"

//STD
#include <experimental/vector>

//LIBS
#include <Enki/Scenetree.hpp>
#include "Enki/Window/WindowSFML.hpp"

Asteroid::Asteroid(enki::EntityInfo info, CustomData* custom_data)
	: Entity(info)
	, custom_data(custom_data)
	, window(custom_data->window->as<enki::WindowSFML>()->getRawWindow())
	, shape(new sf::ConvexShape)
	, shape_ref_count(new int(1))
{
	network_tick_rate = 1;
}

Asteroid::Asteroid(Asteroid&& asteroid) noexcept
	: Entity(std::move(asteroid))
	, shape(asteroid.shape)
	, shape_ref_count(asteroid.shape_ref_count)
	, custom_data(asteroid.custom_data)
	, window(asteroid.window)
	, speed(asteroid.speed)
	, radius(asteroid.radius)
	, rotation_speed(asteroid.rotation_speed)
	, velocity(asteroid.velocity)
	, alive(asteroid.alive)
{
}

Asteroid::Asteroid(const Asteroid& asteroid)
	: Entity(asteroid)
	, shape(asteroid.shape)
	, shape_ref_count(asteroid.shape_ref_count)
	, custom_data(asteroid.custom_data)
	, window(asteroid.window)
	, speed(asteroid.speed)
	, radius(asteroid.radius)
	, rotation_speed(asteroid.rotation_speed)
	, velocity(asteroid.velocity)
	, alive(asteroid.alive)
{
	(*shape_ref_count)++;
}

Asteroid& Asteroid::operator=(Asteroid&& asteroid) noexcept
{
	if (this != &asteroid)
	{
		(*shape_ref_count)--;
		if ((*shape_ref_count == 0))
		{
			delete shape;
			delete shape_ref_count;
		}

		(*asteroid.shape_ref_count)++;
		
		Entity::operator=(std::move(asteroid));
		shape = asteroid.shape;
		shape_ref_count = asteroid.shape_ref_count;
		custom_data = asteroid.custom_data;
		window = asteroid.window;
		speed = asteroid.speed;
		radius = asteroid.radius;
		rotation_speed = asteroid.rotation_speed;
		velocity = asteroid.velocity;
		alive = asteroid.alive;
	}

	return *this;
}

Asteroid& Asteroid::operator=(const Asteroid& asteroid)
{
	if (this != &asteroid)
	{
		(*shape_ref_count)--;
		if ((*shape_ref_count == 0))
		{
			delete shape;
			delete shape_ref_count;
		}

		(*asteroid.shape_ref_count)++;
		
		Entity::operator=(asteroid);
		shape = asteroid.shape;
		shape_ref_count = asteroid.shape_ref_count;
		custom_data = asteroid.custom_data;
		window = asteroid.window;
		speed = asteroid.speed;
		radius = asteroid.radius;
		rotation_speed = asteroid.rotation_speed;
		velocity = asteroid.velocity;
		alive = asteroid.alive;
	}

	return *this;
}

Asteroid::~Asteroid()
{
	(*shape_ref_count)--;
	
	if (*shape_ref_count == 0)
	{
		delete shape;
		delete shape_ref_count;
	}
}

void Asteroid::onSpawn([[maybe_unused]]enki::Packet p)
{
	auto console = spdlog::get("console");

	int sides = p.read<int>();
	float x = p.read<float>();
	float y = p.read<float>();
	speed = p.read<float>();
	constructAsteroid(sides, x, y);
}

std::unique_ptr<enki::Entity> Asteroid::clone()
{
	return std::make_unique<Asteroid>(*this);
}

void Asteroid::update(float dt)
{
	if (!isOwner(custom_data->network_manager))
	{
		return;
	}

	shape->move(velocity.x * dt, velocity.y * dt);

	if (shape->getPosition().x + radius <= 0)
	{
		shape->setPosition(window->getView().getSize().x, shape->getPosition().y);
	}
	else if (shape->getPosition().x - radius >= window->getView().getSize().x)
	{
		shape->setPosition(0, shape->getPosition().y);
	}
	else if (shape->getPosition().y + radius <= 0)
	{
		shape->setPosition(shape->getPosition().x, window->getView().getSize().y);
	}
	else if (shape->getPosition().y - radius >= window->getView().getSize().y)
	{
		shape->setPosition(shape->getPosition().x, 0);
	}
}

void Asteroid::draw(enki::Renderer* renderer)
{
	renderer->draw(shape);
}

void Asteroid::receive(enki::Message* msg)
{
	if (msg->id == hash_constexpr("Collision"))
	{
		handleCollision();
	}
}

void Asteroid::serializeOnConnection(enki::Packet& p)
{
	serializeOnTick(p);
	p << shape->getPointCount() << speed;
}

void Asteroid::deserializeOnConnection(enki::Packet& p)
{
	deserializeOnTick(p);

	std::size_t sides;
	p >> sides >> speed;

	constructAsteroid(sides, shape->getPosition().x, shape->getPosition().y);
}

void Asteroid::serializeOnTick(enki::Packet& p)
{
	p.writeCompressedFloat(shape->getPosition().x, 0, 1280, 0.01f);
	p.writeCompressedFloat(shape->getPosition().y, 0, 720, 0.01f);
	p.writeCompressedFloat(shape->getRotation(), 0, 360, 0.01f);
}

void Asteroid::deserializeOnTick(enki::Packet& p)
{
	float x = p.readCompressedFloat(0, 1280, 0.01f);
	float y = p.readCompressedFloat(0, 720, 0.01f);
	shape->setPosition(x, y);
	shape->setRotation(p.readCompressedFloat(0, 360, 0.01f));
}

bool Asteroid::isAlive() const
{
	return alive;
}

bool Asteroid::canSplit() const
{
	return alive && shape->getPointCount() - 2 >= 5;
}

sf::Vector2f Asteroid::getPosition() const
{
	return shape->getPosition();
}

float Asteroid::getRadius() const
{
	return radius;
}

float Asteroid::getRotation() const
{
	return shape->getRotation();
}

void Asteroid::constructAsteroid(unsigned sides, float x, float y)
{
	radius = (sides - 4) * 8;
	shape->setPosition(x, y);
	shape->setOutlineThickness(1.5f);
	shape->setOutlineColor(sf::Color::Black);
	createShape(sides);

	sf::Vector2f target_pos(
		std::rand() % 1280,
		std::rand() % 720);

	sf::Vector2f target_dir = target_pos - shape->getPosition();
	float length = std::sqrtf((target_dir.x * target_dir.x) + (target_dir.y * target_dir.y));
	sf::Vector2f target_dir_norm;
	if (length != 0)
	{
		target_dir_norm.x = target_dir.x / length;
		target_dir_norm.y = target_dir.y / length;
	}
	velocity.x *= target_dir_norm.x * speed / sides;
	velocity.y *= target_dir_norm.y * speed / sides;
	int rot = (speed * 10) / sides;
	int max_rot = (speed * 20) / sides;
	//rotation_speed = (std::rand() % max_rot) - rot;
}

void Asteroid::createShape(unsigned sides)
{
	float total_angle = (sides - 2) * 180;
	float interior_angle = total_angle / sides;
	float exterior_angle = 180 - interior_angle;
	shape->setPointCount(sides);

	const auto degToVector = [](float angle) -> sf::Vector2f
	{
		return sf::Vector2f(
			std::sin(angle * (3.1415 / 180.0f)),
			-std::cos(angle * 3.1415 / 180.0f));
	};

	for (unsigned i = 0; i < sides; ++i)
	{
		sf::Vector2f angleDir = degToVector(exterior_angle * i);
		shape->setPoint(i, sf::Vector2f(angleDir.x, angleDir.y) * radius);
	}
}

void Asteroid::handleCollision()
{
	if (!isOwner(custom_data->network_manager))
	{
		return;
	}

	split();
}

void Asteroid::split()
{
	if (canSplit())
	{
		const auto newAsteroid = [&]()
		{
			auto x = shape->getPosition().x + (std::rand() % 20 - 10);
			auto y = shape->getPosition().y + (std::rand() % 20 - 10);
			enki::Packet p;
			p << static_cast<int>(shape->getPointCount() - 2);
			p << x;
			p << y;
			p << speed + (std::rand() % 200 + 50);
			
			custom_data->scenetree->createEntityNetworkedRequest(hash("Asteroid"), "Asteroid", 0, p);
		};

		newAsteroid();
		newAsteroid();
	}

	alive = false;
	custom_data->scenetree->deleteEntity(info.ID);
}
