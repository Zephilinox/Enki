#include "PlayerText.hpp"

//LIBS
#include <spdlog/fmt/fmt.h>
#include <Enki/Scenetree.hpp>

//SELF
#include "Ball.hpp"
#include "Paddle.hpp"

PlayerText::PlayerText(enki::EntityInfo info, enki::GameData* game_data)
	: Entity(info, game_data)
{
}

void PlayerText::onSpawn([[maybe_unused]] enki::Packet p)
{
	if (!font.loadFromFile("resources/arial.ttf"))
	{
		fmt::print("Failed to load resources/arial.ttf");
	}

	label.setFont(font);
	label.setString(std::string("Player ") + std::to_string(info.ownerID));
	label.setFillColor(sf::Color::Black);
	label.setCharacterSize(12);
}

void PlayerText::update([[maybe_unused]] float dt)
{
	auto console = spdlog::get("console");
	auto scenetree = game_data->scenetree;
	auto parent = scenetree->findEntity(info.parentID);

	if (parent)
	{
		if (parent->info.type == hash("Paddle"))
		{
			auto parent_paddle = static_cast<Paddle*>(parent);
			label.setPosition(parent_paddle->sprite.getPosition() - sf::Vector2f(6, 16));
		}
		else if (parent->info.type == hash("PlayerText"))
		{
			auto parent_player_text = static_cast<PlayerText*>(parent);
			if (parent_player_text)
			{
				label.setString(std::to_string(info.type) + " " + std::to_string(info.ID));
				label.setPosition(parent_player_text->label.getPosition() - sf::Vector2f(0, 16));
			}
		}
	}
}

void PlayerText::draw(enki::Renderer* renderer)
{
	renderer->draw(&label);
}