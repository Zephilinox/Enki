#include "Console.hpp"

//STD
#include <sstream>

//SELF
#include "IMGUI/imgui_SFML.h"
#include "Enki/Scenegraph.hpp"

namespace enki
{
	Console::Console(enki::EntityInfo info, enki::GameData* game_data)
		: Entity(info, game_data)
	{
		user_input.resize(256);

		commands.emplace_back(Command{
		"help",
		"display help",
		[&](std::vector<std::string> tokens)
		{
			addItem({
				"help",
				"You can type \"commands\" to see a list of commands",
				sf::Color::White,
			Item::Type::CommandOutput,
			});
		}});

		commands.emplace_back(Command{
		"commands",
		"display a list of commands",
		[&](std::vector<std::string> tokens)
		{
			std::string command_strings;
			for (auto& command : commands)
			{
				command_strings += "\"" + command.name + "\": " + command.description + "\n";
			}

			addItem({
				"commands",
				command_strings,
				sf::Color::White,
			Item::Type::CommandOutput,
			});
		}});

		commands.emplace_back(Command{
		"say",
		"say everything past the commands name",
		[&](std::vector<std::string> tokens)
		{
			std::string text;
			for (auto& s : tokens)
			{
				text += s + " ";
			}

			addItem({
				"zephilinox",
				text,
				sf::Color::White,
			Item::Type::CommandOutput,
			});
		}});

		commands.emplace_back(Command{
		"scenetree",
		"print all entities",
		[this](std::vector<std::string> tokens)
		{
			std::string entity_strings;
			this->game_data->scenegraph->forEachEntity([&](const Entity& ent)
			{
				entity_strings += fmt::format("{}\n", ent.info);
			});

			this->addItem({
				"scenetree",
				entity_strings,
				sf::Color::White,
			Item::Type::CommandOutput,
			});
		}});

		commands.emplace_back(Command{
		"delete",
		"delete an entity with the given ID if it exists",
		[this](std::vector<std::string> tokens)
		{
			if (tokens.size() != 1)
			{
				addItem({
					"delete",
					"Failed. There must be exactly one token representing the entity ID",
					sf::Color::Red,
				Item::Type::Other,
				});
				return;
			}

			EntityID id = std::stoi(tokens[0]);
			if (!this->game_data->scenegraph->entityExists(id))
			{
				addItem({
					"delete",
					"Failed. The entity does not exist",
					sf::Color::Red,
				Item::Type::Other,
				});
				return;
			}

			this->game_data->scenegraph->deleteEntity(id);

			this->addItem({
				"delete",
				fmt::format("deleted {}", this->game_data->scenegraph->getEntity(id)->info),
				sf::Color::White,
				Item::Type::CommandOutput,
			});
		}});

		commands.emplace_back(Command{
		"create",
		"create an entity. e.g. create Console MainConsole",
		[this](std::vector<std::string> tokens)
		{
			if (tokens.size() < 2 || tokens.size() > 5)
			{
				addItem({
					"create",
					"Failed. Tokens must be between 2 and 5",
					sf::Color::Red,
				Item::Type::Other,
				});
				return;
			}

			HashedID entity_type = hash(tokens[0]);

			Entity* ent;

			if (tokens.size() == 2)
			{
				ent = this->game_data->scenegraph->createEntity({
					entity_type,
					tokens[1],
				});
			}
			else if (tokens.size() == 3)
			{
				ent = this->game_data->scenegraph->createEntity({
					entity_type,
					tokens[1],
					std::stoi(tokens[2]),
				});
			}
			else if (tokens.size() == 4)
			{
				ent = this->game_data->scenegraph->createEntity({
					entity_type,
					tokens[1],
					std::stoi(tokens[2]),
					static_cast<ClientID>(std::stoi(tokens[3])),
				});
			}
			else if (tokens.size() == 5)
			{
				ent = this->game_data->scenegraph->createEntity({
					entity_type,
					tokens[1],
					std::stoi(tokens[2]),
					static_cast<ClientID>(std::stoi(tokens[3])),
					std::stoi(tokens[4]),
				});
			}

			if (ent == nullptr)
			{
				addItem({
					"create",
					"Failed. The scenegraph could not construct that entity",
					sf::Color::Red,
					Item::Type::Other,
				});
				return;
			}

			this->addItem({
				"create",
				fmt::format("{}", ent->info),
				sf::Color::White,
			Item::Type::CommandOutput,
			});
		}});
	}

