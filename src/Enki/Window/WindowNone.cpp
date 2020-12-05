#include "WindowNone.hpp"

namespace enki
{

WindowNone::WindowNone()
	: Window(type, {})
{
}

WindowNone::WindowNone(Properties properties)
	: Window(type, std::move(properties))
{
	
}

bool WindowNone::poll(Event& e)
{
	return false;
}

bool WindowNone::isOpen() const
{
	return open;
}

bool WindowNone::isVerticalSyncEnabled() const
{
	return properties.vsync;
}

unsigned int WindowNone::getWidth() const
{
	return 0;
}

unsigned int WindowNone::getHeight() const
{
	return 0;
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
	properties.vsync = enabled;
}
void WindowNone::setWidth(unsigned int width) {}
void WindowNone::setHeight(unsigned int height){};

}	 // namespace enki