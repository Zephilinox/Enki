#include "Console.hpp"

//SELF
#include "Enki/Scenetree.hpp"
#include "IMGUI/imgui_SFML.h"
#include "ScenetreeGUI.hpp"

//STD
#include <sstream>

namespace enki
{
Console::Console(EntityInfo info, Scenetree* scenetree)
	: Entity(std::move(info))
	, scenetree(scenetree)
{
	user_input.resize(256);

	commands.emplace_back(Command{
		"help",
		"display help",
		[&](std::vector<std::string> tokens) {
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
		[&](std::vector<std::string> tokens) {
			std::string command_strings;
			for (auto& command : commands)
				command_strings += "\"" + command.name + "\": " + command.description + "\n";

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
		[&](std::vector<std::string> tokens) {
			std::string text;
			for (auto& s : tokens)
				text += s + " ";

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
		[this](std::vector<std::string> tokens) {
			std::string entity_strings;
			this->scenetree->forEachEntity([&](const Entity& ent) {
				entity_strings += fmt::format("{}\n", ent.info);
			});

			this->addItem({
				"scenetree",
				entity_strings,
				sf::Color::White,
				Item::Type::CommandOutput,
			});

			printTree(this->scenetree);
		}});

	commands.emplace_back(Command{
		"gui",
		"open a gui",
		[this](std::vector<std::string> tokens) {
			if (tokens[0] == "scenetree")
			{
				auto* scenetree_gui = this->scenetree->findEntityByType<ScenetreeGUI>(hash("ScenetreeGUI"));
				if (scenetree_gui)
				{
					scenetree_gui->show = true;
					return;
				}

				scenetree_gui = static_cast<ScenetreeGUI*>(this->scenetree->createEntityLocal(hash("ScenetreeGUI"), "ScenetreeGUI"));
				if (scenetree_gui)
					scenetree_gui->show = true;
			}
		}});
	
	commands.emplace_back(Command{
		"delete",
		"delete an entity with the given ID if it exists",
		[this](std::vector<std::string> tokens) {
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

			bool is_pretty = tokens[0][0] == 'L';
			EntityID id;
			if (is_pretty)
				id = generateEntityIDFromPrettyID(tokens[0]);
			else
				id = std::stoll(tokens[0]);

			if (id == this->info.ID)
			{
				addItem({
					"delete",
					"Failed. Cannot delete myself.",
					sf::Color::Red,
					Item::Type::Other,
				});
				return;
			}

			auto e = this->scenetree->findEntity(id);

			if (!e)
			{
				addItem({
					"delete",
					fmt::format("Failed. The entity with ID {}({}) does not exist", id, prettyID(id)),
					sf::Color::Red,
					Item::Type::Other,
				});
				return;
			}

			this->addItem({
				"delete",
				fmt::format("deleted {}", e->info),
				sf::Color::White,
				Item::Type::CommandOutput,
			});

			this->scenetree->deleteEntity(id);
		}});

	commands.emplace_back(Command{
		"create",
		"create an entity. e.g. create Console MainConsole",
		[this](std::vector<std::string> tokens) {
			if (tokens.size() < 2 || tokens.size() > 3)
			{
				addItem({
					"create",
					"Failed. Tokens must be between 2 and 3",
					sf::Color::Red,
					Item::Type::Other,
				});
				return;
			}

			HashedID entity_type = hash(tokens[0]);

			Entity* ent = nullptr;

			if (tokens.size() == 2)
			{
				ent = this->scenetree->createEntityLocal(
					entity_type,
					tokens[1]
				);
			}
			else if (tokens.size() == 3)
			{
				ent = this->scenetree->createEntityLocal(
					entity_type,
					tokens[1],
					std::stoi(tokens[2])
				);
			}

			if (!ent)
			{
				addItem({
					"create",
					"Failed. The scenetree could not construct that entity",
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

	commands.emplace_back(Command{
		"hash",
		"hash the given string",
		[&](std::vector<std::string> tokens) {
			if (tokens.empty())
			{
				addItem({
					"hash",
					"Failed. You must enter a string",
					sf::Color::Red,
					Item::Type::Other,
				});
				return;
			}

			std::string text;
			for (auto& s : tokens)
				text += s + " ";

			text.pop_back();

			addItem({
				"hash",
				std::to_string(hash(text)),
				sf::Color::White,
				Item::Type::CommandOutput,
			});
		}});

	commands.emplace_back(Command{
		"clear",
		"clear the console",
		[&](std::vector<std::string> tokens) {
			if (!tokens.empty())
			{
				addItem({
					"clear",
					"Failed. This command takes no arguments",
					sf::Color::Red,
					Item::Type::Other,
				});
				return;
			}

			history.clear();
			history_index = -1;
			items.clear();
		}});

	commands.emplace_back(Command{
		"hashes",
		"list all registered hashes",
		[&](std::vector<std::string> tokens) {
			if (!tokens.empty())
			{
				addItem({
					"hashes",
					"Failed. This command takes no arguments",
					sf::Color::Red,
					Item::Type::Other,
				});
				return;
			}

			std::vector<std::string> hashes;

			forEachHash([&](HashedID hash, const std::string& string) {
				hashes.push_back(fmt::format("{:>10}: {}\n", hash, string));
			});

			std::sort(hashes.begin(), hashes.end(), [](const std::string& lhs, const std::string& rhs) {
				return lhs < rhs;
			});

			std::string hashes_concat;
			for (const auto& s : hashes)
				hashes_concat += s;

			this->addItem({
				"hashes",
				hashes_concat,
				sf::Color::White,
				Item::Type::CommandOutput,
			});
		}});

	commands.emplace_back(Command{
		"hashToString",
		"If the hash has been registered, prints the string version",
		[&](std::vector<std::string> tokens) {
			if (tokens.size() != 1)
			{
				addItem({
					"hashToString",
					"Failed. This command requires one parameter",
					sf::Color::Red,
					Item::Type::Other,
				});
				return;
			}

			HashedID hash = std::stoi(tokens[0]);
			std::string hashString = hashToString(hash);

			if (hashString.empty())
			{
				addItem({
					"hashToString",
					fmt::format("Hash {} hasn't been registered so its string value is unknown", hash),
					sf::Color::Red,
					Item::Type::Other,
				});
			}
			else
			{
				addItem({
					"hashToString",
					fmt::format("{} is the hash of {}", hash, hashString),
					sf::Color::White,
					Item::Type::CommandOutput,
				});
			}
		}});
}

void Console::input(Event& e)
{
	auto visitor = overload{
		[&](EventKey e)
		{
			if (e.down && e.key == Keyboard::Key::Quote)
				opened = !opened;
		},
		[](auto&){}
	};

	std::visit(visitor, e);
}

void Console::update(float)
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

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));	// Tighten spacing

		for (const auto& item : items)
		{
			if (!filter.PassFilter(item.text.c_str()))
				continue;

			ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 0.8f, 0.0f, 1.0f});
			ImGui::TextUnformatted(item.prefix.c_str());
			ImGui::PopStyleColor();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));	// Tighten spacing
			ImGui::SameLine(0, -1);
			ImGui::PopStyleVar();

			ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f});
			ImGui::TextUnformatted(": ");
			ImGui::PopStyleColor();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));	// Tighten spacing
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

