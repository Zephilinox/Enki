#include "WindowSFML.hpp"

//SELF
#include "Enki/Input/SFML/KeyConversion.hpp"

namespace enki
{
WindowSFML::WindowSFML(Properties properties)
	: Window(std::move(properties))
	, window(sf::VideoMode{properties.width, properties.height}, properties.title)
{
	window.setVerticalSyncEnabled(properties.vsync);
}

bool WindowSFML::poll(Event& e)
{
	sf::Event event;
	bool more_available = window.pollEvent(event);

	switch (event.type)
	{
		case sf::Event::Closed:
		{
			e = EventQuit{};
		} break;

		case sf::Event::Resized:
		{
			e = EventResize{event.size.width, event.size.height};
		} break;

		case sf::Event::LostFocus:
		{
			e = EventFocus{false};
		} break;

		case sf::Event::GainedFocus:
		{
			e = EventFocus{true};
		} break;

		case sf::Event::TextEntered:
		{
			e = EventCharacter{event.text.unicode};
		} break;

		case sf::Event::KeyPressed:
		{
			e = EventKey{
				keyConversionFromSFML(event.key.code),
				true,
				static_cast<unsigned int>(event.key.code)};
		} break;

		case sf::Event::KeyReleased:
		{
			e = EventKey{
				keyConversionFromSFML(event.key.code),
				false,
				static_cast<unsigned int>(event.key.code)};
		} break;

		case sf::Event::MouseWheelMoved:
		{
			e = EventUnknown{event.type};
		} break;

		case sf::Event::MouseWheelScrolled:
		{
			if (event.mouseWheelScroll.wheel == sf::Mouse::HorizontalWheel)
				e = EventMouseWheel{event.mouseWheelScroll.delta, 0};
			else if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
				e = EventMouseWheel{0, event.mouseWheelScroll.delta};
		} break;

		case sf::Event::MouseButtonPressed:
		{
			e = EventMouseButton{
				buttonConversionFromSFML(event.mouseButton.button),
				true};
		} break;

		case sf::Event::MouseButtonReleased:
		{
			e = EventMouseButton{
				buttonConversionFromSFML(event.mouseButton.button),
				false};
		} break;

		case sf::Event::MouseMoved:
		{
			e = EventMouseMove{event.mouseMove.x, event.mouseMove.y};
		} break;

		case sf::Event::MouseEntered:
		{
			e = EventMouseWindow{false};
		} break;

		case sf::Event::MouseLeft:
		{
			e = EventMouseWindow{true};
		} break;

		case sf::Event::JoystickButtonPressed:
		{
			e = EventUnknown{event.type};
		} break;

		case sf::Event::JoystickButtonReleased:
		{
			e = EventUnknown{event.type};
		} break;

		case sf::Event::JoystickMoved:
		{
			e = EventUnknown{event.type};
		} break;

		case sf::Event::JoystickConnected:
		{
			e = EventUnknown{event.type};
		} break;

		case sf::Event::JoystickDisconnected:
		{
			e = EventUnknown{event.type};
		} break;

		case sf::Event::TouchBegan:
		{
			e = EventUnknown{event.type};
		} break;

		case sf::Event::TouchMoved:
		{
			e = EventUnknown{event.type};
		} break;

		case sf::Event::TouchEnded:
		{
			e = EventUnknown{event.type};
		} break;

		case sf::Event::SensorChanged:
		{
			e = EventUnknown{event.type};
		} break;
	}

	return more_available;
}

bool WindowSFML::isOpen()
{
	return window.isOpen();
}

bool WindowSFML::isVerticalSyncEnabled()
{
	return vsync;
}

void WindowSFML::close()
{
	window.close();
}

void WindowSFML::clear(int r, int g, int b)
{
	window.clear({static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b), 255});
}

void WindowSFML::display()
{
	window.display();
}

void WindowSFML::setVerticalSyncEnabled(bool enabled)
{
	vsync = enabled;
	window.setVerticalSyncEnabled(enabled);
};

sf::RenderWindow* WindowSFML::getRawWindow()
{
	return &window;
}

}