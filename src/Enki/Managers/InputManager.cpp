#include "InputManager.hpp"

namespace enki
{
	InputManager::InputManager()
	{
		keys_last_frame.fill(InputState::Up);
		keys_this_frame.fill(InputState::Up);

		mouse_buttons_last_frame.fill(InputState::Up);
		mouse_buttons_this_frame.fill(InputState::Up);
	}

	void InputManager::update()
	{
		for (size_t i = 0; i < keys_last_frame.size(); ++i)
		{
			keys_last_frame[i] = keys_this_frame[i];
		}

		for (sf::Keyboard::Key key = sf::Keyboard::Key::A; key < sf::Keyboard::Key::KeyCount;)
		{
			InputState state;

			if (sf::Keyboard::isKeyPressed(key))
			{
				if (keys_last_frame[key] == InputState::Released ||
					keys_last_frame[key] == InputState::Up)
				{
					state = InputState::Pressed;
				}
				else
				{
					state = InputState::Down;
				}
			}
			else
			{
				if (keys_last_frame[key] == InputState::Released ||
					keys_last_frame[key] == InputState::Up)
				{
					state = InputState::Up;
				}
				else
				{
					state = InputState::Released;
				}
			}

			keys_this_frame[key] = state;
			key = static_cast<sf::Keyboard::Key>(static_cast<int>(key) + 1);
		}

		for (size_t i = 0; i < mouse_buttons_last_frame.size(); ++i)
		{
			mouse_buttons_last_frame[i] = mouse_buttons_this_frame[i];
		}

		for (sf::Mouse::Button button = sf::Mouse::Button::Left; button < sf::Mouse::Button::ButtonCount;)
		{
			InputState state;

			if (sf::Mouse::isButtonPressed(button))
			{
				if (mouse_buttons_last_frame[button] == InputState::Released ||
					mouse_buttons_last_frame[button] == InputState::Up)
				{
					state = InputState::Pressed;
				}
				else
				{
					state = InputState::Down;
				}
			}
			else
			{
				if (mouse_buttons_last_frame[button] == InputState::Released ||
					mouse_buttons_last_frame[button] == InputState::Up)
				{
					state = InputState::Up;
				}
				else
				{
					state = InputState::Released;
				}
			}

			mouse_buttons_this_frame[button] = state;
			button = static_cast<sf::Mouse::Button>(static_cast<int>(button) + 1);

		}

		mouse_desktop_pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition());
		mouse_screen_pos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(*window));
	}

	bool InputManager::isKeyUp(int key)
	{
		return (keys_this_frame[key] == InputState::Up || keys_this_frame[key] == InputState::Released);
	}

	bool InputManager::isKeyReleased(int key)
	{
		return (keys_this_frame[key] == InputState::Released);
	}

	bool InputManager::isKeyPressed(int key)
	{
		return (keys_this_frame[key] == InputState::Pressed);
	}

	bool InputManager::isKeyDown(int key)
	{
		return (keys_this_frame[key] == InputState::Down || keys_this_frame[key] == InputState::Pressed);
	}

	bool InputManager::isMouseButtonUp(int button)
	{
		return (mouse_buttons_this_frame[button] == InputState::Up || mouse_buttons_this_frame[button] == InputState::Released);
	}

	bool InputManager::isMouseButtonPressed(int button)
	{
		return (mouse_buttons_this_frame[button] == InputState::Pressed);
	}

	bool InputManager::isMouseButtonReleased(int button)
	{
		return (mouse_buttons_this_frame[button] == InputState::Released);
	}

	bool InputManager::isMouseButtonDown(int button)
	{
		return (mouse_buttons_this_frame[button] == InputState::Down || mouse_buttons_this_frame[button] == InputState::Pressed);
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
}