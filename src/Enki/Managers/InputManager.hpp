#pragma once

//SELF
#include "Enki/Window/Window.hpp"

//LIBS
#include <SFML/System/Vector2.hpp>

//STD
#include <array>
#include <experimental/map>
#include <map>
#include <string>

namespace enki
{
namespace InputState
{
	enum
	{
		JustReleased,
		JustPressed,
		HeldDown,
		Unheld,
	};
};

class InputManager
{
public:
	InputManager();

	// Call for every event that needs to be handled.
	// note: if multiple events for the same thing occur in a frame, only the last event handled is tracked
	// e.g. if in a frame a player releases, presses, and releases a key again, then only the last release is accounted for
	// for situations where you need to keep track of intermediary key states within a frame occuring, you'll need to figure out your own solution
	// todo: could the input manager keep track of this without too much effort? e.g. count how many transitions occured and of what type
	// for games that have multiple updates per viewable frame, this is less of an issue, since each update is such a small slice of time.
	// but still an issue. In these situations, ignore this input manager and handle all the events manually
	void input(const Event& e);
	// Swaps the input states around (double buffering), call it once per frame (before handling the input)
	void update();

	[[nodiscard]] bool isKeyUp(Keyboard::Key key);
	[[nodiscard]] bool isKeyReleased(Keyboard::Key key);
	[[nodiscard]] bool isKeyPressed(Keyboard::Key key);
	[[nodiscard]] bool isKeyDown(Keyboard::Key key);

	[[nodiscard]] bool isMouseButtonUp(Mouse::Button button);
	[[nodiscard]] bool isMouseButtonPressed(Mouse::Button button);
	[[nodiscard]] bool isMouseButtonReleased(Mouse::Button button);
	[[nodiscard]] bool isMouseButtonDown(Mouse::Button button);
	
	[[nodiscard]] sf::Vector2f getMouseDesktopPos() const;
	[[nodiscard]] sf::Vector2f getMouseScreenPos() const;
	[[nodiscard]] sf::Vector2f getMouseWorldPos() const;

	Window* window = nullptr;

private:
	std::multimap<std::string, int> actions;

	std::array<int, static_cast<std::size_t>(Keyboard::Key::KeyCount)> keys_last_frame;
	std::array<int, static_cast<std::size_t>(Keyboard::Key::KeyCount)> keys_this_frame;

	std::array<int, static_cast<std::size_t>(Mouse::Button::ButtonCount)> mouse_buttons_last_frame;
	std::array<int, static_cast<std::size_t>(Mouse::Button::ButtonCount)> mouse_buttons_this_frame;

	sf::Vector2f mouse_desktop_pos;
	sf::Vector2f mouse_screen_pos;
};
}	// namespace enki