		static auto callback = [](ImGuiInputTextCallbackData* data) -> int {
			auto console = static_cast<Console*>(data->UserData);

			if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
			{
				const int prev_history_index = console->history_index;

				if (data->EventKey == ImGuiKey_UpArrow)
				{
					if (console->history_index < 0)
						console->history_index = static_cast<int>(console->history.size()) - 1;
					else if (console->history_index > 0)
						console->history_index--;
				}
				else if (data->EventKey == ImGuiKey_DownArrow)
				{
					if (console->history_index >= 0)
					{
						console->history_index++;
						if (console->history_index > static_cast<int>(console->history.size()) - 1)
							console->history_index = -1;
					}
				}

				if (prev_history_index != console->history_index)
				{
					std::string history_str;
					if (console->history_index >= 0
						&& console->history_index < static_cast<int>(console->history.size()))
					{
						history_str = console->history[console->history_index];
					}
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, history_str.c_str());
				}
			}
			return 0;
		};

		bool reclaim_focus = false;
		if (ImGui::InputText("Input",
				user_input.data(),
				user_input.size(),
				ImGuiInputTextFlags_EnterReturnsTrue |
					ImGuiInputTextFlags_CallbackHistory,
				callback,
				static_cast<void*>(this)))
		{
			addInput(user_input);
			user_input.clear();
			user_input.resize(256);
			reclaim_focus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		if (reclaim_focus)
			ImGui::SetKeyboardFocusHere(-1);	// Auto focus previous widget

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
	if (input.empty())
		return;

	//save the untrimmed input, it'll be trimmed later anyway
	history.emplace_back(std::move(input));
	history_index = -1;

	std::string trimmed_input;
	for (const auto& letter : user_input)
	{
		if (!letter)
			break;

		trimmed_input.push_back(letter);
	}

	addItem({
		"input",
		trimmed_input,
		sf::Color::White,
		Item::Type::UserInput,
	});

	std::stringstream ss(trimmed_input);
	std::vector<std::string> tokens;
	std::string token;
	
	while (std::getline(ss, token, ' '))
		tokens.push_back(token);

	if (!tokens.empty())
	{
		const std::string name = tokens[0];

		//commands already know the first token, it's their command name
		tokens.erase(tokens.begin());

		executeCommand(getCommand(name), tokens);
	}
}

Console::Command* Console::getCommand(const std::string& name)
{
	for (auto& c : commands)
	{
		if (c.name == name)
			return &c;
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

	try
	{
		command->function(std::move(tokens));
	}
	catch (std::exception& e)
	{
		addItem({
			"exception",
			e.what(),
			sf::Color::Red,
			Item::Type::Other,
		});
	}
}
}	// namespace enki
