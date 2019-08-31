#pragma once

//STD
#include <array>
#include <experimental/map>
#include <map>
#include <string>

//LIBS
#include <SFML/Graphics.hpp>

namespace enki
{
enum InputState
{
	Released,
	Pressed,
	Down,
	Up,
};

class InputManager
{
public:
	InputManager();

	void update();

	[[nodiscard]] bool isKeyUp(int key);

	[[nodiscard]] bool isKeyReleased(int key);

	[[nodiscard]] bool isKeyPressed(int key);

	[[nodiscard]] bool isKeyDown(int key);

	[[nodiscard]] bool isMouseButtonUp(int button);

	[[nodiscard]] bool isMouseButtonPressed(int button);

	[[nodiscard]] bool isMouseButtonReleased(int button);

	[[nodiscard]] bool isMouseButtonDown(int button);

	[[nodiscard]] sf::Vector2f getMouseDesktopPos() const;

	[[nodiscard]] sf::Vector2f getMouseScreenPos() const;

	[[nodiscard]] sf::Vector2f getMouseWorldPos() const;

	sf::RenderWindow* window = nullptr;

private:
	std::multimap<std::string, int> actions;

	std::array<int, sf::Keyboard::Key::KeyCount> keys_last_frame;
	std::array<int, sf::Keyboard::Key::KeyCount> keys_this_frame;

	std::array<int, sf::Mouse::Button::ButtonCount> mouse_buttons_last_frame;
	std::array<int, sf::Mouse::Button::ButtonCount> mouse_buttons_this_frame;

	sf::Vector2f mouse_desktop_pos;
	sf::Vector2f mouse_screen_pos;
};
}	// namespace enki