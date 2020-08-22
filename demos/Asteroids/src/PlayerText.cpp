#include "PlayerText.hpp"

//LIBS
#include <spdlog/fmt/fmt.h>
#include <Enki/Scenetree.hpp>

//SELF
#include "Player.hpp"

PlayerText::PlayerText(enki::EntityInfo info, CustomData* custom_data)
	: Entity(std::move(info))
	, custom_data(custom_data)
{
}

void PlayerText::onSpawn([[maybe_unused]] enki::Packet p)
{
	if (!custom_data->font_manager->registerFont("resources/arial.ttf", "arial"))
	{
		fmt::print("Failed to load resources/arial.ttf");
	}

	label.setFont(*custom_data->font_manager->getFont(hash("arial")));
	label.setString(std::string("Player ") + std::to_string(info.ownerID));
	label.setCharacterSize(10);
}

void PlayerText::update([[maybe_unused]]float dt)
{
	auto console = spdlog::get("console");
	auto parent = custom_data->scenetree->findEntity(info.parentID);

	if (parent && parent->info.type == hash("Player"))
	{
		auto parent_player = static_cast<Player*>(parent);
		label.setFillColor(parent_player->getColour());
		label.setPosition(parent_player->getPosition() - sf::Vector2f(20, 25));
	}
}

void PlayerText::draw(enki::Renderer* renderer)
{
	renderer->draw(&label);
}