	void Console::input(sf::Event& e)
	{
		if (e.type == sf::Event::KeyPressed)
		{
			if (e.key.code == sf::Keyboard::Key::Quote)
			{
				opened = !opened;
			}
		}
	}

	void Console::update(float dt)
	{
		if (opened)
		{
			if (!ImGui::Begin("Console", &opened))
			{
				ImGui::End();
				return;
			}

			ImGui::SetWindowSize({400, 500}, ImGuiCond_FirstUseEver);
			ImGui::SetWindowPos({2, 32}, ImGuiCond_FirstUseEver);
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Close Console"))
					opened = false;
				ImGui::EndPopup();
			}

			if (ImGui::SmallButton("Test"))
			{
				addItem({
					"test",
					"hello\nhello there\nhi",
					sf::Color::White,
					Item::Type::Other,
				});
			}

			ImGui::Separator();

			ImGui::BeginChild("ScrollingRegion",
				{0, -32},
				false,
				ImGuiWindowFlags_HorizontalScrollbar);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

			for (const auto& item : items)
			{
				if (!filter.PassFilter(item.text.c_str()))
					continue;

				ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 0.8f, 0.0f, 1.0f});
				ImGui::TextUnformatted(item.prefix.c_str());
				ImGui::PopStyleColor();
				
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1)); // Tighten spacing
				ImGui::SameLine(0, -1);
				ImGui::PopStyleVar();

				ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f});
				ImGui::TextUnformatted(": ");
				ImGui::PopStyleColor();

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1)); // Tighten spacing
				ImGui::SameLine(0, -1);
				ImGui::PopStyleVar();

				ImGui::PushStyleColor(ImGuiCol_Text, item.colour);
				ImGui::TextUnformatted(item.text.c_str());
				ImGui::PopStyleColor();
			}

			ImGui::SetScrollHereY(1.0f);
			ImGui::PopStyleVar();
			ImGui::EndChild();
			ImGui::Separator();

			bool reclaim_focus = false;
			if (ImGui::InputText("Input",
				user_input.data(),
				user_input.size(),
				ImGuiInputTextFlags_EnterReturnsTrue))
			{
				addInput(user_input);
				user_input.clear();
				user_input.resize(256);
				reclaim_focus = true;
			}

			// Auto-focus on window apparition
			ImGui::SetItemDefaultFocus();
			if (reclaim_focus)
				ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

			ImGui::Separator();
			ImGui::End();
		}
	}

	void Console::addItem(Item item)
	{
		items.emplace_back(std::move(item));
	}

	void Console::addInput(std::string input)
	{
		addItem({
			"input",
			input,
			sf::Color::White,
			Item::Type::UserInput,
		});

		history.push_back(input);

		std::string trimmed_input;
		for (int i = 0; i < user_input.size(); ++i)
		{
			if (!user_input[i])
			{
				break;
			}

			trimmed_input.push_back(user_input[i]);
		}

		std::stringstream ss(trimmed_input);
		std::vector<std::string> tokens;
		std::string token;
		while (std::getline(ss, token, ' '))
		{
			tokens.push_back(token);
		}

		if (!tokens.empty())
		{
			std::string name = tokens[0];
			tokens.erase(tokens.begin());

			executeCommand(getCommand(name), tokens);
		}
	}

	Console::Command* Console::getCommand(std::string name)
	{
		for (auto& c : commands)
		{
			if (c.name == name)
			{
				return &c;
			}
		}

		return nullptr;
	}

	void Console::executeCommand(Command* command, std::vector<std::string> tokens)
	{
		if (!command)
		{
			addItem({
				"failed",
				"The command could not be found",
				sf::Color::Red,
			Item::Type::Other,
			});
			return;
		}

		command->function(tokens);
	}
}
