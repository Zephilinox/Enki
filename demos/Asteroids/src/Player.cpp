#include "Player.hpp"

//STD
#include <experimental/vector>

//LIBS
#include <Enki/Scenetree.hpp>

Player::Player(enki::EntityInfo info, CustomData* custom_data)
	: Entity(info)
	, custom_data(custom_data)
{
	network_tick_rate = 1;
}

//todo: onSpawn isn't called for clients that are receiving the entire entity tree, instead, (de)serializeOnConnection is used, need to fix
void Player::onSpawn([[maybe_unused]]enki::Packet p)
{
	auto console = spdlog::get("console");
	if (!ship_tex.loadFromFile("resources/ship.png"))
	{
		console->error(":(");
	}

	ship.setTexture(ship_tex);
	ship.setOrigin(
		static_cast<float>(ship_tex.getSize().x / 2),
		static_cast<float>(ship_tex.getSize().y / 2));
	ship.setPosition(static_cast<float>(1280 / 2), static_cast<float>(720 / 2));

	if (info.ownerID == 1)
	{
		ship.setColor(sf::Color(0, 100, 200));	  //blue
		up = enki::Keyboard::Key::W;
		down = enki::Keyboard::Key::S;
		left = enki::Keyboard::Key::A;
		right = enki::Keyboard::Key::D;
		slow = enki::Keyboard::Key::Shift;
		shoot = enki::Keyboard::Key::F;
	}
	else if (info.ownerID == 2)
	{
		ship.setColor(sf::Color(200, 60, 60));	  //red
		up = enki::Keyboard::Key::W;
		down = enki::Keyboard::Key::S;
		left = enki::Keyboard::Key::A;
		right = enki::Keyboard::Key::D;
		slow = enki::Keyboard::Key::Shift;
		shoot = enki::Keyboard::Key::F;
	}
	else if (info.ownerID == 3)
	{
		ship.setColor(sf::Color(60, 200, 60));	  //green
		up = enki::Keyboard::Key::W;
		down = enki::Keyboard::Key::S;
		left = enki::Keyboard::Key::A;
		right = enki::Keyboard::Key::D;
		slow = enki::Keyboard::Key::Shift;
		shoot = enki::Keyboard::Key::F;
	}
	else if (info.ownerID == 4)
	{
		ship.setColor(sf::Color(200, 160, 60));	   //orange
		up = enki::Keyboard::Key::W;
		down = enki::Keyboard::Key::S;
		left = enki::Keyboard::Key::A;
		right = enki::Keyboard::Key::D;
		slow = enki::Keyboard::Key::Shift;
		shoot = enki::Keyboard::Key::F;
	}

	view = custom_data->window_sfml->getDefaultView();
}

void Player::update(float dt)
{
	if (!isOwner(custom_data->network_manager))
	{
		return;
	}

	auto* input_manager = custom_data->input_manager;
		
	const float ship_rot_rads = ship.getRotation() * (3.1415f / 180.0f);
	const float ship_sin = std::sin(ship_rot_rads);
	const float ship_cos = std::cos(ship_rot_rads);

	if (input_manager->isKeyDown(up))
	{
		velocity.x += speed * ship_sin * dt;
		velocity.y += -1 * speed * ship_cos * dt;
	}

	if (input_manager->isKeyDown(down))
	{
		velocity.x += -1 * speed * ship_sin * dt;
		velocity.y += speed * ship_cos * dt;
	}

	if (input_manager->isKeyDown(left))
	{
		ship.rotate(-200 * dt);
	}

	if (input_manager->isKeyDown(right))
	{
		ship.rotate(200 * dt);
	}

	float length = std::sqrtf((velocity.x * velocity.x) + (velocity.y * velocity.y));

	if (input_manager->isKeyDown(slow))
	{
		if (length > 5 && velocity.x != 0 && velocity.y != 0)
		{
			sf::Vector2f normVelocity;
			if (length != 0)
			{
				normVelocity.x = velocity.x / length;
				normVelocity.y = velocity.y / length;
			}

			velocity.x -= speed * normVelocity.x * dt;
			velocity.y -= speed * normVelocity.y * dt;
		}
		else
		{
			velocity.x = 0;
			velocity.y = 0;
		}
	}
	
	length = std::sqrtf((velocity.x * velocity.x) + (velocity.y * velocity.y));

	if (length > max_velocity_length)
	{
		velocity.x *= (max_velocity_length / length) * (1.0f - dt);
		velocity.y *= (max_velocity_length / length) * (1.0f - dt);
	}

	ship.move(velocity * dt);

	if (ship.getPosition().x + (ship_tex.getSize().x / 2) <= 0)
	{
		ship.setPosition(custom_data->window_sfml->getView().getSize().x, ship.getPosition().y);
	}
	else if (ship.getPosition().x - (ship_tex.getSize().x / 2) >= custom_data->window_sfml->getView().getSize().x)
	{
		ship.setPosition(0, ship.getPosition().y);
	}
	else if (ship.getPosition().y + (ship_tex.getSize().y / 2) <= 0)
	{
		ship.setPosition(ship.getPosition().x, custom_data->window_sfml->getView().getSize().y);
	}
	else if (ship.getPosition().y - (ship_tex.getSize().y / 2) >= custom_data->window_sfml->getView().getSize().y)
	{
		ship.setPosition(ship.getPosition().x, 0);
	}

	if (input_manager->isKeyDown(shoot) &&
		shoot_timer.getElapsedTime() > shoot_delay)
	{
		shoot_timer.restart();

		int bullets = (std::rand() % 20) + 5;
		length = std::sqrtf((velocity.x * velocity.x) + (velocity.y * velocity.y));
		for (int i = 0; i < bullets; ++i)
		{
			enki::Packet p;
			p << ship.getPosition().x
			  << ship.getPosition().y
			  << 300.0f + length
			  << ship.getRotation() + static_cast<float>((std::rand() % 600) / 100.0f)
			  << ship.getColor().r
			  << ship.getColor().g
			  << ship.getColor().b;
			custom_data->scenetree->createEntityNetworkedRequest(hash("Bullet"), "Bullet", custom_data->scenetree->findEntityByName("Bullets")->info.ID, p);
		}
	}

	if (flashing_timer.getElapsedTime() > flashing_duration)
	{
		custom_data->scenetree->rpc_man.callEntityRPC(&Player::stopInvincible, "stopInvincible", this);
	}
}

