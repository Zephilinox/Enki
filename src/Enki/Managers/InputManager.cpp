#include "InputManager.hpp"

namespace enki
{
InputManager::InputManager()
{
	keys_last_frame.fill(InputState::Unheld);
	keys_this_frame.fill(InputState::Unheld);

	mouse_buttons_last_frame.fill(InputState::Unheld);
	mouse_buttons_this_frame.fill(InputState::Unheld);
}

void InputManager::update()
{
	for (size_t i = 0; i < keys_last_frame.size(); ++i)
	{
		keys_last_frame[i] = keys_this_frame[i];
	}

	for (auto key = sf::Keyboard::Key::A;
		key < sf::Keyboard::Key::KeyCount;
		key = static_cast<sf::Keyboard::Key>(static_cast<int>(key) + 1))
	{
		int state;

		//todo: this is a problem, we should be using events, or we'll miss user input
		if (sf::Keyboard::isKeyPressed(key))
		{
			if (keys_last_frame[key] == InputState::JustReleased
				|| keys_last_frame[key] == InputState::Unheld) 
				state = InputState::JustPressed;
			else
				state = InputState::HeldDown;
		}
		else
		{
			if (keys_last_frame[key] == InputState::JustPressed
				|| keys_last_frame[key] == InputState::HeldDown)
				state = InputState::JustReleased;
			else
				state = InputState::Unheld;
		}

		keys_this_frame[key] = state;
	}

	for (size_t i = 0; i < mouse_buttons_last_frame.size(); ++i)
	{
		mouse_buttons_last_frame[i] = mouse_buttons_this_frame[i];
	}

	for (auto button = sf::Mouse::Button::Left;
		button < sf::Mouse::Button::ButtonCount;
		button = static_cast<sf::Mouse::Button>(static_cast<int>(button) + 1))
	{
		int state;

		//todo: use events for the same reason as above
		if (sf::Mouse::isButtonPressed(button))
		{
			if (mouse_buttons_last_frame[button] == InputState::JustReleased
				|| mouse_buttons_last_frame[button] == InputState::Unheld)
				state = InputState::JustPressed;
			else
				state = InputState::HeldDown;
		}
		else
		{
			if (mouse_buttons_last_frame[button] == InputState::JustPressed
				|| mouse_buttons_last_frame[button] == InputState::HeldDown)
				state = InputState::JustReleased;
			else
				state = InputState::Unheld;
		}

		mouse_buttons_this_frame[button] = state;
	}

	mouse_desktop_pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition());
	mouse_screen_pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(*window));
}

bool InputManager::isKeyUp(int key)
{
	return (keys_this_frame[key] == InputState::Unheld
		|| keys_this_frame[key] == InputState::JustReleased);
}

bool InputManager::isKeyReleased(int key)
{
	return (keys_this_frame[key] == InputState::JustReleased);
}

bool InputManager::isKeyPressed(int key)
{
	return (keys_this_frame[key] == InputState::JustPressed);
}

bool InputManager::isKeyDown(int key)
{
	return (keys_this_frame[key] == InputState::HeldDown
		|| keys_this_frame[key] == InputState::JustPressed);
}

bool InputManager::isMouseButtonUp(int button)
{
	return (mouse_buttons_this_frame[button] == InputState::Unheld
		|| mouse_buttons_this_frame[button] == InputState::JustReleased);
}

bool InputManager::isMouseButtonPressed(int button)
{
	return (mouse_buttons_this_frame[button] == InputState::JustPressed);
}

bool InputManager::isMouseButtonReleased(int button)
{
	return (mouse_buttons_this_frame[button] == InputState::JustReleased);
}

bool InputManager::isMouseButtonDown(int button)
{
	return (mouse_buttons_this_frame[button] == InputState::HeldDown
		|| mouse_buttons_this_frame[button] == InputState::JustPressed);
}

sf::Vector2f InputManager::getMouseDesktopPos() const
{
	return mouse_desktop_pos;
}

sf::Vector2f InputManager::getMouseScreenPos() const
{
	return mouse_screen_pos;
}

sf::Vector2f InputManager::getMouseWorldPos() const
{
	return window->mapPixelToCoords(static_cast<sf::Vector2i>(mouse_screen_pos));
}
}	// namespace enki