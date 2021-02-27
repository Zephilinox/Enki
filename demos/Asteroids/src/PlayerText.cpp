#include "PlayerText.hpp"

//SELF
#include "Player.hpp"

//LIBS
#include <spdlog/fmt/fmt.h>
#include <Enki/Scenetree.hpp>
#include <Enki/Renderer/RendererSFML.hpp>
#include <Enki/Graphics/TextSFML.hpp>

PlayerText::PlayerText(enki::EntityInfo info, CustomData* custom_data)
	: Entity(std::move(info))
	, custom_data(custom_data)
	, label(custom_data->renderer->createText())
{
}

void PlayerText::onSpawn([[maybe_unused]] enki::Packet p)
{
	if (!custom_data->font_manager->registerFont(custom_data->renderer, "resources/arial.ttf", "arial"))
	{
		fmt::print("Failed to load resources/arial.ttf");
	}

	label->setFont(custom_data->font_manager->getFont(hash("arial")));
	label->setString(std::string("Player ") + std::to_string(info.ownerID));
	label->setCharacterSize(10);
}

void PlayerText::update([[maybe_unused]]float dt)
{
	auto console = spdlog::get("console");
	auto parent = custom_data->scenetree->findEntity(info.parentID);

	if (parent && parent->info.type == hash("Player"))
	{
		const auto parent_player = static_cast<Player*>(parent);
		label->setFillColor({
			parent_player->getColour().r,
			parent_player->getColour().g,
			parent_player->getColour().b,
			255,
		});
		
		label->setPosition(
			parent_player->getPosition().x - 20,
			parent_player->getPosition().y - 25
		);
	}
}

void PlayerText::draw(enki::Renderer* renderer)
{
	renderer->as<enki::RendererSFML>()->queue(&label->as<enki::TextSFML>()->getRawText());
}

std::vector<std::pair<std::string, std::string>> PlayerText::serializeToStrings() const
{
	return {
		{"Position", fmt::format("{{{}, {}}}",
		label->getPosition().x,
		label->getPosition().y)},
	};
}