void Player::draw(enki::Renderer* renderer) 
{
	if (!was_damaged)
	{
		renderer->draw(&ship);
	}
	else if (flashing_timer.getElapsedTime() < flashing_duration)
	{
		int milli = static_cast<int>(flashing_timer.getElapsedTime<enki::Timer::milliseconds>());
		int percentage = 20;
		int rem_milli = milli % 1000 % (percentage * 10); //ignore seconds and get a percentage of the remainder
		if (rem_milli < percentage * 5)
		{
			renderer->draw(&ship);
		}
	}
}

void Player::serializeOnTick(enki::Packet& p)
{
	p.writeCompressedFloat(ship.getPosition().x, 0, 1280, 0.01f);
	p.writeCompressedFloat(ship.getPosition().y, 0, 720, 0.01f);
	p.writeCompressedFloat(ship.getRotation(), 0, 360, 0.01f);
}

void Player::deserializeOnTick(enki::Packet& p)
{
	float x = p.readCompressedFloat(0, 1280, 0.01f);
	float y = p.readCompressedFloat(0, 720, 0.01f);
	ship.setPosition(x, y);
	ship.setRotation(p.readCompressedFloat(0, 360, 0.01f));
}

std::vector<std::pair<std::string, std::string>> Player::serializeToStrings() const
{
	return {
		{"Position", fmt::format("{{{}, {}}}", ship.getPosition().x, ship.getPosition().y)},
		{"Velocity", fmt::format("{{{}, {}}}", velocity.x, velocity.y)},
		{"Rotation", std::to_string(ship.getRotation())},
		{"Speed", std::to_string(speed)},
		{"Was Damaged", was_damaged ? "true" : "false"},
		{"Lives", std::to_string(lives)},
		{"Max Velocity Length", std::to_string(max_velocity_length)},
		{"Flashing Timer", std::to_string(flashing_timer.getElapsedTime())},
		{"Flashing Duration", std::to_string(flashing_duration)},
		{"Shooting Timer", std::to_string(shoot_timer.getElapsedTime())},
		{"Shooting Delay", std::to_string(shoot_delay)},
	};
}

sf::Vector2f Player::getPosition() const
{
	return ship.getPosition();
}

bool Player::isInvincible() const
{
	return was_damaged;
}

int Player::getLives() const
{
	return lives;
}

sf::Color Player::getColour() const
{
	return ship.getColor();
}

void Player::startInvincible()
{
	flashing_timer.restart();
	was_damaged = true;
}

void Player::stopInvincible()
{
	was_damaged = false;
}

void Player::handleCollision()
{
	if (!isOwner(custom_data->network_manager))
	{
		return;
	}

	if (was_damaged == false)
	{
		if (lives > 0)
		{
			lives--;
		}
		
		custom_data->scenetree->rpc_man.callEntityRPC(&Player::startInvincible, "startInvincible", this);
	}
}


void Player::serializeOnConnection(enki::Packet& p)
{
	serializeOnTick(p);
}

void Player::deserializeOnConnection(enki::Packet& p)
{
	onSpawn(p);
	deserializeOnTick(p);
}
