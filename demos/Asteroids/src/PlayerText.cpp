#include "PlayerText.hpp"

//LIBS
#include <spdlog/fmt/fmt.h>
#include <Enki/Scenetree.hpp>

//SELF
#include "Player.hpp"

PlayerText::PlayerText(enki::EntityInfo info, enki::GameData* game_data)
	: Entity(info, game_data)
{
}

void PlayerText::onSpawn([[maybe_unused]] enki::Packet& p)
{
	if (!font.loadFromFile("resources/arial.ttf"))
	{
		fmt::print("Failed to load resources/arial.ttf");
	}

	label.setFont(font);
	label.setString(std::string("Player ") + std::to_string(info.ownerID));
	label.setCharacterSize(10);
}

void PlayerText::update([[maybe_unused]]float dt)
{
	auto console = spdlog::get("console");
	auto parent = game_data->scenetree->getEntity(info.parentID);

	if (parent && parent->info.type == "Player")
	{
		auto parent_player = static_cast<Player*>(parent);
		label.setFillColor(parent_player->getColour());
		label.setPosition(parent_player->getPosition() - sf::Vector2f(20, 25));
	}
}

void PlayerText::draw(sf::RenderWindow& window) const
{
	window.draw(label);
}