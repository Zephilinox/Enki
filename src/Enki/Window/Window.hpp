#pragma once

//STD
#include <string>

//LIBS
#include <SFML/Graphics.hpp>

//SELF
#include "Enki/NonCopyable.hpp"
#include "Enki/Hash.hpp"
#include "Enki/Input/Events.hpp"

namespace enki
{
//using Event = sf::Event;

class Window : private enki::NonCopyableAndNonMovable
{
public:
	struct Properties
	{
		unsigned int width{};
		unsigned int height{};
		std::string title{};
		bool vsync{false};
	};

	Window(Properties properties)
		: properties(std::move(properties)){};
	Window(Window&&) noexcept = default;
	Window& operator=(Window&&) noexcept = default;
	virtual ~Window() noexcept = default;

	virtual bool poll(Event& e) = 0;
	virtual bool isOpen() = 0;
	virtual bool isVerticalSyncEnabled() = 0;

	virtual void close() = 0;
	virtual void clear(int r, int g, int b) = 0;
	virtual void display() = 0;

	virtual void setVerticalSyncEnabled(bool enabled) = 0;

	virtual HashedID getType() const = 0;

protected:
	Properties properties;
};
}	 // namespace enki
