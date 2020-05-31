#include "WindowNone.hpp"

//SELF
#include "Enki/Input/SFML/KeyConversion.hpp"

namespace enki
{
WindowNone::WindowNone(Properties properties)
	: Window(std::move(properties))
{
	vsync = properties.vsync;
}

bool WindowNone::poll(Event& e)
{
	return false;
}

bool WindowNone::isOpen()
{
	return open;
}

bool WindowNone::isVerticalSyncEnabled()
{
	return vsync;
}

void WindowNone::close()
{
	open = false;
}

void WindowNone::clear(int r, int g, int b)
{
	
}

void WindowNone::display()
{
	
}

void WindowNone::setVerticalSyncEnabled(bool enabled)
{
	vsync = enabled;
};

}	 // namespace enki