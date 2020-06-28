#include "InputManager.hpp"

//SELF
#include "Enki/Window/WindowSFML.hpp"

namespace enki
{
InputManager::InputManager()
{
	keys_last_frame.fill(InputState::Unheld);
	keys_this_frame.fill(InputState::Unheld);

	mouse_buttons_last_frame.fill(InputState::Unheld);
	mouse_buttons_this_frame.fill(InputState::Unheld);
}

void InputManager::input(const Event& e)
{
	const auto visitor = overload{
		[this](const EventKey& e) {
			if (e.key == Keyboard::Key::Unknown)
				return;	//todo: I'd rather not need to do this error checking
			
			const auto key = static_cast<std::size_t>(e.key);

			int state;
			
			if (this->keys_last_frame[key] == InputState::JustPressed
				|| this->keys_last_frame[key] == InputState::HeldDown)
			{
				if (e.down)
					state = InputState::HeldDown;
				else
					state = InputState::JustReleased;
			}
			else if (this->keys_last_frame[key] == InputState::JustReleased
				|| this->keys_last_frame[key] == InputState::Unheld)
			{
				if (e.down)
					state = InputState::JustPressed;
				else
					state = InputState::Unheld;
			}
			else
			{
				throw; //unreachable
			}
			
			this->keys_this_frame[key] = state;
		},
		[this](const EventMouseButton& e) {
			if (e.button == Mouse::Button::Unknown)
				return; //todo: I'd rather not need to do this error checking
			
			const auto button = static_cast<std::size_t>(e.button);

			int state;

			if (this->mouse_buttons_last_frame[button] == InputState::JustPressed
				|| this->mouse_buttons_last_frame[button] == InputState::HeldDown)
			{
				if (e.down)
					state = InputState::HeldDown;
				else
					state = InputState::JustReleased;
			}
			else if (this->mouse_buttons_last_frame[button] == InputState::JustReleased
				|| this->mouse_buttons_last_frame[button] == InputState::Unheld)
			{
				if (e.down)
					state = InputState::JustPressed;
				else
					state = InputState::Unheld;
			}
			else
			{
				throw;	//unreachable, todo: change code so this isn't needed
			}

			this->mouse_buttons_this_frame[button] = state;
		},
		[this](const EventMouseMove& e)
		{
			mouse_screen_pos = sf::Vector2f(e.x, e.y);
			const auto window_pos = window->as<WindowSFML>()->getRawWindow()->getPosition();
			mouse_desktop_pos = sf::Vector2f(e.x + window_pos.x, e.y + window_pos.y);
		},
		[](const auto&) {}
	};
	
	std::visit(visitor, e);
}

void InputManager::update()
{
	for (size_t i = 0; i < keys_last_frame.size(); ++i)
		keys_last_frame[i] = keys_this_frame[i];

	for (size_t i = 0; i < mouse_buttons_last_frame.size(); ++i)
		mouse_buttons_last_frame[i] = mouse_buttons_this_frame[i];
}

bool InputManager::isKeyUp(Keyboard::Key key)
{
	const auto k = static_cast<std::size_t>(key);
	return (keys_this_frame[k] == InputState::Unheld
		|| keys_this_frame[k] == InputState::JustReleased);
}

bool InputManager::isKeyReleased(Keyboard::Key key)
{
	const auto k = static_cast<std::size_t>(key);
	return (keys_this_frame[k] == InputState::JustReleased);
}

bool InputManager::isKeyPressed(Keyboard::Key key)
{
	const auto k = static_cast<std::size_t>(key);
	return (keys_this_frame[k] == InputState::JustPressed);
}

bool InputManager::isKeyDown(Keyboard::Key key)
{
	const auto k = static_cast<std::size_t>(key);
	return (keys_this_frame[k] == InputState::HeldDown
		|| keys_this_frame[k] == InputState::JustPressed);
}

bool InputManager::isMouseButtonUp(Mouse::Button button)
{
	const auto b = static_cast<std::size_t>(button);
	return (mouse_buttons_this_frame[b] == InputState::Unheld
		|| mouse_buttons_this_frame[b] == InputState::JustReleased);
}

bool InputManager::isMouseButtonPressed(Mouse::Button button)
{
	const auto b = static_cast<std::size_t>(button);
	return (mouse_buttons_this_frame[b] == InputState::JustPressed);
}

bool InputManager::isMouseButtonReleased(Mouse::Button button)
{
	const auto b = static_cast<std::size_t>(button);
	return (mouse_buttons_this_frame[b] == InputState::JustReleased);
}

bool InputManager::isMouseButtonDown(Mouse::Button button)
{
	const auto b = static_cast<std::size_t>(button);
	return (mouse_buttons_this_frame[b] == InputState::HeldDown
		|| mouse_buttons_this_frame[b] == InputState::JustPressed);
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
	return static_cast<WindowSFML*>(window)->getRawWindow()->mapPixelToCoords(static_cast<sf::Vector2i>(mouse_screen_pos));
}
}	// namespace enki