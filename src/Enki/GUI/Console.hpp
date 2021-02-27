#pragma once

//SELF
#include "Enki/Entity.hpp"
#include <Enki/GUI/IMGUI/imgui.h> //todo: move out

namespace enki
{

class Scenetree;

class Console : public Entity
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

	Console(EntityInfo info, Scenetree* scenetree);

	void input(Event& e) override;
	void update(float dt) override;

	void addItem(Item item);

private:
	void addInput(std::string input);
	void executeCommand(Command* command, std::vector<std::string> tokens);
	Command* getCommand(const std::string& name);

	Scenetree* scenetree;
	
	bool opened = false;
	std::string user_input;
	std::vector<Item> items;
	std::vector<std::string> history;
	int history_index = 0;
	std::vector<Command> commands;
	ImGuiTextFilter filter;
};
}	// namespace enki
