#pragma once

//SELF
#include "Enki/Entity.hpp"
#include "IMGUI/imgui.h"

namespace enki
{
	class Console : public enki::Entity
	{
	public:

		struct Item
		{
			enum class Type
			{
				Other,
				UserInput,
				CommandOutput,
			};

			std::string prefix;
			std::string text;
			sf::Color colour = sf::Color::White;
			Type type;
		};

		struct Command
		{

			std::string name;
			std::string description;
			std::function<void(std::vector<std::string>)> function;
		};

		Console(enki::EntityInfo info, enki::GameData* game_data);

		void input(sf::Event& e) override;
		void update(float dt) override;

	private:
		void addItem(Item item);
		void addInput(std::string input);
		void executeCommand(Command* command, std::vector<std::string> tokens);
		Command* getCommand(std::string name);

		bool opened = false;
		std::string user_input;
		std::vector<Item> items;
		std::vector<std::string> history;
		std::vector<Command> commands;
		ImGuiTextFilter filter;
	};
}
