#include "WindowLog.hpp"

//LIBS
#include <spdlog/spdlog.h>

//SELF
#include "WindowNone.hpp"

namespace enki
{

WindowLog::WindowLog()
	: Window(type, {})
	, window(std::make_unique<WindowNone>())
{
}

WindowLog::WindowLog(std::unique_ptr<Window> window)
	: Window(type, {})
	, window(std::move(window))
{
}

bool WindowLog::poll(Event& e)
{
	const auto result = window->poll(e);
	spdlog::info("Poll = {}", result);
	return result;
}

bool WindowLog::isOpen() const
{
	const auto result = window->isOpen();
	spdlog::info("isOpen = {}", result);
	return result;
}

bool WindowLog::isVerticalSyncEnabled() const
{
	const auto result = window->isVerticalSyncEnabled();
	spdlog::info("isVerticalSyncEnabled = {}", result);
	return result;
}

unsigned int WindowLog::getWidth() const
{
	return 0;
}

unsigned int WindowLog::getHeight() const
{
	return 0;
}

void WindowLog::close()
{
	window->close();
	spdlog::info("close");
}

void WindowLog::clear(int r, int g, int b)
{
	window->clear(r, g, b);
	spdlog::info("clear = {{{}, {}, {}}}", r, g, b);
}

void WindowLog::display()
{
	window->display();
	spdlog::info("display");
}

void WindowLog::setVerticalSyncEnabled(bool enabled)
{
	window->setVerticalSyncEnabled(enabled);
	spdlog::info("setVerticalSyncEnabled = {}", enabled);
}

void WindowLog::setWidth(unsigned int width)
{
}

void WindowLog::setHeight(unsigned int height)
{
}

Window& WindowLog::getWindow() const
{
	spdlog::info("getWindow = {}", window->getType());
	return *window;
}

}	 // namespace enki