#pragma once

//LIBS
#include <SFML/Graphics.hpp>

//STD
#include <array>
#include <mutex>
#include <string>
#include <map>
#include <experimental/map>

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

	bool isKeyUp(int key);
	bool isKeyReleased(int key);
	bool isKeyPressed(int key);
	bool isKeyDown(int key);

	bool isMouseButtonUp(int button);
	bool isMouseButtonPressed(int button);
	bool isMouseButtonReleased(int button);
	bool isMouseButtonDown(int button);
	
	sf::Vector2f getMouseDesktopPos();
	sf::Vector2f getMouseScreenPos();
	sf::Vector2f getMouseWorldPos();

	sf::RenderWindow* window;

private:
	std::multimap<std::string, int> actions;

	std::array<int, sf::Keyboard::Key::KeyCount> keys_last_frame;
	std::array<int, sf::Keyboard::Key::KeyCount> keys_this_frame;

	std::array<int, sf::Mouse::Button::ButtonCount> mouse_buttons_last_frame;
	std::array<int, sf::Mouse::Button::ButtonCount> mouse_buttons_this_frame;

	sf::Vector2f mouse_desktop_pos;
	sf::Vector2f mouse_screen_pos;
};
